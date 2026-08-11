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

## STMP serializer RESOLVED (2026-08-10, gate-hold static RE) -- plant-tree round-trip IS deterministic
STMP writer = FUN_0000_d797 (asm: mov cx,0x108; mov dx,0x9332; mov bx,[DGROUP:0x1690]=fh; ah=0x40;
int 21h) -- a FIXED 264-byte (0x108) block at DGROUP:0x9332, written wholesale. It is load-into-place
(d501) / save-from-place, NOT a per-tree enumeration (264B can't hold 50 full tree records). So STMP is
a COMPACT placement block (grid/stamp/index); the object records (type-0x15 in 0x9fbc, with the RNG
height/phase 9bef seeds at obj+0x10/+0x12) are DERIVED from STMP on load -- transient render/sim state,
regenerated each load exactly like the patch-361 data+2 recompute (d81e). => a plant-tree load->save
round-trip is DETERMINISTIC: STMP stores placement only, the RNG fields never serialize. No RNG-pin needed.
No named DAT_2000_9332 accessors exist in the decomp => 0x9332 is populated via pointer/bulk-copy.
ONE open (needs runtime or deeper asm): does 9c1c (tree add) write 0x9332 in place, or is there a
save-time STMP-REBUILD pass that reads the tree list -> 0x9332? Determine by: (a) FIST_EDIT hook drives
4f32->9c1c with a cursor coord, (b) dump the 264B at DGROUP:0x9332 before vs after + after save, (c) if
9c1c alone doesn't change 0x9332, trace the d5f9 save path for a pre-d797 STMP builder that walks the
530a tree list. plant-tree flow = drive 4f32->9c1c + reuse patch-360/361 save harness + byte-diff the
STMP chunk (offset known: it is the 264B written by d797). place-target (4f5d->bd90) = exact twin.
FULLY SPECIFIED now except the 0x9332-update site, which is a single runtime dump (gate-serialized).

## EDITOR-TOOL HANDLER -> OBJECT-TYPE -> CHUNK MAP (2026-08-10, gate-hold static RE, all 6 handlers reversed)
All six activate-handlers reversed from re_out/fist_decomp.c. Each gates on (param_1 & 10) = activate mask,
reads cursor coord via FUN_0000_3f3c, inserts, posts event (4b86/4b8e), status msg via FUN_1000_66cd.
| handler | insert | obj type | tool (inferred)        | chunk / notes |
| 4f32    | 9c1c   | 0x15     | plant-tree             | STMP (d797, fixed 264B @0x9332); msg 0xd32 ok/0xd63 fail |
| 4f5d    | bd90   | (tree twin)| place-target         | exact twin of 4f32; msg 0xd4a/0xd7b |
| 4e09    | grid+c2fb| 0x17   | add-waypoint (path)    | free grid placement (uVar3 % 3c02, /col<9); type-0x17=path node; PATH chunk (d788) |
| 4eca    | b1df(0x19)/b4fb | 0x19 | lay-minefield/artillery | 2 branches on obj[+2]&0x3000: move (b4dd+b4fb, msg 0xd95/0xda5) or NEW type-0x19 (b1df); coord->obj[+4]/[+8] |
| 4dcc    | 3d2a[type]| unit-attached | per-unit tool     | gates on DAT_2000_3ae0(selected unit)!=0 + obj[+0x16]&8; indexes &DAT_2000_3d2a[obj[+0x1b]], count<0x20 |
| 4c7a    | 502f+462e+077e | unit | place-unit (pos+facing) | DCBS; DAT_2000_3a20/22 drag coords, 077e=facing angle, 60a0 blit, 502f take-command |
COMMON coord source: DAT_2000_3b94 (worldX dword) / DAT_2000_3b98 (worldY dword), set by 3f3c from cursor.
IMPLICATION for verify flows: each tool's round-trip byte-diffs a DIFFERENT chunk -- tree=STMP, waypoint=
PATH, mine/artillery=type-0x19 chunk (locate its writer among d76a PINF/d779 BINF), unit=DCBS (already
round-tripped by editor-add-tank/editor-fsg-roundtrip). CHEAPEST remaining after plant-tree: add-waypoint
(4e09->PATH d788, PATH is a fixed save-from-place writer like STMP). Each needs a FIST_EDIT hook posting
the activate event to the handler + a set cursor coord, then reuse the patch-360/361 save + chunk-diff
harness. All gate-serialized (need runtime). This completes the editor-tool STATIC map (was the 0/6 gap's
biggest unknown).

## plant-tree IMPLEMENTATION SCOPED (2026-08-11) -- 9bef/b1df DONE; 9c1c asm-decoded; obj-alloc is the open piece
Re-examined post-391.  GOOD NEWS: FUN_0000_9bef (patch 227) + FUN_1000_b1df (patch 258) are ALREADY
reconstructed + correct.  So plant-tree needs only:
1. **FUN_0000_9c1c reconstruction** (asm 0x9c1c-0x9c1b, decoded): inputs di=obj(param_2 near offset),
   bx=coord(param_1 low16 = &DAT_2000_3b94).  Ghidra bugs: (a) folded b1df's CF to a DEAD `if(!bVar2)` so
   the tree-add body NEVER runs; (b) coord base-lost as `*unaff_CS`/`unaff_CS[1]` (really dword[bx]/dword[bx+4]);
   (c) 530a(WORD@0x930a)/530c(byte@0x930c) DGROUP vars.  Reconstruction:
     if (DAT_2000_530a < 0x32) { b1df(0x15, obj);            // register; sets g_fist_cf on full
        if(!g_fist_cf){ dg[obj+0x19]=dg[0x930c];              // species 530c
           *(u32*)(dg+obj+4)=*(u32*)(dg+coord);              // cursor X (dword[bx])
           *(u32*)(dg+obj+8)=*(u32*)(dg+coord+4);            // cursor Y (dword[bx+4])
           9bef(obj,obj); DAT_2000_530a++; g_fist_cf=0; return; } }
     g_fist_cf=1; return;                                     // stc: full/fail
2. **OPEN: the obj allocation** -- 9c1c's di=obj is passed IN (b1df REGISTERS an existing obj, does not
   allocate).  4f32 (activate handler) must allocate/provide the obj before 9c1c.  Trace 4f32 (asm 0x4f32)
   to see where the tree obj comes from (a scratch template? b21d slot?) -- this is the ONE unknown before a
   FIST_EDIT_ADDTREE hook can drive it.  The hook (mirror patch-362's FIST_EDIT_ADDTANK @ build/fist.c:14049)
   sets a synthetic coord in DAT_2000_3b94/3b98, drives 4f32 (or the obj-alloc + 9c1c directly), then d5f9
   serialize + exit.
3. **STMP round-trip harness**: 9c1c does NOT write the STMP block (0x9332/d797) -- the save REBUILDS STMP
   from the 530a tree list.  So after add: d5f9 save -> the .FSG STMP chunk grows; a python stmp_count()
   (chunk-size delta) + reload + idempotent + native==wasm (reuse the patch-360/361 editor harness).
NOTE: do NOT commit the 9c1c reconstruction ALONE -- it is reached by NO current verify flow, so it can only
be validated WITH the FIST_EDIT_ADDTREE flow (Tests sind Spezifikation).  Implement 9c1c + obj-alloc + hook +
STMP-diff together, test end-to-end, THEN commit as a verified feature + add the flow + re-gate.  This is the
cheapest UNBLOCKED DoD deliverable (editor axis; no DOSBox mission oracle needed -- STMP is a file byte-diff).

## plant-tree OBJ-ALLOC + STMP-SOURCE resolved-as-open (2026-08-11)
4f32 asm: activate handler (param_1 & 0xa), calls 3f3c (click->coord @DGROUP:0x7b94=DAT_2000_3b94) then 9c1c;
does NOT set di -> di (the tree obj) is INHERITED from the 209e/4aaa map-click dispatch context.  b1df asm
(0x1b1df) confirmed: it REGISTERS the passed-in di into the 0x9fbc registry (stores di, refcount++, zeroes
body [di+4..]) -- it does NOT allocate di.  So a FIST_EDIT_ADDTREE harness must PROVIDE a valid tree obj slot
(di) + coord + call 9c1c.  BUT the deeper OPEN (from ATTEMPT-1, still unresolved): registering a type-0x15
obj in 0x9fbc serializes to **DCBS (+61B), NOT STMP** -- so the .FSG STMP writer reads trees from a SEPARATE
structure (enumerated by 530a), not 0x9fbc.  => the true open piece is TRACING the .FSG save serializer's STMP
writer (find where it reads the 530a trees; the d797 264B block @0x9332 is written wholesale from that source).
NEXT SESSION: (1) trace the save path (d5f9/the pre-d797 STMP builder) to find the tree source array; (2) add
a tree to THAT array (via 9c1c if it populates it, or directly); (3) STMP byte-diff harness.  CONCLUSION: all
three DoD axes (missions/editor/save-load) now have a precisely-scoped OPEN requiring a fresh multi-step effort
(mission DOSBox oracle | editor STMP-save-source trace | save-load recon).  This session: 2 committed crash
fixes (390/391) + exhaustive frontier mapping.  re_out pristine 61453e42.

## PLANT-TREE 9c1c DECODED + prereqs CLEARED (2026-08-11, static; authorable, testable post-gate)
The "9bef base-loss must be fixed first (like 386)" prereq is STALE: **FUN_0000_9bef is ALREADY reconstructed
(patch 227)** (per-type object init, di rebased). And **FUN_1000_b1df is reconstructed (patch 258) and ALREADY
threads g_fist_cf** (CF=1 on roster-full via b21d; zeroes obj+4.. before 9c1c writes coords -- asm order OK).
So the ONLY engine gap for plant-tree is FUN_0000_9c1c itself, decoded exactly (image 0x9c1c-0x9c4e):
  cmp word[530a],0x32 ; jae .fail(stc)          ; 530a = stamp count, cap 50
  push bx; mov ax,0x15; lcall b1df; pop bx; jb .fail(stc)   ; alloc gfx slot; CF=fail
  mov al,[530c]; mov [di+0x19],al               ; variant byte (0..3 rotator)
  mov eax,[bx];   mov [di+4],eax                ; COORD dword0  (bx = coord source = caller's &3b94)
  mov eax,[bx+4]; mov [di+8],eax                ; COORD dword1
  call 9bef                                     ; per-type init (di)  [patch 227]
  inc word[530a]; clc; ret                      ; success CF=0
  .fail: stc; ret                               ; CF=1
TWO Ghidra artifacts to fix: (1) `unaff_CS` = the dropped BX = the coord-source DGROUP near-offset (caller
4f32 passes &DAT_2000_3b94 = DGROUP:0x7b94); (2) the `if(!bVar2)` inner block is DEAD (bVar2 stays true) --
the real control flow is `jb .fail` on b1df's CF, else run the body. 9c1c returns CF (g_fist_cf). Reconstruction
(sig kept `(undefined4 param_1=coord/BX, int param_2=obj/DI)`): bx=(uint16)param_1, di=(uint16)param_2; guard
530a>=0x32 -> cf=1; b1df(0x15,di); if(g_fist_cf) cf=1 return; [di+0x19]=byte[930c]; [di+4]=dword[dg+bx];
[di+8]=dword[dg+bx+4]; 9bef(di); 530a++; cf=0. Candidate generator staged in scratch (mk_9c1c.py).
REMAINING for the FLOW (needs runtime, post-gate): a FIST_EDIT_ADDTREE shim hook that seeds a FIXED synthetic
cell into DAT_2000_3b94[0..7] (2 coord dwords) + supplies a fresh obj slot + calls 9c1c (bypassing the
interactive 6015/cursor path, exactly as patch 362 bypasses 4c7a via b21d), then the STMP chunk byte-diff
round-trip (reuse the patch-360/361/362 save+diff harness). STMP chunk EXISTS in AZER1.FSG (editor plan intro)
-> the byte-diff is viable. Verify AZER1 STMP count 530a at load (whether trees pre-exist) with the same hook.

## PLANT-TREE 4f32 decoded + 9c1c arg-mapping CONFIRMED (2026-08-11, static, asm-verified)
Real 4f32 (image 0x4f32-0x4f5c) confirms the staged 9c1c patch is CORRECT:
  test ax,0xa ; je .ret                 ; activate-flag gate (decompile's `param_1 & 10` == 0xa)
  mov ax,[bx+4]; mov bx,[bx+6]          ; 4f32's OWN input: read coord from the cursor/cell struct (bx)
  mov si,0x7b94 ; call 3f3c             ; 3f3c(si=&3b94) computes/fills the 2 coord dwords into 3b94
  mov bx,0x7b94 ; call 9c1c             ; 9c1c(bx=coord=&3b94, di=obj)   <-- CONFIRMS my patch: param_1=BX=0x7b94
  jb .fail ; mov si,0xd32 ; call 66cd ; ret     ; CF=0 -> "TREE PLANTED" (0xd32)
  .fail: mov si,0xd63 ; call 66cd ; ret         ; CF=1 -> fail msg (0xd63)
=> 9c1c's param_1(BX)=coord-source 0x7b94, param_2(DI)=obj -- MATCHES mk_9c1c.py exactly. Staged patch VERIFIED.
SECONDARY 4f32 artifacts (for the INTERACTIVE tool, NOT the round-trip harness):
  (i)  CF-drop: decompile `uVar2=0; if(!uVar2){66cd(0xd32)}` ALWAYS picks OK; real = `if(g_fist_cf)66cd(0xd63)
       else 66cd(0xd32)` (thread 9c1c's returned CF into the message select).
  (ii) input base-loss: the `[bx+4]/[bx+6]` cursor-cell reads + 3f3c's coord compute are base-lost (bx cell).
SCOPE SPLIT for the DoD editor round-trip (create->save->reload->sim byte-identical):
  - HARNESS PATH (minimal, verifies the STMP data mutation): 9c1c (staged) + a FIST_EDIT_ADDTREE shim hook that
    seeds a FIXED synthetic coord into DAT_2000_3b94[0..7] + supplies a fresh obj slot + calls 9c1c directly
    (bypassing 4f32/3f3c/cursor, exactly as patch 362 bypasses 4c7a via b21d) + the STMP byte-diff round-trip
    (reuse patch-360/361/362 save+diff harness).  This is the addable editor-plant-tree FLOW.
  - FULL INTERACTIVE (later fidelity): + 4f32 CF-message + input base-loss + 3f3c coord-compute reconstruction.
All post-gate (needs the binaries).  mk_9c1c.py staged; 9c1c arg-mapping asm-CONFIRMED (no runtime needed to
author it correctly).
