#!/usr/bin/env python3
# =============================================================================
# sim_voxel6980.py -- offline reconstruction of FUN_0000_6980 (ext 0x6980), the
# NovaLogic Voxel-Space column raycaster that fills tile ROWS 0..159 (the terrain)
# on top of the 689a sky-resample (sim_lighttile_689a.py fills rows 160..255).
#
# Inputs are the CR3-RESOLVED render-time buffers of an AZER1 mission frame (the
# extender's 32-bit PM paging maps ext-flat offsets to guest-phys via the page
# table at phys 0x131000 -- identity for the low code pages, remapped for the
# runtime buffers; e.g. tile ext-flat 0x44200 -> phys 0xb78200, NOT the identity
# 0x175200 that burned 14 prior iterations).  Banked compact bundle:
#   tools/oracle/samples/voxel6980_inputs_lt2pass00.bin.gz
# (built from scratch/oracle/lt2.pass00.ram.bin by the CR3 walk; map-static + one
#  frame's globals + the CR3 render-time tile).
#
# VERDICT (see docs/oracle_voxel6980_repro.md): the 6980 GEOMETRY is reproduced
# byte-for-byte in mechanism (rotation recompute is byte-exact; the fill silhouette
# lands exactly on the real terrain region -- where 6980 writes, the real tile is
# LIGHT mean~165-190, 93-97% idx>=140), but the COLOR is WRONG: the colormap 6980
# indexes at [0x85bc+0x100000] is DARK (max 104) in every oracle dump and cannot
# produce the LIGHT terrain -- the documented "85b8 reduce-colormap collapse".  So
# adding 6980 with the available colormap does NOT push whole-tile past 55.55%; it
# REGRESSES (0% color match where it writes).  The colormap is the crux, unresolved.
# =============================================================================
import struct,sys,gzip
M=0xffffffff
def s32(x): x&=M; return x-0x100000000 if x>=0x80000000 else x
def hi(a,b): return (s32(a)*s32(b))>>32   # (int32 a * int32 b) >> 32 == imul; edx

ROOT=sys.argv[1] if len(sys.argv)>1 else '.'
blob=gzip.decompress(open(ROOT+'/tools/oracle/samples/voxel6980_inputs_lt2pass00.bin.gz','rb').read())
assert blob[:4]==b'V698'
mapoff,elen,plen,mlen=struct.unpack('<IIII',blob[4:20])
o=20
ext=blob[o:o+elen]; o+=elen
proj=blob[o:o+plen]; o+=plen
MAP=blob[o:o+mlen]; o+=mlen
tile_o=blob[o:o+0x10000]
def e(off): return struct.unpack('<I',ext[off:off+4])[0]
HM=MAP[:0x100000]; CM=MAP[0x100000:0x200000]

c90c0=e(0x90c0);c90d4=e(0x90d4);c90d8=e(0x90d8);c90dc=e(0x90dc)
n90fc=s32(e(0x90fc));n9100=s32(e(0x9100))
D3a20=e(0x3a20);D38f1=e(0x38f1)&0xffff;D38fd=e(0x38fd);D3901=e(0x3901);D3905=e(0x3905)
D4224=[e(0x4224+4*i) for i in range(D3a20)];D4624=[e(0x4624+4*i) for i in range(D3a20)]
L0=(-(c90dc>>0x19))&0xff

tile=bytearray(0x10000); mask=bytearray(0x10000)
a4a60=[k*0x100 for k in range(D38f1)]; a4e60=[0]*D38f1
def samp(ebx,ebp,d):
    coord=(((ebp>>22)&M)<<10 | (ebx>>22)) & 0xfffff     # shld ecx,ebp,10 ; shld ecx,ebx,10
    return proj[d*0x100 + ((L0+HM[coord])&0xff)], coord  # SMC: projtbl[d][ (L0+height)&0xff ]
for d in range(D3a20):
    ebx=c90d4; ebp=c90d8
    t=D4624[d]>>1
    ebp=(ebp + hi(t,n9100)*-2)&M                # ray V start
    ebx=(ebx + hi(t,n90fc)*-2)&M                # ray U start
    se=hi(D4224[d],n9100)&M                     # per-column U step (SMC 6ad2)
    sp=hi(D4224[d],n90fc)&M                     # per-column V step (SMC 6ad8)
    edx=D38fd; ux=ebx; up=ebp; first=True       # loop 1: center 38fd .. up to 3905
    while True:
        if not first: ux=(ux+se)&M; up=(up-sp)&M
        first=False
        pv,co=samp(ux,up,d); run=a4e60[edx]
        if run<pv:
            a4e60[edx]=pv; col=CM[co]; wp=a4a60[edx]
            tile[wp:wp+(pv-run)]=bytes([col])*(pv-run)
            for i in range(wp,wp+(pv-run)): mask[i]=1
            a4a60[edx]=wp+(pv-run)
            if edx+1==D3905: break
        else:
            if edx+1==D3905: break
        edx+=1
    edx=D38fd; ux=ebx; up=ebp                    # loop 2: center 38fd .. down to 3901
    while True:
        ux=(ux-se)&M; up=(up+sp)&M
        pv,co=samp(ux,up,d); ee=edx-1; run=a4e60[ee]
        if pv>run:
            a4e60[ee]=pv; col=CM[co]; wp=a4a60[ee]
            tile[wp:wp+(pv-run)]=bytes([col])*(pv-run)
            for i in range(wp,wp+(pv-run)): mask[i]=1
            a4a60[ee]=wp+(pv-run)
        edx-=1
        if edx==D3901: break

# ---- report ----
print('6980 fill %.1f%% of tile (%d cols)  L0=%d 3a20=%d cols[%d..%d)'%(
      100*sum(mask)/65536, sum(1 for c in range(256) if any(mask[c*256:c*256+256])), L0,D3a20,D3901,D3905))
lw=[tile_o[i] for i in range(0x10000) if mask[i]]
mw=[tile[i]   for i in range(0x10000) if mask[i]]
print('GEOMETRY: where 6980 writes, the CR3 render-time tile is LIGHT: mean %.1f  frac(idx>=140) %.2f  distinct %d'%(
      sum(lw)/len(lw), sum(1 for x in lw if x>=140)/len(lw), len(set(lw))))
print('COLOR   : my 6980 colours there (from [0x85bc+0x100000]) are DARK: mean %.1f  distinct %d  -> exact-value match %.2f%%'%(
      sum(mw)/len(mw), len(set(mw)), 100*sum(1 for i in range(0x10000) if mask[i] and tile[i]==tile_o[i])/max(1,sum(mask))))
print('CM buffer [0x85bc+0x100000]: distinct %d mean %.1f MAX %d (cannot produce the light 140-252 terrain)'%(
      len(set(CM)), sum(CM)/len(CM), max(CM)))
e159=sum(tile[c*256+r]==tile_o[c*256+r] for c in range(256) for r in range(0,160))
print('rows 0-159 (6980 domain) exact-match: %.2f%% -> whole-tile stays 55.55%% (689a); 6980-overlay REGRESSES on colour'%(100*e159/(256*160)))
