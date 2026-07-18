# STATE — accurate current-state frontier map (2026-07-18)

Read-only recon snapshot. Supersedes the stale CLAUDE.md `Status` section (which stops at **patch 153**;
the repo is at **patch 343 / 341 files**, **219 commits**, **26 verify.sh flows**). Where CLAUDE.md
contradicts the repo, the repo wins — flagged inline.

Engine pristine (unchanged): `re_out/fist.c` **61453e42**, `fist_ext.c` **0051cb56**, `fist_mga.c`
**75c6d726**, `fist_snd.c` **1e0cfd38**. Tree clean at snapshot time.

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
