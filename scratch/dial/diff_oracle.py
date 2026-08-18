#!/usr/bin/env python3
# Diff an oracle spawn RAM dump's player object vs the port's captured object + the oracle AZER1 reference.
# Usage: diff_oracle.py <oracle_azer6.ram.bin>
import struct,sys
def load(p): return open(p,'rb').read()
def find_dg(d):
    def u16(x): return struct.unpack_from('<H',d,x)[0]
    for B in range(0,0x400000,16):
        if u16(B+0x6d34)==u16(B+0x7ae0):
            po=u16(B+0x6d34)
            if 0x4000<=po<=0xf800 and u16(B+po)<=12: return B,po
    return None,None
d=load(sys.argv[1]); B,po=find_dg(d)
print("oracle DGROUP=0x%x playerOff=0x%x"%(B,po) if B else "NO DGROUP FOUND")
if B:
    obj=B+po
    def s16(x): return struct.unpack_from('<h',d,x)[0]
    v55=s16(obj+0x55); u=(abs(v55)>>2)&0xfffe; u=min(u,0x18)
    print("ORACLE player +0x55 = %d -> usi %d  (usi14 target = 56..63)"%(v55,u))
    print("oracle player object words:")
    for base in range(0,0x80,0x10):
        print("  +%02x:"%base, ' '.join("%6d"%s16(obj+base+i*2) for i in range(8)))
