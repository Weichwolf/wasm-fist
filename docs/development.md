# Armored Fist — readable C rewrite

Development lives on `master`. The decompiled and patched reconstruction is available on
`ghidra` at the unchanged annotated tag `reference/reconstruction-v1` (`349ad31`).
The rewrite uses hand-written C11 and pinned
[softgl](https://github.com/weichwolf/softgl) for rendering. Complete functional behavior is the
target; original frame and PCM bit identity is no longer required. Visual quality improves
throughout development.

**Current scope:** a native/WASM renderer integration probe and shared C11 readers for scenario
envelopes/metadata/unit framing (all 47 missions), KLC height/colormap/stamp/sky planes (all 22
files), resource members and VGA palettes (all 32 palettes). Complete terrain bundles and mission
palette maps are verified for all 47 scenarios. A shared softgl terrain inspection scene now
renders original terrain and the roster-zero ground vehicle in native output and the browser.
Owned unit definitions and
normal-side registry/platoon mappings are verified for all 4213 original snapshots. The shared
model loader decodes all 34 original directional sprite families, with original model selection
and complete owned sprite assembly. Typed ground-vehicle initialization and the deterministic
four-stream RNG, ground motion and manual turret updates are shared between targets.
A continuous native SDL2/browser driving scene now connects owned player state, installed
height/contact, timed keyboard controls and live model/camera rendering. Full simulation,
playable missions and audio remain open. See
[architecture](architecture.md), [reference status](reference-status.md),
[work queue](../board/README.md) and [goal](rewrite-goal.md).

## Dependencies and build

On Debian, install `clang`, `clang-format-19`, `clang-tidy-19`, `cmake`, `ninja-build`, `nodejs`,
`emscripten`, `python3`, `git`, `pkg-config` and `libsdl2-dev`. Strict style checks require LLVM **19.1.x**;
set `CLANG_FORMAT=clang-format-19` and `CLANG_TIDY=clang-tidy-19` if unversioned commands differ.
Native softgl requires SSE4.1; WASM requires SIMD128. The WASM build uses softgl pthread workers; the browser requires
cross-origin isolation headers supplied by the included local server. SDL2 supplies the native driving preview window/input and presentation.

```sh
git submodule update --init --recursive
bash tools/build.sh all
python3 tools/check_style.py
python3 tools/serve.py
```

Open `http://localhost:8000` for the browser renderer preview. Native smoke tests run via CTest;
WASM executes the same probe in Node and fails on incomplete execution. Builds are sequential.
Outputs default to `/tmp/wasm-fist-rewrite`; override with `FIST_REWRITE_BUILD_ROOT` and pass the
native build directory to `check_style.py --build-dir`. CMake rejects builds inside the checkout.

Compiler flags for owned C and softgl are `-Wall -Wextra -Wpedantic -Wno-unused-parameter
-Wno-unused-function -fno-strict-aliasing -ffast-math`. Owned code additionally uses `-Werror`.
clang-format and clang-tidy are mandatory local checks. The pinned dependency is excluded
from rewrite style changes. softgl is pinned at
`7963be1d5b5e1bebbe97ece2c655228c8bc0a838`, verified as the latest published `origin/master`
on 2026-10-06. CMake includes only `deps/softgl/libsoftgl`; meshoptimizer lives under
`deps/softgl/tools/third_party/meshoptimizer` for offline tools and is not required by softgl or
the rewrite build. See [dependency verification](../board/closed/0050_softgl-dependency-verification.md).
Simulation and audio tests must verify behavior under these production flags, including fast-math.

## Original files and reference

`armoredfist/` remains ignored and read-only. Existing provisioning verifies the archive and
preserves local files:

```sh
make provision
python3 tools/provision_game.py --help
```

The script retains the requested Armored Fist archive URL and checksum and supports refreshed
`--url` and offline `--archive` input. Never commit original binaries. Run isolated game copies
under `/tmp`. Create a reference worktree in a fresh clone with:

```sh
git worktree add --detach /tmp/wasm-fist-reference reference/reconstruction-v1
```

The frozen worktree contains `re_out/`, `patches/`, shims and legacy tools. Those files
are absent from `master`. Only softgl lives in `deps/`; external reconstruction tools live
under `/tmp` (the local DOSBox binary/source are in
`/tmp/wasm-fist-reference-workspace-349ad31/third_party/`). Rewrite tests, probes and fixtures live directly in `tests/`; development
commands live in `tools/`. Legacy instructions are archived in [reconstruction README](reconstruction-README.md).
Reference tests read checksum-pinned instruction images from `/tmp/wasm-fist-reference-images`.
Run `make reference-images` to provision them: engine and video images come from the immutable
reference Git commit; the ignored kernel image is regenerated from read-only `armoredfist/FIST.RUN`
using the extractor from that same commit. `FIST_REFERENCE_ROOT` may select a different dedicated
`/tmp` directory. Missing originals or altered image hashes fail verification. An optional
Unicorn environment can be installed outside the checkout with:

```sh
python3 -m venv /tmp/wasm-fist-decoder-oracle
/tmp/wasm-fist-decoder-oracle/bin/pip install -r tests/oracle_requirements.txt
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_mission_ready.py --help
```

Historical acceptance claims remain scoped to that revision. New work follows [AGENTS.md](../AGENTS.md).

The scenario decoder and its bounded coverage are documented in [scenario format](scenario-format.md).
The [unit definitions](unit-definitions.md) document owned typed poses, identity and roster
assignment. `python3 tests/test_units.py --originals` checks all 47 missions on native and
WASM; its optional `--oracle` gate executes pinned original assignment instructions.
The [vehicle start state](vehicle-start-state.md) documents typed controls, class defaults,
complete component templates and explicit deterministic random input. Its
`test_vehicle_start.py --originals --oracle` gate compares all 960 ground snapshots and every
16-bit random input in every stream with complete original routine execution on both targets.
The [vehicle motion](vehicle-motion.md) implements speed/hull control, both velocity lanes,
position integration and manual turret slew. `test_vehicle_motion.py --originals --oracle`
checks complete original motion returns, every heading, all four slope profiles to their
actual speed caps and sustained driving on both targets. Collision and full targeting/weapon integration remain open.
The shared [ground route progress](ground-route-progress.md) consumes canonical waypoints
and retained unsigned range, including a proved full-capacity neighbor-read repair. The parent
command phase and playable battle remain pending.

The shared [planar geometry](planar-geometry.md) supplies original navigation bearing and
distance precision for the pending command phase; its required original gate checks 397671
returns and all 960 saved actor/goal pairs on both targets. Full navigation dispatch remains open.
The [ground contact](ground-contact.md) supplies installed height and independent hull/
turret slopes in shared typed state. `test_ground.py --originals --oracle` checks every heading,
all height-byte differences and all 960 ground snapshots at each original detail level against
complete original returns. The [controlled scene](driving-scene.md) supplies class altitude transfer, a shared rational
controller clock and live input/rendering.
The [position history](vehicle-history.md) and
[movement maintenance](vehicle-maintenance.md) share the controlled actor's phase dispatch.
Maintenance consumes its movement word, updates the saved speed counter and refreshes the
original class components; complete original phase/counter/speed domains and all 960 saved
ground snapshots are verified on both targets. Full living dispatch and audible battle remain open.
The [model format](model-format.md) documents complete directional sprite families and
their ownership/validation. `python3 tests/test_models.py --originals` requires all 170
model files; optional `--oracle` compares all texels and lookup fields with original instructions.
Synthetic scenarios run in the normal build gates. To verify the complete provisioned original
corpus explicitly, run `python3 tests/test_scenario.py --originals`; missing originals,
changed hashes, incomplete output or skipped original coverage fail. This does not claim gameplay
completion. Original binaries remain ignored.

The [height-field resampler](heightfield-resampling.md) supplies owned periodic height
expansion and exact knot reduction before ground installation. Its `test_heightfield.py --originals`
gate compares all square original fields and all eight height maps at runtime sizes through 4096
with complete original returns on both targets. Ground queries/contact and live
scene integration are delivered; full mission behavior remains open.
The [terrain format](terrain-format.md) documents the shared KLC/resource/palette readers
and the optional pinned instruction oracle. Synthetic terrain contracts run in both build gates;
`test_terrain_assets.py --originals` requires every pinned original and compares complete outputs
against actual original decoder instructions on native/WASM. Unicorn is an optional verification
dependency, outside the game and renderer builds.
The [terrain loader](terrain-loading.md) also validates all required scenario asset references,
direct/archive palette resolution, original mission palette preparation and full bundle ownership.
The [terrain scene](terrain-scene.md) documents the recovered world axes/scale, inspection
camera, quality choices and native/browser preview commands. `prepare_terrain_preview.py` copies
only required pinned inputs into an empty `/tmp` directory; original files stay read-only.

## Browser verification

The repeatable browser gate uses Chromium and Playwright 1.63.0. Install the locked test tooling
outside the checkout (Debian packages `chromium` and `npm`), then run:

```sh
mkdir -p /tmp/wasm-fist-browser-tools
cp tests/browser/package*.json /tmp/wasm-fist-browser-tools/
npm ci --prefix /tmp/wasm-fist-browser-tools --ignore-scripts --no-audit --no-fund
python3 tests/verify_browser.py --screenshot /tmp/wasm-fist-rewrite/browser.png
```

This starts/stops its own isolated local server and checks worker startup, completed output,
canvas size, alpha and vertical color orientation, plus browser runtime errors. `CHROMIUM` may
select another installed Chromium executable. It is renderer integration coverage only.
After preparing terrain inputs, `verify_browser.py --terrain` checks every canvas pixel against
the complete shared C scene output. Preview preparation also supports constructed terrain
without original content. Build, strict style and actual-browser gates are run locally. The pinned dependency currently emits infinity/fast-math warnings under
Clang 19; owned code is warning-free with `-Werror`. Controlled driving is verified through `verify_browser.py --driving` and
`verify_driving_native.py`; see [driving scene commands and scope](driving-scene.md).
