Type: Work item
Title: Shared C11 KLC planes and resource palettes
Parent: 0049
Depends: 0048

## Contract

Decode complete original KLC1 height/colormap/stamp/sky planes and RESOURCE1 palette members into
shared typed C assets on native/WASM. Preserve decoded indices/palette bytes, consume complete
inputs, validate malformed data, and prove output against actual original decoder instructions.
This bounded data milestone precedes terrain placement, palette remapping and scene rendering.

## Evidence

Parent commit `d4febf6`, frozen reference `349ad31`; verified on 2026-10-06. Fresh disassembly,
wire layout, ownership and oracle boundaries are recorded in `docs/terrain-format.md`.

- `src/assets/klc.c`, `resource.c` and `palette.c` implement the documented readers. Shared view
  and endian helpers have one owner; test probes share their complete EOF reader. Generated
  engine code and dependency sources are unchanged and are not linked into the asset readers.
- `bash tools/build.sh all` passed sequential native/WASM production builds, all three
  native CTest contracts and Node renderer/scenario/terrain contracts. Default gates run ten
  synthetic terrain groups and explicitly skip only the separately requested original group.
- `/tmp/wasm-fist-decoder-oracle/bin/python tests/test_terrain_assets.py --originals`
  passed all eleven groups with no skips on both targets: 22 complete KLC/SKY files, 3,146,880
  pixels, all embedded palette bytes and all 32 complete PAL.RES members. Expected output is
  obtained by executing the original instructions, with full byte comparison and pinned hashes.
- `python3 tests/test_scenario.py --originals` passed all eight groups without skips
  across all 47 original missions on native/WASM after the shared helper extraction.
- `python3 tools/check_style.py` passed strict LLVM 19.1.7 checks for all nine owned C
  translation units and formatting of their headers. Compiler/tidy checks were not weakened.
- An isolated native asset-probe build with `-fsanitize=address,undefined -fno-omit-frame-pointer`
  passed all eleven terrain groups including complete originals, with no sanitizer failures.
  Reproduce by configuring CMake Debug under `/tmp`, building target `fist_asset_probe`, then
  passing that executable as `--native-probe` with `--target native --originals` to the same gate.
- `git diff --check` passed. Temporary fixtures are removed by the test runner; the completed
  sanitizer build is disposable. Original assets remain ignored/read-only and hashes unchanged.

No presentation code changed. This milestone proves asset bytes and validation, not a real scene,
simulation, audio, original DOS transport or full mission functionality. Parent 0049 remains active.

## Next

Continue 0049: scenario asset bundle resolution, mission palette remapping, world height/axis units,
terrain geometry and a camera supported by original scenario/player evidence. Review the actual
native/browser scene before accepting that parent milestone.

## Accept

Complete decoded planes and selected palette members match the buffered original instruction
oracle on native/WASM; constructed invalid-input/ownership contracts and strict checks pass.
Commit/push this verified foundation without claiming terrain rendering completion.
