Type: Work item
Title: Freeze reference and prepare readable C / softgl rewrite

## Contract

Preserve the reconstruction as immutable reference and prepare a separate, reproducible readable
C11 native/WASM development path, pinned softgl, strict style/static analysis and a functional
work plan. Original bit identity is explicitly no longer the rewrite target.

## Evidence

Preparation on 2026-10-06, parent/reference `349ad31a9fd21b350d435651bb2e90afda40cf60`:

- Branch `rewrite/softgl`; annotated tag `reference/reconstruction-v1`; detached reference worktree
  `/tmp/wasm-fist-reference`. master remains unchanged. Old active publication/verification owners
  were stopped; private unfinished experiments are excluded.
- Historical board files and README preserved byte-for-byte under `board/reference/`; recovered
  C/patches/legacy shims and provisioning are unchanged. Originals remain ignored and read-only.
- softgl pinned to latest published master `7963be1d5b5e1bebbe97ece2c655228c8bc0a838`.
  Both compile databases contain only C translation units and no meshoptimizer/tool sources.
- Fresh sequential native/WASM builds: `bash tools/rewrite/build.sh all` passed. Native CTest
  renderer integration passed; complete Node WASM probe exited zero. Both printed
  `softgl integration: 320x200 RGBA8 fnv1a=3fa856ed`.
- `python3 tools/rewrite/check_style.py` passed with LLVM 19.1.7. All owned C is present in the
  checked compile database; headers are formatting-checked and included by translation units.
  Negative controls proved that bad formatting and identifier naming produce failing exits.
- `python3 tools/rewrite/verify_browser.py --screenshot /tmp/wasm-fist-rewrite/browser.png` passed
  with locked Playwright 1.63.0 and actual Chromium 154.0.8037.92. Cross-origin isolation was true;
  canvas size was 320×200, black opaque corner, blue top `[13,14,229,255]`, red bottom-left
  `[220,19,16,255]`, completed checksum matched and no browser runtime errors occurred. Screenshot
  reviewed visually; native coverage is headless renderer output, not an interactive window.
- Script/JS syntax and `git diff --check` passed. `make` now selects rewrite checks; legacy
  reconstruction remains available explicitly. CI runs builds, strict checks and browser integration.

Artifacts remain outside the checkout under `/tmp/wasm-fist-rewrite`; old intermediate builds were
removed before the fresh final build. Commands and compact results here are durable evidence.
Remote CI execution is separate from these local checks.

The initial no-pthread probe exposed an expected-worker wait in the previous softgl pin. The final
build uses the supported pthread path with a three-helper prestarted pool matching the latest
renderer's automatic WASM worker limit. COOP/COEP serving is provided. No dependency code was patched.
The dependency emits Clang infinity/fast-math warnings; owned code passes `-Werror`. The triangle
only proves integration. Full rendering/depth, original data, game simulation/audio/input and
interactive native presentation remain open under 0041–0047.

## Next

Continue 0041 with a bounded original map/model decoder contract and fixtures. No further
implementation belongs to this preparation milestone.

## Accept

Reference preserved; branch, submodule, CMake builds and actual renderer probes work on both
targets; strict formatting/tidy gates reject violations; original provisioning remains intact;
architecture, rules and functional work queue describe the changed goal honestly. Commit and push
this verified bounded preparation together with the reference tag.
