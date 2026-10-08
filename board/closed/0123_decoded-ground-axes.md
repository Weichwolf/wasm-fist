Type: Work item
Title: Deliver complete shared decoded ground-axis consumption
Depends: 0121, 0122

## Contract

Own complete f69:aae4 in the shared driver. Restore signed saved +a1/+a2
inputs and retain them through complete initialization and readiness. Steering
>=24 or <=-24 updates requested heading from current heading with word wrap;
the dead zone retains the request. Pedal >=24 or <-24 produces doubled negated
demand, otherwise zero; clamp to [-240,284]. Negative speed always sets profile 3
and refreshes, even when already selected. Otherwise modes above 1 set profile 0;
modes 0/1 retain their profile and emit explicit no-refresh. Reuse the existing
drive-profile/component/display owner. Validate used actor/output before mutation;
failure preserves both. Do not read clocks, targets, pool/RNG or device records,
and do not install a partial class/manual callback bank.

## Evidence

Original checkpoint ecea3cc publishes the exact independent model and four
fixtures:528,384 complete whole-memory/ABI axis/profile returns, 384 retained
four-actor calls through three genuine analog callbacks, 4,096 complete
class-start/readiness returns and 4,096 complete decoded-record copies. The
immutable reference, original image and pinned softgl remain unchanged.

The signed-byte reader uses portable per-branch two's-complement conversion.
Canonical fist_vehicle_axes contains two int8_t values. Numeric promotions are
explicit; strict checks remain unchanged. fist_driver_apply_axes operates on a
copy and commits actor/event together. Every success writes the event, including
false. Tests seed true before every application so missing false assignments
fail. Whole typed write-footprint checks permit only requested heading, throttle,
control mode and the existing per-class profile component; every unrelated field,
including axes and unused malformed target, is preserved.

Required native/WASM and production-fast-math ASan/UBSan each verify 762,368
complete observations without skips: 528,384 full original axis/heading/profile
contexts,6,144 saved-axis restore/initialize/prepare cases,204,800 held calls and
23,040 observations from 960 original ground actors across all 47 saved sources.
An empty case batch succeeds without output. Empty files, missing files, malformed,
truncated, trailing and late-invalid records fail without partial output. Null output/actor and malformed used type/component preserve
actor/event. Output digest:
c90ace26e3004f882f5a19641978b982874f6ae15f3839a7010a4b7ea42e3ab9.

Strict LLVM19.1.7 format/tidy passes all 99 owned translation units. The complete
unchanged sequential production command is exit code 0: 51 native CTests, 49 WASM
Python suites (47 unittest suites/346 cases and two plain-Python suites),
plus both Node gates. Compiler warnings are absent. Renderer verification
confirms 320x200 RGBA, checksum 3fa856ed and the fixed 2 GiB shared heap.
Required owner/corpus/memory groups have no skips; prior optional reference
groups retain their explicit policy. A detached owning subprocess waits for
each exact command and records its actual terminal exit, log/program hashes
and unchanged source pins before advancing; no missing or partial run passes.

The initial strict run exposes implicit conditional integer promotions and
signed-char-to-number conversions. Explicit branch returns/casts resolve them;
no checks are disabled. Final source review also catches a development-specific
Python default build root. Restore /tmp/wasm-fist-rewrite and interrupt the
beginning production attempt before acceptance. The complete unchanged final
script runs again on the corrected, frozen source; no expected output changes.
Compact failure/repair receipts retain this distinction.

Actual native SDL and browser gates are exit code 0 on isolated canonical TRAIN1
assets and exact production programs. Normal five-second native and 30-second
browser deadlines are unchanged. Complete input/frame/weapon/fire/reload/
pause/focus/shutdown/failed-start checks pass. Seventeen native and six browser
captures are pinned; after frames were visually inspected and show complete
textured terrain, vehicle and HUD. No deliberate visual-design change is made.

Exact source/program/command/terminal/capture pins and compact receipts are under
/tmp/wasm-fist-0123-review. Reproduce with an external production build:

    FIST_REWRITE_BUILD_ROOT=/tmp/wasm-fist-axes-reproduction-build bash tools/build.sh all
    python3 tools/check_style.py --build-dir /tmp/wasm-fist-axes-reproduction-build/native
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tests/test_ground_axes.py --target all --originals --build-root /tmp/wasm-fist-axes-reproduction-build

For memory checks, configure a separate /tmp build with RelWithDebInfo,
-fsanitize=address,undefined -fno-omit-frame-pointer, unchanged production
fast-math and sanitizer link flags. Build fist_ground_axes_probe sequentially.
Run the same required corpus with --target native --originals --native-probe
pointing to the instrumented program, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
and UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1.

## Next

Reuse this complete driver owner while composing the remaining original manual
and class returns under 0121. Complete curved manual turret consumption is the
next bounded C step 0124; full a57a/device producers and camera/contact/engine
contracts remain required. Continue canonical living world scheduling and first
playable battle/PCM/outcomes afterward.

## Accept

Met: the complete decoded-axis consumer, original domains, saved-state retention,
held events and failure/write-footprint contracts pass on both targets. Exact
source/program/terminal/capture pins, strict full style, production regression,
fast-math memory and actual scene checks all pass. Publish only the exact nine
C/probe/build files. Full manual/class/world scheduling, device producers, battle/
PCM/outcomes and complete-game acceptance remain open; the final WASM streak is 0.
