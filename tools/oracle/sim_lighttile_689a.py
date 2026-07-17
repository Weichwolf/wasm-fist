#!/usr/bin/env python3
# Offline reconstruction of the extender per-frame PERSPECTIVE tile-resample FUN_0000_689a
# (ext 0x689a, re_out/fist_image.bin) that fills the terrain colormap TILE (ds:0x3918, ext-flat
# 0x44200) which the affine renderer FUN_0000_9200 then samples.  Source = ds:0x3911 = the 128KB
# decoded 5.SKY (KLC1 128x1024).  Reproduces the LIGHT (sky/horizon) rows 160..255 of the oracle
# frame-matched tile at ~100%; rows 0..159 are the FUN_0000_6980 voxel-column overlay (not here).
# Verified vs tools/oracle/samples/oracle_9200_framematched_pass08.cap : 55.55% whole-tile
# (== the complete 689a contribution).  Constants pinned from the render-time RAM (38ed/38f1=256, cl=10).
import struct, sys
ROOT=sys.argv[1] if len(sys.argv)>1 else '.'
cap=open(ROOT+'/tools/oracle/samples/oracle_9200_framematched_pass08.cap','rb').read()
glob=cap[20:20+0x200]
g=lambda o: struct.unpack('<I',glob[o-0x9000:o-0x9000+4])[0]
tile_o=cap[20+0x200+256:20+0x200+256+0x10000]
src=open(ROOT+'/tools/oracle/samples/oracle_lighttile_source_ds3911_static.bin','rb').read()[:0x20000]
M=0xffffffff
s32=lambda x:(x&M)-0x100000000 if (x&M)>=0x80000000 else (x&M)
mul_shrd16=lambda a,b:(((a&M)*(b&M))>>16)&M          # unsigned mul + shrd eax,edx,16
imul_shrd16=lambda a,b:((s32(a)*s32(b))&(2**64-1))>>16&M  # signed imul + shrd 16
imul_low=lambda a,b:(s32(a)*s32(b))&M
D38ed=256; D38f1=256; cl=10
c0,b0,b4,dc,e0=g(0x90c0),g(0x90b0),g(0x90b4),g(0x90dc),g(0x90e0)
i910c=(-s32(e0))&M; i9110=(dc>>3)&M
ebp0=mul_shrd16(c0,b0); esi0=imul_shrd16(ebp0,b4); ebp=(ebp0<<3)&M
i910c=(i910c-imul_low(D38ed>>1,esi0))&M
i9110=(i9110-imul_low(D38f1>>1,ebp))&M
tile=bytearray(0x10000); out=0; edx=i910c
for col in range(D38f1):
    esi_col=((edx>>22)&0x3ff)<<7            # top 10 bits of edx, <<7 = source column base
    ebx=i9110
    for _ in range(D38ed>>1):               # 128 texel-pairs
        tile[out]=src[(esi_col+((ebx>>25)&0x7f))&0x1ffff]; ebx=(ebx+ebp)&M
        tile[out+1]=src[(esi_col+((ebx>>25)&0x7f))&0x1ffff]; ebx=(ebx+ebp)&M
        out+=2
    edx=(edx+esi0)&M                         # 90bc per-column advance
eq=sum(tile[i]==tile_o[i] for i in range(0x10000))
print('689a: %d/65536 = %.2f%%  sim(dist=%d mean=%.1f) oracle(dist=%d mean=%.1f)'%(
      eq,100*eq/65536,len(set(tile)),sum(tile)/65536,len(set(tile_o)),sum(tile_o)/65536))
# per row-band (689a owns 160..255, 6980 owns 0..159)
for rb in range(0,256,32):
    e=sum(tile[c*256+r]==tile_o[c*256+r] for c in range(256) for r in range(rb,rb+32))
    print('  rows %3d-%3d: %.1f%%'%(rb,rb+31,100*e/(256*32)))
