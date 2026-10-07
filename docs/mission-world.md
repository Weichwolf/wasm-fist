# Typed saved-mission world installation

WI 0075 adds `sim/mission_world`, owning the physical allocation metadata, typed payloads,
normal-side physical roster and retained initialization RNG. It installs the complete TRAIN1
DCBS sequence, including every actor that is not currently reachable from the registry.
This is the saved-object installation stage. Terrain/contact, camera/take-control, mission
settings/objectives, complete living class dispatch, dynamic combat installation, rendering
and audio remain separate required stages. The continuous driving scene still owns its existing
player-only baseline; this delivery does not claim an integrated playable battle.

## Original loader evidence

The original load module `re_out/fist_dat_image.bin` retains SHA-256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
Addresses below are raw load-module offsets, near CS zero, DS/SS paragraph `0x1c00`.

```sh
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xd81e --stop-address=0xd84e re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x43c1 --stop-address=0x4430 re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xc296 --stop-address=0xc31e re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x9be8 --stop-address=0x9c5d re_out/fist_dat_image.bin
```

`d81e` loads saved type, registry index and saved registry word, then calls the actual far
snapshot allocator f69:bb12 (physical `1b1a2`). `d82e..d847` preserves the newly allocated
word +2 across the DOS read of all saved bytes after the type. `d84a` calls `43c1`.

Type 23 always ORs flags `40` and secondary flags `24` (hexadecimal), then writes its alternate
platoon/member roster slot unless that computed slot is zero. Other records return immediately
when flags bit `20` is clear. Participants call complete `c296` and their ground-class initializer,
then write the normal platoon/member roster slot. Side swapping via `6db4` is explicitly outside
this normal-side installer. Link byte `6dae` is supplied separately and retains its actual
all-byte behavior; only value 2 changes the initialized operating flags.

All 960 ground records in the pinned corpus participate; no saved short record participates.
Each participating ground actor draws twice from the original four-stream RNG. The initializer
runs in file order, including actors whose registry binding is subsequently overwritten. The
former driving loader initialized only its selected player and discarded the local final RNG;
the new world stage preserves the full initialization sequence for the future mission caller.

TRAIN1 contains 85 records: one type 0, two type 2, 49 type 21, seven type 23, 22 type 26 and
four type 27. Neither tree restoration nor ordinary target restoration runs a random creation
method. For example `9bef` randomizes a newly created tree, but it is not called by loading an
unflagged saved tree. Applying that constructor to saved records would consume wrong randomness
and replace saved pose/extent data.

## Ownership and transactions

`fist_mission_world_initialize` borrows already decoded `fist_units` and initial RNG, builds a
complete staged world, and publishes it only after every record and roster reference succeeds.
It reuses `object_pool` for physical admission, arena/type choice, duplicate registry writes
and finite exhaustion repairs. The pool's physical type is the union tag. Old physical orphans
remain populated; the roster resolves immutable definition ordinals to physical slots, so an
overwritten registry binding does not destroy an independently assigned roster reference.

Delivered payload tags are ground 0..3, aircraft 5/6, smoke 17, tree 21, wreck 23 and targets
26/27. Existing typed restoration owns aircraft/wreck/smoke/target fields. `fist_vehicle_restore`
now owns modeled saved ground fields without initialization or RNG consumption, including stored
ammunition/components, flags/control state, both headings/offsets, movement gate and random
phases. `fist_vehicle_initialize` reuses that decoder, then applies its already-proved actual
class defaults and two random draws. Nonparticipants keep saved state; initialization is not
fabricated merely because their type is a ground vehicle.

Successful output owns no snapshot pointers or transient input storage. Reinitializing an
already populated output replaces the complete world. The initial RNG may alias `out->random`:
the staged initializer captures it before publication, allowing a reload with the current RNG
as its declared new start state. Reset clears every payload, arena, binding, roster and RNG;
the read-only physical getter includes orphans and rejects unused/invalid slots.

Unknown payload classes return `FIST_MISSION_UNSUPPORTED` (2) for the entire installation.
Physical exhaustion returns `UNAVAILABLE` (1); malformed definitions/subtypes, bad RNG and null
arguments return -1. Every failure preserves the previous complete output. No record is skipped
to manufacture a successful mission. A participating short non-wreck is invalid for this typed
ground-participation contract; original c296's extended writes do not fit its 55-byte arena.
Type-26 restoration retains its existing valid mode range 0..7. Living methods are not supplied
by this initializer, and unsupported runtime classes remain required under 0041/0043.

## Saved trees and their conditional method

`sim/tree` owns type-21 pose, extent, scale, flags, ground byte and variant. Complete `9c4f`
copies global byte `930c` to saved byte +19 only when global `930d` equals exactly 1. The typed
request carries those two bytes without converting the change flag to bool or clamping the
variant. Its caller must eventually supply the recovered shared producer `9c5d`; this step does
not replace that producer with an invented animation clock. Current identities are required
for dispatch; a physical orphan can be inspected but is not dispatched through a stale binding.
The separate empty collision method for type 21 does not make its world update empty.

## Verification and limits

`test_mission_world.py` checks full metadata, every installed typed payload, every roster slot
and the complete retained RNG. It covers all ground classes against all 256 flags, both real
participation branches, all link bytes, cursor/seed boundaries and reversed initialization order.
Reload cases initialize the same C output with its current RNG and compare both resulting
worlds against complete original installations from the corresponding declared start states.
Registry/roster overwrite, orphan access, placeholder slot zero, saved words 0/1/ffff and both
physical capacities are covered, including complete 182-object admission and atomic overflow.

Tree verification covers all 65,536 combinations of change/variant bytes, null/stale/type/slot
errors and source release. The C probe checks the entire world is unchanged apart from the
allowed variant byte after each successful tree update. Failure comparisons capture bytes from
the same output object, rather than comparing padding copied through different structs.

The required corpus gate hashes all 47 complete original files. Ten complete missions contain
only delivered payload tags, totaling 671 installed objects, including all 85 TRAIN1 records.
The remaining 37 files are checked for explicit whole-transaction unsupported rejection. They
are not counted as installed, partially simulated, skipped tests or completed missions.

`original_mission_world_oracle.py` runs actual complete pool/roster resets and constructors,
supplies only the declared DOS saved-byte read boundary while preserving the fresh pool word,
then executes actual d84a/43c1 and complete c296/class returns. It compares every complete
original 55/251-byte restored/initialized record, all unrelated prior live/orphan payloads,
metadata, roster and RNG. Tree calls execute complete actual 9c4f returns and compare all
55 bytes. No code patch, hook, initializer replacement or fabricated return is used. Explicit
unsupported, invalid and repaired-exhaustion inputs are C-only failure assertions; unsafe
original overflow is not executed as a successful counterpart.

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_mission_world.py \
  --target native --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_mission_world.py \
  --target wasm --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_vehicle_start.py \
  --target all --originals --oracle
```

Compile the memory probe sequentially after production builds are terminal:

```sh
mkdir -p /tmp/wasm-fist-0075-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itools/rewrite src/assets/*.c src/sim/*.c \
  tools/rewrite/probe_io.c tools/rewrite/object_pool_probe_io.c tools/rewrite/combat_probe_io.c \
  tools/rewrite/vehicle_probe_io.c tools/rewrite/mission_probe_io.c \
  tools/rewrite/mission_world_probe.c -lm \
  -o /tmp/wasm-fist-0075-sanitizer/mission_world_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/rewrite/test_mission_world.py \
  --target native --originals --native-probe /tmp/wasm-fist-0075-sanitizer/mission_world_probe
```

Missing output, unequal traces, required skips and incomplete runs fail. This step changes no
presentation/device/PCM/persistence code. Required native and WASM original gates each pass all
six groups without skips: 900 fixtures, 4,979 installed object states including reload, 306
explicit rejections and 65,569 tree requests. The production-flags ASan/UBSan/LSan probe passes
the same full corpus and counts without skips. All 25 native CTest gates, the complete WASM
build/test script and strict LLVM 19.1.7 format/tidy for 69 owned C units pass. Standard build
gates retain their explicitly optional corpus/device groups; required gates have zero skips.
See [WI 0075](../board/closed/0075_mission-world-installation.md) for exact timings and limits;
compact hashes/results remain under `/tmp/wasm-fist-0075-world-review`, and obsolete owned logs
and memory binaries are removed after verified commit/push.

Canonical combat visits now share `world.combat.roster` with this loader and publish dynamic
payloads before further traversal. See [mission combat](mission-combat.md) for delivered visits
and remaining living-class/UI/device boundaries.
