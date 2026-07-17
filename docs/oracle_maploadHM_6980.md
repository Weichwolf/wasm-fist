# The port's OWN op-0x18 map-load HM is CORRECT — the "6980 over-draw" is a shadow-capture artifact, NOT a map-load defect

> ## HARD VERDICT (2026-07-17)
> The previous iteration's premise — *"the port over-draws 5.11× because the FLAT capture heightmap does
> not reproduce render-time paging; build the port's OWN op-0x18 map-load HM and 6980 will stop
> over-drawing"* — is **DEBUNKED by direct measurement.**
>
> 1. **The port's op-0x18 map-load already builds the correct contiguous HM.** Its `[0x85bc]` heightmap
>    (first 1 MB = exactly what 6980 addresses) is **99.60 % byte-identical** to the oracle render-time
>    HM banked in the 6980 capture (distinct/min/max/mean identical: 82 / 15 / 96 / 53.2). Feeding the
>    port's OWN HM into the real 6980 produces **byte-identical output** to feeding the capture HM. **The
>    HM was never the problem.**
> 2. **The "5.11× over-draw" is a MEASUREMENT ARTIFACT.** The banked oracle *"shadow"* used as ground
>    truth is a **sparse/incomplete capture** (75 % single-pixel runs, mean run 1.5) — it is the DENSE
>    render-time tile `tile_o` with **~88 % of pixels zeroed** (all 7585 shadow-nz are an exact-value
>    subset of `tile_o`). Comparing the port's DENSE voxel tile against this sparse subset spuriously
>    reports "over-draw 5.11×". Against the REAL dense `tile_o`, in 6980's terrain region (rows 0-159)
>    the port **covers 97.8 %** (29735/30400) — there is **no over-draw**.
> 3. **The genuine residual is COLOUR, not geometry, and it is a 6980-execution issue.** The port fills
>    each column as ONE solid span (`proj[0][idx]=255` for typical heights → depth-0 fills the whole
>    column) → only **5 dark distinct colours (84-132)** vs `tile_o`'s **63 (123-252)**. This reproduces
>    faithfully whether the proj is the injected capture table OR rebuilt by the port's own `395e`, and
>    whether the HM is the capture's or the port's own. It points to a subtle 6980 fill/occlusion
>    reconstruction detail or a globals↔`tile_o` frame-skew in the capture bundle — **NOT the map-load.**
>
> Engine PRISTINE (`61453e42`/`0051cb56`/`75c6d726`/`1e0cfd38`). Only `tools/native_main.c` diagnostics
> (all default-OFF) + one banked sample changed. No regression: `make check` clean, dedicated build
> default boot alive (rc=124), `mainmenu` flow AE=0.

## Repro
```bash
NATIVE=/tmp/fist_hm make native
SCR=$(mktemp -d); cp -a armoredfist "$SCR/af"
MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0"

# (1) dump the port's OWN op-0x18-decoded heightmap ([0x85bc] first 1MB, 6980's addressable window)
setarch -R env FIST_DATADIR="$SCR/af" FIST_COOP_TICK=1 FIST_TICK_HZ=25000 FIST_MOUSE="$MOUSE" \
  FIST_HMDUMP=/tmp/port_HM.bin FIST_HMEXIT=1 /tmp/fist_hm 2>&1 | grep -i hmdump

# (2) distinct-counts of every terrain buffer after 89b0
setarch -R env FIST_DATADIR="$SCR/af" FIST_COOP_TICK=1 FIST_TICK_HZ=25000 FIST_MOUSE="$MOUSE" \
  FIST_MAPPROBE=1 FIST_MISSFB=/tmp/pf.ppm FIST_MISSFB_N=1 /tmp/fist_hm 2>&1 | grep mapprobe

# (3) drive the REAL port 6980 with the port's OWN HM + the LIGHT reduce, compare vs the DENSE tile_o
FIST_6980PROVE=1 FIST_6980PROVE_HM=/tmp/port_HM.bin /tmp/fist_hm 2>&1 | grep -iE "vs-DENSE|value-set"
```

## (a) The port's op-0x18 map-load — HM byte-match + reduce collapse

`op 0x18` runs `84c0` (task-setup allocator) + `89b0` (map-load): opens `D32.KLC`→`[0x85bc]` (heightmap),
`C32.KLC`→`[0x85bc]+0x100000` (colormap), `532.pal`, `5.SKY`. `detail[0x8490]=0x0b dim[0x8494]=2048`.

`FIST_MAPPROBE` after `89b0`:
```
[mapprobe] 85b8(colormap) ptr=08704200 distinct=89   nonzero=4194304/4194304   <- the LIGHT reduce slot: COLLAPSED (should be 254)
[mapprobe] 85bc(height)   ptr=08304200 distinct=117  nonzero=4190492/4194304   <- D32.KLC decode (4 MB @ dim 2048)
[mapprobe] 85bc+1M        ptr=08404200 distinct=105  nonzero=1047525/1048576   <- 6980's colour slot = DARK C32
[mapprobe] bc90(blendmtx) ptr=082c0000 distinct=256  nonzero=65505/65536       <- block A preload OK
[mapprobe] 3918(tile)     ptr=082f0000 distinct=175  nonzero=65536/65536
```

**HM byte-match (decisive):** the port's `[0x85bc]` first 1 MB vs the capture HM
(`voxel6980_framematched_pass08`):
```
byte-identical 1044432/1048576 (99.60%)
port HM: distinct=82 min=15 max=96 mean=53.2 nz=1048576   ==   cap HM: distinct=82 min=15 max=96 mean=53.2 nz=1048576
```
6980 addresses the heightmap as `hm[(v>>22)<<10 | (u>>22)]` = a 1024×1024 (1 MB) window. The map is
decoded at `dim=2048` (4 MB) but the first 1 MB — what 6980 reads — matches the oracle byte-for-byte
(only 4144/1048576 bytes differ, and those 4144 bytes do not change 6980's output at the capture camera
pose). Banked: `tools/oracle/samples/port_op18_heightmap_85bc_1mb.bin.gz`.

**Light reduce (`[0x85b8]`): collapsed to 89 distinct** (oracle 254). Documented root cause
(`memory: colormap-groundtruth` part 2): the shim aliases `[bc90]`→tile3918 across `89b0`, so `bdc4`
reads the tile, not block A. **The crude "skip the alias" (`FIST_NOTILEALIAS`) does NOT fix it** (stays
89) — it lets `bc9c` overwrite block A. The faithful fix needs a mid-`89b0` hook that restores
`[bc90]`=block A *between* `bc9c` (tile) and `bdc4` (reduce). **BUT this is not the current colour
blocker** (see (c)): even feeding 6980 a byte-exact 254-distinct LIGHT reduce yields only 5 dark colours.

## (b) Driving the port's REAL 6980 with the port's OWN buffers — over-draw UNCHANGED

`FIST_6980PROVE=1` runs the port's `m_ext_FUN_0000_6980` (patch 342) over the frame-matched inputs.
`FIST_6980PROVE_HM=/tmp/port_HM.bin` swaps in the port's OWN op-0x18 heightmap.

| inputs | vs sparse shadow (old metric) | vs DENSE tile_o (real) |
|---|---|---|
| capture HM + LIGHT reduce | tile_nz 38760 vs shadow_nz 7585 → "over 5.11×" | 1.46 % exact; rows0-159 cover 97.8 %; 5 distinct 84-132 |
| **port's OWN op-0x18 HM** | **byte-IDENTICAL** (38760 / over 31496) | **byte-IDENTICAL** (1.46 %; 97.8 %; 5 distinct) |

Swapping the capture HM for the port's own contiguous HM changes **nothing** (the HM is 99.6 % identical),
so it cannot fix the "over-draw." **The premise is debunked.**

**Why the shadow is not ground truth:** the bundle also carries the DENSE render-time tile `tile_o`
(distinct 66, nz 65536, mean 199) — the actual tile 9200 samples. The "shadow" = `tile_o` with 88 %
zeroed:
```
tile_o : runs=190  1px=0%   mean_run=256  nz=65536   (dense solid columns)
shadow : runs=5053 1px=75%  mean_run=1.5  nz=7585    (sparse; ALL 7585 nz are an exact-value SUBSET of tile_o)
```

## (c) The genuine residual — 6980 fills solid columns (colour), reproduced from byte-exact inputs

Row-band breakdown, port vs `tile_o`:
```
rows[0:160]  port_nz=29735 tileo_nz=30400 cover 97.8%  exact 958(3.2%)  port distinct=5 (84-132)   tileo distinct=63 (123-252)
rows[160:256]port_nz=9025  tileo_nz=18240              exact 0          port distinct=1 (132)      tileo distinct=37 (133-252)  <- 689a sky (port didn't run 689a)
```
The port COVERS 6980's terrain region (rows 0-159) at 97.8 % — no over-draw. The residual is that each
port column is a SOLID span of ONE colour: `proj[0][idx]=255` for idx 1-127, and `idx=(L0+h)&0xff` with
`L0=207` maps the dominant heights (49-67) to idx 2-18 → `proj[0]=255` → depth-0 fills the whole column,
subsequent depths occluded. `tile_o` columns are depth-structured (col128: 169,179,…,212,225,…). Because
the port samples ONE coord per column (depth-0) it emits only 5 distinct colours; `tile_o` samples one per
depth → 63.

This is **independent of the map-load and the proj source:**
- port's OWN HM → identical (5 distinct).
- inject capture proj vs rebuild via `395e` from banked ramps (`FIST_6980PROVE_BUILDPROJ`) → identical.
- LIGHT reduce (max 228) at the colour slot → still 5 dark colours (84-132).

So the divergence from `tile_o` is either a subtle unfaithfulness in the 6980 fill/occlusion loop
reconstruction, or a globals↔`tile_o` frame-skew in the voxel6980 capture bundle (the same class of
capture frame-skew that `colormap-groundtruth` parts 5/6 repeatedly hit). **It is NOT the map-load HM, the
colormap layout, or the light reduce.**

## Verdict

- **op-0x18 map-load HM: CORRECT / solved.** 99.60 % byte-identical to the oracle render-time HM; layout
  matches 6980's addressing; feeding it to 6980 is byte-identical to the capture. It was never the
  over-draw cause.
- **"6980 over-draw": debunked** as a sparse-shadow measurement artifact. Vs the real dense tile the port
  covers 6980's region 97.8 % with no over-draw.
- **The real residual is 6980 COLOUR/depth-structure** (solid-column fill), reproducing from byte-exact
  proj/ramps/camera/HM — a 6980-execution or capture-frame-skew question, **not a map-load or reduce
  defect.**
- **Light reduce** at `[0x85b8]` is a real, separately-documented collapse (89 vs 254) needing a
  mid-`89b0` bc90→blockA hook — but it is NOT the current colour blocker (the solid-column fill gates it).

**Remaining real work is therefore NOT "the port's own map-load HM+reduce"** (the HM is correct): it is
(1) resolve the 6980 solid-column colour-structure — asm-audit the fill/occlusion loop AND/OR obtain a
genuinely frame-matched globals↔dense-tile 6980 capture (the `tile_o` in the bundle appears frame-skewed
vs the injected globals); then (2) feed the LIGHT reduce to 6980's colour slot; then (3) 689a recovery +
`3931→689a→6980` wiring before `9200` in op-0x24 + the camera bridge.

## Diagnostics added (all default-OFF, `tools/native_main.c`)
- `FIST_HMDUMP=<file>` / `FIST_HMEXIT` — dump the port's `[0x85bc]` heightmap first 1 MB (+ colormap to
  `/tmp/port_CM.bin`) after `89b0`.
- `FIST_6980PROVE_HM=<file>` / `FIST_6980PROVE_RED=<file>` — override the capture HM / LIGHT reduce fed to
  the standalone 6980 prove with the port's own buffers.
- `FIST_6980PROVE_BUILDPROJ` — force 6980 to rebuild the proj via `395e` from the banked ramps.
- `FIST_6980PROVE_ODUMP=<file>` — dump the bundle's dense `tile_o`; the prove now also reports
  `vs-DENSE-tile_o` (the real ground truth) alongside the legacy sparse-shadow metric.
- `FIST_NOTILEALIAS` — skip the bc90→tile3918 alias (reduce-source probe; corrupts the tile build).
