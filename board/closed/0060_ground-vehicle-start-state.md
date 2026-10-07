Type: Work item
Title: Initialize owned typed ground vehicle state from original definitions
Depends: 0059

## Contract

Introduce shared C ground-vehicle runtime start state, separating hull/turret directions,
drive commands and initialized control/weapon/component state from immutable FSG definitions.
Execute the original two random draws and class defaults using explicitly supplied random
state and link mode. Missing/invalid inputs preserve output and random state. Do not execute
target selection, terrain installation or a full simulation tick until their rules are recovered.

## Evidence

DOS c296..c2f1 calls 0291 twice and dispatches type 0..3 initialization through DGROUP e4e0
to 7b91/8744/8f9f/973a. These initialize controls, projection/camera fields, weapon counts and
complete component templates copied through 1e27. Actual original bytes/routines are available
in the frozen DOS image. 7d0f/8917/9911 set word 10 = word 26 + word 89: 26 is hull heading,
10 is turret heading. WI 0057's labels were reversed; numeric selection remained correct.

## Next

Complete. Continue parent 0041 with original movement/controller updates and terrain/suspension
installation, then connect initialized state to interactive native/browser rendering.

## Accept

All four classes initialize deterministically on native/WASM with the original control fields,
class defaults, component payloads and random consumption. Definitions may be destroyed before
observing state. Full original-corpus, input/error/ownership, strict-style and production checks
pass; initialization is not claimed as a playable mission or full vehicle-update implementation.


## Verification

2026-10-06: `bash tools/build.sh all` passes all ten native CTests and every WASM
asset/scene/pixel gate. `python3 tools/check_style.py` passes all 27 owned C units
with the unchanged LLVM 19.1 checks and required compiler flags. Default tests explicitly skip
the requested original-corpus group; the complete gate has no skips.

`PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python
 tests/test_vehicle_start.py --originals --oracle` passes all seven groups on both
production targets (final run 85.399 s). All 960 ground snapshots across 47 hash-pinned FSGs
and constructed cases give 3012 complete initialized observations per target. Independent
expected writes match every byte of the original 251-byte object after actual c296, class
initializer, 1e27 template copy and RET. Each template byte and final random state match typed C.
There are no instruction hooks, patched branches or substituted original methods.

The random gate checks all 65536 input words in each of four stream slots and three complete
65536-draw sequences: 458752 returned values per target versus actual complete 0291 returns.
It verifies cursor wrap, zero seeds, final words and original AX/SS:0342 agreement. Class tests
cover all 256 object/secondary flag bytes, independent hull/turret directions and signed motion
fields, all 256 link-mode bytes, complete payload sizes 57/62/59/61, non-ground/empty scenarios
and missing/malformed inputs. Direct API checks preserve every output byte and random field
on unsupported types, invalid pointers, inconsistent identity/length and invalid cursor.
All FSG/snapshot storage is destroyed before observations are printed. Original hashes remain
unchanged. An independent ASan/UBSan/leak-checking probe passes all seven native groups and
the full original corpus (5.100 s).

Original turret wrappers 7d0f/8917/9911 additionally execute 27 complete returns, proving
word +10h = +26h + relative offset +89h, including unsigned turn wrap. Correct the earlier
reversed hull/turret labels in the public header and WI 0057 documentation; numeric model
selection and bitmap expectations stay unchanged. The original output comparison also caught
and corrected byte-placement errors in the first transcription of the T80/BMP templates.

See `docs/vehicle-start-state.md` for recovered fields and reproduction. This state-only step
changes no displayed frame; reviewed WI 0059 native/browser captures remain the presentation
baseline. Runtime terrain/suspension installation, targets, movement, firing/damage/animation,
boot seeding/link setup and save-state restoration remain open. Commit/push this verified
bounded success and remove owned temporary logs and the sanitizer executable from `/tmp`.
