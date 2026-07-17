# Oracle frame-matched AZER1 mission SPAWN capture — the port's black terrain is a RENDER bug, not a flight-model gap

> ## CAPTURE + INJECT VERDICT (2026-07-17, camera-orientation iteration) — **THE COLOUR RESIDUAL IS NOT THE
> CAMERA/ORIENTATION/PROJECTION. Proven by direct injection of the ORACLE spawn camera + projection + tile
> + horizon — the port terrain STILL renders (110,81,58) vs oracle (122,111,89).** The residual is
> DOWNSTREAM: ~59% is the PALETTE, ~41% is a residual sampler/index difference (both deep subsystems).
>
> **Captured oracle spawn render-camera TCB orientation** (RAM dump `SIGUSR2` at frame-1, TCB @ phys 0x10000,
> X@+0x2c — X=584282 Y=1142097 alt=12800, matches the committed spawn):
>
> | field | oracle spawn | port live spawn | match? |
> |---|---|---|---|
> | +0x38 heading | **26729** (0x6869) | 26729 | ✓ |
> | +0x3a | **512** (0x0200) | 0 | ✗ |
> | +0x3c | **256** (0x0100) | -256 (0xff00) | ✗ |
> | +0x3e foc | **256** (0x0100) | 256 | ✓ |
>
> **Captured oracle spawn projection globals** (extender base phys 0x131000; the state 9200 walks):
> `90b8=0xfff9b7aa 90bc=0x00ffec42 90d4=0xb1c0a498 90d8=0x39331d90 90b4=0x00020000 90c0=0x01000000
> 9104=0x7ff62180 9108=0xfcdbd542 90e8=0x02000000`, view width `90f8=81`, colheight `90f0=288`,
> horizon table `9114→0x7568` = `[6,4,3,2,1, 0×64, 1,2,3,4,5,6,7,8,8,9,9]`, colormap base `3918=0x44200`.
> **Oracle live tile3918** (ext-flat 0x44200 = phys 0x175200, 64 KB): 212 distinct, min idx 37 max 249,
> top idx 80(6926)/111(1703)/102(1539)/125(1513)/97(1245).
>
> **INJECT-CONFIRMATION (FIST_ISO_* diagnostic seams in `tools/native_main.c`, default OFF; port spawn frame,
> `FIST_MISSFB_N=1`, terrain band rows 8-88, oracle ref `ref/mission_azer1_spawn_native320.png`):**
> - **The port's 8120 is BYTE-EXACT correct.** Injecting the oracle orientation (`FIST_ISO_3A=512
>   FIST_ISO_PITCH=256`) makes the port's 8120 produce the oracle projection globals BYTE-FOR-BYTE
>   (90d4=b1c0a498, 90d8=39331d90, 90b8=fff9b7aa, 90bc=00ffec42 — all identical to the oracle). So the
>   only camera defect is the port's wrong live 0x3a/0x3c; 8120 transforms it faithfully.
> - Yet the colour barely moves: baseline terrain mean **(107.6,78.7,56.6)** → orientation-inject
>   **(110.5,80.7,58.4)** → +projection+horizon **(110.5,80.7,58.4)** (identical — because injected
>   orientation already yields the oracle projection) → +oracle-tile **(110.4,81.0,58.6)**. Oracle =
>   **(122.5,111.9,89.0)**. Every full injection saturates ~76% short of the oracle colour.
> - **Coverage is fine** (port 319 / oracle 318 cols filled; the "narrow window" premise is outdated).
> - **A spatial shift search is flat** (142–148 mean-abs-diff/px for every dy∈[-8,8], dx∈[-8,8]) → NOT a
>   projection/alignment offset; it is a uniform colour shift (oracle brighter + greener everywhere).
> - **Decomposition** (port tile+proj indices vs oracle ref): an IDEAL per-index palette drops the residual
>   from ~147 to **60.5** mean-abs-diff/px → **~59% of the colour error is the PALETTE** (532.pal is likely
>   the WRONG terrain palette for AZER1 — the port hardcodes it in the shim merge), and **~41% (60.5/px) is
>   a residual sampler/index difference** that survives even with the oracle tile + oracle projection
>   injected (the port's 9200 samples slightly different texels; camera XY is NOT injected — port
>   583982/1142557 vs oracle 584282/1142097).
>
> **CONSEQUENCE.** The task premise "the residual is the CAMERA/depth-projection" is **DISPROVEN by
> injection**. Fixing the port's wrong orientation (0x3a/0x3c) is a genuine defect but does NOT fix the
> colour (it slightly WORSENS the per-pixel AE: inject AE 5.37M vs baseline 5.29M, because the
> palette/sampler residual dominates and the current port orientation happens to align marginally better).
> The orientation is written by the extender flight-model (absent in the port); the render TCB +0x2c/0x30/
> 0x34 are bridged by dd15/a84c but +0x38..+0x3e (orientation) are NOT — that bridge is extender-role
> (a20d rotated camera / flight-model init), a deep subsystem, not a clean base-loss. **No engine/default
> change lands this iteration** (doctrine: no band-aid; the fix doesn't move the acceptance metric and would
> regress the deterministic spawn md5 with no colour benefit). Owners of the real frontier: the terrain
> PALETTE (which .PAL AZER1's colormap uses vs the hardcoded 532.pal merge) + the 9200 sampler/index
> fidelity. Diagnostic seams committed: `FIST_ISO_3A` (TCB+0x3a), `FIST_ISO_PROJ` (force oracle projection
> globals post-8120), `FIST_ISO_HZ` (oracle horizon), `FIST_ISO_TILE=<64KB file>` (force the 3918 colormap).


> ## VERDICT (2026-07-17) — **THE ORACLE RENDERS FULL VOXEL TERRAIN + SKY AT FRAME-1 / SPAWN.** The
> port's black windshield is a **render-wire + camera-Z bug fixable NOW** — it does NOT need the
> extender flight-model to first drive/settle the vehicle. Proven by a genuine frame-matched DOSBox
> capture of the ORIGINAL at the earliest mission frame, compared against the port's deterministic
> frame `6bc113e6` at the SAME sim state.

## Method (genuine, non-circular, 1:1 320×200)
`tools/oracle/capture_mission_spawn.sh` drives the ORIGINAL (`LOADGAME FIST.RUN`) under the instrumented
DOSBox (`tools/oracle/dosbox_vga_terrain_trace.patch`, `/tmp/debs/dosbox-fist`) through
BATTLES→OK→ACCEPT, then polls the SDL surface every 0.3 s. The `LOADING MISSION WITH SUPER DETAIL…`
photo grabs are large PNGs (~30–40 KB); the **first windshield frame** drops to ~14–22 KB. That
transition **is** frame-1 / spawn. The grab is the `output=surface scaler=none aspect=false` mode-13h
surface (`-trim` rect asserted **320×200**), 6→8-bit DAC = `(v<<2)|(v>>4)` — the same expander
`fist_vga.c`/`FIST_MISSFB` use, so it is byte-comparable to the port dump.
**Non-circular:** the reference is DOSBox rendering the ORIGINAL, never a copy of the port; its md5
(`75d40a0d…`) ≠ the port frame-1 md5 (`6bc113e6…`).

At the exact frame-1 grab, `SIGUSR2` dumps guest RAM; the live camera is read by scanning for the
spawn XY.

## What the oracle draws at frame-1 / spawn
- **Full voxel TERRAIN is rendered** — brown/tan hills, 91 distinct colours, only 1.2 % black in the
  windshield (rows 8–88). It is NOT a fade-in / black / cockpit-only frame.
- **SKY is present** — blue cloud band (e.g. RGB `93,97,117`) across the top of the windshield.
- The LOADING photo → windshield transition is a hard cut; the very first windshield frame already has
  the terrain drawn (camera barely moved: odometer ≈ spawn, XY within ~500 units of spawn).

## The camera at frame-1 / spawn (guest RAM, live TCB mirror @ phys `0x1002c`, robust across 2 runs)
| | X | Y | alt | terrain h = alt/≈292 |
|---|---|---|---|---|
| **oracle spawn (frame-1)** | 584312 | 1142051 | **12800** | **43.8** |
| mission-file spawn record (`0x034d14`) | 583982 | 1142557 | 0 | — |
| **port live spawn** (per task) | 583982 | 1142557 | **3328** | 11.4 |
| oracle settled (task) | — | — | 26880 | 92 (h≈97) |

- The oracle camera Z **tracks the terrain height under the camera**: at spawn `h=43 → alt≈12800`
  (`43×292≈12556`), settled `h=97 → alt≈26880`. This "climb" is the **tank driving from low ground
  (h=43) onto higher ground (h=97)** — it is applied per-frame (instant terrain-follow at spawn), NOT a
  multi-second altitude ramp.
- **The port's spawn alt (3328) is itself wrong** — far below the oracle's 12800; 3328 corresponds to
  h≈11.4, i.e. the port camera is set well BELOW the terrain surface it sits on. The port likely sets
  camera Z to a fixed eye offset WITHOUT adding `terrain_height × scale`.

## The CORRECT comparison (port frame-1 `6bc113e6` vs oracle frame-1, both at spawn)
Cockpit chrome is already registered (proof the frames are aligned), so the whole delta is the missing
terrain+sky render — not a sim-position mismatch:

| region | metric |
|---|---|
| terrain/windshield rows 8–88 (25600 px) | **AE = 23438** |
| port terrain black | **87.2 %** (vs oracle **1.2 %**) — the window is empty, not garbage-terrain |
| port / oracle distinct terrain colours | 74 / 91 |
| sky band rows 8–20 | port 90 % black; oracle blue `93,97,117` |
| cockpit gearstick/gauge chrome (rows 108–150, cols 88–160) | **AE = 0** (frames registered) |

The prior frame-1-vs-**settled** terrain AE was 24618; this frame-1-vs-**frame-1** AE (23438) confirms
the magnitude was not driven by the sim-position mismatch — it is the missing render.

## Consequence for the frontier
The port's black terrain is **fixable now**, independent of the extender flight/momentum model:
1. **Render-wire:** the windshield voxel writer (extender `9200`, dispatched via op-0x24) must actually
   draw into `0xA0000` — the oracle proves terrain renders at the exact spawn camera state.
2. **Camera-Z init:** set the spawn camera alt to follow terrain height (`h×scale + eye`) so it matches
   the oracle's ~12800 (not the current 3328); the port camera is presently below the surface. This is
   an instant per-frame terrain-follow, NOT the gradual flight-model settle.

Whether (1) alone suffices, or (2) is also required for 9200 to produce non-black pixels, is the
port-side next step (set the port camera alt to 12800 and re-check 9200's output) — outside this
capture task.

## Artifacts committed
- `ref/mission_azer1_spawn_native320.png` — genuine 1:1 320×200 oracle frame-1 (md5 `75d40a0d…`).
- `tools/oracle/samples/oracle_mission_loading.png` — the LOADING photo (context; the frame BEFORE spawn).
- `tools/oracle/samples/oracle_mission_settled_ref.png` — a settled windshield (context; terrain evolved).
- `tools/oracle/capture_mission_spawn.sh` — reproducible capture + live-camera read.

## Repro
```bash
# needs the instrumented dosbox (tools/oracle/README_terrain.md) at /tmp/debs/dosbox-fist
DOSBOX=/tmp/debs/dosbox-fist tools/oracle/capture_mission_spawn.sh
# port frame-1 for the AE comparison (md5 6bc113e6):
SCR=$(mktemp -d); cp -a armoredfist "$SCR/af"
setarch -R env FIST_DATADIR="$SCR/af" FIST_COOP_TICK=1 FIST_TICK_HZ=25000 \
  FIST_MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0" \
  FIST_MISSFB=/tmp/port_f1.ppm FIST_MISSFB_N=30 /tmp/fist_native
```

## HONEST STATUS after patch 295c3bf (self-measured 2026-07-17) — voxel terrain DRAWS at spawn, but only PARTIALLY covers the windshield
Patch 295c3bf (shim, in-mission-guarded) wired 9200 + the oracle-exact camera-Z terrain-follow. Verified by direct measurement (not the agent's window-only figure):
- **Spawn frame (op-0x24 post #1) is DETERMINISTIC: md5 `6678719e`, 3/3 identical.** Camera alt = 12800 (oracle-exact, `(h<<8)+1792`, h=43). REAL win — the voxel terrain draws for the first time, and the camera-Z is asm/oracle-exact.
- **BUT the full 320-wide windshield (rows 8-88) is 91.6% black (23450/25600)** vs the oracle's 1.2% (303/25600). The agent's "black 87%→1%" was measured on the ~81-col voxel WINDOW only (`0x90f8=81 cols`, fb+0x650), NOT the full width — misleading for the full frame. Only ~2150 px are non-black at spawn vs the oracle's ~25300. So the port draws terrain in a NARROW sub-window; the oracle fills the FULL windshield width.
- **Later op-0x24 posts are NON-deterministic** (post #30 md5 varies `20692847`/`aac4b376`) because the live camera XY BLOWS UP (66M/331M) — the wall-clock-dependent flight-model over-run (only visible now that terrain renders). Post #1/spawn is stable; later frames need the camera-XY determinism (a separate frontier).
- verify.sh both = 24/24 (in-mission guard → menu flows unaffected); pristine engine unchanged.
**RANKED spawn-frame residuals (vs ref/mission_azer1_spawn_native320.png):** (1) **render coverage** — the voxel terrain covers only ~2150 px of the windshield vs the oracle's ~25300 (the ~81-col window / horizon / clip — WHY does the port cover a fraction of the width the oracle fills? = the biggest residual); (2) **tile palette ~2× too dark** (browns 60,32,12 vs 125,109,77 — the tile3918/85b8 colormap-collapse, 175 vs 212 distinct — NOT block A which is byte-exact); (3) **sky band** (blue 93,97,117 rows 8-20, `.SKY`-fed, absent); (4) camera projection / horizon position; (5) later-post camera-XY determinism.

> ## CORRECTION (2026-07-17, colour-fidelity iteration) — RESIDUAL #2 IS **NOT** A PALETTE BUG **NOR A TILE-CONTENT BUG**; IT IS THE RENDERER SAMPLING THE **DARK ROWS** OF A CORRECT TILE (the depth-row / camera-projection geometry = residual #1/#4). No palette or bc9c fix warranted this iteration.
>
> **Method (decisive, mostly offline; live spawn frame deterministic md5 `b15766c0`, FULL-frame AE 33253).**
> Captured the default 92a2692 spawn (`setarch -R FIST_COOP_TICK=1 FIST_TICK_HZ=25000 …
> FIST_MISSFB_N=1`, ext image `re_out/fist_image.bin` present — it is gitignored, `make kernel-image` in a
> fresh worktree first or g_ext_ready stays false and NO op-0x24 post fires). Dumped the fb indices
> (`FIST_MISSFB_FBIDX/FBDUMP`), the merged DAC (`pal_merged.bin`), the live tile (`FIST_MTXDUMP`) and the
> bc9c block-base (`FIST_BBDUMP`), and mapped everything through the loaded 532.pal.
>
> **(a) PALETTE RULED OUT.** The mission DAC band 80..255 is byte-exact 532.pal (`FIST_MISSFB_PALCMP`:
> eng782 == ext5598 in the terrain band, e.g. idx220), the 6→8 expansion is correct VGA bit-replication,
> and **532.pal CONTAINS the oracle browns** — the oracle's (125,109,77) is idx 90/106/170 (dist ≤13),
> its (117,101,48) is **exactly idx 180**. So the light tans the oracle shows are reachable; the palette
> is not the problem.
>
> **(b) TILE CONTENT RULED OUT.** The port bc9c block-base (`FIST_BBDUMP`) windowed at **0xf200 == oracle
> blockB rows 14..255 = 61952/61952 (100%)** (rows 0..13 = bc06 LOD, the known separate gap), and — mapped
> through 532.pal — the port block-base and the oracle blockB have **IDENTICAL colour distribution**: same
> mean RGB **(120,88,61)** and identical top-5 indices+counts (idx80 6885/6926, idx111 1703, idx102 1539,
> idx125 1513, idx97 1245). The 175-vs-212 distinct is only in the rare tail. bc9c is fine for colour.
>
> **(c) THE REAL DEFECT = the renderer samples the DARK rows of the tile.** The port RENDER's terrain-band
> mean is **(109,78,54)** — BELOW the tile-content mean (120,88,61); its top rendered indices are 83
> (65,48,12), 118 (85,40,24), 151 (44,28,24) = the DARK end. The oracle RENDER is **(122,106,71)** — ABOVE
> the tile mean, GREENER than the tile content, i.e. it samples the LIGHT/green rows. Since the tile the two
> read is byte-equal in content, the divergence is **which texels the ray-march / depth-projection selects**
> (the tile's row axis IS the depth-shade ramp; the port picks deeper/darker rows). Empirically **no window
> offset fixes it**: `FIST_TILEWIN`/a faithful wrap-around rotation (new default-OFF `FIST_TILEROT` seam,
> tested 0x4200/0xf200) move the render mean by ≤6 and never reach (122,106,71); FULL AE stays 33248..33279.
>
> **VERDICT.** Residual #2 as originally phrased ("tile palette 2× too dark") is a **red herring** — the
> per-pixel (60,32,12) vs (125,109,77) top-colour was a geometry artifact (different coords sample different
> texels). Colour fidelity is **GATED behind residual #1/#4** (the windshield coverage + camera/depth
> projection): once the port samples the same texels as the oracle, the identical tile+palette will produce
> the identical colours. Owners: camera-settle-render / windshield-render-wire / voxel-render-coverage.
> **No engine/ext/shim change lands** (the `FIST_TILEROT` experiment did not help → reverted; pristine
> `61453e42`/`0051cb56`/`75c6d726` unchanged).
