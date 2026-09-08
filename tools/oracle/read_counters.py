#!/usr/bin/env python3
"""Read the mission-outcome counters out of an oracle RAM dump.  board:0017

The instrumented DOSBox writes a full 16 MB guest RAM image (`<prefix>.ram.bin`, armed with
FIST_MEMARM_BOOT=1 / FIST_MEMDUMP_N).  The engine's DGROUP sits at a load-dependent guest address, so
this locates it by SIGNATURE rather than assuming an offset: DGROUP:0xe454 is the type-0 entry of the
per-object update-method table that the c0e5 walk dispatches through (`call [bx-0x1bac]`, so type t is
at DGROUP:(t*2-0x1bac) and type 0 is at 0xe454).  Its first eight words are fixed for this build, and the offset is checkable without running anything:
the signature sits at file offset 0x2a454 in re_out/fist_dat_image.bin, the image loads at linear 0 and
DGROUP is at linear 0x1c000, so 0x2a454 - 0x1c000 = 0xe454.

Then it prints the three counters FUN_1000_a5dc decides on, so an original run can be compared against
the port's:

    DGROUP:0x978e  live   -- 0xdfbc entries with byte[si+0x17]&8; ZERO -> VICTORY (0x1a6ae)
    DGROUP:0x9790  peak   -- its high-water mark, gates the victory test at 0x1a68d
    DGROUP:0x6d38  ownA   -- own side live count; ZERO -> DEFEAT (0x1a6bb)
    DGROUP:0x6d39  ownB   -- the other side's live count (the pair store at 0x16085)
    DGROUP:0x6d3a/b       -- their peaks
    DGROUP:0x6da6/7/8     -- the mission clock MM:SS:sub
    DGROUP:0xe294/6       -- the small-object and vehicle rosters (a294 / a296)
    DGROUP:0xe814  over   -- the mission-over flag; nonzero means resolved
    DGROUP:0x6da0  code   -- the outcome code written just before it

    python3 tools/oracle/read_counters.py /tmp/ocensus.ram.bin
"""
import sys, struct

SIG = struct.pack('<8H', 0x7c1d, 0x87df, 0x902c, 0x97d5, 0xbab4, 0x9e2b, 0x9e2b, 0xb5e7)
SIG_AT_DGROUP = 0xe454          # type-0 entry of the c0e5 update-method table

FIELDS = [
    ('live  0x978e', 0x978e, 2), ('peak  0x9790', 0x9790, 2), ('prev  0x9792', 0x9792, 2),
    ('ownA  0x6d38', 0x6d38, 1), ('ownB  0x6d39', 0x6d39, 1),
    ('pkA   0x6d3a', 0x6d3a, 1), ('pkB   0x6d3b', 0x6d3b, 1),
    ('clkMM 0x6da6', 0x6da6, 1), ('clkSS 0x6da7', 0x6da7, 1), ('clkSub 0x6da8', 0x6da8, 1),
    ('code  0x6da0', 0x6da0, 2), ('timer 0x6da2', 0x6da2, 2),
    ('a294  0xe294', 0xe294, 2), ('a296  0xe296', 0xe296, 2),
    ('over  0xe814', 0xe814, 1),
]

def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    ram = open(sys.argv[1], 'rb').read()
    hits = []
    i = 0
    while True:
        i = ram.find(SIG, i)
        if i < 0:
            break
        hits.append(i)
        i += 1
    if not hits:
        sys.exit('signature not found -- is this a mission-time dump?  (%d bytes)' % len(ram))
    if len(hits) > 1:
        print('note: signature found %d times at %s -- using the first'
              % (len(hits), ' '.join(hex(h) for h in hits)))
    dg = hits[0] - SIG_AT_DGROUP
    print('DGROUP guest base = 0x%x   (port linear 0x1c000, load offset +0x%x)' % (dg, dg - 0x1c000))
    for name, off, width in FIELDS:
        raw = ram[dg + off: dg + off + width]
        val = raw[0] if width == 1 else struct.unpack('<H', raw)[0]
        print('  %-14s = 0x%0*x  (%d)' % (name, width * 2, val, val))

if __name__ == '__main__':
    main()
