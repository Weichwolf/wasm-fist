#!/usr/bin/env python3
# board:0017 -- replay_cmp.py <oracle regs.txt> <port FIST_RNGTRACE> <prefix>_sched.txt [port binary]:
# the two draw streams per sim step; the first differing step, with each draw's caller on both sides
# (the oracle's from the regtrace's ret=[sp]:[sp+2] near-return offsets resolved through fist.c's
# function headers; the port's from FIST_RNGTRACE's from= addresses through addr2line when the binary
# is given).  The port's scheduled polls (the sched count) are stripped from each step's tail.
import re, sys, subprocess, bisect, os
orc, prt, schedf = sys.argv[1], sys.argv[2], sys.argv[3]
binp = sys.argv[4] if len(sys.argv) > 4 else None
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
funcs = []
for l in open(os.path.join(ROOT, 'build', 'fist.c')):
    m = re.search(r'@ ([0-9a-f]{4}):([0-9a-f]{4}) =====', l)
    if m: funcs.append(((int(m.group(1), 16) << 4) + int(m.group(2), 16), '%s_%s' % (m.group(1), m.group(2))))
funcs.sort(); fstarts = [f[0] for f in funcs]
def encl(lin):
    i = bisect.bisect_right(fstarts, lin) - 1
    return funcs[i][1] if i >= 0 else '?'
def oname(ret):
    a, b = ret.split(':')
    return encl(int(a, 16)) + '<' + encl(int(b, 16))
KIND = {'0001d275': 'S', '00024ab0': 'P', '000158e4': 'M', '00011447': 'R', '00011454': 'R'}
ev = []
for l in open(orc):
    m = re.search(r'lin=(000[0-9a-f]{5}) eax=([0-9a-f]{8}).*ret=([0-9a-f]{4}:[0-9a-f]{4})', l)
    if m and m.group(1) in KIND: ev.append((KIND[m.group(1)], int(m.group(2), 16) & 0xffff, m.group(3)))
m0 = [i for i, e in enumerate(ev) if e[0] == 'M'][0]
osteps, after_p = [[]], False
for k, v, r in ev[m0 + 1:]:
    if k == 'S': osteps.append([]); after_p = False
    elif k == 'P': after_p = True
    elif k == 'R':
        if after_p: after_p = False
        else: osteps[-1].append((v, oname(r)))
sched = [int(x) for x in open(schedf).read().split()]
pev, started, addrs = [], False, set()
for l in open(prt):
    if l.startswith('seed'): started = True; continue
    if not started or not (l.startswith('rng') or l.startswith('sim')): continue
    m = re.match(r'(rng|sim) ?([0-9a-f]{4})? ?t=(\d+)(?: from=(0x[0-9a-f]+)(?:<(0x[0-9a-f]+))?)?', l)
    if m.group(1) == 'sim': pev.append(('S', 0, int(m.group(3)), None))
    else: pev.append(('R', int(m.group(2), 16), int(m.group(3)), (m.group(4), m.group(5)))); addrs.update((m.group(4), m.group(5)))
names = {}
al = sorted(a for a in addrs if a)
if binp and al:
    out = subprocess.run(['addr2line', '-f', '-e', binp] + al, capture_output=True, text=True).stdout.split('\n')
    for i, a in enumerate(al): names[a] = out[2 * i] if 2 * i < len(out) else '?'
segs, ticks = [[]], ['init']
for e in pev:
    if e[0] == 'S': segs.append([]); ticks.append(e[2])
    else: segs[-1].append((e[1], names.get(e[3][0], '?') + '<' + names.get(e[3][1], '?')))
psteps = []
for i, sg in enumerate(segs):
    n = sched[i] if i < len(sched) else 0
    psteps.append(sg[:len(sg) - n] if n else sg)
n = min(len(osteps), len(psteps)); first = None
for i in range(n):
    if [v for v, _ in osteps[i]] != [v for v, _ in psteps[i]]: first = i; break
print('oracle steps', len(osteps), 'port steps', len(psteps), 'first differing step', first,
      'port tick', ticks[first] if first is not None else None)
if first is not None:
    for i in range(max(0, first - 1), min(n, first + 2)):
        print(' step', i, 't=%s' % ticks[i])
        print('   oracle:', ' '.join('%04x(%s)' % (v, f) for v, f in osteps[i]))
        print('   port:  ', ' '.join('%04x(%s)' % (v, f) for v, f in psteps[i]))
sys.exit(0 if first is None else 1)
