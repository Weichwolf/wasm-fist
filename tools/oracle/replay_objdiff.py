#!/usr/bin/env python3
# board:0017 -- replay_objdiff.py <oracle dir> <port dump prefix> [max hits]: the object tables of the
# two DGROUP windows at every sim step -- the oracle's <dir>/dg.bin.<n> (replay_capture.sh) against the
# port's <prefix>.<n> (FIST_STEP0_DUMP=<prefix> FIST_STEPEVERY_DUMP=1 FIST_STEP_DUMPLEN=10000).  The
# display table 0xdfbc (182 slots) is walked; a slot's object (0xfb bytes for the vehicle types 0..3,
# 0x37 otherwise) is compared byte for byte, the render bookkeeping "3" bytes of the vehicles excluded.
import struct, os, sys
odir, pfx = sys.argv[1], sys.argv[2]
maxshow = int(sys.argv[3]) if len(sys.argv) > 3 else 8
MASK_VEH = set(range(0xc4, 0xd2)) | {0xdb, 0xe4, 0xec, 0xf7, 0xf8}
def diff(o, p):
    ou = lambda x: struct.unpack_from('<H', o, x)[0]; pu = lambda x: struct.unpack_from('<H', p, x)[0]
    out = []
    for i in range(182):
        a = ou(0xdfbc + i * 4); b = pu(0xdfbc + i * 4)
        if a != b: out.append('slot%d:o%04x/p%04x' % (i, a, b)); continue
        if a == 0: continue
        t = pu(a); size = 0xfb if t <= 3 else 0x37
        d = [k for k in range(size) if o[a + k] != p[a + k] and not (t <= 3 and k in MASK_VEH)]
        if d: out.append('obj %3d off=%04x t%02x: %s' % (i, a, t, ' '.join('+%02x(o%02x/p%02x)' % (k, o[a + k], p[a + k]) for k in d[:16])))
    return out
shown = 0; scanned = 0; last = 0
steps = sorted(int(f[7:]) for f in os.listdir(odir) if f.startswith('dg.bin.') and f[7:].isdigit())
for h in steps:                     # the oracle's dumps may be every k-th step (OC_DUMPEVERY)
    of = os.path.join(odir, 'dg.bin.%d' % h); pf = '%s.%d' % (pfx, h)
    if not os.path.exists(pf): break
    scanned += 1; last = h
    d = diff(open(of, 'rb').read(), open(pf, 'rb').read())
    if d:
        print('step', h, ':', ' | '.join(d[:6])); shown += 1
        if shown >= maxshow: break
print('scanned', scanned, 'dumps to step', last, ',', shown, 'differing')
sys.exit(1 if shown else 0)
