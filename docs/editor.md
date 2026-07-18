# EDITOR — Phase-0 recon + the FIRST ROUND-TRIP DELIVERABLE (BATTLE .FSG save)

## STATUS (2026-07-18) — `.FSG` round-trip GENERALIZED to more battles (patch 380)

The load→save fixed-point round-trip (360/361, originally AZER1-only) is now bit-verified for a
representative set spanning every theatre plus the largest battle file. Patch **380** adds an env-gated
harness override `FIST_FSG_BATTLE=<NAME>` (default OFF → behaviour-neutral) that, right before
`FUN_0000_d501`, overwrites the DGROUP filename buffer (`0x79a9` = `&DAT_2000_39a9`) that both the LOAD
(`d501`) and the re-save (`d5f9`, SI=0x79a9) use — so the single `RT_MOUSE` navigation
(BATTLES→OK→ACCEPT) loads any of the 47 `.FSG`, not just the list's default selection AZER1. No
serializer/parser change was needed: `d6e4`/`d501` generalize as-is. Seven new verify.sh flows
(`editor-fsg-cyprus1`/`india1`/`saudi1`/`syria1`/`ukraine1`/`train1`/`ukraine8`) — one per theatre +
the largest file **UKRAINE8** (19022 B, most chunks/units) — each verified as an idempotent FIXED POINT
(`load(orig)→save=f1`; `load(f1)→save=f2`; `f1==f2`), native AND wasm, native `f1` == wasm `f1` (0-diff).
`bash tools/verify.sh both` = **35/35** (28 prior + 7). `re_out/*.c` pristine
(`61453e42`/`0051cb56`/`75c6d726`/`e6d610c5`); no regression; `armoredfist/` git-clean after (WRITE
flows use fresh `cp -a` scratch datadirs).

## STATUS (2026-07-18) — ADD-TANK edit-op IMPLEMENTED + verified (patch 362); load→save round-trip (360/361)

The SECOND editor DoD deliverable — drive one EDIT op (ADD TANK) → save → verify the delta — is
**landed, deterministic, crash-free, and file-verified on native AND wasm** (verify.sh flow
`editor-add-tank`, the 28th flow; `bash tools/verify.sh both` = 28/28). Like the round-trip it is
DATA-level (no terrain/voxel/render dependency).

**What runs:** `BATTLES → OK → ACCEPT` loads `AZER1.FSG` via `FUN_0000_d501`, then the patch-362
harness hook (env `FIST_EDIT_ADDTANK`, default OFF) (1) enters EDITING mode (`DAT_2000_2dab |= 2`,
the bit2 flag `FUN_0000_540a` toggles), (2) adds ONE tank through the engine's REAL unit machinery,
(3) posts the editor status `"TANK ADDED"` (`FUN_1000_66cd(0xd27)`, msgid = STRSEG:0xd27 = editor
string image 0x2e467; msgid = image_off − 0x2d740), and (4) drives `FUN_0000_d5f9` to save, then
exits. The DCBS unit count goes **80 → 81**; the chunk grows by exactly one 0x37-byte friendly record
+ its 6-byte header (61 B).

**The add is through PROVEN load-path machinery, not a fabricated record:**
- `FUN_1000_b21d` (patch 200) — the REAL roster allocator: reserves a fresh unit slot in the friendly
  pool (DGROUP:0xa022 stride 0x37) or enemy pool (0xc05c stride 0xfb), keyed on the team bit
  `byte[obj−0x19ec]&1`, bumps the roster count/high-water/occupancy bitmap, returns the slot's DGROUP
  near-offset (its DI return, exposed by patch 200);
- the new unit's serializable body is CLONED from an existing loaded friendly (tank) record — a
  genuine, well-formed unit; only the 0x37-byte serializable prefix (exactly what `d6e4` writes and
  `d81e` reads back) matters, and `b21d` has already set slot[0]=obj / slot[2]=roster-idx, so only the
  body `[+4 .. +0x37)` is cloned;
- the new slot is registered into the first free entry of the 182-entry unit registry `DGROUP:0x9fbc`
  (the exact table `c157`/`c164` → `d84e`/`d6e4` walk on save), so it is counted + serialized;
- spatial-index registration (`FUN_0000_43c1` → DGROUP:0x6d3c) is NOT run here — not needed to
  SERIALIZE (`d6e4` walks 0x9fbc), and the RELOAD (`d501→d7e1→d81e→43c1`) does it faithfully.

**HONEST SCOPE (why not the interactive map-click):** the fully-interactive add-tank is the
map-canvas activate handler `FUN_0000_4c7a` (dispatched `4aaa`→tool-table @ DGROUP:0x7c54, tool-slot 8),
which pops the `FUN_0000_64ea` vehicle-type picker and projects the click to a world cell via
`FUN_0000_3f3c` off the map camera-state pointer `DAT_1000_d552`. That path is gated on the
**post-6015 live editor MAP-VIEW** (extender-side `93c0`, §4 — an unreconstructed frontier): the
object descriptor `4c7a` threads as its unit record (register DI, `param_6`) has **no
statically-resolvable origin before 6015**, and `3f3c`'s cell is non-deterministic without the
map-view camera block initialized (traced + confirmed, this iteration). So the harness supplies the
type+body+cell directly, through the same roster/registry/serializer machinery — sub-deliverable 3
(the interactive map-view drive) remains the open frontier.

**THE REFERENCE (DoD option b, honest — a DOSBox original-editor byte-compare is NOT like-for-like,
same limitation as patch 361):** verified as WELL-FORMED + a deterministic FIXED POINT:
- edit→save has exactly **base+1 (81)** DCBS units;
- the edited file **RELOADS** via `d501` (reload keeps 81 units — the added tank is valid);
- the raw edit output is (like an editor-authored file) not itself a fixed point — the loader `d81e`
  canonicalizes each unit's roster-index field slot[2] via `b21d` — **but load→save on the edited file
  is IDEMPOTENT**: `load(edit)→save = file_E2`; `load(file_E2)→save = file_E3`; **`file_E2 == file_E3`**;
- native AND wasm, native (raw edit output) **== wasm 0-diff**, deterministic across reruns.

**Patch 362** — the env-gated `FIST_EDIT_ADDTANK` hook in `4754` (fires after `d501`, before
`FUN_1000_6015`/`e4bb`). `b21d`/`66cd`/`d5f9` asm-verified vs `re_out/fist_dat_image.bin`. The 27
prior flows never set the env → behaviour-neutral. `re_out/*.c` pristine.

**Next editor deliverables:** the interactive map-view drive (§4 extender-side `93c0`) → drive `4c7a`
with real map-clicks + the `64ea` type-popup + `3f3c` camera projection; then the other edit tools
(waypoint / minefield / air-base / artillery); then a DOSBox original-editor oracle for a true
byte-for-byte DoD gate.

---

## STATUS (2026-07-18) — `.FSG` load→save round-trip is IMPLEMENTED + bit-verified (patches 360/361)

The first LEVEL/MISSION-EDITOR DoD deliverable — the BATTLE `.FSG` save round-trip — is **landed,
deterministic, crash-free, and byte-verified on native AND wasm** (verify.sh flow
`editor-fsg-roundtrip`, the 27th flow; `bash tools/verify.sh both` = 27/27). It is DATA-level: no
terrain / voxel / render dependency (§4 windshield is untouched).

**What runs:** `BATTLES → OK → ACCEPT` loads `AZER1.FSG` via `FUN_0000_d501` (crash-free), then the
patch-361 harness hook (env `FIST_FSG_ROUNDTRIP`, default OFF) drives the reconstructed serializer
`FUN_0000_d5f9` right after the load and exits. All 7 chunks (SHDR/DCBS/PATH/STMP/PINF/BINF/TERM) are
written, byte-identical size (10762 B), crash-free.

**THE REFERENCE (resolved with asm evidence — NOT a fixed point of the original file):**
The re-saved `.FSG` differs from the shipped `AZER1.FSG` by **144 bytes**, and this is FAITHFUL, not a
save defect. The LOAD path canonicalizes each unit on the way in:
- `FUN_0000_d81e` (asm `0xd847 pop 0x2(%di)`) reads a unit record but **restores the `data+2` word to
  the value `FUN_1000_bb12` recomputes from `idx/val`, discarding the file's stored `data+2`**;
- `FUN_0000_43c1` (patch 200) + `FUN_0000_c296` (patch 214) set per-unit state flags, the cell-index
  registration, and derived fields;
- SHDR fields `2daf`/`3b84` are runtime mode/sim state.
So an editor-authored `.FSG` is **not itself a fixed point** of load→save. But load→save is
**IDEMPOTENT**: a re-saved file reloads and re-saves byte-identically. The round-trip is therefore
verified as a FIXED POINT — `load(orig)→save = file1`; `load(file1)→save = file2`; **`file1 == file2`,
native AND wasm, native `file1` == wasm `file1` (0-diff)**. The `.FSW`/`.FSE` side files round-trip
trivially: `d651` only creates them if missing (AH=4300 get-attr gate), so existing ones are preserved.

**Patches:**
- **360** — the DCBS unit-chunk serializer `FUN_0000_d6e4` + count pass `d84e` + pre-walk dispatch
  `d740` + unit list-unlink `b40b` + `.FSW/.FSE` filename builder `d638`. All were Ghidra base-lost
  (c164 iterator si/CF folded into host-ptr walks of unused params → the `0x79a9` filename-offset
  SIGSEGV; `d6e4`'s `push ax;push si`/`pop dx;pop cx` mangled into `param_4`/uStack; `b40b`'s
  `byte[di+0x16]` host deref → `0xa038` SIGSEGV). Reconstructed via the c164/`g_fist_cf` idiom (cf.
  d755 patch 169); the per-unit `bcc4`(`b354`) index/val lookup is inlined (shared `b354` drops DX +
  mis-scales its stride under host 4-byte int). The fixed-size writers (SHDR/PATH/STMP/PINF/BINF/TERM)
  were already faithful (load-into-place / save-from-place; static magic+len in the DGROUP image).
- **361** — the env-gated round-trip harness hook in `4754` (fires `d5f9` right after `d501`, pre-sim,
  for a deterministic pristine-post-load re-save; behaviour-neutral, the 26 prior flows never set it).

**Open (faithfulness confirmation, not banked):** a DOSBox-oracle compare of the port's `file1` vs the
ORIGINAL editor's own load→save is deferred — the original saves from the editor game-loop context
(post-`6015`/sim, editing mode), a different save-point than the deterministic pre-`6015` fire used
here, so it is not a like-for-like compare without driving the original editor's SAVE UI. The
serializer AND parser are independently asm-verified vs `re_out/fist_dat_image.bin`, and the fixed
point holds, so `file1` is the faithful load→save output for the pristine post-load state.

**Next editor deliverables:** enter EDITING mode (`53c5→540a`, `2dac` unlock) + drive one edit op
(ADD TANK → save → verify the delta); then the editor map-view render (extender-side `93c0`).

---

# EDITOR — Phase-0 recon (in-engine LEVEL/MISSION EDITOR)

Read-only recon, 2026-07-18. All offsets vs `re_out/fist_dat_image.bin`
(md5 `7f1dc243`, base 0, DGROUP seg 0x1c00). Engine md5s unchanged
(`61453e42`/`0051cb56`/`75c6d726`/`e6d610c5`). No engine/patch change.

The editor is a **BATTLE editor** ("battle generator"): it edits the units, paths,
trees, minefields, targets, air-base and artillery placements of a *battle* over an
existing `.KLC` map, and serialises them to a chunk-based **`.FSG`** file (plus the
`.FSW`/`.FSE` WEST/EAST side variants). It does NOT edit the terrain (heightmap/
colormap) — the map is *selected* ("SELECT MAP TO USE..."), not authored.

---

## 0. Correction to the stale recon offsets

CLAUDE.md / recon.md say the 6 INT-33h editor sites are "FIST.DAT 0x14d37–0x14dc8".
Those are **FIST.DAT file offsets** (with the 5120-byte / 0x1400 MZ header). In the
extracted image they are **0x13937–0x139c8** (0x14d37 − 0x1400 = 0x13937). And they
are **not the editor** — they are the generic mouse-driver primitives:

| image off | AX | INT-33h call | role |
|---|---|---|---|
| 0x13937 | 0 | reset driver | mouse INIT (FUN @ ~0x1392a) |
| 0x1396e | 4 | set cursor pos (after AX=7/8 range) | init |
| 0x13973 | 3 | get button/pos | init |
| 0x1399f | 0 | reset | mouse SHUTDOWN/check (FUN @ ~0x13992) |
| 0x139ba | 0 | reset | shutdown |
| 0x139c8 | 0x14 | swap-interrupt (install event handler) | helper @ 0x139bd |

This is `fist_input.c` territory (the shim IS the mouse driver). The editor tools are
dispatched by the **display-list event system** (cursor pos/buttons → event queue →
element hit-test → activate method), the same machinery as the already-bit-verified
menus/dialogs — NOT directly off these INT-33h sites.

---

## 1. ENTRY POINT — BATTLES → OK → briefing → ACCEPT (pinned)

Main menu item **BATTLES** (menu state `DAT_2000_b78e == 2`, set by `FUN_0000_e934`)
→ `FUN_0000_e714` menu dispatch (`(*(code *)*(short *)(b78e - 0x1886))()`) →
**`FUN_0000_e87a`** (@ 0000:e87a, decomp line 30140). Chain:

1. `e39f()` screen setup.
2. `cb7c(".FSG", …, si=0x4509 "SELECT BATTLE", di=&DAT_2000_39a9)` → SELECT BATTLE
   list dialog (47 `.FSG`). CF (CANCEL) → back to `e714`.  *(bit-verified flow `battles`.)*
3. `7088(0)` → mission BRIEFING modal (`<battle>.FSW` text). CF (CANCEL) → menu.
   *(bit-verified flows `battles-ok` / `battles-cancel-briefing`.)*
4. **ACCEPT (!CF) → the editor/mission build cascade** (e87a lines 30166–30202):
   `e43f()` → set flags `b6ce|=0x22` → `e459()` (LOADING title) →
   **`4754(...)` (builds display list; calls `d501` = `.FSG` LOADER)** → `e006` `d755`
   `9d26` → `459a()` (mission spin) → `1631()` (LOD models) → **`e4bb()` (the in-mission
   / editor GAME LOOP, template 0x2d1d)**.

**The editor is a MODE inside that game loop, not a separate screen.** Mode flag =
`DAT_2000_2dab`:
- bit 1 = active/pause toggle;
- **bit 2 = EDITING enabled** (`FUN_0000_540a` toggles it: `2dab ^= 2`).

`FUN_0000_540a` (@ 540a) is the enable-editor command. It is gated by the **battle-lock**
flag `DAT_2000_2dac` (set from the `.FSG` header at load, `d501` line 28196): if the
battle is locked and not already editing → "BATTLE IS LOCKED!" and refuse. Its button
is `FUN_0000_53c5` (an element-activate method, tool id 0x18).

**BATTLES vs CAMPAIGNS:** BATTLES loads an *editable* `.FSG` (unlocked → editor
available). Campaign missions run the same `e4bb` loop but with the battle locked
(`2dac`), so the editor commands refuse. So **BATTLES is the editor entry**; the
port's ACCEPT cascade (patches 154–290, crash-free per STATE.md §2a) already reaches
`e4bb` — i.e. the editor code path is *entered today*; only the tool UI / edit-mode
drive is unexercised.

State-machine summary:

```
main menu (b78e)
  └─ BATTLES (b78e=2) → e714 → e87a
        ├─ cb7c ".FSG"  → SELECT BATTLE  (CANCEL→menu)   [verified]
        ├─ 7088(0)      → BRIEFING       (CANCEL→menu)   [verified]
        └─ ACCEPT → e43f/e459/4754(→d501 LOAD .FSG)/459a/1631/e4bb  [crash-free, UI undriven]
                         e4bb game loop: display list (template 0x2d1d)
                           element-activate → editor tool handlers (2dab&2 gated)
                           53c5→540a  toggle EDITING (2dab^2), lock-gated by 2dac
```

---

## 2. EDITOR UI SCOPE

### Render surface = overhead MAP VIEW (2D), not the voxel windshield
The editor is a **2D overhead map editor**: "MOVE MAP", "CHANGE MAP MAGNIFICATION",
"CENTER MAP ON PLATOON LEADER", "MOVE TOOL PALETTE", place-unit-at-map-coordinate.
Its map backdrop is the tactical MAP VIEW (`93c0`, **extender-side** per
`memory/voxel-render-is-engine-side.md`), NOT the cockpit voxel windshield raycaster
`9200` (the deep-blocked §4 terrain frontier). **The editor does not depend on the
windshield terrain renderer.**

### Tool / command dispatch = display-list element-activate vectors
Editor commands are element-activate methods, installed as `le16` engine offsets in
DGROUP vector tables — same event machinery as the menus. The **tool-palette / command
vector table** is at image **0x206c0** (DGROUP:0x46c0):

```
0x206c0 4aaa 50b1 510c 51cd 521e 52a8 52f2 535e   toolbar/tool buttons
0x206d0 53c5 54a8 5577 56d1 5767 57d6 5a34 5889   edit-toggle + edit-tool buttons
0x206e0 598f 57f8 2104 5aeb 5b60 5bd5 5c4a 2104   more (2104 = default/no-op filler)
0x206f0 61b6 61f0 6253 62b3 6313 63e1 2104 2104
0x20700 ccde cd3b cd8f ce37 cf6b cfcc 2104 2104   list-dialog handlers (already patched 142-145)
```

Additional command tables: **0x23c40** (`5612` remove-all-tanks, `567d` clear-battle;
referenced in-code as `[iVar+0x7c46]`) and **0x24730** (`540a` edit-toggle, `571d`
save). Confirmed-mapped editor handlers:

| FUN | role |
|---|---|
| `53c5` | enable-editing button → `540a` (tool id 0x18) |
| `540a` | **toggle EDITING mode** (`2dab^2`), lock-gated by `2dac` |
| `54a8`/`5577`/`56d1` | edit-mode tool buttons (set current tool `DAT_2000_3b9e`=0x1a/0x1b/0x1c, `3b9c^=8`) |
| `55c3` | tool op (unit type lookup `67ca` → `d3ce`) |
| `571d` | **SAVE BATTLE** (→ `d5f9`), gated `2dab&2` |
| `5612`/`5652`/`5666` | REMOVE ALL tanks / trees / minefields (`FUN_1000_b330` by type) |
| `567d` | CLEAR BATTLE (`4779` re-init + repaint) |

Current-tool state = `DAT_2000_3b9e` (tool id). A map click in editing mode dispatches
the map-canvas element's activate → applies the current tool at the clicked map coord,
emitting status via `FUN_1000_66cd(msgid)` ("TANK ADDED", "A TREE HAS BEEN PLANTED",
"MINEFIELD LAYED", "NO UNIT SELECTED!", "CANNOT COMMAND ENEMY UNIT!", …).

### The 8 tools (from the tooltip strings, image 0x2e7f2..)
`TOOL: SELECT AND MOVE UNITS` / `MOVE MAP` / `CREATE NEW TANK` / `ADD WAYPOINT TO
PLATOON PATH` / `CUT UNIT OR PATH POINT` / `LAY MINEFIELD` / `SET LOCATION OF AIR
BASE` / `SET LOCATION OF ARTILLERY`. Plus toolbar: SPECIFY TIME LIMIT, CHANGE MAP
MAGNIFICATION, CHANGE TIME COMPRESSION, TAKE COMMAND OF SELECTED UNIT, SHOW/HIDE
COMPANY STATUS, MOVE TOOL PALETTE, CENTER MAP ON PLATOON LEADER, REVIEW BA…, and the
"planted a tree" / "placed a target" ops.

### Full editor string set (image ~0x2e33a..0x2e900, seg 0x2000, beyond DGROUP reach)
Modes: `IN ACTION` / `PAUSED` / `EDITING` / `MANUAL`. Ops: `TANK ADDED`, `A TREE HAS
BEEN PLANTED`, `A TARGET HAS BEEN PLACED`, `MINEFIELD LAYED`, `AI IS OFF`, `MAP
SELECTED: ALL TANKS REMOVED`, `ALL TREES REMOVED`, `ALL MINEFIELDS REMOVED`, `BATTLE
COMPLETELY CLEARED`, `ALL PLATOONS FULLY POPULATED!`. Errors: `BATTLE IS LOCKED!`,
`ERROR SAVING BATTLE!`, `NO UNIT SELECTED!`, `CANNOT COMMAND ENEMY UNIT!`, `UNABLE TO
PLANT A TREE!`, `UNABLE TO PLACE A TARGET!`, `UNABLE TO LAY A MINEFIELD!`. Confirm
dialogs: `ARE YOU SURE YOU WANT TO REMOVE ALL UNITS?` / `…LOCK THIS BATTLE?` / `…ABORT
THIS BATTLE?` / `…CREATE A NEW BATTLE?` / `BATTLE ALREADY EXISTS! …REPLACE IT?`. File:
`SELECT MAP TO USE...`, `SAVE BATTLE`, `ENTER FILENAME FOR BATTLE...`, `EXIT TO MAIN MENU`.

(NB: these strings sit at seg-0x2000 offsets ≥0xe000 = DGROUP offset >0xffff, so they
are unreachable from DS=0x1c00 and are addressed via the `66cd` message subsystem, not
a fixed DS immediate — that is why a naive `mov reg,off` grep finds no xref.)

---

## 3. SAVE / LOAD — the round-trip (fully present, chunk-based)

### SAVE `.FSG` = `FUN_0000_d5f9` (@ d5f9), called from `571d` (SAVE handler)
1. INT 21h **AH=3C** create (image 0xd605), filename ptr `0x7b22` (DGROUP), DS=0x1c00.
2. Header chunk `d683`: copies battle state (`DAT_2000_2dac` lock, `3b84 3b88 3a38
   3a3c 543c 5440 5444 5448` = map ref / time-limit / magnification / etc, `2daf`)
   into a struct `DAT_2000_a982..a9a4` and writes **0x3c=60 bytes** (AH=40 from
   0xe97c) — chunk tag `SHDR`.
3. Data chunk writers (each INT 21h AH=40): `d6e4`, `d788`, `d797`, `d76a`, `d779`,
   `d7a6` — units, platoon paths/waypoints, trees, minefields, targets, air-base/
   artillery.
4. INT 21h **AH=3E** close.
5. `d638`: appends `'W'`, `'E'` as the final filename char and calls `d651` twice →
   writes the **`.FSW` / `.FSE`** side-briefing variants (AH=4300 get-attr, AH=3C
   create, AH=40 write 0x18 bytes from 0xe9c8, AH=3E close).

### LOAD `.FSG` = `FUN_0000_d501` (@ d501), called from `4754` (ACCEPT cascade) + `476a`
- INT 21h **AH=3D00** open (filename `&DAT_2000_39a9` / `0x79b9`).
- Reads 6-byte chunk headers; per chunk, matches a **7-entry dispatch table
  `DAT_2000_a9e6`** (image 0x2a9e6, stride 3 words = `[magic_lo, magic_hi, handler]`):

| chunk tag | handler | content |
|---|---|---|
| `SHDR` | `d7b5` | header |
| `DCBS` | `d7e1` | (unit database) |
| `TERM` | `d7da` | (terrain features) |
| `PATH` | `d87f` | platoon paths / waypoints |
| `STMP` | `d8e9` | stamps (trees/targets) |
| `PINF` | `d8a9` | platoon info |
| `BINF` | `d8d3` | battle info |

- Unknown chunks are skipped via INT 21h AH=4201 seek. On EOF (AH=3E close) it
  commits the header fields back to the battle-state DGROUP vars (`2dac`, `3b84`, …).

### Round-trip is DATA-LEVEL and byte-verifiable
The `.FSG` is a deterministic serialisation of the battle-state DGROUP structures; the
`.KLC` terrain map is *referenced by the header*, not written. So the DoD round-trip
(create/edit → save → reload → sim byte-identical) is verifiable by **byte-comparing
the saved `.FSG`/`.FSW`/`.FSE` against the original editor's output** — no render, and
crucially **no windshield voxel terrain** needed. `d5f9`/`d501` are already on the
executed cascade path (`4754→d501` runs today).

---

## 4. Is the round-trip reachable WITHOUT the deep-blocked terrain?  YES.

The §4 terrain frontier (STATE.md) is the **windshield voxel raycaster `9200`** — the
cockpit ground view. The editor uses the **overhead map view (`93c0`)** and its
deliverable (the file round-trip) is pure data I/O. Neither the SAVE/LOAD nor the
edit-op data mutations touch `9200`. **The editor round-trip is NOT gated on the
terrain frontier.** Only a *pixel-perfect map-view render* of the editor would need
the extender-side `93c0`, and that is a separate, later (visual) deliverable, not the
round-trip DoD gate.

---

## 5. RECOMMENDED first implementation step + tractability ranking

Sub-deliverables ranked by value × tractability:

1. **`.FSG` load→save round-trip byte-verify (DATA-level) — HIGH value, HIGH tractability.**
   `d501` (LOAD) and `d5f9` (SAVE) are complete in the decompile and already on the
   ACCEPT path. First step: a harness flow that (a) loads AZER1.FSG via the cascade,
   (b) immediately re-saves it to a scratch datadir (drive `571d`/`d5f9`), (c)
   byte-compares the re-saved `.FSG`/`.FSW`/`.FSE` against the original. This
   exercises the entire serialiser + parser and banks the round-trip's data core WITH
   NO render dependency — the same WRITE-isolation pattern as flows 21–24
   (`selplayer-ok`). Reconstruct the base-losses in `d501`/`d5f9`/`d683`/chunk writers
   as they execute (the INT-21 reg-file `uRam000f00xx` block + the host-ptr-in-DX
   class, patch-077/146).

2. **Enter EDITING mode + drive one edit op (ADD TANK) → save → verify — HIGH value,
   MEDIUM tractability.** Drive a synthetic click (FIST_MOUSE) onto the enable-edit
   button (`53c5→540a`, needs `2dac` unlocked), select the CREATE-TANK tool, click a
   map coord (→ "TANK ADDED", a data mutation), save, and byte-compare the delta
   against the original editor's same-edit output. Depends on (1) + the tool-palette
   display-list template being built in `e4bb`.

3. **Editor map-view VISUAL bit-verify — HIGH value, LOW tractability.** Requires the
   extender-side map view `93c0` render to be reconstructed (its own frontier). Defer;
   the round-trip DoD does not need it.

**Do first: sub-deliverable 1.** It is the cleanest tractable slice of a first-class
DoD deliverable, reuses the proven WRITE-isolation harness, and is independent of both
the cockpit residual (§2b) and the windshield terrain block (§4).

---

## Evidence index (all vs re_out/fist_dat_image.bin / re_out/fist_decomp.c)
- INT-33h mouse primitives: image 0x13937–0x139c8 (mouse init/shutdown, not editor).
- Editor strings: image 0x2e33a–0x2e900 (seg 0x2000).
- Entry: `e714`→`e87a` (b78e=2), cascade `4754`→`d501`, loop `e4bb` (template 0x2d1d).
- Mode flag `DAT_2000_2dab` (bit2=EDIT), lock `DAT_2000_2dac`, current tool `DAT_2000_3b9e`.
- Toggle `FUN_0000_540a` / button `53c5`; tool table image 0x206c0 (DGROUP:0x46c0).
- SAVE `FUN_0000_d5f9` (create 0xd605, header `d683`, chunks d6e4/d788/d797/d76a/d779/
  d7a6, close, `.FSW`/`.FSE` via `d638`/`d651`); SAVE handler `571d` (gated 2dab&2).
- LOAD `FUN_0000_d501`; chunk table `DAT_2000_a9e6` (image 0x2a9e6): SHDR/DCBS/TERM/
  PATH/STMP/PINF/BINF.
