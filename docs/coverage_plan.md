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

### twin #3 MECHANISM pinned (2026-08-11) -- 6f1f(cockpit) vs 4937(0x1e) first-paint race via d548
Two view-element PAINT handlers, both gated on d548==0 (first-paint-wins), set the view + d548=1:
  - FUN_1000_6f1f (cockpit): d548=1; d552=0x8d60; a84c(2d34) -> d549=0x1c -> CHAIN A (renders).
  - FUN_0000_4937 (0x1e view): d548=1; 67e3() -> d549=0x1e -> CHAIN B (378e render_di=0x6bbe overrun -> hang).
Whichever element the 209e display-list walk PAINTS FIRST wins the view.  AZER1/SAUDI1: 6f1f (cockpit) painted
first -> chain A (the genuine AZER2 confirms cockpit is correct).  AZER2 (+ "2+" missions): 4937 (0x1e) painted
first -> chain B -> hang.  So twin-#3 = the DISPLAY-LIST ELEMENT ORDER/COMPOSITION: AZER2's 0x1e-view element
is walked/painted before its cockpit element (both exist; the real game renders the cockpit, so the 0x1e
element should NOT win at spawn).  FIX (bounded, engine, no oracle): trace the display-list build -- find what
adds the 4937/0x1e-view element + its order vs the 6f1f cockpit element for "2+" missions; a base-loss likely
mis-orders them (or adds a spurious 0x1e element).  Candidates: the 209e walk order (element z/link), or the
element-CREATE that sets paint=4937.  Once the cockpit element paints first (d549=0x1c), AZER2 renders chain A;
verify vs ref/mission_azer2_cockpit_native320.png (genuine, central AE~72 vs azer1).  This unblocks AZER2 +
all "2+"/D31 missions (the dominant mission blocker).  NEXT: grep for what sets an element's paint method to
0x4937 / 0x6f1f + the display-list insert order for the view elements.

### twin #3 refined: the 0x1e view = the MAP VIEW (2026-08-11)
6f1f sets d552=0x8d60 (COCKPIT camera); 67e3 sets d552=map-view camera + d54a from 3b19.  So d549=0x1e =
the MAP/commander view (toggled from the cockpit by a key normally).  => AZER2's port wrongly enters the MAP
VIEW at the spawn frame (d548==0 -> 4937 map-node paints before the 6f1f cockpit-node) instead of the cockpit;
the map-view render (chain B, 378e) overruns the phase table -> hang.  The real game starts in the COCKPIT
(genuine AZER2 = cockpit).  FIX: the cockpit node (6f1f) must win the first-paint race (or the map node must
not be dirtied/painted at spawn).  NEXT: runtime-trace the 209e display-list walk ORDER for AZER2 vs AZER1
(which view-node is painted first + its dirty state) -- a FIST_ diag on the 209e walk / 4937 vs 6f1f entry,
or dump the display-list node link order + dirty flags at the first 459a frame.  The base-loss is likely in
the node dirty-state / link order that makes AZER2 paint the map node first (the "2+" richer roster perturbs
it).  Bounded engine fix; verify vs ref/mission_azer2_cockpit_native320.png.  This is the single highest-
leverage unlock (AZER2 + all "2+"/D31 missions).  re_out pristine 61453e42.

### twin #3 traced to: AZER2 never inits the cockpit view (a84c) -> map-toggle wins (2026-08-11)
FIST_DIAGVIEW trace (diag on 4937/6f1f entry, removed): AZER1 renders the cockpit WITHOUT painting either
view-node (no 4937, no 6f1f) -- a84c/cockpit-init comes from a NON-paint path; AZER2 paints 4937(map-toggle)
with **d549==00** (cockpit view NEVER initialized) + d548==00 -> 67e3 -> map view (d549=0x1e) -> chain B -> hang.
So the ROOT: AZER2 does NOT call a84c (cockpit-view init, sets d549=0x1c) at mission entry, so d549 stays 0 and
the map-toggle node (4937) initializes the MAP view instead of the cockpit.  a84c callers: FUN_1000_6f1f@58605
(a paint path, not used by AZER1 either) + 59739/60852/61766/62363.  AZER1 inits the cockpit via one of those;
AZER2 skips it.  NEXT: find which a84c caller AZER1 uses at mission entry + why AZER2 skips it (a base-loss in
the guard, likely tied to the "2+" roster / a mission-state flag).  Once AZER2 inits d549=0x1c early, the
map-toggle sees d548!=0 -> stays cockpit -> chain A -> renders.  Verify vs ref/mission_azer2_cockpit_native320.png.
The single highest-leverage mission unlock; bounded engine fix (no oracle).  re_out pristine 61453e42.

### twin #3 root narrowed: 6 view-toggle nodes, first-painted wins; AZER2's MAP toggle paints first (2026-08-11)
The view is set by 6 VIEW-TOGGLE paint nodes (on-screen camera buttons), each `if(d548==0){ d548=1; d552=<cam>;
<viewinit>() }`:  5 COCKPIT: 6f1f(0x8d60/a84c), 59739(0x8e91), 60852(0x8f8c), 61766(0x9118), 62363(0x91fe) ->
d549=0x1c chain A;  1 MAP: 4937 -> 67e3 d549=0x1e chain B.  FIRST-PAINTED (d548==0) wins the view.  AZER1's
first-painted toggle is a COCKPIT node (renders); AZER2's is the MAP node 4937 (d549==00 confirmed = no toggle
before it) -> chain B -> hang.  So twin-#3 = the 209e display-list WALK ORDER / DIRTY-STATE of the 6 toggle
nodes: AZER2's map toggle is walked/dirtied before its cockpit toggles.  NEXT: diag all 6 nodes' paint order +
dirty flags for AZER1 vs AZER2 (which node first, why) -> the base-loss (a node link/dirty mis-order for the
"2+" roster, or the map toggle wrongly dirtied at spawn).  Fix so a cockpit toggle paints first (or the view is
pre-set d549=0x1c before any toggle).  Bounded engine fix; verify vs ref/mission_azer2_cockpit_native320.png.

### twin #3 ROOT PINNED (2026-08-11): AZER2's active/starting VIEW ELEMENT = map (should be cockpit)
FIST_DIAGVIEW (all 6 view-toggle nodes instrumented, removed): AZER1's FIRST-painted view element = COCKPIT
795c (cam 0x8e91) -> d549=0x1c chain A -> renders.  AZER2's FIRST-painted = MAP 4937 -> d549=0x1e chain B ->
hang.  Both d549==0 before (no pre-set), so the FIRST-painted VIEW ELEMENT wins.  => the ROOT is the
ACTIVE/STARTING view-element SELECTION: the 209e walk paints AZER2's MAP element first (it is the active/dirty
one at spawn), but the real AZER2 starts in the COCKPIT (genuine ref).  So AZER2's active-view element is
wrongly the map (4937) instead of a cockpit (795c-class).  FIX: find what SETS the active/starting view element
(dirties one of the 6 at mission entry) -- a view-index or the display-list element the mission-setup activates.
For AZER2 (+ "2+" missions) it selects the map; a base-loss (mission-state/roster-tied) mis-selects it.  NEXT:
grep the mission setup (e4bb/d501/459a-entry) for what activates/dirties a view element (795c/4937 element
create or a view-index write); trace why AZER2 -> map, AZER1 -> 795c cockpit.  Fix so the cockpit element is
active at spawn -> AZER2 renders chain A -> verify vs ref/mission_azer2_cockpit_native320.png.  Bounded engine
fix, no oracle.  THE highest-leverage unlock (AZER2 + all "2+"/D31 missions).  re_out pristine 61453e42.

### twin #3 diagnostics (2026-08-11): the fix is NOT a 4937 gate -- it's the display-list active-view composition
Tested two env-gated diagnostics in 4937 (both removed): (a) FIST_NOMAP (skip 67e3 map-init) -> AZER2 STILL
hangs (d549 stays 0 -> the 2322 re-seed uses word[0x4a88+0]=0x4aae -> a different loop); (b) FIST_FORCECOCKPIT
(set d548=1,d552=0x8d60,a84c in 4937 like 6f1f) -> AZER2 STILL hangs (a84c alone is INCOMPLETE -- 6f1f also
runs the `if(4d0e==2){6e75; d55a=4e1c ...}` camera-position setup; a partial force doesn't render).  CONCLUSION:
twin-#3 is NOT fixable by gating 4937 -- the real fix is making AZER2's display list ACTIVATE the COCKPIT view
element (the full 795c/6f1f path with its camera setup) at spawn instead of the map element (4937).  So the root
is firmly the DISPLAY-LIST active-view-element COMPOSITION/selection: AZER2's active view element is the map;
it must be a cockpit.  NEXT: trace the mission-cockpit display-list BUILD -- how the view elements are inserted
+ which is made active/dirty at spawn (the template copy / the 209e element link + dirty for the view container);
find the base-loss that makes AZER2 (+ "2+" missions) activate the map element.  A deeper display-list-composition
investigation (bounded, engine, no oracle).  re_out pristine 61453e42.  THE highest-leverage mission unlock.

### twin #3 -- view handlers are RUNTIME method-vectors, not a static table (2026-08-11)
Image scan: the 6 view-handler offsets do NOT cluster as a static dispatch table (6f1f/8cbf/93f3 = 0
occurrences; 795c/8390/4937 scattered singletons).  => the view-node paint methods are RUNTIME-INSTALLED
DGROUP method vectors (reloc sections, like the 0x0a..0x36 / STRSEG method vectors), and the display-list
view NODE references a method SLOT whose current handler = the active view.  22bb resets d548=0 per frame ->
the 209e walk re-paints the view node -> its slot's handler sets the view (d549).  AZER2's view-node slot
holds the MAP handler (4937); AZER1's holds a COCKPIT handler (795c).  So twin-#3's fix is in HOW the view
node's paint slot is selected/installed at mission spawn (the current-view -> method-slot mapping).  This is a
DGROUP method-vector / display-list-node trace (same class as the earlier method-vector reloc work).  NEXT
(focused fresh): find the view node + its paint-method slot; trace what sets it to the map vs cockpit handler
for AZER2 vs AZER1 (a current-view index or a reloc/install base-loss); verify vs ref/mission_azer2_cockpit_native320.png.
This is the practical depth limit reached this session -- twin-#3 root is pinned to the view-node method-slot
selection; the fix is a bounded but deep display-list-node investigation for a focused continuation.

### twin #3 -- EXACT dispatch site pinned: 209e node paint = word[DGROUP:(nd+0x423c)] (2026-08-11)
FUN_0000_209e (the 0x423c dirty-walk, build/fist.c:9088): per node it dispatches
`nd = word[DGROUP:bx]; off = word[DGROUP:(nd+0x423c)]; call fist_icall_near(0,off)`.  So the VIEW node's paint
handler = word[DGROUP:(view_nd + 0x423c)].  AZER2's view node -> that slot = the MAP handler 4937; AZER1's ->
a COCKPIT handler 795c.  So twin-#3's exact fix site = what sets the view node's paint slot (nd+0x423c) / which
NODE is the active view node, for AZER2 vs AZER1.  Two sub-cases to distinguish (one diag): (i) SAME node,
different slot value -> a runtime write / reloc-install base-loss sets word[DGROUP:(nd+0x423c)] to the map
handler for AZER2; (ii) DIFFERENT node (nd) -> the display-list build gives AZER2 a map view node instead of a
cockpit one.  DIAG: at the 4937/795c paint, log word[DGROUP:0x3e08] (=bx, the node the 209e walk published) +
word[DGROUP:bx] (nd) + word[DGROUP:(nd+0x423c)] (the slot) for AZER1 vs AZER2.  Then trace the setter of that
slot / node.  This is the PRECISE fix site (view-node paint-slot selection); the rest is a focused
display-list-node trace.  TWIN-#3 ROOT FULLY LOCALIZED (10 diagnostic layers) -> the highest-leverage mission
unlock, ready for a focused continuation.  verify vs ref/mission_azer2_cockpit_native320.png.  re_out pristine.

### twin #3 LOCALIZED TO ONE WORD (2026-08-11): word[DGROUP:0x3b3c] = view-node content (map for AZER2)
FIST_DIAGNODE (removed): the 209e view node is at bx=0x3b3c for BOTH AZER1 and AZER2 (same node), but its
CONTENT word[DGROUP:0x3b3c] (=nd) DIFFERS: AZER1 nd=0x011a (COCKPIT element, paint 795c), AZER2 nd=0x0060 (MAP
element, paint 4937 @ word[0x0060+0x423c]).  => twin-#3 root = word[DGROUP:0x3b3c] is set to the MAP element
(0x0060) for AZER2, should be a COCKPIT element (0x011a).  No direct C write to 0x3b3c (=g_mem+0x1fb3c) -> the
writer is a POINTER-BASED display-list link (a node->content = element_off write Ghidra rendered via a base
ptr).  FIX SITE FOUND: find what writes word[DGROUP:0x3b3c]=0x0060 for AZER2.  METHOD: a memory WATCHPOINT on
g_mem+0x1fb3c (gdb-to-mission TIMES OUT -> use a shim mprotect-watch on that page, like FIST_FBTRAP, logging
the writer EIP when it stores 0x0060; or -- since it is set EARLY at mission-init -- a coarse bisect).  0x0060
= the map view element, 0x011a = a cockpit view element; both are display-list elements with paint at +0x423c.
Once word[0x3b3c]=cockpit(0x011a) for AZER2 -> chain A -> renders (verify vs ref/mission_azer2_cockpit_native320.png).
TWIN-#3 LOCALIZED TO ONE WORD (11 diagnostic layers) -- the highest-leverage mission unlock, one watchpoint from
the fix.  re_out pristine 61453e42.

### twin #3 -- dynamic watchpoints CONFIRMED impractical; STATIC trace is the path (2026-08-11)
Ran a gdb HW watchpoint on word[DGROUP:0x3b3c] (=g_mem+0x1fb3c) for AZER2, 400s background: gdb reached ONLY
the INTRO (TITLE.KDV) in 400s -- nowhere near the mission (only the boot-time 0-write @main+40 fired).  So
gdb-to-mission is ~orders of magnitude too slow (not a timeout to extend).  A shim mprotect-watch is also
impractical: 0x1fb3c is on a HOT DGROUP page written constantly -> single-stepping every write slows the
engine ~1000x -> never reaches the mission.  => the writer of word[0x3b3c]=0x0060(map) MUST be found by a
STATIC display-list-build trace.  The view node (bx=0x3b3c, stride-0xc 209e node) has content nd=word[0x3b3c]
= the view ELEMENT offset (0x0060 map / 0x011a cockpit); the writer LINKS the view element to the node
(node->content = element, pointer-based, no direct C ref to 0x3b3c).  NEXT (static, focused): trace the
mission-cockpit display-list BUILD (the 1cdb/1e27 template copy + the element create/link at mission entry;
the view container's child = the current view element) -- find where the view node (0x3b3c) content is set to
0x0060 vs 0x011a, and the base-loss that picks the map for AZER2 (+ "2+" missions).  Examine the element structs
at DGROUP:0x0060 (map) + 0x011a (cockpit) + their link.  This is the twin-#3 fix path -- a static engine trace,
no dynamic tooling (both watchpoint routes ruled out).  re_out pristine 61453e42.

### twin #3 -- REFINED mechanism (static source trace, 2026-08-11) + a decoded latent bug
The per-frame view = the FIRST DIRTY-dispatched view-toggle element in the 209e walk.  6f1f (cockpit,
class 0x11a) sets DAT_1000_d548=1 UNCONDITIONALLY on first paint (d548==0) -> it always "wins" if walked
first.  4937 (map) GUARDS itself: `if (DAT_2000_3c08==2 || DAT_2000_3a40!=0) return;` -> it refuses to win
when cockpit-mode.  So map wins for AZER2 iff (3c08!=2 && 3a40==0) AND the map node is dispatched before the
cockpit viewport (or the cockpit viewport is not dirty).

RED HERRING (ruled out this trace): the explicit view-SWITCH machine 3c08/5a34/449c is INERT in our port and
NOT how AZER1 gets its cockpit:
  - DAT_2000_3c08 (mode selector, DGROUP:0x7c08) has NO writer in the engine -> stuck at 0 -> the map guard
    `3c08==2` is ALWAYS false for every mission; the guard reduces to `3a40!=0`.
  - FUN_0000_5a34 (the view-mode jumptable dispatcher) is BASE-LOST + its index is unwritten.  Real asm:
      5a34: mov di,[ds:0x7c08]          ; di = DAT_2000_3c08 (==0, unwritten)
      5a38: jmp [di+0x7c0a]             ; DGROUP-relative vector[di]
    Ghidra emitted `fist_icall_near(0, *(undefined2*)(DAT_2000_3c08 + 0x7c0a))` = deref (di+0x7c0a) as a
    HOST pointer (base-loss) AND di is the unwritten 3c08 value.  The 0x7c0a vector table holds the enter
    handlers 4473/4479(map,5cce,3a40=0) / 449c(cockpit,5cf0,3a40=2) / 44be...  5a34, 449c, 5cf0 are all
    fmap-ONLY (no direct C caller) -> reached only via this (broken) jumptable or a view-switch KEY.
  => 5a34 is a REAL bug but LATENT: no current flow presses the in-mission view-switch key, so it never
     executes.  Fix when a view-switch flow exists: rebase to `word[DGROUP:(uint16_t)(di+0x7c0a)]` via
     fist_icall_near, AND find the writer of DAT_2000_3c08 (the mode selector; likely an aliased/pointer
     write, same no-writer smoking gun -- disassemble the view-key handler).  DOCUMENTED LATENT DEBT, NOT
     PATCHED (untested path -> gate-regression risk).

TRUE mission-start lever (where twin #3 lives): AZER1's cockpit wins because the cockpit VIEWPORT (class
0x11a element, DIRTY method 6f1f, dirtied by the op-0x24 poster 795c) is dirty and dispatched before the
map node.  For AZER2 (D31) the cockpit viewport is either (a) not created in the mission display-list build,
or (b) created but never marked dirty (its op-0x24 poster path 795c/a329/dab0/db02/db11 doesn't run), so the
always-dirty map node wins.  This is a DISPLAY-LIST-BUILD / dirty-bootstrap difference between D31 and D32
missions -- NOT the 3c08/3a40 switch machine.  RESOLUTION PATH (needs runtime, currently blocked): a SHIM
DIAGNOSTIC (env-gated printf in the 209e dispatch site, which is shim-adjacent) dumping, for AZER1 vs AZER2,
the ORDER of view-element classes dispatched + each node's dirty flag -> pinpoints (a) vs (b).  This needs a
build+run to the mission; do it when the /tmp binaries are free (never during a gate).  Both dynamic
watchpoint routes remain ruled out (gdb reaches only the intro in 400s; shim mprotect on the hot DGROUP page
= ~1000x slowdown).  re_out pristine 61453e42.

### mission-sweep INVENTORY (2026-08-11, patch-391 build) -- the *2-7 battle triage
Swept 32 candidate battles (SAUDI/SYRIA/INDIA/CYPRUS 2-7, TRAIN1-4, AZER4-7) for a crash-free cockpit spawn
with central-chrome (MC_REGION 100x92+80+96) AE=0 on both targets vs the existing refs.  All 32 .FSG exist
(no NO-SUCH-BATTLE).  Ref crops: azer1==saudi1 (AE=0 identical); cyprus1 differs from both by AE=90 (a
spawn-frame dynamic-instrument residual -- NOT the map view, which would be AE in the thousands).

ADDABLE (rc=0 + AE=0 both targets) -> LANDED as flows (commit 1334827):
  SAUDI7(saudi1ref)  SYRIA3(cyprus1ref)  INDIA4(cyprus1ref)  CYPRUS7(cyprus1ref)  AZER7(cyprus1ref)

BLOCKED buckets (future criterion-#1 work; each a distinct class):
  - MAP-OR-DIFF rc=0 AE~72-85: SAUDI5 INDIA7 CYPRUS5.  RE-CLASSIFIED: AE~80 (not thousands) => these render
    the COCKPIT with a different dynamic-instrument residual, NOT the map view.  Likely addable with a
    mission-specific genuine DOSBox ref capture (or the residual is a deterministic per-mission instrument
    state -> capture + pin).  CHEAPEST next expansion after the gate.
  - CRASH rc=139: SAUDI4 INDIA2 INDIA3 INDIA5 CYPRUS3.  More base-loss twins (391-class); ASLR-off repro +
    twin-migration per [[aslr-off-deterministic-crash-repro]].
  - HANG rc=124: SAUDI2 SAUDI3 SAUDI6 SYRIA2 SYRIA4 SYRIA5 SYRIA6 SYRIA7 INDIA6 CYPRUS2 CYPRUS4 CYPRUS6
    TRAIN1-4 AZER4 AZER5 AZER6.  The LARGEST bucket -- a timeout, not a crash/map.  Distinct from twin-#3
    (map-select) and from crashes.  Root-cause class UNKNOWN (needs a shim diagnostic: where does the mission
    loop spin?  candidate = the same op-0x1c / cooperative-tick / view-select bootstrap as twin-#3, OR a
    resource-load wait).  AZER2/3 (skipped in the sweep, known twin-#3 map-select) may belong here or in
    map-select -- re-triage with fresh instrumentation.

NB the HANG bucket + twin-#3 map-select likely share a root (the mission-start view/sim bootstrap): a mission
that neither renders the cockpit nor the map and just spins = the cockpit viewport never becomes dirty AND the
map path also stalls.  A single shim diagnostic at the 209e dispatch + the 459a loop gate should separate
"spins with no dirty view" (bootstrap) from "spins in resource load".  Do AFTER the 51-flow gate frees the
binaries (never build/run during a gate).

### KEY SYNTHESIS (2026-08-11): the HANG bucket == twin #3 (one root, ~20 missions)
Cross-referencing the *2-7 sweep buckets against the twin-#3 mechanism (first-dirty view-toggle wins; map
toggle 4937 -> d549=0x1e -> 22dd chain B -> 378e phase-table overrun -> null-dispatch SPIN):
  - ADDABLE (5)      = cockpit / chain A / AE=0.                      [landed]
  - MAP-OR-DIFF (3)  = cockpit / chain A / AE~80 (instrument residual, NOT map -- AE~80 not thousands).
                       cheapest: a mission-specific genuine DOSBox ref.
  - CRASH rc=139 (5) = a base-loss twin BEFORE the view select (391-class); ASLR-off twin-migration.
  - HANG rc=124 (~18)= map-view select -> chain B -> SPIN == TWIN #3's exact failure mode.
So the HANG bucket (SAUDI2/3/6, SYRIA2/4/5/6/7, INDIA6, CYPRUS2/4/6, TRAIN1-4, AZER4/5/6) + AZER2/3 are all
ONE root: "these missions wrongly select the map view (chain B) instead of the cockpit (chain A)".  ~20
missions unblocked by a SINGLE fix => twin #3 is the HIGHEST-LEVERAGE criterion-#1 item, not a one-mission
edge case.  CONFIRM (post-gate, via the 209e-order diag): run it on AZER2 AND a HANG mission (e.g. SYRIA2) --
if both dispatch MAP/4937 and never COCKPIT/6f1f, the synthesis holds and one display-list-create/dirty fix
clears the whole HANG bucket.  (Caveat to verify, not assume: some HANGs could be a resource-load wait, not
the chain-B spin -- the diag's [209e] trace vs a silent hang distinguishes them.)  AZER7 (D31) renders the
cockpit AE=0 -> the map-select is NOT keyed on the D-map number; it's per-mission display-list content.

### twin #3 CONFIRMED by the 209e-order diagnostic (2026-08-11, throwaway diag, re_out pristine)
Ran the FIST_209E 209e-dispatch log on AZER1(default)/AZER2/SYRIA2.  RESULTS (decisive):
- **HANG bucket == twin #3 PROVEN:** AZER2 and SYRIA2 (a HANG-bucket mission) are BYTE-IDENTICAL at the view
  node: `bx=3b3c nd=0060 off=4937 d548=00 d549=00 MAP/4937`.  One root, ~20 missions.
- **Exact mechanism (byte-level):** the view-node slot (walk pos bx=0x3b3c) holds content `nd`:
    AZER1: nd=**0x011a** (COCKPIT viewport), paint off=**0x77dc** -> d548 transitions 00->81, d549=**0x1c**
           (cockpit chain A); AZER1 also dispatches the cockpit cluster off=0x79xx-0x7bxx (7a26/79bf/7b15/
           7903/77dc/7afc/7ae3/7aa5...).  Renders + exits at op-0x24.
    AZER2: nd=**0x0060** (MAP viewport), paint off=**0x4937** -> 67e3 -> d549=**0x1e** (chain B) -> hang;
           dispatches the map/chain-B cluster off=0x50xx-0x5cxx + 4846/4937 (none of AZER1's cockpit cluster).
  Also bx=3b0c: AZER1 nd=0118 off=77f7 (cockpit) vs AZER2 nd=005e off=4846 (map).  So the display-list NODE
  STRUCTURE is the same (same bx slots) but the CONTENT element each node links differs: AZER1 -> cockpit
  elements (0x0118/0x011a), AZER2 -> map elements (0x005e/0x0060).
- The cockpit paint is **0x77dc** (FUN_0000_77dc), NOT 6f1f (6f1f/8390/8cbf/93f3/795c are camera sub-methods).
  The map paint is 0x4937 (FUN_0000_4937).  Element table: paint = word[DGROUP:(nd+0x423c)].
- Genuine DOSBox AZER2 capture (prior session) = COCKPIT, so word[view-node]=0x0060(map) is a BUG; it should
  be 0x011a(cockpit) like AZER1.
=> THE FIX TARGET (next): find where the view-container's content element is set to the MAP viewport (0x0060,
   paint 4937) for AZER2 instead of the COCKPIT viewport (0x011a, paint 77dc).  Both viewport elements exist
   in the element table (nd+0x423c); the divergence is the view-SELECT / display-list-build linking the map
   viewport as the active child for AZER2.  Watchpoints on the node word fail (gdb too slow); next diag =
   instrument the map-viewport element CREATE/attach (paint=4937 install) + the cockpit-viewport (77dc) to see
   which is created/activated per mission and what gates it.  This one fix unblocks AZER2/3 + the ~18-mission
   HANG bucket.  SHARED render path -> after the fix: full 51-flow verify + AZER1/CYPRUS1 AE=0 + re-gate.

### twin #3 selector trace (2026-08-11, source, partial -- next step needs a rebuild-diagnostic)
Traced toward the build-time view selector.  Findings:
- The cockpit viewport paint is FUN_0000_77dc = a THUNK to FUN_1000_795c (the class-0x11a op-0x24 DIRTY
  poster); the map viewport paint is FUN_0000_4937 (-> 67e3, d549=0x1e).  Both are template element paints
  at word[DGROUP:(nd+0x423c)] (cockpit nd=0x011a->slot 0x435a=0x77dc; map nd=0x0060->slot 0x429c=0x4937).
- DAT_2000_2dab (EDIT-mode, bit2) is NOT the selector: it's a save/restore slot (14511 saves 2dab->3ae9,
  14544 restores 2dab=3ae9, 14124 resets 2dab=0 at mission init).  Default is 0 (play) -> 2dab doesn't
  explain AZER2's map view.
- The mission build FUN_0000_4754 = 4779 + d501(.FSG LOAD) + [harness hooks] + loop; the viewport element
  is created during d501/the build from mission/.FSG data.  The selector = which viewport element (cockpit
  0x011a vs map 0x0060) d501 creates/dirties for the mission.  AZER1->cockpit, AZER2/D31->map.
NEXT (needs the binaries free -- do when the gate is not running): a rebuild-diagnostic to resolve (a) vs (b):
dump word[DGROUP:0x435a] (cockpit-viewport paint slot, =0x77dc if the element exists) + word[DGROUP:0x429c]
(map paint slot) at mission-render for AZER2.  If 0x435a==0x77dc -> the cockpit element EXISTS but isn't
dirtied first (fix = the dirty-bootstrap / view-select); if empty -> the cockpit element is NOT CREATED (fix =
d501/build's viewport-create, a base-lost mission-field read for D31/AZER2).  Then trace that create site.
Oracle already banked (AZER2=cockpit, prior-session genuine DOSBox capture) -> no new oracle needed.

### twin #3 ROOT B CONFIRMED (2026-08-11, corrected (a)/(b) diag)
The (a)/(b) view-slot resolver (dump paint[0x423c+nd] when a view element is dispatched): BOTH AZER1 and
AZER2 have cockpit[0x4356]=0x77dc AND map[0x429c]=0x4937 present.  => the cockpit viewport element EXISTS in
AZER2's mission display list (NOT absent).  ROOT B: the divergence is the DIRTY-SELECT at mission init --
AZER1 dirties/dispatches the COCKPIT view element first (77dc wins the d548==0 race -> d549=0x1c chain A);
AZER2 dirties the MAP element first (4937 -> d549=0x1e chain B -> hang).  Both view nodes land at the SAME
walk slot bx=0x3b3c (content word[DGROUP:0x3b3c] = 0x011a cockpit AZER1 / 0x0060 map AZER2).  (NB earlier
"0x435a" was an arithmetic slip: 0x423c+0x11a = 0x4356.)
=> FIX SITE: the mission-init "show initial view" that marks the cockpit vs map view element DIRTY.  Both
elements exist; only the dirty-select differs.  This is a dirty-bootstrap fix (more tractable than a create
base-loss).  NEXT: find the mission-init dirty-mark of the view element (candidate: 6015 / the e4bb loop init
/ a "current view index"); determine the base-lost mission field making AZER2 pick the map view.  Oracle
banked (AZER2=cockpit).  Throwaway diag mk_209e_diag.py (view-slot resolver) staged.

### twin #3 fix-site narrowed to the d501 FSG-chunk view-mode field (2026-08-11, source, race-free boundary)
Build path: e43f->4754(build)->459a(loop). 4754 = 4779 + d501(.FSG LOAD) + [harness] + `if(!CF){6015();39a8=1}`.
The view mode is NOT set in 4754 (6015=object-table setup @DGROUP:0x6d3c, not view).  d501 (FUN_0000_d501,
build 34128-34270) = the .FSG OPEN (INT21 AH=3D on param_4 filename) + a 7-entry CHUNK dispatch table at
DGROUP:0xe9e6 (patch 198; FSG chunks SHDR/DCBS/PATH/STMP/PINF/BINF/TERM).  => the cockpit-vs-map MODE is set
by one of d501's FSG chunk handlers from mission data.  5 missions (AZER1/CYPRUS1/SAUDI1/SYRIA1/INDIA1) parse
it as COCKPIT (correct, AE=0 flows); AZER2 + the HANG bucket parse it as MAP -> the bug is a per-mission
FSG-field read (base-loss) or a genuine field difference our port mishandles.
NEXT (needs binaries free -> when the gate is idle): a rebuild-diag comparing the parsed mode value AZER1 vs
AZER2 -- instrument each of the 7 chunk handlers (or the post-d501 mode state: which view element gets dirtied)
to find where AZER2 diverges.  Then the fix is the base-lost FSG-field read.  ROOT B already banked (both view
elements exist; only the dirty-select differs).  This is the ~20-mission unblock, fully characterized down to
d501's chunk parse.

### twin #3: SHDR word0/a975 RULED OUT as the view selector (2026-08-11, .FSG data diff, race-free)
Diffed AZER1.FSG vs AZER2.FSG SHDR chunks directly: SHDR word0 (-> DAT_2000_a982, read correctly by d7b5;
a975=(a982==0)) = 0(AZER1)/1(AZER2).  a975 gates the PATH(d87f)/PINF(d8a9)/STMP(d8e9) handlers: a975==1 ->
READ the chunk, a975==0 -> SEEK PAST.  CANDIDATE tested by correlating word0 across missions vs cockpit/hang:
  cockpit-OK: AZER1=0 CYPRUS1=0 INDIA1=0 SAUDI7=0 INDIA4=0  BUT ALSO SAUDI1=1 SYRIA1=1 AZER7=1 CYPRUS7=1 SYRIA3=1
  HANG:       AZER2=1 AZER3=1 SYRIA2=1 SYRIA4=1 CYPRUS2=1 UKRAINE1=1  BUT ALSO TRAIN1=0 SAUDI2=0
=> NO clean correlation: SAUDI1/AZER7 (word0=1) render the COCKPIT fine (passing flows); TRAIN1/SAUDI2
(word0=0) hang.  So word0/a975 gates chunk-LOADING but is ORTHOGONAL to the cockpit-vs-map view.  RULED OUT.
The real view selector remains unknown; next step is a RUNTIME diagnostic (instrument what dirties the initial
view element for a HANG mission vs a cockpit one -- e.g. the mission-init "show initial view" / the player-unit
-> view binding), which needs the binaries free (races the gate).  ROOT B stands (both view elements exist;
dirty-select differs).  Data-diff method (read .FSG SHDR fields directly) is the race-free tool for the next
candidate -- but SHDR offset/size fields are file-specific noise, so diff SAME-map missions of OPPOSITE outcome.

### twin #3 FIXED (2026-08-11, commit 6cb54a9, patch 392) -- the store-width ROOT
The elusive selector behind ROOT B: FUN_0000_5d43 (the 2ce7-gated in-mission view-select) gates the cockpit
setup on the player vehicle: `if (3 < *puVeh || flag&1) goto skip; if(!(flag&8)) 44be(cockpit)`.  `*puVeh`
was read as a uint (4 bytes, Ghidra puVeh=uint*) but the asm `cmp WORD [di],3` reads word[+0] only.  AZER2's
vehicle word[+2]=0x0008 -> the 4-byte read 0x00080000 > 3 -> wrong `goto 5d7c` -> 44be SKIPPED -> the view
node word[DGROUP:0x3b3c] stayed the map element 0x0060 (paint 4937->67e3->d549=0x1e -> chain-B phase-table
overrun HANG) instead of cockpit 0x011a (77dc->795c->a84c->d549=0x1c chain A).  Fix (patch 392): WORD compare
+ BYTE [di+0x16] flag tests (matching asm).  AZER2 hang->cockpit crash-free, native<->wasm crop 0-diff, AE=0
vs ref/mission_azer2_cockpit_native320.png; all 7 prior mission flows AE=0 (no regression); verify both=52/52.

METHOD (banked for reuse): a MISSION-INIT FUNCTION-TRACE DIFF cracked it after word0/3a40/mode-init were ruled
out.  Inject env-gated entry/state logs (direct edits to build/fist.c, then `bash tools/build_native.sh` to
compile without re-running `make patch` which would wipe them), run AZER1(cockpit) vs AZER2(hang), diff.
Progression: view-mode handlers (both 5cce, 3a40=0 -- ruled out) -> 209e node visits (bx=3b3c nd=0x011a
cockpit AZER1 / 0x0060 map AZER2, both dirty) -> word[3b3c] timeline (both 0x0060 after 1cdb; diverges in
5cce AFTER 4823 = the 2ce7-gated 5d43->44be) -> 5d43 branch (both pass flags, but AZER2 takes goto 5d7c) ->
4-byte-vs-word dump (word[+2]=0x0008 AZER2 / 0 AZER1) = the store-width bug.  Generators in scratch:
mk_vtrace.py / mk_nodes.py / mk_vn.py / mk_2ce7.py / mk_5d43.py / mk_44be.py / mk392.py.

HANG BUCKET now reaches the cockpit path -> re-sweep (subagent running): ADDABLE (cockpit rc=0 both + AE=0)
land as flows; rc=139 = per-mission downstream twins (389/390/391 class, ASLR-off twin-migration); NEEDS-REF
= cockpit but own instrument residual (genuine DOSBox ref). This one fix is the highest-leverage in-mission
coverage unblock of the project so far.

### CRASH-bucket twins CHARACTERIZED (2026-08-11, parallel-to-gate, no rebuild -> no race)
Repro'd the 6 post-twin-#3 CRASH missions with the EXISTING /tmp/fist_native (setarch -R + FIST_SEGV_BT,
separate datadirs -> safe alongside the running 61-flow gate on a 4-core box).  Two crash sites:
- **5 missions share EIP m_mga_FUN_0000_2b1e+0x190, fault 0x9575000** (SYRIA2/SYRIA4/CYPRUS4/AZER3/UKRAINE1).
  2b1e = patch-312's cockpit-HUD transparency blitter (2ae9/[0x6d8]); works for AZER1/AZER2 but these 5 feed
  it a WILD source: `param_2 = g_mem + (m_260c_recseg<<4) + off` with m_260c_recseg garbage (fault
  0x9575000-g_mem ~= 0x13AFE40 => recseg ~0x13AFE4, wild).  All 5 are M1/type-0/idx0 (d550=0, 3ae0=player)
  like AZER1/AZER2 -> NOT the non-M1 reticle-index debt; a DATA-specific 260c record for these missions.
  ONE fix likely clears all 5.  Next (post-gate, needs rebuild): add a shim/diagnostic dump of m_260c_recseg
  + the 260c record at the 2b1e call for SYRIA2, find why 260c resolves a wild record, rebase.  [[reticle-sprite-dispatcher-debt]] territory but idx0.
- **TRAIN3: EIP FUN_0000_aea8+0x1e, fault 0xc0a2** (reticle d550=0x338, distinct).  aea8 base-loss: param_1
  is a DGROUP NEAR OFFSET (0xc05c) but `*(char*)(param_1+0x46)` derefs it as a host ptr (0xc05c+0x46=0xc0a2).
  Straightforward rebase param_1 -> g_mem+0x1c000+(uint16)param_1 (asm-verify aea8 first; it also reads
  +0x26 and passes param_1 to af1c -> rebase consistently).  Fix needs rebuild (post-gate).
NEEDS-REF (9, cockpit renders, own instrument residual): SAUDI5/6 SYRIA5/7 INDIA6/7 TRAIN1 AZER6 CYPRUS5 ->
genuine DOSBox capture each (post-gate; DOSBox CPU-contends the gate).  POST-GATE ORDER: aea8 (1 mission,
easy) -> 2b1e (5 missions, one fix) -> NEEDS-REF captures (9) -> re-gate.  All characterized in parallel.

### CRASH twins: aea8 FIXED (patch 393) + next-twin map (2026-08-11)
Patch 393 fixed TRAIN3's aea8 base-loss (param_1 near-offset -> host ptr) -- AZER1/CYPRUS1 no regression;
TRAIN3 MIGRATED to the next twin **FUN_0000_8f3a+0x24 (fault 0x8fca)** = the SAME easy base-loss class
(param_2 near-offset; `*(char*)(param_2+0x90/0xca)` -> rebase g_mem+0x1c000+(uint16)param_2).  TRAIN3 is a
CHAIN of quick base-losses (reticle path, d550!=0) -- fix 8f3a then re-repro.
The 5-mission 2b1e crash ROOT: `[0x6d8]` (=m_mga 2b1e blitter) is dispatched by the 5 cockpit CAMERA paints
(6f1f/795c/8390/8cbf/93f3, engine build/fist.c:58647/59782/60890/61804/62400) with a 260c-resolved sprite
record.  For SYRIA2/4/CYPRUS4/AZER3/UKRAINE1 (M1/idx0, like the working AZER1/AZER2) 260c (m_mga, build/
fist_mga.c:6060, patch 114) resolves a WILD record -> 2b1e blits from garbage (fault 0x9575000).  DATA-
specific (the HUD sprite these missions try to blit).  DIAGNOSTIC (post-verify rebuild): dump 260c's inputs
(sprite id + position descriptor) + m_260c_recseg for SYRIA2 vs AZER1 at the 2b1e call -> find why the record
is wild, rebase/guard-free-fix.  [[reticle-sprite-dispatcher-debt]] but idx0 -> a NEW sub-case.
POST-393 PRIORITY (all need the freed binaries): (a) NEEDS-REF captures (9 missions, SUREST +9 -- port already
renders, just genuine DOSBox refs); (b) 2b1e diagnostic (5 missions, uncertain depth); (c) TRAIN3 8f3a-chain
(1 mission, several easy base-losses).  Then re-gate on the grown matrix.

### 2b1e 5-mission crash: ROOT mechanism found (2026-08-11, diagnostic)
Instrumented m_mga 2b1e for AZER1 vs SYRIA2: **AZER1 NEVER calls 2b1e; SYRIA2 does**, with
`fb_di = word[DGROUP:0x724] = 0` -> `fb_seg = word[DGROUP:0] = 0xcbc3` (garbage) -> the blit dest
`g_mem+(fb_seg<<4)+...` is wild -> crash (fault 0x9575000).  word[DGROUP:0x724] = _DAT_1000_c724 = the
fb-descriptor pointer, INSTALLED at fist_mga.c:1087 (`*(uint16*)(g_mem+0x1c724)=off`).  It is 0 when SYRIA2
dispatches 2b1e.  2b1e ([0x6d8]) is dispatched by 6f1f's SECOND-paint block guarded by `4d0e!=2 && 2d34[0xd]&1
&& 4<word[0x2561a]<0x8f` (build/fist.c:58640).  AZER1 SKIPS this block (4d0e==2 full-cockpit, or another cond);
SYRIA2 enters it (4d0e!=2?) -> dispatches 2b1e before the fb descriptor is installed -> crash.  NEXT (deeper):
instrument 6f1f's block conditions (4d0e, 2d34[0xd], word[0x2561a]) + the c724 install timing for SYRIA2 vs
AZER1 -> either (a) 4d0e should be 2 for SYRIA2 (cockpit-init incomplete -> the 44be/72d2 mode path differs),
or (b) the fb descriptor (c724) must be installed before this HUD blit.  Connects to the 4d0e mode flag (5d43->
44be cockpit path).  A multi-step dig -- deferred below the surer NEEDS-REF captures.  Throwaway diag mk_2b1e.py.

## 2b1e 5-mission crash — ROOT-CAUSED to the 22dd cursor-bracket 024f re-dispatch (2026-08-18)

The commit-9600d15 premise "wild m_260c_recseg source" is DEBUNKED: for SYRIA2 the sprite record
recseg=0x4b16 has a VALID header (rows=11 cols=31). The crash is purely a WILD FRAMEBUFFER DESTINATION.

**Mechanism (gdb hardware-watchpoint on DGROUP:0x724, SYRIA2, ASLR-off):**
- 00e8 (VGA mode-set, boot) sets `word[DGROUP:0x724]=0x726` (the fb descriptor; desc[0x726][0]=0xa000).
- In-mission frame 0 of 459a, `FUN_0000_22dd` takes its `if (0 < (int8_t)d548)` branch and runs the
  CURSOR-BRACKET: asm `mov bx,[0x156a]; lcall [0x554]` ... cursor-draw loop ... `lcall [0x554]`.
- `[DGROUP:0x554]` (DAT_1000_c554) = far ptr **0x3e78:0x024f = the MGAVIDEO driver's FUN_0000_024f**
  (fb-descriptor install: `while(*p==0){*p=0xa000;p[2]=0x140;} word[0x724]=param_1; 02c1(...)`).
- Our __allregs dispatch `(*(code*)fist_icall_far(c554))()` DROPS the arg -> 024f reads an
  uninitialised stack slot for param_1 (gdb: 0x9ba9; in the plain crash run: 0) -> `word[0x724]=garbage`.
- Frame 1's paint (459a->206f/209e->77dc->795c->2ae9->2b1e) reads `word[0x724]=0` -> fb_di=0 ->
  fb_seg=word[DGROUP:0]=0xcbc3 -> 2b1e blits to seg 0xcbc3 -> SIGSEGV at 2b1e+0x190.
- **AZER1 never crashes because its d548<=0 at this frame -> it SKIPS the whole cursor-bracket.**
  c554 (->024f) and d56a (=0x156c) are BIT-IDENTICAL AZER1 vs SYRIA2; the sole difference is the branch.

**Why the obvious "thread the asm's BX=d56a" is INSUFFICIENT (do NOT ship it):** the faithful arg is
`BX = word[DGROUP:0x156a] = DAT_1000_d56a = 0x156c`. But `word[DGROUP:0x156c]=0x4b15` (a sprite seg,
NOT 0xa000), so 024f(0x156c): the while-loop is SKIPPED (*p!=0) -> `word[0x724]=0x156c` -> 2b1e
fb_seg=0x4b15 -> STILL a wild write. So either (a) DGROUP:0x554 should not resolve to 024f here, or
(b) the cursor object at DGROUP:0x156c is upstream-wrong (its [0] should be 0xa000, i.e. d56a should be
a fresh fb descriptor), or (c) our patched 024f body diverges from the real driver's.

**NEXT (needs the ORACLE, not improvisation):** DOSBox/QEMU-trace the real game at 22dd's `lcall [0x554]`
for a mission whose cursor is active in-cockpit: capture (1) the real target of [0x554], (2) BX at the
call, (3) `word[0x724]` before/after. That settles whether 024f is the right target and what descriptor
it must install. The whole family SYRIA2/4, CYPRUS4, AZER3, UKRAINE1 shares this one bracket -> one fix.
Repro: `setarch -R env FIST_SEGV_BT=1 FIST_DATADIR=<warm> FIST_TICK_HZ=25000 FIST_FSG_BATTLE=SYRIA2
FIST_MOUSE=<MC> /tmp/fist_native` -> EIP 2b1e+0x190. Watchpoint recipe: SIGSTOP-at-frame-0 pause hook
(raise(SIGSTOP) when word[0x724]==0x726 && fr==0) + gdb `-p` attach + `handle SIGALRM nostop noprint pass`
(the 25kHz timer makes a from-start gdb run never reach the mission).

### SYRIA2 chain state after patches 395+396 (2026-08-18): CRASH -> HANG (twin-#3 class)
2b1e(395) -> ac9e(396) -> now **HANG rc=124** (not a crash).  SYRIA2 reaches the display-list op-loop but
spins on ops 0x08/0x2c/0x5c without ever posting **op-0x24 (present)** -> no MISSFB frame.  Last op before
the freeze: `op 0x2c inbox=00003b3c` -- **0x3b3c = the view-node DGROUP offset** (word[DGROUP:0x3b3c] =
cockpit 0x011a vs map 0x0060; the exact twin-#3 selector, patch 392).  So SYRIA2's hang is a twin-#3-CLASS
view/phase-select residual: patch 392 fixed the 5d43 store-width for ALL missions, but SYRIA2's specific
data evidently still lands on the wrong view chain (like AZER2 pre-392).  AZER1/CYPRUS1 stay AE=0 (they
never reach ac9e/this path).  NEXT (needs runtime, binary must be free): FIST_OPHIST=1 op-trace + the 22dd
per-phase log (the AZER2 twin-#3 method) on SYRIA2 vs AZER1 -> find why word[0x3b3c] stays the map element
for SYRIA2 despite 5d43's WORD compare.  Repro: `setarch -R env FIST_SEGV_BT=1 FIST_DATADIR=<warm>
FIST_TICK_HZ=25000 FIST_FSG_BATTLE=SYRIA2 FIST_MOUSE=<MC> FIST_MISSFB=/tmp/x.ppm timeout 70 /tmp/fist_native`
-> rc=124, last op 0x2c inbox=0x3b3c.  Shared view path -> full regression validate + re-gate after any fix.

### SYRIA2 hang CORRECTED (2026-08-18): NOT the view-select -- it reaches the COCKPIT, hang is downstream.
Instrumented 5d43's gate (word[di]=vehicle type, byte[di+0x16]=flags): **SYRIA2 = word[di]=0x0000,
flags=0x66 -> cockpit-44be**, BYTE-for-BYTE the same decision as AZER1 (veh_off differs 0xc252 vs 0xc05c,
word[di+2]=0x0002 vs 0, but neither poisons the WORD compare).  So the twin-#3 view-select (5d43/392) is
CORRECT for SYRIA2 -- it takes chain A (cockpit).  The hang is DOWNSTREAM in the cockpit op-loop: the engine
re-posts display-list ops 0x08 / 0x2c(inbox=0x3b3c view-node) / 0x5c every frame but never advances to
op-0x24 (present).  Same SHAPE as AZER2's pre-392 op-0c freeze but on chain A, so it's a DIFFERENT root than
twin #3.  NEXT: FIST_OPHIST + the 22dd per-phase log (the [[mission-coverage-plan]] AZER2 twin-#3 method) to
find which chain-A phase handler spins for SYRIA2's roster (op0x54 count likely differs, as it did for AZER2).
Not view-select; do NOT re-chase 5d43. Prior "twin-#3-class view residual" note above is SUPERSEDED.

### SYRIA2 hang PINPOINTED to op-0x58 (2026-08-18, FIST_OPHIST diff vs AZER1).
FIST_OPHIST op-sequence + histogram (setarch -R, warm dd, FIST_FSG_BATTLE):
- AZER1 (renders): FIRST-op sequence ends `op 0x08 (t=538) -> op 0x24 (t=539)` PRESENT -> frame.
  Final histogram: op54=39 op60=16 **op24=1** total=539.  tile3918 nz=65536 (terrain OK).
- SYRIA2 (hangs rc=124): FIRST-op sequence ends `... op 0x2c(t=545) 0x5c(t=546) 0x1c(t=554) **op 0x58
  (t=1003)**` then SPINS FOREVER (total -> 1.74M, no new op, **op24=0** = never presents).
  Final histogram: op54=**55** op60=**6** op24=**0** total>>.  tile3918 nz=65536 (terrain OK).
**op-0x58 is the divergent op**: SYRIA2 posts it (AZER1 never does) and then the display-list op-loop spins
without ever reaching op-0x24 (present).  Roster also differs: op54=55 (vs 39 units) op60=6 (vs 16).  So the
hang is NOT view-select (SYRIA2 correctly reaches cockpit-44be) and NOT terrain (tile nz=65536) -- it is a
cockpit-render/display-list op-loop spin triggered by SYRIA2's larger/different roster reaching an op-0x58
handler that AZER1's roster never triggers.  NEXT (bounded): find op-0x58's handler (ext-service dispatch or
the 22dd phase chain) + why it re-posts without advancing to present; compare the roster entry that emits
op-0x58 on SYRIA2 vs the AZER1 roster.  Same investigative shape as AZER2 twin #3 (op-0c freeze) but a
DIFFERENT op (0x58) and a different (chain-A/cockpit) layer.  Method: FIST_OPHIST + an op-0x58-post backtrace
hook (mirror FIST_OP0C_BT in native_main.c:1236).

### SYRIA2 op-0x58 poster chain + hang nature (2026-08-18, FIST_OP58_BT backtrace).
op-0x58 FIRST-post state: **node[3b3c]=0x011a (cockpit), d548=01 d549=1c (chain A), frame[0x452]=27
(tick ADVANCING -- NOT the AZER2 op0c tick-freeze).**  Poster backtrace: 459a+0x23f -> c0ca (sim step /
flight model, per the FIST_OP0C_BT note) -> c0e5 -> **902c -> ab03 -> b011** (per-object AI/update chain,
0xa000-0xb000 = the ac9e/objtype family) -> aa08(ext) -> e1f0/e21c/e339 (ext gate) -> op post.  op-0x58 is
posted ONCE (n=1); the 1.74M-op spin is the display-list build RE-POSTING 0x08/0x2c/0x5c every frame without
the phase chain ever completing to op-0x24 (present).  So: SYRIA2 correctly reaches the cockpit, the sim
runs (tick advances) and processes its larger roster (op54=55 vs 39) posting an op-0x58 that AZER1's roster
never emits, but the per-frame display-list/phase chain never signals "done" -> no present -> hang.  This is
a phase-COMPLETION spin (twin-#3 SHAPE, chain-A/cockpit layer), triggered by the object at 902c/ab03/b011
that emits op-0x58.  NEXT: instrument 902c/ab03/b011 (the op-0x58 emitter) -- which roster object/type, and
what condition it sets that blocks the frame's present-gate; compare to AZER1 (never posts 0x58).  Diagnostic
FIST_OP58_BT recipe is in the git history (mirror of FIST_OP0C_BT, native_main.c:1236); re-add when resuming.

### SYRIA2 op-0x58 hang ROOT NARROWED to the 795c/d548 present-gate (2026-08-18).
Op-cycle detector (FIST_OPSEQ ring-buffer): the spin is `08 2c 5c ... 1c` repeating; op-0x2c poster chain =
459a->206f->**209e (dirty-walk)->77dc->795c->df0e**->ext (the COCKPIT paint chain, chain A -- SAME as AZER1).
So the 209e dirty-walk re-dispatches the cockpit element (77dc/795c) forever because it never clears dirty.
**FUN_1000_795c state machine (re_out:55979):** top line always re-marks the element dirty
(`*(byte*)(param_4 + word[param_3+2]) = 3`); it only PRESENTS+clears (df0e + `d548=0`) in the
`(d548 & 0x7f)==1 && d548<0` branch, i.e. d548 == **0x81** (bit 7 set).  d548 progression must be
0 -> 1 (795c first call: a84c reticle, d548=1) -> **0x81** (bit 7 set by the phase-complete handler
FUN_0000_23ce `orb $0x80,[0x1548]`, patch-308 note) -> present -> 0.  **For SYRIA2 d548 STAYS 1**
(`if(-1 < (char)d548) return` -> positive 1 returns early, no progress), so the element never presents and
209e loops.  795c is byte-identical for AZER1 (which converges) -> the divergence is UPSTREAM in the d548
phase progression: **the bit-7 setter FUN_0000_23ce (reached via the 22dd/2322 per-frame phase chain) never
fires for SYRIA2's cockpit frame.**  NEXT: trace d548 transitions AZER1 vs SYRIA2 (env-gated log at 795c
entry + at 0x23ce) + the 2322 phase-chain that gates 0x23ce -- find why SYRIA2's chain-A never reaches the
phase-complete that sets d548 bit 7 (roster op54=55 vs 39 likely stalls a per-object phase count).  This is
the twin-#3 phase mechanism at the cockpit/chain-A layer.  Shared by SYRIA2/4 CYPRUS4 AZER3 UKRAINE1.

### SYRIA2 hang FRONTIER mapped: two-chain phase alternation (2026-08-18, 22dd handler trace).
FIST_DIAG22DD (log each 22dd do-while handler: 0a86, word[0a86], c450, d548) AZER1 vs SYRIA2:
- **AZER1 (converges):** ONE phase-chain pass @0x6c82 -> `2322 23ef 41c7 2471 286e 3b59 23ce` (23ce sets
  d548|=0x80 + c450=0 -> loop exits) -> 459a presents -> MISSFB dumped -> done.  7 iterations total.
- **SYRIA2 (hangs):** ALTERNATES two chains forever, never presenting the final frame:
  - d548=0x01 -> chain @0x6c82 (same as AZER1, ends 23ce)
  - d548=0x03 -> chain @**0x6c9a** = `23ea 41df **378e** 23ce`  (378e = the twin-#3 phase-table-overrun
    record writer!)
  Each chain reaches 23ce (d548|=0x80 -> 0x81), but by the next 22dd entry d548 is back to 0x03/0x01 (bit 7
  cleared, bit 1 sometimes set) and 795c never sees d548==0x81 at paint time -> never presents -> 209e
  dirty-walk loops (the 08/2c/5c op spin).
**Chain selector** FUN_0000_2322 tail: `0a86 = word[DGROUP:0x4a88 + d549]` -- d549 (view kind) picks the
chain.  SYRIA2's d549 evidently ALTERNATES (0x1c -> other -> 0x1c) frame-to-frame, pulling in the 378e chain
@6c9a on alternate frames; AZER1's d549 is stable -> single chain -> present.  ROOT QUESTION (next session):
why does SYRIA2's d549 (word[DGROUP:0x1549]) alternate?  Trace d549's writer across frames + what sets d548
bit 1 (0x03).  This is the SAME 378e/phase-table machinery as AZER2 twin #3 (patch 392 fixed the 5d43
view-SELECT, but this is the per-frame phase d549/d548 CYCLING, a deeper layer).  Deep multi-session-class
dig, now precisely mapped.  Method: d548/d549 write-trace (throwaway 22dd + 795c + 23ce logging) AZER1 vs SYRIA2.

### SYRIA2 hang ROOT (2026-08-18, FIST_DIAG2322 -- FULLY mapped): 3 cockpit VIEWPORTS cycling, no terminate.
2322 selector trace (d549 = viewport kind, at each chain-select), node[3b3c]=0x011a (cockpit) throughout:
- **AZER1:** d549=0x1c d548=0x01 -> chain @6c82 -> ONE pass -> present -> done.  Single viewport.
- **SYRIA2:** cycles FOREVER: `0x1c(d548=01,@6c82) -> 0x20(d548=03,@6c9a) -> 0x1c -> 0x22(d548=02,@6ca2)
  -> 0x1c -> 0x20 -> 0x1c -> 0x22 ...`  THREE viewport kinds (0x1c main + 0x20 + 0x22), each its own phase
  chain, never converging to a present.
So SYRIA2's cockpit display list carries TWO EXTRA viewport elements (d549=0x20 and 0x22) that AZER1 lacks
-- additional instrument viewports (e.g. an independent commander/target sight) -- and the multi-viewport
render loop never terminates: each viewport stays dirty (795c-class present needs d548==0x81; the 0x20/0x22
viewports sit at d548=0x03/0x02 -> `-1<(char)d548` early-return, never present, never clear) so 209e keeps
re-dispatching them.  **ROOT QUESTION for next session (two branches):** (a) are the 0x20/0x22 viewports
LEGIT SYRIA2 cockpit elements -> then the multi-viewport TERMINATION/present logic is what's broken (each
viewport's paint method must reach its own d548==0x81 present, or the frame must present after all are
walked); or (b) are they SPURIOUS (a display-list-construction base-loss creating phantom viewport elements
for SYRIA2's larger roster) -> find the element-CREATE that adds them.  METHOD: dump the cockpit display-list
elements (209e walk: each element's word0 class + d549 kind) SYRIA2 vs AZER1 to see if 0x20/0x22 are real
elements; + trace each viewport's paint method + its d548 present-gate.  This is the deepest cockpit-render
layer, now fully mapped from "hang rc=124" down to "3-viewport non-terminating render".  Shared SYRIA2/4
CYPRUS4 AZER3 UKRAINE1.

### SYRIA2 3-viewport root -- REFINED to branch (b) SPURIOUS (2026-08-18).
DECISIVE: SYRIA2's player vehicle is **M1/type-0 (2d34=0xc252, type=0x0000) -- IDENTICAL to AZER1**.  Same
vehicle => same cockpit template (0x2d1d) => it should build AZER1's SINGLE viewport (d549=0x1c).  The extra
d549=0x20 and 0x22 viewport elements are therefore **SPURIOUS**, not legitimate multi-viewport instruments.
So the render loop is fine; the DISPLAY-LIST CONSTRUCTION wrongly adds two phantom viewport elements for
SYRIA2, and each intercepts before the 0x1c chain's present (23ce sets d548=0x81 but the next 2322 picks the
0x20 viewport instead of presenting).  The phantoms correlate with SYRIA2's larger roster (op54=55 vs AZER1's
39), so the cockpit-template/viewport element-CREATE has a base-loss where a roster count or index over-runs
into extra viewport records.  NEXT (next session, sharp target): find the cockpit display-list build that
creates viewport elements (the 0x2d1d template build in the mission-init, 4823/1cdb/1e27 family + the per-
element CREATE), dump the element list SYRIA2 vs AZER1, and locate the base-loss that emits the 0x20/0x22
phantoms.  Fixing it makes SYRIA2 (and SYRIA4/CYPRUS4/AZER3/UKRAINE1) present the single-viewport cockpit
like AZER1 -> +5 missions.  This is the terminal, sharpest characterization of the 2b1e-chain SYRIA2 hang.

### SYRIA2 phantom-viewport methods IDENTIFIED (2026-08-18): a80b (d549=0x22) + a84c (d549=0x1c).
The view/camera-setup methods that set d549 (load d558=angle, d55a/d55e/d562=camera X/Y/Z from an object):
- **FUN_1000_a84c** (build:64595, patch 302): d549=0x1c, camera from the PLAYER vehicle (param_1) -- the
  cockpit main view.  AZER1's ONLY viewport; reached from 795c.
- **FUN_1000_a80b** (build:64550): d549=0x22, d548=2, camera from a DIFFERENT object (param_2 near-offset).
  An EXTRA (non-player) camera view.  SYRIA2 dispatches this (+ the d549=0x20 variant); AZER1 never does.
So SYRIA2's cockpit display list carries extra camera-view elements dispatching a80b (0x22) and a 0x20 setter,
which cycle 0x1c->0x20->0x1c->0x22 without the frame ever presenting.  Since SYRIA2 is M1 == AZER1 (same
player cockpit), these extra object-cameras are the SUSPECT phantoms.  d549=0x20 has NO literal setter in
build/fist.c (only 0x1c@a84c, 0x1e@57930-map, 0x22@a80b) -> the 0x20 comes from a computed store or an
un-decompiled method -> FIND IT (grep the mga/ext units + non-literal d549 writes).  NEXT SESSION (sharpest):
(1) find a80b's and the 0x20-setter's CALLERS (which display-list element/class dispatches them); (2) dump the
209e element list SYRIA2 vs AZER1 to confirm the extra elements; (3) trace their CREATE to the base-loss (or
confirm legit + fix the multi-view termination).  Terminal map of the 2b1e-chain SYRIA2 hang; +5 missions when fixed.

### SYRIA2 -- CORRECTION: a80b viewports are LEGIT (secondary PLAYER views), bug is d548 multi-viewport sequencing.
FUN_1000_7a38 (build:59845, patch 305) = a "sibling viewport DIRTY method (same class as 795c/79f5)" that
calls **a80b(0xf, di_veh)** where di_veh = DAT_2000_2d34 = the PLAYER vehicle.  So d549=0x22 is a SECONDARY
PLAYER viewport (not a foreign-object phantom -- my earlier "spurious" note is SUPERSEDED).  7a38's setup is
gated `if((DAT_2000_2d16 & 1)==0)` and 795c increments 2d16 -> the main (0x1c) and secondary (0x22/0x20)
viewports ALTERNATE by 2d16 parity (a designed multi-pass cockpit render).  The real bug: d548 is a SINGLE
GLOBAL render-phase, and each viewport presents only in its own d548 state (795c: d548==0x81; 7a38: d548==0x82
via the `(d548&0x7f)==2` branch + c694).  SYRIA2 has >1 active viewport so the global d548 can't satisfy all
present-gates in sequence -> no viewport ever completes to op-0x24 -> spin.  AZER1's mission-cockpit flow may
just exit at the FIRST present before its secondary pass.  RESOLUTION needs a 209e element-order + per-element
d548-state dump (AZER1 vs SYRIA2): confirm which viewports are active, and whether the port's global-d548
model mis-sequences the multi-viewport present (vs the original driving each viewport's d548 to its present
state in turn).  This is a real render-architecture question, not a one-line base-loss -> deferred to a focused
session.  The 2b1e/ac9e CRASH fixes (395/396) stand; the SYRIA2 HANG is this multi-viewport sequencing.
