Type: Work item
Title: Inspect original terrain from the roster-selected vehicle pose
Depends: 0053, 0054

## Contract

The static inspection preview uses explicit default roster slot zero, its owned unit X/Y and
heading, rather than SHDR positions or the first record. The inspection altitude remains a
deliberate ground-relative view, not original cockpit initialization. Missing/nonvehicle player
definitions fail without publishing a frame. No artificial vehicle model is introduced.

## Evidence

0054 proves original registry/roster and pose fields, including TRAIN1's registry-index-32
player away from its header/first record. Original player selection chooses a roster slot;
the preview default is explicitly zero. Runtime selection UI and gameplay remain open.

Verified 2026-10-06:

- `bash tools/build.sh all`: five native CTest contracts and all WASM gates pass. The
  new eight-group scene suite mutates actual unit poses for period/signed-width checks, proves
  SHDR/first-object changes do not move the view, and rejects missing/nonvehicle roster zero.
- `python3 tools/check_style.py`: strict LLVM 19.1.7 checks pass for all 16 owned C units.
- `python3 tests/test_terrain_scene.py --originals`: all eight groups pass on both
  targets, no skips; complete pinned AZER1/TRAIN1 inputs are copied and hash-rechecked.
- `/tmp/wasm-fist-decoder-oracle/bin/python tests/test_units.py --originals --oracle`:
  all seven unit groups and all 47 original missions still pass on native/WASM with no skips.
- External ASan/UBSan production-flag build: all eight native scene groups, including both
  original scenes and failure paths, pass with leak detection and halt-on-error enabled.
- Native original PPM images and actual Chromium canvas screenshots are visually reviewed.
  AZER1 is unchanged: native `11444fd3`, WASM `41900eee`, 38156 colors. TRAIN1 is placed at its
  roster-zero unit (registry 32): native `8b721dac`, WASM `afb2be08`, 38318 colors. Its WASM PPM
  SHA-256 is `50805f5cdbd13a765cf1d6cba615d192619717de1346f5d99dac0fd2ecd66501`.
- Both actual original-browser gates compare every canvas RGB/alpha/row with the complete C
  output, confirm isolation/completion and no runtime errors. The constructed browser view
  also passes and is visually reviewed (`cbe478ea`, 3877 colors), confirming the CI fixture.
  Current review inputs and images are under `/tmp/wasm-fist-roster-review/`; server processes
  stop automatically. Obsolete scratch/sanitizer output is removed after acceptance.

The explicit camera choices and reproduction commands are in docs/terrain-scene.md. Vehicle
models, original cockpit/ground initialization, interactive presentation and gameplay/audio
remain open; this change does not close parent 0041 or the final WASM completion gate.

## Next

Roster-based inspection is complete. Recover model catalog/sprite layouts and initialize/install
the vehicle; continue interactive native/browser controls, simulation, HUD/combat/outcome/audio.

## Accept

Both production targets pass complete frame, pose/heading, periodic/signed coordinate and
missing-player tests. Strict format/tidy pass. Actual TRAIN1/AZER1 native/browser presentation
is reviewed, with full canvas/C-output comparison and unchanged pinned original inputs.
Record, commit and push this bounded presentation change.
