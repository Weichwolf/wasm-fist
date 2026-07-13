> **PARTLY SUPERSEDED (2026-07-13) by `docs/oracle_tile3918_producer.md`.** That doc pins the producer
> DEFINITIVELY with backtraces + a physical-stability proof and CORRECTS two premises below: (1) the tile is
> a **256×256 pairwise PALETTE-BLEND LUT** built by `bc9c` reading the **mission palette `ds:0x5598`** — the
> `bc06`/`bc5a` "averaging upsample of the colormap" is a secondary LOD block, NOT the dominant fill, and the
> source is the **palette**, not the colormap `85b8`; (2) the "paging moves the buffer" caveat is DISPROVEN —
> the tile is at a FIXED phys `0x175200` via a data-SEGMENT base `0x131000` (ext-ptr `0x44200`), stable across
> map-load and dump. The camera source (mission TCB @ phys `0x10000`, fields `+0x2c/0x30/0x34/0x38/0x3e`) is
> also captured there. Read the new doc first.

# Oracle recon — the tile-3918 WRITER traced on the original + the port-side collapse verdict

**Date:** 2026-07-13 · **Method:** instrumented-DOSBox general-memory write-trace of a live AZER1 mission
(new `host_writeb` tile hook), plus content analysis of the settled-frame RAM dump, plus a port-side
per-stage buffer probe (`FIST_MAPPROBE`). **Scope:** recon + one env-gated diagnostic seam; pristine
unchanged (`re_out/fist.c`=`61453e42`, `fist_mga.c`=`75c6d726`, `fist_ext.c`=`0051cb56`).

This ENDS the tile-3918 flip-flop (voxel-6980 vs 9e60/9ec0 static-remap) with hard evidence.

## HEADLINE — tile 3918 is a STATIC map-load buffer, written by the LOD-upsample/palette-blend cluster; the port never fills it AND collapses the colormap

### (1) The write-trace (decisive)
The instrumented DOSBox was extended to histogram writes whose physical target lands in the tile range
(`FIST_TILEPHYS`, default `0x175200` = flat `0x44200` = `DAT_0000_3918` under the `flat==phys−0x131000`
key). Two captures:

- **Armed at the SETTLED terrain frame** (`trace_terrain.sh`): **0 writes** to the tile range — while
  the VGA writer `FUN_0000_9200` fires (the 46 %-of-writes terrain writer). ⇒ **tile 3918 is NOT
  rewritten per frame; it is a STATIC precomputed buffer.** This alone kills the "per-frame voxel walk
  `6980` fills the tile" premise (op 0x08/`6980` is also never posted — see `docs/stage1.md`).

- **Armed at MAP-LOAD** (new `trace_tilebuild.sh`, arms `SIGUSR1` *before* the ACCEPT click): the tile
  range is written by **6 extender IPs, all cs=`0x2b` flat, at image offsets `0xbc2a`/`0xbc30`/`0xbc74`/
  `0xbc7a`/`0xbcf2`/`0xbcf6`** — i.e. the bodies of **`FUN_0000_bc06`, `FUN_0000_bc5a`, `FUN_0000_bc9c`**
  (`scratch/oracle/tilebuild.tilewriters.txt`). Disassembly (`re_out/fist_image.bin`):
  - `bc06`/`bc5a` = **averaging LOD upsample** (`mov (%esi),%al; add %al,%ah; rcr $1,%ah; mov %ah,1(%edi)`)
    — bilinear interpolation of a source plane, doubling resolution per axis (`[0x5578]`/`[0x557c]` dims).
  - `bc9c` (dominant, 32 k+29 k writes, tile off `[0x0e00..0xffff]`) = **palette-space colour blend**:
    `mov [0xbc90],%ecx; movzbl %cl,%eax; mov 0x5598(%eax,%eax,2),%bx …; stc; adc %bl,%al` — reads the
    **mission VGA palette at ext+`0x5598`** and blends colour pairs.
  So the tile-3918 producer is the **`FUN_0000_bdc4`-family LOD-upsample + palette-blend cluster,
  reading the COLORMAP (`85b8`) + the mission PALETTE (`0x5598`), at map-load** — NOT `6980`, NOT a plain
  `9e60`/`9ec0` remap.

  **HONEST paging caveat:** the `flat==phys−0x131000` key was proven at *dump* (settled-frame) time. During
  the map-load armed window the extender's paging can differ, so the exact source↔dest *buffer* mapping at
  build time is not pinned to `0x44200` with certainty (at dump time `DAT_bc90`=`0x10000`≠tile `0x44200`).
  What is certain: the **writer CODE** (the `bc06/bc5a/bc9c` upsample/blend cluster) and that it runs at
  **map-load**, reading colormap+palette. A CR3/page-table capture is the follow-up to pin the exact buffer.

### (2) Content analysis (independent confirmation)
Tile 3918 (256×256, 212 distinct) rendered grayscale (`scratch/oracle/tile3918_gray_big.png`) is a **smooth
depth/quadrant gradient**, whereas the colormap `85b8` (`layout/85b8_color_*.png`) is **natural rocky
top-down terrain**. Quantitatively the tile is **NOT** a downsample of the colormap (0.7 % pixel match),
**NOT** a crop (0.8 %), and **NOT** a clean palette-blend LUT (diag/symmetry weak) — consistent with it
being the upsample/blend cluster's synthesized output, not a texture copy.

### (3) The port-side collapse verdict — a REAL, two-fold bug (`FIST_MAPPROBE`, after `89b0`)
| buffer | port distinct | oracle distinct | verdict |
|---|---|---|---|
| `85b8` colormap | **36** | 254 | **COLLAPSED** (7× under-populated) |
| `85bc` heightmap | 117 | 97 | plausible |
| `85bc+1M` | 105 | 160 | height data (both — the `6980` "colour" read = height ⇒ black) |
| `bc90` blend-matrix | 55 (47 % filled) | ~254 (99 % filled) | partially built |
| **`3918` tile** | **1 (EMPTY, 0 nonzero)** | **212** | **NEVER FILLED** |

So the port's `89b0` map-load **(a) collapses the colormap `85b8` to 36 distinct** and **(b) leaves the tile
`3918` completely empty**. The tile-fill (the `bc06/bc5a/bc9c` upsample-into-`3918` at map-load) is not
driven in the port, and the colormap the fill would read is itself collapsed.

### (4) Builder-fidelity proof — the port's current `6980` is the WRONG producer
Driving the port's `FUN_0000_6980` with the **oracle camera + oracle ray tables injected** (`FIST_R3D2_V18`
+ `FIST_R3D2_TILEREF`) yields **tile-vs-oracle 0/65536 (0.0 %), distinct=30 vs 212**. Even with a perfect
camera, `6980` cannot reproduce the tile — it reads `85bc+0x100000` (heightmap, per the trace/oracle),
producing garbage. This confirms patch-287's `6980` path is not the reference-frame tile producer.

### (5) Camera diagnosis (Part 3) — 4d0e never reaches 2
`FUN_1000_6f1f` writes the per-frame camera into `d548[0x12..]` **only if `DAT_2000_4d0e == 2`**
(`re_out/fist.c:54984`). The only writer of `4d0e = 2` is **`FUN_0000_72d2`** (`fist.c:18453`, the cockpit/
first-person view setup: also sets object flag `+0x17|=2`, calls `771a`→`6e75` camera-init + `7310`). The
other view setters `72b2`/`72b5` write `4d0e = 0`. `72d2` appears **only in the icall registry**
(`fist.c:68708`) — it is dispatched data-driven (indirect) and is **never reached in the port cascade**, so
`4d0e` stays 0 → `6f1f`'s camera branch never runs → `4e1c/4e20/4e24 = 0` (matches the documented zero
camera). Root frontier: locate the mission-entry view-select that should dispatch `72d2` (place the player
in the vehicle → cockpit view). Note the player object `DAT_2000_2d34` (set at `fist.c:13139`) feeds `6e75`.

## Consequence for the port (the faithful path, now concrete)
1. **Fix the `85b8` colormap collapse** (36→~254): audit `89b0`'s `643c` decode + `9ec0`/`ac70` reduce +
   `bdc4` upsample for the port; the reduce/upsample is over-collapsing the colormap (asm-verify vs
   `re_out/fist_image.bin`). The oracle `85b8` stays 254 distinct — the reduce does NOT gut it.
2. **Drive the tile-3918 fill at map-load** = the `bc06`/`bc5a`/`bc9c` (`bdc4`-family) upsample+palette-blend
   into `3918` from the (fixed) colormap `85b8` + palette `0x5598`. This is the STATIC map-load producer the
   trace identified — NOT `6980` (retire the `FIST_R3D2_BUILD`/`6980` seam for the reference frame).
3. **Fix the camera** (reach `72d2` → `4d0e=2`) so `9200` samples the filled tile through a real projection.
   Order matters: (1)+(2) give a non-empty tile; (3) makes `9200` draw coherent terrain.

## Tooling added (reproducible)
- `tools/oracle/dosbox_vga_terrain_trace.patch` — now also hooks `include/mem.h host_writeb/w/d` (gated on
  `fist_mem_armed`) → `<prefix>.tilewriters.txt` (CS:EIP histogram of tile-range writers). `FIST_TILEPHYS`
  sets the 64 KB range (default `0x175200`).
- `tools/oracle/trace_tilebuild.sh` — arms `SIGUSR1` **before** the ACCEPT click to capture the map-load
  builder (the settled-frame `trace_terrain.sh` misses it — tile is static).
- `FIST_MAPPROBE` (env-gated, `tools/native_main.c`) — per-buffer distinct/nonzero after `89b0`.

## Reproduce
```bash
# rebuild instrumented DOSBox (adds the tile hook): patch src/hardware/vga_memory.cpp + include/mem.h, make
DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/trace_tilebuild.sh 160 100 205 128 40 186 60 12 12 20 5 8 \
   scratch/oracle/tilebuild     # -> scratch/oracle/tilebuild.tilewriters.txt
FIST_MAPPROBE=1 FIST_CASCADE=1 FIST_COOP_TICK=1 FIST_TICK_HZ=25000 FIST_RUNMS=40000 \
   FIST_MOUSE="…9-event ACCEPT route…" /tmp/fist_native 2>&1 | grep mapprobe
```
