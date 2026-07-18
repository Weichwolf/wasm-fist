# Terrain colormap — HARD VERDICT: the colormap CONTENT is already CORRECT; the residual is the 6980 COORD/HM LAYOUT

> ## HARD RESULT (2026-07-18) — measured with actual bytes, in the LIVE `FIST_TILEFILL` native wired run
> The port's LIVE colour source that `FUN_0000_6980` dereferences **IS the bdc4 LIGHT reduce** (max 228,
> mean 141, 78 distinct, 8.4 % of cells in the ref-target band 169–183, dominant values 151/132/145/156/163)
> — byte-near-identical to the banked frame-matched reduce. **The colormap CONTENT is not the defect. The
> `bc90→blockA` hook is UNNECESSARY — the reduce is already present at `[0x85b8]`.**
>
> `terrain-residual`'s core claim ("6980 reads DARK / no offline port colormap source has 169–183 / oracle_85b8
> tops at 151/159") is **REFUTED by bytes**: every colormap source (live `[0x85b8]`, banked RED, banked
> `oracle_85b8_first64k`) has **max 228** and **contains** the full 169–183 band. The "dark 87–102" is what
> 6980 *emits on screen* — it samples the DARK subset (84–130, mode 87–111) of the CORRECT reduce, where the
> oracle samples the BRIGHT subset (mean 172). The tile's "max 252 / 32 % bright" that looked like a wrong
> colormap was **689a SKY LEFTOVER** (6980 overlays only 26686 of ~40000 tile cells; its own output maxes at 228).
>
> **The endgame-vs-gate contradiction, resolved with bytes:** endgame was RIGHT that `[0x85b8]` ≈ the light
> reduce, and RIGHT that `bc90` is unnecessary — but WRONG that feeding RED gives a 0-px frame: it changes
> **10360 px** and moves WHOLE AE 27325 → 27388 (slightly WORSE). Feeding a colormap variant cannot help
> because the content is already correct.
>
> **The real lever = the raycaster COORD MARCH (6980's HM/reduce SPATIAL LAYOUT).** 6980's colour is
> `colormap[coord]` (unshaded — the L0 shade only indexes the proj table for screen-Y). coord is driven by
> the depth ramps (`coord = base + 4224/4624[depth]` + per-column) which are **byte-exact frame-matched**
> (verified) and by the camera (frame-matched). So same colormap, same ramps, same camera → the ONLY
> remaining input is the **heightmap/reduce coord LAYOUT**, which under the port's flat map-load differs from
> the oracle's render-time NON-CONTIGUOUS paging. Feeding the frame-matched HM lifts the terrain STRUCTURAL
> correlation 0.161 → **0.515** (correct structure) — proving the layout is the lever — but flattens the
> colour (the banked HM mismatches the port's own camera globals). This is the documented offline-validation
> / paging-confound limit (`docs/oracle_6980_colour_proof.md`, `docs/oracle_proj395e.md`), not a
> colormap-content bug.
>
> **NO colormap fix improves the AE.** Terrain AE stays at baseline; the content lever is exhausted.
> Engine PRISTINE (`61453e42`/`0051cb56`/`75c6d726`/`1e0cfd38`), `make check` clean, mainmenu AE=0, default
> boot rc=0. Only `tools/native_main.c` (two default-OFF read-only diagnostics) + this doc changed.

## 1. What 6980 actually reads — the bytes

`FIST_TILEFILL` wiring (`tools/native_main.c`): it builds `hmcm[0..1MB] = [0x85bc]` (heightmap),
`hmcm[1MB..2MB] = [0x85b8]` (colormap), repoints `[0x85bc]→hmcm`, so 6980's colour read
`mov al,[esi+ecx+0x100000]` dereferences `hmcm[coord+0x100000]` = the content of `[0x85b8]`.

Dumped LIVE at the AZER1 spawn render (`FIST_TILEFILL_CMDUMP`, read-only):

| buffer | distinct | min | max | mean | cells in 169–183 |
|---|---|---|---|---|---|
| **port LIVE `[0x85b8]` (what 6980 reads)** | **78** | 82 | **228** | 141.1 | **88306 / 1048576 (8.4 %)** |
| banked RED frame-matched reduce | 146 | 15 | 228 | 140.7 | 87884 (8.4 %) |
| banked `oracle_85b8_first64k` | 67 | 84 | 228 | 139.3 | 4511 / 65536 (6.9 %) |

Band distribution of the port's LIVE colour source: 84–130 = 319846, **131–168 = 587372**, 169–183 = 88306,
184–214 = 46337, 215–255 = 6306. It is the light reduce, dominant in the mid band, tailing into 169–183.

**On-screen indices** (DAC inversion of `ref` is EXACT — max sq-dist 0), terrain rows 45–88 × 32–288:

| | distinct | top values |
|---|---|---|
| PORT terrain | 97 (scattered) | 97, 99, 111, 96, 91, 87 (DARK 84–130), + 151/224 |
| REF terrain | 33 (tight) | **179, 151, 163, 156, 166, 183, 169, 176, 172** (BRIGHT-TAN 151–183) |

The port samples the DARK tail of the correct reduce; the ref is the tight bright band. Same colormap
(contains both), different **coords**.

## 2. The endgame-vs-gate contradiction — resolved with bytes

- **`terrain-residual` / `colour_gate`:** "6980 reads DARK; no port colormap source has 169–183; oracle_85b8
  tops at 151/159." — **REFUTED.** The port's LIVE `[0x85b8]` maxes at 228 and holds 88306 cells in 169–183.
  "tops at 151/159" was misreading the top-FREQUENCY values (151, 159) as the maximum. The "dark 87–102" is
  the on-screen EMISSION (dark subset of the correct reduce), not the colormap content.
- **`endgame-wire`:** "`[0x85b8]` first 1 MB ≈ light reduce (99.8 %); bc90 unnecessary." — **CONFIRMED**
  for the content claim. But "feeding RED vs port's own = 0-px byte-identical" is **REFUTED**: feeding the
  banked RED changes **10360 px** and moves WHOLE AE 27325 → 27388 (worse). endgame's own extreme-colormap
  test (RED == native reduce → 0-px) held only because the port's `[0x85b8]` already IS ~the reduce; a
  genuinely different reduce (the banked RED, aligned to the oracle HM layout, not the port's) does change
  the frame.

## 3. Does the light reduce contain 169–183? YES — the bc90→blockA hook is unnecessary

Every colormap source (§1 table) has **max 228** and contains the 169–183 band. The port already runs
`bdc4` into `[0x85b8]` producing the light reduce (78 distinct — the docs' "89 collapse" figure is over the
full 4 MB LOD range; the first 1 MB that 6980 samples is the correct reduce). Building the reduce more
faithfully (the `bc90→blockA` hook) does not add 169–183 that is already present, and cannot fix the
coord-layout mismatch that makes 6980 sample the dark cells.

## 4. The real lever — coord/HM layout (verified by elimination)

6980 colour = `colormap[coord]`, unshaded (patch 342: `sy = proj[(L0 + heightmap[coord]) & 0xff]` — the L0
shade indexes the proj table for screen-Y only; the colour byte is the verbatim `colormap[coord]`).
coord = `base + 4224/4624[depth]` + per-column term.

| input | status |
|---|---|
| **depth ramps 3a24/3e24** | **byte-EXACT frame-matched** (== bundle); native 90c0=0x00ffffff == capture → recomputed 4224/4624 frame-matched. NOT the residual. |
| **camera** | frame-matched (position lockstep at spawn; pitch/roll seeded 384/256). NOT the residual. |
| **colormap content** | correct light reduce (§1). NOT the residual. |
| **HM/reduce coord LAYOUT** | port flat map-load (max 96) vs oracle render-time paged layout (max 140); **1.51 % byte-identical** — different coord→cell mappings. **THE residual.** |

Even `FIST_6980PROVE` (all frame-matched inputs — HM + ramps + camera + scalars + proj) reproduces only
~40 % of the oracle 6980 shadow — the flat capture HM does not reproduce the render-time non-contiguous
paging of the deep-ray samples.

## 5. Measurements (no colormap fix helps)

Spawn frame vs `ref/mission_azer1_spawn_native320.png`, per region (rows: sky 8–45, terrain 45–88 ×32–288,
cockpit 88–200; struct_corr = grayscale structural correlation over the terrain band):

| config | WHOLE AE | sky | terrain | cockpit | struct_corr |
|---|---|---|---|---|---|
| **A: baseline (port's own light reduce)** | **27325** | 5462 | **10150** | 8368 | **0.161** |
| B: colour source = banked RED reduce | 27388 | 5462 | 10231 | 8368 | 0.179 |
| C: banked HM + banked RED (frame-matched) | 33275 | 9472 | 10544 | 8368 | **0.515** |

(Whole AE 27325 matches `docs/oracle_terrain_residual.md` exactly; my terrain-region uses cols 32–288 so
the absolute terrain count differs from that doc's convention, but the direction is what matters.)

- **B** (correct-content reduce, wrong layout for the port HM): terrain AE +81, WORSE. Content is not the lever.
- **C** (frame-matched HM+reduce): terrain STRUCTURE jumps 0.161→0.515 (correct coord layout) but AE worse
  (colours flatten — the banked HM mismatches the port's own camera globals). This is the decisive proof:
  the LAYOUT drives the structure; the port's own HM layout is the residual.

**Terrain AE does NOT drop. The colormap-content lever is exhausted (the content is already correct). The
open frontier is the 6980 HM/coord LAYOUT under the extender's render-time paging — a data-provenance /
paging-confound frontier, not a colormap-content bug.**

## 6. No-regression + repro

Only `tools/native_main.c` changed: two default-OFF read-only diagnostics inside the `FIST_TILEFILL` block —
`FIST_TILEFILL_HM=<file>` (override the heightmap half, frame-match test) and `FIST_TILEFILL_CMDUMP=<file>`
(bank the LIVE `[0x85b8]` colour source + `/tmp/port_hm.bin`). Engine md5s pristine
(`61453e42`/`0051cb56`/`75c6d726`/`1e0cfd38`); `make check` clean; dedicated build
(`NATIVE=/tmp/fist_terraincm make native`) default boot rc=0; mainmenu verify flow AE=0.

```bash
NATIVE=/tmp/fist_terraincm make native
SCR=$(mktemp -d); cp -a armoredfist "$SCR/af"
MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0"
BASE="FIST_DATADIR=$SCR/af FIST_COOP_TICK=1 FIST_TICK_HZ=25000 FIST_MOUSE=$MOUSE FIST_TILEFILL=1 FIST_MISSFB_N=1"
# dump the LIVE colour source 6980 reads (proves it is the light reduce, max 228):
setarch -R env $BASE FIST_TILEFILL_CMDUMP=/tmp/port_cm.bin FIST_MISSFB=/tmp/x.ppm /tmp/fist_terraincm
# feed a correct-content reduce -> proves content is not the lever (worse):
setarch -R env $BASE FIST_TILEFILL_RED=/tmp/banked_red.bin FIST_MISSFB=/tmp/B.ppm /tmp/fist_terraincm
# feed the frame-matched HM+reduce -> structure 0.161->0.515 (layout IS the lever):
setarch -R env $BASE FIST_TILEFILL_HM=/tmp/banked_hm.bin FIST_TILEFILL_RED=/tmp/banked_red.bin FIST_MISSFB=/tmp/C.ppm /tmp/fist_terraincm
```
