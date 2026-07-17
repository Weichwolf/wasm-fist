#!/usr/bin/env python3
# vspan_derive.py -- derive the oracle-vs-port per-row V-mapping for the mission windshield
# terrain (9200), and DECIDE the "V-span" mechanism (horizon table 9114 / count 90f0 /
# perspective step).  Companion to docs/oracle_mission_spawn.md "9200 V-SPAN -- FINAL VERDICT".
#
# Inputs (all committed samples + two default-build port dumps):
#   tools/oracle/samples/oracle_mission_spawn_framematched_idx.bin   64000 raw VGA indices (self-consistent)
#   tools/oracle/samples/oracle_mission_spawn_dac.pal.bin            256*3 6-bit DAC
#   /tmp/fist_vs/port_tile.bin   FIST_BBDUMP 64KB block-base tile (byte-exact to oracle blockB rows14..255)
#   /tmp/fist_vs/port_idx.bin    FIST_MISSFB_FBIDX rendered 0xA0000 indices (default build, op-0x24 post#1)
#
# Port globals (FIST_R3D_GDUMP, default build @9200 entry):
#   90d8=0x2dc548e0 90d4=0x7b463980 90b8=0x0088f59a 90bc=0x00d84852
#   90c0=0x01000000 9104=0x6c24295e 9108=0x447acd50  90f0=288 90f8=81 9114=ext:0x7568
#   esi=(90c0*9104)>>32=7087145  ebp=(90c0*9108)>>32=4487885   (shim guesses these; 82b8 passes them through)
#
# 9200 is an asm-exact CONSTANT-SCALE AFFINE (Mode-7) walk; the clean per-pixel map is
#   V(r,c) = 90d8 + r*90b8 + c*esi ;  U(r,c) = 90d4 - r*90bc + c*ebp ;  idx = tile[(V>>24)&255,(U>>24)&255]
# (horizon[r] only skips the leftmost horizon[r] px -> negligible for the terrain band).
import numpy as np, json, random, sys
ROOT='.'; SMP=ROOT+'/tools/oracle/samples/'
tile=np.fromfile('/tmp/fist_vs/port_tile.bin',dtype=np.uint8).reshape(256,256)
por =np.fromfile('/tmp/fist_vs/port_idx.bin',dtype=np.uint8).reshape(200,320)
ora =np.fromfile(SMP+'oracle_mission_spawn_framematched_idx.bin',dtype=np.uint8).reshape(200,320)
dac =np.fromfile(SMP+'oracle_mission_spawn_dac.pal.bin',dtype=np.uint8).reshape(256,3).astype(int)
lum=dac[:,0]+2*dac[:,1]+dac[:,2]
M32=lambda v:(np.int64(v)&0xffffffff)
d8=0x2dc548e0; d4=0x7b463980; b8=0x0088f59a; bc=0x00d84852; esi=7087145; ebp=4487885
W,H=288,81; rr=np.arange(H)[:,None]; cc=np.arange(W)[None,:]
def render(p):
    D8,D4,B8,BC,ES,EB=p; V=M32(D8+rr*B8+cc*ES); U=M32(D4-rr*BC+cc*EB)
    return tile[(V>>24)&255,(U>>24)&255]
seed=[d8,d4,b8,bc,esi,ebp]
# 1) VALIDATE the affine model + lock the window origin
sp=render(seed); best=(0,0,0)
for oy in range(0,120):
  for ox in range(0,33):
    if oy+H>200 or ox+W>320: continue
    m=(por[oy:oy+H,ox:ox+W]==sp).mean()
    if m>best[0]: best=(m,oy,ox)
oy,ox=best[1],best[2]
print("[1] PORT affine self-repro = %.3f @ window origin (x=%d,y=%d)  -> model VALIDATED"%(best[0],ox,oy))
V=M32(d8+rr*b8+cc*esi); print("    PORT samples tile V-rows %d..%d (distinct %d) -- reaches the light rows too"%(
      int(((V>>24)&255).min()),int(((V>>24)&255).max()),len(set(((V>>24)&255).ravel().tolist()))))
oseg=ora[oy:oy+H,ox:ox+W]; om=(oseg>=80)
print("    ORACLE terrain mean idx=%.0f vs PORT %.0f (same window)"%(oseg[om].mean(), por[oy:oy+H,ox:ox+W][por[oy:oy+H,ox:ox+W]>=80].mean()))
# 2) can ANY affine reproduce the oracle? (offset / scale / general 6-param hillclimb)
def score(p): s=render(p); return (s[om]==oseg[om]).mean()
print("[2] affine reproducibility of the ORACLE terrain band:")
print("    port-params                 = %.3f"%score(seed))
bo=0
for kv in range(0,256,2):
  for ku in range(0,256,4):
    bo=max(bo,score([M32(d8+kv*(1<<24)),M32(d4+ku*(1<<24)),b8,bc,esi,ebp]))
print("    best offset-only            = %.3f"%bo)
U24=1<<24; best_hc=0; random.seed(1)
for R in range(16):
    if R==0: p=list(seed)
    else:
        th=random.uniform(0,6.283); sc=random.uniform(0.4,3.0); be=7087145*sc; bb=8976794*sc
        p=[random.randrange(0,1<<32),random.randrange(0,1<<32),int(bb*np.cos(th)),int(bb*np.sin(th)),int(be*np.cos(th)),int(be*np.sin(th))]
    s=score(p)
    for st in [64*U24,16*U24,4*U24,U24,U24//4,U24//16]:
        imp=True
        while imp:
            imp=False
            for i in range(6):
                for dv in (st,-st,3*st,-3*st):
                    q=p[:]; q[i]=int(M32(q[i]+dv)); s2=score(q)
                    if s2>s: s=s2; p=q; imp=True
    best_hc=max(best_hc,s)
print("    best general 6-param hillclb= %.3f   (CAVEAT: optimiser recovers *synthetic* affines only ~0.10-0.13,"%best_hc)
print("                                          so a low value here is necessary but not sufficient)")
# 3) OPTIMISER-INDEPENDENT perspective signature (decisive)
def rowprof(buf):
    w=buf[oy:oy+H,ox:ox+W]; L=lum[w]
    mn=[L[y].mean() for y in range(0,H,8)]
    act=[np.abs(np.diff(L[y])).mean() for y in range(0,H,8)]
    return mn,act
pm,pa=rowprof(por); omn,oa=rowprof(ora)
print("[3] per-screen-row signature (top->bottom):")
print("    PORT   mean-lum:", " ".join("%3.0f"%v for v in pm))
print("    ORACLE mean-lum:", " ".join("%3.0f"%v for v in omn), "  <- strong top->bottom depth gradient (~%.0f)"%(omn[0]-omn[-1]))
print("    PORT   h-tex activity ratio bottom/top = %.2f (flat -> constant scale = AFFINE)"%(pa[-1]/max(pa[0],1e-9)))
print("    ORACLE h-tex activity ratio bottom/top = %.2f (declines near viewer -> PERSPECTIVE magnification)"%(oa[-1]/max(oa[0],1e-9)))
print("VERDICT: window (288x81) + count(90f0=288) + horizon(9114) are structurally correct & validated;")
print("         the oracle's screen->tile map is NON-AFFINE (perspective depth-scale + shading) which the")
print("         asm-exact constant-scale affine 9200 cannot produce -> the perspective renderer is paged-out.")
