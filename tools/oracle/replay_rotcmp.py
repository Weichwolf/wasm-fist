#!/usr/bin/env python3
# board:0017 -- replay_rotcmp.py <oracle regs.txt> <port FIST_RNGTRACE>: the 3-D rotation 0459 call by call
# -- the oracle's regtrace at its entry 0x115e9 (AX CX DX in) and exit 0x11657 (AX DX CX out) against the
# port's `rot` lines (patch 607) -- per sim step; the first step whose sequence differs.
# The render's a20d (the windshield present's camera vector) also calls 0459; its cadence is the paint
# loop's, not the sim's, so those calls are dropped on both sides (the oracle's by the caller's return
# offset at the exit, 0f69:ab7d..abd5 = a20d; the port's by the `from=` frame, given the binary).
import re, sys, subprocess
orc, prt = sys.argv[1], sys.argv[2]
binp = sys.argv[3] if len(sys.argv) > 3 else None
osteps = [[]]; seen_m = False; pend = None
for l in open(orc):
    m = re.search(r'lin=([0-9a-f]{8}) eax=([0-9a-f]{8}) ebx=[0-9a-f]{8} ecx=([0-9a-f]{8}) edx=([0-9a-f]{8}).*ret=([0-9a-f]{4}):([0-9a-f]{4})', l)
    if not m: continue
    lin = m.group(1); ax = int(m.group(2), 16) & 0xffff; cx = int(m.group(3), 16) & 0xffff; dx = int(m.group(4), 16) & 0xffff
    if lin == '000158e4': seen_m = True; osteps = [[]]; continue
    if not seen_m: continue
    if lin == '0001d275': osteps.append([])
    elif lin == '000115e9': pend = (ax, cx, dx)
    elif lin == '00011657' and pend is not None:
        outer = int(m.group(6), 16)
        if not (0xab7d <= outer <= 0xabd5): osteps[-1].append(pend + (ax, dx, cx))
        pend = None
psteps = [[]]; started = False; raw = []; addrs = set()
for l in open(prt):
    if l.startswith('mission'): started = True; psteps = [[]]; continue
    if not started: continue
    if l.startswith('sim'): psteps.append([])
    elif l.startswith('rot '):
        f = l.split(); fr = f[8].split('=')[1].split('<') if len(f) > 8 else ['', '']
        addrs.update(fr); psteps[-1].append((tuple(int(x, 16) for x in (f[1], f[2], f[3], f[5], f[6], f[7])), fr[-1]))
names = {}
al = sorted(a for a in addrs if a)
if binp and al:
    out = subprocess.run(['addr2line', '-f', '-e', binp] + al, capture_output=True, text=True).stdout.split('\n')
    for i, a in enumerate(al): names[a] = out[2 * i] if 2 * i < len(out) else '?'
psteps = [[t for t, fr in sg if names.get(fr, '') != 'FUN_1000_a20d'] for sg in psteps]
n = min(len(osteps), len(psteps)); first = None
for i in range(n):
    if osteps[i] != psteps[i]: first = i; break
print('oracle steps', len(osteps), 'port steps', len(psteps), 'calls compared', sum(len(s) for s in osteps[:n]), 'first differing step', first)
if first is not None:
    o, p = osteps[first], psteps[first]
    for k in range(max(len(o), len(p))):
        a = o[k] if k < len(o) else None; b = p[k] if k < len(p) else None
        if a != b:
            print(' call', k, 'oracle', None if a is None else ' '.join('%04x' % v for v in a), '| port', None if b is None else ' '.join('%04x' % v for v in b)); break
sys.exit(0 if first is None else 1)
