Type: Work item
Title: Render original terrain in a shared C inspection scene
Parent: 0049
Depends: 0052

## Contract

Render original scenario terrain and mission colors through shared C11/softgl on native and
WASM. Recover world axes/scale before placing it; make inspection-camera and visual-quality
choices explicit. Review actual native/browser output and require complete frame presentation.
This does not implement vehicle placement, playable controls or a cockpit.

## Evidence

`docs/terrain-scene.md` records original sampler/camera/projection instruction addresses and
the recovered 524288-coordinate period, reversed Y/row axis, 256-position-unit height scale and
heading basis. `verify_original_terrain_coordinates.py` executes four complete original sampler
cases and three camera setups, covering signed extremes/wrap and the original altitude clamp.

`src/render/terrain_scene.c` builds a complete periodic decoded-height grid with triangle
geometry and smooth normals, the original mapped colormap, full RGB display, directional sun,
depth and fog. The chosen inspection pose is SHDR X/Y, ground+64, north heading, pitch -0.25;
field of view is 60 degrees, fog 384→768. Optional heading supports the reviewed AZER1 pose
(unit-zero heading 26729). Mapped sky average supplies background/fog; panorama remains open.
Render workers complete before temporary buffers are freed. No offline meshoptimizer is linked.

Verified locally on 2026-10-06:

- `bash tools/build.sh all`: all four native CTests pass; WASM triangle, scenario,
  terrain-asset and terrain-scene contracts pass. Default gates deliberately skip optional
  original corpus/scene coverage; it is required separately below.
- `python3 tools/check_style.py`: all fourteen owned C units and their headers pass
  LLVM 19.1.7 strict formatting/tidy. No checks or compiler flags were weakened.
- `python3 tests/test_terrain_scene.py --originals`: six complete groups on both
  targets with no skips. Complete size/alpha, sky/textured ground, changed headings and height,
  full-period equality, signed-limit equality, invalid headings and absent required files
  are covered. Pinned AZER1 inputs are copied, validated by shared C and hash-rechecked.
- An external native AddressSanitizer/UndefinedBehaviorSanitizer build of `fist_terrain_preview`
  passed the same six scene groups including AZER1 with leak detection enabled. It uses the
  production fast-math flag; owned temporary allocations/worker completion show no errors.
- Native AZER1 PPM was visually reviewed; FNV1a `11444fd3`. Actual Chromium terrain preview
  was visually reviewed; FNV1a `41900eee`, complete PPM SHA-256
  `a8d028dd641c96106c3aea4caa3f80c344776532c88f8ef00623724afc40ee69`, 38156 distinct RGB colors,
  isolation true and no page errors. Every canvas pixel equals the completed C frame including
  alpha and row orientation. Node WASM emits the same complete PPM as Chromium.
- Constructed Chromium terrain was also visually reviewed: FNV1a `cbe478ea`, complete PPM
  SHA-256 `5e32e9dad171951c0284df3a3cf7b93e39c0e98122b884e607ae592f6bdff6f6`, 3877 colors and
  complete presentation. CI now prepares this fixture and runs the same browser gate without
  original bytes. These are local results; remote CI is not claimed.
- Existing actual-browser triangle still passes with `3fa856ed` and correct vertical colors.

Only 239 of 768000 AZER1 RGB components differ across native/WASM (237 by one and two by two),
recorded as target rasterization differences. Original or cross-target bit identity is not the
rewrite acceptance target. Neither missing output nor partial presentation passes.

Inputs/builds/captures are under `/tmp`; obsolete scratch logs/sanitizer output are removed after
verification. Browser input manifests verify full sizes and SHA-256 before decoding. Normal build
and CI bytecode caches also use `/tmp`.

## Next

This inspection step is complete. Under 0041, establish player/unit semantics and model geometry,
native SDL2/browser interactive loops, fixed-step movement/camera, HUD, combat/objectives and audio.
Original upsampling/stamps/collision sampling and sky panorama are not supplied by this preview.

## Accept

Recovered world placement and explicit inspection quality are documented. Original terrain is
visible in complete native and browser frames, with cross-target behavior, strict tooling,
memory-lifetime and actual visual evidence. Commit/push this bounded scene milestone.
