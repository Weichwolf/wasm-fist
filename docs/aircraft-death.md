# Retained aircraft death and smoke emitter lifetime

WI 0073 supplies the complete original type-5/6 update method for retained death behavior 12.
Shared C owns altitude, rotor phase, speed and heading servos, airborne spin, grounded
destruction, post-release smoke and planar motion. It reuses the height, rotation, random,
allocation, explosion and drifting-smoke owners. Snapshot restoration copies every reached
field before releasing input storage. Living aircraft initialization/AI and other behaviors
remain required methods; unsupported behaviors fail atomically.

This is a simulation delivery. The driving scene still needs the mutable live-world pass,
fire eligibility/input, live type-26 firing, selected-player loss, battle rendering, audible
PCM, AI and mission outcomes. No presentation code changes here; the reviewed driving scene
remains the visual baseline. Scripted death/effect sequences do not prove a playable mission.

## Original method and field evidence

The untouched load image `re_out/fist_dat_image.bin` has SHA-256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
Addresses below are load-module offsets, CS=0, DGROUP=0x1c000. The actual word update table
at DGROUP e454 maps both types 5 and 6 to **9e2b**. Callback table **CS:9f0f** is indexed
by the behavior's byte offset, without multiplying by two; behavior 12 selects **a03f**.
The independent oracle pins both tables before executing original instructions.

```sh
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x9e2b --stop-address=0x9f0f re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xa03f --stop-address=0xa069 re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x19caa --stop-address=0x19cfd re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x1b201 --stop-address=0x1b21a re_out/fist_dat_image.bin
```

| Payload offset (hex) | Recovered contract |
| --- | --- |
| +0d | Second byte of the altitude dword; periodic ground/height servo changes only this byte. |
| +18 | Wrapped byte difference between that altitude byte and center terrain height. |
| +1a | Rotor frame, independent global animation word 6d14 &7. |
| +1b / +1d | Signed word current speed / target speed. |
| +23 | Decrementing word behavior countdown. |
| +25 | Behavior byte, distinct from common mode +19. |
| +2e | Desired heading word. |
| +30 | Signed-byte motion-heading servo. |
| +32 | Unsigned byte desired offset above ground. |
| +33 | Damage byte. |

The earlier `animation_parameter` and `animation_frame` names from damage restoration are
now `target_speed` and `altitude_offset`. Real critical M1 damage sets these to 32 and zero;
the noncritical reaction sets target speed 56. These writes retain their previous bits.

Every 9e2b entry draws RNG through 0291 and retains the roll for heading. When tick word
6cde's low two bits are zero, e1d1/op54 samples center terrain at the initial XY, before
movement. Clamp the unsigned altitude byte upward to ground, then move it one toward the
wrapped byte `ground + altitude_offset`. All other altitude bytes remain untouched. The
offset can wrap below ground: no second clamp conceals that original behavior. Record the
wrapped ground difference after the servo. Rotor animation uses 6d14, independently of 6cde.

Decelerate current signed speed by one every update when above target; accelerate by one
only when tick &15 is zero. Move the motion-heading byte one toward the signed high byte
of the current heading, before turning the heading. Interpret the desired-minus-current
heading difference as wrapped signed 16-bit, then limit the turn magnitude to
`45 + (entry_roll &15)`. Decrement the behavior countdown with word wrap; invoke a03f when
its low five bits become zero.

Airborne a03f adds 1456 to the desired heading with word wrap and copies its high byte to
motion heading. Grounded a03f allocates the existing normal-priority 9c65 explosion, requests
sound 9, releases the aircraft and sets flag 1. Damage already owns the destruction census;
this callback adds no second count. Effect admission failure still requests sound and releases.
The complete explosion constructor, callback heights and 99-update natural release reuse
[other damage](other-damage.md) and [projectile flight](projectile-flight.md).

The enclosing method continues after callback release. When damage >10 and tick &3 is zero,
draw a second RNG word even if smoke is disabled or admission will fail. If its low two bits
are zero, attempt 19caa with extent base 384. Successful low-priority admission consumes the
constructor's additional RNG draw. The final 9eef tail rotates signed speed using the motion
heading byte and applies wrapped dword XY movement. No early return suppresses this tail.

## Reaching emitter-loss defect and deliberate repair

Untouched original execution proves a lifetime defect in the grounded callback path. The
aircraft releases its physical slot before the enclosing method attempts smoke allocation.
Successful low-priority allocation can select that same slot. Allocator 1b201..1b219 clears
the new 55-byte payload before smoke constructor 19cdc..19cf6 reads the borrowed emitter.
DI and SI then point to the same payload: original smoke is born at **(0, 0, 768)** rather
than at the crash position. The final original tail reads the new constructor's zero speed
and motion heading, so it does not move the replacement smoke.

The shared smoke creator captures the emitter pose **before admission**, preserving its
lifetime independently of world-slot reuse. This is the sole deliberate behavior repair in
this step. Constructor extent/scale, all other bytes, random draws, effects, metadata, flags,
frame/counter and retirement remain exact. Returned released actors must leave the world
schedule before installing effects or smoke in reused storage. In the same-slot case their
reported historical state is their last state before replacement; the tail applies to the
new smoke's constructor-zero motion instead. Nonalias release still runs the old motion tail.

The original oracle never patches its machine. It independently asserts the complete actual
lost-emitter constructor, including zero XY and altitude 768, and asserts that 9eef leaves
that replacement unchanged. Only observations of these proved alias emissions receive the
captured position correction for comparison with the repaired C. Ordinary emissions and all
other original fields remain direct observations. XY observation correction uses dword wrap;
altitude preserves the corrected birth high word and translates only the low word during
9b11 rise. A reaching ground-height-252 case gives a corrected birth low word ffff and proves
that subsequent low-word rise must not propagate carry. A separate sequence releases two
earlier slots, proving that post-release smoke can also allocate without aliasing its emitter.

## Verification and ownership scope

```sh
export PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_aircraft_death.py --target native --originals --oracle
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_aircraft_death.py --target wasm --originals --oracle
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_other_damage.py --originals --oracle
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_destruction.py --originals --oracle
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_projectile_flight.py --originals --oracle
```

Six required groups cover the complete ground-byte domain, altitude/offset wrap, phase and
flag domains, signed-speed/heading/countdown boundaries, every motion-heading byte at both
rotation qualities, random streams, smoke settings, rotor phase, all short occupancies 1..150,
word clock wrap and complete 1500-update death / 700-update actual M1 damage sequences.
Smoke and explosion methods run through their complete natural lifetimes. Malformed input,
unsupported behaviors, null/invalid/stale owners and output preservation fail explicitly.
Atomic checks capture bytes from the same object before the call; copied struct padding does
not determine equality. Probes free input storage before observing any restored actor.

The 47 pinned mission files contain **no saved type-5/6 objects**. Required corpus coverage
therefore constructs 1,920 critical successors, one of each aircraft type at every one of the
960 saved ground-unit positions, keeping each complete mission occupancy context. These are
constructed aircraft with a declared varied 4x4 terrain fixture, not original saved aircraft
or fully installed mission height fields. Actual M1 dispatch/impact/source release precede
aircraft updates; independent original damage gates retain full census/roster coverage.

`original_aircraft_death_oracle.py` executes real 9e2b/a03f, op54 height transfer, 19caa, 9b11
and bab4. It reuses the existing original height-service owner, extended to accept the declared
field size. There are no instruction hooks, patched bytes or substituted original methods.
Sound 9's unloaded resource supplies its actual configured device return; the test proves the
request register, not audible PCM. Every modeled actor/effect/smoke field, complete new 55-byte
constructor, pool bitmap/count/physical type, all registry bindings and RNG state are compared.
Unrelated payload bytes and imported objects, including orphans, remain unchanged.

The fixture order is aircraft → physical-slot smoke → physical-slot effects. This explicit
sequence is not the live world scheduler: c105..c120 instead visits the current 182 registry
entries in ascending order, reading each current pointer/type on arrival. Later-entry
allocations can run in the same pass. Install that mutable shared owner before live integration.
`combat_probe_io` now owns complete typed damage/death/destruction observations; the independent
effect golden arithmetic has one owner shared by flight, damage and aircraft verification.

Compile the sanitizer probe sequentially after the production build is terminal:

```sh
mkdir -p /tmp/wasm-fist-0073-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itools/rewrite src/assets/units.c src/assets/scenario.c src/assets/vehicle.c src/assets/klc.c \
  src/sim/random.c src/sim/rotation.c src/sim/vehicle_state.c src/sim/ground.c \
  src/sim/object_pool.c src/sim/collision.c src/sim/projectile_launch.c src/sim/projectile_flight.c \
  src/sim/smoke_animation.c src/sim/vehicle_damage.c src/sim/damage_common.c src/sim/other_damage.c \
  src/sim/smoke.c src/sim/aircraft_death.c tools/rewrite/probe_io.c \
  tools/rewrite/object_pool_probe_io.c tools/rewrite/combat_probe_io.c \
  tools/rewrite/aircraft_death_probe.c -o /tmp/wasm-fist-0073-sanitizer/aircraft_death_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/rewrite/test_aircraft_death.py \
  --originals --target native --native-probe /tmp/wasm-fist-0073-sanitizer/aircraft_death_probe
```

Run the shared damage sanitizer using its [updated reproduction command](other-damage.md).
Final counts/times and verification limits are recorded in WI 0073. Missing output, unequal
lengths, required skips and incomplete runs fail. Originals and frozen reconstruction stay
pristine. Exact owned temporary logs/binaries are cleaned after commit/push; compact hashes
and results remain under `/tmp/wasm-fist-0073-aircraft-review`.

Verified on 2026-10-07: both production builds (23 native CTest and complete WASM gates),
strict LLVM 19.1.7 format/tidy for all 64 C units, both complete original aircraft comparisons
and all required both-target damage/destruction/flight regressions pass. Each aircraft target
covers 59,488 fixtures, 67,466 class updates, 124,648 smoke states, 15,565 effect states and
2,102 releases; each original run independently proves 129 lost-emitter constructors. Required
native/WASM aircraft original gates take 731.731/741.424 seconds. Production-flags aircraft
memory verification takes 46.813 seconds; shared damage memory verification takes 8.950 seconds.
All required original/memory groups include their complete declared corpora with zero skips.
See [closed 0073](../board/closed/0073_aircraft-death-updates.md) for exact scope and results.
