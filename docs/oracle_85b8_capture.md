# Oracle 85b8 colormap LIVE capture — 85b8 is COMPUTED by bdc4, and it is NOT the windshield terrain-colour residual

> ## VERDICT (2026-07-17, the 85b8 "89 vs 254 collapse" frontier, settled by a live boot-arm capture + a direct port build+measure)
>
> 1. **Oracle 85b8 = 254 distinct. It is COMPUTED by `bdc4`, NOT preloaded.** (Answers the task's central
>    question: computed, not a memcpy/preload like block A.)
> 2. **The port's 85b8 collapse (89 distinct) IS root-caused: the shim aliases `[bc90]`→tile3918 across the
>    WHOLE 89b0 build, so `bdc4` reads the tile (176-distinct symmetric) instead of the preloaded block A
>    (256-distinct asymmetric DSOUNDS matrix).** Restoring `[bc90]`=block A before the bdc4 upsample →
>    port 85b8 89→256 distinct (the richness is restored).
> 3. **BUT: fixing 85b8 (89→256) produces a BYTE-IDENTICAL spawn frame (0 / 192015 bytes changed), and the
>    windshield renderer `FUN_0000_9200` contains NO reference to 85b8.** 85b8 feeds only `89b0` (the build),
>    `8fa0` (rotated variant) and `93c0` (the overhead MAP-VIEW). **So 85b8 is NOT the windshield
>    terrain-colour residual** — the task's/memory's premise ("render_index = tile[depth][col], col from
>    85b8") is DISPROVEN by measurement. The windshield colour residual = the TILE (3918, port bc9c 176 vs
>    oracle 212 distinct) + the DAC palette + the camera/projection. **Redirect the colour frontier there.**
> 4. The bdc4→blockA fix is ALSO not byte-oracle (port 256 vs oracle 254; first16 `8c1571…` vs `79797c…`;
>    1.09% byte-equal) — a SECONDARY residual in the base colormap that bdc4 upsamples. Not chased: it has
>    zero render effect (point 3).

**Date:** 2026-07-17 · **Method:** the instrumented DOSBox (`tools/oracle/dosbox_vga_terrain_trace.patch`
CR3-aware flat + per-byte last-writer hooks, `tools/oracle/trace_flat3918.sh` / `trace_bc90.sh`) driving
BATTLES→OK→ACCEPT into AZER1, PLUS a direct port build with `bdc4` reading block A (measured, then reverted
— engine PRISTINE `61453e42`/`0051cb56`/`75c6d726`, no patch committed for a zero-render-effect change).

## Address model (confirmed)
- Ext data-seg base `0x131000`, paging OFF at map-load (flat==phys, identity map confirmed by the CR3 walk).
- `[85b8]` value = **0x474e60** (stable across activate/aa10/flat3918 settled dumps AND the live trace) →
  colormap buffer CPU linear/phys **0x5a5e60**. It is the 2048²=4 MB colormap, directly ABOVE the heightmap
  `[85bc]`=0x74e60 (linear 0x1a5e60); `0x74e60 + 0x400000 = 0x474e60`.
- `[bc90]`=0x10000 (block A, phys 0x141000), `[3918]`=0x44200 (block B = the terrain tile).

## Oracle 85b8 — captured
| what | value |
|---|---|
| distinct | **254** |
| first16 | `79 79 7c 85 85 85 85 91 91 91 91 91 85 85 85 85` |
| CPU linear | 0x5a5e60 |
| committed sample | `tools/oracle/samples/oracle_85b8_colormap_first64k.bin` (first 64 KB, md5 b7c05c33…) |

## Last-writer trace — 85b8 is COMPUTED by bdc4
- **Map-load-armed** (SIGUSR1 before ACCEPT, CR3-aware flat watch on 0x5a5e60): **0 distinct writer IPs**.
  This initially looked "preloaded", but it only means 85b8 is built BEFORE the ACCEPT click (during the
  OK/briefing op-0x18 map-load), outside that arm window.
- **Boot-armed** (`FIST_MEMARM_BOOT=1`, per-byte last-writer over `[0x5a5e60, +64K)`, `tools/oracle/samples/
  oracle_85b8_lastwriters.txt`): **4 owning IPs — extip 0xbe6c / 0xbe80 / 0xbeb4 / 0xbeb8, all INSIDE
  `bdc4` (0xbdc4..0xbed1)** — bdc4's two-pass upsample inner stores. So 85b8 is the `bdc4` spatial-upsample
  OUTPUT, computed per-map. This mirrors the block-B (tile) capture (bc9c's inner stores own the tile) and
  contrasts with block A (0 writers → the DSOUNDS memcpy preload).

## Port collapse root cause (asm-cited)
`89b0` builds 85b8: (1) `9f70`→`bc9c` writes the pairwise palette-blend matrix M into `[bc90]&0xffff0000`
(bc9c disasm 0xbc9c `mov ecx,[0xbc90]`; stores `mov [ecx],bl` @0xbcf2/0xbcf6); (2) `bdc4(85b8)` upsamples
the reduced colormap 512²→2048², indexing the SAME `[bc90]&0xffff0000` blend matrix (bdc4 disasm 0xbdc4
`iVar6=[bc90]`, read `CONCAT21(high16([bc90]),texel)<<8`). There is **NO `mov [0xbc90],…` in the entire
extender image** (full objdump scan) — the extender juggles `[bc90]` between the tile (block B, for bc9c)
and block A (the DSOUNDS preload, for bdc4); the oracle's live `[bc90]` is the tile during bc9c and block A
during bdc4. The port's `84c0` allocates block A `[bc90]` and the tile `[3918]` as SEPARATE flat buffers,
so the shim aliases `[bc90]`→tile3918 for the whole 89b0 build (native_main.c op-0x18) — held across BOTH
stages → bdc4 reads the tile (port bc9c output: 176 distinct, diag all-80) → 85b8 = **89**.

### The fix that restores the richness (measured, reverted — zero render effect)
Restoring `[bc90]`=block A before the bdc4 loop (block A byte-EXACT in the port: `fist_preload_blockA`
DSOUNDS.BIN dump == `oracle_bdc4_matrix_blockA_0x141000.bin` **65536/65536**) → port 85b8 **89→256 distinct**.
NOT committed because:
- **256 ≠ 254** and only **1.09% byte-equal** to the oracle 85b8 (base-colormap secondary residual), and
- **the spawn frame is byte-IDENTICAL with/without it** (0/192015 bytes) — `9200` (windshield) never reads
  85b8 (it SMC-samples the tile 3918 + projection globals 90xx). 85b8 feeds 8fa0/93c0 (map-view), not the
  windshield, and the map-view is not drawn in the spawn frame.

## Measured terrain (spawn post #1, vs `ref/mission_azer1_spawn_native320.png`, rows 8-88 mean + full-frame sum-AE)
| build | terrain mean (R,G,B) | full-frame sum-AE |
|---|---|---|
| oracle spawn ref | (122.5, 111.9, 89.0) | — |
| baseline (default) | (106.1, 77.9, 57.6) | 4 647 202 |
| **85b8-fix alone** | (106.1, 77.9, 57.6) — **byte-identical to baseline** | 4 647 202 |
| 85b8-fix + PAL5260 + TILEWRAP=0xf200 | (71.3, 59.9, 46.1) — FARTHER (regresses) | 5 248 012 |

The combined case reproduces the documented non-composition (memory pin colormap-groundtruth UPDATE): the
byte-exact components do NOT compose because 85b8 is not on the windshield render path in the first place.

## Repro
```bash
# oracle 85b8 boot-arm last-writer (bdc4 owns it):
FIST_MEMARM_BOOT=1 FIST_TILEPHYS=0x5a5e60 bash tools/oracle/trace_bc90.sh 160 100 205 128 40 186 55 12 12 30 5 8 scratch/oracle/cm85b8boot
# oracle 85b8 settled read: scratch/oracle/*.ram.bin, [85b8]@ext+0x85b8 -> value+0x131000, 4MB, distinct=254
# port 85b8 (default = 89): FIST_CMDUMP=/tmp/cm.bin on the mission flow; the bdc4->blockA fix -> 256 (no render change)
```
