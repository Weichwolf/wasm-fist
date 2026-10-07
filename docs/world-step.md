# World time and mutable registry traversal

WI 0074 supplies two shared C contracts in `src/sim/world_step`: the common world tick prefix
and a resumable traversal of the current object registry. They provide the ordering needed to
connect the delivered combat owners to a live mission. They do not install mission payloads,
dispatch living class methods, bind firing input, mix PCM or resolve mission outcomes.

## Original evidence

The frozen DOS load module is `/tmp/wasm-fist-reference-images/fist_dat_image.bin`, SHA-256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
Addresses below are raw load-module offsets, CS zero, DS paragraph `0x1c00`.
The following original instructions were rechecked, rather than relying on patch descriptions:

```sh
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x4712 --stop-address=0x47e0 /tmp/wasm-fist-reference-images/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x5167 --stop-address=0x5177 /tmp/wasm-fist-reference-images/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x1a5dc --stop-address=0x1a618 /tmp/wasm-fist-reference-images/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xbeb7 --stop-address=0xbf15 /tmp/wasm-fist-reference-images/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xc0e5 --stop-address=0xc123 /tmp/wasm-fist-reference-images/fist_dat_image.bin
```

`c0e5` calls `4712`, increments simulation word `6cde`, calls `beb7`, then decrements each
nonzero auxiliary word `969e` and `969c`, in that order. `4712` treats `6da6..6da8` as three
independent bytes: minutes, seconds and subticks. Minutes `ff` disables the countdown; all-zero
time stays zero. Otherwise subticks decrement with byte wrapping; an `ff` result borrows into
seconds and reloads subticks to 59. A seconds borrow reloads seconds to 59 and decrements minutes.
The API retains all byte states, including noncanonical values, without imposing a clock mask.
`47c7` initializes `(60,0,0)`; `5167` copies the mission-limit byte into minutes and clears
seconds. The supervisor at `1a5ef` tests all three bytes for zero before selecting TIME EXPIRED.
That supervisor and its teardown/outcome actions are not implemented by this prefix.

`beb7` compares non-`ff` pending byte `9fd6` against its scheduled word `9fd7` and the newly
incremented simulation tick. On equality it calls `befb`, records ISR word `0452` in `9fca`,
and consumes the pending byte even with voice mode 2. Unmuted `befb` reaches `e2c2` with
`AX=0x280|voice`, `DL=0`, `ECX=0`. The C result exposes this producer command, or `NO_COMMAND`,
and distinguishes a consumed muted request from no due request. It does not claim audible PCM.
The caller supplies the device timer separately from the wrapping simulation tick.

`c105..c120` walks 182 entries from `dfbc`, ascending by four-byte registry entry. At each
arrival it reads the current physical pointer, then the current physical type. The indirect
call at `c117` selects the word at `e454 + type*2`. It does not filter flags or snapshot the
registry. `c11b` restores SI/CX after the actual class call and proceeds to the next entry.
An allocation into a later entry can run during this pass; a birth at the current or an earlier
entry waits for the next pass. An overwritten orphan is not visited merely because its physical
slot remains allocated. Physical allocation order is not execution order.

## Shared ownership

`fist_world_begin_tick` stages its clock and result before publication. Null input preserves
both owners. `fist_world_next` validates the existing allocation owner and cursor, reads the
pool afresh on every call, and returns type, physical slot, registry index and saved word.
The successful call advances the cursor before the caller executes that object's method.
End preserves the previous allocation output; repeated end is stable. Invalid pool, null
arguments or a cursor beyond 182 preserve both cursor and output. A zero-initialized pass starts
at entry zero. The iterator owns no payload storage or class dispatch.

The world probe connects actual existing type-4 explosion, type-17 drifting smoke, type-18
muzzle, type-23 wreck, destroyed type-26 and type-27 owners through this iterator. Smoke births
are installed before the next query. Fresh allocation/import/retype removes any stale fixture
payload before it can be dispatched. Unknown classes and living type-26 restoration are errors,
rather than successful empty updates. This probe is a verification caller, not a production
mission installer. Living trees also require their actual conditional `9c4f` animation update;
their separate collision method being empty does not make their world method empty.

Complete wreck observations now share `combat_probe_io`, and independent parent/smoke arithmetic
shares the existing destruction golden owner. Other production class implementations are reused
without changes. Existing aircraft behavior-12 and damage/flight contracts remain as documented
in their own owners; this fixture's complete class dispatch is explicitly limited to the six
listed types.

## Verification boundaries

`test_world_step.py` compares every clock/RNG/pool state, visit and complete typed payload output.
Its six required groups cover all 256 values of each countdown byte against the other lanes'
zero/borrow/disabled boundaries, wrapping simulation words, both auxiliary countdown boundaries,
all pending voice bytes, all voice modes, due/missed schedules and a long countdown-to-zero trace.
Mutation fixtures cover every arrival index, all 28 physical types at the call boundary, holes,
overwrite/orphans, retype, release/reuse, current/future/earlier births and repeated end.

Complete class lifetime traces use the real mutable pass, including full effect/smoke retirement,
parent emissions, capacity 1/119/120/149/150, disabled smoke, immediate filler retirement and
registry indices 0/91/181. Original constructors and natural release determine the resulting
visit sequence. A wreck at entry zero can emit smoke that advances later in the same pass;
a lone wreck at 181 emits into an earlier entry and that smoke waits. The complete pinned corpus
adds all 4,213 snapshot imports and all 47 occupancy contexts. These corpus queries stop at
the class-call boundary: they verify current bindings and orphan traversal, not whole mission
payload initialization or living class behavior.

`original_world_step_oracle.py` executes untouched original instructions. Muted/no-request
prefixes run through the actual common prefix to `c105`. Unmuted requests stop at the real
`e2c2` producer boundary; the actual caller fragments `bece..bedb` and `c0ef..c105` are checked
separately. No protected-mode audio return is fabricated and no PCM comparison is claimed.
Traversal-only fixtures pause before the class call. Lifetime fixtures execute the actual
indirect `c117` CALL through its real `c11b` return, preserving the outer loop's pushed SI/CX.
Fixture allocation commands use an independent caller stack so they cannot overwrite that frame.
No original instructions, methods or constructors are replaced by hooks or return stubs.

The oracle compares complete metadata and RNG after every operation, complete reached parent/
effect/muzzle/smoke fields, every new 55-byte smoke constructor, all unrelated payload bytes,
and all other live/orphan payloads. Canonical metadata observations may be reused only for an
exact identity key containing both counts, both complete allocation bitmaps, every registry
binding and every physical type word. Payload observations are never cached. Two C-only invalid/
repaired exhaustion fixtures have no valid original API equivalent and are explicitly separate;
they do not weaken the original comparisons or appear as skipped required tests.

## Reproduction

```sh
bash tools/build.sh all
python3 tools/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_world_step.py \
  --target native --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_world_step.py \
  --target wasm --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_destruction.py \
  --target all --originals --oracle
```

Compile memory probes sequentially after the production build is terminal:

```sh
mkdir -p /tmp/wasm-fist-0074-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itests src/assets/units.c src/assets/scenario.c src/assets/vehicle.c src/assets/klc.c \
  src/sim/random.c src/sim/rotation.c src/sim/vehicle_state.c src/sim/ground.c \
  src/sim/object_pool.c src/sim/collision.c src/sim/projectile_launch.c src/sim/projectile_flight.c \
  src/sim/smoke_animation.c src/sim/vehicle_damage.c src/sim/damage_common.c src/sim/other_damage.c \
  src/sim/smoke.c src/sim/destruction_updates.c src/sim/world_step.c tests/probe_io.c \
  tests/object_pool_probe_io.c tests/combat_probe_io.c \
  tests/world_step_probe.c -o /tmp/wasm-fist-0074-sanitizer/world_step_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tests/test_world_step.py \
  --originals --target native --native-probe /tmp/wasm-fist-0074-sanitizer/world_step_probe
```

Use the same compiler/source command with `tests/destruction_probe.c` instead of
`world_step_probe.c` and output `destruction_probe`; run `test_destruction.py --originals
--target native --native-probe /tmp/wasm-fist-0074-sanitizer/destruction_probe` with the same
sanitizer environment. This regression verifies the extracted observation/arithmetic owners.
Missing output, unequal complete traces, required skips and incomplete runs fail. This step
changes no presentation, device input, PCM or persistence code. The reviewed driving baseline
remains the presentation evidence; it is not evidence of a playable battle.

Verified on 2026-10-07: all 24 native CTest gates, complete WASM gates and strict LLVM 19.1.7
format/tidy for all 66 owned C units pass. Each required world target has 19,288 fixtures,
45,040 visits, 40,107 class updates and 40,262 tick prefixes. Native/WASM original runs pass
in 212.293/304.888 seconds; world memory verification passes in 60.215 seconds. The required
both-target destruction original regression passes in 179.935 seconds with 9,135 fixtures,
9,166 parent updates and 14,701 smoke states per target; its memory gate passes in 5.777 seconds.
Every required original/memory group includes its complete declared corpus with zero skips.

Exact coverage and limits are recorded in [closed WI 0074](../board/closed/0074_mutable-world-pass.md).
Compact hashes/results remain
under `/tmp/wasm-fist-0074-world-review`; obsolete owned logs/binaries are removed after push.
