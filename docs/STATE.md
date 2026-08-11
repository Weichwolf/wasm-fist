# STATE — current frontier (2026-08-11, commit 86d02cc)

## MAJOR MILESTONE (2026-08-11): wasm-mission blocker CLEARED + 5 missions dual-target + 46-flow 10x gate
- **wasm-mission tick-hold fix (commit 49f8cfb, shim-only, engine pristine)** resolves the native<->wasm
  hard-invariant divergence: the in-mission INT-8 tick c452 was wall-clock/pump-based, so native (fast) and
  wasm (~15x slower) accumulated different c452 by the spawn frame -> wasm crossed the 459a per-tick sim-gate
  before op-0x24 -> the flight-model sim (op-0x1c) fired -> hang.  Fix: hold the cooperative tick during
  mission-LOAD-pre-cockpit so wasm's spawn-frame tick matches native's frozen count.
- **5 MISSIONS dual-target bit-verified** (was 2): AZER1, CYPRUS1, + SAUDI1/SYRIA1/INDIA1 (commit 86d02cc).
  Central-chrome (MC_REGION) AE=0 both targets vs GENUINE DOSBox refs.  New tool: tools/refcap_saudi.sh
  (scroll-capable, SELF-VALIDATING DOSBox mission-ref capture).  Genuine SAUDI1 ref proves M1 central chrome
  is map-group-invariant (D30==D32).
- **46-FLOW 10x GATE PASSED** (commit 86d02cc) -> criterion #2 re-banked on the grown matrix.
- **THE key remaining mission blocker = twin-#3** (AZER2/D31/"2+" NATIVE chain-B render_di overrun @0x6c96):
  chain B's 378e writes ~8 display-list records at render_di=0x6bbe, overrunning the phase table (chain A
  relocates via 2471, chain B doesn't).  ALL "2+" missions crash/hang on this (SAUDI2/SYRIA2 hang, INDIA2
  crashes -- richer rosters).  Fixing it unblocks AZER2 + dozens of missions.  NEEDS a DOSBox/QEMU chain-B
  mission-state oracle (real object-cull count / render_di) -- no clean cull base-loss in the C path.
- Editor round-trip (plant-tree 9c1c+STMP-source), save/load, controls, full-frame terrain bit-verify, and the
  10x on the COMPLETE matrix all remain.  DoD NOT reached.  Full detail: docs/coverage_plan.md.

---

# STATE — accurate current-state frontier map (2026-07-18; UPDATE 2026-07-20 at top)

Read-only recon snapshot. Supersedes the stale CLAUDE.md `Status` section. Where CLAUDE.md
contradicts the repo, the repo wins — flagged inline.

Engine pristine (unchanged): `re_out/fist.c` **61453e42**, `fist_ext.c` **0051cb56**, `fist_mga.c`
**75c6d726**, `fist_snd.c` **e6d610c5** (evolved via the audio patches from 1e0cfd38).

---

## UPDATE 2026-08-11 — matrix grown 36→**43 flows** + 3 engine fixes; **10× CONSECUTIVE-CLEAN GATE PASSED** (commit `d769482`)

Since the 36/36 snapshot below, the bit-verify matrix grew to **43 flows** and 3 asm-verified engine fixes landed;
`tools/consecutive.sh 10` then ran the full 43-flow suite **10 times in a row with ZERO failures** (`== GATE PASSED
:: 10 consecutive clean full-suite runs (commit d769482) ==`). This banks the DoD's **criterion #2 (10× consecutive
clean) for the CURRENT 43-flow matrix** — NOT the final DoD (which requires 10× on the COMPLETE exhaustive matrix:
44 more missions, 6 editor tools, save/load, controls all still deferred). Every flow is native↔wasm byte-identical
+ AE=0 vs a genuine DOSBox ref, stable across all 10 iterations.

- **Patch 386 (`24e5e0d`)** — bd09-class DGROUP-table base-loss (3 derefs in FUN_0000_bd09 → `g_mem+0x1c000+
  (uint16)(idx-0x61xx)`). Fixed the CYPRUS1 **wasm** OOB → broke open the first non-AZER1 in-mission wasm render.
- **Patch 387 (`49ee2b2`)** — SMOKE-LED store-width: retyped `DAT_2000_4b4f` (0x8b4f state) + `DAT_2000_4bb5`
  (0x8bb5 dirty) → `undefined1`; the WORD dirty-write had clobbered the renderer-index byte → LED never re-rendered.
- **Patch 388 (`fd01bb4`)** — AUTO TURRET LED store-width (0x8b4d/0x8bbe → `undefined1`), asm-faithful; its verify
  FLOW is harness-blocked (cursor/region-crop), deferred — the patch is in the tree and 10×-proven for no-regression.
- **7 new flows**: `mission-cockpit-cyprus1` (first non-AZER1 in-mission bit-verify, region-crop cols80-180 rows96-188)
  + 6 settings toggles (`settings-sound-fx-off`, `settings-joy-{flightstick,tmfcs,ch,tmwcs}`, `settings-smoke-off`).
- Engine `re_out/fist.c` **pristine 61453e42** (all fixes are patches; shim edits only). Docs-only commits since
  d769482 (17139ef/61cd423/ea07d7c/9645615) don't touch the build → the 10× result covers current HEAD.
- **Static prep banked for the next two frontiers** (docs/coverage_plan.md, docs/editor_tools_plan.md): AZER2
  mission mga-2004 blitter (args + 4 rebase artifacts), and the full 6-tool editor map incl. plant-tree fully
  specified (STMP=d797 264B @0x9332, round-trip deterministic).

---

## UPDATE 2026-07-20 (later) — master `c47d366`, patches 383–385 + watchdog shim fix → **verify.sh both = 36/36**

Since the bbd2c81 snapshot below, four fixes landed; the suite is now a COMPLETE clean 36/36 (first full clean
iteration on the fixed codebase — the base the 10× gate runs on). Engine md5s still pristine (fixes are patches +
one shim edit).

- **Watchdog shim fix (commit `7cc9ef8`, `tools/native_main.c`)** — THE real "flaky gate" blocker, not the 26de
  flake. The `FIST_RUNMS` display-flow dump-and-exit watchdog stored an ABSOLUTE epoch-ms deadline (~1.78e12) in a
  32-bit `long` (-m32 / wasm32) → overflowed NEGATIVE for cyclic ~25-day windows → the `>0` guard silently disabled
  it → EVERY FBDUMP display flow hung to `timeout` (rc=124) on BOTH targets while editor/intro/mission (other exit
  seams) passed. A whole broken window looked like "24/36 flaky". Fixed 64-bit. Took the suite 12/36 → 35/36. See
  [[runms-watchdog-32bit-epoch-overflow]].
- **Patch 383 (`0ecfe0c`)** — eliminated the intermittent boot-time f842 far-reloc-applier SIGSEGV (~1-2.6%/boot).
- **Patch 384 (`bf5cdea`)** — fixed the WASM menu-audio blocker: `f7ad`'s dropped template-copy args let the alloc
  land at DGROUP:0 on wasm, clobbering the SOUNDDVR gate signature → no OPL. Threads the asm-correct args. BUT its
  correct 0x3e0 alloc SHIFTS the MEMMGR heap (the real game's behaviour) → exposed a latent mission-render bug →
- **Patch 385 (`c47d366`)** — reconstructed the 3 MISSED 209e cockpit-HUD icon-paint thunks `75a3/7b3d/7cea` (bare
  arg-less c6b4=26a1 blits; Ghidra dropped ax=sprite-id + bx=elem+4; same class as siblings 305/315/316 fixed for
  7b71/75c2/7b57). The heap shift perturbs the ABI-leftover registers they read → garbage sprites over the left
  cockpit panel. Reconstructing all three (asm-verified, 26a1 5-arg) → mission-cockpit AE=0 on BOTH targets,
  native↔wasm 0-diff, crop md5 `355e5bc63da5` = original banked frame, with patch 384 KEPT (audio stays fixed). No
  revert, no band-aid. See [[patch384-mission-cockpit-conflict]].

**Frontier-list correction:** the old #3 "26de-sprite-resolver flake as the must-fix gate blocker" was a
MISDIAGNOSIS — the gate flakiness was the watchdog epoch-overflow (now fixed). The 26de flake is a real but
SEPARATE, lower-frequency debt: a SYMPTOM of 5 un-reconstructed sibling reticle-blit dispatchers
(2660/2758/290c/2a39/29f4) + 3 non-M1 reticle setters (74e3/8463/9497), all BUILD-DEPENDENT and only reachable on
non-M1/idx≠0 missions (NOT the covered AZER1 flow). Full asm spec in [[reticle-sprite-dispatcher-debt]].

**Remaining toward the DoD (unchanged, ranked):** 10×-consecutive-clean gate on the now-clean 36/36 (runnable);
audio bit-exact + a WebAudio `audio-menu` flow; terrain windshield (§4); other missions/maps + the reticle debt;
save/load; controls-config; interactive editor (6 INT-33h tools).

---

## UPDATE 2026-07-20 — master `bbd2c81`, patch 382, **36 verify.sh flows**

Since the 07-18 snapshot below (patch 343 / 26 flows), three big fronts advanced:

**MISSION RENDER — the AZER1 cockpit is the FIRST DUAL-TARGET in-mission surface (bit-verified both targets).**
The mission went from "crashes before render" to native crash-free + cockpit central-chrome bit-exact on
BOTH native+wasm. Arc (patches 364–382):
- **364** wasm op-0x50 spin — turned out to be a WORKAROUND, not a fix (misdiagnosed DAT_2000_3ae0 as a
  host pointer; it's a RAW near offset 0xc05c on both targets). Superseded by 382.
- **365/366** objtype-0x02 per-object update sweep (+ the ae32 param_2 FALSE-GREEN caught and fixed).
- **367** b059 BEARING/MOVEMENT cluster → ACCEPT-COOP mission renders crash-free 15/15 native, sustained
  226714 op-0x24 posts.
- **`+0x3e` camera seed** (native_main.c shim, commit 444f6e4) — the op-0x24 camera bridge missed TCB+0x3e
  (85d0's divisor) → SIGFPE; oracle-anchored 0x3e=256 unblocks the extender render.
- **381** native RETICLE fix — the reticle mis-aimed (position shift from 364's wrong offset), NOT a colour
  bug; restricted 364's subtract to `__EMSCRIPTEN__` → native cockpit AE=0.
- **382** the wasm render divergence ROOT (class-C ABI): display-walk container methods 20d6/2043 read an
  UNPASSED `base` arg — native's nonzero stack leftover worked by luck, wasm zero-fills → base=0 → unbounded
  walk → op-0x50 spin. Fix publishes base via g_fist_paintbp (209e/patch-057 precedent) + drops 381's
  `__EMSCRIPTEN__` gate (both use 0xc05c — band-aid GONE, doctrinally clean) + a missing 85d0 extern.
  → **flow 36 `mission-cockpit`**: BATTLES→OK→ACCEPT → op-0x24 render → crop central-chrome (100x92+80+96,
  cols80-180 rows96-188) → native AE=0 + wasm AE=0 + native↔wasm 0-diff vs `ref/mission_azer1_cockpit_native320.png`.
  REGION-LIMITED (full-frame still 116px in rows0-19 = the terrain windshield, §4 frontier). Needs `re_out/fist_image.bin`.

**AUDIO — menu music PLAYS (was "no sound at all"). Bit-exact = CHECKPOINT.** Patches 344–359 + `fist_opl.c`
(DOSBox DBOPL core) + `fist_sb.c`: the menu music is OPL2/OPL3 FM (port 0x388/0x389, NOT SB-DMA — proven via
DOSBox oplmode A/B), SOUNDDVR device-3 AdLib, sequencer 0a28→0c39→0b5d reading MIDI in MAINMENU.MS3. Plays at
the correct tempo/density. **iter-19 (master 8533775) DEBUNKED the "LAYER/CHAIN redistribution" blocker — it was a
PHANTOM from an INTRO-CONTAMINATED oracle reference.** The port's VOICING is ALREADY FAITHFUL: at real song-load
([ds:0x6]!=0) the driver DS tables [0x20]/[0x2a]/[0xc01] are BYTE-IDENTICAL to the oracle RAM dump; 0c39 is already
correct (patch 359 superseded the earlier "concrete bug"); iter-18's "garbage [0x20]" was a pre-song-load transient
snapshot; all 588 oracle key-ons route voice=channel = the port. **No 0c39/[0x20]/[0xc01]/0997 patch — one would be
WRONG.** The old ref straddled intro→menu (30 ch3 note-ons/half vs the song's 6 = intro leak); CORRECTED faithfully
to the pure-menu window (`tools/oracle/make_menu_ref.sh`; NOT fitted — port raw-xcorr DROPPED 0.034→0.021). **REAL
residuals (the actual audio path) = tempo/phase-pin (re-verify vs the CLEAN ref) + fnum/envelope-glide (0a28) + wasm
sound-dispatch (call_indirect) → an `audio-menu` verify flow.** Not yet a verify flow. Full detail docs/audio.md iter-19.

**EDITOR — .FSG load→save round-trip DONE + bit-verified; ADD-TANK edit-op DONE.** Patches 360–362 + 380:
- `editor-fsg-roundtrip` flow: idempotent fixed point (load→save=f1, load(f1)→save=f2, f1==f2), native+wasm 0-diff.
- `editor-add-tank` flow: ADD-TANK edit-op (80→81 DCBS units, +61 bytes).
- `editor-fsg-*` (cyprus1/india1/saudi1/syria1/ukraine1/train1/ukraine8): 7 battle round-trips; ALL 47 .FSG
  battles independently validated as idempotent fixed points, zero serializer defects. Entry: BATTLES→ACCEPT→e4bb
  (mode 2dab bit2). **Interactive editor (93c0 map-view + the 6 INT-33h tools) still untouched.**

**36 flows = the 26 below + `editor-fsg-roundtrip` + `editor-add-tank` + 7 `editor-fsg-<battle>` + `mission-cockpit`.**
`run_fsg`/`run_addtank` native timeout is 120s (was 60 — load-margin for the 10× gate under concurrent-agent load).

**OPEN frontiers toward the 10× DoD gate (ranked):** (1) **terrain windshield** rows0-19 (§4, the deep 6980
coord-layout paging confound — the one input-state lever left; the cockpit is region-limited around it). (2)
**AUDIO bit-exact** (sequencer layer/chain + a WebAudio verify flow — half the DoD). (3) **the pre-existing
`cancel` 26de-sprite-resolver flake** (~1/30 rc=139 SEGV — a MUST-FIX before the 10× gate: a flaky flow randomly
resets the streak). (4) **other missions/maps** (only AZER1 driven; the full campaign C##/D##.KLC set — gated on §4
for full-frame, but cockpit-chrome flows are cheap now). (5) **gameplay-in-motion, save/load, controls-config,
interactive editor** — untouched. (6) **the 10×-consecutive-clean harness** itself (a passrun/consecutive wrapper).

Detail of each in memory notes: [[mission-cockpit-camera-3e]], [[wasm-mission-op50-blocker]], [[audio-subsystem]],
[[mission-terrain-tile-pipeline]], [[editor-entry-and-roundtrip]], [[verify-shared-binary-race]] (10×-gate timeout margin).

The 07-18 snapshot below is kept for its still-accurate §4 terrain deep-dive + the front-end flow detail.

---

A surface counts as **bit-verified** only if it has a passing `tools/verify.sh` flow (AE=0 native AND wasm,
native↔wasm 0-diff). Everything else is honestly downgraded.

---

## 1. BIT-VERIFIED surfaces — 26 verify.sh flows

All AE=0 on native + wasm, native md5 == wasm md5, deterministic. Grouped:

| # | flow | surface |
|---|---|---|
| 1 | `intro` | TITLE.KDV FMV settled "ARMORED FIST" title card (frame 385, Route-1 KDV player) |
| 2 | `mainmenu` | main menu static frame |
| 3 | `about` | ABOUT dialog (credits text + OK) |
| 4 | `settings` | SETTINGS screen static render |
| 5 | `settings-sky` | SKY checkbox toggle (activate → re-paint OFF) |
| 6 | `settings-detail-med` | DETAIL radio → MEDIUM (value 2, status 0x8b3f) |
| 7 | `settings-detail-low` | DETAIL radio → LOW (value 0, status 0x8b3d) |
| 8 | `settings-sound-fx-med` | SOUND-FX radio → MEDIUM |
| 9 | `settings-music-off` | MUSIC radio → OFF |
| 10 | `settings-joystick` | joystick-type radio → STD ("NOT RESPONDING!") |
| 11 | `review` | REVIEW vehicle encyclopedia (M1A2 wireframe modal) |
| 12 | `selplayer` | SELECT PLAYER list dialog (7 .FPL enumerated) |
| 13 | `battles` | SELECT BATTLE list (47 .FSG) |
| 14 | `campaigns` | CAMPAIGNS → SELECT PLAYER roster |
| 15 | `campaigns-select` | click row inside SELECT PLAYER (red bar moves) |
| 16 | `battles-select` | click row inside SELECT BATTLE |
| 17 | `cancel` | CAMPAIGNS dialog CANCEL → menu restored |
| 18 | `battles-cancel` | BATTLES dialog CANCEL → menu (cb7c front-end) |
| 19 | `battles-scroll` | press-and-hold down-arrow → clamp at bottom |
| 20 | `battles-page` | click scrollbar track → PAGE DOWN |
| 21 | `selplayer-ok` | SELECT PLAYER OK → save D.FPL + "CURRENT PLAYER:D" (WRITE, isolated) |
| 22 | `selplayer-ok-kkr` | OK row-select KKR → KKR.FPL (WRITE, isolated) |
| 23 | `campaigns-ok` | CAMPAIGNS OK → SELECT A CAMPAIGN modal (WRITE, isolated) |
| 24 | `campaign-missions` | campaign OK → mission-select screen (tick-pinned; WRITE, isolated) |
| 25 | `battles-ok` | BATTLES OK → mission BRIEFING (AZER1.FSW text) |
| 26 | `battles-cancel-briefing` | briefing CANCEL → menu restored (tick-pinned) |

`WRITEFLOWS = selplayer-ok selplayer-ok-kkr campaigns-ok campaign-missions` (run against a fresh `cp -a`
scratch datadir so the engine's faithful .FPL writes never mutate the repo).

**Coverage reality:** the ENTIRE front-end — intro, all 7 main-menu items, every SETTINGS toggle
(checkbox + all radio groups), every list-dialog interaction (select/scroll/page/cancel), the OK save
path, campaign + battle drill-down through the mission-select and briefing screens — is bit-verified on
both targets. The menu/dialog subsystem is effectively DONE.

**CLAUDE.md contradiction:** the Status section's "next" work (SETTINGS toggle interactions) is **already
done** by patches 318–320 → flows 5–10. Do not re-delegate it. (This is the wasted-iteration the team lead
flagged.)

---

## 2. RECONSTRUCTED but NOT bit-verified

Landed, executes, deterministic/crash-free — but **no verify flow**, not AE=0. All are the in-mission
gameplay surface reached via `BATTLES → OK → ACCEPT` (cascade wired onto the real path by **patch 290**,
retiring the FIST_CASCADE diagnostic gate).

### 2a. Mission ACCEPT cascade + flight tick — crash-free, not verified
- Patches **154–290** reconstruct ACCEPT → extender MAP-LOAD (op 0x18) → mission loop.
- Patches **321–341** eliminate the flight-tick / AI-object SIGSEGV family: **deterministically crash-free
  through 42 s of gameplay on the worst-case ASLR-off layout** (commit 9cf9e01, patches 335–340: the
  c0e5/ab03 per-object-method base-loss chain b5e7/addb/a18e/ab7f/a904/e200).
- The mission loop **iterates**: c452 advances (1→314+), op-0x24 re-posts, FB changes frame-to-frame
  (patches 322/323, commit a2283fa).
- **State:** crash-free and running, but NO mission verify flow exists (`grep 'azer|mission' verify.sh` = 0
  flow entries; the 12 hits are comments). Not deterministic-frame-pinned into the harness.

### 2b. Cockpit / HUD — reconstructed, ~AE 6209 residual
- Patches **300–317, 331–333**: op-0x24 209e paint-walk chain, cockpit sprite methods (75e3/75c2/7b57/
  2ae9/c68c), reticle geometry (mga 0x2004 bbox analyzer → 288×81 matching the oracle, patch 332), reticle
  crash cleared (327/330).
- Localized residual **AE ≈ 6209** on the cockpit region (commit 6801af0). Executes; not AE=0, not a flow.

### 2c. Mission windshield terrain — WIRED behind `FIST_TILEFILL`, deep-blocked (see §4)
- `FUN_0000_689a` (sky resample) + `FUN_0000_6980` (voxel raycaster) wired into op-0x24 (commit 140c3b7).
- Structurally + chromatically correct scene (cloudy sky + tan desert + horizon); WHOLE AE
  **27325/64000**, sky band 42% exact, terrain structural correlation 0.515.
- **Gated behind `FIST_TILEFILL` (default OFF)** — not on the default mission path, not a flow.

**Net:** the AZER1 spawn frame is reconstructed end-to-end but is **not** bit-verifiable yet — the terrain
band (§4) blocks AE=0, and the cockpit carries a 6209 residual on top.

---

## 3. GENUINELY UNTOUCHED big deliverables (evidence-backed)

| deliverable | evidence of untouched | notes |
|---|---|---|
| **AUDIO PLAYBACK** | `re_out/fist_sb.c` does **not exist**; `grep -ci 'webaudio\|AudioContext\|sb_play\|dsp.*play\|sdl.*audio' native_main.c` = **0**; `fist_snd.c` is a 256-fn *decompile*, not wired to any output. Only SOUND.CFG parsing + DSOUNDS.BIN read (the [0xe00:] region is used as a **colormap** table, not sound). | **No sound is produced at all.** This is HALF of the Definition-of-Done ("bit-identical audio stream"). Completely open. |
| **LEVEL / MISSION EDITOR round-trip** | No editor patch (`ls patches \| grep -i edit\|waypoint\|round` = none real); no entry-point work; the 6 INT-33h editor tools at FIST.DAT 0x14d37–0x14dc8 (ADD WAYPOINT / CUT UNIT / TANK ADDED) are untouched. | First-class DoD deliverable (create→save→reload→sim byte-identical). Entirely open; even the entry point is only recon-noted, not located. |
| **Other missions / maps** | Only **AZER1** is driven (`ref/` has only `mission_azer1*`). The 47 .FSG battles, all campaigns' .CAM/.MS3/.KLC pairs beyond AZER1 = unexercised. | The full campaign/map set is DoD scope. |
| **Gameplay-in-motion** | Only the spawn frame is measured; no input-driven flight/combat frame is verified. Flight model (patches 328/334) reconstructed for crash-free, not for correct motion. | |
| **Save/Load (game state)** | Only the **.FPL player-profile** write exists (patch 146). No mission-state save/load, no save slots. | |
| **Controls-config screen** | The SETTINGS joystick-type radio is verified, but the full controls-config / calibration screen (JOYMGR, JOYSTICK.CFG, throttle/hat/pedals) is untouched. | |
| **Serial link** | No work. | Lowest priority (DoD-optional in practice). |

---

## 4. Mission-windshield-terrain deep frontier — memory note CHECK

The team lead's memory note (6980 HM/coord layout under render-time non-contiguous paging;
offline-unvalidatable ~40%; colormap/HM/camera content levers byte-exhausted; correct scene gated
`FIST_TILEFILL`) is **ACCURATE**. Cross-checked against the three current docs:

- **`docs/oracle_terrain_colormap.md` (2026-07-18, the latest verdict):** colormap CONTENT is **already
  correct** — the port's live `[0x85b8]` IS the bdc4 LIGHT reduce (max 228, contains the 169–183 band).
  The `bc90→blockA` hook is UNNECESSARY. The on-screen "dark 87–102" is 6980 *sampling* the dark subset of
  a correct colormap. **The real lever is the raycaster COORD MARCH — 6980's HM/reduce SPATIAL LAYOUT**,
  which under the port's flat map-load differs from the oracle's render-time non-contiguous paging. Feeding
  the frame-matched HM lifts structural correlation **0.161 → 0.515** (proving layout is the lever) but
  flattens colour (banked HM mismatches the port's own camera globals) — the documented paging confound.
- **`docs/oracle_terrain_residual.md`:** full camera frame-match moves terrain AE only **−8 px** → NOT
  camera position. Ruled out pixel-shift, DAC, sim de-lockstep.
- **`docs/oracle_maploadHM_6980.md`:** the port's op-0x18 map-load HM is **99.60% byte-identical** to the
  oracle render-time HM; the "5.11× over-draw" was a sparse-capture artifact. Map-load is NOT the bug.
- **`docs/oracle_endgame_wire.md` / `oracle_6980_colour_proof.md` / `oracle_proj395e.md`:** proj tables
  (3a24/3e24/4224/4624), 90b0/90b4, ramps — all byte-exact / faithful. Renderer 9200 is faithful (runtime
  == static image; no paged-out perspective renderer). Residual is 100% INPUT-STATE = the coord/HM layout.

**Summary: the note is right.** Colormap content, camera, ramps, projection, renderer, DAC — all
byte-exhausted and faithful/frame-matched. The one un-nailed input is the **spatial coord layout** the
render-time raycaster walks, which the port's flat memory model cannot reproduce and which offline capture
cannot isolate (the banked HM/globals are frame-skewed relative to the port's own camera → any injected
buffer flattens colour or mismatches structure).

**Tractable angle the note may be missing:** every doc validates OFFLINE against banked buffers that are
frame-skewed vs the port's live camera — that skew IS the confound. The unblock is an **instrumented
DOSBox (`dosbox-fist`)** that, at a camera-frame-matched tick, dumps 6980's **actual per-column
`[0x85bc]+coord+0x100000` dereference ADDRESSES** (not just the buffer contents) — i.e. capture the paging
map, not the paged data. With the true address→value map for one frame, the port's flat layout can be
remapped to reproduce it, breaking the paging confound that every purely-offline iteration has hit. This is
expensive (build + instrument dosbox) but is the only lever not yet pulled; all content-lever iterations
are exhausted and should stop.

---

## 5. RECOMMENDED next targets — ranked value × tractability

Ranked by (progress toward the 10×-consecutive-clean DoD gate) × (tractability). The gate needs the *full*
exhaustive harness clean 10× — front-end is done, so the remaining weight is audio + gameplay + editor.

**1. AUDIO subsystem (fist_sb.c + WebAudio) — HIGH value, MEDIUM tractability.**
Untouched, and it is literally half the Definition-of-Done ("bit-identical audio stream"). No amount of
video work reaches the gate without it. DD2 proved the pattern (SB/GUS DMA+IRQ → WebAudio); `fist_snd.c`
decompile already exists. **First step:** trace whether the engine issues DSP playback (port 0x2x0 + 8237
DMA + IRQ) on the intro/menu path under DOSBox, implement `fist_sb.c` as the DSP+DMA trap → PCM ring, wire
SOUNDDVR's playback method vectors, and bit-verify a menu click / intro sting against a DOSBox audio
capture. Start with a single deterministic sound (a menu blip) as the first verify-flow analog.

**2. Cockpit / HUD static bit-verify (partial mission flow) — HIGH value, MEDIUM tractability.**
The cockpit/HUD (patches 300–333) is reconstructed and crash-free; only a ~6209 residual and the
terrain-band block stand between it and the first *gameplay* verify flow. **First step:** add a mission
verify flow that dumps the AZER1 spawn frame region-limited to the cockpit rows (88–200), driving down the
6209 residual there independently of the deep-blocked terrain band — banking the first bit-verified
in-mission surface without waiting on §4. (The terrain band gets its own gated flow once §4 lands.)

**3. Mission-terrain COORD/HM layout (dosbox-fist paging capture) — HIGHEST value, LOW tractability.**
The single blocker for a fully bit-verified gameplay frame, and the gate to every other map. Deep-blocked
offline (§4). **First step:** build/instrument `dosbox-fist` to dump 6980's per-column dereference
*addresses* at a port-frame-matched tick (capture the paging map, not the data), then remap the port's flat
HM/colormap layout to it. Do NOT spend more iterations on colormap-content / camera / ramp levers — byte-
exhausted per §4.

**4. EDITOR round-trip — HIGH value (first-class DoD), LOW tractability.**
Entirely untouched, entry point not even located. **First step:** Phase-0 recon — the BATTLES ACCEPT tail
(e43f/4754/459a/1631) already partly traced is the editor build; locate the map-view entry and the 6
INT-33h tools at 0x14d37–0x14dc8, and determine the create/save/reload path before any patch. Large but
required for DoD.

**5. Second map / mission (breadth) — MEDIUM value, MEDIUM tractability, GATED on §3.**
Once one mission frame is bit-verified, the harness must cover the campaign/map set. **First step:** after
§3, drive a second .FSG (e.g. CYPRUS1) through ACCEPT and add its spawn flow — cheap incremental coverage
once the terrain pipeline is verified, but blocked until then.

**Do-NOT-redo (already done — stop re-delegating):** any SETTINGS toggle (flows 5–10, patches 318–320);
any menu/dialog screen or interaction (flows 1–26); the .FPL OK-save path (patch 146, flows 21–24). The
mission is already crash-free (patches 335–340) — do not re-chase the ACCEPT SIGSEGV family.
