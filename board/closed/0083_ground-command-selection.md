Type: Work item
Title: Own and consume the complete ground command mode selector
Depends: 0082

## Contract

Implement the complete ab82 -> f69:b4ef command-selection return using canonical installed
PINF and world RNG. Own saved mode +43, maneuver +45 and target-reference word +97, retained
across ground-class initialization. Preserve the sole +42 counter owner. Recover all priority
branches, chance boundaries and conditional extra random consumption; no private random stream.
Reject malformed used selectors transactionally, without clamping unused descriptor words.
This complete nested callback is a prerequisite within 0081; it does not replace the full
heading/command phase, target discovery, navigation, roster work or diagnostic presentation.

## Evidence

Frozen image SHA256 d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5.
ab82 is the actual far call followed by near RET. Complete raw 1ab7f..1ac09 selects +43;
raw b26a calls RNG 0291 and RETF, not a target-distance query. Priority and scalar reads
are recorded in docs/ground-command-selection.md. Original UI tails 620c/621c and 626f/6282
cycle descriptor words +0/+2 through four values, independently of corpus values.

All 47 saved inputs contain 960 ground snapshots; 14 have nonzero target references.
Selection tests presence only. Reference resolution and target discovery require subsequent
recovery; neither raw saved words nor their alignment establishes a physical-slot identity.

## Next

Accepted below. Continue active 0081 with complete remaining goal/bearing/navigation/target/roster
callbacks and the parent heading/admission/presentation contract. Do not wire a partial parent
bank merely because this nested selector is complete. Keep the canonical order/RNG owners.

## Accept

Both production targets agree with complete original 251-byte actor and shared RNG returns
for valid admitted fixtures. Used malformed descriptor words, invalid inputs and worlds fail
without mutations. Complete real TRAIN1 installed orders and other 84 payloads survive each
selection; all original files remain unchanged. Full builds/style/memory gates pass.
0081 and the first playable battle remain open; no partial parent-phase dispatch is installed.

## Implementation

`src/sim/ground_command.c` implements the complete nested selector with staged actor/RNG
publication. Earlier priorities preserve the damage bit and do not consume extra randomness.
A reached damage check clears exactly 20h; target presence conditionally consumes one further
canonical RNG value. Used malformed descriptor words fail without provisional mutations;
unused full-width words remain retained. `fist_vehicle_state.command` owns mode, maneuver and
the saved reference value. No guest pointer is dereferenced or guessed as a physical identity.

Common vehicle observations expose all three fields, preserving prior expectations. The bulk
probe overwrites/releases source bytes before selection and compares the entire world outside
the allowed mode/control/RNG changes. Explicit canonical probe mode installs complete orders
and observes the whole world after every command, including physical registry orphans.
Unloaded worlds and malformed metadata fail atomically. Probe-only byte capture/comparison
now shares one common helper; production simulation contains no representation comparison.

## Verified acceptance — 2026-10-07

- Required `test_ground_command.py --target all --originals --oracle` passes all nine groups,
  zero skips in 83.063 seconds: 162676 fixtures / 572 admitted-input rejections / all 960 saved
  ground states per target, including all 47 pinned files and their 14 nonzero target words.
  Full control-word, target-reference and selector-byte domains, every low roll byte, high-word
  independence, equality bounds, all four RNG streams and ordered priority interactions agree
  with complete original 251-byte/RNG returns. Unrelated original DGROUP is preserved.
- Complete original UI mutation tails pass 128 platoon/word/value/direction boundaries, proving
  four choices independently of shipped corpus values. Invalid worlds additionally cover 322
  loaded-flag/component/type/metadata boundaries on each probe invocation, plus null, unused
  and invalid slot calls. Invalid platoon/RNG domains and partially cleared damage-bit failures
  preserve every world byte. Malformed batches publish no partial output.
- Sixty-three complete canonical boundaries per target cover the real 85-object TRAIN1 world
  and an eight-platoon constructed world with seven physical registry orphans. Complete orders,
  all unrelated payloads, registry/roster and RNG remain required at every selection. Real
  original installation/load/selection returns agree; no live class callback is replaced.
- Required ASan/UBSan/LSan command gate passes the same nine groups / 162676 fixtures / 63 world
  boundaries, zero skips in 79.558 seconds with production fast-math. Canonical sanitizer gate
  passes seven groups / 123 fixtures / 924 timed boundaries / 907 objects / 60 rejections,
  zero skips in 12.040 seconds. No memory diagnostics or incomplete runs are accepted.
- Ground class initialization still retains these saved fields. The full native original
  start gate passes seven groups, zero skips in 109.483 seconds. A scoped required WASM corpus
  gate passes all 47 files / 960 original ground initializations in 6.096 seconds. The expanded
  shared flag fixture proves every saved mode/maneuver byte on all four classes, with nonzero
  full-width target words, against complete original initialization on both targets in 2.144
  seconds; this scoped group does not claim a second full start-suite run.
- Required canonical driving original gate remains green on both targets: seven groups,
  zero skips / 123 fixtures / 924 complete timed boundaries / 907 objects / 60 rejections.
  Native takes 15.939 seconds, WASM 23.663 seconds. Complete first-difference/input regression
  remains green: three original-only groups / all 47 files / 376 read/seek returns / 109040 bytes,
  zero skips in 0.511 seconds. Full 0081 acceptance remains unproved.
- Final `CTEST_PARALLEL_LEVEL=4 bash tools/rewrite/build.sh all` exits zero: 32 native CTests
  (55.26 seconds), 30 WASM Python groups and both renderer/pixel gates. The subsequently expanded
  retention fixture and additional orphan-world group pass separately as recorded above.
  Both Ninja trees are current. LLVM 19.1.7 format/tidy passes all 76 owned C translation units;
  no warning, analyzer, formatter or compiler policy was weakened.
- Actual isolated TRAIN1 SDL and Chromium scene/device gates pass complete frames, held inputs,
  turret/weapon/reload/cycle, pause, focus and shutdown. Chromium checks every canvas pixel
  against the complete C frame, worker isolation and failed-start cleanup. Selected physical
  actor remains 151. Four reviewed before/after PNGs show upright tank/terrain/HUD and independent
  browser turret rotation. These are current control/render regressions, not complete battle,
  PCM or mission acceptance.

Initial tidy findings were corrected with named recovered modes/flags, a request value instead
of easily swapped arguments, common exact-byte probe helpers and separated order installation.
A malformed-order probe load originally leaked 307 bytes in two allocations after successful
unit decoding. A temporary negative control without cleanup reaches that exact LeakSanitizer
failure; the final probe releases decoded units and all four malformed/missing order cases pass
without diagnostics. A preliminary full build was intentionally stopped before acceptance to
include that cleanup fix; only the final terminal-zero build is accepted.

Commit/push the bounded success and retain compact `/tmp/wasm-fist-0083-review/summary.json` with
commands, scope, source/binary/log/image hashes and terminal statuses. Remove owned raw logs,
captures, isolated assets, negative-control executable/source and sanitizer build afterwards.
Originals, frozen generated/reference files and dependencies are unchanged. Active 0081,
0041/0065 and remaining full rewrite requirements stay open; complete WASM streak remains zero.
