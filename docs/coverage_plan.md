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
