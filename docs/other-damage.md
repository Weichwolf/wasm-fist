# Remaining collision-reachable M1 primary damage

`sim/other_damage` consumes pending M1 type-8/profile-0/parameter-5 hits against types
5/6, 23, 26 and 27. The shared typed actor owns the modeled short-record fields; its allocation
type tags the class state. Restoration copies fields from an owned definition's complete
55-byte snapshot and retains no borrowed storage. This is snapshot restoration, not class
initialization, AI or subsequent class updates. Type 23 uses the independently owned wreck
payload from [ground damage](vehicle-damage.md); its damage entry needs no mutable payload.

`sim/damage_common` owns source/environment validation, random draws and word-width roll/scaling
arithmetic for ground and other damage. Pool, RNG, combat census and explosion state retain their
existing owners. Both targets call the same C. Valid damage leaves the complete source unchanged
and allocated until [impact continuation](projectile-flight.md) executes. Invalid owners, stale
bindings, wrong types/profiles/parameters, invalid subtypes and malformed definitions fail
atomically. Retained destroyed actors accept a subsequent hit as a no-op; released actor identities
are stale and fail. The historical released descriptor must be discarded before installing an
effect that reuses its physical slot.

## Original method evidence

Evidence uses the immutable DOS image at `reference/reconstruction-v1`, SHA-256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
Addresses below are load-module offsets; data offsets use DGROUP `0x1c000`.

Collision dispatcher `e518` admits ground classes 0..3 and types 5/6/23/26/27 to a type-8 hit.
Types 16 and 21 have damage methods but are not reachable through that query. `bbb7` receives
AX=5/BX=0 from flight, stores the source at `e3b2` and dispatches through `e550`/`c31e`:

| Target | Actual method | Complete bounded behavior |
| --- | --- | --- |
| 5/6 | `a0c8..a134` | Skip behavior 12; apply wrapped-byte damage; friendly first reaction; critical census followed by random immediate release or death-animation transition. |
| 23 | `c335` | No damage, random draw, effect or actor mutation. |
| 26 | `bd09..bd68` | Skip mode bit 4; subtype roll/limit; critical type-26 target transition and subtype effect while retaining its binding. |
| 27 | `b396..b406` | Skip nonzero mode; roll/threshold; critical debris transition and shock effect while retaining its binding. |

Every valid dispatch ends with the `c31e`/`60f4` damage-display refresh request, including no-ops.
Render dispatch `c4fb..c503` indexes `e48c` as bytes, assigning 5/6 model codes 50/52
(`APACHE`/`HIND`), 26 code 62 (`TARGETS`) and 27 code 66 (`ARTILL`). The oracle pins these
assignments. Earlier collision documentation incorrectly called type 26 a helicopter;
the collision heights/behavior were already correct, but names now follow the actual type.
Type-26 subtype semantics are not inferred from appearance or that historical label.

Recheck the render-table byte load and later unsigned destruction-word comparison directly:

```sh
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xc4fb --stop-address=0xc507 re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xbc9a --stop-address=0xbcbf re_out/fist_dat_image.bin
```

`b274` and `b396` use the next RNG word's low byte: `base + 1 + ((low * spread) >> 8)`.
Scaling multiplies by the source-side word at `e3ae/e3b0`, truncates the product to 16 bits,
then shifts right eight. Source flag 8 selects the side. These targets do not apply ground-class
aspect factors. M1 profile zero uses the following parameter-5 records:

| Target/subtype | Base, spread | Critical comparison |
| --- | --- | --- |
| 5/6 | 90, 50 (`9b44`) | Wrapped updated damage byte is at least 100; carry is deliberately ignored. |
| 26 mode 0/2 | 100, 40 | Byte carry or updated byte at least the saved limit. |
| 26 mode 1 | 60, 40 | Same saved-limit comparison. |
| 26 mode 3 | 40, 30 | Same saved-limit comparison. |
| 27 | 120, 90 | Byte carry or updated byte at least 80. |

For noncritical 5/6 hits, clear target-side flag 8 and clear secondary bit 2 admit the sole
reaction: set secondary bit 2, behavior 4 and target speed 56, and request voice 38 through
`befb`. This producer has no selected-player gate. Critical damage increments `79a0` or `799c`
according to the victim side, without changing ground counters or source credit. A second RNG
low byte at least 128 creates the normal-priority effect, requests sound 9 and immediately releases
the target. Otherwise behavior becomes 12, target speed 32 and altitude offset zero;
the binding stays occupied. The behavior byte is original +0x25, distinct from common mode +0x19.
The consuming [aircraft death method](aircraft-death.md) proves the signed target-speed word
and altitude-offset byte identities; both names replace the earlier provisional animation names.

Type-26 target restoration accepts original modes 0..7; modes 4..7 are destroyed no-ops. Critical
active modes set flags `(old | 1) & ~6`, clear secondary bit 8, set mode bit 4 and unsigned
destruction parameter 1152. Original later `bc46` compares this word unsigned; restoration
retains its complete 0..65535 domain. Projection scales become 640/640/896/1152 for subtypes
0/1/2/3. Later update semantics of
the parameter remain separate. Type 27 sets mode 1, debris parameter 768, projection extent 320,
animation counter zero, flags `(old & ~6) | 1` and clears secondary bits 8/16. Neither transition
changes the census, roster or allocation type; both request sound 9.

## Effects, ordering and outbound requests

The existing `fist_explosion_create` owner gains three exact authored templates:

| Original template | Model | Extent | Callback | Last frame | Period | Natural release tick |
| --- | --- | --- | --- | --- | --- | --- |
| `9c65`, immediate 5/6 destruction | 20 | 768 | 4 | 10 | 9 | 99 |
| `9c6d`, type-26 target 0/2 | 20 | 1280 | 4 | 10 | 11 | 121 |
| `9c2d`, type-26 target 3 | 16 | 2048 | 0 | 22 | 10 | 230 |

Type-26 target 1 reuses `9c1d` (model 16, extent 768, callback 0, last 22, period 6, release 138).
Type 27 reuses `9c5d` (model 20, extent 448, callback 4, last 10, period 7, release 77).
All initialize scale 2048, frame/height/flags zero and countdown equal to period. Existing
callback/animation/release code supplies the full lifetime, including final released metadata.

Effects allocate before immediate target release. Subsequent shell impact also allocates before
source release. Short-pool capacity therefore changes which effects are admitted and which
physical slot/generation they receive. Sound requests remain valid when effect allocation fails.
No fabricated effect or skipped retirement hides capacity failure. Voice/sound/display results
are explicit producer requests; this step does not claim audible PCM or visible UI execution.

## Verification and reproduction

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_other_damage.py --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_vehicle_damage.py --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_projectile_flight.py --originals --oracle
```

The optional oracle uses pinned Unicorn 2.1.4 from `tools/rewrite/oracle_requirements.txt`.
It executes complete untouched dispatch, target, allocator and effect-update methods. It checks
all modeled actor fields, every unrelated target/source/live-orphan payload byte, initialized
effects' complete 55 bytes, complete pool metadata, census, roster, random state and actual
producer registers. Voice device byte `6ce6=2` and sound 9's unloaded resource provide actual
configured device returns; no hooks, patched instructions or replacement methods execute.

The nine groups cover every initial random low byte across active classes/subtypes and both
source sides, all damage/limit bytes, word scaling extremes/carry, every death-choice byte and
victim side/census wrap, complete flags/secondary/behavior/mode domains, repeated hits and all
short occupancies 2..150. Invalid-input tests check stale actor/source/hit identities and
atomic output/owner preservation. Input storage is freed before simulation observes restored
actors. Explicit no-op/stale checks also follow every constructed critical transition.

Eight reaching cases advance an already-launched shell three ticks to actual collision, then
execute damage, impact and 240 full effect-clock ticks through natural release. This starts at
the declared launched-flight boundary, not an invented class initializer or full mission loop;
[0070](vehicle-damage.md) separately proves complete M1 launch before flight.

The required corpus rehashes all 47 scenarios and covers 1,085 current remaining-target snapshots:
802 type-26 targets, 236 type-27 actors and 47 wrecks, with no unreachable orphan targets. It restores
each actual target's complete original bytes, imports the entire mission occupancy and keeps its
complete original roster. Other imported payloads retain constructor bytes; these cases do not
claim full mission class installation/AI. Types 5/6 are absent from these saved snapshots and
have constructed plus independent-original method/reaching coverage, not snapshot coverage.
Missing originals, skipped required groups, incomplete output and unequal transcript lengths fail.

For ASan/UBSan/leak verification, compile after the single-writer production build finishes:

```sh
mkdir -p /tmp/wasm-fist-0071-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itools/rewrite src/assets/units.c src/assets/scenario.c src/assets/vehicle.c src/assets/klc.c \
  src/sim/random.c src/sim/rotation.c src/sim/vehicle_state.c src/sim/ground.c \
  src/sim/object_pool.c src/sim/collision.c src/sim/projectile_launch.c src/sim/projectile_flight.c \
  src/sim/smoke_animation.c \
  src/sim/vehicle_damage.c src/sim/damage_common.c src/sim/other_damage.c \
  tools/rewrite/probe_io.c tools/rewrite/object_pool_probe_io.c tools/rewrite/combat_probe_io.c \
  tools/rewrite/other_damage_probe.c \
  -o /tmp/wasm-fist-0071-sanitizer/other_damage_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/rewrite/test_other_damage.py \
  --originals --target native --native-probe /tmp/wasm-fist-0071-sanitizer/other_damage_probe
```

No presentation code changes in this step; the reviewed driving scene remains the visual baseline.
Type-23 wreck, destroyed type-26 modes 4..7, complete type-27 updates and type-17 smoke
creation/drift/retirement are delivered separately in [destruction smoke](destruction-smoke.md).
Aircraft 5/6 retained behavior-12 updates and the reaching emitter lifetime repair are delivered
in [aircraft death](aircraft-death.md). Other aircraft behaviors, live type-26 firing,
selected-player loss, full live world scheduling,
fire commands/eligibility, effect rendering, PCM, AI and mission outcomes remain required.
No complete playable mission or final full-WASM acceptance is claimed. Temporary inputs,
logs and sanitizer binaries stay in `/tmp` and are removed after commit/push; compact verified
results are recorded in closed 0071.

Verified on 2026-10-06: all 21 native CTest and complete WASM build gates, strict LLVM 19.1.7
format/tidy for 57 C units, and the complete nine-group both-target original comparison pass.
Per target: 14,510 fixtures, 17,218 damage, 14,510 impact, 24 flight and 1,920 effect-clock ticks;
all required 1,085 snapshots/47 occupancy contexts are included without skips. Final production-
flags ASan/UBSan/leak verification passes the same nine groups and corpus in 15.671 seconds.
Ground damage, flight/effects and corrected collision complete original regressions also pass
both targets; final ground sanitizer verification passes in 25.408 seconds. Full commands,
elapsed gate times, ordering of the label/unsigned-field correction and scope live in
[closed 0071](../board/closed/0071_m1-other-target-damage.md).
