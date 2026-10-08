Type: Work item
Title: Deliver complete shared curved manual turret control
Depends: 0121, 0122, 0123

## Contract

Consume complete a376/a3a8 curved turret helpers and genuine a59c/a5a6 callers
in the existing shared weapon owner. Reuse driver control refresh and the same
private target/elevation cancellation used by elevation. Preserve wrapped
selector arithmetic, the entire nonmonotonic sixteen-word curve, logical view
scaling and wrapping requested offset. Fixed callers retain shared selector
88/232 across actors. Validate used actor/control/direction/reference before
mutation; failure preserves actor/controls. No clock, RNG, allocation, target
resolution or partial manual/class bank.

## Evidence

Published a4f5723 versions the independent model and complete original fixtures:
393,216 whole-memory/ABI helper returns and 2,048 retained genuine fixed-caller
returns across all four actual allocated classes. Both fixed selectors choose
word 9 = 122 before view scaling; the complete original recheck published with
0123 confirms this. The immutable reconstruction and pinned softgl are unchanged.

The complete helper adjusts a copied actor and commits only after existing
control refresh succeeds. The caller selects controls on a copy and commits both
only after the helper succeeds. Direction validation accepts only left/right;
explicit invalid/count enum markers permit rejection tests without invalid enum
casts. Tests require whole typed actor preservation outside allowed fields,
including nonselected actors, shared selectors, runtime target bindings and
unrelated axes/RNG/phase state. Absent targets retain elevation; present targets
clear both binding forms and elevation before adjusting the prior turret request.

Required native/WASM and production-fast-math ASan/UBSan each verify 935,696
complete observations without skips: 393,216 full curve/view/selector/requested
contexts, 512,000 held calls, 1,200 runtime-target contexts, 6,240 cross-actor shared
calls and 23,040 observations from 960 ground actors across all 47 saved sources.
The shared sequence includes 2,048 fixed callers and 32 raw-helper tails for each
of three initial seeds, using one actual shared controls object throughout.
Raw helpers preserve its selector. Empty batches succeed without observations;
empty/missing files, malformed/truncated/trailing and late-invalid records fail
without partial output. Null inputs and malformed used actor/direction/reference
preserve complete actor/controls. Output digest:
15c427cec0875434448b2096fd62c1e0adb3c85ff160cb7356cd001eb867079c.

The initial strict probe run reports cognitive complexity, ambiguous adjacent
parameter order, parentheses and constant enum casts. Small shared-step/caller/
helper functions and explicit validated direction markers resolve these findings.
Checks and expected behavior stay unchanged. The production owner is clean.

Strict LLVM19.1.7 format/tidy passes all 100 owned translation units. The complete
unchanged sequential production command exits 0: 52 native CTests, 50 WASM
Python suites (48 unittest suites/352 cases and two plain-Python suites), plus
both Node gates. Compiler warnings are absent. Renderer verification confirms
320x200 RGBA, checksum 3fa856ed and the fixed 2 GiB shared heap. Required owner/
corpus/memory groups have no skips; prior optional reference policy is unchanged.
The detached owning subprocess waits for each exact command and pins actual
terminal exit, source, log and program hashes before proceeding.

Actual native SDL and browser gates exit 0 on isolated canonical TRAIN1 assets
and exact production programs. Normal five-second native and 30-second browser
deadlines are unchanged. Complete input/frame/weapon/fire/reload/pause/focus/
shutdown/failed-start checks pass. Seventeen native and six browser captures
are pinned; after frames were visually inspected and show complete textured
terrain, ground vehicle and HUD. No deliberate visual-design change is made.

The first browser run, concurrent with production regression, exceeds the
unchanged 30-second limit after four captures and is explicitly rejected.
Its failure/log/capture/process receipts are retained. A read-only comparison
against pinned published 0123 programs proves identical standard WASM sections,
JavaScript loader and all 26 UI assets. The exact browser command is rerun
after production terminal success and passes with the same normal deadline
and complete checks; no code, expected output or timeout is changed. This
records the rejected run without claiming a proven timeout cause.

A separate timing-only browser copy retains every original assertion and the
normal 30-second bound. It completes in 16.408 seconds; the explicit 5.5-second
simulation advance takes 51 milliseconds. Its six captures, timing marks and
log hash are pinned separately. This diagnostic is not the required unmodified
browser command and is not used as its acceptance substitute.

Exact source/program/command/terminal/capture pins and compact receipts are under
/tmp/wasm-fist-0124-review. Reproduce with an external production build:

    FIST_REWRITE_BUILD_ROOT=/tmp/wasm-fist-turret-reproduction-build bash tools/build.sh all
    python3 tools/check_style.py --build-dir /tmp/wasm-fist-turret-reproduction-build/native
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tests/test_manual_turret.py --target all --originals --build-root /tmp/wasm-fist-turret-reproduction-build

For memory checks, configure a separate /tmp RelWithDebInfo build with
-fsanitize=address,undefined -fno-omit-frame-pointer and sanitizer link flags,
keeping production fast-math. Build fist_manual_turret_probe sequentially and
run the same required corpus with --target native --originals --native-probe
pointing to the instrumented program, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
and UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1.

## Next

Recover complete original selected a57a manual composition, retained selector
and decoded view fields, full camera refresh and shared control/clock ordering.
Then compose existing shared owners in one complete manual transaction. Full
device producers, contacts/target maintenance/engine/class/world scheduling and
first playable battle/PCM/outcomes remain required under 0121/0041/0065.

## Accept

Met: complete curved helper/caller control, original domains, retained shared
selector, runtime-target and failure/write-footprint contracts pass on both
targets. Exact source/program/terminal/capture pins, strict complete style,
production regression, fast-math memory and actual scenes pass. Publish only
the exact six C/probe/build files. Complete a57a/device/class/world scheduling,
battle/PCM/outcomes and full-game acceptance remain open; final WASM streak is 0.
