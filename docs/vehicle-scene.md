# Ground vehicle inspection scene

The shared C renderer draws the roster-zero ground vehicle over its decoded mission terrain.
The preview resolves the default C family, selects hull/turret facing and variant independently,
composes its original sprite pieces and submits one transparent billboard to softgl. Native
and WASM use the same asset, camera, composition and rendering code; browser JavaScript only
loads verified files and presents the completed frame.

This is a static inspection scene. It does not execute original runtime vehicle installation,
suspension, animation, cockpit, movement, weapons, AI or audio. Its ground placement follows
the two triangles of the decoded preview mesh, rather than the uninitialized snapshot altitude.

## Recovered projection and deliberate quality decisions

Original DOS `0731/059a` computes observer-to-object bearing from integer X/Y differences.
Zero follows +Y, a quarter turn +X. The preview uses continuous `atan2(deltaX, deltaY)` with
these same axes, retaining the original nearest-32-facing rounding and per-part heading choice.
It deliberately replaces the original coarse atan table. Positions are periodic over 524288
original units; camera-relative placement chooses the nearest periodic image without signed
overflow, including negative and extreme original coordinates.

Service `15f15..15fc9`, called by `42cc`, installs default C-model scales 272/272/336/304 for
M1/M3/T80/BMP. `3ed4..3ef0` posts the selected table entry to TCB `480`; the snapshot's high
byte at `14` is a different node/projection input and is not this model scale. `de70..de85`
sets TCB `ca` to 176 during scene setup. Kernel `ad74..ada1` calculates the 16.16 inverse
source step as `((floor(0xffffffff/(modelScale*176))*depthTable) >> 20)`. Horizontal source
stepping doubles vertical stepping. Object projection `bb71..bb99` places a height-byte
difference at `2^21/depthTable` screen pixels. Removing fixed-point division bias yields
vertical render units per texel `modelScale*176/131072`; horizontal extent is half that.
One render unit is one height byte or 256 original horizontal units, as recovered for terrain.
The new perspective/FOV supersedes original minification limits, rounding, shear and final
device viewport scaling; these constants do not assert metre units or vehicle physics.

Actual complete `ba7d` object projections place heights 50 and 51 at frame Y 128 and 129:
original frame Y increases upward. Sprite column scans also increment Y. The composed bitmap
therefore stores **bottom row first**, with minimum authored Y named `bottom`. WI 0058's
top-row-first description and diagnostic PNG orientation were wrong, although its bytes,
anchors, bounds, sorting and transparency were correct. WI 0059 corrects the public field and
diagnostic presentation; all 6432 complete corpus bitmap byte comparisons still pass. The
earlier upside-down contact sheets are not evidence of final scene orientation.

The inspection camera is 64 render units behind the selected vehicle along the requested
heading and 32 units above its interpolated ground origin; pitch points at that origin. These
are explicit framing choices. The quad faces the camera, uses recovered origin-relative bounds,
nearest texture sampling and the original MAL RGB6 colors expanded to RGB8. Retaining MAL
colors is intentional; original mission-palette remapping/shading is not applied to the vehicle.
The existing perspective, terrain depth and distance fog apply. Alpha testing excludes index-zero
texels from color and depth writes. No guessed object fallback, opaque sprite rectangle or hidden
variant clamp replaces missing inputs. This billboard does not add a ground shadow or slope tilt.

## Reproduce

```sh
export PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
python3 tools/rewrite/test_terrain_scene.py --originals
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/verify_original_vehicle_projection.py
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_model_bitmap.py --originals --oracle
/tmp/wasm-fist-rewrite/native/fist_terrain_preview \
  armoredfist/FISTDATA/AZER1.FSG armoredfist/FISTDATA /tmp/azer1-vehicle.ppm 0 vehicle
# Use a new empty directory; originals are copied/hash-verified and stay read-only.
python3 tools/rewrite/prepare_terrain_preview.py --vehicle --heading 0 \
  --scenario armoredfist/FISTDATA/AZER1.FSG --output-dir /tmp/vehicle-preview/assets
# Place links to the built WASM preview and index.html/terrain.html/terrain.js
# in /tmp/vehicle-preview, then run the actual Chromium canvas gate:
python3 tools/rewrite/verify_browser.py --terrain --build-dir /tmp/vehicle-preview \
  --screenshot /tmp/vehicle-preview/wasm.png
```

The native CTest adds a complete pixel contract: transparent texels preserve the entire
background, below-ground geometry is depth-occluded, red/blue authored rows appear in the right
vertical order, opaque output remains opaque and a subsequent terrain draw reproduces its
previous complete frame. The same C probe runs in WASM. Ten scene groups cover constructed
vehicle transparency, the four class scales, periodic placement, missing files/variants and
the previous terrain/camera contracts. The separately required originals add AZER1/TRAIN1/INDIA3
in four directions on both targets and prove that deleting only copied model texels removes
visible vehicle contributions. Required missing inputs publish no frame.

The optional original-instruction gate executes 27 complete bearings, four C-model scale
postings, twelve perspective/step ratios and two complete height projections. It uses the
pinned DOS/kernel images and Unicorn version already required by the model oracle, without
patched instructions or replacement hooks. It is verification tooling, not runtime code.

On 2026-10-06 actual native PNGs and Chromium canvas captures were visually reviewed for AZER1
heading 0, TRAIN1 heading 32768 and INDIA3 heading 16384. Vehicles are upright, their tracks,
hull and barrels are visible, and transparent bounds reveal terrain. The browser gate matches
every canvas RGBA pixel with the complete C PPM and checks worker isolation and runtime errors.

| Mission | Native FNV1a | WASM FNV1a | Complete browser C-PPM SHA256 |
| --- | --- | --- | --- |
| AZER1 | ed943ae5 | 5f0e8575 | 435e1e36566b212472920861732215714040813afb1fdbfefa569984e7a17c80 |
| TRAIN1 | db4cc7c1 | 893e8b49 | 049b01938095cdbb660cfa50ac72f8298efa3299bf0453ef575c5362756d649e |
| INDIA3 | d5ac7add | 85ffa24c | 21b3403ad1b4e7bbf23301f1e512139eeb3883742742975b8dee0671ce28dd17 |

Corrected actual C contact sheets on both targets hash to
`fba1f36b1392921b6b8de87dade3234f84c2f9ea1c83a74e013433229b780578`.
Required builds, strict formatting/tidy and a separate ASan/UBSan/leak-detection build pass;
the sanitizer scene runner covers all ten groups with original assets, alongside the pixel probe.
All temporary inputs/builds/logs are under `/tmp`; only compact reviewed PNGs are retained.
No result closes WI 0041 or the all-function final acceptance gate.

Sanitizer reproduction:

```sh
cmake -S . -B /tmp/vehicle-sanitized -G Ninja -DCMAKE_C_COMPILER=clang \
  -DCMAKE_BUILD_TYPE=Debug \
  '-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer'
cmake --build /tmp/vehicle-sanitized --target fist_terrain_preview fist_vehicle_scene_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  /tmp/vehicle-sanitized/fist_vehicle_scene_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  python3 tools/rewrite/test_terrain_scene.py --target native --originals \
  --native-preview /tmp/vehicle-sanitized/fist_terrain_preview
```
