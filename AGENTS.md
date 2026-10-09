# Armored Fist — rewrite working rules

Target: readable, maintainable hand-written C11 Armored Fist for native and WASM using pinned
softgl. Preserve complete functional behavior while greatly surpassing the original's quality
in every area. The complete game must run without any original Armored Fist game files.
Follow the full objective in `board/GOAL.md` and owned-content plan in `docs/owned-content.md`.
Original pixel/palette/PCM bit identity is not an acceptance target.

- Follow `board/README.md`, one bounded step at a time. Verify evidence and keep WIs current.
- Active rewrite development uses `master`. Branch `ghidra` retains the frozen decompiled and
  patched reconstruction; do not move the immutable reference tag.
- Own code lives in `src/`. Platform code handles presentation/devices/storage only. Share
  simulation, asset decoding, rendering and audio across targets. One owner per contract.
- Recover formats and gameplay rules from optional original files/behavior and frozen reconstruction.
  Prove hypotheses before acceptance. No silent stubs, invented gameplay constants or disabled
  checks that conceal missing functionality.
- Rebuild every visual asset in the repository: Blender models, complete vehicle interiors and
  cockpits, environment objects, effects, UI and all other inventoried content. Generate textures
  procedurally; generate heightmaps and colormaps through a JSON-parameterized map generator.
  Prioritize this generator next. All octave, hydraulic erosion, palette, water-level,
  temperature/climate and related controls must be explicit in JSON. Iteratively inspect
  generated maps against original development references until their terrain forms and visual
  character are very close, while increasing resolution/detail; generation must need no originals.
  Create new audio for every inventoried sound/music/voice requirement. No original visual,
  terrain, palette, cockpit or audio asset may be needed or shipped by the completed game.
- Keep Blender sources, generation scripts, JSON recipes and final generated runtime assets in
  `assets/`. Pin offline tool versions and seeds, record provenance and reproducible generation
  commands, and verify generated outputs. Disposable intermediates/builds/captures stay in `/tmp`.
  Offline Blender/content tools do not become runtime dependencies; shared runtime stays C11.
- Own mission/map/content definitions and their runtime loading, editor round trips and packaging.
  Required build, distribution, startup, gameplay, save/load and editor acceptance must work in a
  clean checkout with no original files, original downloads or reference-tool installation.
  Original-dependent comparisons remain explicit optional development reference gates; required
  owned-content and complete functional gates must never skip missing assets or functionality.
- `reference/reconstruction-v1` is immutable. `re_out/`, `patches/` and legacy shims/tools
  live on `ghidra`; reference tests provision pinned images under `/tmp` with
  `tests/reference_images.py`. `board/reference/` retains historical work-item evidence.
  Do not restore generated engine code or reconstruction dependencies to this checkout.
  Historical register/segment/bit-parity requirements apply to that reference work only.
- softgl is a pinned `deps/softgl` submodule. Keep dependency changes separate and justified.
  Use unmodified upstream softgl; do not edit or publish changes to its sources.
  Configure the shared three-helper-plus-caller rendering budget in owned application code.
  Runtime and renderer stay C11. meshoptimizer is for offline tools only; do not link it.
  Keep softgl as the only dependency in `deps/`; external reference tools belong under `/tmp`.
- Target 60 FPS on this machine at 640x360, genuine 4x MSAA and four total render threads.
  Treat the user's 100000-200000 triangles and 20-30 materials/frame as initial scene planning
  estimates, not measured 60-FPS capacity. Measure representative owned scenes with reserve
  for overdraw, alpha tests, complex materials, simulation/audio and presentation; do not reduce
  requested image quality or disable MSAA to claim the target. Follow docs/render-budget.md.
- Keep `armoredfist/` ignored and read-only; provision optional development references and run
  isolated copies. Never make original provisioning a requirement for the shipped game.
- Enforce `.clang-format` and `.clang-tidy` with LLVM 19.1.x on owned rewrite C/headers.
  Run `python3 tools/check_style.py` and `bash tools/build.sh all` for C,
  build configuration or dependency changes. Warnings fail; do not weaken checks to get a pass.
- Use the requested compiler flags in `CMakeLists.txt`; keep `-Werror` for owned code.
  Fast-math is enabled: verify simulation/audio behavior with the production build.
- Tests specify behavior. Change expectations only with evidence or an intentional visual design
  change. Missing output/coverage and incomplete runs fail. Original bit-parity tests do not
  govern deliberate visual improvements.
- Treat visible graphics as an acceptance priority. Review actual native and browser images
  and motion for terrain, units, camera, effects and HUD; record concrete quality findings.
  Require substantially higher quality throughout the asset inventory, including cockpits;
  upscaled original sprites or superficial substitutions do not meet the target. Review new
  audible output for quality as well as event/timing correctness. Numerical tests and changed
  image hashes do not establish visual quality. Follow 0046.
- Build sequentially with one writer. Disposable builds/logs/captures/isolated assets belong in
  `/tmp`. Clean obsolete owned artifacts after each success; retain compact summaries and version
  reproduction commands/fixtures. Do not erase unrelated jobs or evidence still in use.
- Commit and push every verified bounded success; exclude unfinished experiments. Repository
  text is English. No generated-by/co-author attribution.

Done: every menu, mission/map, vehicle, objective/outcome, setting/device, input/link, campaign,
save/load and editor create→save→reload→simulate flow works correctly on both targets using only
complete owned assets/content. Every inventoried asset, cockpit, environment and audio category
substantially surpasses the original's quality. Require original-file-free clean-checkout builds
and complete runs, meaningful behavior/asset/audio tests, actual visual and listening checks,
and ten consecutive complete clean WASM runs
independently confirmed by a subagent; failure resets the streak. See WI 0047. That final gate
allows a subagent for independent verification, not delegated implementation.
