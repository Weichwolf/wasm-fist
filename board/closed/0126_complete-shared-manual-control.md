Type: Work item
Title: Deliver complete shared selected-ground manual composition
Depends: 0121, 0122, 0123, 0124, 0125

## Contract

Implement both complete selected manual banks through the existing axis, driver,
curved turret and elevation owners. Retain saved weapon/view inputs and shared
controls across actors; select and refresh the view before weapon scaling.
Validate used state and commit actor, controls and events atomically. Unselected
and inhibited paths ignore unused state. Physical devices and complete class/
world scheduling remain separate contracts.

## Evidence

Closed 0125 supplies the independent original parent model, complete banks,
saved retention, all 47 sources / 960 actors and genuine selector producers.
The shared owner is fist_driver_apply_manual. It reuses all existing children;
fist_vehicle_refresh_turret_view owns the complete three-component refresh.
Saved +a0/+a4 bytes restore and retain through initialization/readiness. No
physical device identity is inferred from these decoded input bytes.

Eight required tests pass 1250505 observations per production native, WASM and
production-fast-math ASan/UBSan target without skips: 591872 parent domains,
288000 held calls, 12288 cross-actor calls, 6144 restoration/start/readiness
observations, 1944 runtime-target cases, 263857 guards and 86400 all-47 corpus
observations. Entire typed write footprints, late failure preservation, explicit
false events and invalid batches without partial output are checked. The domain
generator is AST-identical to the proved original fixture; the independent
parent model is unchanged. Output SHA256:
c681e2b9f98f9c7ee40a52c12fa3909d94a3498e0ed17fde4d0aaf52f26f991b.

Strict LLVM 19.1.7 format/tidy passes all 101 owned translation units. Actual
compile commands retain all requested warning/alias/fast-math flags and owned
-Werror; the memory build also retains address/undefined sanitizers. The full
unchanged production command exits 0: all 53 native CTests, 51 WASM Python
scripts (49 unittest suites / 360 tests and two plain verifiers) and both Node
gates pass. Default optional-original policy is unchanged; the separate required
manual --originals gates above have no skips.

The first production attempt is interrupted with exit 143 during incomplete
WASM regression and is rejected. A transient user service owns the complete
successful restart and actual terminal receipts. The first native scene attempt
exits 1 before window creation because the new build lacks isolated preview
assets; it is rejected. The unchanged prepare_driving_preview.py provisions
25 pinned read-only TRAIN1 source assets plus a manifest under /tmp. Both exact
scene commands then pass sequentially on the same production programs: native
10.916 seconds and browser 9.541 seconds, with normal five-second native and
30-second browser deadlines unchanged.

All 13 canonical native and six browser captures pass complete output/state
linkage. Four earlier native acknowledgement frames are retained separately.
Ten representative actual frames are inspected: terrain/vehicle/HUD, movement,
independent turret heading, weapon selection/cycling and reload/pause labels.
Browser CSS scaling is inverted for capture linkage; its before/after pixels
match the actual complete C-frame hashes. An external auditor's initial unscaled
browser-size assumption is corrected from unchanged HTML, without changing any
runtime test or assertion. Exact source/program/command/log/capture pins, rejected
attempts and compact receipts are under /tmp/wasm-fist-0126-review.

Reproduce from the committed source:

    FIST_REWRITE_BUILD_ROOT=/tmp/wasm-fist-manual-reproduction-build bash tools/build.sh all
    python3 tools/check_style.py --build-dir /tmp/wasm-fist-manual-reproduction-build/native
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tests/test_manual_control.py --target all --originals --build-root /tmp/wasm-fist-manual-reproduction-build
    python3 tests/prepare_driving_preview.py --mission --scenario armoredfist/FISTDATA/TRAIN1.FSG --build-root /tmp/wasm-fist-manual-reproduction-build --output-dir /tmp/wasm-fist-manual-reproduction-build/wasm/assets
    python3 tests/verify_driving_native.py --mission --scenario /tmp/wasm-fist-manual-reproduction-build/wasm/assets/TRAIN1.FSG --assets /tmp/wasm-fist-manual-reproduction-build/wasm/assets --native-preview /tmp/wasm-fist-manual-reproduction-build/native/fist_driving_preview --output-dir /tmp/wasm-fist-manual-reproduction-native
    python3 tests/verify_browser.py --driving --build-dir /tmp/wasm-fist-manual-reproduction-build/wasm --output-dir /tmp/wasm-fist-manual-reproduction-browser --port 8135

For memory checks, configure a separate /tmp RelWithDebInfo build with
-fsanitize=address,undefined -fno-omit-frame-pointer and sanitizer link flags,
retaining production fast-math. Build fist_manual_control_probe sequentially and
run the required test with --target native --originals --native-probe pointing
to it, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 and
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1.

## Next

Prove complete original selected/unselected physical contacts, tree damage and
cached-source producer/lifetime behavior before shared C. Reuse existing geometry,
predictive obstacle, damage arithmetic and physical pool owners. Preliminary
external contact research does not accept the complete next contract. Contact,
maintenance/engine, devices, full class/world scheduling, playable battle, PCM,
outcomes and all remaining game surfaces stay open under 0121/0041/0065.

## Accept

Met: both complete manual banks, shared controls/refresh order, retained saved
inputs, used-state guards, full write footprints and late transactions pass
required original/native/WASM/corpus/fast-math memory coverage without skips.
Strict complete style, full production regression and actual native/browser
scenes pass. Publish the exact nine verified code/test/build files and updated
contracts. No dependency or deliberate visual design change is included. Full
class/device/world/game acceptance remains open; final complete-game WASM streak
is 0 until independent complete-game verification.
