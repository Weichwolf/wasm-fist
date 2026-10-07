Type: Work item
Title: Own complete terrain visibility for runtime target discovery
Depends: 0063, 0090

## Contract

Recover and implement the complete protected-mode op-58/8030 terrain visibility return in
shared readable C11. Borrow the existing installed height plane and effective source/target
positions. Preserve wrapped DWORD differences, signed subdivision, original interior samples,
unsigned terrain/altitude comparison and asymmetric range rejection. Reuse the ground sampler
owner; keep numeric scratch local and preserve caller inputs, world state and RNG.
The DOS wrapper owns class-specific aim offsets; full target discovery and 0081 remain open.

## Evidence

Pinned original kernel dispatch word at d0b resolves operation byte offset 58 to 1103, which
calls 8030. Source XYZ are mailbox d2/d6/da; target XYZ are de/e2/e6. Map-loader instructions
8acf/8ad4 install the height detail in both visibility SHLD immediates 8105/8109. These are
not permanently fixed to the image's initial detail of ten bits. Original e1f0/e21c computes
class-specific aim heights before this device request, separately from the terrain service.

## Next

Prove complete original returns across range/subdivision/coordinate/altitude edges, terrain
occlusion and all original maps/saved mission positions. Run native/WASM production tests,
full builds/style and sanitizers. Continue complete target identity/discovery and command
consumption under 0081 after this prerequisite, without a partial parent callback bank.

## Accept

Both production targets pass required complete original comparisons, corpus checks, immutable
inputs and atomic invalid-input tests without skips. Full builds, strict formatting/tidy and
memory checks pass. Record exact evidence and limitations, commit/push and clean obsolete
owned /tmp artifacts. No playable battle, full command/AI/audio or final ten-run claim follows.

## Verified acceptance

Required native `test_visibility.py --target native --originals --oracle` passes all six
groups, zero skips, in 168.725 seconds. Required WASM `--target wasm --originals` passes the
same six groups, zero skips, in 169.183 seconds. Both targets compare 173917 complete returns:
67584 height/unsigned-fraction cases, 2457 displacement/subdivision/wrap cases, 1144 actual
occlusion/interior/endpoint cases, 32768 deterministic full-DWORD cases and 69964 saved-pose
corpus queries. All 47 pinned scenarios contribute 17491 opposing-side candidate pairs;
all eight original numerical height maps are installed at four sizes, 512 through 4096.
The independent complete original service validates the identical expected fixture stream;
unrelated kernel/mailbox/DGROUP/full-stack memory is unchanged and height memory is read-only.
Saved XYZ are declared effective inputs, not a prepared aim-offset/target-selection claim.

ASan/UBSan/LeakSanitizer passes the same six required groups, zero skips and complete 173917
query domain in 134.662 seconds with production fast-math flags. The probe owns its data and
poisons/frees the decoded source before use. Null/invalid APIs preserve the output; incomplete,
trailing, count, missing-file and argument failures emit no partial output. Source/height
validation and the sampler are reused; no class/world/RNG state or callback bank is mutated.

`bash tools/build.sh all` exits zero: all 37 native CTests in 525.61 seconds, all 35 WASM
Python suites / 266 cases, and both Node pixel probes. Standard suites have 43 expected optional
original skips; the separate required visibility/corpus gates above have none. Full strict
LLVM 19.1.7 formatting/tidy passes every owned header and all 82 C translation units, without
weakening checks. Existing motion, contact, world, damage and canonical control suites pass.

Current isolated TRAIN1 native SDL and Chromium gates pass complete opaque changing/stable
frames, held steering/turret input, weapon choice/reload, pause/focus and shutdown; browser
startup failure is also checked. Both resulting paused frames were manually reviewed: terrain,
assembled tank sprite and ammunition/weapon/pause HUD remain present and coherent. This helper
is not yet wired into discovery; these are existing presentation/control regressions, not a
new full-battle claim. Original 419 files, immutable reference/ghidra and softgl pin are intact.

An initial strict run rejected the probe's memcpy/memset and unnamed size. Replaced them with
bounded loops/named constants, stopped the early full build (exit 143), and reran the complete
gate. The final oracle checks the entire stack except its two actual return words. Height-byte
fixtures use one complete 16x16 plane/batch, retaining every combination and avoiding repeated
process launches. Final required gates above use those final sources; initial runs are not
substituted for final acceptance. Reproduction is in docs/terrain-visibility.md. Retain the
compact `/tmp/wasm-fist-0091-review/receipt.json` and two reviewed frames; remove obsolete logs,
isolated assets and terminal sanitizer builds after verification. Continue full runtime aim
offsets/identities/discovery and command consumption under 0081. The rewrite goal is active;
first playable battle/PCM/outcomes and independent final WASM streak remain unproved, streak zero.
