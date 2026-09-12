#!/usr/bin/env python3
# board:0017 -- replay_sched.py <oracle regs.txt> <out prefix>: the oracle's LFSR seed at the battle load
# (4754, linear 0x158e4) and its three schedules for the port's replay (patch 592):
#   <prefix>_seed.txt    FIST_RNG_SEED    ptr,w84,w86,w88,w8a -- the state at the sync point, from the
#                                         last four draws before it (state word = draw + 1)
#   <prefix>_sched.txt   FIST_RNG_SCHEDULE  polls (24ab0 -> 0291) before each sim step
#   <prefix>_frames.txt  FIST_FRAME_SCHEDULE  sim steps before each render (1346d)
#   <prefix>_phases.txt  FIST_PHASE_SCHEDULE  phase dispatches (13487) per render
#   <prefix>_draws.json  the sim's draws per step (the polls' excluded), for replay_cmp.py
import re, sys, json, collections
src, out = sys.argv[1], sys.argv[2]
KIND = {'0001d275': 'S', '00024288': 'T', '00024ab0': 'P', '000158e4': 'M', '0001346d': 'D', '00013487': 'H',
        '00011447': 'R', '00011454': 'R'}
ev = []
for l in open(src):
    m = re.search(r'lin=(000[0-9a-f]{5}) eax=([0-9a-f]{8})', l)
    if m and m.group(1) in KIND:
        ev.append((KIND[m.group(1)], int(m.group(2), 16) & 0xffff))
mi = [i for i, e in enumerate(ev) if e[0] == 'M']
assert mi, 'no battle-load marker (0x158e4) in the trace'
m0 = mi[0]
before = [v for k, v in ev[:m0] if k == 'R'][-4:]
seed = '1f84,%04x,%04x,%04x,%04x' % tuple((x + 1) & 0xffff for x in (before[3], before[0], before[1], before[2]))
sched, polls, draws, after_p = [], 0, [[]], False
for k, v in ev[m0 + 1:]:
    if k == 'S': sched.append(polls); polls = 0; draws.append([]); after_p = False
    elif k == 'P': polls += 1; after_p = True
    elif k == 'R':
        if after_p: after_p = False
        else: draws[-1].append(v)
frames, cur = [], 0
for k, v in ev[m0 + 1:]:
    if k == 'D': frames.append(cur); cur = 0
    elif k == 'S': cur += 1
phases, cur = [], None
for k, v in ev[m0 + 1:]:
    if k == 'D':
        if cur is not None: phases.append(cur)
        cur = 0
    elif k == 'H' and cur is not None: cur += 1
if cur is not None: phases.append(cur)
open(out + '_seed.txt', 'w').write(seed)
open(out + '_sched.txt', 'w').write('\n'.join(map(str, sched)) + '\n')
open(out + '_frames.txt', 'w').write('\n'.join(map(str, frames)) + '\n')
open(out + '_phases.txt', 'w').write('\n'.join(map(str, phases)) + '\n')
json.dump(draws, open(out + '_draws.json', 'w'))
print('seed', seed, 'steps', len(sched), 'renders', len(frames), 'polls before the first step', sched[0] if sched else None)
print('phases per render', sorted(collections.Counter(phases).items())[:12])
