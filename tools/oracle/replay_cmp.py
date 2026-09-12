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
IMG = open(os.path.join(ROOT, 're_out', 'fist_dat_image.bin'), 'rb').read()
def site(off, inner):
    """the cluster (0, 0f69, 1000) whose code has a call to `inner` ending at `off` -- a far return offset
    names a place in the caller's own segment, and the three clusters overlap."""
    for base in (0, 0xf690, 0x10000):
        lin = base + off
        if lin < 5 or lin > len(IMG): continue
        b5 = IMG[lin - 5:lin]; b3 = IMG[lin - 3:lin]
        if b5[0] == 0x9a and int.from_bytes(b5[1:3], 'little') == (inner & 0xffff): return lin      # lcall seg:inner
        if b3[0] == 0xe8 and (lin + int.from_bytes(b3[1:3], 'little', signed=True)) & 0xffff == (inner & 0xffff) and base == 0: return lin  # near call
    return off
def oname(ret):
    a, b = ret.split(':')
    inner_lin = int(a, 16)                       # [sp]: the near return into 0291's caller (seg 0)
    i = bisect.bisect_right(fstarts, inner_lin) - 1
    inner = funcs[i][0] if i >= 0 else 0
    return encl(inner_lin) + '<' + encl(site(int(b, 16), inner))
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
