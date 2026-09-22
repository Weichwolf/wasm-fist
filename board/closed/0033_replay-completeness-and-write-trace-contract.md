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

## Remaining scope

These are the existing snapshots/crops, editor contracts and captured audio intervals; this does
not prove full temporal fidelity or final mixed audio. Continue 0034 for complete synchronized
sequences, 0003 for mixer fidelity and 0012 for full-run parity. Diagnostic replay gaps remain 0035.
