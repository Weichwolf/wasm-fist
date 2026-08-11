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

### twin #3 ROOT-CAUSED (2026-08-11) -- phase-table / display-list-buffer OVERLAP on chain B
Traced with a throwaway 22dd-do-while diag (logs phase 0a86 / word[0x6c96] / g_fist_render_di per iter; removed).
The 22dd per-frame phase walk is `c450=3; do{ call word[DGROUP:0a86](vp); }while(c450!=0)`; handler 2322 (it1)
picks a phase CHAIN by setting 0a86:
  AZER1 -> chain A @0x6c82: 23ef,41c7,**2471**(depth-sort),286e,3b59,23ce(term). 2471 relocates render_di to a
    safe sort-node buffer (rdi=0x538e, BELOW the phase table) -> records don't touch 0x6c82.. -> walk completes.
  AZER2 -> chain B @0x6c8e: 23ea,41d0(op-0x0c),3fba,**378e**(direct object walk),41ee,23ce(term). Chain B has
    NO 2471, so 378e writes display-list records via ca2f at the UNRELOCATED render_di=0x6bbe (378e's `mov
    $0x6bbe,di`), stride 0x32, ~8 records -> rdi grows 0x6bbe->0x6d4e, OVERRUNNING the phase table 0x6c82..0x6c98
    and zeroing chain-B's own 0x6c96 entry (static 0x41ee -> 0) BEFORE it6 reads it -> word[0x6c96]=0 ->
    fist_icall_near(0,0) no-op that never advances 0a86 / decrements c450 -> INFINITE SPIN (the twin-#3 hang).
Confirmed: static image DGROUP:0x6c96=0x41ee (valid handler 41ee), 0x6c98=0x23ce(term); at it1-it5 word[0x6c96]=
0x41ee, at it6 (after 378e) =0x0000; AZER1 rdi=0x538e (safe) vs AZER2 rdi=0x6d4e (overlap).
FIX (twin #3, next): chain B's object-render (378e path) must write records to a SAFE buffer like chain A's
2471 does -- i.e. render_di must be relocated off 0x6bbe before 378e/ca2f write, OR 2322 wrongly selects chain B
for AZER2 (a base-lost mode select) and AZER2 should use chain A. INVESTIGATE: (a) FUN_0000_2322 chain-select
logic (why B for AZER2 vs A for AZER1 -- a state/flag, likely base-lost); (b) chain B's intended render_di
setup (is a 2471-equivalent relocation dropped in chain B, or does 378e-in-chain-B use a different di base than
the phase-walk 0x6bbe?). Likely a base-loss in the chain-B render_di init OR in 2322's selection. Then AZER2
advances to op-0x24 present. re_out/fist.c pristine 61453e42 (no engine change this step -- diagnosis only).

### twin #3 chain-select resolved (2026-08-11) -- d549 0x1c (chain A) vs 0x1e (chain B)
2322 (it1) re-seeds `0a86 = word[DGROUP:0x4a88 + d549]` (d549 = viewport kind byte[0x1549]). Static table:
  d549=0x1c -> 0x6c82 (chain A: 23ef,41c7,**2471**depth-sort,286e,3b59,23ce) ; d549=0x1e -> 0x6c8e (chain B:
  23ea,41d0,3fba,**378e**,41ee,23ce). AZER1 renders the cockpit as d549=0x1c (chain A -> 2471 relocates
  render_di to 0x538e, safe); AZER2 as d549=0x1e (chain B -> 378e writes at render_di=0x6bbe, overruns the
  phase table @0x6c96). So the DIVERGENCE is d549 (0x1c vs 0x1e). NEXT (twin #3 fix, fresh session, SHARED
  mission-render path -> full validate + re-gate after):
  (Q1) WHY is d549=0x1e for AZER2 vs 0x1c for AZER1 -- a legit viewport-mode difference (different .FSG view
       config) or a base-loss in the d549 setup? Find who writes word[DGROUP:0x1549] pre-2322 + why it differs.
  (Q2) If chain B (d549=0x1e) is legit: it MUST relocate render_di off 0x6bbe before 378e/ca2f (chain A does
       this via 2471); the dropped relocation is the base-loss. If d549=0x1e is WRONG: fix the d549 setter so
       AZER2 uses chain A like AZER1.
  Also verify whether AZER1 EVER runs chain B (a later 22dd call/viewport) without hanging -- if so, AZER1's
  chain B has a relocated render_di and the AZER2 base-loss is in that relocation path specifically.
  Diagnosis-only this session; re_out/fist.c pristine 61453e42, repo clean.

### twin #3 FULL trigger chain (2026-08-11) -- d548==0 -> 67e3 -> d549=0x1e -> chain B render_di overrun
The d549=0x1e view is entered by FUN_1000_67e3 (sets d548=1,d549=0x1e), called from ONE site: FUN_0000_4937
(a 209e paint handler, patch 180) `if (d548==0) { if(3a35==1) 5087(); 67e3(); return; }`.  So AZER2 has
DAT_1000_d548==0 when 4937 runs -> switches to view 0x1e -> chain B.  a84c (d549=0x1c cockpit, 5 callers) is
the normal path AZER1 takes (d548 != 0 at 4937).  d549=0x1e is a LEGIT view mode (67e3), so the bug is NOT
the view select -- it is chain B's render_di: chain A relocates render_di via 2471 (it4) to a safe sort-node
buffer (0x538e); chain B has no 2471, so 378e (it5) writes ca2f records at render_di=0x6bbe (378e asm 0x379e
`mov $0x6bbe,di; call [0x1630]`=c33c), stride 0x32, ~8 recs -> 0x6d4e, OVERRUNNING the phase table @0x6c96.
OPEN (the fix): in the REAL 16-bit game chain B's 378e ALSO sets di=0x6bbe, yet the phase table is at 0x6c82
(static) -- so either (a) render_di's RECORD writes go to a SEPARATE buffer set up by an earlier chain-B
handler (23ea/41d0/3fba) that our port's reconstruction DROPPED (render_di stayed 0 until 378e per the diag
-> a dropped buffer-alloc base-loss), or (b) 378e's di=0x6bbe is the phase-walk cursor and the record dest is
a different di the port mis-threads.  NEXT: read 378e asm (0x378e..) + trace where its ca2f records' di base
comes from vs chain A's 2471; the dropped relocation/alloc is the base-loss to restore.  Then chain B completes
-> op-0x24 present -> AZER2 renders.  Diagnosis-only; re_out pristine 61453e42.  Also worth checking: why is
d548==0 for AZER2 vs !=0 for AZER1 at 4937 (may be an orthogonal state diff, but 0x1e is legit either way).

### twin #3 diagnosis CLOSED (2026-08-11) -- 378e asm + the decisive next test
378e asm (0x378e-0x3822) confirmed: it walks objects via c33c(si=[0x4b9e], di=0x6bbe), dispatches each
via `call [0x4b52+byte[0x6bc2]]`, and on exhaustion (c33c CF=1) does `addw $2,[0x4a86]` (advance 0a86 to
0x6c96) + ret.  di=0x6bbe is LITERAL (asm 0x379e), so chain-B records go to 0x6bbe -> only SAFE if FEW
objects are visible (records don't reach 0x6c96).  => the two candidate roots are ENTANGLED:
  ROOT-A: AZER2 wrongly enters chain B -- d548==0 at 4937 -> 67e3 -> d549=0x1e.  If d548==0 is a base-loss
    (AZER1 has d548!=0 -> a84c/d549=0x1c/chain A), fixing d548 makes AZER2 render via chain A like AZER1.
  ROOT-B: chain B is legit for AZER2 but its record buffer (0x6bbe) collides with the phase table for a
    full roster -- the real game must place chain-B records elsewhere (a dropped alloc/relocation).
DECISIVE NEXT TEST (fresh session): temp-diag d548 (+ 3c08/3a40/3a35) at FUN_0000_4937 entry for AZER1 vs
AZER2 -- if AZER1 d548!=0 and AZER2 d548==0, ROOT-A (find the d548 setter that differs; likely a mission-init
base-loss).  If both d548==0 (both would enter chain B, but AZER1 renders because fewer visible objects),
ROOT-B (chain-B record buffer).  Either fix is a SHARED mission-render change -> AZER1/CYPRUS1 AE=0 + 43/43 +
re-gate.  Diagnosis fully mapped this session; re_out/fist.c pristine 61453e42; patches 390+391 committed.

### twin #3 A/B RESULT (2026-08-11) -- AZER1 never calls 4937; AZER2 does (d548==0)
Decisive temp-diag at FUN_0000_4937 entry (removed): AZER1 renders (rc=0) and NEVER calls 4937 -> stays in
a84c/d549=0x1c/chain A, dumps at op-0x24, exits.  AZER2 calls 4937 ONCE with d548=00,3c08=0,3a40=0,3a35=0
-> 67e3 -> d549=0x1e -> chain B -> hang.  So the divergence is UPSTREAM of d548: AZER2's 209e display-list
walk reaches an element whose PAINT handler is 4937 (the 0x1e-view initializer); AZER1's walk does not (it
reaches op-0x24 first and the MISSFB dump exits).  AZER2=map D31, AZER1=map D32 -> different display lists,
so chain B (d549=0x1e) is plausibly a LEGIT path for AZER2.  => most likely ROOT-B: chain B's 378e records
(render_di=0x6bbe LITERAL) overrun the phase table because our port renders TOO MANY objects (a visibility/
cull base-loss -> 8 records to 0x6d4e; real game likely culls to <4 so records stay below 0x6c82), OR the
chain-B record buffer should be relocated.  NEXT (fresh session): (1) trace 378e's object-visibility cull
(`call [0x4b52+byte[0x6bc2]]` per node) -- how many records SHOULD chain B write for AZER2's view vs how many
it does; (2) compare to a DOSBox AZER2 oracle for the 0x1e view.  A cull base-loss (too many records) is the
leading hypothesis.  This is the deepest mission-render frontier; fully mapped, needs runtime cull analysis +
a DOSBox oracle.  re_out/fist.c pristine 61453e42; patches 390+391 committed + 43/43-verified.

## MISSION SURVEY (2026-08-11, post-391) -- 3 more crash-free missions found (native)
run_mission survey (native, FIST_FSG_BATTLE, MC_REGION crop vs the genuine azer1/cyprus1 DOSBox refs):
| battle  | map     | native result   | central-chrome crop match |
|---------|---------|-----------------|---------------------------|
| AZER1   | D32/C32 | OK (flow)       | == azer1ref (AE=0)        |
| CYPRUS1 | D06/C06 | OK (flow)       | == cyprus1ref (AE=0)      |
| SAUDI1  | D30/C30 | OK-rendered     | **== azer1ref (AE=0)**    |
| SYRIA1  | D30/C30 | OK-rendered     | **== azer1ref (AE=0)**    |
| INDIA1  | D06/C06 | OK-rendered     | **== cyprus1ref (AE=0)**  |
| UKRAINE1| D31/C31 | CRASH (segv)    | -- (twin-class, D31 like AZER2) |
| TRAIN1  | D32/C32 | HANG            | -- (twin-#3 class)        |
| AZER3   | D31/C31 | CRASH (segv)    | -- (D31 like AZER2)       |
The M1 central chrome has TWO variants (AE=90 apart, likely the spawn-speed MPH needle): azer1-type
(AZER1/SAUDI1/SYRIA1) and cyprus1-type (CYPRUS1/INDIA1).  SAUDI1/SYRIA1/INDIA1's port crop is BIT-IDENTICAL
(AE=0) to an existing GENUINE DOSBox ref -> they are near-ready new flows.  D31 maps (AZER2/AZER3/UKRAINE1)
crash or hang = the twin-class frontier (chain B / object cull).  TRAIN1 (D32) hangs = twin-#3 class too.
NEXT to add SAUDI1/SYRIA1/INDIA1 as verify flows: (1) native<->wasm identity (SAUDI1 running bg id=bda8wedup),
(2) a GENUINE per-mission DOSBox ref (refcapture_mission_row.sh, or accept the azer1/cyprus1 ref IF a DOSBox
SAUDI1 capture confirms the M1 chrome is truly mission-invariant), (3) add 3 rows to verify.sh + re-gate.
Rigor note: using azer1ref for SAUDI1 assumes the real SAUDI1 DOSBox chrome == AZER1's; confirm with a genuine
SAUDI1 DOSBox capture before banking (code-is-truth).  This survey de-risks 3 of the 40+ missions cheaply.

### SURVEY CORRECTION (2026-08-11) -- SAUDI1 wasm TIMES OUT (native<->wasm parity NOT confirmed)
SAUDI1 wasm (node, timeout 260s, correct OUTJS=/tmp/fisttest/fistrun.js): **rc=124, NO frame** -- native
renders SAUDI1 in ~15s, wasm does NOT in 260s (vs cyprus1/azer1 wasm which pass at 220s).  So SAUDI1 (and
likely SYRIA1/INDIA1) hit a WASM-SIDE hang or pathological slowness -> the native<->wasm HARD INVARIANT is
NOT met -> they are NOT addable as dual-target flows yet.  This is the [[wasm-mission-op50-blocker]] territory
(a known OPEN wasm mission-render divergence) OR a heavier-map wasm slowdown.  So the survey's "3 near-ready"
is DOWNGRADED: native-crash-free + native-chrome-bit-exact, but wasm-BLOCKED.  NEXT before adding any of them:
diagnose the SAUDI1 wasm timeout (op-histogram on the wasm build; is it stuck like the op-0x1c blocker, or
just slow -> try a longer timeout / higher FIST_TICK_HZ).  The native-side result (chrome == genuine azer1/
cyprus1 ref AE=0) still stands + de-risks the NATIVE render, but dual-target parity is the gate.  Honest: 0
new flows addable from this survey until the wasm mission timeout is resolved (shared with twin-class work).

### SAUDI1 wasm = REAL HANG (2026-08-11) -- confirms the wasm-mission divergence blocks the new missions
SAUDI1 wasm at timeout 520s: STILL rc=124, no frame -> a REAL wasm HANG (not slowness; native renders in
~15s).  So SAUDI1 (native-renders) HANGS on wasm = a native<->wasm HARD-INVARIANT divergence, the
[[wasm-mission-op50-blocker]].  AZER1/CYPRUS1 wasm render (patches 381/386 fixed their maps), but SAUDI1
(map D30) does not -> the wasm mission divergence is MAP/MISSION-specific and affects at least SAUDI1/SYRIA1/
INDIA1.  => these 3 are NATIVE-ready but WASM-BLOCKED; 0 addable until the wasm divergence is fixed.  This is
a distinct high-priority frontier (blocks multiple missions AND breaks the dual-target invariant on a working
native render).  NEXT (fresh session): FIST_OPHIST on the SAUDI1 WASM build to find the stall op (vs native's
op sequence), then trace that op's wasm-vs-native path (call_indirect signature / a base value that differs
under EMULATE_FUNCTION_POINTER_CASTS).  Two independent mission frontiers now mapped: (i) twin-#3 native hang
(AZER2/D31 chain-B render_di), (ii) wasm-mission divergence (SAUDI1/D30 + op50-blocker).  Both fresh-session.

### wasm-mission divergence SHARPENED (2026-08-11) -- native NEVER posts op 0x1c; wasm does -> control-flow split
FIST_OPHIST diff SAUDI1 (native /tmp/fist_native vs wasm node /tmp/fisttest/fistrun.js):
  NATIVE: ...op54(roster)=63, op60=12, -> op 0x24 present @total559 -> renders (NO op 0x1c ever).
  WASM:   ...op54 @t50, op60 @t113, -> **op 0x1c @t125 -> FREEZE** (op 0x24 never reached).
=> the SAME engine build (build/fist.c) takes a DIFFERENT branch on wasm: wasm posts op 0x1c (an alt display-
list cmd) at total~125 while native never posts it and continues to op 0x24.  So it is a native<->wasm CONTROL-
FLOW divergence rooted in a value that differs between builds (uninitialized/UB, or a pointer/offset handled
differently under -sEMULATE_FUNCTION_POINTER_CASTS), NOT a pure call_indirect trap.  NEXT (fresh session, slow
wasm iters ~6min): (1) find the engine site that posts op 0x1c (the poster family d94e..e37b; op 0x1c=28 to the
TCB inbox task+0x3f2) + its branch condition; (2) instrument that branch's deciding value on BOTH targets
(a shim print, since node has no backtrace) to see which value diverges; (3) that value's base-loss/UB is the
fix.  Distinct from twin-#3 (AZER2 native chain-B).  This blocks SAUDI1/SYRIA1/INDIA1 dual-target flows.

### wasm-mission divergence ROOT-CAUSED to c0ca/2dab store-width (2026-08-11) -- but fix needs a DOSBox trace
The op-0x1c poster is FUN_0000_dde2, called ONLY by FUN_0000_c0ca: asm `cmpb $0,[0x6dab]; jne ret` -> posts
op 0x1c (dde2) IFF byte[0x6dab]==0.  Ghidra typed DAT_2000_2dab undefined2 (WORD) but ALL ~40 asm accesses to
0x6dab are BYTE (cmpb/testb/xorb/movb); 2daa(0x6daa) + 2dac(0x6dac) are ALSO byte flags (2dac = editor BATTLE-
LOCKED).  So the WORD read of 2dab pulls in 2dac's byte.  NATIVE: 2dac!=0 -> (2dab|2dac<<8)!=0 -> c0ca SKIPS
op 0x1c -> op 0x24 -> renders (AE=0 vs DOSBox).  WASM: 2dac==0 -> WORD==0 -> c0ca POSTS op 0x1c -> HANG.
**CRITICAL: do NOT blindly retype 2dab->undefined1** -- the asm condition is `byte[0x6dab]==0`; if that byte is
0 on BOTH targets (only 2dac differed), retyping makes BOTH post op 0x1c -> BOTH hang (native REGRESSION).
Native currently renders AE=0-vs-DOSBox WHILE skipping op 0x1c (via the 2dac accident) -> either DOSBox also
skips (its 2dab byte!=0) OR op 0x1c doesn't affect the crop.  TWO possible real fixes, DISAMBIGUATED ONLY by a
DOSBox SAUDI1 trace at c0ca: (A) if real 2dab byte!=0 -> wasm's 2dab byte==0 is a separate base-loss (find the
2dab setter that differs; retype 2dab->byte THEN fix the value); (B) if real 2dab byte==0 -> op 0x1c SHOULD be
posted + our extender gate's op-0x1c handling is what hangs (fix dde2/e339 op-0x1c path).  NEXT: DOSBox SAUDI1
to capture byte[0x6dab] @ c0ca + whether the real game posts op 0x1c.  Same oracle dependency as twin #3.
Both mission frontiers now bottleneck on a DOSBox mission oracle -> that oracle setup is the true next task.

### wasm-mission divergence CORRECTED (2026-08-11) -- it's the COOPERATIVE-TICK model, NOT 2dab store-width
Native c0ca diag (FIST_DIAGC0CA): c0ca is NEVER reached on native (SAUDI1 or AZER1) -> the 2dab read at c0ca
was DOWNSTREAM, not the cause (native never gets there).  c0ca's callers (build/fist.c:13769/13832) are the
PER-TICK SIM step, gated by `9 < (DAT_1000_c452 - DAT_2000_2ce0)` (>=10 elapsed INT-8 ticks) then
`[2ce4]() ; c0ca(); 461b()` (flight model).  NATIVE SAUDI1: reaches op 0x24 (MISSFB dump) @total559 and the
tick gate NEVER fires (c452-2ce0 stays <10) -> renders + exits.  WASM SAUDI1: c452 advances FASTER relative to
render -> the gate fires EARLY (c0ca @total~125, before op 0x24) -> c0ca posts op 0x1c (dde2) -> the per-tick
sim/tick path HANGS.  So the ROOT is a native<->wasm COOPERATIVE-TICK-MODEL divergence (the INT-8 tick c452
advances at a different rate vs render on wasm), a SHIM-level parity bug (tools/native_main.c fist_timer_pump /
the in-mission cooperative tick, #ifdef __EMSCRIPTEN__), NOT an engine base-loss.  AZER1/CYPRUS1 wasm render
because they reach op 0x24 faster (39 roster op54 vs SAUDI1's 63) -> before the tick gate.  NEXT (shim-level,
no DOSBox needed -- it's native<->wasm parity): make the in-mission cooperative tick advance c452 IDENTICALLY
relative to render on both targets (align fist_timer_pump's per-pump tick increment; native in-mission uses the
same cooperative path per native_main.c:434 in_mission gate).  THEN either the sim runs deterministically on
both (and op 0x1c must be handled) or neither reaches it before op 0x24.  Supersedes the 2dab-store-width note.

### wasm-mission tick-cadence root (2026-08-11) -- pre-cockpit c452 divergence; fix risks AZER1/CYPRUS1
native_main.c:420-460: MENUS converge (timing-independent); MISSION coop-cadence (native driven 1 tick/pump
like wasm) activates ONLY at d549==0x1c (cockpit).  During mission LOAD (before cockpit) native uses SIGALRM
(up-to-4 ticks/pump -> [0x452]=1 at map-load) while wasm uses 1/pump ([0x452]=274) -> c452 diverges HUGELY
pre-cockpit.  AZER1/CYPRUS1 reach op 0x24 (MISSFB dump) before the `9 < c452-2ce0` sim-gate fires -> render
dual-target.  SAUDI1 (63 roster op54 vs 39) crosses the sim-gate on wasm FIRST (c0ca @total125) -> op 0x1c ->
hang.  FIX (shim-level, no DOSBox): align c452 native<->wasm from mission-LOAD (extend the coop cadence to
g_fist_after_map, not just d549==0x1c) so both advance the tick identically.  **REGRESSION RISK: this changes
the exact cadence under which AZER1/CYPRUS1 are bit-verified (their MISSFB frame may depend on c452)** -> after
any cadence change, re-verify AZER1/CYPRUS1 mission-cockpit AE=0 + the full 43-flow gate BOTH targets, and if
their frames shift, re-capture the genuine DOSBox refs at the aligned cadence.  This is the accurate root of
the wasm-mission hang (supersedes both the op-0x1c-poster and 2dab-store-width notes -- those were downstream).
Careful shim-timing work; fresh session.  All findings verified vs asm + native runtime ("code is the truth").

### wasm-mission tick root REFINED (2026-08-11) -- sim-gate baseline + the real fix is the loop pump model
DAT_2000_2ce0 = DAT_1000_c452 is REFRESHED (build/fist.c:13731/13747/13810), so `9 < c452-2ce0` fires the
per-tick SIM (c0ca/461b flight model) every ~10 elapsed ticks.  Native accumulates ticks at ~real-time
(SIGALRM) so op-0x24 renders BETWEEN sims (MISSFB dumps first); wasm's 1-tick-per-pump OVER-accumulates within
one frame's drain loop (13755-13767) -> the sim fires BEFORE op-0x24 -> c0ca -> op 0x1c, and the post-sim
mission loop then spins WITHOUT cooperatively pumping the tick (the known "459a tick-pump" class,
native_main.c:1225) -> HANG.  So the TRUE fix is the mission-loop tick/pump/sim COOPERATIVE MODEL, 3 parts:
(1) a DETERMINISTIC tick rate identical native<->wasm AND matching the game's ~18.2Hz-vs-render cadence (so
the sim fires at the same logical points on both, like DOSBox); (2) the post-sim loop must pump the tick so
it progresses (not spin); (3) op 0x1c (dde2) must be handled by the extender gate so the sim step completes.
This is careful multi-part shim+loop work with REGRESSION RISK to the bit-verified AZER1/CYPRUS1 cadence
(re-verify + possibly re-capture refs after).  Options to explore: tie c452 to a deterministic render-milestone
count (op-0x24 frames) instead of per-pump/wall-clock; or cap ticks-per-frame so the sim can't outrun the
render.  This supersedes/completes the tick-cadence note as the accurate, full characterization of the
wasm-mission hang.  Fresh session; no DOSBox oracle needed for the parity part (native<->wasm), but a DOSBox
SAUDI1 cadence reference helps confirm the ~18.2Hz sim rate.

### wasm-mission tick fix — EXPERIMENT RULED OUT (2026-08-11): coop-from-map-load regresses CYPRUS1
Tested the leading hypothesis (align c452 by making native cooperative — 1 tick/pump — from mission-LOAD
g_fist_after_map, not just cockpit d549==0x1c).  RESULT: AZER1 still AE=0, but **CYPRUS1 CRASHED (rc=139)**
and SAUDI1 still hung (rc=124).  Cleanly reverted (git checkout native_main.c); AZER1+CYPRUS1 AE=0 restored.
CONCLUSION: native's SIGALRM tick cadence DURING mission-load is LOAD-BEARING (some load-phase code depends on
the real-time-ish tick rate; forcing 1/pump there breaks CYPRUS1's load).  So the fix must PRESERVE the
load-phase tick behavior while aligning ONLY the in-mission sim-cadence (c452-2ce0 gate) native<->wasm -- a
more surgical change than extending coop earlier.  Solution space narrowed: NOT a coop-activation-timing
change.  Candidate directions for next session: (a) make the sim-gate delta (c452-2ce0) computed from a
deterministic per-frame counter identical on both targets (decouple the sim cadence from the raw tick count);
(b) throttle ONLY the wasm in-mission tick to a per-rendered-frame rate (op-0x24 count) so it matches native's
real-time cadence without touching load.  Both need full 43-flow re-gate (broad tick surface).  Empirical
iteration: hypothesis tested + ruled out + reverted clean ("Zurück ist erlaubt").

### wasm-mission: 459a loop structure decoded (2026-08-11) -- render(206f) vs sim ordering is the divergence
FUN_0000_459a (build/fist.c:13731-13774) is the fixed-timestep mission loop:
  2ce0=c452; 2ce2=0;
  do { fist_timer_pump();                         // 1 tick/outer-iter (patch 295)
       206f(...);                                  // RENDER (display-list walk; op-0x24 present path)
       2ce2 += (c452-2ce0); 2ce0=c452;            // accumulate elapsed ticks
       for(; 2ce2!=0; 2ce2--){ drain events (while c40a poll !empty -> 1e4b);
                               if (9 < c452-2ce0) break;   // SIM-GATE
                               [2ce4](); c0ca(); 461b(); } // SIM STEP (flight model -> c0ca -> op-0x1c)
     } while(...)
KEY: 206f (render) runs BEFORE the sim each outer iteration.  NATIVE SAUDI1: the MISSFB harness exits at
op-0x24 (reached via 206f's render path) on the first outer iter BEFORE the sim loop -> c0ca NEVER called
(matches the FIST_DIAGC0CA diag: 0 calls native).  WASM SAUDI1: op-0x24 is NOT reached before the sim fires
(c0ca -> op-0x1c @total125) -> hang.  So the divergence is 206f's render-completion (op-0x24 present) TIMING
vs the sim, through the cooperative event/tick pump ORDERING -- NOT the raw tick baseline.  NEXT (measure, not
guess): instrument 459a per-outer-iteration (log c452, 2ce2, did-206f-post-op-0x24, did-c0ca-run) on native
(fast) + wasm (1 run) to see exactly why wasm's 206f doesn't post op-0x24 before the sim.  Likely: op-0x24 is
posted by an EVENT dispatched in the drain (1e4b->df0e), and wasm's event-queue/drain timing differs so the
present event isn't dispatched before the sim-gate.  The fix targets the cooperative event-drain vs sim
ordering (deterministic + native<->wasm-identical), preserving the load-phase cadence (ruled-out experiment).

### wasm-mission divergence — COMPLETE root cause + fix spec (2026-08-11, measured both targets)
459a per-iter trace (FIST_DIAG459A, removed): NATIVE SAUDI1 enters 459a with c452=1, stays 1 (2ce2=0) -> NO
sim -> op-0x24 spawn -> MISSFB exits.  WASM SAUDI1 enters with c452=274 (load-phase 1-tick-per-pump
accumulation) AND 206f advances it +40 -> 314 (206f's cooperative render spin-waits pump the tick per
iteration) -> 2ce2=40 -> the per-tick SIM (c0ca->op-0x1c) runs 40x before the spawn op-0x24 -> HANG.
COMPLETE ROOT CAUSE: the in-mission tick is WALL-CLOCK/PUMP-based -> native (fast, few ticks to spawn) and
wasm (~15x slower, hundreds of ticks) accumulate DIFFERENT c452 by the spawn frame; wasm's excess crosses the
sim-gate before op-0x24.  The tick is NOT deterministic (depends on wall-clock speed + cooperative-pump-
iteration count, which differ native<->wasm).  FIX (the real one): a DETERMINISTIC FRAME-TIED tick -- advance
c452 by a FIXED amount per op-0x24 PRESENT (rendered frame), NOT per fist_timer_pump/SIGALRM.  Then both
targets advance c452 identically per frame -> the sim fires at the same logical points -> parity, AND it
matches the game's ~per-frame tick cadence (DD2 frame-clock precedent, CLAUDE.md).  Scope to in-mission
(fist_timer_pump stops advancing c452 during the render spins; the op-0x24 gate advances it once per present).
CAUTION: the mission tick timing is DELICATE (the coop-from-map-load experiment regressed CYPRUS1) -> after
implementing, full 43-flow gate BOTH targets + re-verify AZER1/CYPRUS1 mission-cockpit AE=0 (and re-capture
refs if the frame-tied cadence shifts their spawn frame).  This SUPERSEDES all prior wasm-mission notes as the
complete, measured characterization.  It is the accurate, bounded fix -- careful broad-surface shim work,
fresh session, no DOSBox oracle needed (native<->wasm parity).

### wasm-mission tick-hold FIX LANDED (2026-08-11, commit 49f8cfb) -- 3 missions unblocked dual-target
The wasm-mission divergence is FIXED (tools/native_main.c fist_timer_pump; wasm-only; engine pristine
61453e42; verify.sh both = 43/43 no regression).  SAUDI1/SYRIA1/INDIA1 now RENDER on BOTH targets (were
wasm-hung): each central-chrome crop AE=0 native==wasm, and == the genuine DOSBox refs (SAUDI1/SYRIA1 ==
azer1ref, INDIA1 == cyprus1ref -- confirming the two M1-chrome variants).
NEXT (add the 3 dual-target mission flows -> "every mission" axis 2->5):
- RIGOR: the DoD wants each mission compared to ITS OWN genuine DOSBox ref.  SAUDI1/SYRIA1 are deep in the
  scrollable SELECT BATTLE list (rows ~21/~28; AZER1-7,CYPRUS1-7,INDIA1-7,SAUDI1-7,SYRIA1-7,TRAIN,UKRAINE),
  so refcapture_mission_row.sh (single row-click, 8 visible rows) can't reach them without SCROLL support.
  Options: (a) extend refcapture to scroll the list then click; (b) capture ONE genuine SAUDI1 ref to PROVE
  SAUDI1-real == azer1ref (M1-chrome mission-invariance) then add SAUDI1/SYRIA1 vs azer1ref + INDIA1 vs
  cyprus1ref (INDIA1 is row ~14, also needs scroll).  Either needs the scroll-click DOSBox capture.
- Then add 3 rows to verify.sh (run_mission variant with FIST_FSG_BATTLE, MC_REGION crop, SETTLE=2) + re-gate
  the grown 46-flow matrix (10x).
This is the cheapest remaining "every mission" gain (the fix did the hard part).  twin-#3 (AZER2 NATIVE
chain-B) remains separate + open (DOSBox oracle).

### mission-flow EXPANSION plan (2026-08-11, post-46-flow-gate) -- same-map ref reuse = many missions cheaply
The MC_REGION central chrome is MAP-GROUP-invariant (proven: genuine SAUDI1 D30 == azer1ref D32 AE=0).  So
same-map missions share a genuine DOSBox ref -> NO new capture needed, just a crash-free dual-target render +
AE=0 check.  Map groups (from the .FSG list): D32/D30 -> azer1-type (azer1ref / saudi1ref); D06 -> cyprus1-type
(cyprus1ref).  Maps: AZER=D31(2-7)/D32(1); CYPRUS/INDIA=D06; SAUDI/SYRIA=D30; TRAIN=D32; UKRAINE=D31.
CHEAP additions (M1, reuse refs; POST-GATE, need per-battle crash-free dual-target check via run_mission):
  - D30 (saudi1ref): SAUDI2-7, SYRIA2-7   - D06 (cyprus1ref): CYPRUS2-7, INDIA2-7   - D32 (azer1ref): TRAIN1-4
CAVEAT: D31 missions CRASH/hang (AZER2/AZER3/UKRAINE1 = twin-#3 native chain-B, TRAIN1 hung earlier -- retest
w/ the tick-hold fix, some hangs may now be fixed).  METHOD per battle: run_mission native+wasm (FIST_FSG_BATTLE),
if both render + crop AE=0 vs the same-map ref -> add the flow.  Batch these post-gate, re-gate the grown matrix.
The scroll-ref-capture (/tmp/refcap_saudi.sh) is only needed for a NEW map group (none left among M1: D30/D06/D32
covered; D31 blocked on twin-#3).  This could take "every mission" from 5 to ~20+ M1 missions cheaply once the
D31/twin-#3 native crash is fixed (the remaining mission blocker).  DO NOT run these during a gate (CPU/RUNMS).

### EXPANSION plan CORRECTED (2026-08-11, post-46-gate native survey) -- "2+" missions crash (twin-class)
Native survey of same-map "2" missions: SAUDI2 HANG, SYRIA2 HANG, INDIA2 CRASH (rc=139).  So the same-map ref
reuse does NOT give free missions -- the mission NUMBER (richer unit roster / type-1 objects) triggers the
twin-class native crashes (like AZER2), independent of the map.  The 5 bit-verified missions (AZER1/CYPRUS1/
SAUDI1/SYRIA1/INDIA1) are all "1"s = simplest rosters.  ALL "2+" missions + all D31 (AZER2/3, UKRAINE) are
blocked on the SAME frontier: **twin-#3 native chain-B render_di overrun @0x6c96** (docs "twin #3 ROOT-CAUSED":
chain B's 378e writes ~8 display-list records at render_di=0x6bbe, overrunning the phase table; chain A
relocates via 2471, chain B doesn't).  So twin-#3 is THE key mission-expansion blocker -- fixing it unblocks
AZER2 + SAUDI2-7 + SYRIA2-7 + INDIA2-7 + more (dozens of missions).  The faithful fix needs the real game's
chain-B cull count / buffer layout (a DOSBox AZER2/SAUDI2 mission oracle) to know whether the real game
overruns (=> a cull base-loss in our port renders too many) or relocates render_di.  THAT is the highest-
leverage next target (a DOSBox mission-state trace of chain B).  The tick-hold fix already cleared the wasm
side; the native twin-#3 crash is the remaining mission blocker.

### twin #3 REFRAMED by DOSBox oracle (2026-08-11) -- AZER2 real = COCKPIT (chain A); port wrongly enters chain B
Captured a GENUINE AZER2 DOSBox frame (index 1, row y100, no scroll; ref/mission_azer2_cockpit_native320.png).
Result: AZER2's central chrome AE=72 vs azer1ref (CLOSE = a cockpit, minor AZER2-specific chrome diff); full
frame AE=27472 vs azer1 (its own D31 terrain).  So the REAL AZER2 renders the COCKPIT view (d549=0x1c, chain A)
like AZER1/SAUDI1 -- NOT the 0x1e external view (chain B).  => the port's twin-#3 hang is NOT a chain-B cull
issue; it is a WRONG-VIEW-SELECTION bug: the port wrongly enters chain B (d548==0 -> 4937 paints the 0x1e-view
element -> 67e3 sets d549=0x1e), overruns the phase table, hangs -- while the real game stays in the cockpit
(chain A).  The 4937/0x1e-view element is in the port's AZER2 display list (+ all "2+" missions) but NOT
AZER1/SAUDI1's (AZER1 never calls 4937).  FIX DIRECTION: find why the port composes/selects the 0x1e-view for
"2+" missions at the spawn frame (a base-loss in the display-list build or the d548/view-select) so AZER2
renders chain A.  This is NO LONGER oracle-blocked -- the oracle CONFIRMED the target (cockpit).  NEXT: trace
what adds the 4937 element / sets d548==0 for AZER2 vs AZER1 (the display-list composition or the a84c-vs-67e3
view path).  Once AZER2 renders chain-A cockpit, verify vs the genuine azer2 ref (AE=0) + unblock the "2+"
missions.  This reframes twin-#3 from "cull oracle needed" to "view-select base-loss" -- a bounded engine fix.
