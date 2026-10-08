Type: Build
Title: Production renderer numerics and stable WASM shared heap
Depends: 0040, 0050, 0064

## Contract

Remove the existing infinity/fast-math and pthread/growing-memory diagnostics by
repairing their causes. Preserve the requested production compiler flags and all
existing test expectations. Keep dependency and presentation-policy integration
separate from the unaccepted ground-command and target-motion C work.

Use the existing wasm32 maximum capacity of 2 GiB for a fixed shared heap in the
three presentation targets. Preserve allocation failure as a returned null pointer.
Keep one link-options owner and verify actual default shared memory and matching
typed-array buffers; a test-supplied memory override would not prove the policy.

## Evidence

The pinned softgl base is 7963be1d5b5e1bebbe97ece2c655228c8bc0a838. The separately
published repair branch refs/heads/fist/hz-fast-math contains HZ commit
b79353d69fe701eaac2bca3ac6b4146777d0b49c and DOT3/clamp commit
f93dbe9e744b48fa01d7f8eb8a44d2ff510b2d18. It does not import the newer upstream
scene pipeline and its independently reproduced failures.

HZ maxima/NaN invalidation/recovery/classification pass 24,400 production cases
per target. The complete repaired native renderer builds without warnings and
passes all 737 native tests against Mesa and the internal contracts. Shared DOT3
evaluation passes 300,000 exact general/specialized comparisons on native SSE4.1
and WASM SIMD128. Stored-bit clamps pass all 18,087,936 existing lanes on each
production target, including every NaN payload, both zero signs and subnormals.
The old native fast-math clamp fails the unchanged oracle; no flags or checks
are weakened. A common call boundary prevents caller-dependent dot reduction;
integer masks retain clamp bits that floating comparisons lose under fast-math.

Of 237 complete native comparison streams, 236 remain byte-identical. The normalmap
scene changes nine color bytes by one. Its actual image and the DOT3 baseline image
were inspected. The existing Mesa comparisons still pass without changed tolerances.
Two paired 600-frame normalmap timing samples average 2.41 ms before and 2.55 ms
after the repair, approximately 6% slower in this limited scene. This is a numerical
correctness tradeoff, not a performance improvement or a complete-game measurement.

Receipts and reproduction fixtures are under /tmp/wasm-fist-0115-review:
dot3-repair-development.json, dot3-native-visual-comparison.json and
dot3-native-performance.json. Previous completed HZ presentation outputs were
retired after preserving all 1,185 file hashes and the changed-scene before image,
reclaiming 928,297,953 bytes; hz-native-output-cleanup.json records the scope and
regeneration command. Current libraries, executables, sources and evidence remain.

The owning-project candidate starts at master commit
622da081cad8e8ddc91541ac0eb385f5019d4aa5 in
/tmp/wasm-fist-renderer-integration-source, with builds in
/tmp/wasm-fist-renderer-integration-build. Only CMakeLists.txt, tests/check_wasm.cjs,
tools/build.sh and the dependency pin differ. The actual default-heap guard accepts
the isolated fixed-capacity renderer and rejects the previous growing renderer;
no memory is injected by the test. Strict LLVM 19.1 style passes all 94 owned C
translation units under session9069. Full owning-project production regression
is live under session54164 and is not yet acceptance. Exact source pins and the
integration patch are in renderer-integration-development.json and
renderer-integration.patch. Original assets remain ignored and read-only.

The candidate's actual TRAIN1 SDL gate is terminal0 under session91379: complete
frames, pause publication, held driving/hull/turret inputs, weapon controls, focus
loss and clean shutdown pass. All 17 captures are hashed and the final scene was
visually inspected; renderer-integration-native-scene.json retains the complete
result and program/checker pins. All 419 original files still match their pinned
hashes and remain read-only. The full session54164 regression and current-candidate
browser/corpus gates remain open.

The same candidate rejects all 12 independently executed native failed starts:
missing arguments, invalid mode, missing/empty/truncated scenario, missing assets,
truncated model palette and five malformed height values. Each exits with status 1,
empty stdout and the expected usage diagnostic before scene publication. The fixture
and exact program/source pins are retained in native_start_failures.py and
renderer-integration-native-start-failures.json under the review directory.

All ten native terrain-scene groups with --originals pass without skips under
session88523, terminal0. renderer-integration-native-terrain.json retains the command,
program pin and complete compact result; the redundant completed raw log is retired.
The production regression has passed its first 14 native tests and remains live;
this partial count does not accept the full integration or the WASM target.

After the repaired softgl suite completed all 737 tests and its image comparison,
the 1,185 regenerable PPM/RGBA outputs were retired, reclaiming another 928,297,953
bytes. dot3-native-output-cleanup.json preserves every file hash and the reproduction
command. The changed-scene before/after images, source, libraries and executables
remain available. Active regression logs and inputs are retained.

## Next

Poll session54164 to terminal completion without mutating its inputs. Require both
targets and absence of compiler/link warnings. Verify the candidate's actual native
and browser triangle/terrain/TRAIN1 scenes, input/shutdown/failure behavior and the
all-47/four-detail presentation comparison. Check unchanged reference refs and
original assets before publishing only the verified integration patch to master.

## Accept

The exact integration source passes python3 tools/check_style.py and
bash tools/build.sh all with LLVM 19.1.x, production fast-math and no compiler/link
warnings. The default shared-heap assertion rejects the old configuration and
passes the new one. Real native/browser scenes and complete all-47/four-detail
comparisons pass; dependency fixtures prove conservative HZ and consistent DOT3/
clamp behavior on both targets. Publish the bounded dependency/presentation change
separately from simulation feature work, retain compact receipts and clean obsolete
owned artifacts. This gate does not accept full 0081, living battle, PCM or the
independent ten-run complete-game WASM gate.
