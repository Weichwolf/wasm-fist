Type: Work item
Title: Shared C11 scenario envelope and metadata decoder
Parent: 0041
Depends: 0040

## Contract

Read complete original FSG chunk envelopes into typed metadata and bounded immutable chunk/unit
views on native and WASM. Preserve full position widths, all known payload bytes and declared
DCBS record framing; fail incomplete/malformed input. Do not link the reconstructed engine.

## Evidence

Preparation parent `aef14d1`, reference `349ad31`; verified on 2026-10-06. Original reader machine
code d501/d7b5/d7e1 and BINF consumer/writer evidence are recorded in `docs/scenario-format.md`.

- `bash tools/rewrite/build.sh all` passed sequential native/WASM builds; native CTest passed
  renderer integration and scenario contracts. WASM passed renderer and seven synthetic scenario
  groups (the explicitly separate original-data group is skipped in the default CI gate).
- `python3 tools/rewrite/test_scenario.py --originals` passed all eight groups with no skips:
  47 pinned original missions, 574215 input bytes, 4213 records, 28 asset combinations, complete
  output equality with an independent format interpretation on both targets. Hashes are unchanged.
- The reaching INDIA4 failure showed a 16-byte space-padded sky name with no NUL. Fixed bounded
  DOS field handling; a synthetic regression covers that original representation.
- Tests cover all truncated synthetic prefixes, unchanged output on failure, full signed 32-bit
  positions, zero-count rosters, unknown chunk skipping, duplicate/missing chunks, invalid record
  counts/lengths, incomplete/trailing/invalid TERM and bad filename fields.
- `python3 tools/rewrite/check_style.py` passed strict LLVM 19.1.7 checks for every owned C unit;
  `git diff --check` passed. No compiler/style check was weakened.

The probe reads files to EOF, compares all known payload and unit-state bytes and uses NODERAWFS
only in the separate Node test executable. Runtime decode itself takes a byte span with no platform
I/O or allocations. Temporary fixtures are isolated under `/tmp` and removed by the test runner.

Raw PATH/STMP/PINF and DCBS state views are not gameplay implementations. Header metadata does
not establish mission timing rules. Parent 0041 and full acceptance remain open.

## Next

Continue 0041 through 0049: validated terrain/colormap KLC decoding and palette lookup for a real
scene, then model catalog/geometry, controls, simulation/HUD/combat/audio in bounded steps.

## Accept

Complete envelope/metadata output for every current original scenario agrees with the independent
interpretation on native and WASM; complete framing and malformed-input contracts pass with strict
tooling and immutable originals. Commit/push this verified bounded decoder.
