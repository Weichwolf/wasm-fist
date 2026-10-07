Type: Work item
Title: Remaining collision-reachable M1 primary target damage
Depends: 0070, 0068

## Contract

Consume M1 type-8/profile-0/parameter-5 hits against remaining collision-reachable types 5/6,
23, 26 and 27 in readable shared C. Restore typed short actor fields from owned definitions.
Reuse pool, random, combat census and explosion owners, sharing source validation and word-width
damage arithmetic with 0070. Preserve ordered reaction/census/effect/release behavior, including
the real type-23 no-op. Keep the source unchanged/allocated until impact continuation. Expose
actual voice/sound/display producer requests. Subsequent actor/debris/type-26 target updates, live
world scheduling, command input, drawing, audible PCM and complete mission remain under 0065/0041.

## Evidence

e518 collision dispatch can reach ground 0..3 and 5/6/23/26/27. Types 16 and 21 have damage methods
but do not admit a type-8 unit collision; do not invent those reaching paths. e550 dispatches
5/6 to a0c8, 23 to c335, 26 to bd09 and 27 to b396, followed by c31e/60f4 display refresh.
a0c8 ignores carry in its byte damage comparison, skips behavior 12 without RNG, requests
unselected friendly reaction voice 38 and a097, then on critical damage updates 79a0/799c and
uses an additional random byte to choose immediate effect/release or behavior-12 death animation.
bd09 skips modes with bit 4, uses the subtype damage table, retains its binding after setting
deleted flags/subtype/parameter/scale and creates the subtype's actual authored effect. b396
skips nonzero modes, honors damage carry or threshold 80, retains binding after its exact debris
transition and allocates the 9c5d effect. b274 and b396 select source scale from e3b2's projectile
flag. Actual 9c65/9c6d/9c2d templates differ in extent/period, not just their names.

Byte-indexed render table e48c assigns 5/6 to APACHE/HIND, 26 to TARGETS and 27 to ARTILL.
The former collision type-26 helicopter label was wrong; names and documentation are corrected,
and the new oracle pins all four assignments. Original bc46 compares the type-26 destruction
word unsigned; the typed state preserves that width/sign contract. No class identity or subtype
behavior is inferred from an unverified old label. See docs/other-damage.md for exact ownership,
instruction evidence and reproduction commands.

## Verified

Verified on 2026-10-06 with LLVM 19.1.7 and pinned Unicorn 2.1.4:

- Final `bash tools/build.sh all`: all 21 native CTest and complete WASM build gates
  pass, including the new required damage probe. Routine build gates explicitly omit original
  corpus; every required complete-corpus gate below has no skips.
- Final `python3 tools/check_style.py`: strict format/tidy passes all 57 owned C units.
  The final header-comment-only clarification also passes the direct format check.
- Final `test_other_damage.py --originals --oracle`: all nine groups pass both production
  targets in 570.618 seconds, without skips. Per target: 14,510 fixtures, 17,218 complete damage
  observations, 14,510 impact continuations, 24 reaching flight updates and 1,920 full effect-clock
  ticks. Eight reaching cases execute actual collision→damage→impact→240-tick natural effect
  cleanup from the declared already-launched flight boundary. Complete source/unrelated original
  record bytes, modeled actor fields, effects, metadata/census/roster/RNG and producer requests
  match. Every critical C transition also checks the retained no-op or released stale-owner path.
- The required corpus includes all 1,085 current remaining targets across all 47 complete
  occupancy/roster contexts: 802 type-26 TARGETS, 236 type-27 ARTILL and 47 wrecks; zero orphan
  targets. Saved snapshots contain no type 5/6 APACHE/HIND actors, so those have constructed
  plus independent-original method/reaching coverage, not falsely claimed snapshot coverage.
- Complete `test_vehicle_damage.py --originals --oracle` regression passes both targets in
  411.250 seconds: 22,969 fixtures, 25,499 damage, 22,969 impact and 2,736 retirement updates
  per target plus the reaching complete-launch pipeline; all 960 ground targets/47 contexts.
  Complete `test_projectile_flight.py --originals --oracle` regression passes both targets
  in 373.625 seconds: 37,947 fixtures/41,925 updates, all 179 M1 launch positions/47 worlds.
  These full regressions precede the subsequent internal-name/probe unsigned-field correction;
  final production builds and final memory gates consume the corrected C. Their shared numeric
  ground/flight rules remain unchanged.
- Corrected complete `test_collision.py --originals --oracle` passes both targets in 337.899
  seconds, all seven groups with no skips, including all 4,213 snapshot source queries and
  179 M1 launch-position queries over all 47 worlds. Type-26 names change, not height/query rules.
- Final production-flags ASan/UBSan/leak probes pass both nine-group damage suites with required
  originals and no skips: other targets in 15.671 seconds and ground targets in 25.408 seconds,
  with the same complete respective fixture/transition totals and reaching sequences above.

Durations are elapsed verification times with concurrent read-only gates, not runtime performance
claims. Production/sanitizer compilation remains sequential with one writer. Frozen code is not
linked into the rewrite; originals remain ignored/read-only and rehashed. No presentation code
changes, so the reviewed driving scene stays the visual baseline. Later death/debris/target/wreck
class updates, selected-player consumers, live world scheduling, fire eligibility/input, battle
rendering, audible PCM, AI and mission outcomes remain required under 0065/0041. This closes the
stated damage kernel, not a playable mission or final full-WASM acceptance. Commit/push this
bounded success; remove owned scratch logs/inputs/sanitizer binaries and retain the compact
summary under `/tmp/wasm-fist-0071-other-damage-review`.

## Next

Continue 0065 with consuming destruction/class updates and selected-player/wreck flows, then
install the delivered launch/flight/damage/effect owners into actual live-world scheduling.
Preserve complete mission occupancy, damage before impact continuation and same-pass allocation/
update order before fire input, visible battle and audible events. Parent 0041 still requires
AI, objectives and a resolved playable mission; final ten complete WASM runs remain unproved.

## Accept

Both targets pass complete declared remaining-target coverage and original corpus contexts
without required skips, missing output or unequal lengths. Complete original-method execution
supports all modeled actor fields, untouched unrelated payloads, metadata/census/random and
producer requests at explicit audio boundaries. Source remains unchanged through damage;
capacity and immediate actor release affect the following impact correctly. Invalid/stale/type/
profile/owner inputs fail atomically. Required builds/style and memory checks pass, original
files/reference remain pristine, and the verified bounded change is committed and pushed.
