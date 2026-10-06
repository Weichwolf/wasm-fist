# Terrain inspection scene

The shared C11 renderer draws the decoded original height field through softgl, with the
scenario's colormap and prepared mission palette. Native emits a complete 640×400 P6 frame;
WASM renders the same scene into a canvas using its browser-compatible MEMFS input provider.
This is a static inspection preview. WI 0059 adds an optional [ground vehicle scene](vehicle-scene.md).
Runtime installation, controls, collision, HUD, gameplay and audio remain under WI 0041.

## World coordinates and original evidence

The frozen kernel image and optional Unicorn version are pinned as in
[terrain format](terrain-format.md). Evidence is original code, separate from the renderer:

- Kernel `11bb/11be` shifts integer X/Y left 13; `11c1` negates Y. `8480` combines the
  upper row and column bits with SHLD, then reads one byte. The repeated integer domain is
  therefore `2^(32-13) = 524288`. Rows increase opposite original Y; columns follow original X.
  The original sampler operates on its expanded 1024-square plane. The preview preserves the
  same domain while triangulating the decoded base plane, without claiming the original
  resampling/stamp/collision behavior.
- Kernel `85e2–8624` applies X << 13, -(Y << 13), min(altitude, 0x7f00) << 17 and
  -(uint16 heading << 16) to the original TCB camera fields. `69f2–69fb` converts altitude
  back to a height byte with >> 25. Kernel `39b2–39e2` builds its vertical projection table:
  inverse horizontal ray step times 32, high-half multiplication by height difference << 24,
  then output >> 8. Thus one height-byte difference projects as `2^21 / ray_step` pixels,
  equivalent to 256 original horizontal integer units (256 << 13 = 2^21).
- Render units consequently use original X/Y divided by 256, a 2048-unit periodic domain
  and one vertical unit per height byte. No physical metre interpretation is asserted.
  The original sine table at `9450/9650` starts at sin(0)=0, cos(0)=INT32_MAX; its negative
  heading basis in `6997–6a8f` makes heading zero look along increasing original Y
  (decreasing decoded row). The new camera uses X right, altitude up and decoded row as Z.
- AZER1 SHDR starts at X=583982, Y=1142557. At base resolution 256, its wrapped cell is
  column 29, row 210, height 44. DCBS unit zero has the same position and heading 26729;
  the reviewed original preview explicitly supplies that heading. This does not establish a
  generic player-record rule for other scenarios.

Reproduce the complete sampler/camera cases, including signed coordinate limits, wrap edges
and the original altitude clamp, with actual original instructions:

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/verify_original_terrain_coordinates.py
objdump -D -b binary -m i386 -M intel \
  --start-address=0x395e --stop-address=0x3a17 re_out/fist_image.bin
```

The sampler oracle uses a single sentinel in a complete synthetic 1024-square plane. Four
positions and three complete camera setups reach their required original stop addresses;
no hooks replace instructions. Projection-table derivation above is inspection evidence,
not an assertion of original-frame identity.

## Deliberate preview quality

The inspection camera uses X/Y and heading of the ground vehicle at explicit roster slot zero,
resolved by the [owned unit definitions](unit-definitions.md), 64 render units above the nearest
decoded ground sample, pitch -0.25 radians and a 60-degree vertical field of view. An optional
unsigned 16-bit heading overrides the vehicle heading. Missing/nonvehicle slot zero fails;
neither SHDR nor the first DCBS record supplies a fallback. The height/pitch/field of view are
explicit inspection choices, not recovered vehicle/cockpit initialization. Snapshot altitude
is not a substitute for initialized ground placement. Camera altitude uses finite height units.

Every base height cell contributes two triangles with shared vertices. Central-difference
normals, a fixed sun, linear texture filtering and depth testing produce a readable relief view.
Color pixels pass through the shared original mission map; DAC6 components expand to the full
RGB8 display range. One complete domain surrounds the camera and both textures and height
sampling repeat at map edges. Workers complete before temporary geometry/texture storage is
released; the draw borrows a successfully loaded bundle only for its duration.

Fog starts at 384 and ends at 768 render units. Its color and the clear background are the
average of the mapped sky pixels. The sky panorama's orientation/rendering remains open;
the decoded sky is currently used only for that average. Original base-plane upsampling,
stamps, LOD, models and the playable camera remain subsequent work. There is no meshoptimizer
runtime dependency; the entire scene and softgl build graph remain C.

## Run and verify

```sh
export PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
# Complete original scene on both targets; inputs are copied and hash-verified:
python3 tools/rewrite/test_terrain_scene.py --originals
# Native output, using AZER1's roster-zero vehicle position and heading:
/tmp/wasm-fist-rewrite/native/fist_terrain_preview \
  armoredfist/FISTDATA/AZER1.FSG armoredfist/FISTDATA /tmp/azer1.ppm
# Output directory must be empty. The tool copies only required pinned inputs.
python3 tools/rewrite/prepare_terrain_preview.py \
  --scenario armoredfist/FISTDATA/AZER1.FSG
python3 tools/rewrite/verify_browser.py --terrain \
  --screenshot /tmp/wasm-fist-rewrite/azer1-terrain-browser.png
python3 tools/rewrite/serve.py
```

Open `http://localhost:8000/terrain.html`. Omitting `--scenario` prepares constructed asymmetric
terrain for CI without original content. Original files remain ignored/read-only; all copied
inputs, manifests, builds and captures stay under `/tmp`. The manifest lists complete sizes and
SHA-256; the browser rejects changed, missing or incomplete input before shared C decodes it.

Both build gates test complete output size/alpha, textured ground and sky, heading changes,
height-field changes, actual unit-position map-period repeats, signed coordinate limits, invalid
headings and missing required inputs. They prove that changing only SHDR/first-object coordinates
does not move the camera, selected-unit coordinates do, and unit heading equals the matching
explicit override. Missing/nonvehicle player definitions fail without publishing a frame.
`--originals` additionally requires pinned AZER1 and TRAIN1 assets without skips. The
browser gate checks every emitted RGB pixel against the canvas RGBA output, including row
orientation and opaque alpha, along with worker isolation, completed execution and runtime errors.

On 2026-10-06 native/browser AZER1 and constructed browser captures were visually reviewed.
AZER1 native FNV1a is `11444fd3`; Node and Chromium WASM are `41900eee` with full PPM SHA-256
`a8d028dd641c96106c3aea4caa3f80c344776532c88f8ef00623724afc40ee69`. Their 768000 RGB bytes differ
in 239 components (237 by one, two by two); this records normal target rasterization differences,
without a new cross-target bit-identity requirement. Native AddressSanitizer/UBSan passes all six
scene groups including original inputs. No claim covers a native interactive window or a playable
mission. The original triangle browser gate still passes independently.

WI 0055 changes the camera source from SHDR/north to roster-zero unit pose. Actual native/browser
AZER1 and TRAIN1 frames are reviewed; AZER1 retains the recorded outputs above because its unit
position matched SHDR and its explicit old heading already matched the vehicle. TRAIN1 now
uses registry index 32 at `(-1061797, 1816527)` heading `51700`. The reviewed view shows textured
hills and sky from that location: native FNV1a `8b721dac`, Chromium WASM `afb2be08`, complete PPM
SHA-256 `50805f5cdbd13a765cf1d6cba615d192619717de1346f5d99dac0fd2ecd66501` and 38318 RGB colors.
All eight scene groups pass on both targets with originals; strict format/tidy passes.
An external ASan/UBSan build also passes all eight native groups, including original inputs,
missing-player paths and copied-input lifetime checks, with leak detection enabled.
Both browser views pass full C-frame/canvas equality, isolation and absence of runtime errors.
This is a vehicle-positioned inspection scene; vehicle models, native interactive window,
cockpit/control/simulation, sky panorama and audio remain open.
