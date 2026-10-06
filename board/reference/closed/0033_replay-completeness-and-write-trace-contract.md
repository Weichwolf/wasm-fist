Type: bug
Title: Frame and PCM acceptance rejects missing, malformed and truncated output
Parent: 0012

## Delivered

The verifier rejects missing/invalid RGB frames, missing/extra PCM samples, format differences,
failed producers and absent required image references. Editor mode markers retain their file
contracts. PCM container metadata may differ; rate, channels, width, count and samples may not.
`tools/compare_output.py` reports the first differing pixel/sample; `wavcompare.py` is diagnostic.

## Proof

Implementation follows baseline 22dfff2; see this item's closing commit for the exact source.

- Old verifier failed four negative integration cases (empty selection, absent/truncated frame,
  failed producer with a frame): `scratch/verify/accepted-0033/original-red.log`.
- `python3 -B -m unittest discover -s tests -v`: 19 tests pass, including the editor-marker
  regression found during integration. No existing frame, PCM or editor expectation was weakened.
- `verify.sh both`: all 178 flows pass in disjoint groups 45+45+44+44. All four exit 0; the flow union
  exactly matches the manifest. Logs, captures, filters, source/binary hashes and `coverage.txt`:
  `scratch/verify/accepted-0033/`. The failed earlier matrix remains in `scratch/verify/full-0033/`.
- `bash tools/check_flow.sh '^intro$'` exits 0: all tests, `make check`, native/WASM builds and
  original-reference/cross-target intro comparison. Evidence: `scratch/verify/run.9V9tB0/`.
  Reproduce the complete matrix with `bash tools/check_flow.sh` (no filter).
- Build prerequisite regression at base `0e211fa`: system-installed `EMCC=emcc` made the WASM
  builders invoke `./em++`, so the real C++ compilation failed in `run.FOoxMo/wasm-build.log`.
  Both builders now resolve the compiler through PATH before selecting its sibling `em++`.
  `test_compiler_on_path_compiles_cpp_and_links` covers the command-name invocation on both scripts.
  `bash tools/check_flow.sh '^(intro|mainmenu)$'` passes 47 tests, all 586 exact engine patches,
  native/WASM builds and both selected reference/cross-target flows (2 passed, 0 failed).
  Evidence: `scratch/verify/run.GItDVW/` (base plus saved patch). This is bounded build/flow proof;
  the broad matrix was stopped after four passing flows and is not counted as full coverage.

## Remaining scope

These are the existing snapshots/crops, editor contracts and captured audio intervals; this does
not prove full temporal fidelity or final mixed audio. Continue 0034 for complete synchronized
sequences, 0003 for mixer fidelity and 0012 for full-run parity. Diagnostic replay gaps remain 0035.
