# Shared ordered unit collision

`src/sim/collision` owns the complete original bb1b unit-interaction query and the following
hit-aspect arithmetic. It borrows the shared object pool and a complete array of physical body
views. Views borrow live typed poses; they own no duplicate vehicle/projectile state. The module
returns the first hit's physical slot, current registry index/saved word and aspect, and advances
only the shared random owner when the actual interaction rule calls it.

This query is required by the consuming projectile flight stage under active 0065. It does not
advance flight, apply damage, retire objects, animate effects or deliver audio. It is not a guessed
sphere/mesh/ray substitute for those later stages.

## Original instructions and rules

The frozen DOS image pin and executor are shared with [object allocation](object-pool.md).
Actual physical load-module instructions were rechecked: bb1b..bb63 registry walk, 0ea9..0f2f
range test, c14f..c156 interaction dispatch, each e518-table method and b65c..b66b aspect arithmetic.
Frozen patches 538/564/595 locate the relevant repaired reference contracts, but runtime C links
none of that code and the independent oracle executes unmodified original instructions.

The walk reads each of 182 current registry entries in order. It ignores absent bindings, self,
and bodies without flag 0x40. Overwritten imports remain physically occupied but their orphaned
bindings are excluded from the target walk. An orphaned source can still query the current world.
Deletion/side flags do not add extra exclusions: the original uses only the collidable bit here.

0ea9 tests **both independent XY magnitudes** against the inclusive bound
`(target.projection_scale + 256) modulo 65536`. It forms each subtraction modulo a dword before
signed absolute conversion. No Euclidean sum, map-period remapping, swept segment or Z bound is
part of this primitive. In particular opposite signed-dword endpoints are adjacent after wrap.
The original caches signed Z but delegates its acceptance to the type method.

| Target type | Complete acceptance after XY admission |
| --- | --- |
| 0 / 1 / 2 / 3 | Unsigned wrapped source-altitude minus target-altitude is below 2048 / 2304 / 2048 / 1792. |
| 23 / 27 | The same unsigned difference is below 1280 / 3072. |
| 26 | Below the mode-indexed 9ebf height: 4096, 4608, 2816, 3328, repeated for modes 4–7. Source mode mask 0x04 divides that height by four. |
| 5 / 6 | First advance 0291 RNG; its low byte must be below the 95e4 threshold indexed by the **source type**, then signed wrapped height must lie in inclusive -512..512. |
| Every other type | The actual dispatched method returns no interaction. |

Type 26's earlier helicopter label was wrong. Byte-indexed render table `e48c`, read by
`c4fb..c503`, assigns it code 62 (`TARGETS`); codes 50/52 (`APACHE`/`HIND`) belong to types 5/6.
The original heights and interaction behavior above are unchanged. Internal names now describe
type 26 without asserting unsupported subtype semantics; see [remaining damage](other-damage.md).

The type-5/6 thresholds are 128 for source 7; 38 for 8/9/10/12/13; 76 for 11; 253 for 15; zero
otherwise. A reached type-5/6 candidate always consumes its random stream even if its threshold
or height fails. Out-of-range/noncollidable candidates consume no random value. Encounter order
therefore affects future randomness even on a miss. No wall-clock/random-device source is used.
Their further gameplay identity is not claimed by this module.

Type 21 (trees) dispatches to **9c97's clc/ret**, not adjacent 9c99's height-table method. Source
mode mask 0x04, rather than the type-26 target's mode, chooses the reduced height.
The complete eight-word height table ends at 9ecf, where the next table starts. The actual corpus
has type-26 modes 0/1/2/3/7; tests cover all eight supported values, including destroyed modes.
An unsupported mode fails explicitly instead of indexing unrelated following data or masking it.

First accepted candidate wins, including the firing actor. The flight routine only compares the
returned target against the shell's physical origin **after** the walk; it does not resume scanning
for a different hit when its origin wins. The C query retains that ordering. Aspect is the actual
word-width `(-source.heading - target.heading) modulo 65536`, logically shifted right by 12.
It is useful on hits other than the origin when the flight owner reaches damage dispatch.

A valid miss returns NO_SLOT for both slot/index and zero value/aspect. The original's stale last-hit
global on a miss is internal caching, not a new hit. Invalid/null/short views, inconsistent pool,
missing live poses, unsupported type-26 target modes, missing source or invalid random cursor return
-1 without changing random/output. The complete pool and body views remain unchanged on every
query. Prevalidation prevents a late malformed candidate from consuming random values first.

## Verification

```sh
export PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache
bash tools/build.sh all
python3 tools/check_style.py
/tmp/wasm-fist-decoder-oracle/bin/python tests/test_collision.py --originals --oracle
```

The seven-group collision gate is required by native CTest and WASM Node. Its 35,271 valid query
observations on each target cover every target/source type, deterministic height boundaries,
all eight type-26 target modes and source reduction bit, inclusive XY edges/corners, word-bound wraps,
signed-dword endpoints, every source-type/random-byte pair at all four stream cursors, type-5/6
height/order/consumption, all collidable flag bytes, registry vs storage order, duplicate/orphan
bindings, origin hits and aspect wrap/sector boundaries. Invalid world/API/file cases preserve
outputs or fail before publishing observations. The source request is freed before simulation.

The explicitly requested corpus requires all 47 pinned mission snapshots: every one of 4,213
objects queries its complete imported snapshot world, including overwritten bindings, followed
by a launch-position type-8 query for each of 179 M1 actors. These are declared snapshot/contact
boundaries, not proof of all class installation or AI updates. Complete original full-method
execution checks the same inputs and deterministic random sequence without instruction hooks or
replacement callbacks. It compares complete results/RNG and checks all arena and metadata bytes
unchanged. The following aspect instructions are separately executed on each hit before damage.

Peak test storage stays bounded through small complete batches; all original assets remain
read-only. Strict format/tidy and required builds are mandatory. No scene/presentation change
is implied, and full flight/ground impact/damage/effect lifecycle/PCM remain required under 0065.
Final verification results are recorded in closed 0068.

Production-flags sanitizer reproduction (after the sequential production build):

```sh
mkdir -p /tmp/wasm-fist-collision-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itests src/assets/scenario.c src/assets/units.c src/sim/random.c \
  src/sim/object_pool.c src/sim/collision.c tests/probe_io.c \
  tests/collision_probe.c -o /tmp/wasm-fist-collision-sanitizer/collision_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  python3 tests/test_collision.py --originals --target native \
  --native-probe /tmp/wasm-fist-collision-sanitizer/collision_probe
```
