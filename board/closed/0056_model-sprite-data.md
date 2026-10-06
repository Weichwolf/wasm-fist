Type: Work item
Title: Load owned original directional model sprite parts
Depends: 0054

## Contract

Decode and own complete original model families (MAL, M00, M08, M16, M32) in shared C11,
including sprite atlases, directional records, part variants, priority/placement and mirror/base
references. Install the complete 32-direction record map in original load order. Preserve all
record bytes; source buffers may be reused/freed. Missing/malformed model data fails atomically.
These original assets contain composed sprites, not vertex meshes. Runtime unit-to-model catalog,
mission palette translation, model rendering and simulation remain subsequent contracts.

## Evidence

Original 1a45/1b35/2fb7/301d instructions supply record lengths, facing rotation, part tables,
sprite dimensions/texels and reference flags. Frozen patches 471/573 are locating evidence;
original instruction execution must verify the new decoder's output before acceptance.

`docs/model-format.md` records the wire layout, signed reference/placement contract, direction
selectors and exact verification commands. The owned source callback may reuse every input
buffer; both files and families preserve complete record/texel data independently.

Verified 2026-10-06:

- `bash tools/rewrite/build.sh all`: six native CTest contracts and all WASM gates passed.
  Constructed-fixture gates deliberately skip the separately requested original corpus.
- `python3 tools/rewrite/check_style.py`: LLVM 19.1.7 strict formatting/tidy passed for all 18
  owned C translation units; no checks or requested compiler flags were weakened.
- `/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_models.py --originals --oracle`
  (native and WASM): all six groups passed with no skips. All 34 families/170 pinned inputs,
  612 records and 1679860 texels are checked completely, not through checksums/filters.
- The independent original-instruction oracle confirms every sprite's dimensions/complete
  texels, all part variants' priority/placement/mirror/base fields and complete final direction
  mappings, including original 197b/1996/19b9 neighbor fills. DOS/heap/relocation, palette
  translation and final rasterization are outside the proved data contract.
- ASan/UBSan/leak-detection probe with production flags passed all six groups and all original
  families. Complete snapshots/pixels are inspected after source destruction or input overwrite
  and free. Repeated destruction, null/getter/error output contracts, exact EOF/truncated prefixes,
  invalid offsets/dimensions/refs, absent files/faces, last-record replacement and 128-variant
  boundary behavior are covered. Originals are rehashed and unchanged after use.
- Presentation is unchanged; no vehicle/mission visual completion is claimed. Builds/fixtures/
  sanitizer outputs/logs are external under `/tmp`; obsolete owned scratch is cleaned after success.

## Next

Model data loading is complete. Resolve runtime unit catalog and part pose/animation/palette,
recover sprite anchor/projection, and install/draw original vehicles in the shared scene.

## Accept

Native/WASM behavior and ownership/error tests pass, with complete coverage of every pinned
original model family and no missing output or skipped original coverage. Actual original
instructions independently confirm directional mapping, parts and all sprite selection fields.
Strict format/tidy and memory checks pass. Record, commit and push the bounded success.
