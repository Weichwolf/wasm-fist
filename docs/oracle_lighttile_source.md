# Light-tile source hunt — the render-time tile @ ext-flat 0x44200 is COMPUTED per-frame by FUN_0000_689a, not preloaded

> ## VERDICT (2026-07-17) — **The LIGHT tile 9200 samples at render time is NOT a preload (block-A style) and NOT bc9c's static blockB. It is REBUILT EVERY FRAME by the extender's terrain-fill pipeline: the dominant writer is `FUN_0000_689a` (a per-frame PERSPECTIVE RESAMPLE of a static source buffer `ds:0x3911`), overlaid by `FUN_0000_6980` (the voxel-column walk). The port reproduces 0% of it because (a) it never runs the fill pipeline (its op-0x24 render is `8deb→85d0→8120→9200`, no tile-fill) and (b) `FUN_0000_689a` is ABSENT from the port's decompile. This is a multi-part pipeline reconstruction, NOT a clean 1-patch base-loss nor a memcpy preload — a PRECISE PLAN + pinned source is the deliverable, no patch landed (landing anything would be a band-aid + regress the deterministic spawn).** Engine PRISTINE unchanged (`61453e42`/`0051cb56`/`75c6d726`); only DOSBox oracle instrumentation added (`[r92phys]` phys-resolution + `.src3911.bin` source dump in `dosbox_vga_terrain_trace.patch`).

**Date:** 2026-07-17 · **Method:** CR3-aware physical resolution of the render-time tile + a boot-armed
per-byte last-writer (fixed-phys AND CR3-flat, both agree) on a live AZER1 mission of the ORIGINAL under the
instrumented DOSBox, driven BATTLES→OK→ACCEPT; plus offline disassembly of `re_out/fist_image.bin` and an
offline first-principles reconstruction of the producer that reproduces 55% of the light tile from the
captured source. Supersedes the "tile @ 0x175200 = bc9c blockB" premise for the RENDER-TIME tile.

## (1) The physical page — CONFIRMED
| linear (ext base 0x10000000) | render-time phys | note |
|---|---|---|
| tile `0x10044200` (16 pages) | **`0xb78200`** (contiguous `0xb78000..0xb88000`; last 0x200 B wrap to `0x140000`) | LIGHT tile (68 distinct, mean 200.2, min 116 max 252) |
| (fixed-phys `0x175200`) | — | DARK blockB (212 distinct, mean 128.7) — bc9c's output, a DIFFERENT buffer, 0.08% byte-equal |

Confirmed by a per-4KB-page search of the render-time RAM dump for the banked light-tile content: every
one of the tile's 16 linear pages maps to a distinct, CONTIGUOUS physical page at `0xb78xxx` (~12 MB =
extended memory, PM-only), byte-identical to `oracle_9200_framematched_pass08.cap`'s tile. The extender's PM
paging remaps the tile's linear pages to `0xb78200` at render time, NOT `0x175200` (the paging confound
`colormap-groundtruth` warned about is exactly this — every prior fixed-phys `0x175200` tile watch was the
WRONG page). The old cr3walk sample watched flat `0x44200` (identity, paging off) and caught **16-bit engine**
writers (cs 4ec3/4ab0) — the wrong address space; the extender PM tile is at linear `0x10044200` (cs base
`0x10000000`).

## (2) The last-writer / source — PINNED
Boot-armed watch on phys `0xb78000` (per-byte last-writer) AND CR3-flat on linear `0x10044200` — both
independently identify the SAME producers (all cs=0x2b extender PM, counts are per-frame so ENORMOUS):

| ext IP | function | tile bytes owned | role |
|---|---|---|---|
| **`0x6964`** | **`FUN_0000_689a`** (entry 0x689a) | dominant (flat: full 0..0xffff, 88 M writes) | **`stosw` in the perspective RESAMPLE loop** |
| `0x6b2b` / `0x6bab` | `FUN_0000_6980` | ~23% (0x2100..0xdefe) | the voxel-column walk (writes tile via `4a60[]` row-ptrs) |
| `0x0b006` | (minor) | 10 bytes | stray |

- **NOT bc9c** (0xbcf2/0xbcf6, which owns blockB @ 0x175200 — a different buffer). **NOT a preload** (block A
  had 0 writers during the cascade + a single boot memcpy; the light tile has 88 M per-frame writes).
- **`FUN_0000_689a` (the producer, disasm 0x689a..0x6976):** computes per-column steps from the projection
  globals (`90c0·90b0`, `90b4`, `90dc`, `90e0`), then fills the tile `edi=ds:0x3918` by resampling a SOURCE
  `esi=ds:0x3911` at a projection-dependent fixed-point coordinate: `tile[col][j] = source[ (edx>>shift) +
  (ebx>>25) ]`, `ebx += ebp` per texel, `edx += 90bc` per column. It is invoked per-frame via the render
  entry **`FUN_0000_3931 → 85d0(camera) → [0x3958] → 6980`**, where `[0x3958]` is a DISPATCH set by the
  selector at ext `0x7660`: **`[0x3958]=0x689a` (resample) iff the current TCB's byte `[TCB+0xcc] != 0`**
  (detail/texture flag), else `0x6877` (a constant memset of the tile). The AZER1 spawn TCB has `[+0xcc]!=0`
  → the resample (light tile) is selected.

## (3) The source `ds:0x3911` — a STATIC map-load buffer (one level deeper)
| property | value |
|---|---|
| content | 256 KB, 253 distinct, mean 177; **100% frame-invariant across all captured passes** (map-load buffer, not per-frame) |
| structure | 4×64K: block0/1 = LIGHT terrain colormap (28/30 distinct, mean 224/229) · block2/3 = dark-rich (251/248 distinct, mean 128) |
| physical | contiguous at phys **`0x978e60`** (page-offset `0xe60` = the SAME allocation family as colormap `85b8`=`0x474e60`), NOT blockA/blockB (≤1.5% byte-equal) |

**Offline mechanism proof:** a cold first-principles reconstruction of `FUN_0000_689a` fed the banked source
+ the `.cap` projection globals (90c0=0x01000000, 90b4=0x00020000, 90b0=0x3d00, 90dc=0x64000000,
90e0=0x97970000) reproduces **55.55%** of the light tile at column-shift 10 (mean 224, 28 distinct = source
block0). The residual is the `6980` voxel-walk overlay (0x6b2b/0x6bab, ~23% of bytes, samples the heightmap +
darker source blocks) + the exact runtime column-shift/base. This DEFINITIVELY confirms the light tile =
`689a`-resample(source `ds:0x3911`) + `6980`-voxel-walk. Repro: `tools/oracle/samples/`
`oracle_lighttile_source_ds3911_static.bin`.

## (4) Why the port renders dark + the FIX (precise plan, NOT landed)
The port's default op-0x24 render is `8deb→85d0→8120→9200` (native_main.c ~1194) — it NEVER runs the
tile-fill (3931/689a/6980), so `9200` samples the stale dark tile (bc9c blockB / the shim's `[bc90]`→3918
alias). The framematched inject already proved: inject the light tile → `9200` reproduces **93.44%**; the
port's own dark tile → **0.02%**. So the tile is THE dominant lever and the fix is to make the port build it.

**The faithful fix is a multi-part pipeline reconstruction (each part is bounded, none is a band-aid):**
1. **Recover `FUN_0000_689a`** into `re_out/fist_ext.c` — it is ABSENT (the port has only 686f/6877 const-fills
   + 6977/6980). Add 0x689a to the ext seeder and re-decompile (it is a clean ~40-instruction resample loop).
2. **Build the static source `ds:0x3911`** at map-load. It is a 256 KB map-load buffer (phys 0x978e60, colormap
   `85b8` family) — the NEXT-level last-writer hunt (boot-armed watch on phys 0x978e60, block-A method) pins
   its builder. The port must reproduce it before the render.
3. **Run the fill pipeline per-frame before `9200`:** `3931 → 85d0 → (select [0x3958]=0x689a when TCB[+0xcc]!=0)
   → 6980`, wiring the detail selector at ext 0x7660 (sets `[0x3958]`, `[0x395c]` from `TCB[+0xcc]`).
4. **Correct projection globals** — `689a`/`6980` read `90xx` from the camera via `85d0/8120`; this is GATED
   on the known SECONDARY defect (mission render-camera TCB `+0x3a/+0x3c` pitch/roll stale, `a20d/a192/0459`
   bridge threads only heading `+0x38` — paged-out extender bridge, not a clean base-loss).

Because (1)+(2)+(4) are each real work and (4) is itself blocked, landing a partial fix now would regress the
deterministic spawn md5 with no colour benefit (proven pattern: component fixes don't compose while the tile
is dark). Deliverable = this pinned plan; the immediate next step is the **phys-0x978e60 source-builder hunt**.

## Repro
```bash
# instrumented dosbox (README_terrain.md) with the light-tile phys-resolution + source dump:
FISTLOG=scratch/oracle/lt DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/capture_9200_framematched.sh
#   -> lt.passNN.src3911.bin (the 0x689a source) + lt.pass00.ram.bin (render-time RAM); [r92phys] = tile/src phys
# last-writer on the render-time tile page (both watches agree = 689a/6980):
FIST_MEMARM_BOOT=1 FIST_TILEPHYS=0xb78000 FIST_WATCHFLAT=0x10044200 FIST_WATCHFLATSPAN=0x10000 \
  DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/trace_bc90.sh 160 100 205 128 40 186 60 12 12 30 5 10 scratch/oracle/ltw
# offline: disassemble the producer + selector
objdump -b binary -m i386 -M intel -D --start-address=0x689a --stop-address=0x6977 re_out/fist_image.bin
objdump -b binary -m i386 -M intel -D --start-address=0x7660 --stop-address=0x76a0 re_out/fist_image.bin
```
