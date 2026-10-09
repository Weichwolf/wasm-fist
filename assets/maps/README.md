# Owned map recipes

`training-valley.json` reconstructs broad D32 terrain geography as 1936 analytic
hill/valley features, then deforms contours with explicit periodic JSON domain-
warp octaves and adds owned procedural detail, hydraulic erosion, climate,
materials and lighting. Optional original development observations
guided geography and visual tuning; no original height/color raster or game-file
path is stored or read during generation. Noise and final texturing are new.

The generated training baseline is 1024x1024, with 16-bit height and RGB color.
It is an iterative development asset, not final visual approval or completed
native/WASM runtime integration. Active 0131 tracks original-reference comparison,
remaining local relief/material defects and the other terrain families. See
`../generator/README.md` for parameters and reproduction.


## Family authoring coverage

| Owned recipe | Optional development geography reference | Character |
| --- | --- | --- |
| `rocky-highlands.json` | D03 | Olive lowlands and warm rocky highlands. |
| `temperate-forest.json` | D06 | Green lowlands, brown uplands and exposed gray rock. |
| `arid-ridges.json` | D07 | Bright dry valleys and darker ridges. |
| `sandy-desert.json` | D08 | Warm sand and exposed brown rocky uplands. |
| `dry-mountains.json` | D12 | Brown lowlands and pale mountain surfaces. |
| `limestone-valleys.json` | D30 | Beige terrain with darker steep rock faces. |
| `snowy-alpine.json` | D31 | Height/climate-dependent snow and exposed steep rock. |
| `training-valley.json` | D32 | Brown hill/valley terrain and fine procedural relief. |

Each recipe defines complete terrain, erosion, water, climate, palette, material,
lighting and detail controls. Geography is analytic and generation reads no
original raster or game file. The authoring profiles are development baselines;
family coverage does not establish final visual similarity or runtime acceptance.
Use `../generator/generate_maps.py` to generate every family in sequence. Review
full-resolution and near-ground native/browser results before final approval.
