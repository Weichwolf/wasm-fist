Type: Build
Title: Production renderer numerics and stable WASM shared heap
Depends: 0040, 0050, 0064

## Contract

Repair the infinity/fast-math and pthread/growing-memory diagnostics without
weakening production flags or existing expectations. Keep this dependency and
presentation-policy change separate from unaccepted ground-command/target-motion C.
Use the previous wasm32 maximum of 2 GiB for a fixed shared heap in the three
presentation targets, with null allocation failure and one link-options owner.
Verify the actual default shared memory and matching typed-array buffers.

## Evidence

The old softgl pin is 7963be1d5b5e1bebbe97ece2c655228c8bc0a838. The published
refs/heads/fist/hz-fast-math repair contains HZ commit
b79353d69fe701eaac2bca3ac6b4146777d0b49c and DOT3/clamp commit
f93dbe9e744b48fa01d7f8eb8a44d2ff510b2d18, now pinned by this change. The newer
upstream scene pipeline has separately reproduced failures and is not imported.

HZ maxima/NaN invalidation/recovery/classification pass 24,400 production cases
per target. Shared DOT3 evaluation passes 300,000 exact general/specialized
comparisons on native SSE4.1 and WASM SIMD128. Stored-bit clamps pass all
18,087,936 existing lanes per target, including every NaN payload, both zero signs
and subnormals. The old fast-math clamp fails the unchanged oracle. A common call
boundary prevents caller-dependent dot reduction; integer masks preserve clamp
bits. All 737 native renderer tests pass against Mesa and internal contracts,
with a fresh warning-free dependency build and unchanged tolerances.

Of 237 complete native image streams, 236 remain byte-identical. The normalmap
scene changes nine bytes by one; actual before/after images were inspected. Two
paired 600-frame samples average 2.41 ms before and 2.55 ms after, approximately
6% slower in this limited scene. This is a correctness tradeoff, not a whole-game
performance measurement. Receipts in /tmp/wasm-fist-0115-review are
dot3-repair-development.json, dot3-native-visual-comparison.json and
dot3-native-performance.json.

Only CMakeLists.txt, tests/check_wasm.cjs, tools/build.sh and the dependency pin
change in the integration candidate. One interface owner fixes shared capacity
for renderer, terrain and driving presentation. The real default heap assertion
passes with 2,147,483,648 bytes and consistent HEAPU8/HEAP32 buffers; it rejects
the previous growing configuration without injecting memory. A 188-world run
peaks at 257,781,760 resident bytes; reserved capacity is not resident allocation.

The first isolated full regression, session54164, completes with status 0 and
all native/WASM regressions passing, but emits 36 existing RelWithDebInfo DWARF
limited-postlink-optimization warnings. It does not establish clean acceptance.
The accepted candidate translates only the standalone RelWithDebInfo -g token
to -gsource-map for Emscripten, preserving other flags and native debug policy.
The generated map has nonempty mappings to owned and dependency sources. Link
diagnostics fail through -Werror. Source-map mode retains source locations while
allowing Binaryen optimization; it changes debug representation rather than
suppressing a warning. Documented mode:
https://emscripten.org/docs/tools_reference/emcc.html#emcc-gsource-map

The accepted isolated source is /tmp/wasm-fist-renderer-integration-v2-source,
starting from published commit 622da081cad8e8ddc91541ac0eb385f5019d4aa5; builds
are /tmp/wasm-fist-renderer-integration-v2-build. All 503 source pins were rechecked.
Session78978 is terminal0: all 46 native CTest contracts, 44 WASM Python suites
(329 cases) and both actual Node probes pass. Fresh production compilation has
156 native steps and 156 WASM steps (40 renderer smoke plus 116 remaining steps),
with no compiler/link warnings. LLVM is 19.1.7 and Emscripten is 3.1.69; requested
C11/fast-math/warning flags and owned -Werror were checked in compile commands.
Strict LLVM 19.1 style/tidy passes all 94 owned translation units, session60701,
terminal0. Default optional original oracles are skipped by design; the required
terrain-original groups below are separately complete without skips.

All ten terrain-original groups per target pass, sessions30804/36074, terminal0.
Actual native TRAIN1 input/presentation/pause/focus/weapon/shutdown checks pass,
session72560, terminal0, with 17 complete captures. All 12 independently executed
native failed-start cases reject malformed/missing inputs with status 1, empty
stdout and the expected usage diagnostic before publication. Actual isolated
browser triangle, terrain and TRAIN1 gates pass, sessions25339/99145/8130,
terminal0. Held driving/hull/turret inputs, selection/reload/cycle, fifth slot,
pause/focus, shutdown and failed startup reach the shared state and canvas.
The actual candidate native-after, browser-after and browser terrain images were
visually inspected; 25 native/browser captures are retained and hashed.

All 47 missions and four details pass the actual paired WASM presentation run,
session86158, terminal0: 188 complete worlds and 376 complete frames per module,
all 18 state values, opaque full frames, held inputs, destruction and post-destroy
guards. Digest: 849e71ca6422b3f18ae61b4e77e8f709d0d3b5059028041be4d754bdec6e6175.
All 419 original files remain hash-matched, read-only and ignored. The frozen ghidra
commit, immutable reference tag object and peeled reference commit remain unchanged.
All 16 root prototype files were checked before integration; their simulation
changes are excluded from this accepted change.

Compact exact commands, source/program/checker/capture hashes and complete results
are in /tmp/wasm-fist-0115-review:
- renderer-integration-v2-development.json and renderer-integration-v2.patch
- renderer-integration-v2-production-all.json
- renderer-integration-v2-presentation.json and renderer-integration-v2-corpus.json
- renderer-integration-v2-native-start-failures.json
- renderer-integration-v1-production-all.json (superseded warning-bearing run)

Reproduce from the exact accepted tree with ignored originals provisioned:

    FIST_REWRITE_BUILD_ROOT=/tmp/wasm-fist-renderer-integration-v2-build bash tools/build.sh all
    python3 tools/check_style.py --build-dir /tmp/wasm-fist-renderer-integration-v2-build/native
    python3 tests/test_terrain_scene.py --target native --build-root /tmp/wasm-fist-renderer-integration-v2-build --originals
    python3 tests/test_terrain_scene.py --target wasm --build-root /tmp/wasm-fist-renderer-integration-v2-build --originals
    node /tmp/wasm-fist-renderer-integration-v2-corpus.cjs

Installed Valgrind 3.24.0 verifies the actual native headless renderer: 54 allocations,
54 frees, 886,768 allocated bytes, zero remaining blocks/errors/suppressions and
status 0. Instrumented full SDL attempts are not accepted: early exits or the
five-second window discovery deadline stopped them. The latest diagnostic attempt
was terminated during asset/first-frame setup; its leak output is not evidence
of clean shutdown. Normal SDL/browser gates above pass. No production SDL hints,
suppressions or global permissions were changed. perf 6.12.111-1 is installed but
its actual task-clock attempt is denied at perf_event_paranoid=3; PCM 202502-1 is
available at /usr/sbin/pcm but no hardware-counter measurement is accepted.
renderer-integration-v2-tooling.json records this exact limited scope.

Completed HZ and repaired-renderer output cleanup reclaimed 928,297,953 bytes each,
retaining hashes/reproduction commands and changed-scene images in
hz-native-output-cleanup.json and dot3-native-output-cleanup.json. Completed
research/style/smoke logs were retired only after preserving compact results in
reached-m1-and-v2-completed-cleanup.json. Source, fixtures, binaries, meaningful
captures and unresolved profiling evidence remain under /tmp. Eleven completed
integration/presentation/headless-Memcheck logs were retired after preserving
compact results and hashes, reclaiming 88,080 bytes;
renderer-integration-completed-cleanup.json records the exact scope.

## Next

Continue full shared ground-parent/class acceptance under 0081. Resolve complete
SDL/game memory instrumentation and hardware profiling in subsequent reached
steps; the clean headless Memcheck result does not accept those surfaces. Living
battle, PCM and the independent ten complete-game WASM runs remain open.

## Accept

Delivered: the exact bounded integration source passes full native/WASM production
build/regression and strict LLVM 19.1 style without compiler/link warnings. Actual
default shared memory, complete scene/input/failure gates, all-47/four-detail
presentation and both-target dependency contracts pass. No flags, test tolerances,
threading or original/reference protections are weakened.
