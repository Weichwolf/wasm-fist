Type: Work item
Title: M1 primary hits, ground-vehicle damage and immediate destruction transition
Depends: 0069, 0060

## Contract

Consume the pending M1 primary type-8/profile-0/parameter-5 hit against the four registered ground
vehicle classes. Preserve original aspect/source-side word-width scaling, ordered random
reactions and selected-player damage feedback. Implement critical damage's immediate census,
normal-priority effects/wreck allocation, roster replacement and in-place type-19 conversion,
then the actual four-update retirement. Keep the projectile allocated/unchanged until its
separate impact continuation, so damage effects affect allocation capacity first. Reuse the
existing pool/explosion/random owners. Expose actual voice/sound/display and selected-player
loss/takeover requests without claiming implemented audio or UI consumers. Other projectile
profiles, target classes, live world/input and complete mission remain under 0065/0041.

## Evidence

Actual e550 ground actions c336 call f69:bd0c (physical 1b39c). This method's popped BX is the
class aspect table, so subsequent reaction admissions read its bytes 2/3/4; they do not read the
weapon record's trailing bytes. a02d returns AX with its fire-flag AL and zero AH when triggered,
which the following admission consumes. Fatal byte carry is significant even when damage wraps
below 100. a93e creates two type-4 effects and a normal-priority type-23 wreck before b2d3 changes
the target to type 19 without releasing its arena/binding. Actual type-19 c0ba decrements its
secondary byte and releases on zero. See original bbb7/c31e/1b39c/a93e/1b2d3/c0ba methods and
1a02d/1a064/1a080, 1610e census, 160e5/160f9 roster updates, 1b10c wreck initialization and
1697e four-platoon count publication. Recheck frozen comments against these methods.

## Verified

Verified on 2026-10-06, LLVM 19.1.7 and pinned Unicorn 2.1.4:

- `bash tools/rewrite/build.sh all`: all 20 native CTest gates and complete WASM build gates pass.
  The damage gate is required on both targets. Routine gates explicitly omit original corpus;
  the separate required original gates below have no skips.
- `python3 tools/rewrite/check_style.py`: strict format/tidy pass for all 54 owned C units.
- `test_vehicle_damage.py --originals --oracle`: all nine groups pass on both production targets
  in 315.256 seconds, without skips. Complete coverage is 22,937 fixtures, 25,467 damage,
  22,937 impact and 2,608 retirement updates per target, plus the independently executed reaching
  launch/three-flight-tick/damage/impact/four-retirement/complete-effect-cleanup pipeline.
- A subsequent complete critical/carry/scale/feedback/census group passes both targets against
  the original in 7.425 seconds: 404 fixtures/404 damage/404 impact/1,048 retirement updates,
  including 32 additional fatal cases across every class × projectile/victim side × selection.
  Distinct current coverage totals **22,969 fixtures, 25,499 damage, 22,969 impact and 2,736
  retirement updates per target**, plus the reaching pipeline. All 960 current ground targets
  over all 47 complete original arena/registry occupancy/roster contexts are included; no
  orphan targets occur. Complete outputs, unrelated original records and producer requests match.
- `test_projectile_flight.py --originals --oracle`: all seven groups pass both targets in
  277.461 seconds without skips after the shared effect creation/binding-owner refactor.
  37,947 fixtures/41,925 complete updates include all 179 M1 launch positions over 47 worlds.
- Production-flags ASan/UBSan/leak detection: all nine current damage groups and required corpus
  pass in 18.222 seconds, without skips, with those same distinct totals and reaching pipeline.
  The preceding retirement identity check was proved insufficient: the reached critical M1
  pipeline fails new wrong-vehicle-owner assertions with that preceding check, while the fixed
  sanitizer pipeline passes. Current tests also reject stale/double retirement atomically.

See `docs/vehicle-damage.md` for original addresses, tables, exact ownership and reproduction.
No rendered frame changes; prior driving visual review remains the baseline, not evidence of
visible combat or audible PCM. Selected fatal original execution explicitly pauses before
a97a and resumes at a990; complete selected-player UI/camera/takeover remains required. Later
wreck updates, other target actions/profiles, world scheduling, device firing and audio consumers
remain open. Parent 0065/0041 acceptance is preserved. Owned temporary logs/transcripts,
fixtures, preceding-check probe and sanitizer binaries are cleaned after commit/push; a compact
summary remains under `/tmp/wasm-fist-0070-damage-review`.

## Next

Continue 0065 with remaining reached M1 target actions, selected-player/wreck consumers and typed
live-world installation/scheduling before command eligibility, fire input, battle drawing and
audible events. Preserve same-pass allocation/update order and complete mission occupancy;
the first playable mission still requires AI, objectives and outcome behavior under 0041.

## Accept

Both targets pass complete declared damage and immediate destruction/retirement coverage,
all required original corpus cases and reaching damage→impact continuation sequences, without
required skips or missing output. Independent untouched original methods support arithmetic,
random consumption, payload writes, pool identity, roster/census and producer event requests.
Explicit device/player-UI boundaries remain documented. Missing/stale/wrong-profile/type/owner
inputs fail atomically. Strict LLVM and both production build gates plus production-flags
ASan/UBSan/leak checks pass. Preserve parent acceptance and original files; clean owned /tmp logs.
