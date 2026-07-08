#!/usr/bin/env python3
"""capture_dgroup.py -- capture the running engine's DGROUP service vectors.

Talks to the QEMU HMP monitor of a running oracle instance, dumps all physical
RAM (pmemsave), locates the 16-bit engine (FIST.DAT) by its data strings,
derives the load base, and prints the DGROUP service-vector table -- in
particular the far pointer at DGROUP:0x12 that the engine reaches via
`call far [DGROUP:0x12]`.

Works even after the engine has crashed/exited: DOS does not zero freed memory,
so the last-installed vectors are still resident (dump promptly).

Findings this reproduces (see README.md):
    engine load base   = <found> (segment aligned; e.g. 0x14190 / seg 0x1419)
    DGROUP linear      = base + 0x1c000   (runtime segment = load_seg + 0x1c00)
    DGROUP:0x0e..0x36  = far-pointer table into the engine's service code
                         segment (load_seg + 0xF69), 4-byte stride
    DGROUP:0x12        = <seg>:01b2  -> engine routine at base-0 offset 0xf842
                         (a guarded word-copy/relocation service)

Usage:
    capture_dgroup.py [MON_PORT] [RAMFILE]
        MON_PORT default 5512 ; RAMFILE default ./ram.bin (relative -> qemu cwd
        via the monitor, then read back here).  Pass an absolute path only if it
        contains no '/' after quoting -- the HMP parser treats '/' as division,
        so this script always quotes the filename.
"""
import socket, sys, time, re, struct

MON_PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 5512
RAM      = sys.argv[2] if len(sys.argv) > 2 else "engram.bin"

def hmp(cmds, port=MON_PORT):
    s = socket.create_connection(("127.0.0.1", port), timeout=15); s.settimeout(3)
    def drain():
        o = b""
        while True:
            try: d = s.recv(65536)
            except socket.timeout: break
            if not d: break
            o += d
        return o
    drain()
    for c in cmds:
        s.sendall((c + "\n").encode())
        time.sleep(2.5 if c.startswith(("pmemsave", "memsave")) else 0.4)
        drain()
    s.close()

# 1. dump all 32 MB of physical RAM (filename MUST be quoted for the HMP parser)
hmp([f'pmemsave 0 0x2000000 "{RAM}"'])
data = open(RAM, "rb").read()
print(f"[capture] RAM dump: {len(data)} bytes")

# 2. locate the engine by anchor strings, derive load base
#    (offsets of these strings in re_out/fist_dat_image.bin, the base-0 image)
IMG_OFF = {b"MAINMENU": 0x25f6e, b"Armored": 0x2d786, b"AF-FD": 0x31c3e}
bases = {}
for s, io in IMG_OFF.items():
    for m in re.finditer(re.escape(s), data):
        bases.setdefault(m.start() - io, 0)
        bases[m.start() - io] += 1
if not bases:
    print("[capture] engine not resident in this dump (boot further / dump sooner)"); sys.exit(1)
base = max(bases, key=bases.get)
load_seg = base >> 4
print(f"[capture] engine load base = {base:#07x} (segment {load_seg:#06x}), votes={bases[base]}")

# 3. DGROUP = base + 0x1c000 (static model DGROUP segment 0x1c00)
DG = base + 0x1c000
print(f"[capture] DGROUP linear = {DG:#07x} (runtime segment {load_seg + 0x1c00:#06x})")

# 4. print the service-vector table (far pointers, 4-byte stride from 0x0e)
print("[capture] DGROUP service vectors (far ptr seg:off):")
for off in range(0x02, 0x40, 4):
    o, seg = struct.unpack_from("<HH", data, DG + off)
    tgt = (seg << 4) + o
    tag = "   <== DGROUP:0x12  (call far target)" if off == 0x12 else ""
    b0  = f"base-0 {tgt-base:#07x}" if base <= tgt < base + 0x40000 else "extern"
    print(f"    DGROUP:0x{off:02x} = {seg:04x}:{o:04x}  linear {tgt:#07x}  ({b0}){tag}")

o, seg = struct.unpack_from("<HH", data, DG + 0x12)
print(f"\n[capture] *** DGROUP:0x12 = {seg:04x}:{o:04x}  linear {(seg<<4)+o:#07x} "
      f"(engine base-0 {((seg<<4)+o)-base:#07x}) ***")
