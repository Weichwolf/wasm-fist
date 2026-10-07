Type: Work item
Title: Retained Apache/Hind death updates and emitter lifetime repair
Depends: 0071, 0072

## Contract

Consume actual retained type-5/6 behavior-12 damage successors through complete 9e2b/a03f
updates: ordered RNG, periodic terrain/altitude-byte servo, rotor phase, signed speed/motion
heading servos, word heading turn, countdown-gated spin/ground destruction, exact effects,
release, intermittent smoke and final planar motion. Reuse terrain, rotation, pool, RNG,
effect and smoke owners. Rename newly proved speed/altitude fields and restore every reached
field without retaining snapshots. Living aircraft AI/initialization, other behaviors, live
type-26 firing, selected-player/world/input/render/audio/AI/outcomes remain required.

## Evidence

Original e454 update table maps 5/6 to 9e2b. CS byte-indexed word table 9f0f maps behavior 12
to a03f. Entry always draws RNG before phase-gated terrain service e1d1/op54. Only altitude
byte +0d changes; +32 is the desired height offset and +18 the wrapped ground difference.
+1b/+1d are signed current/desired speed, +2e desired heading, +30 motion-heading byte,
+23 decrementing word behavior countdown, +1a rotor phase (6d14 &7). a03f spins desired
heading by 05b0 while airborne; grounded callback creates 9c65, requests sound 9 and releases.
The enclosing method still attempts damage smoke and final motion after release.

Untouched reaching execution proves a lifetime defect: phase-zero grounded callback releases
the aircraft, then successful low-priority 19caa can allocate its same physical slot. Constructor
1b201..1b219 clears XYZ before 19cdc..19cf6 reads the released emitter. Smoke then appears at
XYZ=(0,0,768) instead of the crash site. Repair the cause by retaining the emitter pose before
admission. Verify original failure independently; preserve every other constructor/RNG/pool
field and prove corrected C smoke at the captured position. Same-slot replacement also makes
the final original planar tail read constructor-zero speed/motion heading, not retired fields.

## Verified delivery

Verified on 2026-10-07. Shared `sim/aircraft_death` supplies complete 9e2b for behavior 12,
including a03f, actual periodic altitude-byte transfer, independent rotor phase, signed speed/
motion servos, word turn/countdown/spin, ground destruction, post-release emission and motion.
Newly proved `target_speed` and `altitude_offset` replace provisional animation names; actor
restoration owns all reached fields. `combat_probe_io` supplies one complete typed observation
owner for damage/death/destruction. The independent effect golden arithmetic also has one
shared owner across flight, damage and aircraft verification. No presentation/device flow changes.

The smoke creator snapshots the emission pose before admission. This repairs the demonstrated
original same-slot lifetime failure without changing other constructor fields, RNG, metadata
or motion semantics. The original machine remains untouched: complete failed constructors
are independently asserted before correcting solely their XYZ observations for the C repair.
Low-word-only rise and corrected birth low word ffff have reaching regressions. Same-slot
replacement's zero-motion tail and nonalias post-release movement both pass. Callers discard
retired owners before installing returned objects; these scripts do not implement the live pass.
See docs/aircraft-death.md for actual instructions, field widths, configured service/device
boundaries, observation correction, ownership and reproducible commands.

- `bash tools/build.sh all`: all 23 native CTest gates and complete WASM gates pass;
  native CTest elapsed 150.82 seconds. Default gates explicitly omit separately requested
  original corpus/oracle coverage. All following required gates have zero skips.
- `python3 tools/check_style.py`: strict LLVM 19.1.7 format/tidy passes all 64 C units.
  Probe include/arithmetic diagnostics were fixed; no rule or warning was disabled. Atomic
  preservation checks capture bytes from the same object rather than copied struct padding.
- Required `test_aircraft_death.py --target native --originals --oracle`: all six groups pass
  in 731.731 seconds. WASM: all six groups pass in 741.424 seconds. Each covers 59,488 fixtures,
  67,466 class updates, 124,648 smoke states, 15,565 effect states and 2,102 class releases.
  Each actual original run independently proves 129 lost-emitter constructors. Full 1500-tick
  death, 700-tick M1 hit/death/cleanup, clock wrap and both alias/nonalias reuse sequences pass.
- Required aircraft corpus: all 47 pinned missions contain no saved 5/6 records. Constructed
  critical successors place both types at all 960 saved ground positions with all complete
  occupancy contexts, totaling 1,920 cases. Their declared varied 4x4 field is a fixture,
  not the complete installed mission terrain. They execute real M1 damage, impact and release
  before the aircraft stages. Saved and constructed coverage are explicitly distinguished.
- Sequentially compiled production-flags ASan/UBSan/LSan aircraft probe: all six groups,
  complete corpus and same totals pass in 46.813 seconds. Shared damage memory regression:
  all nine groups and full corpus pass in 8.950 seconds, 14,510 fixtures/17,218 damage/14,510
  impact/24 flight/1,920 effect observations per target. Both have zero skips.
- Full both-target original damage regression: all nine groups, the same 14,510/17,218/14,510/
  24/1,920 totals, 1,085 current targets and all 47 occupancy/roster contexts pass in 300.929
  seconds; zero skips. Added observations preserve complete pair fields and type-26/type-27
  emission counters through damage and unrelated payload/source/orphan checks.
- Full both-target original destruction regression: all six groups pass in 187.079 seconds,
  9,135 fixtures/9,166 parent updates/14,701 complete smoke states per target. All 317 unchanged
  saved targets plus 796 constructed critical type-26 successors and 47 complete occupancy
  contexts remain covered; zero skips.
- Full both-target original flight/effect regression: all seven groups pass in 299.700 seconds,
  37,947 fixtures/41,925 complete updates per target, all 179 M1 positions/47 worlds; zero skips.
  This independently regresses the shared height service and effect golden arithmetic. An
  initial new aircraft golden used wrong callback/frame/final-release expectations; actual
  original execution and the existing proved effect owner corrected that golden. Production
  effect code and its acceptance assertions were not weakened.

Complete modeled fields, new 55-byte effect/smoke constructors, unrelated payload preservation,
full pool/registry/RNG state, malformed/null/stale/unsupported-owner errors and source-release
independence pass. Other aircraft behaviors remain explicit unsupported methods, not no-ops.
No generated engine is linked; frozen reference and ignored originals remain pristine.
Commit/push this verified bounded delivery and clean exact owned logs/binaries afterward,
retaining compact hashes/results under /tmp/wasm-fist-0073-aircraft-review.

## Next

Continue 0065/0041 toward the first playable mission: install delivered typed world/combat
owners in the actual mutable registry order and recover command eligibility/phase-driven fire.
Rechecked c0e5 first consumes 4712's mission countdown, increments 6cde, runs beb7's scheduled
voice and decrements 969c/969e before c105..c120's live registry traversal. Later-entry
allocations can execute in the same pass. The driving session currently retains neither
complete live payloads nor post-initialization world RNG; recover both owners before integration.
TRAIN1's actual saved classes are 0/2/21/23/26/27. Its 22 type-26 records include 16 active
modes (0:9, 1:2, 2:5) and six destroyed mode-7 records; do not substitute destroyed updates
or no-op methods for their living behavior. Recover living-class,
selected-player, render/audio and objective/outcome consumers as reached. Prioritize a complete
first mission, then preserve every later all-function and independent ten-full-WASM requirement.
This numerical delivery supplies neither the live pass nor a complete playable mission.

## Accept

Complete declared state/phase/servo/countdown/heading/quality/capacity/natural-lifetime sequences
pass on both targets with zero skips in required corpus/original/memory gates. Actual original
execution proves complete modeled fields, unrelated payload preservation, constructor/metadata/
RNG ordering and the reaching emitter-loss defect. The only intentional behavioral repair is
capturing the retired emitter before constructor reuse; do not normalize other differences.
Atomic invalid/stale/unsupported-owner inputs fail. Reference/originals stay pristine; required
build/style/memory gates pass, bounded delivery is committed/pushed and owned /tmp is cleaned.
