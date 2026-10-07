# Canonical mission combat visits

`sim/mission_update` consumes the objects owned by `fist_mission_world`, using the existing
allocation, flight, damage and class-lifetime owners. The pool's physical type tags the payload
union. `world.combat.roster` is the sole physical platoon list, including overwritten registry
orphans; the installer and damage methods share it. Import/reset do not establish combat
factors, selected player, census or complete mission initialization.

A type-8 visit rebuilds all current collision views, including physical orphans, advances the
canonical projectile and applies actual reached damage before allocating its impact effect or
releasing the shell. Every returned explosion, wreck or smoke is installed in its actual slot
before another current registry entry is read. Ground collision adapters borrow canonical XY,
altitude and common/turret heading for that call; they are temporary projections, never a
second retained pose. Original `b65c..b66b` reads heading `+10`, whereas wreck construction uses
hull heading `+26`. A reaching shot with distinct headings exposed and corrected the initial
adapter error: actual hit aspect 15 had incorrectly become 6.

The caller uses `fist_world_next` to read each current binding on arrival. Newly allocated
objects at later entries can run in the same declared span; earlier entries wait. Each visit
stages the complete world on an owned heap allocation and publishes world/result together on
success. Invalid or unsupported visits leave every byte of both owners unchanged. This initial
transaction strategy has not been profiled as a complete battle frame.

The consuming visit dispatches shell 8, explosion 4, drifting smoke 17, muzzle 18, ground
retirement 19, conditional tree 21, persistent wreck 23, destroyed target 26, artillery 27 and
aircraft 5/6 behavior 12. All reuse delivered kernels, including the proved aircraft emitter
lifetime repair. Living ground methods, other aircraft behaviors and living target 26 return
`FIST_MISSION_UNSUPPORTED`. This is an explicit missing class-call boundary, not a successful
empty update. Tree producer inputs, weather, actual class tick and animation phase are supplied
by the caller.

Selected fatal ground damage publishes its actual effects/roster/census and sets
`pending_player_impact`. It returns before impact continuation. Every further visit and world
fire attempt rejects while this handoff is pending. Only after the real player-loss UI/takeover
consumer has completed may the caller invoke `fist_mission_world_resume_impact`. That explicit
acknowledgement performs impact allocation/release once, publishes the effect and clears the
handoff; it neither repeats damage nor implements UI/takeover. The verification driver supplies
an explicit acknowledgement at this open consumer boundary.

## Evidence and required verification

The canonical combat probe releases decoded scenario/unit storage before consuming the world.
It serializes every used physical payload, allocation/registry entry, RNG word, sole roster,
combat factor, selection, census and pending handoff after every visit and command. A separate
Python model reuses the existing independent initialization, fire, collision, damage and
lifetime golden owners. Exact byte lengths and SHA-256 of the complete, unfiltered output
streams must agree. Both streams include every state record. On disagreement, a fresh golden
replay reports the first differing line; small monitored event lines support additional
assertions only after the complete comparison passes. Temporary streams live in `/tmp` and
are automatically deleted, including failing subprocess runs.

Fixtures cover all four ground classes, all sixteen hit aspects, surviving/fatal and selected
loss, all remaining M1 collision targets and target subtypes, terrain gates, shell expiry,
normal/low-priority capacity pressure, dynamic birth order, physical orphan origins and actual
same-slot aircraft replacement. Null/invalid/stale/RNG/continuation API rejection checks capture
complete world/output bytes from the same objects. Malformed request/scenario input must fail
without a partial success trace.

The additional original instruction gate starts at an explicitly published shot/world boundary.
It installs complete fixture records, used maps, counts, registry, roster, RNG and combat state,
then executes untouched actual flight/collision/damage/constructors/impact/release and delivered
non-aircraft lifetime methods. It compares every complete physical raw record, all typed state,
actual metadata counters, RNG and combat state after each reached boundary. Original height and
configured device gates reuse the existing oracles. Selected fatal damage names the existing
`a97a..a990` player UI boundary; that UI is not emulated by this test. Original aircraft death,
including the separately proved repair relation, remains in its required independent regression
gate. Constructor, living method, UI, device or full-game execution is not fabricated.

The required TRAIN1 gate checks the pinned scenario hash and all 85 installed objects. Its
player is physical slot 151, registry entry 32. It selects/takes control, runs 320 declared
reload phases, requests/fires and follows 485 declared class-boundary spans. Height input in
this probe is a constructed 2x2 plane. These spans explicitly expose unsupported living calls;
they are not complete world ticks, real installed TRAIN1 terrain, playable missions or final
WASM acceptance runs. The production scene still uses its separate player-only baseline.

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_mission_combat.py \
  --target native --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_mission_combat.py \
  --target wasm --originals --oracle
```

Required original regressions on each target are `test_primary_fire.py`,
`test_vehicle_damage.py`, `test_other_damage.py`, `test_aircraft_death.py` and
`test_collision.py`, each with `--originals --oracle`; the installer regression is
`test_mission_world.py --originals --oracle`. Zero required skips and terminal success are
mandatory. The default build explicitly skips the additional corpus and instruction groups.

Build memory instrumentation sequentially after any previous compile returns:

```sh
mkdir -p /tmp/wasm-fist-0077-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-sanitize-recover=all \
  -fno-omit-frame-pointer -g -O1 -Isrc -Itools/rewrite src/assets/*.c src/sim/*.c \
  tools/rewrite/probe_io.c tools/rewrite/object_pool_probe_io.c \
  tools/rewrite/vehicle_probe_io.c tools/rewrite/combat_probe_io.c \
  tools/rewrite/mission_probe_io.c tools/rewrite/mission_combat_probe.c -lm \
  -o /tmp/wasm-fist-0077-sanitizer/mission_combat_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_mission_combat.py \
  --target native --originals --oracle \
  --native-probe /tmp/wasm-fist-0077-sanitizer/mission_combat_probe
```

Living class/AI dispatch, installed terrain/contact/take-control, actual fire eligibility,
targeted weapons, battle drawing, PCM, player-loss consumer and mission outcomes remain required
under 0065/0041. This stage changes no presentation, input, audio or persistence code. Complete
playable-mission and final ten-run WASM acceptance remain open.

On 2026-10-07, the required both-target instruction gate passes all eight groups without skips:
363 fixtures/75,095 declared visits per target, 12,415 actual original class calls, one explicit
impact acknowledgement, 2,740 unexecuted living boundaries and 553,679 complete raw-payload
observations. The same required memory gate passes in 103.269 seconds; production comparison
elapsed 206.207 seconds. All 27 native CTest gates, 25 WASM Python gates and both renderer probes
pass the final full build; strict LLVM 19.1.7 style passes all 73 owned C units. Required original
regressions retain their full documented corpora and zero skips. See
[WI 0077](../board/closed/0077_canonical-combat-visits.md) for final bounded evidence and limits.
