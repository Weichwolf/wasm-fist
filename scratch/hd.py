import sys
f=open('armoredfist/FIST.RUN','rb').read()
off=int(sys.argv[1],0); ln=int(sys.argv[2],0) if len(sys.argv)>2 else 64
for i in range(0,ln,16):
    b=f[off+i:off+i+16]
    hexs=' '.join('%02x'%c for c in b)
    asc=''.join(chr(c) if 32<=c<127 else '.' for c in b)
    print('%08x  %-47s  %s'%(off+i,hexs,asc))
