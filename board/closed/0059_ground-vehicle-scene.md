Type: Work item
Title: Draw composed ground vehicles in the shared terrain inspection scene
Depends: 0058

## Contract

Render the roster-zero ground vehicle with original directional part selection and recovered
world texel scale on native and WASM. Use an explicit follow inspection camera and the owned
terrain mesh for placement. Preserve transparent texels and terrain depth. Missing assets or
invalid selections fail without publishing a frame. This is static visualization, not gameplay
installation, initialized suspension/cockpit behavior or a playable mission.

## Evidence

Original DOS 0731/059a computes observer-to-object bearing: zero follows +Y, quarter turn +X.
Original service 15f15..15fc9 installs C-model scales 272/272/336/304 for M1/M3/T80/BMP.
DOS de70 installs TCB ca=176. Kernel ad74..ada1 derives inverse sprite steps from their product;
horizontal stepping doubles vertical stepping. Kernel bb71..bb99 projects one height byte by
2^21/depth-table pixels. Their ratio gives vertical world units per authored texel
model-scale*176/131072, with half that horizontally. Integer projection bias/minification
limits are superseded by softgl perspective. Existing composition owns all original texels.

## Next

Complete. Continue parent 0041 with runtime vehicle installation and original movement/control
rules, followed by interactive platform loops, HUD, combat/objectives and audio.

## Accept

Both targets show the selected original vehicle over its terrain from reviewed directions;
complete frame tests prove transparency, depth, pose, scale, wrap and missing-input behavior.
Original scale/bearing evidence, required builds and strict formatting/tidy pass.


## Verification

2026-10-06: required `bash tools/rewrite/build.sh all` passes all nine native CTests and WASM
asset/scene/pixel gates. `python3 tools/rewrite/check_style.py` passes all 24 owned C units.
`python3 tools/rewrite/test_terrain_scene.py --originals` passes all ten groups on both targets,
including four original view directions and visible vehicle contributions against copied
zero-texel controls. Default gates explicitly skip the two requested-original groups only.
The optional `verify_original_vehicle_projection.py` passes 27 bearings, four actual posted
C-model scales, twelve perspective ratios and two complete height projections without hooks.
`test_model_bitmap.py --originals --oracle` passes all 34 families/6432 complete bitmaps per
target; corrected contact sheets retain identical authored bytes and appear upright.

Actual native PNG and Chromium captures are reviewed for AZER1/0, TRAIN1/32768 and INDIA3/16384;
all three browser gates match every complete C-frame/canvas pixel, isolation and no runtime
errors. A separate `/tmp/wasm-fist-0059-sanitized` ASan/UBSan/leak-detection build passes all ten
native original/constructed scene groups and the complete pixel contract. Commands, recovered
scale math, full capture hashes and intentional visual differences are in `docs/vehicle-scene.md`.
No generated engine or meshoptimizer runtime dependency is introduced. Runtime vehicle
installation/animation, other objects, playable controls, HUD, combat and audio remain open.

The first real placement exposed the earlier top-row-first description as wrong: actual
original object projection proves Y increases with height. Correct the public coordinate to
`bottom`, label storage bottom-row-first and invert diagnostic PNG presentation; full bitmap
byte expectations stay unchanged. Transparent/depth/orientation tests exercise the reaching
failure. Retain compact reviewed PNGs under `/tmp/wasm-fist-0059-review`; clean transient
inputs, PPMs, sanitizer build and logs after commit/push.
