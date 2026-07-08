import sys,re
lines=open('scratch/seg6f.asm').read().splitlines()
ranges=[(int(a,16),int(b,16)) for a,b in (x.split('-') for x in sys.argv[1:])]
rx=re.compile(r'^\s+([0-9a-f]+):')
for L in lines:
    m=rx.match(L)
    if not m: continue
    o=int(m.group(1),16)
    if any(a<=o<=b for a,b in ranges): print(L)
