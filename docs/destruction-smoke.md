# Persistent destruction updates and drifting smoke

WI 0072 consumes the retained destruction states produced by M1 damage: type-23 wrecks,
destroyed type-26 modes 4..7 and the complete type-27 update method. Shared C owns every
reached field and type-17 smoke creation, wind drift, animation and natural release. Pool,
randomness and pose arithmetic reuse their existing owners. The word-counter/frame helper
also owns existing type-18 muzzle animation, retaining its distinct period and last frame.

These are simulation methods and explicit integration sequences. The continuous driving scene
still needs live world installation/scheduling, player firing eligibility/input, aircraft death,
live type-26 firing, selected-player loss, battle presentation, audible PCM, AI and objectives.
The unchanged reviewed driving scene is the presentation baseline; this step claims no new
visible smoke or complete playable mission.

## Original instructions and state

Untouched load image `re_out/fist_dat_image.bin` has SHA-256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
Near addresses below are physical load-module offsets, CS=0, DGROUP=0x1c000.
Smoke creation is physical **19caa**, far f69:a61a; its address is not near 9caa.

```sh
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xb355 --stop-address=0xb396 re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xbc0c --stop-address=0xbce4 re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x19caa --stop-address=0x19cfd re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x9b11 --stop-address=0x9b76 re_out/fist_dat_image.bin
```

| Method | Counter | Emission and persistent state |
| --- | --- | --- |
| Type 23, bc0c | Word +1f | OR flags 40 and secondary flags 44; increment counter. Emit when low six bits clear and unsigned word +21 exceeds 128. Parameter is unchanged. |
| Type 26 modes 4/6, bc46 | Byte +1e | Increment counter. At low-six-bit zero and unsigned word +1c above 128, attempt smoke, subtract one, then subtract four more if the updated parameter exceeds 768. |
| Type 26 modes 5/7, bc46 | Unchanged | Execute actual flags OR 8 only; no RNG draw, emission or decay. |
| Type 27, b355 | Word +1b | Increment counter. At low-six-bit zero and unsigned word +1d above 128, attempt smoke and subtract one. Mode 1 clears flags 6 and secondary flags 18, then sets flags 1. |

Flag masks and offsets in this table are hexadecimal; thresholds are decimal. Counters wrap
at their actual widths. Type-26/type-27 parameter decay executes even with smoke disabled or
admission exhausted. Wrecks and these targets remain allocated; flags 1 alone does not release
their storage. Aircraft classes 5/6 remain separate required update methods.

Type 27's word **+1b** is distinct from **+1f**, reset by critical damage b396. Typed state
retains both counters. Reaching hit→class-update tests prove that the emission counter survives
the hit while the other counter becomes zero. Type 26 retains its new byte +1e across damage;
the resulting mode/parameter selects the actual destroyed update branch. The API explicitly
rejects living type-26 modes 0..3 instead of replacing their firing logic with a stub.

Wreck restoration owns pose, model/class, scale, parameter, platoon/member, both flags and word
+1f. Smoke restoration owns pose, extent, scale, both flags, ground byte, frame and counter.
Short snapshots are borrowed only during restoration; probes free input storage before updates.
The M1 ground wreck constructor continues to initialize +1f to zero, as proved by its existing
complete 55-byte original constructor comparison.

## Smoke creation, movement and retirement

19caa requires setting byte 8b4f to equal **1**. It then attempts low-priority type-17 allocation
with the existing 120-short-object admission rule. Disabled/full creation consumes no RNG and
preserves output. Successful allocation draws exactly once from the shared RNG. Extent is
`(parameter + (random & 63))` modulo 65536; scale is extent shifted left two modulo 65536.
XYZ copy the emitter and add 768 to the full altitude dword with wrap. Heading, frame, counter,
flags, secondary flags, ground byte and remaining constructor fields start at zero. The oracle
checks the entire new 55-byte payload, not just these exposed fields.

9b11 releases immediately if the setting differs from 1. Otherwise it adds signed dword wind
92f2/92f6 to XY with 32-bit wrap, then adds eight to **only the low altitude word**. A carry does
not increment the high word: altitude 0x1234ffff becomes 0x12340007. Creation's full-dword +768
and update's low-word +8 are deliberately different contracts.

The word counter increments modulo 65536. On unsigned counter >=12 it resets to zero and the
byte frame increments modulo 256. A resulting frame >=30 releases through the original
find/release methods and sets flags 1. Constructor frame/counter zero therefore lives exactly
360 enabled updates. Saved counter ffff wraps to zero; frame ff wraps to zero. These are real
boundary behaviors, not clamps. Type 18 uses the same animation arithmetic with period 8 and
last frame 7, without type-17 movement/settings.

Allocation identity means current physical slot plus registry index and saved word. It is not
a monotonically increasing generation. Released payloads must leave the schedule before their
physical/registry storage can be reused. Invalid/stale/type/owner inputs fail atomically.
The existing allocator's proved finite-search exhaustion repairs remain those of WI 0066;
the new smoke creator reuses them and does not execute the original corrupting unbounded search.

## Verification scope

```sh
export PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_destruction.py --originals --oracle
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_projectile_flight.py --originals --oracle
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_other_damage.py --originals --oracle
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_vehicle_damage.py --originals --oracle
```

`original_destruction_oracle.py` reuses the pinned original executor. It executes complete
19caa, 9b11, bc0c, the declared destroyed bc46 branches and complete b355 without instruction
hooks, patched bytes or replacement methods. Critical successor cases execute real M1 damage,
impact allocation and shell release before parent updates. The declared sequence order is
parent→smokes in physical-slot order→existing impact effects; this is a constructed integration
fixture, not evidence for the full original world scheduler or same-pass admission order.
Rechecked c105..c120 visits all 182 **live registry entries in ascending registry order**, reading
pointer/type at each visit. Allocation into a later entry can therefore execute that same pass;
an earlier entry waits. The full c0e5 prefix/global behavior and this mutable pass still need a
shared live-world owner before integration into the scene.

Six required groups cover 9,135 valid fixtures, 9,166 parent updates and 14,701 complete smoke
states per target. They include all setting/frame bytes, all type-26 counter bytes, all type-27
mode bytes, word counter/parameter boundaries, RNG low-six-bit choices, flag domains, signed
wind/altitude wrap, low-word carry, every short occupancy 1..150, 119/120/149/150 multi-tick
capacity sequences, full 360-tick release and 500-tick damage/emission/cleanup sequences.
Required corpus cases include 317 unchanged targets (17:28, 23:47, 26:6, 27:236) plus 796
explicitly constructed critical type-26 successors from saved live targets, totaling 1,113
cases across all 47 complete mission occupancy contexts. Those 796 snapshots change only the
damage byte to 99 before the actual critical hit; they are not unchanged original save states.

Original comparisons observe every exposed parent/smoke field, pool bitmap/count/physical type,
all 182 registry bindings and RNG state at every declared clock. All unrelated parent/smoke
payload bytes and imported objects, including orphans, remain unchanged. The source shell is
checked entirely unchanged through damage; after impact release its storage may be reused.
Other imports retain constructor payloads; their real class updates, AI and roster behavior
are not scheduled here. Existing full damage/flight gates independently cover their effect,
roster/census and field contracts. Missing output, unequal lengths, skipped required corpus
groups and incomplete runs fail. Probe null/stale/output checks capture bytes from the same
object before the invalid call; no comparison depends on padding from a copied C structure.

Run sanitizer compilation sequentially after the production build is terminal:

```sh
mkdir -p /tmp/wasm-fist-0072-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itools/rewrite src/assets/units.c src/assets/scenario.c src/assets/vehicle.c src/assets/klc.c \
  src/sim/random.c src/sim/rotation.c src/sim/vehicle_state.c src/sim/ground.c \
  src/sim/object_pool.c src/sim/collision.c src/sim/projectile_launch.c src/sim/projectile_flight.c \
  src/sim/smoke_animation.c src/sim/vehicle_damage.c src/sim/damage_common.c src/sim/other_damage.c \
  src/sim/smoke.c src/sim/destruction_updates.c tools/rewrite/probe_io.c \
  tools/rewrite/object_pool_probe_io.c tools/rewrite/destruction_probe.c \
  -o /tmp/wasm-fist-0072-sanitizer/destruction_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/rewrite/test_destruction.py \
  --originals --target native --native-probe /tmp/wasm-fist-0072-sanitizer/destruction_probe
```

Final verified counts/times and limitations are recorded in closed WI 0072. Originals and the
frozen reference stay untouched. Temporary inputs, logs and sanitizer executables remain in
`/tmp` and are cleaned after commit/push; a compact verification summary is retained there.

Verified on 2026-10-07: both production builds (22 native CTest gates and complete WASM gates),
strict LLVM 19.1.7 format/tidy for 61 C units, both complete original smoke comparisons and
full both-target damage/flight regressions pass. New destruction ASan/UBSan/leak verification
passes all six groups and required corpus in 12.547 seconds; shared muzzle/flight memory
regression passes all seven groups and complete corpus in 14.911 seconds. All required
original/memory gates have zero skips. See [closed 0072](../board/closed/0072_destruction-smoke-updates.md)
for exact elapsed times, counts and the preserved open world/player/AI/render/audio boundaries.
