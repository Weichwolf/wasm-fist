#!/usr/bin/env python3
# extract_image.py -- static extractor for the FIST.RUN 32-bit application image.
#
# Target: armoredfist/FIST.RUN  (NovaLogic "Armored Fist", 1994; 71626 bytes)
# Authorized preservation/porting work on a legally-owned game.
#
# ============================================================================
# ON-DISK FORMAT (all values traced to specific header fields / instructions;
# see docs/recon.md for the outer MZ analysis and the notes below each field):
# ============================================================================
#
#   FIST.RUN
#   0x00000..0x0053a  Stub MZ #1  (real-mode DPMI probe; not used here)
#   0x0053a..0x117ca  Extender MZ #2  == the Doug Huffman DOS extender
#                     ("DOS extender Copyright 1991-1994 by Doug Huffman").
#                     cparhdr=10 -> 160B header; module para0 (flat linear
#                     load base of the 16-bit loader) = 0x53a+160 = 0x5da.
#
#   Inside MZ #2, the module splits into two parts:
#
#   0x005da..0x0583a  16-bit real/protected-mode LOADER + GDT template + data.
#                     - GDT template @ file 0x5ea: flat 32-bit selectors
#                       0x20/0x23 (data, base 0, 4GB) and 0x28/0x2b (code32,
#                       base 0, 4GB).  => the 32-bit app runs FLAT, base 0.
#                     - MZ #2 entry (cs:ip 0x6f:0) -> jmp 0x1d00 (file) ->
#                       self-relocate, VCPI/DPMI detect, GDT/IDT setup,
#                       mode switch, then hand off to the 32-bit image.
#
#   0x0583a..0x117ca  32-bit APPLICATION IMAGE (data-segment offset 0 here).
#                     ---- DATA_BASE = 0x583a (file offset of app-relative 0) ----
#                     Pinned to file 0x583a by 5 independent matches: the 32-bit
#                     stub prints '$'-strings via `mov edx,<off>; int 21h/ah=9`,
#                     and off 0xbf2->"No CheckSum", 0xc00->"...Corrupted",
#                     0xb91->"EXTENDER: Memory Error", 0xbac->"...Allocate...",
#                     0xbd0->"...Null Pointer..." each land exactly on the
#                     corresponding string in the file (file = 0x583a + off).
#                     Image size = EOF - 0x583a = 0x117ca-0x583a = 0xbf90 (49040 B).
#                     The app is linked at flat base 0 (all absolute refs are
#                     app-relative offsets < 0xbf90); NO app relocation table
#                     exists (no fixup array in the image; nothing to apply).
#
#   The 32-bit image is SELF-DECRYPTING.  A plaintext stub in the middle of the
#   image checksums then XOR-decrypts the rest in place, then jumps to the real
#   32-bit entry.  Layout in APP-RELATIVE offsets (all figures read out of the
#   32-bit stub disassembled at file 0x6312 / app off 0xad8):
#
#     0x0000..0x0ab4   plaintext  (low data / 0x4b-filled scratch / zero pad)
#     0x0ab4..0x0ad4   ENCRYPTED region 1  (32 B, static keys)
#     0x0ad4..0x0c20   plaintext  (stored checksum @0xaa8, the checksum+decrypt
#                                  stub, and the extender '$'-error strings)
#     0x0c20..0x0bf00  ENCRYPTED region 2  (0xb2e0 B, rolling keys)
#     0x0bf00..0x0bf90 plaintext  ("KYLE" marker, far-ptr records, footer)
#
#   CHECKSUM (verified, not required to decrypt): rolling XOR/ROR over the
#   ENCRYPTED bytes app-off 0xab4..0xbf03, compared to the dword @app-off 0xaa8.
#   For FIST.RUN both equal 0x088c5c9d.  Sentinel 0x1234dead would disable it.
#
#   DECRYPTION (exact transcription of the stub at file 0x635c..0x6399):
#     initial keys  EAX=0x9324abe1  EBX=0x439a9efa
#     region 1 (static): for 4 units of 8 bytes starting at app-off 0xab4:
#                          [p]^=EAX ; [p+4]^=EBX ; p+=8         (EAX,EBX const)
#     region 2 (rolling): count = (0xbf03-0xc20)>>3 = 0x165c (5724) units,
#                         starting at app-off 0xc20, ECX counts DOWN from count:
#                          [p]^=EAX ; [p+4]^=EBX ;
#                          EBX=ror(EBX,1) ; EAX=ror(EAX,1) ; EAX^=ECX ; p+=8
#     (EAX/EBX carry over from region 1 -- the stub does NOT reload them.)
#
#   ENTRY POINT (into the DECRYPTED image): app-off 0x0d9d (file 0x65d7) -- the
#   final `jmp` target of the stub.  After decryption this is valid 32-bit code
#   (C runtime "PROTMAIN": reads PSP:0x80 cmdline, DOS EXEC/alloc, calls game).
#
#   LOAD BASE (for a disassembler such as Ghidra): 0x0.  The flat data/code
#   selectors have base 0 and the app is linked at 0, so app-off == linear addr.
#
# OPEN ITEMS:
#   * BSS beyond the on-disk image (app-off 0xbf90 = EOF) is zero-extended by
#     the runtime; its size is not encoded in the MZ header (minalloc=0) and is
#     computed inside the crt at runtime -> not statically fixed here.
#   * The extender->stub 32-bit handoff EIP (start of the plaintext stub, near
#     file 0x6312 / app-off 0xad8) is reached via the mode-switch; the exact
#     first-instruction EIP would need a runtime trace to confirm to the byte.
#     The DECRYPTED-image entry (0x0d9d) above is confirmed statically.
# ============================================================================

import os
import struct

SRC       = os.path.join(os.path.dirname(__file__), "..", "armoredfist", "FIST.RUN")
OUT_DIR   = os.path.join(os.path.dirname(__file__), "..", "re_out")
OUT_BIN   = os.path.join(OUT_DIR, "fist_image.bin")

DATA_BASE = 0x583a      # file offset of app-relative offset 0
IMG_SIZE  = 0xbf90      # EOF(0x117ca) - DATA_BASE
LOAD_BASE = 0x0         # app is linked flat at 0
ENTRY     = 0x0d9d      # entry into the decrypted image (file 0x65d7)

# encrypted regions, in app-relative offsets (from the stub @ file 0x635c)
R1_OFF, R1_UNITS = 0x0ab4, 4                         # static-key region
R2_OFF, R2_END   = 0x0c20, 0x0bf03                   # rolling-key region
KEY_A, KEY_B     = 0x9324abe1, 0x439a9efa
CSUM_OFF         = 0x0aa8                             # stored checksum dword
CSUM_RANGE       = (0x0ab4, 0x0bf03)                 # checksummed span
MASK             = 0xffffffff


def ror32(x):
    return ((x >> 1) | ((x & 1) << 31)) & MASK


def rd(buf, off):
    return struct.unpack_from("<I", buf, off)[0]


def wr(buf, off, val):
    struct.pack_into("<I", buf, off, val & MASK)


def verify_checksum(img):
    """Rolling XOR/ROR checksum over the ENCRYPTED image, per file 0x6312."""
    esi, end = CSUM_RANGE
    acc = 0
    for _ in range((end - esi) >> 3):          # mov ecx,(0xbf03-0xab4)>>3
        acc ^= rd(img, esi);     acc = ror32(acc)
        acc ^= rd(img, esi + 4); acc = ror32(acc)
        esi += 8
    stored = rd(img, CSUM_OFF)
    return acc, stored


def decrypt(img):
    """Decrypt regions 1 and 2 in place (exact transcription of the stub)."""
    eax, ebx = KEY_A, KEY_B
    # region 1: static keys
    p = R1_OFF
    for _ in range(R1_UNITS):
        wr(img, p,     rd(img, p)     ^ eax)
        wr(img, p + 4, rd(img, p + 4) ^ ebx)
        p += 8
    # region 2: rolling keys (eax/ebx carried over from region 1)
    p = R2_OFF
    ecx = (R2_END - R2_OFF) >> 3
    while ecx > 0:
        wr(img, p,     rd(img, p)     ^ eax)
        wr(img, p + 4, rd(img, p + 4) ^ ebx)
        ebx = ror32(ebx)
        eax = ror32(eax)
        eax ^= ecx
        p += 8
        ecx -= 1


def main():
    with open(SRC, "rb") as f:
        raw = f.read()
    assert len(raw) == 0x117ca, "unexpected FIST.RUN size %d" % len(raw)
    assert DATA_BASE + IMG_SIZE == len(raw), "image does not reach EOF"

    img = bytearray(raw[DATA_BASE:DATA_BASE + IMG_SIZE])

    # cross-check the embedded checksum against the still-encrypted image
    computed, stored = verify_checksum(img)
    csum_ok = (computed == stored) or (stored == 0x1234dead)

    decrypt(img)

    os.makedirs(OUT_DIR, exist_ok=True)
    with open(OUT_BIN, "wb") as f:
        f.write(img)

    r2_units = (R2_END - R2_OFF) >> 3
    print("=== FIST.RUN 32-bit image extract ===")
    print("source file        : %s (%d bytes)" % (os.path.normpath(SRC), len(raw)))
    print("app image file span : 0x%05x .. 0x%05x" % (DATA_BASE, DATA_BASE + IMG_SIZE))
    print("data base (file)    : 0x%05x  (app-off 0 -> this file offset)" % DATA_BASE)
    print("load base (linear)  : 0x%x   (flat, app linked at 0)" % LOAD_BASE)
    print("image size          : 0x%05x (%d bytes)" % (IMG_SIZE, IMG_SIZE))
    print("entry point         : 0x%05x (decrypted; file 0x%05x)"
          % (ENTRY, DATA_BASE + ENTRY))
    print("relocations         : 0 (position-dependent, base 0; no fixup table)")
    print("checksum @0x%04x     : computed=0x%08x stored=0x%08x  %s"
          % (CSUM_OFF, computed, stored, "OK" if csum_ok else "MISMATCH"))
    print("decrypt region 1    : app-off 0x%04x, %d units x 8 B (static keys)"
          % (R1_OFF, R1_UNITS))
    print("decrypt region 2    : app-off 0x%04x..0x%04x, %d units x 8 B (rolling)"
          % (R2_OFF, R2_OFF + r2_units * 8, r2_units))
    print("keys                : EAX=0x%08x EBX=0x%08x" % (KEY_A, KEY_B))
    print("output              : %s" % os.path.normpath(OUT_BIN))
    print()
    print("NOTE: BSS beyond app-off 0x%05x (EOF) is zero-extended at runtime;" % IMG_SIZE)
    print("      its size is not encoded in the MZ header (minalloc=0) -> OPEN.")


if __name__ == "__main__":
    main()
