Type: Work item
Title: Original model catalog and ground-vehicle part pose
Depends: 0054, 0056

## Contract

Resolve every original model code to its DOS basename and decode the original default render
selection of ground-vehicle types 0–3 from owned snapshots. Preserve independent turret/hull
headings, render scale and all part variant/direction bits. Quantize a supplied view bearing
with the original 32-direction rounding, then expose each part as a typed model pose. This
immutable visual selection does not execute gameplay initialization, projection or drawing.

## Evidence

Original class dispatcher c4fb..c507 reads codes 4/16/28/40, corresponding to M1_C/M3_C/T80_C/BMP_C
in the STR model-loader list and MGA filename service. Builder c5fc/c91c/ca6b exchanges turret
and hull headings, emits scale from snapshot +14 and XORs the two bytes at +a9/+aa with 80h.
Renderer 2fa0..2fca chooses the second heading using bit 7 and indexes variants with low 7 bits.
Facing quantization is 059a (bearing recovery) followed by 05a8..05b6 (rounding).

Implemented `src/assets/vehicle.{h,c}`, a single complete catalog owner and bounded immutable
visual/part-pose APIs. Added a production probe, native CTest/WASM contract gate and the original
DOS/MGA instruction oracle. See `docs/vehicle-model-selection.md` for the recovered fields and
exact scope. No generated/reference source or originals were modified.

Verified 2026-10-06:

- `bash tools/build.sh all`: all seven native contracts and all WASM gates pass. Default
  constructed-fixture gates explicitly skip the separately requested original corpus.
- `python3 tools/check_style.py`: strict LLVM 19.1.7 format/tidy passes all 20 owned C
  translation units, with unchanged compiler flags and no suppressed/disabled checks.
- `PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_vehicle.py --originals --oracle`:
  all six groups pass on production native and WASM with no skips. Complete outputs cover every
  model catalog entry, all 960 ground snapshots in the 47 pinned FSGs and all 65536 relative
  angles. Three complete unsigned-bearing sweeps are checked on each target. All 170 named
  model assets are verified against their size/hash pins; original inputs remain unchanged.
- Original filename service 0197 is executed for the complete STR loader list; c4fb/c5fc/c91c/
  ca6b build full nodes and restore the entire source snapshot. Actual 2fa0 direction selection,
  2fc5 variant extraction and 05a9 subtraction/rounding independently confirm the C fields.
  No hooks or instruction patches are used. The host supplies only the two-part count/output
  node at the render-list boundary. Bearing recovery, initialization and rasterization are excluded.
- Synthetic cases cover all 256 bytes for both parts in every ground type, distinct turret/hull
  headings, bearing wrap, zero/maximum scale, empty and unsupported sets, malformed/missing
  input and atomic getter/decode errors. The probe frees both the FSG input and all owned unit
  snapshots before observing visual output/poses, proving the new independent field ownership.
- ASan/UBSan with leak detection and the production flags passes all six groups with the
  complete original corpus. No presentation changed; vehicle drawing is not claimed.
- All builds, logs and experiments are external under `/tmp`. Obsolete owned research/log/
  sanitizer artifacts are removed after recording this compact evidence.

## Next

Catalog and immutable ground-vehicle part selection are complete. Recover sprite anchor/
projection and palette translation, then install/draw original parts in the shared scene.
Animation updates, other-class selection and gameplay initialization remain open under 0041.

## Accept

All 34 catalog names and all 960 original ground-vehicle snapshots match original instructions
on native/WASM; every 16-bit relative angle is checked. Meaningful synthetic tests cover separate
headings, both direction selectors and all variant bytes, signed poses, zero/max scale, absent
inputs and unsupported/malformed definitions. Required builds/style pass. No rendering claims.

## Heading-label correction

2026-10-06, WI 0060: original 7d0f/8917/9911 establish word +10h = word +26h +
relative turret offset +89h. The primary +26h heading is the hull; secondary +10h is
the absolute turret heading. The earlier descriptions reversed those names. Correct the
public header/documentation; numeric offsets, node ordering and complete bitmap
expectations remain unchanged. The complete initialization oracle confirms both independent
preserved directions.
