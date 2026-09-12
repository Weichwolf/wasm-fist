#!/usr/bin/env python3
# board:0017 -- replay_drawcmp.py <oracle regs.txt> <port FIST_RNGTRACE>: the shells DRAWN per sim step
# on both sides (the oracle's regtrace at 0xc659 = linear 0x1d7e9 with ESI, the port's `draw` lines from
# patch 600's hook in c64a), in order; the first step whose list differs.  c64a flips a shell's
# heading once per frame it is drawn, so the draw lists decide the guided shells' seeker parity.
import re, sys
orc, prt = sys.argv[1], sys.argv[2]
osteps = [[]]; seen_m = False
for l in open(orc):
    m = re.search(r'lin=(000[0-9a-f]{5}) .*esi=([0-9a-f]{8})', l)
    if not m: continue
    lin = m.group(1)
    if lin == '000158e4': seen_m = True; osteps = [[]]; continue
    if not seen_m: continue
    if lin == '0001d275': osteps.append([])
    elif lin == '0001d7e9': osteps[-1].append(int(m.group(2), 16) & 0xffff)
psteps = [[]]; started = False
for l in open(prt):
    if l.startswith('mission'): started = True; psteps = [[]]; continue
    if not started: continue
    if l.startswith('sim'): psteps.append([])
    elif l.startswith('draw'): psteps[-1].append(int(l[5:9], 16))
n = min(len(osteps), len(psteps)); first = None
for i in range(n):
    if osteps[i] != psteps[i]: first = i; break
print('oracle steps', len(osteps), 'port steps', len(psteps), 'first differing step', first)
if first is not None:
    for i in range(max(0, first - 2), min(n, first + 3)):
        print(' step', i, 'oracle', ['%04x' % v for v in osteps[i]], '| port', ['%04x' % v for v in psteps[i]])
sys.exit(0 if first is None else 1)
