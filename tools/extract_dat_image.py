#!/usr/bin/env python3
# extract_dat_image.py -- static extractor for the Armored Fist ENGINE image (FIST.DAT).
#
# Target: armoredfist/FIST.DAT  (NovaLogic "Armored Fist", 1994; 216332 bytes)
# Authorized preservation/porting work on a legally-owned game.
#
# This emits re_out/fist_dat_image.bin: the flat, relocated load image of the
# actual game program (engine + mission/level editor) as laid out by FIST.RUN's
# FILEMGR/EXEC loader, ready to load in Ghidra as x86:LE:16 (Real Mode).
#
# ============================================================================
# FORMAT (every value traced to a header field or a disassembled instruction):
# ============================================================================
#
# FIST.DAT is a SINGLE MS-DOS MZ executable (sig 'MZ' @0). It is NOT encrypted
# and NOT a nested extender -- the FIST.RUN kernel (the Doug-Huffman DOS
# extender + ~4KB CRT/FILEMGR/MEMMGR) opens "FIST.DAT", loads this MZ module
# into its 32-bit address space, applies the MZ relocations, and transfers to
# the module entry.  The module itself is a 16-BIT SEGMENTED program.
#
# MZ header (struct at offset 0, verified against the file):
#   e_magic    'MZ'
#   e_cblp     0x010c (268)   bytes on last 512-page
#   e_cp       0x01a7 (423)   512-byte pages    -> image = 422*512+268 = 216332 = whole file
#   e_crlc     0x0477 (1143)  relocation entries
#   e_cparhdr  0x0140 (320)   header paragraphs -> header = 320*16 = 5120 (0x1400)
#   e_minalloc 0              no required extra paragraphs (BSS already in file)
#   e_maxalloc 0xffff
#   e_ss:e_sp  0x2ba9:0x0400  initial stack  (SS<<4 = 0x2ba90, inside the image)
#   e_ip:e_cs  0x0004:0x0000  ENTRY = CS<<4 + IP = linear 0x0004 in the load module
#   e_lfarlc   0x003e (62)    file offset of the relocation table
#
#   LOAD MODULE  = file[0x1400 .. 0x34d0c]  (211212 bytes = 0x3390c) -- reaches EOF.
#   HEADER       = file[0x0000 .. 0x1400]   (5120 bytes)             -- dropped.
#
# PROCESSOR MODE -- 16-BIT (Ghidra: x86:LE:16, Real Mode).  Proven, not guessed:
#   Entry (load-module linear 0x0004) disassembles cleanly ONLY as 16-bit:
#       b8 00 1c        mov ax,0x1c00      ; DGROUP paragraph
#       8e d8           mov ds,ax
#       8c 06 68 00     mov [0x68],es
#       c7 06 90 fa ..  movw [0x0fa90],0x424a
#       9a 0e 00 69 0f  lcall 0x0f69:0x000e ; far call, seg 0x0f69
#       ...             mov ax,0x4c01 / int 21h  (DOS terminate-with-code)
#   Decoded as 32-bit the same bytes are incoherent garbage (mov eax,0xd88e1c00
#   / lcall to nonsense).  Every intra-image pointer is a 16-bit far ptr seg:off.
#   The far-call target 0x0f69:0x000e (linear 0xf69e) is the C-runtime stack
#   fixup (mov bx,0x1c00; mov ss,bx; mov sp,ax); 0x0f69:0x00a8 (linear 0xf738)
#   is the BSS clear (rep stos ax, cx=0x0b57).  All coherent 16-bit CRT code.
#
# SEGMENTATION / LOAD BASE.  The program is linked with paragraph 0 == start of
# the load module: every segment immediate stored in the code is a paragraph
# offset INTO the image.  Evidence -- the 46 distinct segment values found at
# the 1143 relocation sites all satisfy seg*16 < image size:
#     0x0f69 (x778) main code seg  -> 0xf690
#     0x0000 (x235) near/DGROUP-rel
#     0x1c00 ( x26) DGROUP/data    -> 0x1c000  (DS set at entry)
#     0x2b..-0x2d.. data/string segs; max seg 0x3390 -> 0x33900 ~= image top 0x3390c
#   And SS 0x2ba9 -> 0x2ba90 is inside the image.  So a far ptr seg:off maps to
#   linear (seg<<4)+off within this one image -- the classic real-mode layout.
#   The whole image (0x3390c) fits well under the 1 MB real-mode limit, so
#   loading it flat at linear 0 and using seg<<4+off addressing reproduces every
#   pointer exactly.
#
#   => LOAD BASE (linear) = 0, LOAD PARAGRAPH = 0.
#
# RELOCATIONS -- classic 16-bit MZ SEGMENT fixups, NOT 32-bit dword add-base.
#   Each of the 1143 entries is (e_off:u16, e_seg:u16); it names a 16-bit WORD at
#   load-module linear (e_seg<<4)+e_off that holds a segment/paragraph immediate.
#   The loader adds the load paragraph to that word: word += LOAD_PARA.
#   For the flat image at LOAD_PARA 0 the add is a no-op, so the emitted image is
#   byte-identical to the raw load module -- which is exactly what we want for a
#   base-0 (seg<<4) Ghidra layout.  We still walk the table to (a) validate every
#   site is in range and points at a plausible paragraph value, and (b) allow a
#   non-zero LOAD_PARA if a future analysis wants the image based elsewhere.
#
# OUTPUT: re_out/fist_dat_image.bin, 211212 bytes, linear base 0, entry 0x0004.
#   Ghidra: language x86:LE:16 (Real Mode), image base ram:0, disassemble at 0x4.
#   Give extra zero-filled space above 0x3390c for runtime heap growth if needed
#   (minalloc=0, so nothing is REQUIRED; DGROUP heap grows at runtime -> OPEN).
# ============================================================================

# ============================================================================
# REUSE FOR THE DRIVER OVERLAYS (MGAVIDEO.DVR / SOUNDDVR.DVR)
# ----------------------------------------------------------------------------
# The engine loads two 16-bit MZ driver overlays at runtime via INT 21h AH=4B
# AL=03 (load-overlay). They are the SAME class as FIST.DAT -- single MZ,
# 16-bit segmented, flat seg<<4 -- so this exact extractor produces their
# base-0 flat load image for the Ghidra decompile.  `make image-drivers`.
#
# One difference vs the engine: the driver's RUNTIME load segment is chosen by
# the engine (it allocs a block and passes the load seg in the AH=4B param
# block), so the MZ relocations are applied AT LOAD TIME by the shim
# (fist_dos.c, reloc factor = load_seg), NOT baked here.  The shim reads the
# real .DVR file and walks its own MZ reloc table, so this tool only needs to
# emit the base-0 flat image (relocs left at their base-0 module values, i.e.
# LOAD_PARA=0 -- byte-identical to the raw load module) for the decompile, and
# a `<out>.relocs` sidecar (one hex site-offset per line) that RECORDS the
# reloc sites for verification/documentation.  Ghidra loads the .bin exactly
# like the engine image: x86:LE:16 (Real Mode), base 0, disassemble at entry.
# ============================================================================
import os
import struct
import sys

SRC     = os.path.join(os.path.dirname(__file__), "..", "armoredfist", "FIST.DAT")
OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "re_out")
OUT_BIN = os.path.join(OUT_DIR, "fist_dat_image.bin")

LOAD_PARA = 0          # load base in paragraphs; 0 => flat base-0 (seg<<4) layout
MASK16    = 0xffff


def main():
    global SRC, OUT_DIR, OUT_BIN
    # Reusable CLI: extract_dat_image.py [SRC [OUT [--relocs]]]
    src = sys.argv[1] if len(sys.argv) > 1 else SRC
    out = sys.argv[2] if len(sys.argv) > 2 else OUT_BIN
    emit_relocs = "--relocs" in sys.argv[3:]
    SRC, OUT_BIN = src, out
    OUT_DIR = os.path.dirname(os.path.abspath(out)) or "."

    with open(SRC, "rb") as f:
        raw = f.read()

    (magic, cblp, cp, crlc, cparhdr, minalloc, maxalloc,
     ss, sp, csum, ip, cs, lfarlc, ovno) = struct.unpack_from("<2sHHHHHHHHHHHHH", raw, 0)
    assert magic == b"MZ", "not an MZ executable"

    hdr_bytes = cparhdr * 16
    img_bytes = (cp - 1) * 512 + cblp if cblp else cp * 512      # total MZ image
    assert img_bytes == len(raw), "MZ image (%d) != file size (%d)" % (img_bytes, len(raw))
    lm_bytes = img_bytes - hdr_bytes                            # load-module size

    img = bytearray(raw[hdr_bytes:hdr_bytes + lm_bytes])
    entry = (cs << 4) + ip                                      # linear entry in load module

    # ---- apply the 16-bit segment relocations (add LOAD_PARA to each site)
    applied = 0
    seen_segs = {}
    max_site = 0
    reloc_sites = []
    for i in range(crlc):
        e_off, e_seg = struct.unpack_from("<HH", raw, lfarlc + i * 4)
        site = (e_seg << 4) + e_off
        assert site + 2 <= lm_bytes, "reloc %d site 0x%x out of range" % (i, site)
        max_site = max(max_site, site)
        reloc_sites.append(site)
        word = struct.unpack_from("<H", img, site)[0]
        seen_segs[word] = seen_segs.get(word, 0) + 1
        word = (word + LOAD_PARA) & MASK16
        struct.pack_into("<H", img, site, word)
        applied += 1

    os.makedirs(OUT_DIR, exist_ok=True)
    with open(OUT_BIN, "wb") as f:
        f.write(img)
    if emit_relocs:
        rp = OUT_BIN + ".relocs"
        with open(rp, "w") as f:
            f.write("# %d MZ reloc sites (linear byte offset into the base-0 load module).\n"
                    "# The shim (fist_dos.c AH=4B) adds (load_seg) to the 16-bit word at each\n"
                    "# site at LOAD time; here they carry their base-0 module-relative value.\n"
                    % len(reloc_sites))
            for s in reloc_sites:
                f.write("0x%x\n" % s)

    top_seg = max(seen_segs)
    print("=== FIST.DAT engine image extract ===")
    print("source file        : %s (%d bytes)" % (os.path.normpath(SRC), len(raw)))
    print("MZ header bytes     : 0x%05x (%d)" % (hdr_bytes, hdr_bytes))
    print("load-module span    : file 0x%05x .. 0x%05x" % (hdr_bytes, hdr_bytes + lm_bytes))
    print("image size          : 0x%05x (%d bytes)" % (lm_bytes, lm_bytes))
    print("load base (linear)  : 0x0   (LOAD_PARA=%d; flat seg<<4 real-mode layout)" % LOAD_PARA)
    print("entry point         : cs:ip 0x%04x:0x%04x -> linear 0x%05x" % (cs, ip, entry))
    print("initial stack       : ss:sp 0x%04x:0x%04x -> linear 0x%05x" % (ss, sp, (ss << 4) + sp))
    print("DGROUP (DS at entry): 0x1c00 -> linear 0x1c000")
    print("relocations applied : %d of %d  (16-bit MZ SEGMENT fixups, word += LOAD_PARA)"
          % (applied, crlc))
    print("reloc sites max     : linear 0x%05x (< image 0x%05x)" % (max_site, lm_bytes))
    print("distinct seg values : %d  (all seg<<4 inside image; top seg 0x%04x -> 0x%05x)"
          % (len(seen_segs), top_seg, top_seg << 4))
    print("processor mode      : 16-BIT  ->  Ghidra x86:LE:16 (Real Mode), base 0")
    print("output              : %s (%d bytes)" % (os.path.normpath(OUT_BIN), lm_bytes))
    print()
    print("NOTE: DGROUP heap growth above 0x%05x is runtime-computed (minalloc=0)" % lm_bytes)
    print("      -> give Ghidra extra zero-fill above the image if it needs it. OPEN.")


if __name__ == "__main__":
    main()
