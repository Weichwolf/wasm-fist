import struct
f=bytearray(open('armoredfist/FIST.RUN','rb').read())
DB=0x583a  # data offset 0 -> file 0x583a
def gd(d): return struct.unpack_from('<I',f,DB+d)[0]
def sd(d,v): struct.pack_into('<I',f,DB+d,v&0xffffffff)
M=0xffffffff
def ror(x): return ((x>>1)|((x&1)<<31))&M
eax=0x9324abe1; ebx=0x439a9efa
# region1: static keys, 4 units at data 0xab4
esi=0xab4
for _ in range(4):
    sd(esi, gd(esi)^eax); sd(esi+4, gd(esi+4)^ebx); esi+=8
# region2: rolling, data 0xc20.. count=(0xbf03-0xc20)>>3
esi=0xc20
ecx=(0xbf03-0xc20)>>3
while ecx>0:
    sd(esi, gd(esi)^eax); sd(esi+4, gd(esi+4)^ebx)
    ebx=ror(ebx); eax=ror(eax); eax^=ecx
    esi+=8; ecx-=1
open('scratch/dec_full.bin','wb').write(f)
# write just the decrypted app image (file 0x583a..EOF) at base data-0
open('scratch/app_dec.bin','wb').write(f[DB:])
print('decrypted. region2 iters=%d, covered data 0xc20..0x%x'%((0xbf03-0xc20)>>3, 0xc20+(((0xbf03-0xc20)>>3)*8)))
# entropy check of decrypted region2
import math
from collections import Counter
def ent(b):
    c=Counter(b); n=len(b); 
    return -sum((v/n)*math.log2(v/n) for v in c.values()) if n else 0
for base in range(0x6400,0x11400,0x1000):
    print('file %06x  H=%.2f'%(base,ent(f[base:base+0x1000])))
