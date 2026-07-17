# Terrain residual — HARD VERDICT: the AZER1 spawn terrain is a COLORMAP-CONTENT mismatch, not camera

> ## HARD RESULT (2026-07-18)
> A byte-exact FULL render-camera frame-match (position X/Y + alt + heading + pitch/roll + foc + detail,
> seeded to the oracle ref-frame values) moves the spawn WHOLE AE only **27325 → 27317 (−8 px)** and the
> terrain band **12096 → 12088 (−8 px)**; sky and cockpit are byte-unchanged. **The dominant terrain
> residual (~12088/64000) is NOT camera position.** Proven, with evidence, to be a **colormap-CONTENT
> mismatch**: the port's asm-faithful `FUN_0000_6980` emits palette indices in the DARK band (87–102)
> where the DOSBox reference terrain uses the BRIGHT-TAN band (169–183). It is **not** a pixel shift, **not**
> a DAC/palette error, and **not** whole-mission sim de-lockstep (the mission IS in positional lockstep at
> spawn). The blocker is the extender's render-time colormap RE-PAGING (the documented `colour_gate` limit,
> [[colormap-groundtruth]]): the page `6980` dereferences via `[0x85bc]+coord+0x100000` does not match the
> page the oracle DOSBox dereferenced.
>
> Engine PRISTINE (`61453e42`/`0051cb56`/`75c6d726`/`1e0cfd38`), `make check` clean, mainmenu AE=0, default
> boot alive. Only `tools/native_main.c` (two env-gated diagnostics) + this doc changed.

## 1. The full oracle render-camera at the ref frame (pinned)

From the banked per-9200-frame DOSBox capture (`tools/oracle/samples/oracle_camera_bridge_trajectory.txt`,
`FIST_R9200CAP` [r92cam]; TCB at flat 0x10000 via the wrapped EA). The ref frame
(`ref/mission_azer1_spawn_native320.png`, DAC == captured frame-1 DAC) frame-matches the **settling state**
oracle frames ~4–22:

| field (TCB off) | oracle ref frame | port LIVE @ spawn dump | Δ |
|---|---|---|---|
| X (+0x2c) | 584042 | **583982** | −60 |
| Y (+0x30) | 1142465 | **1142557** | +92 |
| alt (+0x34) | 12800 | 12800 (seeded) | 0 |
| head (+0x38) | 26729 | 26729 | 0 |
| pitch (+0x3a) | 384 | 384 (seeded) | 0 |
| roll (+0x3c) | 256 | 256 (seeded) | 0 |
| foc (+0x3e) | 256 | 256 | 0 |
| detail (+0xcc) | 1 | 1 | 0 |

**The port's LIVE camera position (583982 / 1142557) is EXACTLY the oracle's frame-1 spawn position.** The
only positional difference is the oracle's ~4–22-frame settling drift (X +60, Y −92 — sub-tile: the map is
1024-wide, so 60 world units ≈ 0.06 of a heightmap tile). Heading/foc/detail already match. So the port's
own mission sim spawns the camera at the byte-correct oracle position — it is in positional lockstep.

## 2. Full frame-match measurement (FIST_TILEFILL, `FIST_FULLCAM=584042:1142465:12800:26729:256:1`)

| config | WHOLE AE | sky[8-45] | terrain[45-88] | cockpit[88-200] |
|---|---|---|---|---|
| FIST_TILEFILL, port live camera (attitude+alt seeded) | 27325 | 6524 | 12096 | 8368 |
| **+ FULL position frame-match (X/Y/head/foc/det = oracle ref)** | **27317** | 6524 | **12088** | 8368 |

**−8 px whole-frame, −8 px terrain.** Negligible. Control: `6980`'s tile DOES respond to position (the
ref-frame 60/92 offset changes 2848/65536 = 4.3 % of the 256×256 tile; a 4096-unit offset changes 39 %) —
but that tile change is below the affine-9200 sampling resolution of the ref frame, so the on-screen
terrain is essentially unchanged. Position is genuinely not the lever.

## 3. HARD VERDICT on the terrain residual's nature — colormap CONTENT, ruled in by elimination

Region = terrain rows 45–88, cols 32–288 (11008 px), port at the exact oracle frame-match camera.

**(i) Pixel shift — RULED OUT.** Cross-correlation over dy,dx ∈ [−10,10]: the minimum AE is at **(0,0)**
(10150), no shift peak lowers it. Not a horizontal/vertical off-by-N in 9200's screen-origin or the tile
U/V origin.

**(ii) DAC / palette — RULED OUT.** The port palette (ext+0x5260) CONTAINS the exact reference terrain
colours — ref-dominant tan (125,109,77) = 6-bit (31,27,19) is present at **index 179**, with a full
bright-tan band at indices 169–183. The *set* of indices the port terrain uses even has the correct mean
palette brightness (73.2 vs ref 73.7, 6-bit sum). The DAC is not darkening the terrain.

**(iii) Colormap CONTENT — RULED IN.** The port terrain looks dark (weighted mean brightness 51 vs ref 74)
purely because `6980` **emits the wrong indices**: its most-frequent terrain indices are the DARK band
**97, 99, 111, 91, 96 (87–102)** — 6-bit ≈ (13,9,7) dark brown — whereas the reference terrain is
dominated by the BRIGHT-TAN band **169–183**. `6980` plots `colour = colormap[coord]` with no shading, so
the emitted index band == the colormap page's value band. **No offline-available port colormap source has
values in the 169–183 band** — `oracle_85b8_colormap_first64k` tops out at 151/159, the banked light reduce
at 228 but distributed low. The render-time colormap the oracle dereferenced is a HIGHER-band page than any
captured source. This is the documented extender colormap **RE-PAGING** limit (`colour_gate` / proj docs;
[[colormap-groundtruth]] part 2 — the mid-`89b0` `bc90`→block-A hook governs which page lands).

**Texture structure also differs (secondary).** Grayscale structural correlation port↔ref terrain = **0.246**
(1.0 = identical texture, 0 = uncorrelated). A per-channel gain+bias fit lifts 85 % of pixels to "close"
(<48 summed RGB) but 0 % to exact. So on top of the band offset, the fine pixel pattern does not line up —
consistent with a wrong colormap scrambling the coord→index mapping (colour = colormap[coord]; a wrong
colormap scrambles the local light/dark pattern) and/or the depth-ramp exactness (3a24/3e24 are banked from
a frame-skewed capture). Both are RENDER/DATA inputs, not sim evolution.

## 4. DECISIVE VERDICT — reachable by render/data fixes, NOT whole-mission sim-lockstep

**The spawn frame is reachable toward bit-exact by render/data fixes. It does NOT require whole-mission
deterministic lockstep.** The camera POSITION is already frame-matched (the port spawns at the byte-correct
oracle position; the ref-frame drift is 60/92 units → 8 px). The residual is a **data-provenance** problem
(which colormap page `6980` samples + depth-ramp exactness), a bounded `colour_gate` frontier — not the
mission sim evolving to a different state.

Concrete plan to close the spawn frame:
- **TERRAIN (dominant, ~12088).** Feed `6980` the render-time colormap so it emits the bright-tan band
  (169–183) instead of dark (87–102). This is the `colour_gate` / block-A colormap-source resolution
  ([[colormap-groundtruth]] part 2, the mid-`89b0` `bc90`→block-A hook) driven NATIVELY (the port runs the
  whole pipeline in one coherent frame — no frame-skewed offline capture needed). Verify the emitted index
  band matches 169–183, then re-check the 0.246 structural correlation (a correct colormap should lift it).
  Confirm the depth ramps 3a24/3e24 with a native (not frame-skewed) source.
- **SKY (6524).** The attitude lever is applied (halved sky AE 10993→6524). Remaining = `689a` sky-resample
  exactness (rows 160–255) — reachable, a shim-reconstruction refinement.
- **COCKPIT (8368).** Separate cockpit DEVICE-subsystem debt (left-panel AUTO-CONTROL/speed noise, patches
  301/302), present in the baseline; independent of terrain.

**Promotion note:** the full-position seed is NOT promoted to the FIST_TILEFILL default. It gives only 8 px
and is LESS faithful than the sim's own spawn position (which is already the byte-correct oracle spawn) —
hardcoding the ~4–22-frame drifted position over the sim's correct value would be a step backward. Kept as
the env-gated diagnostic `FIST_FULLCAM` for future frame-match sweeps.

## 5. Reproduce
```bash
NATIVE=/tmp/fist_cam make native
SCR=$(mktemp -d); cp -a armoredfist "$SCR/af"
MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0"
# full-camera frame-match spawn dump:
setarch -R env FIST_DATADIR="$SCR/af" FIST_COOP_TICK=1 FIST_TICK_HZ=25000 FIST_MOUSE="$MOUSE" \
  FIST_TILEFILL=1 FIST_FULLCAM=584042:1142465:12800:26729:256:1 \
  FIST_MISSFB=/tmp/spawn_fc.ppm FIST_MISSFB_N=1 /tmp/fist_cam
compare -metric AE /tmp/spawn_fc.ppm ref/mission_azer1_spawn_native320.png /dev/null   # 27317
# raw 6980 tile (position-sensitivity / index-band analysis):  + FIST_TILEDUMP=/tmp/tile.bin
# port live camera at spawn:                                    + FIST_MISSFB_PROBE=1   ([missfb] cam[2c/30/34])
# raw framebuffer indices in the terrain band:                  + FIST_MISSFB_FBIDX=1 FIST_MISSFB_FBDUMP=1
```
Diagnostics added to `tools/native_main.c` (both env-gated, default frame byte-unchanged):
`FIST_FULLCAM="X:Y:alt:head:foc:det"` (full render-camera override) + `FIST_TILEDUMP=path` (raw 6980 tile).
