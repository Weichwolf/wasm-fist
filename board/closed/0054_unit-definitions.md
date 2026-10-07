Type: Work item
Title: Decode owned mission unit definitions and original roster identity
Depends: 0048

## Contract

Decode original DCBS snapshots into owned, immutable typed identity/pose definitions in shared
C11. Preserve every snapshot byte. Recover normal-side registry and 32-slot platoon roster
assignment, including last-record replacement and type-23 placeholder slot-zero exclusion.
Saved pool indices are evidence, not runtime identities. This step does not initialize vehicles,
swap sides, simulate units, resolve models or implement player selection UI.

## Evidence

Original image and instruction ranges are recorded in docs/unit-definitions.md. The original
constructor uses type flags to allocate 55/251-byte objects and installs registry generations;
the scenario reader restores the newly allocated pool index after reading the saved snapshot.
Roster assignment is separate from SHDR position and DCBS record order.

Verified 2026-10-06:

- `bash tools/build.sh all`: all five native CTest contracts and WASM gates passed.
  The default constructed-fixture gates explicitly skip local-original coverage.
- `python3 tools/check_style.py`: strict LLVM 19.1.7 format/tidy passed for all 16 owned C
  translation units; no warning suppressions were added.
- `/tmp/wasm-fist-decoder-oracle/bin/python tests/test_units.py --originals --oracle`:
  all seven groups passed on both targets with no skips. All 4213 records in 47 hash-pinned
  scenarios retain every byte; constructor class, full-width poses, every registry generation
  and all roster slots match actual original instructions within the documented scope.
- `python3 tests/test_scenario.py --originals`: all eight native/WASM groups passed with
  no skips after centralizing the signed little-endian read contract.
- ASan/UBSan/leak-detection unit probe: all seven groups and the complete original corpus passed.
  Exact reproduction commands are in docs/unit-definitions.md. Presentation is unchanged.

## Next

Definition decoding is complete. Use the selected roster vehicle's pose for inspection and
playable scene installation, then recover model catalog/sprite parts and vehicle initialization.

## Accept

Both production targets pass typed decoding/ownership/error tests and complete 47-scenario,
4213-record tests. An independent pinned original-instruction oracle proves allocation class,
registry generation, pose offsets and normal-side roster assignment, with its scope explicit.
Strict format/tidy pass. Record evidence, commit and push the bounded success.
