# Shared runtime object allocation

`src/sim/object_pool.c` owns physical occupancy, live counts and runtime registry binding for
both original object classes. It supplies reset, dynamic allocation, explicit snapshot import
and release. Native and WASM use the same C11 implementation. `assets/units.c` owns the common
type-to-state-size classification used by both the immutable decoder and this runtime allocator.

This is the allocation prerequisite for live weapon/world state. The module owns metadata;
typed vehicle, projectile, effect and other object payloads belong to their simulation owners.
It does not install those payloads into the driving session, fire a shot, integrate flight,
resolve hits or play audio. New payload owners must initialize their complete typed state after
allocation and apply deletion semantics when releasing it. No raw guest pointer enters C state.

## Identity and admission

| Arena | Capacity | Original record size | Original near-address range |
| --- | --- | --- | --- |
| Short objects | 150 | 55 bytes | `a022` through `c05c` exclusive |
| Extended objects | 32 | 251 bytes | `c05c` through `dfbc` exclusive |
| Registry | 182 entries | pointer word + saved word | `dfbc` through `e294` exclusive |

Types 0, 1, 2, 3 and 19 use the extended arena; the other valid types through 27 use the short
arena. This matches every flag at DGROUP `e614..e62f`. Flat C slots 0–149 address short storage,
150–181 extended storage. The original constructor's pool index is relative to its arena;
the C allocation result deliberately exposes the flat typed-storage slot instead.

Dynamic allocation selects the first free physical slot in the type's arena and the first
registry entry whose pointer **and saved word** are empty. Its saved word becomes one.
An explicit snapshot binding selects the first physical slot, but uses the requested registry
index and saved word. Duplicate imports overwrite that binding while retaining the older
physical allocation until reset, exactly as the original loader does. Reset clears all live
occupancy and bindings, including these orphaned allocations.

Release detaches the current registry binding, frees its physical slot and decrements the
saved word modulo 65536. A remaining nonzero word keeps that vacant registry entry reserved;
the next dynamic allocation skips it. Zero therefore wraps to 65535 on release. The historical
immutable-definition field named `generation` retains this saved value; it is not a monotonic
generation ID and cannot alone prove that a reused reference remains valid. Runtime references
therefore use a physical slot plus an opaque process-issued lifetime. Allocation/import gets
a fresh lifetime; release/reset invalidates it; in-place retype and binding orphaning retain
it. The immediate allocation tuple still identifies current saved-format metadata, not an
allocation lifetime. See docs/target-discovery.md for the reaching alias proof and consumers.

Low-priority allocation uses the original `b1d6` admission gate: a short-object count of 120
or greater rejects the request before dispatch to either type class. Normal allocation can
use all 150 short slots. The M1 primary handler uses normal admission for its type-8 projectile,
then low-priority admission for type-18 muzzle smoke. Starting at 118 short objects admits both;
starting at 119 admits the projectile and suppresses smoke. At 150, the projectile allocation
fails **after** the actual ammunition decrement and ammunition-component refresh.

The API returns OK, UNAVAILABLE or -1 for invalid state/input. Failed allocation/import/release
preserves the entire pool and output. It validates arena/type/count/registry ownership instead
of continuing through inconsistent state. Missing release bindings return UNAVAILABLE.

## Recovered instructions and explicit corruption repairs

The image pin, DGROUP and Unicorn environment are described in [unit definitions](unit-definitions.md).
Actual physical routines are reset `1b176`, explicit binding `1b1a2`, low-priority admission
`1b1d6`, dynamic allocation `1b1df`, arena allocator `1b21d` and release `1b2ef`.
Short count/bitmap are DGROUP `e294`/`e2f7`; extended count/bitmap are `e296`/`e38d`.
Reset clears exactly 150 short and 32 extended occupancy bytes and 182 registry entries.
Legacy allocation high-water diagnostic words are outside the new gameplay metadata contract.

The actual original extended allocator tests count against 150 at `1b25b`, although its
251-byte arena ends after 32 slots at the registry. Complete original execution of 33 explicit
type-0 imports returns success with DI = `dfbc`: the constructor writes `(type=0, pool=32)`
over registry entry zero, then clears subsequent registry bindings as payload. The C allocator
rejects the 33rd extended allocation without mutation. Its capacity comes from the actual
arena end, reset extent and 32-slot roster, with a reaching corruption proof.

The dynamic registry scan at `1b1eb..1b1fa` has no end check. With all 182 entries vacant but
reserved by saved word one, actual original execution allocates a physical slot and returns
registry index **184**, writing outside the registry into diagnostic globals. C rejects that
request without reserving physical storage. This is a deliberate repair of proved memory
corruption, consistent with the functional rewrite target; tests do not disguise it as original
success/failure identity. All 47 original missions stay within the valid arenas: corpus maxima
are 132 short objects, 32 extended objects and 164 total records.

## Verification

```sh
bash tools/build.sh all
python3 tools/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_object_pool.py --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_units.py --originals --oracle
```

Seven test groups cover every type and admission route, physical/registry holes, duplicate
imports, reset, retained/wrapped saved values, both exhaustion repairs, invalid requests/state,
complete file rejection and all 4,213 original snapshot bindings followed by runtime release/
reallocation. Every operation observes complete live counts, all 182 occupancy/type slots,
all 182 registry bindings and the allocation output on both targets.

`original_object_pool_oracle.py` executes complete original methods without hooks or substituted
calls. It additionally checks complete original constructor payload initialization and the
release's sole deletion-flag write. It proves the two corruptions separately from normal state
comparisons. The complete M1 primary-handler runner now accepts an explicit initial short-pool
occupancy; its unchanged real handler proves full actor changes, allocated objects, carry and
muzzle admission at 118, 119, 149 and 150. This does not implement C firing eligibility/launch.
Missing requested originals/instruction coverage, unequal output or incomplete runs fail.

Production-flags memory verification:

```sh
clang -std=c11 -O2 -g -Wall -Wextra -Wpedantic \
  -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math \
  -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer \
  -Isrc -Itests tests/object_pool_probe.c tests/probe_io.c \
  src/sim/object_pool.c src/assets/units.c src/assets/scenario.c \
  -o /tmp/wasm-fist-0066-pool-sanitized-probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_object_pool.py \
  --target native --originals --oracle --native-probe /tmp/wasm-fist-0066-pool-sanitized-probe
```

No presentation, existing driving state, original asset contents or frozen reconstruction
changes in this step. WI 0065/0041 retain firing, projectiles/hits, objectives and audible events.
