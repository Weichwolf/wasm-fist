# Authored-resolution model composition

`src/render/model_bitmap.c` composes a loaded original model's complete selected parts into an
owned bottom-row-first palette-index bitmap. This supplies texture data for the shared softgl
renderer. Each selection contains exactly one valid facing/variant pose per part, in part
order. The input model and poses are borrowed only during the call.

The bitmap retains the complete original DAC6 palette, signed left/bottom coordinates relative to
the model origin, width/height and independently allocated indices. Palette index zero is
transparent. Model/source destruction does not affect the composed output. Failed selections
and allocations preserve the caller output; destruction resets all state and is idempotent.

## Original data and drawing decisions

The model reader's bounded parts/variant/piece APIs remain the owners of model format parsing;
see [model-format.md](model-format.md). Composition first resolves and validates every selected
piece, allocates its selected descriptors, computes the complete declared rectangle and sorts
selected parts. Only after selection succeeds does it allocate/draw/publicize the output.
No partially validated piece or guessed fallback enters the bitmap.

Original DOS instructions 2fd4..2ffd insert a part after every existing part whose unsigned
priority is less than or equal to the new priority. The C compositor uses stable ascending
insertion sort. Pieces retain their list order within each part. Zero texels leave the earlier
piece/part visible. Base-atlas references use M00; local references use the selected facing
record. Missing variants, faces, required parts, sprites or texel extents fail rather than
clamping a selection or returning a replacement model.

Source sprite texels are **column-major**, addressed as `column * height + row`. Kernel mode-6
instructions b31d..b422 step vertically through one column, then advance by its height. The
mirror flag reverses column order while retaining row order. The output converts that layout
to row-major indices without losing source texels. The background-preserving zero branch is
at b3dc..b3e0. Both complete mirror paths are executed against a nonzero background by the
independent oracle, including full-frame unchanged-background evidence.

X is the original signed piece byte. Kernel ad47 negates DH **as a byte** before signed
conversion at ae05. Consequently Y is the negative saved byte, with -128 wrapping back to
-128. That exception is the actual recovered width contract. The bitmap keeps these authored
coordinates before perspective division and viewport rounding. A union of all declared piece
rectangles sets bounds, including transparent borders; origin-relative placement is not lost
by trimming. Byte anchors and byte dimensions imply an extent at most 510 texels per axis,
so the bitmap's uint16 dimensions and int16 offsets cover the complete format.

A selection whose lists explicitly contain no pieces succeeds with a copied palette, zero
width/height and NULL indices. Missing model data or invalid selections fail. This distinguishes
an authored empty animation frame from absent output/data.

WI 0059's complete original object projections establish that frame Y increases upward.
The public minimum-Y field is therefore `bottom` and the storage is bottom-row-first.
The original WI 0058 description/contact-sheet orientation was inverted; composition bytes,
anchors, bounds and complete byte expectations are unchanged. The corrected diagnostic review
and actual scene evidence live in [vehicle scene](vehicle-scene.md).

## Deliberate rendering scope

The composer retains every source texel at authored resolution, with nearest/transparent
assembly and each model's own MAL palette. The original perspective rasterizer samples the
source using different horizontal/vertical steps, fixed-point viewport rounding, optional
shear, depth/ground clipping and palette interpolation/shading. Those are outside this texture
construction contract. The original b31d unfiltered scan body is used as evidence for storage,
mirroring and transparency, rather than claiming the whole mission uses that mode.

The anchor oracle executes actual ad1e..ae40 setup with a mathematically derived projection
fixture yielding an inverse source step of exactly 65536. It checks every possible signed X/Y
byte against actual sign/negation/division/viewport instructions. The fixture separates the
original half-horizontal projection and biased truncation from the authored bitmap anchors.
It is an explicitly constructed projection input, not an inferred game constant.

The real scene, browser canvas and playable controls are unchanged by this step. Integrating
origin-relative textured vehicles into the shared scene, recovering world scale and view
bearing, animation, runtime initialization and model mission-palette mapping remain under 0041.

## Verification and visual evidence

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_model_bitmap.py \
  --originals --oracle --review-dir /tmp/wasm-fist-model-review
```

Unicorn uses `tools/rewrite/oracle_requirements.txt`; it is optional verification tooling, not a
runtime dependency. The DOS image pin/executor is shared with the previous model oracle. The
kernel pin is SHA256 102a5465e03f3b0397041b1bd783d33992ac2c14bb6e42a713af610ec08c3ab1.
`original_sprite_oracle.py` executes actual original priority insertion, full unfiltered sprite
loops and full anchor preparation without instruction hooks or code patches. Only projection/
scan parameters are supplied at their declared boundaries. Identical complete sprite inputs
may reuse cached results; all original entries and both mirror decisions are still verified.

The native CTest/WASM Node gate adds six groups for complete bitmap/ownership/error coverage.
The separately requested corpus requires all 34 pinned families and fails on skips. All 6,432
full bitmaps on each target include the 32 baseline facings plus every individual part's variant
at an opposing facing, with the other parts at variant zero. This covers every reachable list
in the full family direction map, independent part directions and every available variant;
it does not claim an exhaustive Cartesian product of all multi-part poses. Every bitmap byte,
complete declared bounds and retained post-destruction palette/indices are compared. All atlas
entries, including unused/lower-LOD sprites, are checked through original scanning in both
mirror directions. Original inputs are rehashed and remain unchanged.

Constructed fixtures prove unequal/equal/extreme unsigned priorities, observable piece order,
base/local selection, transparent overlap, mirroring, every signed-anchor extreme, explicit
empty frames, maximum 255x255 sprites, missing/malformed files and atomic invalid-selection
failures before and after partial selection. Sanitizer runs with the production flags and
ASan/UBSan/leak detection also pass all six groups and every original family.

`--review-dir` creates compact diagnostic PNG contact sheets from **actual C output transcripts**,
using only the Python standard library. Row order is BMP_C, M1_C, M3_C, T80_C; columns are facings
0/8/16/24. Integer nearest scaling fits every tile without cropping. The native and WASM sheets
were both inspected: tracks, hulls and barrels are recognizable in front/rear and side views,
parts assemble without rectangular transparent borders, and the source palette is retained.
These are authored asset-plane reviews, not final perspective mission frames. Both PNGs hash to
67ff74a1744a8ad58bbdb3d891b64ae8fb0eb0ede3ff624b1413e1e142a07f11 on the verified build.
The two contact sheets occupy under 64 KiB in total; temporary builds/logs/sanitizers stay under `/tmp`.

Sanitizer reproduction:

```sh
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itools/rewrite src/assets/model.c src/assets/palette.c src/render/model_bitmap.c \
  tools/rewrite/probe_io.c tools/rewrite/probe_source.c tools/rewrite/model_bitmap_probe.c \
  -o /tmp/wasm-fist-model-bitmap-sanitized
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/rewrite/test_model_bitmap.py \
  --target native --native-probe /tmp/wasm-fist-model-bitmap-sanitized --originals
```
