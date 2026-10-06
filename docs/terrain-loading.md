# Scenario terrain bundles and mission palette mapping

`fist_terrain_load` resolves a decoded scenario's four BINF names through a storage callback.
Height, color and sky files are required. A directly available palette is used first; only a
file-not-found result permits lookup of that member in `PAL.RES`. Read errors and corrupt direct
palettes fail rather than falling back. Archive interpretation stays in the shared asset layer.

The callback view may expire on the next read. Each input is decoded/copied before another read;
the final bundle owns all three planes, the prepared mission palette and two mapping tables.
Raw KLC indices/embedded palettes are retained. The renderer uses the separate maps to resolve
color and sky indices into mission colors. No source, file handle or scenario view is retained.
Any failure destroys partial allocations and leaves the caller's destination unchanged. Map
planes must be square powers of two, matching the original wrapped sampling domain.

## Original preparation and mapping

Fresh instruction evidence uses the frozen kernel image/hash from [terrain format](terrain-format.md).
The normal mission band comes from engine `db47` in `re_out/fist_dat_image.bin`: it writes `0x50`
to TCB+0x54. Kernel `8b34` reads that field, and `8b58–8b5d` loads the named PAL and prepares it.

`9f10` selection-sorts indices 80–255 by `R + 2G + B`, leaving the reserved prefix unchanged.
It selects the first minimum and swaps colors; equal-luminance order is therefore not stable.
`a033` builds the search palette by shifting DAC6 components right once. Colormap and sky KLC
palettes pass through `4a3c` (RGB8 >> 2), then `9e60`/`ac70` build their mapping tables.

Index zero maps to zero. Each other source color searches mission indices 80–255. The distance is:

```text
sum(channel_weight * (floor(mission_DAC6 / 2) - floor(source_RGB8 / 8))^2)
channel_weight = [961, 1849, 676] = [31^2, 43^2, 26^2]
```

These weights are verified against all 256 entries of the original tables at a060/a460/a860.
Strict improvement selects the first minimum, including equal reduced colors. C uses direct
integer arithmetic instead of guest registers, distance tables or self-modifying instructions.
This preserves palette selection while allowing the new renderer to output full RGB later.

The instruction oracle executes original `9f10`, `a033`, `4a3c` and `9e60`/`ac70`, including its
actual self-modifying search instructions. ESI must be restored to 5598 for `9e60`, as supplied by
the original caller at `9ecc`; omitting that input would quantize unrelated memory. No instruction
hook, cache workaround or replacement search supplies the expected output.

## Verification and scope

`bash tools/rewrite/build.sh all` runs complete bundle and palette contracts on both targets.
Fixtures cover direct/archive resolution, storage-view destruction, missing files, corrupt and
trailing inputs, read errors without fallback, missing archive members, unsupported map dimensions,
reserved colors, selection-sort ties, quantization and first-minimum mapping. Failed output remains
unchanged and owned planes are destroyed twice safely.

The optional original gate now also compares all 47 complete mission bundles against original
instruction output: three complete planes and embedded palettes, the prepared DAC6 palette and
both 256-entry maps. It requires all pinned FSG/KLC/SKY/PAL.RES files, rejects skips and rechecks
their hashes after all uses. There are 19 case-normalized asset tuples, represented by 28 spelling
variants in BINF, and 38 distinct image/palette mapping pairs. The distinction follows original
DOS filename folding; spelling variants are not additional physical assets.

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_terrain_assets.py --originals
```

See [oracle setup](terrain-format.md#verification). Native/WASM, LLVM 19.1.7 formatting/tidy and
the complete original suite passed on 2026-10-06. Terrain geometry, camera, mission installation,
stamps and simulation remain separate work; loading a terrain bundle is not gameplay completion.
