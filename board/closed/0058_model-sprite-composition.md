Type: Work item
Title: Complete model sprite composition for shared rendering
Depends: 0056, 0057

## Contract

Compose every selected part of a loaded original model into an owned palette-index bitmap.
Preserve original stable ascending part priority, piece order, local/base atlas choice,
column-major texels, horizontal mirroring, transparent zero and signed piece anchors. Retain
all source texels at authored resolution for subsequent softgl textures; perspective scaling
and original palette interpolation are outside this unfiltered asset-plane contract.

## Evidence

Original 2fd4..2ffd inserts parts by unsigned priority, after existing equal priorities.
Kernel ad1e/ad47 and adf1..ae40 recover signed anchor conventions; the b31d..b422 unfiltered
sprite loop advances vertically within each column, mirrors column order and preserves the
background for zero texels. These actual instructions were rechecked and executed independently.

Implemented the shared C compositor in `src/render/model_bitmap.{h,c}`, with complete validated
selection before output publication and independently owned indices/palette. The probe and
six behavior groups are mandatory native/WASM build gates. Piece byte extent is a single public
model-format constant, reused by decoder, compositor and probe. See `docs/model-composition.md`.

Verified 2026-10-06:

- `bash tools/rewrite/build.sh all`: all eight native CTest contracts and all WASM gates pass.
  Default fixture runs deliberately skip the separately requested original corpus.
- `python3 tools/rewrite/check_style.py`: strict LLVM 19.1.7 format/tidy passes all 22 owned C
  translation units. Checks/compiler flags are unchanged; no suppression was added.
- `/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_model_bitmap.py --originals
  --oracle --review-dir /tmp/wasm-fist-model-review` with the `/tmp` Python cache: all six groups
  pass on production native/WASM without skips. All 34 pinned families produce 6432 completely
  compared bitmaps per target (32 baselines and every individual part/variant at opposing facing).
  Every full pixel, bounds and retained post-destruction palette/indices are checked; this does
  not claim exhaustive multi-part pose combinations or original perspective equivalence.
- Actual original 2fd4 insertion confirms unsigned priority ordering/stable ties. Kernel b31d
  executes full integer-source scans for all atlas entries and both mirror paths, with complete
  nonzero-background comparison; actual ad1e..ae40 proves every signed-byte anchor case. Identical
  complete sprite inputs may reuse cached instruction results. No instruction hooks/patches
  replace behavior; only declared projection/scan boundary parameters are provided.
- Full original model-reader regressions (`test_models.py --originals --oracle`) pass all six
  groups on native/WASM after sharing the piece-size constant.
- Synthetic cases reach unequal/equal/extreme priorities, base/local pieces, transparent overlap,
  piece-order effects, mirrored columns, signed extrema, empty frames, 255x255 sprites, missing/
  malformed files and atomic API failures, including failure after partial part selection.
  Complete model/source destruction precedes retained bitmap/palette observation.
- ASan/UBSan/leak detection with production flags passes all six groups and every original
  family. Original inputs remain read-only and unchanged by post-run size/hash checks.
- Actual native and WASM PNG contact sheets were visually inspected: recognizable tracks,
  hulls/barrels in four directions, correct assembled parts and transparent borders. Row order
  BMP_C/M1_C/M3_C/T80_C; facing columns 0/8/16/24. Both compact sheets have SHA256
  67ff74a1744a8ad58bbdb3d891b64ae8fb0eb0ede3ff624b1413e1e142a07f11 and remain in `/tmp`.
  Scene/browser presentation is unchanged; this is an authored asset-plane review.
- Temporary builds/logs/sanitizers are external; obsolete owned scratch is cleaned after
  recording the compact evidence. Frozen/generated engine sources remain pristine.

## Next

Composed model textures and origin-relative anchors are ready. Recover world scale/view bearing
and mission-palette selection, then project/draw the selected ground vehicle in the shared scene.

## Accept

Complete native/WASM bitmap outputs, bounds and all original source-list/pixel decisions are
verified; malformed/absent selections fail atomically. Originals remain unchanged. Memory and
strict style/build checks pass. Actual assembled ground vehicles are visually inspected. The
scene is unchanged; perspective vehicle rendering, animation and gameplay remain under 0041.

WI 0059 correction: complete original object projection proves frame Y increases upward.
The composed bytes are bottom-row-first; the old top-row-first API description and diagnostic
contact-sheet orientation were inverted. Storage/anchor/byte expectations remain valid;
see `docs/vehicle-scene.md` for the corrected API and actual scene verification.
