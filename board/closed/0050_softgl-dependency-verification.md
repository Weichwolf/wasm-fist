Type: Work item
Title: Verify latest published softgl and offline meshoptimizer boundary

## Contract

Use the latest published softgl revision. meshoptimizer is an offline-tool dependency and must
not be required, compiled or linked by softgl or the native/WASM rewrite.

## Evidence

Verified on 2026-10-06:

- `git -C deps/softgl fetch origin` and `git -C deps/softgl ls-remote origin HEAD refs/heads/master`
  both identify `7963be1d5b5e1bebbe97ece2c655228c8bc0a838`. The existing submodule pin already
  matches it; no dependency source or pin change was necessary.
- That upstream commit moves meshoptimizer from `libsoftgl/third_party/` into
  `tools/third_party/`. Root CMake includes only `deps/softgl/libsoftgl`.
- Native and WASM compile databases each contain 24 translation units, all `.c`, with no
  meshoptimizer paths. Neither `build.ninja` contains meshoptimizer or softgl offline-tool rules.
- `bash tools/build.sh all` passed: both native CTest contracts and the WASM renderer
  and synthetic scenario contracts. The optional original-scenario suite is skipped by this
  command and is unrelated to the renderer dependency boundary.
- `python3 tools/check_style.py` passed with LLVM 19.1.7.
- `python3 tests/verify_browser.py --screenshot
  /tmp/wasm-fist-rewrite/softgl-verification.png` passed in Chromium. The screenshot was visually
  reviewed; the completed renderer checksum is `3fa856ed`, isolation is enabled, and no browser
  runtime errors occurred. This is diagnostic renderer coverage, not gameplay completion.

Builds and the current capture remain under `/tmp`; no new logs or binary assets are tracked.

## Next

Dependency verification is complete. Continue terrain decoding and rendering under 0049/0041.

## Accept

Latest published revision confirmed; native, WASM, strict tooling and real-browser integration
pass; both build graphs exclude offline meshoptimizer/tools. Record and push this verification.
