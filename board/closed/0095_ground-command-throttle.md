Type: Work item
Title: Own complete command throttle and drive-profile transitions
Depends: 0082, 0094

## Contract

Recover and implement complete ad2f eight-entry throttle dispatch, its unconditional ad3b
drive-profile continuation and the controlled bank's standalone ad3b. Preserve unsigned range
comparisons, signed throttle outputs, lazy PINF +6 choice admission and signed terrain-pitch
thresholds. Retain independent saved word +99; do not replace it with navigation range +53.
Execute and account for actual a19e class setters and complete f69:7a5b display refresh.
Use canonical actor/orders and one shared profile setter. Preserve RNG, targets, routes and
unrelated world state. No partial parent bank or playable-battle claim.

## Evidence

The frozen image has DS:97f0 entries ad62/ad8c/ae06/addb/ae07/ae25/ae26/ae2c. Mode 4 is a
genuine return while mode 6 consumes word +99. ad3b admits profile byte +90 <=1, compares
signed word +34 with 0x0e00 and invokes a19e on a transition. Actual class setters write +90,
mark component +d6/+c8/+ca/+d2 with 3 and execute f69:7a5b. That complete far service writes
3 to four global display bytes at DS:8e58/8e5a/8e5c/8e5e; it has no voice/device call.
Complete unchanged original returns and the independent actor/world model verify these
instruction observations at the child-callback boundary.

## Next

Complete. Continue 0081 with target feedback production/acquisition, remaining parent
callbacks and full heading/RNG/class consumption. Preserve this child contract and the
existing physical target-lifetime repair. First playable battle and full rewrite acceptance
remain open.

## Accept

Complete current-artifact native/WASM and sanitizer comparisons pass without required skips;
all 47 real prepared missions consume the complete callback. Original complete returns prove
exact actor/component/display effects and unchanged unrelated DGROUP/RNG. Restoration and
readiness retain +99. Used invalid inputs fail atomically; unused descriptor/target fields do
not hide original branches. Strict builds/style and actual controls/presentation remain good.
Full target feedback production/acquisition, remaining sixteen-entry parent callbacks, living
battle, audible PCM/devices/outcomes and the independent full-game WASM streak stay open.

## Verification

The final required current-artifact production gate passes eight groups without skips on
both native and WASM in 766.590 seconds. The complete ASan/UBSan retry passes the same eight
groups without skips in 502.599 seconds. Each target proves:

- 872120 scalar cases and 780 atomic rejections.
- 871340 complete original command/profile-setter returns, plus 512 reached original class
  motion/manual-turret returns.
- 188 canonical prepared worlds: all 47 original missions, all eight original height planes
  and four details, with 30720 complete throttle-bank callbacks.
- All 65536 control words in every mode; complete navigation/target-range and signed-pitch
  domains, every retained profile/maneuver byte and all direct setter bytes for all classes.
- 64 actual PINF increment/decrement UI cycles and 768 actual C target-loss goal/bearing
  continuations into throttle, including 352 stops without a valid replacement goal.
- Full actor/component/display effects, unrelated DGROUP/world preservation, unchanged RNG,
  source release and atomic failures for used invalid fields/output/batch transport.

Production and sanitizer normalized output SHA256:
459f0937ba5f4344cf1db0bae10b27cedbfb1632fe44ee559a393299d46f8321.

The separate required original-retention gate passes one group without skips in 0.321 seconds.
All four classes, links 0/1/2 and eight word boundaries give 96 complete c296 starts and 96
complete readiness-method returns. Full actors match independent start/reset models; +99,
unrelated readiness DGROUP and RNG are retained. Output SHA256:
1effaddf85c6ec7bf37c53c597d226ff11f46640956ba73038db4d50b2e2840c.
The C batch independently verifies supplied +99 words through restoration/start/preparation.
This additional original test has its own verified source hash and complete gate; the 246
source/configuration hashes and 16 selected artifacts used by the main gates stayed unchanged.

Strict LLVM 19.1.x format/tidy passes all 87 owned translation units and headers. Complete
`tools/build.sh all` passes 41 native CTests in 218.05 seconds, 39 WASM Python suites and two
Node pixel probes. Optional original skips in the default build are covered by the separate
required original gates with zero skips. Owned -Werror and all requested flags remain active;
unchanged external toolchain warnings are not claimed as owned diagnostics.

Actual SDL, isolated Chromium and ASan/UBSan-instrumented SDL TRAIN1 controls/presentation pass
with terminal exit zero. Both production after frames were visually reviewed. Movement,
steering, turret, pause, weapon selection/reload, focus loss, shutdown and startup failure
remain good. The producer exposes complete drive-display refresh; platform UI consumption is
still a later flow, and these callbacks are not yet installed in a partial living parent bank.

The first required memory attempt ended with authoritative exit 143 (SIGTERM), without a
reported cause or complete summary/receipt. Its partial progress is excluded. The unchanged
full retry ran after other gates completed and passed; no sanitizer defect or skipped required
coverage is accepted. Earlier filtered/pre-final-probe pilots are supporting evidence only.

Compact source/artifact/original hashes, immutable refs, exact commands, complete gate results,
log hashes/tails and two reviewed images remain in `/tmp/wasm-fist-0095-review/receipt.json`.
Obsolete owned 0095 builds/captures/logs are removed after retaining that evidence. Original
files, ghidra, the immutable reconstruction tag and pinned softgl are unchanged. Full parent,
target feedback/acquisition, living battle/PCM/devices/outcomes and the independent complete
WASM streak remain open; the streak is zero.

## Reproduction

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_ground_throttle.py --target all --originals --oracle --review-dir /tmp/wasm-fist-0095-both-required
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_ground_range_retention.py --review-dir /tmp/wasm-fist-0095-retention-required
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/check_style.py
    CTEST_PARALLEL_LEVEL=4 bash tools/build.sh all

For the required memory gate, build the four probe/scene targets with ASan/UBSan and execute
the full throttle suite with --target native --native-probe pointing to the sanitized probe,
--originals --oracle, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 and
UBSAN_OPTIONS=halt_on_error=1. Exact configuration, target list and real scene commands are
retained in the receipt. See docs/ground-command-throttle.md for the complete recovered bank.
