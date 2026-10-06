# Armored Fist — rewrite working rules

Target: readable, maintainable hand-written C11 Armored Fist for native and WASM using pinned
softgl. Preserve complete functional behavior and continuously improve visual quality.
Original pixel/palette/PCM bit identity is no longer an acceptance target.

- Follow `board/README.md`, one bounded step at a time. Verify evidence and keep WIs current.
- Own code lives in `src/`. Platform code handles presentation/devices/storage only. Share
  simulation, asset decoding, rendering and audio across targets. One owner per contract.
- Recover formats and gameplay rules from original files/behavior and frozen reconstruction.
  Prove hypotheses before acceptance. No silent stubs, invented gameplay constants or disabled
  checks that conceal missing functionality.
- `reference/reconstruction-v1` is immutable. Preserve `re_out/`, `patches/`, legacy shims/tools
  and `board/reference/` as evidence. Do not link generated engine code into the rewrite.
  Historical register/segment/bit-parity requirements apply to that reference work only.
- softgl is a pinned `deps/softgl` submodule. Keep dependency changes separate and justified.
  Runtime and renderer stay C11. meshoptimizer is for offline tools only; do not link it.
- Keep `armoredfist/` ignored and read-only; provision originals and run isolated copies.
- Enforce `.clang-format` and `.clang-tidy` with LLVM 19.1.x on owned rewrite C/headers.
  Run `python3 tools/rewrite/check_style.py` and `bash tools/rewrite/build.sh all` for C,
  build configuration or dependency changes. Warnings fail; do not weaken checks to get a pass.
- Use the requested compiler flags in `CMakeLists.txt`; keep `-Werror` for owned code.
  Fast-math is enabled: verify simulation/audio behavior with the production build.
- Tests specify behavior. Change expectations only with evidence or an intentional visual design
  change. Missing output/coverage and incomplete runs fail. Original bit-parity tests do not
  govern deliberate visual improvements.
- Build sequentially with one writer. Disposable builds/logs/captures/isolated assets belong in
  `/tmp`. Clean obsolete owned artifacts after each success; retain compact summaries and version
  reproduction commands/fixtures. Do not erase unrelated jobs or evidence still in use.
- Commit and push every verified bounded success; exclude unfinished experiments. Repository
  text is English. No generated-by/co-author attribution.

Done: every menu, mission/map, vehicle, objective/outcome, setting/device, input/link, campaign,
save/load and editor create→save→reload→simulate flow works correctly on both targets. Require
meaningful behavior/asset/audio tests, visual checks and ten consecutive complete clean WASM runs
independently confirmed by a subagent; failure resets the streak. See WI 0047. That final gate
allows a subagent for independent verification, not delegated implementation.
