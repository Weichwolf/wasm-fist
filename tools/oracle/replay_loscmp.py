#!/usr/bin/env python3
# board:0017 -- replay_loscmp.py <oracle regs.txt> <port FIST_RNGTRACE>: the line-of-sight service (op 0x58)
# call by call -- the candidate-object deltas at the handler's entry (the oracle's regtrace at 0x1000805a:
# EBX = dx, EDX = dy, EAX = dz) and its verdict at the return (0x10001108: EAX), against the shim's `los`
# / `losr` lines -- per sim step; the first step whose sequence differs.
import re, sys
orc, prt = sys.argv[1], sys.argv[2]
osteps = [[]]; seen_m = False; pend = None
for l in open(orc):
    m = re.search(r'lin=([0-9a-f]{8}) eax=([0-9a-f]{8}) ebx=([0-9a-f]{8}) ecx=[0-9a-f]{8} edx=([0-9a-f]{8})', l)
    if not m: continue
    lin = m.group(1)
    if lin == '000158e4': seen_m = True; osteps = [[]]; continue
    if not seen_m: continue
    if lin == '0001d275': osteps.append([])
    elif lin == '1000805a': pend = (int(m.group(3), 16), int(m.group(4), 16), int(m.group(2), 16))
    elif lin == '10001108' and pend is not None: osteps[-1].append(pend + (int(m.group(2), 16),)); pend = None
psteps = [[]]; started = False; pend = None
for l in open(prt):
    if l.startswith('mission'): started = True; psteps = [[]]; continue
    if not started: continue
    if l.startswith('sim'): psteps.append([])
    elif l.startswith('los '):
        f = l.split(); pend = (int(f[1], 16), int(f[2], 16), int(f[3], 16))
    elif l.startswith('losr') and pend is not None: psteps[-1].append(pend + (int(l.split()[1], 16),)); pend = None
n = min(len(osteps), len(psteps)); first = None
for i in range(n):
    if osteps[i] != psteps[i]: first = i; break
tot = sum(len(s) for s in osteps[:n])
print('oracle steps', len(osteps), 'port steps', len(psteps), 'calls compared', tot, 'first differing step', first)
if first is not None:
    o, p = osteps[first], psteps[first]
    for k in range(max(len(o), len(p))):
        a = o[k] if k < len(o) else None; b = p[k] if k < len(p) else None
        if a != b:
            print(' call', k, 'oracle', None if a is None else ['%08x' % v for v in a], '| port', None if b is None else ['%08x' % v for v in b]); break
sys.exit(0 if first is None else 1)
