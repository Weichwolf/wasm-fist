# Armored Fist — readable C rewrite

Development lives on `rewrite/softgl`. The reconstruction is frozen at annotated tag
`reference/reconstruction-v1` (`349ad31`). The rewrite uses hand-written C11 and pinned
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
four-stream RNG are shared between targets. Interactive movement, full simulation, playable
missions and audio remain open. See
[architecture](docs/architecture.md), [reference status](docs/reference-status.md),
[work queue](board/README.md) and [goal](docs/rewrite-goal.md).

## Dependencies and build

On Debian, install `clang`, `clang-format-19`, `clang-tidy-19`, `cmake`, `ninja-build`, `nodejs`,
`emscripten`, `python3` and `git`. Strict style checks require LLVM **19.1.x**;
set `CLANG_FORMAT=clang-format-19` and `CLANG_TIDY=clang-tidy-19` if unversioned commands differ.
Native softgl requires SSE4.1; WASM requires SIMD128. The WASM build uses softgl pthread workers; the browser requires
cross-origin isolation headers supplied by the included local server. SDL2 is planned for the native interactive milestone
and is not required by this headless probe.

```sh
git submodule update --init --recursive
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
python3 tools/rewrite/serve.py
```

Open `http://localhost:8000` for the browser renderer preview. Native smoke tests run via CTest;
WASM executes the same probe in Node and fails on incomplete execution. Builds are sequential.
Outputs default to `/tmp/wasm-fist-rewrite`; override with `FIST_REWRITE_BUILD_ROOT` and pass the
native build directory to `check_style.py --build-dir`. CMake rejects builds inside the checkout.

Compiler flags for owned C and softgl are `-Wall -Wextra -Wpedantic -Wno-unused-parameter
-Wno-unused-function -fno-strict-aliasing -ffast-math`. Owned code additionally uses `-Werror`.
clang-format and clang-tidy are mandatory CI checks. Dependencies and historical reconstruction
sources are excluded from rewrite style changes. softgl is pinned at
`7963be1d5b5e1bebbe97ece2c655228c8bc0a838`, verified as the latest published `origin/master`
on 2026-10-06. CMake includes only `deps/softgl/libsoftgl`; meshoptimizer lives under
`deps/softgl/tools/third_party/meshoptimizer` for offline tools and is not required by softgl or
the rewrite build. See [dependency verification](board/closed/0050_softgl-dependency-verification.md).
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

This local reference worktree already exists after preparation. The existing Makefile,
`re_out/`, `patches/`, shims and legacy tools remain reference material; new CMake targets do not
link them. Legacy instructions are archived in [reconstruction README](docs/reconstruction-README.md).
Historical acceptance claims remain scoped to that revision. New work follows [AGENTS.md](AGENTS.md).

The scenario decoder and its bounded coverage are documented in [scenario format](docs/scenario-format.md).
The [unit definitions](docs/unit-definitions.md) document owned typed poses, identity and roster
assignment. `python3 tools/rewrite/test_units.py --originals` checks all 47 missions on native and
WASM; its optional `--oracle` gate executes pinned original assignment instructions.
The [vehicle start state](docs/vehicle-start-state.md) documents typed controls, class defaults,
complete component templates and explicit deterministic random input. Its
`test_vehicle_start.py --originals --oracle` gate compares all 960 ground snapshots and every
16-bit random input in every stream with complete original routine execution on both targets.
The [model format](docs/model-format.md) documents complete directional sprite families and
their ownership/validation. `python3 tools/rewrite/test_models.py --originals` requires all 170
model files; optional `--oracle` compares all texels and lookup fields with original instructions.
Synthetic scenarios run in the normal build gates. To verify the complete provisioned original
corpus explicitly, run `python3 tools/rewrite/test_scenario.py --originals`; missing originals,
changed hashes, incomplete output or skipped original coverage fail. This does not claim gameplay
completion. Original binaries remain ignored.

The [terrain format](docs/terrain-format.md) documents the shared KLC/resource/palette readers
and the optional pinned instruction oracle. Synthetic terrain contracts run in both build gates;
`test_terrain_assets.py --originals` requires every pinned original and compares complete outputs
against actual original decoder instructions on native/WASM. Unicorn is an optional verification
dependency, outside the game and renderer builds.
The [terrain loader](docs/terrain-loading.md) also validates all required scenario asset references,
direct/archive palette resolution, original mission palette preparation and full bundle ownership.
The [terrain scene](docs/terrain-scene.md) documents the recovered world axes/scale, inspection
camera, quality choices and native/browser preview commands. `prepare_terrain_preview.py` copies
only required pinned inputs into an empty `/tmp` directory; original files stay read-only.

## Browser verification

The repeatable browser gate uses Chromium and Playwright 1.63.0. Install the locked test tooling
outside the checkout (Debian packages `chromium` and `npm`), then run:

```sh
mkdir -p /tmp/wasm-fist-browser-tools
cp tools/rewrite/browser/package*.json /tmp/wasm-fist-browser-tools/
npm ci --prefix /tmp/wasm-fist-browser-tools --ignore-scripts --no-audit --no-fund
python3 tools/rewrite/verify_browser.py --screenshot /tmp/wasm-fist-rewrite/browser.png
```

This starts/stops its own isolated local server and checks worker startup, completed output,
canvas size, alpha and vertical color orientation, plus browser runtime errors. `CHROMIUM` may
select another installed Chromium executable. It is renderer integration coverage only.
After preparing terrain inputs, `verify_browser.py --terrain` checks every canvas pixel against
the complete shared C scene output. CI prepares constructed terrain without original content.
CI runs the builds, strict style checks and both actual-browser gates; remote results are separate
from local verification. The pinned dependency currently emits infinity/fast-math warnings under
Clang 19; owned code is warning-free with `-Werror`. Interactive rendering remains open.
