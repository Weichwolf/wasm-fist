#!/usr/bin/env python3
"""Triage decompiled variable widths against the widths the image actually uses.  board:0023

Ghidra infers a DGROUP variable's width from how it happens to see it used.  A too-WIDE declaration is
the damaging direction: the macro overlaps its neighbour, so every store corrupts an unrelated live
variable.  Patches 546, 547 and 549 each fixed one instance of exactly that.

This scans build/fist.c for `DAT_2000_*` macros, finds every reference to each one's 2-byte absolute
displacement in the engine image, and classifies it by the opcode immediately preceding it.  It reports:

  --byte    undefined2 macros with byte-only accesses     (candidates for undefined1)
  --dword   undefined4 macros with no 0x66-prefixed access (candidates for undefined2)

`succ` is the number of image references to the NEXT byte.  A too-wide byte variable is only harmful
when its successor is itself live, so that column is the priority order.

This is TRIAGE, not proof.  The 2-byte search also hits data, and an operand-size prefix further than
five bytes away is missed.  Read the instruction before changing any declaration.

    python3 tools/width_triage.py --byte | head -40
"""
import re, sys, os, signal

signal.signal(signal.SIGPIPE, signal.SIG_DFL)   # so `| head` exits quietly

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IMG  = os.path.join(ROOT, 're_out/fist_dat_image.bin')
SRC  = os.path.join(ROOT, 'build/fist.c')

# opcode signatures immediately preceding a 2-byte absolute displacement (modrm /06 and moffs forms)
def _sig(pairs):
    return set(pairs)
BYTE = _sig([(0xa0,), (0xa2,), (0xc6, 0x06), (0x80, 0x3e), (0xf6, 0x06), (0xfe, 0x06), (0xfe, 0x0e), (0x84, 0x06)]
            + [(op, mod) for op in (0x88, 0x8a, 0x38, 0x3a, 0x80)
               for mod in (0x06, 0x0e, 0x16, 0x1e, 0x26, 0x2e, 0x36, 0x3e)]
            + [(op, 0x06) for op in (0x00, 0x02, 0x08, 0x0a, 0x10, 0x12, 0x18, 0x1a,
                                     0x20, 0x22, 0x28, 0x2a, 0x30, 0x32)])
WORD = _sig([(0xa1,), (0xa3,), (0xc7, 0x06), (0x83, 0x3e), (0x81, 0x3e), (0xf7, 0x06),
             (0xff, 0x06), (0xff, 0x0e), (0x85, 0x06)]
            + [(op, mod) for op in (0x89, 0x8b, 0x39, 0x3b, 0x83, 0x81, 0x8c, 0x8e)
               for mod in (0x06, 0x0e, 0x16, 0x1e, 0x26, 0x2e, 0x36, 0x3e)]
            + [(op, 0x06) for op in (0x01, 0x03, 0x09, 0x0b, 0x11, 0x13, 0x19, 0x1b,
                                     0x21, 0x23, 0x29, 0x2b, 0x31, 0x33)])
MOVZB = (0x0f, 0xb6, 0x06)

def classify(img, off):
    """-> (byte_refs, word_refs, dword_refs) for the DGROUP offset `off`."""
    disp = bytes([off & 0xff, (off >> 8) & 0xff])
    i = b = w = dw = 0
    while True:
        i = img.find(disp, i)
        if i < 0:
            return b, w, dw
        p1 = (img[i-1],) if i >= 1 else ()
        p2 = (img[i-2], img[i-1]) if i >= 2 else ()
        p3 = tuple(img[i-3:i]) if i >= 3 else ()
        wide = i >= 3 and 0x66 in img[i-5:i-1]
        if p3 == MOVZB:                  b += 1
        elif p2 in WORD or p1 in WORD:   (dw := dw + 1) if wide else (w := w + 1)
        elif p2 in BYTE or p1 in BYTE:   b += 1
        i += 1

def macros(src, ctype):
    pat = r'#define (_?DAT_2000_[0-9a-f]+) \(\*\(%s \*\)\(g_mem\+0x2([0-9a-f]+)\)\)' % ctype
    for m in re.finditer(pat, src):
        yield m.group(1), int(m.group(2), 16) + 0x4000   # DAT_2000_X lives at DGROUP offset X+0x4000

def main():
    mode = sys.argv[1] if len(sys.argv) > 1 else '--byte'
    img, src = open(IMG, 'rb').read(), open(SRC).read()
    rows = []
    if mode == '--byte':
        for name, off in macros(src, 'undefined2'):
            b, w, dw = classify(img, off)
            if b and not w and not dw:
                rows.append((classify(img, off + 1)[0] + classify(img, off + 1)[1], b, name, off))
        rows.sort(reverse=True)
        print('undefined2 macros accessed only by BYTE: %d  (of which %d have a live successor)'
              % (len(rows), sum(1 for r in rows if r[0])))
        print('%-20s %-8s %5s %5s' % ('macro', 'DGROUP', 'refs', 'succ'))
        for succ, b, name, off in rows:
            print('%-20s 0x%-6x %5d %5d' % (name, off, b, succ))
    elif mode == '--dword':
        for name, off in macros(src, 'undefined4'):
            b, w, dw = classify(img, off)
            if (b or w) and not dw:
                rows.append((b + w, name, off))
        rows.sort(reverse=True)
        print('undefined4 macros with NO 32-bit access: %d' % len(rows))
        print('%-20s %-8s %5s' % ('macro', 'DGROUP', 'refs'))
        for n, name, off in rows:
            print('%-20s 0x%-6x %5d' % (name, off, n))
    else:
        sys.exit(__doc__)

if __name__ == '__main__':
    main()
