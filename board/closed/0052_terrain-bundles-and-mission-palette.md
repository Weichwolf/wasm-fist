Type: Work item
Title: Complete scenario terrain bundles and original mission palette mapping
Parent: 0049
Depends: 0051

## Contract

Resolve required height/color/sky/palette names, prefer directly available palettes and use PAL.RES
only for missing palette files. Decode owned terrain bundles and prepare original mission palette
sorting/quantization on native/WASM. Failure preserves the caller's output and releases partial data.

## Evidence

Parent `03319d1`, reference `349ad31`; verified on 2026-10-06. Original register/format evidence,
wire/output contracts and verification scope are documented in `docs/terrain-loading.md`.

- Shared `terrain.c` owns bundle resolution and cleanup; storage callbacks own bytes only. The probe
  provider releases its source/scenario buffers before writing every owned bundle output byte.
- `palette.c` implements original normal mission preparation: reserved prefix 80 from engine db47,
  selection sort 9f10, RGB8/DAC6 reductions 4a3c/a033, weighted first-minimum mapping 9e60/ac70.
  All original distance-table entries verify the recovered squared weights. No generated code is linked.
- `bash tools/rewrite/build.sh all` passed native/WASM production builds and all default contracts:
  three native CTest entries, Node renderer/scenario and sixteen synthetic terrain groups. Only the
  separately requested original group is skipped in the default gate.
- The explicit original terrain gate passed all seventeen groups without skips on both targets:
  all 22 planes/32 palettes plus all 47 complete scenario bundles and 38 image/palette mapping pairs.
  Every raw plane/palette, sorted mission component and mapping byte equals executed original
  instructions. The original caller's ESI input to 9e60 is reproduced; no replacement search is used.
- Normalizing actual BINF names proves 19 asset tuples represented by 28 spelling variants. Both
  counts are asserted from originals; prior case-sensitive spelling counts do not imply 28 physical tuples.
- Tests cover every required missing/corrupt file, direct-file precedence, no fallback on corruption
  or read errors, missing archive members, invalid map shape, owned lifetime after input destruction,
  repeated destruction, quantization, ties and reserved prefix/index zero handling.
- `python3 tools/rewrite/check_style.py` passed strict LLVM 19.1.7 for all twelve owned C units and
  headers; `git diff --check` passed. Checks/compiler flags were not weakened.
- An isolated native Debug build of both asset/terrain probes with
  `-fsanitize=address,undefined -fno-omit-frame-pointer` passed all seventeen groups, including all
  47 complete bundles and all originals, without sanitizer failures. Pass both `--native-probe`
  and `--terrain-probe` paths with `--target native --originals` to reproduce that coverage.

No presentation code changed. Terrain placement/rendering, camera, mission objects and simulation
remain open; a loaded bundle does not prove a playable mission. Assets remain read-only/ignored;
all pinned file hashes are checked after the complete original gate. Scratch stays under `/tmp`.

## Next

Continue 0049 with world placement, actual terrain rendering and a supported inspection camera.
Review native/browser output before parent acceptance, then continue 0041 toward the playable mission.

## Accept

Complete native/WASM terrain bundle output agrees with original instruction evidence for every
provisioned scenario; required error/ownership contracts and strict checks pass. Commit and push.
