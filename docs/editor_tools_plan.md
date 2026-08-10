# Editor interactive-tools coverage plan (DoD — editor as first-class deliverable)

Banked 2026-08-10 (recon on master, read-only). Sibling of `docs/coverage_plan.md`.
**Do not edit `tools/verify.sh` while `tools/consecutive.sh` (the 10× gate) runs.** Apply after 10/10 or reset.

## Premise correction (important)
The recon note "6 INT-33h sites at 0x14d37–0x14dc8 drive the editor tools" is WRONG (docs/editor.md §0
already refutes it). Those `swi(0x33)` sites are the generic mouse-driver primitives (reset/setpos/
getbutton/swap-interrupt) = fist_input.c/mouse init, NOT editor logic. The interactive tools are
display-list **element-activate** handlers — same machinery as the verified menus/dialogs — dispatched
via map-click `FUN_0000_4aaa` (fist.c:13522, vector `obj+0x7c54`).

## Honest current scope
`editor-add-tank` (verify.sh:221, patch 362) verifies ONLY the file-level ADD-TANK data mutation +
serializer round-trip (DCBS 80→81, reload-valid, idempotent fixed point). It does NOT exercise the
interactive tool `FUN_0000_4c7a` (no map click, no vehicle picker 64ea, no 3f3c projection). So:
**one tool is data-verified; ZERO tools are interactively verified.** `.FSG` round-trip = patch 361
(FIST_FSG_ROUNDTRIP); battle-select = patch 380 (FIST_FSG_BATTLE).

## Editor entry
BATTLES(b78e==2)→e714→e87a(fist.c:34101)→cb7c ".FSG"→7088(0) briefing→ACCEPT→4754(fist.c:13216)→
d501 loader→e4bb(fist.c:33712) loop. Editor is a MODE: FUN_0000_540a (fist.c:14665) `2dab ^= 2` (bit2),
gated by lock byte 2dac (msgid 0xcd3 "BATTLE IS LOCKED!"). Button method 53c5 sets tool 3b9e=0x18.

## Tool handlers (the real "6")
Two-stage: palette button sets DAT_2000_3b9e + flips redraw 3b9c^=8; map-canvas click →
4aaa(:13542)→obj+0x7c54 vector→canvas handler→3f3c(:12200) projects click to world cell via the
extender-side map-view camera ptr DAT_1000_d552 (:12207, valid only post-6015).

| Tool | palette btn | handler | mutates→chunk | status msgid |
|---|---|---|---|---|
| CREATE NEW TANK | 5577→55c3(0x1a) :14788 | 4c7a :13706 | roster→DCBS (picker 64ea) | 0xd27 "TANK ADDED" :13757 |
| ADD WAYPOINT / CUT PATH PT | 0x1b/0x1c | 4e09 :13834 | path array param_4[0x3e95]→PATH (ADD if 3b20&8 :13877 else shift-del :13883) | none |
| LAY MINEFIELD | 0x1c | 4eca :13911 | b4dd/b4fb list :13924 | 0xd95/0xda5 |
| PLANT TREES | 57f8(0x17) :15074 | 4f32 :13952 | 9c1c stamp list :13962→STMP | 0xd32/0xd63 |
| PLACE STATIC TARGET | — | 4f5d :13975 | bd90 target list :13985→STMP | 0xd4a/0xd7b |
| SET AIR BASE / ARTILLERY | — | 4dcc :13814 | 3d2a[] coords→BINF/PINF | none |

Registered fist.c:68478-68485. Strings by msgid (image_off = msgid+0x2d740) via FUN_1000_66cd — NOT
DS immediates (strings at seg-0x2000 > 0xffff, unreachable from DS=0x1c00).

## Prioritized worklist (cheapest first — all reuse RT_MOUSE nav :286 + run_addtank fixed-point shape :327-350 + patch-362 in-4754 hook site; verify by .FSG chunk byte-diff, the DoD-appropriate gate)
1. **PLANT TREE** 4f32→9c1c :13962, STMP, `FIST_EDIT_ADDTREE`. Low. Direct clone of 362; has msgid.
2. **PLACE TARGET** 4f5d→bd90 :13985, STMP, `FIST_EDIT_ADDTARGET`. Low. Identical shape.
3. **LAY MINEFIELD** 4eca→b4dd/b4fb :13924, minefield chunk, `FIST_EDIT_ADDMINE`. Low-Med (two-branch &0x3000 :13923).
4. **ADD WAYPOINT / CUT PATH** 4e09 :13834, PATH, `FIST_EDIT_ADDWP`/`…CUTWP`. Med — needs selected platoon (reuse 362 roster walk); NO status msg → .FSG-diff only.
5. **SET AIR BASE / ARTILLERY** 4dcc :13814, BINF/PINF, `FIST_EDIT_SETAIRBASE`/`…ARTILLERY`. Med-High — needs live-selected object (3ae0 / 5889 take-command); NO status msg.

## Reconstruction pattern (all five)
Cheapest = bypass 3f3c/d552 by calling the tool's underlying list mutator with a FIXED synthetic world
cell, exactly as patch 362 bypasses 4c7a by calling b21d directly. Each needs a new FIST_EDIT_* env hook
(patterned on 362/380) in 4754 after d501, + a python chunk-counter analog to dcbs_units() (:317-326).
All carry the dropped-DX/host-ptr-in-DX base-loss risk on their list-mutator args (class 077/146/200) →
asm-verify against re_out/fist_dat_image.bin before banking, as b21d/d5f9 were for 362.

## Deferred
A screen-frame status-text cross-check ("TANK ADDED" render) needs the map-view render 93c0 (the
unreconstructed extender frontier, docs/editor.md §2/§4) → NOT cheap; the .FSG byte-diff is the gate.

## Feasibility confirmed (2026-08-10)
- AZER1.FSG chunk tags present: SHDR/DCBS/PATH/STMP/PINF/BINF/TERM (one each). STMP chunk EXISTS ->
  plant-tree (4f32->9c1c) and place-target (4f5d->bd90) write STMP; a byte-diff harness is viable.
- plant-tree internals: FUN_0000_4f32 (fist.c:13952): `if(param_1&10){ 3f3c(&DAT_2000_3b94); 9c1c(&3b94,param_3);
  66cd(0xd32 ok / 0xd63 fail) }`. FUN_0000_9c1c (fist.c:24660): `if(DAT_2000_530a<0x32){ b1df(0x15,param_2);
  param_2[0x19]=DAT_2000_530c; param_2[4..8]=CS; 9bef(...); 530a++ }`. DAT_2000_530a = tree count (max 50),
  DAT_2000_3b94 = tree scratch descriptor (DGROUP:0x3b94). 3f3c fills world coords from the map-click camera
  DAT_1000_d552 (post-6015, blocked) -> harness supplies a FIXED synthetic cell into 3b94 and calls 9c1c
  directly (exactly as patch 362 bypasses 4c7a via b21d). Verify: FIST_EDIT_ADDTREE hook in 4754 after d501;
  a python stmp_count() (analog dcbs_units) asserts +1 + reload + idempotent fixed point; native==wasm.
- RISK: LOW (env-gated, mission-path only, file-verified; does NOT touch the fragile shared mga blitter).
  Prefer this over the AZER2 mga-2004 mission fix (high regression risk to 3 bit-verified flows).
