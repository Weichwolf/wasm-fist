Type: Work item
Title: Render complete owned high-precision terrain on native and WASM
Depends: 0133 0134

## Contract

Consume the eight versioned owned FMAP maps in a shared C11 scene, without
original terrain images, palettes, scenarios, sprites or downloads. Preserve
all 16 height bits and RGB bytes. Render at 640x360 with actual 4x MSAA and three
started helpers plus the caller through the owned 0134 renderer profile.

Use `fist_owned_terrain_surface` as the canonical triangle surface owner.
World-coordinate conversion stays under the existing world contract. Define
authoring-Y, render-Z and camera orientation explicitly, and prove both periodic
seams, negative positions and heading changes. Contact/gameplay consumption must
use this same surface contract when reached; rendering is not their acceptance.

Own terrain geometry/texture lifetime separately from a frame. Load/create once,
retain across draws and release completely on unload or failed installation.
Use validated geometry bounds and conservative visibility selection rather than
rebuilding/submitting the complete map every frame. Any later LOD or material
change needs visible before/after review and geometric error evidence.

## Evidence

0133 delivers all eight complete 1024-square FMAP bundles, a checked C11 decoder
with independent plane ownership, and periodic barycentric height sampling.
The maps contain explicit minimum/maximum/water metadata. Every current JSON
recipe exports heights in0..255 while preserving 16-bit fractional detail.

`src/sim/world.h` defines the map period 524288 and position scale 256, giving a
render-space period 2048. `fist_map_coordinate`, `fist_map_y_coordinate` and
`fist_map_delta` already own legacy world conversion/wrapping. The owned format
states that rows increase in authoring Y. Do not infer a new row reversal from
an old preview's screen orientation or introduce another world-unit constant.

The existing `terrain_scene.c` samples unsigned 8-bit KLC heights, converts
original palette colors, builds full geometry/RGBA data, creates a texture and
frees everything per draw. It has no owned FMAP consumer. A full 1024-square
grid has 2097152 triangles; the current complete-grid approach cannot stand in
for measured visibility/caching or the user's representative triangle budget.

The pinned texture sampler (`fragment.c`, `sg_sample_tex2d`, and the corresponding
`frag_hot.h` repeat paths) uses `u * width - 0.5` and `v * height - 0.5` for linear
filtering. A colormap sample at grid vertex (column,row) therefore needs texture
coordinates ((column+0.5)/side,(row+0.5)/side) if colors are authored at the same
sample locations as heights. Coordinates column/side,row/side blend neighboring
texels at those vertices. Specify the alignment explicitly and verify distinct
synthetic RGB samples and both repeated boundaries; do not inherit the old
preview's unexamined half-texel offset. This is owned geometry/UV configuration,
not a dependency change.

The generator's RGB output already includes recipe-controlled directional
lighting. Applying the old preview's additional fixed lighting would shade
that baked image again. Begin the comparison scene with faithful RGB display;
later dynamic lighting needs explicit unlit/albedo content and a reviewed design,
not an unexplained change of brightness. Owned skies/environment presets remain
required under 0130/0131; the nineteen observed environment combinations are
inventoried in `docs/terrain-mission-inventory.md`.

## Next

After 0134 completes, implement an owned retained scene contract and a real
native/browser preview consuming FMAP files directly. Keep storage/presentation
in platform code and geometry/surface/rendering shared. Use explicit owned scene
parameters for camera/projection/background and reproducible paths; reject invalid
inputs without changing an installed scene or publishing an incomplete frame.

Specify behavior with synthetic fractional-height/color geometry, periodic axes,
pose/orientation, source-release ownership, repeated scene/context lifecycles,
failed installation cleanup and the eight actual complete maps. Validate culling
against complete visible reference geometry, including near/frustum boundaries;
camera movement must not reveal cracks, disappearing terrain or stale textures.

Review actual native and browser full frames and motion for every family, with
near-ground and overview views. Compare owned maps against optional development
references at matching positions/frusta, and record concrete ridge/gully,
material, silhouette, water and seam defects for continued 0131 refinement.
Own recipes and final runtime assets must remain sufficient for all required runs.

Run strict LLVM19 style and the complete sequential native/WASM production gate,
then relevant memory checks. Measure warmed complete scene frames, retained
resource behavior, submitted/visible triangles and material counts with the
actual requested profile. Keep 60-FPS acceptance under `docs/render-budget.md`;
one terrain material does not cover the complete representative game workload.

## Accept

All eight owned terrains load/render without originals on both targets; complete
meaningful behavior, resource/lifetime, strict style and production checks pass;
actual native/browser frames and motion establish high-precision surface/color
consumption, correct pose/orientation and seamless conservative visibility.
Record exact commands, pins, frames and remaining quality/performance limitations.
Close this bounded scene delivery only when those checks finish. Complete-game,
all-content quality, collision/mission integration and 60-FPS acceptance remain open.
