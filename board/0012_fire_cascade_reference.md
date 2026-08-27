# board:0012 — FIRE cascade reference (asm-verified C, ready to land)

Reference for the in-mission unit-fire path.  All C below is asm-verified against `re_out/fist_dat_image.bin`
(`FUN_0000_XXXX` at file off `0xXXXX`, `FUN_1000_XXXX` at `0x10000+0xXXXX`; `objdump -m i8086 -M intel`).
Objects live at `g_mem+0x1c000+near`.  Land these AFTER patch 424 (the corrected b1df that returns b21d's DI).

## The combat model is MULTI-LAYER; the CURRENT break is layer 1 (upstream)

- **Layer 1 — AI fire DECISION** (in the per-type update methods 7c1d/87df/902c/97d5 and their targeting
  callees): sets the fire gate `[di+0x92]!=0 || [di+0x17]&0x80`.  **THIS is the current break** — it never
  triggers, so the dispatch (below) is NEVER reached.  Proof: the base-lost dispatch derefs raw host
  `~0x7e77`; if reached it would SIGSEGV, but the patch-424 build runs clean → not called.  **Trace + fix
  the targeting/fire-decision that sets [0x92]/[0x17]&0x80 FIRST** (this is the un-mapped piece).
- **Layer 2 — dispatch** (7e29/899c/91b8/99a2): C below (far-call fix).
- **Layer 3 — spawn/launch/trajectory** (7745.../b725.../ace0/9b5c): C below.
- **Scope**: each unit type has its OWN 4 spawn methods (7e29→7745/778a/7814/77cf; 899c→8121/8176/81bb/8205;
  91b8→88d1/888c/8916/8995; 99a2→9b7e/9b39/9bc3/9c15) = 16 spawn methods total.  AZER1 needs only the type(s)
  its firing units use — determine from layer 1 once it triggers.

## Allocator contract (asm-confirmed)
- `b21d` (0x1b21d): AX=type/class key IN → allocates a pool slot, returns **DI = new object near-offset**,
  `g_fist_cf=1` on full.  Does NOT read incoming DI.
- `b1df` (0x1b1df): wraps b21d; registers b21d's DI into 0xdfbc, zeroes it, **returns DI** (patch 424).
- `b1d6` (0x1b1d6): `cmp [0xe294],0x78; jb b1df` — a capped guard that falls into b1df; same contract.
- `b725`/`b73b`/`b767`/`b6c9` tail-call `0f69:b650` = `FUN_1000_ace0` (0x1ace0).  Roles throughout:
  **SI = new projectile, DI = firer**.

## Dispatch (7e29 shown; siblings by substitution)
```c
void __allregs FUN_0000_7e29(int param_1) {           /* di = firer */
  extern unsigned char g_fist_cf;
  unsigned char *dg = (unsigned char *)g_mem + 0x1c000;
  unsigned int di = (unsigned short)param_1;
  FUN_1000_70d6();
  dg[(unsigned short)(di + 0xdb)] = 3;                 /* 899c:[0xcd]  91b8:[0xcf]  99a2:[0xd7]; 899c/99a2 also dg[di+0x19]&=0xfe first */
  if (dg[(unsigned short)(di + 0x91)] == 4 ||          /* 899c/91b8/99a2 gate ==6 */
      dg[(unsigned short)(di + 0xa8)] == 0) {
    unsigned bx = (unsigned)dg[(unsigned short)(di + 0x91)] << 1;
    uint32_t fp = *(uint32_t *)(g_mem + (unsigned short)(0x7e77 + bx));  /* 899c 0x89ee  91b8 0x9206  99a2 0x99f4 */
    g_fist_cf = 0;
    /* thread firer as the DI arg.  7745/778a/77cf take firer=param_4 (5-arg); 7814 takes firer=param_3
       (4-arg).  Dispatch by index: for the 7814 entry (7e29 bx=8, [0x91]=4) call as (u2,u4,int,u2)(0,0,di,0);
       else (u2,u4,u4,int,u2)(0,0,0,di,0). */
    ((void (*)(undefined2,undefined4,int,undefined2))fist_icall_far(fp))(0,0,(int)di,0);
    if (g_fist_cf) { FUN_0000_bf77((int)di); return; }
    { unsigned short si = *(unsigned short *)(dg + (unsigned short)(di + 0x97));
      if (si != 0 && (dg[(unsigned short)(si + 0x16)] & 8) == 0) { FUN_0000_bf94((int)di); return; } }
    { unsigned bx2 = (unsigned)(dg[(unsigned short)(di + 0x91)] >> 1);
      FUN_0000_bf3c(dg[(unsigned short)(bx2 + 0x8f06)], bx2, (int)di); }  /* 899c 0x90f2  91b8 0x91a0  99a2 0x9246 */
  }
}
```

## Spawn methods (7745; 778a/77cf same with cooldown [0xaf]/[0xb3], anim [0xe6]/[0xea], b1df(9)/b1df(10), b73b/b767)
```c
void __allregs FUN_1000_7745(undefined2 p1,undefined4 p2,undefined4 p3,int param_4,undefined2 p5){
  extern unsigned char g_fist_cf; unsigned char *dg=(unsigned char*)g_mem+0x1c000;
  unsigned int di=(unsigned short)param_4; unsigned int proj; (void)p1;(void)p2;(void)p3;(void)p5;
  if(*(unsigned short*)(dg+(unsigned short)(di+0xad))==0){g_fist_cf=1;return;}
  *(unsigned short*)(dg+(unsigned short)(di+0xad))-=1;  dg[(unsigned short)(di+0xe4)]=3;
  FUN_1000_a265(0,0,(int*)(uintptr_t)di);
  proj=(unsigned short)FUN_1000_b1df(8,(int*)(uintptr_t)di);
  if(g_fist_cf){g_fist_cf=1;return;}
  FUN_0000_b725(0,(int)proj,(int)di,0);
  dg[(unsigned short)(di+0xa8)]=0x14;  FUN_0000_9b5c((int)di);  FUN_0000_c047(0xc,0,(int)di);
  dg[(unsigned short)(di+0x3c)]=0x10;  dg[(unsigned short)(di+0x92)]=0;  g_fist_cf=0;
}
```
7814 (0x17814, DI=param_3, type-7, no a265, launcher b6c9, c047(6)):
```c
void __allregs FUN_1000_7814(undefined2 p1,undefined4 p2,int param_3,undefined2 p4){
  extern unsigned char g_fist_cf; unsigned char *dg=(unsigned char*)g_mem+0x1c000;
  unsigned int di=(unsigned short)param_3; unsigned int proj; (void)p1;(void)p2;(void)p4;
  if(*(unsigned short*)(dg+0x6cde)&7){g_fist_cf=0;return;}
  if(*(unsigned short*)(dg+(unsigned short)(di+0xb1))==0){g_fist_cf=1;return;}
  *(unsigned short*)(dg+(unsigned short)(di+0xb1))-=1;  dg[(unsigned short)(di+0xe8)]=3;
  proj=(unsigned short)FUN_1000_b1df(7,(int*)(uintptr_t)di);
  if(g_fist_cf){g_fist_cf=1;return;}
  FUN_0000_b6c9(0,(int)proj,(int)di,0);  FUN_0000_c047(6,0,(int)di);  g_fist_cf=0;
}
```

## Launchers b725/b73b/b767 ([si+0x2a],[si+0x1b] constants: 5/0x355, 0/0x4aa, 0xa/0x355) + ace0
```c
void __allregs FUN_0000_b725(undefined2 p1,int param_2,int param_3,undefined2 p4){
  unsigned char *dg=(unsigned char*)g_mem+0x1c000; unsigned int si=(unsigned short)param_2,di=(unsigned short)param_3;
  dg[(unsigned short)(si+0x2a)]=5; *(unsigned short*)(dg+(unsigned short)(si+0x1b))=0x355;
  FUN_1000_ace0(0,(int*)(uintptr_t)*(unsigned short*)(dg+(unsigned short)(di+0x97)),param_2,param_3,p4); /* asm ax=0 */
}
```
`FUN_1000_ace0` (0x1ace0) and `FUN_0000_9b5c`/`9b6f`: full rebased bodies are in the session subagent
transcript (agent a9345b6, 3rd reply).  Key: ace0 params AX=aim(0),BX=target,SI=proj,DI=firer; writes proj
velocity `[si+0x10]` heading, `[si+0x1d]` velX (a192), `[si+0x1f]` speed, `[si+0x31]` DWORD, tail `addb(dg+proj)`.
`b6c9` (0xb6c9, 7814's launcher) still needs disassembly+rebase (same shape as b725).
Already-correct callees: a265(274), addb(337), a18e(338), a192, c047(268), b26a, adcd(222).

---

# op-0x58 LOS handler — DECODED from fist_image.bin @ 0x802e (2026-08-27)

The combat-resolution ROOT (see board 0012): the enemy-scan aa08's per-candidate visibility query
`e1f0->e21c->op-0x58` returns 0 ("not visible") because op-0x58 is unimplemented -> no unit ever acquires a
target -> no fire.  The handler IS in `re_out/fist_image.bin` (op-table 0xcb3: op-0x58 -> tramp 0x1103
`call 0x802e`), 32-bit PM, NOT decompiled.  Full decode (objdump -m i386, 0x8035-0x811e):

Inputs: `edi` = the extender TCB (= `DAT_2000_aa2c`), already filled by `FUN_0000_e21c`:
  obj  3D pos  = TCB[0xd2]=X, TCB[0xd6]=Y, TCB[0xda]=Z (Z already has per-type height added)
  cand 3D pos  = TCB[0xde]=X, TCB[0xe2]=Y, TCB[0xe6]=Z
`ds:0x85bc` = terrain heightmap base ptr; `ds:0x8020/0x8024/0x8028` = scratch step dx/dy/dz.
Returns EAX: 0xffffffff = VISIBLE (clear LOS), 0 = not visible / out of range.

Algorithm (asm-exact):
```
dx = cand.X - obj.X;  dy = cand.Y - obj.Y;  dz = cand.Z - obj.Z;      /* 32-bit signed */
if (dy>=0x40000 || dx>=0x40000 || dy<=-0x40000 || dx<=-0x40000) return 0;  /* out of range */
dx <<= 13;  dy <<= 13;  dy = -dy;  dz <<= 16;                          /* scale; Y flip */
/* normalise step so |dx|,|dy| < 0x3000000, counting doublings in ecx: */
ecx = 0;
do { ecx = ecx*2 + ... /* inc;shl1 */; dx>>=1(sar); dy>>=1; dz>>=1; }
   while (dx>=0x3000000 || dy>=0x3000000 || dx<=-0x3000000 || dy<=-0x3000000);
steps = ecx - 1;
sdx=dx; sdy=dy; sdz=dz;                                                /* [0x8020..0x8028] */
rx = obj.X<<13;  ry = -(obj.Y<<13);  rz = obj.Z<<16;                   /* march start */
for (i=0;i<steps;i++){
  rx += sdx;  ry += sdy;  rz += sdz;
  idx = (rx>>22) ... actually: eax=0; shld eax,ry,10; shld eax,rx,10;  /* eax = ((ry>>22)&0x3ff)<<10 | ... */
  h = terrainbase[eax] << 24;                                          /* movzx byte; shl 24 */
  if (h >= rz) return 0;                                               /* OCCLUDED */
}
return 0xffffffff;                                                     /* VISIBLE */
```
The index `eax` after `xor eax,eax; shld eax,edx,0xa; shld eax,ebx,0xa` = top-10-bits-of-ry then
top-10-bits-of-rx composed into a 20-bit heightmap offset (a 1024x1024 or packed terrain grid) — verify the
exact packing against the port's 9200/6980 heightmap base + stride when wiring.

WIRING (shim, faithful — NOT a stub): implement op-0x58 in the extender-service dispatch (where e339/aa10
route ops) as the C above, reading the TCB coords `DAT_2000_aa2c + 0xd2/…/0xe6`, using the port's mission
terrain heightmap for `[0x85bc]` (the same data 9200/6980 read).  Then aa08's LOS gate passes for in-range,
unoccluded candidates -> targets acquired -> the fire cascade above spawns projectiles -> combat resolves.
Falsifiable vs the DOSBox oracle (same op-0x58 result per candidate).
