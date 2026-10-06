# Armored Fist — readable C rewrite

Development lives on `rewrite/softgl`. The reconstruction is frozen at annotated tag
`reference/reconstruction-v1` (`349ad31`). The rewrite uses hand-written C11 and pinned
[softgl](https://github.com/weichwolf/softgl) for rendering. Complete functional behavior is the
target; original frame and PCM bit identity is no longer required. Visual quality improves
throughout development.

**Current scope:** a working native/WASM renderer integration probe and the rewrite work plan.
No playable game, asset decoder, simulation or audio engine exists yet. See
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
`7963be1d5b5e1bebbe97ece2c655228c8bc0a838`, the latest published master at preparation.
meshoptimizer belongs to offline tools and is not a runtime dependency. Simulation and audio tests must verify behavior
under these production flags, including fast-math.

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
CI runs the builds, strict style checks and this actual-browser gate; remote results are separate
from local verification. The pinned dependency currently emits infinity/fast-math warnings under
Clang 19; owned code is warning-free with `-Werror`. Broader depth/rendering coverage remains open.
