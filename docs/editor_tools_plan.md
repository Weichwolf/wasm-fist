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

## Correction (2026-08-10, after reading 9c1c/b1df/9bef/3f3c internals)
The "Low / direct clone of 362" estimate for plant-tree is OPTIMISTIC. Unlike add-tank (clone a unit body
into a b21d slot + register in 9fbc), 9c1c does more: b1df(0x15,obj) allocates a graphics slot via b21d(0x15)
AND registers `obj` into 9fbc AND zeroes obj[2..], then 9bef initializes animation/RNG fields (obj[0x10]=0291
rng, obj[0x14]=0x200, flags obj[0x16]|=0x40 obj[0x17]|=4, obj[0x12]=table[byte[obj+0x19]*2 - 0x6cde]) --
another DGROUP table base-loss at 9bef (-0x6cde/-0x6cd6, DS-relative, same class as patch 386 bd09). And the
coords come from 3f3c reading DAT_1000_d552 (post-6015 map camera, blocked). Faithful harness options:
(a) supply synthetic valid coords into DAT_2000_3b94[0..3] + a fresh obj slot + call 9c1c (but 9bef's table
base-loss must be fixed first, like 386); or (b) if AZER1 STMP is non-empty (530a>0 at load -- UNVERIFIED),
clone an existing tree object + re-register (pure 362 pattern, avoids 9c1c). Next session: check 530a at load
first; if >0, option (b) is the low-risk path. NB 9bef -0x6cde/-0x6cd6 is another bd09-class DGROUP table
base-loss that any tree-render/plant path will hit -> a patch-386-style rebase.

## plant-tree: clone path DE-RISKED (2026-08-10) -> ready to implement
AZER1.FSG STMP chunk (@0x27fa) = 4B tag + 4B header (02 01 10 00) + **32 records x 8 bytes** = {X32,Y32}
world coords (rec0: X=0x000a3d76 Y=0x0010c309). So DAT_2000_530a = 32 at load (< max 0x32=50, room for 18
more). CLONE PATH VIABLE + LOW-RISK (mirrors patch 362 add-tank; avoids 9c1c/9bef base-loss + the d552
coord blocker): FIST_EDIT_ADDTREE hook in 4754 after d501 -> 2dab|=2 (EDIT) -> allocate a new tree object
(b21d(0x15) or clone an existing tree's in-mem object body), set coords from an existing tree, register in
the tree list, DAT_2000_530a++ -> d5f9 serializes -> STMP grows +8 bytes (32->33 records). VERIFY: a python
stmp_count() (chunk-size delta / 8) asserts +1 + reload keeps 33 + idempotent fixed point + native==wasm,
exactly the run_addtank shape. In-mem tree object > 8B (b1df zeroes obj[2..0x33]; 9bef sets obj[0x10] rng /
obj[0x12] table / obj[0x14]=0x200 / flags obj[0x16]|=0x40 obj[0x17]|=4) but only {X,Y} serialize to STMP,
so the clone need only carry a valid body -> clone an existing tree's object (like add-tank cloned a unit).
This is the recommended FIRST editor-tool flow (lower risk than the AZER2 mga mission fix).

## Correction: d5f9 is NOT the serializer (2026-08-10)
FUN_0000_d5f9 (fist.c:32200) = the INT-21h file CREATE/WRITE wrapper (uRam000f0000=CONCAT11(0x3c,..);
uRam000f0014=0x21; fist_int_dispatch()) -- it writes the already-built file buffer to disk. The SAVE path
serializes the 0x9fbc registry: UNITS -> DCBS chunk, TREES -> STMP chunk, discriminated by object TYPE.
b1df registers BOTH units and trees into DAT_2000_9fbc (the shared object registry). So plant-tree via clone
= EXACTLY add-tank's shape: (1) find an existing TREE entry in 0x9fbc [needs the tree TYPE discriminator --
the ONE remaining unknown; add-tank found friendly units by `dg[(uint16)(obj-0x19ec)]&1==0`, trees have a
different marker -- determine by comparing a known tree obj vs a unit obj, or trace which 9fbc entries the
STMP loader in d501 created], (2) b21d(0x15) new slot, (3) clone the tree body, (4) register in 0x9fbc,
(5) DAT_2000_530a++. Then the save serializes it -> STMP 32->33 records (+8B). NEXT SESSION: find the tree
type discriminator (1 analysis step), then patch 387 is a direct add-tank clone. Everything else is specified.

## plant-tree ATTEMPT 1 FAILED + REVERTED (2026-08-10) -- corrected model
Patch 387 (clone a type-0x15 0x9fbc entry via b21d(0x15) + register + 530a++) was WRONG and REVERTED.
Empirical test (FIST_EDIT_ADDTREE harness, STMP-chunk-length + idempotent + native==wasm): the planted
entry grew the **DCBS chunk by +61 bytes (one UNIT record), NOT STMP** -- so registering a type-0x15 obj
in 0x9fbc makes the DCBS serializer emit it as a bogus unit; trees do NOT round-trip to STMP via 0x9fbc.
STMP len stayed 264; idempotent=NO (the bogus DCBS unit is non-canonical). native==wasm=YES (harness is
deterministic). So the model "trees = type-0x15 in 0x9fbc, serialized to STMP by type" is FALSE.
CORRECTED next step: the STMP serializer reads a DEDICATED tree structure (enumerated by DAT_2000_530a=27),
SEPARATE from the 0x9fbc registry. Runtime dump (FIST_DUMP_REG, committed in native_main.c) DID show 27
type-0x15 objects in 0x9fbc -- but those feed the RENDER/sim, not the STMP save. To do plant-tree correctly:
TRACE the save path's STMP writer (find where it enumerates the 530a trees and what array/list it reads --
it is NOT 0x9fbc) then add a tree to THAT structure. The STMP writer is in the .FSG save serializer (the
code that builds the file buffer before d5f9 writes it); find it by the STMP tag emission ('STMP' = the
4 bytes 53 54 4d 50) or by who reads DAT_2000_530a in a serialize context (fist.c 530a readers: 24629/
24668/24676/24726/24765/24775/24785 are the tree add/remove ops, not the serializer -- the serializer is
elsewhere, likely reached from the SAVE path, not 4754). NB place-target (4f5d->bd90) also writes STMP ->
same corrected model applies. FIST_DUMP_REG diagnostic retained (native_main.c, env-gated) for next session.

## plant-tree ATTEMPT-1 OPEN QUESTION RESOLVED (2026-08-10, gate-hold static RE)
The "dedicated tree structure, NOT 0x9fbc" next-step is now pinned by reversing the REAL tree-add
FUN_0000_9c1c (fist.c ~20700). It does: `if (DAT_2000_530a < 0x32) { b1df(0x15,obj); obj[0x19]=530c
(species 0..3); obj[4..7]=cursorX(3b94); obj[8..0xb]=cursorY(3b98); FUN_0000_9bef(coord,obj); 530a++; }`.
So the tree record is type 0x15 (species@+0x19, worldX@+4 dword, worldY@+8 dword) and the KEY link is
**FUN_0000_9bef** -- the tree-LIST linker that ATTEMPT 1 never called. ATTEMPT 1 used the wrong allocator
(b21d not b1df) AND skipped 9bef, so its record went only into 0x9fbc (=> DCBS unit) and never into the
dedicated tree list the STMP serializer enumerates via 530a. CONCLUSION: do NOT hand-clone -- the faithful
plant-tree is to DRIVE the real activate handler 4f32 (mask param_1&10, cursor coord in 3b94/3b98) which
calls 9c1c, then save + STMP byte-diff. place-target (4f5d->bd90) is the exact twin. Base-loss to patch in
9c1c: the coord source is `*unaff_CS`/`unaff_CS[1]` (CS-based) -- should be the param_1-derived pointer
(&DAT_2000_3b94 with the high-word seg); + the dropped success CF (uVar2 always 0) that picks the "added"
vs "failed" status msg (4f32: 0xd32 ok / 0xd63 fail). NEXT (needs runtime, gate-serialized): a FIST_EDIT_
hook that posts the activate event to 4f32 with a set cursor coord, then reuse the patch-360/361 save +
STMP-length/idempotent/native==wasm harness. Reverse 9bef next to confirm the dedicated list head.

## plant-tree RNG dependency (2026-08-10, gate-hold) -- harness implication
FUN_0000_9bef (called by 9c1c) seeds tree fields with RNG FUN_0000_0291(): +0x10=rng (sway phase),
+0x14=0x200, +0x12=(rng & htab[species*2-0x6cde])+hbase[species*2-0x6cd6] (randomized height), sets
flags +0x17|=4 / +0x16|=0x40, then FUN_1000_adcd(species*2) inserts into a spatial index. IMPLICATION:
a planted tree is NOT deterministic from cursor coord+species alone (height/phase are RNG). Before the
plant-tree STMP-diff flow: EITHER pin the RNG seed (find 0291's state var, seed it via a FIST_ hook) OR
confirm the STMP serializer writes only coord(+4/+8)+species(+0x19) and drops the render fields (+0x10/
+0x12/+0x14) -- reverse the STMP writer to decide. Only then is the round-trip byte-diffable. This is the
last unknown before plant-tree is fully specified; everything else (drive 4f32->9c1c) is pinned.
