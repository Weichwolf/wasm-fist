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
