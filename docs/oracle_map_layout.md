# Oracle recon — the voxel MAP-BUFFER LAYOUT, resolved from the ORIGINAL

**Date:** 2026-07-13 · **Method:** flat→physical resolution of the instrumented-DOSBox RAM dump
(`scratch/oracle/terrain.ram.bin`, from `tools/oracle/trace_terrain.sh`) of a live **AZER1** mission ·
**Scope:** read-only recon, NO port/patch changes (pristine `re_out/fist.c` = `61453e42`,
`fist_mga.c` = `75c6d726`, `fist_ext.c` = `0051cb56` — unchanged).
**Reproduce:** `python3 tools/oracle/capture_map_layout.py`

## HEADLINE — the "+1 MB co-location, 1024²" premise is WRONG; the real map is 2048², colormap +4 MB

The prior iteration's premise (and the task brief) was: *"in the original DOS extender the RAW colormap
sits EXACTLY 1 MB after the heightmap; the map is 1024²; `FUN_0000_6980` reads `byte[85bc+0x100000]`."*
**The oracle contradicts this decisively.** Measured on a settled AZER1 terrain frame at **detail 11**:

| symbol | meaning | value | note |
|---|---|---|---|
| `DAT_0000_85bc` | heightmap base (flat) | `0x00074e60` | 4 MB buffer |
| `DAT_0000_85b8` | colormap base (flat) | `0x00474e60` | 4 MB buffer |
| **`85b8 − 85bc`** | plane separation | **`0x400000` (4 MB)** | **NOT 0x100000** |
| `DAT_0000_8494` | map dimension | `0x800` = **2048** | **NOT 1024** |
| `DAT_0000_8498` | plane size | `0x400000` (4 MB) | = 2048² |
| `DAT_0000_8490` | detail level | `11` | high detail |
| `DAT_0000_3918` | 256×256 terrain tile | `0x00044200` | the tile `9200` samples |

### The flat→physical key (the CR3 resolution the raw dump lacked)
The Doug-Huffman extender's flat 32-bit linear space maps **`flat_linear == guest_physical − 0x131000`**
in this capture. Proven two ways: (1) `DAT_0000_3918`'s stored value `0x00044200` reads back byte-exact at
guest-phys `0x131000+0x3918`; (2) the 256×256 tile at guest-phys `0x131000+0x44200` is a **212-distinct**
terrain texture (a real colormap tile), not noise. So a flat pointer `P` points at guest-phys `0x131000+P`.
This is the paging resolution the earlier note (`docs/oracle_terrain_writer.md`, "HONEST caveat") flagged as
missing — for the low-mem buffer region it is a clean constant offset, so the buffer CONTENTS are now solid.

## Plane contents (distinct / dominant index) — what each buffer actually holds

```
heightmap  85bc          [+1MB]: distinct=97   dominant idx 45-51   (a smooth heightfield)
6980 color 85bc+0x100000 [+1MB]: distinct=160  dominant idx 60-67   (STILL HEIGHT data, renders BLACK)
colormap   85b8          [+4MB]: distinct=254  dominant idx 128-163 (the desert palette — the REAL colormap)
tile       3918          [64KB]: distinct=212  dominant idx 37-249  (the built terrain texture)
```

- `85bc` grayscale-renders as a coherent smooth heightfield (`scratch/oracle/layout/85bc_height_1024.png`).
- **`85bc+0x100000` — the exact address `FUN_0000_6980`'s binary reads (`6b1a: mov al,[esi+ecx*1+0x100000]`,
  a literal, 1024² index `(v>>22)<<10|(u>>22)`) — holds HEIGHTMAP bytes (60-67), which the mission palette
  maps to black.** It is NOT the colormap.
- The real colormap is at `85b8 = 85bc+0x400000` (idx 128-163, desert khaki/olive).
- The rendered tile `3918` = **212 distinct** (the step-2 validation target the task names) — its broad range
  (37-249) matches a full colormap texture, NOT the 160-distinct height plane at `85bc+0x100000`.

## What this means — the frontier is REORIENTED (detail-dependent geometry)

**`FUN_0000_6980` reading `85bc+0x100000` cannot produce coherent terrain against this layout** — that
address is inside the heightmap. The reconciliation is **detail-dependence**: `85bc`/`85b8` are allocated at
a detail-driven dimension (`8494`), and this capture is **detail 11 → 2048², planes 4 MB apart**. The
`6980` `+0x100000`/1024² path is a **LOWER-detail LOD** (where a 1024² map would put the colormap exactly
+1 MB); it is **not the renderer of the reference frame**. The reference frame (detail 11) is drawn by the
oracle-proven opaque LOD **`FUN_0000_9200` (op 0x09), which samples the 256×256 tile `DAT_0000_3918`**
(`docs/oracle_terrain_writer.md`), and that tile is filled from the real 2048² colormap `85b8` — NOT from
`85bc+0x100000`.

### Consequence for the port (honest, load-bearing)
- The prior `FIST_R3D2_BUILD` seam drives `6980` (the +1 MB / 1024² low-detail builder). Feeding it the
  detail-11 2048² buffers is a **geometry mismatch** — even with a port-side "co-location", it would sample
  height as colour and DIVERGE from the original. **A `+1 MB` co-location fix would build coherent-looking
  terrain that is NOT what the original renders — so it was NOT implemented.**
- The faithful path for a bit-identical detail-11 frame is: **(1)** load `C32.KLC` into `85b8` as a 2048²
  254-distinct colormap (the port previously had `85b8` = only `{0,4}` — the real data gap), **(2)** fill the
  256×256 tile `3918` (target 212 distinct) from `85b8` via the remap path (`9e60`/`9ec0`/`ac70`), **(3)**
  drive `9200` (op 0x09), **(4)** fix the zero camera source.
- **Camera source confirmed zero in the port** (this iteration, `FIST_R3D2` engine-cam probe on the live
  cascade): `4e1c=4e20=4e24=4e34=0`, `7b1c(head)=0`, `4d0e=0` — the `2d34→6e75→4e1c/20/24→6f1f→d548→dd15`
  chain delivers a degenerate camera, matching the documented frontier.

### Open follow-up (needs a LIVE function trace, not a static snapshot)
To confirm which IP fills tile `3918` at detail 11 (and whether `85bc` is ever repointed to a co-located
1024² voxel LOD at lower detail), extend `tools/oracle/dosbox_vga_terrain_trace.patch` to histogram writes
to guest-phys `0x175200` (= flat `0x44200` = tile `3918`) in addition to the `0xA0000` framebuffer, and take
a second capture at a LOW detail level to measure the 1024² layout directly.

## Reproduce
```bash
# (1) regenerate the RAM dump from the ORIGINAL under instrumented DOSBox (~2-3 min):
DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/trace_terrain.sh
# (2) measure the layout:
python3 tools/oracle/capture_map_layout.py
```
