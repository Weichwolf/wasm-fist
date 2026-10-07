# M1 primary damage and immediate destruction

WI 0070 delivers shared typed C damage for an untargeted M1 primary shell against the four
registered ground classes. It consumes a pending type-8/profile-0/parameter-5 unit impact from
the [flight owner](projectile-flight.md). Other weapons, other target classes, input eligibility,
the live world scheduler and the complete mission remain required under 0065/0041.

`fist_vehicle_damage_m1` borrows the current shell and changes the target, shared pool, combat
state and random owner transactionally. Invalid, stale, wrong-profile/parameter/type/owner
inputs preserve all owners and output. The shell remains allocated and entirely unchanged.
The caller finishes the shell's impact **after** damage: destruction allocations have already
consumed capacity when the shell owner attempts its separate impact effect and release.

The vehicle owner retains snapshot damage, damage-alarm countdown and platoon/member alongside
its existing motion, weapon and complete component state. Combat state owns source-side scale
words, the selected physical slot, the 32-slot roster, four published platoon counts, destruction
census and damage flash. Scale words are explicit caller inputs recovered from e3ae/e3b0;
mission/settings installation of these words remains a subsequent consumer. They are selected
by the **projectile** side bit, not the victim's side.

## Recovered damage and reactions

Actual bbb7 stores the projectile pointer at e3b2 and dispatches c31e with AX=5, BX=0 and the
collision target at 9a25. Ground entries c336 in e550 call f69:bd0c (physical 1b39c).
The dispatch's caller has loaded AX/BX from the shell; bbb7 itself does not load those fields.
The independent reaching test executes the real launcher and flight prefix to prove them.

| Target class | M1 profile-0/parameter-5 damage base | Random spread |
| --- | ---: | ---: |
| 0, M1 | 90 | 25 |
| 1, M3 | 120 | 90 |
| 2, T80 | 90 | 60 |
| 3, BMP | 120 | 90 |

Damage is base + 1 + ((random AL × spread) >> 8), multiplied by the class's 16-entry aspect
table and then by the selected source-scale word. Each multiplication retains the low 16 bits
before the unsigned eight-bit shift. The final AL is added to the target's byte damage. Carry
or an updated byte of at least 100 destroys the target; a carried result below 100 still destroys.
The hit first sets control bit 32 and retains every other component/weapon/motion field unless
the actual subsequent reaction writes it.

The method pops BX back to the **aspect table** before reaction admissions. Its bytes 2/3/4
supply the fire, turret and track thresholds; the weapon record's trailing bytes do not.
Fire 1a02d consumes a random word, tests mask 0x78, and selects flags from
`{2,4,2,4,2,4,2,6}`. It stops throttle and arithmetically halves signed speed. Its returned AX has
zero AH when triggered, which changes the next turret admission. Turret 1a064 tests mask 0xf8
unless already disabled; track 1a080 tests mask 0x78 unless already disabled, sets motion bit 8
and changes operating bits from 16 to 32. Random consumption follows actual admission/flag order.
Reaction voice producer IDs are 26, 34 and 28, with the original suppression for existing fire.

Nonfatal selected-player hits without secondary bit 2 set both alarm and flash to 4 for damage
0/1 or 20 otherwise, and request sound 18. Actual c31e/60f4 always requests the damage display
refresh. These are producer requests; the voice channel, timer, player/device and sample gates
are not implemented as audible output by this kernel.

## Critical transition and retirement

Actual a93e executes the following sequence:

1. 1610e increments 799e for a side-bit-clear target, or 799a for a side-bit-set target. For a
   clear target hit by a clear projectile it also increments 79a2. Words wrap. Debrief/mission
   credit interpretation remains separate from these recovered counters.
2. ba33 normally allocates a type-4 shock from 9c5d, then a type-4 fire from 9c3d. Both may be
   suppressed by capacity. The existing [effect owner](projectile-flight.md) owns creation and
   complete subsequent animation/retirement for all four reached fixed templates.
3. Request sound 9, then normally allocate a type-23 wreck. 1b10c copies target XYZ and hull
   heading, stores class/model `{10,22,34,46}`, platoon/member, scale 256, parameter +21=768,
   flags 64 and secondary 4. It does not copy the side bit. The remaining full constructor
   payload is zero. The wreck's later bc0c updates are a separate required stage.
4. 160f9 replaces **all** 32 roster occurrences of the target with its wreck. If allocation
   failed, 160e5 removes all such occurrences instead.
5. If selected, publish an explicit player-loss/UI/camera/takeover request. The caller must
   consume this boundary before resuming its world schedule; these consumers remain open.
6. 1b2d3 changes the target's occupied arena header to type 19, sets deletion bit 1 and
   secondary 4. It retains physical occupancy, registry association and saved word. The pool's
   shared `retype` owner requires a current allocation and an unchanged storage class.
7. 1697e publishes counts for four groups of four in the **first 16** roster slots, excluding
   absent entries and type-23 wrecks, and requests their HUD refresh.

Critical damage requests sound 45 before a93e. Effects/wreck are optional payloads returned at
their shared pool slots; they are not a second allocation owner. The type-19 target must remain
owned and dispatched despite its deletion flag until `fist_vehicle_retirement_advance` has
completed the actual c0ba four-update byte countdown. Only zero frees its binding/arena. Stop
dispatch after release; stale or invalid calls fail atomically.
Retirement also checks that the retained vehicle's registry index and generation match the
allocation, preventing a valid slot from being freed through a different vehicle payload.

## Independent original evidence and coverage

`original_vehicle_damage_oracle.py` pins the unchanged DOS image through the existing oracle
owner and executes bbb7/c31e/1b39c, reactions, census, effect/wreck constructors, roster updates,
in-place conversion, platoon publication and c0ba. Unicorn exits observe voice and sound entry
registers before their configured device gates; there are no instruction hooks, patched code
or replacement methods. Speech is busy, positional sound is unfocused and the destruction sound
resource is unloaded. Original producer methods execute their real configured returns.

Selected critical damage stops before a97a and resumes at a990. The balanced intervening
UI/camera/takeover block is an **explicit boundary**, not independent proof of its functionality.
Likewise post-damage impact runs the actual effect constructor and release methods; notification,
voice and sample consumers retain the separate boundary documented in 0069.

Original comparisons observe complete retained target state/components, every created effect
and full 55-byte wreck, RNG, census/roster/counts and all 182 physical/registry metadata entries.
Every unrelated original arena record, including import orphans and the source projectile, is
checked unchanged during damage. Only the actual declared target writes may change. Missing
output and unequal lengths fail.

The required corpus covers all 960 current ground targets from all 47 hash-pinned scenarios
with complete original arena/registry occupancy and the original definition-owner roster.
Targets use their class-installed snapshots with explicitly constructed damage/reaction inputs;
unrelated imports retain constructor payloads. This is complete occupancy and target coverage,
not a full installed mission tick. Orphan targets would be counted explicitly because ordered
collision cannot reach their overwritten bindings; the pinned corpus has none.

Constructed cases cover all initial random low bytes × 16 aspects × four classes, both source
and victim side choices, cursor positions, ordered reaction admissions and every low byte of
each reaction word paired with every high byte, all fire choices, disabled flags and signed-speed
extrema, carried/critical health, word scale extremes, selected/secondary feedback, counter wrap,
repeated hits, short-capacity boundaries and all-32 roster replacement/removal. Invalid-owner
assertions verify atomic failure. The shared allocator's previously proved exhaustion repairs
remain those of 0066; this kernel does not reproduce the original unbounded registry search.

The `-p` reaching probe initializes a registered M1 and T80, launches an actual typed shell and
muzzle, advances three ticks to its independently proved unit hit, dispatches critical damage,
then finishes the impact and retires the target, all explosions and muzzle naturally. The
original gate executes the real corresponding launch/flight/damage/cleanup instructions and
observes full stage state and metadata. Original flight establishes AX=5/BX=0 at its damage
call. This proves the integration sequence; it supplies no visible battle, audible PCM, input
binding, AI or mission outcome.

## Reproduction

```sh
bash tools/build.sh all
python3 tools/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_vehicle_damage.py --originals --oracle
```

The optional oracle environment uses `tests/oracle_requirements.txt` (Unicorn 2.1.4).
The complete required corpus flag forbids skips; routine build tests explicitly omit the corpus.
Run sanitizer compilation after the sequential production build has completed:

```sh
mkdir -p /tmp/wasm-fist-0070-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itests src/assets/units.c src/assets/scenario.c src/assets/vehicle.c src/assets/klc.c \
  src/sim/random.c src/sim/rotation.c src/sim/vehicle_state.c src/sim/ground.c \
  src/sim/object_pool.c src/sim/collision.c src/sim/projectile_launch.c src/sim/projectile_flight.c \
  src/sim/smoke_animation.c \
  src/sim/vehicle_damage.c src/sim/damage_common.c \
  tests/probe_io.c tests/vehicle_probe_io.c \
  tests/object_pool_probe_io.c tests/vehicle_damage_probe.c \
  -o /tmp/wasm-fist-0070-sanitizer/vehicle_damage_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tests/test_vehicle_damage.py \
  --originals --target native --native-probe /tmp/wasm-fist-0070-sanitizer/vehicle_damage_probe
```

No presentation code changes in this step. Earlier driving-scene review remains the visual
baseline; no new combat visualization or audio verification is claimed. Compact accepted
results are recorded in closed 0070. Temporary logs, inputs, transcripts and sanitizer binaries
belong under `/tmp` and are removed after commit/push.

[Remaining M1 targets](other-damage.md) reuse the ground contract's validation and word-width
roll/source scaling through `sim/damage_common`. Their separate 5/6 census lives in the same
combat owner; ground damage retains its existing counters and reactions. Later wreck updates
remain a consuming class stage; the actual type-23 damage method is a no-op.

Verified on 2026-10-06: all 20 native CTest and complete WASM build gates pass; strict LLVM
19.1.7 format/tidy passes all 54 owned C units. The nine-group original comparison passes both
targets in 315.256 seconds with no skips, covering 22,937 fixtures, 25,467 damage, 22,937 impact
and 2,608 retirement updates per target plus the independently reached pipeline. A subsequent
complete critical/census-group oracle run passes both targets in 7.425 seconds, including 32
new class/projectile-side/victim-side/selection cases. Distinct current totals are 22,969
fixtures, 25,499 damage, 22,969 impact and 2,736 retirement updates per target plus the pipeline.
The complete current nine-group production-flags ASan/UBSan/leak gate passes those same totals
and the required corpus in 18.222 seconds with no skips. The shared-creation refactor's complete
flight/effect original regression also passes both targets: 37,947 fixtures/41,925 updates in
277.461 seconds, with no skips. These are kernel/build gates, not full playable mission runs.

Type-23 wreck counter/emission updates and complete type-17 smoke creation, wind drift,
low-word altitude rise and natural release are delivered in [destruction smoke](destruction-smoke.md)
(WI 0072). New wrecks retain their constructor-zero emission counter. The ground damage test
above covers the actual constructor; the separate smoke gate covers restored wreck updates.
The combined ground launch/hit sequence above does not yet schedule those wreck updates.
