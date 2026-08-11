# Mission-coverage expansion plan (DoD criterion #1 — "every mission/map")

Banked 2026-08-10 (recon on master ff53f25, read-only). Prerequisite for closing the
"every mission/map" axis. **Do not edit `tools/verify.sh` while `tools/consecutive.sh` runs** —
it re-reads verify.sh each iteration; changing the matrix mid-streak corrupts the 10× gate.
Apply the additions below only *after* the current gate reaches 10/10 (or resets).

## State
- Bit-verified in-mission render today: **exactly one** — `mission-cockpit`, AZER1, WEST/M1,
  reticle idx0 (method `26a1`), map D32/C32. `tools/verify.sh:231` + `MC_MOUSE`/`MC_REGION`
  at `:353-354`, special-case comparator `:402-424`, ref `ref/mission_azer1_cockpit_native320.png`.
- Enabling hook: `FIST_FSG_BATTLE=<NAME>` (patch 380) drives the standard `MC_MOUSE`
  (BATTLES 160,100 → OK 205,128 → ACCEPT 40,186) into ANY of the 47 `.FSG` through the full
  mission path. So a different-map M1 flow needs NO engine reconstruction — only a `run_mission`
  overload that exports `FIST_FSG_BATTLE`.

## Vehicle/reticle selection chain (asm-verified in pristine re_out/fist.c)
- Player unit chosen from roster `DAT_2000_2d3c[]` by side flag `DGROUP:0x6db4` (WEST=0 → M1;
  EAST=1 → opposing T80-class). `FUN_0000_463f` (fist.c:13106).
- `FUN_0000_466c` (fist.c:13137): `DAT_2000_2d32 = *param_1 * 2` (type word of the unit record),
  then per-type builder `word[0x6d7c + type*2]` installs that vehicle's methods incl. its reticle setter.
- 5 per-vehicle reticle setters write `DAT_1000_d550`: `7a38`=M1 (table 0x2360, COVERED);
  non-M1 `74e3`(0x2132)/`8463`(0x253a)/`8daa`(0x2700)/`9497`(0x2a10).
- Draw dispatch `FUN_0000_3a52` (fist.c:11702): `method = word[(d550&7)*2 + 0x3a52]`;
  idx0=26a1(covered) 1=2660 2=279d 3=2758 4=294d 5=290c 6=2a39 7=29f4.
- Debt (memory `reticle-sprite-dispatcher-debt`): [A] non-M1 setters STRSEG table-read base-loss;
  [B] sibling dispatchers idx∈{1,3,5,6,7} hardcoded in_CF=0 + `[0x158c]` blit-arg base-loss.

## Map per battle (from .FSG string tables)
AZER1=D32/C32(covered) · CYPRUS1/INDIA1=D06/C06 · SAUDI1/SYRIA1=D30/C30 ·
AZER2/UKRAINE8=D31/C31 · TRAIN1=D32/C32 · UKRAINE1=D32/C32.

## Prioritized flows to add (post-gate)
1. **mission-cockpit-cyprus1** — M1/idx0, map D06/C06 (new heightmap+colormap+PAL506). No
   reconstruction. Row: `"mission-cockpit-cyprus1|25000|missfb|CYPRUS1|$ROOT/ref/mission_cyprus1_cockpit_native320.png"`.
   `run_mission` variant exports `FIST_FSG_BATTLE=CYPRUS1`; keep `MC_MOUSE`/`MC_REGION` (M1 chrome
   is map-invariant). Oracle: `FIST_FSG_BATTLE=CYPRUS1 tools/refcapture_mission.sh 160 100 205 128 40 186 42 9 9 28 ref/mission_cyprus1_cockpit_native320.png`.
2. **mission-cockpit-azer2** — M1/idx0, map D31/C31. Same handler, `FIST_FSG_BATTLE=AZER2`. No reconstruction.
3. **mission-cockpit-azer1-east** — cheapest NON-M1: flip `DGROUP:0x6db4=1` on AZER1 (reuses loaded
   map) → non-M1 setter + idx≠0 sibling dispatcher. Needs a NEW env hook `FIST_MISSION_SIDE=EAST`
   (write [0x6db4] before d501, patterned on patch 380). Forces debt [A]+[B]. Add this FIRST as the
   *spec* for the [A]/[B] reconstruction (methodology: flow-before-reconstruction). Non-M1 chrome
   differs → reclassify a phase-invariant static crop region before banking AE=0.
4. **mission-cockpit-<apache/hind>** — remaining non-M1 setter (74e3/8daa) + its idx dispatcher.
   Needs `FIST_PLAYER_VEH=<type>` hook (write player-unit type word, analog to 466c).
5. **idx sweep** — after [A]/[B] land, pin a phase where `d550&7 ∈ {1,3,5,6,7}` to bit-verify each
   of 2660/2758/290c/2a39/29f4 (via FIST_MISSFB_N / a DUMPTICK phase pin).

## Caveats (honest)
- Flows 1-2 assume the non-AZER1 map renders crash-free on BOTH targets; a different map/side may
  re-expose latent object/event-subsystem or tile-pipeline debt (docs/mission_cockpit.md:47-72) —
  which is exactly what the flow is meant to surface.
- Flows 3-5 require new env hooks (none exist beyond FIST_FSG_BATTLE); the ".FSG player-unit" is not
  a standalone byte — it is roster+side selection, type = first word of the chosen unit record.

## AZER2 next-twin: FUN_0000_c7ca (2026-08-11, post-patch-389, static structure)
After patch 389 fixes the 2004 crash, AZER2's next crash = FUN_0000_c7ca+0x5c (fault-addr 0x2001a). Static:
c7ca is a per-object render helper -- `if((2d2c&1) && (2dab&2 || !(obj[0x16]&8) || obj[0x36])) { uVar1=c3ed(obj);
uVar2=(obj[0x26]+0x400)>>8 & 0xf8; iVar3 = uVar2 + (obj[0x16]&8 ? 0x11d8 : 0x10d8); uVar2=CONCAT11(id,uVar1);
if(obj[0x1c]) uVar2&=0xff; c8c2(uVar2,iVar3,param_1,obj,param_3); }`. param_2=obj (int). The fault at +0x5c is
a deref WITHIN c7ca (not inside c8c2) at raw 0x2001a (>64KB = a near-offset/derived ptr used without the
g_mem+0x1c000 base, OR an OOB index). NEEDS RUNTIME: repro AZER2 with FIST_SEGV_BT=1 FIST_SEGV_EBP=1 (+ extend
the segv diag to dump param_2/obj[0x26]/the derived iVar3) to pin which deref + the base. Same twin-migration
method as 2004/patch 389. Caller: get the EBP chain (c7ca's caller identifies the dispatch + whether obj is
base-lost like 47e0's params were). Blocked while the 10x gate holds /tmp binaries.

**STATIC REFINEMENT (2026-08-11, gate-hold RE).** fault-addr 0x2001a solved arithmetically: c7ca's earliest
obj derefs are `*(byte*)(param_2+0x16)` (both the gate cond @ build/fist.c:32266 and the branch @:32270).
0x2001a - 0x16 = **0x20004** => param_2(=obj) arrives as the RAW LINEAR offset 0x20004 (seg 0x2000, off 4),
NOT g_mem+0x20004. 0x20004 << g_mem base (0x081c51c0) => definitively a base-loss (missing g_mem), NOT a valid
host ptr and NOT a 16-bit near-offset (>0xffff, so it's a collapsed DAT_2000_-region far address = the object
roster/table segment). On AZER1 param_2 arrives as a proper host ptr => same class as 47e0->2004: a CALLER
passes obj without the g_mem base. **c7ca has ZERO direct C callers** (grep build/fist.c: only the fmap row
{0xc7cau,&FUN_0000_c7ca}) => it is dispatched via fist_icall_far from an object method slot (per-object render
walk). => the exact caller + the AZER1-vs-AZER2 object-ptr convention CANNOT be pinned statically; the EBP
chain at the 0x2001a fault is MANDATORY (extend FIST_SEGV_MGA-style diag or FIST_SEGV_EBP). Fix will be a
CALLER-side g_mem rebase of obj (like 389), NOT a c7ca-internal edit (c7ca's obj derefs are correct once obj
is a host ptr). Do NOT author blind: c7ca is a shared per-object render helper reached by every mission =>
wrong rebase regresses the 43-flow gate. Sequence: gate 10/10 -> repro AZER2 w/ EBP -> identify caller ->
confirm AZER1/CYPRUS1 obj convention -> author patch 390 -> re-gate -> add mission-cockpit-azer2.

## AZER2 c7ca ROOT-CAUSED + FULLY DECODED (2026-08-11, post-gate, runtime + asm) -> patch 390 spec
Gate a0bccac PASSED 10/10 (binaries freed). Ran the AZER2 segv capture (setarch -R, FIST_SEGV_BT/EBP/MGA):
- fault EIP 0x08082eb5 = c7ca+0x5c, fault-addr 0x2001a; **param_2(obj)=0x00020004** (confirmed: raw arg,
  not a base-loss of a real ptr). EBP chain: app_entry->00d0->cae6->e714->459a->22dd->378e->**c33c**->
  **c4df**->**c7ca**. So c7ca is dispatched ARG-LESS by c4df (asm c551 `call *%gs:0x3632(%bx)`), => ALL
  c7ca __allregs params are STALE garbage. NOT the c68c/389 base-loss class -- it is the c681/patch-310
  arg-less-render class. obj = SI = **g_fist_render_si** (c4df publishes it; c4df sets SI=obj, BX=CX=
  classid*2, DX=type_code, DI=render_di, BP=inherited).
- c7ca asm 0xc7ca-0x816 decoded: gate `if(!(2d2c&1)){cf=1;ret}`; skip `if(!(2dab&2)&&(obj[0x16]&8)&&
  obj[0x36]==0){cf=1;ret}`; render: c3ed(obj); bx=((obj[0x26]+0x400)>>8)&0xf8; if(!(obj[0x16]&8)){bx+=
  0x10d8; ah=0x24}else{bx+=0x11d8; ah=0x0e}; if(obj[0x1c])ah=0; c8c2(ax,bx,BP,obj,0); cf=0 (c8c2 clc ->
  c4df jae ret). ALL obj derefs -> dg+si. CF thread: skip=1(next obj), render=0(stop walk) via g_fist_cf.
- c3ed (asm 0xc3ed-0x47c) ALSO base-lost (never reached before): writes 6 {word,word,tag} records from
  obj[0x6e..0x85] (tags 'E'0x45..'@'0x40) into STRSEG:[a6e2] (GS=word[DGROUP:0x70]=STRSEG; a6e2=near
  offset, c3dc inits it 0x31ca; advance +30). SI=obj. Ghidra deref'd param_1 + *a6e2 as host ptrs.
  Its AX return is DISCARDED by c8c2 (c8c2 sets al=0x10) -> called for side-effect only.
- c8c2/ca2f (patch 310) already correct: obj arg = NEAR offset (ca2f does dg+(uint16)param_4); [di+0x1a]=
  BP, [di+0x1c]=BX(sprite base); `mov cx,bx` @ca38 is DEAD (CX unused). So only BP is inherited/untraced.
- BP ([di+0x1a]): not set in 378e/c33c/c4df; sibling c681->c962 passes 0 for this field; twin c584 (patch
  321, renders on AZER1 mission-cockpit AE=0) also uses inherited BP -> field is plausibly OUTSIDE the
  MC_REGION central-chrome crop. patch 390 passes 0 (documented; pin once the azer2 oracle exists).
PATCH 390 = reconstruct c7ca (obj=g_fist_render_si, dg-rebase, CF-thread) + c3ed (SI/STRSEG/a6e2 base-loss).
VALIDATE: AZER1+CYPRUS1+mission-cockpit AE=0 unchanged (they are type-0/M1, do NOT reach c7ca -> zero risk);
AZER2 advances past c7ca. Then build mission-cockpit-azer2 oracle (row 130,100 or FIST_FSG_BATTLE=AZER2,
MC_REGION crop, SETTLE=2), add flow, re-run 10x gate on the grown matrix.

## AZER2 twin #2: FUN_0000_3823 (2026-08-11, post-patch-390, asm-decoded) -> patch 391 spec
After patch 390 (committed 10ad4c7) fixes c7ca, AZER2's NEXT crash = FUN_0000_3823+0x74 (EIP 0x0805c1ff,
fault-addr 0xffff). EBP chain: 00d0->cae6->e714->459a->22dd->**378e+0xe8**->3823. 3823 is a per-object
SPRITE render (obj in BX at entry; C param_3=obj). asm 0x3823-0x38a9:
- 3d4c(0x6bc8)->iVar1; dx=-iVar1; si=word[obj+0x32]=sprite id.
- **es=word[DGROUP:0x4f0] (sprite-dir seg); les es:(si) (sprite far ptr); dx=word[es:si-4] (sprite w/h hdr).**
  The DECOMPILE (build/fist.c:12197 `*(undefined4*)*(undefined2*)(param_3+0x32)-4`) DROPPED the ES=[0x4f0]
  base + the les indirection -> derefs the sprite id (0xffff sentinel on AZER2) as a RAW host ptr => fault
  0xffff. Same sprite-dir idiom as 260c (`mov es,[0x4f0]; les si,es:[si]`).
- di=word[DGROUP:0x156a] (viewport struct); builds coord struct at DGROUP:0x6c16 (2c16/2c18/2c1a/2c1c) from
  viewport w/h (di+0xa/di+0xc >>1) + screen offsets; lcall [DGROUP:0x6c8]=c6c8 (sprite blit method).
- THEN a direct FB plot: es=word[di] (fb seg); si=width/2-iVar1 clamp<width; row=word[si*2+[0x794]]; 
  di2=height/2+cx clamp<height; al=byte[0x6c1b]; if(al) byte[es:di2+row]=al. (build/fist.c:12209-12212;
  base-loss: es=word[di] dropped; DAT_1000_c794 rowtable.)
RECONSTRUCTION (patch 391): rebase the sprite-dir resolution (es=word[0x4f0], les, [sprite-4]) + the FB plot
(es=word[di]) + the viewport di=word[0x156a] near-offset reads. **RUNTIME REACH-CHECK FIRST** (binaries were
busy w/ verify): 3823 is a general sprite-render likely ALSO on the AZER1/CYPRUS1 path -> confirm whether
they reach it (a diagnostic or dump obj[0x32]) BEFORE reconstructing; if they reach it AE=0 via the current
base-lost path the reconstruction could change output = regression. If NOT reached on AZER1/CYPRUS1 (likely,
since they're crash-free on the base-lost code), it is AZER2-type-1-object-specific -> safe. Twin-migration
continues (391 -> next). c6c8 (DGROUP:0x6c8) method install state also needs confirming (c6c1/c6d4/... family).

### patch 391 reconstruction sketch (symbols resolved 2026-08-11; WRITE+BUILD+TEST next slice, binaries were gate-held)
3823 is a per-object sprite render; obj in BX at entry (C param_3=obj near offset; param_2=cx). Resolved:
- DAT_1000_d56a ALREADY rebased (patch 325): (int*)(g_mem+0x1c000+word[0x156a]); [5]=width word[di+0xa],
  [6]=height word[di+0xc], *d56a=word[di]=FB seg (a VALUE; use (uint16_t)*DAT_1000_d56a << 4 for the FB base).
- DAT_1000_c794 = word[DGROUP:0x794] rowtable base (a near offset); correct read = *(int16_t*)(g_mem+0x1c000+
  (uint16_t)(X*2 + DAT_1000_c794)) (cf. the working pattern at build/fist.c:12442).
- sprite dir: es=word[DGROUP:0x4f0] (=*(uint16_t*)(g_mem+0x1c4f0)); sid=word[dg+obj+0x32]; les es:(sid) ->
  spr_off=word[(es<<4)+sid], spr_seg=word[(es<<4)+sid+2]; sprite hdr word = word[(spr_seg<<4)+spr_off-4].
Base-loss fixes in build/fist.c FUN_0000_3823 (12187-12216):
1. :12197 uVar4=sprite-hdr -> the es=[0x4f0] les chain above (Ghidra dropped ES + the les indirection ->
   deref'd sid(0xffff on AZER2) as a raw host ptr = fault 0xffff).
2. all obj derefs (param_3+0x32, param_3+5) -> dg+obj (near offset).
3. :12203 `2c1c = 2c18 + (uVar4 & 0xff)` is a Ghidra WIDTH error -- asm `add dx,ax` adds the FULL 16-bit
   sprite-hdr word: 2c1c = 2c18 + (uint16_t)uVar4. (2c1a = 2c16 + (uVar4>>8 & 0xff) is correct per asm.)
4. :12210 FB plot -> fb=g_mem+((uint16_t)*DAT_1000_d56a << 4); rowent=*(int16_t*)(g_mem+0x1c000+
   (uint16_t)(uVar4*2+DAT_1000_c794)); fb[(uint16_t)(uVar2+rowent)] = byte[dg+obj+5].
5. c6c8 (lcall [DGROUP:0x6c8]) already threaded elsewhere (c6c1/c6d4 family); pass bx=0x6c16 coord struct,
   al=sid -- confirm the DGROUP:0x6c8 install state on the AZER2 path (dump if the blit no-ops).
VALIDATE: full 43-flow verify BOTH must stay 43/43 (AZER1/CYPRUS1 either don't reach 3823 or now render it
correctly to the same AE=0 ref); AZER2 advances past 3823 to the next twin. Then continue twin-migration
until AZER2 reaches the MISSFB spawn frame -> build the mission-cockpit-azer2 oracle + add the flow + re-gate.

## AZER2 twin #3: op-0x60 spin / no op-0x24 present (2026-08-11, post-patch-391 HANG, not crash)
Patch 391 (3823 sprite render) fixes the 0xffff crash; AZER1/CYPRUS1 AE=0 (no regression). AZER2 now
advances PAST 3823 but HANGS (rc=124, no frame): the mission loop repeatedly posts `[ext] service op 0x60`
(display-list DATA, constant inbox=00000008 args 0f30/0000/0526) and NEVER posts op 0x24 (the windshield-
render present that AZER1/CYPRUS1 reach + that triggers the FIST_MISSFB dump at the Nth op-0x24). So AZER2's
459a/378e per-frame render never completes a frame -> spins before first present. Twin #3 = a HANG (a base-
lost loop counter / an object-render method that doesn't advance the frame for AZER2's type-1 object set, OR
a present-gate condition never met). DIAGNOSE next slice (needs a running binary; binaries were verify-held):
run AZER2 with the ophist diagnostic (native_main.c:1296 logs op40/op24/op0c/op18/op54/op60 counts + total)
to see which op-count grows unbounded + whether total climbs (spin) vs stalls (deadlock); then trace 378e's
object walk / the c4df a6e6/a6e8 cursor / the c7ca-or-3823-or-sibling method that fails to advance. NB the
op-0x60 poster is engine-side (459a/e4bb mission loop); the constant inbox=8 suggests the SAME display-list
cmd re-posted (frame never finalizes). Patch 391 is committable forward progress (crash->hang = deeper).

### twin #3 PINPOINTED (2026-08-11, ophist diff AZER1 vs AZER2) -- op-0x08 vs op-0x0c present divergence
FIST_OPHIST diff (both build terrain tile 3918 nz=65536 dist=175 fine; both op0x60=16):
  AZER1 (rc=0, frame): op0x18=1, op0x54(roster)=39, ... op 0x08 @total538 -> **op 0x24 present @total539** -> dump.
  AZER2 (rc=124 hang): op0x18=1, op0x54(roster)=**75**, ... **op 0x0c @total574 -> FREEZE** (total stops climbing;
   DEADLOCK not spin); op 0x24 NEVER posted, op 0x08 NEVER posted.
So at the frame-finalize point AZER1 posts op 0x08 (-> the op-0x24 windshield present) but AZER2 posts op 0x0c
and then WAITS on a completion the shim never satisfies. op 0x0c is a DIFFERENT display-list/finalize cmd,
conditional on mission state; AZER2's larger type-1 roster (75 vs 39 op-0x54) routes the mission loop
(459a/e4bb) into the op-0x0c branch. NEXT (twin #3 root-cause): find the engine site that posts op 0x08 vs
op 0x0c to the TCB inbox (task+0x3f2, posters 0xd94e..0xe37b) + the branch condition -- likely a base-lost
value/flag that for AZER2 selects op-0x0c (an error/alt-finalize path) instead of the op-0x08 present. Then
either fix the base-loss (so AZER2 takes the op-0x08 present) or, if op-0x0c is a legit cmd, handle it in the
shim's fist_extender_gate so the engine's post-0x0c wait completes -> op 0x24. Repro: FIST_OPHIST=1 ...
FIST_FSG_BATTLE=AZER2. FIST_OP0C_BT=1 (native_main.c:1228) backtraces the op-0x0c poster.

### twin #3 poster chain symbolicated (2026-08-11, FIST_OP0C_BT on AZER2)
op-0x0c backtrace (state: TCB seg aa2e=0x9000, d548=01, d549=0x1e cockpit; frame[0x452]=1, tcb+2c X=36958):
  fist_extender_gate <- e339 <- **db99** (PATCH 193 "op-0xc display-list geom-record poster") <- **41d0**
  (PATCH 193 "22dd handler, calls db99") <- **22dd** (phase dispatcher, +0xb8) <- 459a <- 22dd <- e714 <- cae6.
So op-0x0c = db99's geom-record post, dispatched by 41d0 as a 22dd phase handler.  The post RETURNS; then the
engine freezes with NO further ops (ophist total frozen at 574) = a HARD DEADLOCK in 22dd's phase walk AFTER
the op-0x0c post (not a spin -- 0 new ops).  AZER1 at the same point posts op 0x08 (-> op 0x24 present); AZER2
posts op 0x0c then a subsequent 22dd phase handler spins.  NEXT (twin #3 root-cause, needs runtime): trace
22dd's phase table (analog c33c's 10-entry table) to find which handler AFTER 41d0 loops for AZER2's type-1
roster -- add a per-phase log in 22dd's dispatch (like the FIST_C33C seam) to see the last phase entered before
freeze; or check whether 459a's present-pump spins because op-0x24 is gated on a flag AZER2 doesn't set (the
line-1225 tick-pump class -- but total is frozen, so it's an engine loop posting nothing, likely a base-lost
counter in the post-0x0c phase).  db99/41d0 themselves are fine (op-0x0c is a legit geom cmd; AZER1 posts it too
in other frames).  This is a deep mission-loop frontier -- fresh session, phase-by-phase.
