# Untargeted M1 primary launch

`src/sim/projectile_launch` owns the complete already-eligible, untargeted M1 station-0 launch
transaction. It consumes typed vehicle state and the shared object pool, returning initialized
projectile and optional muzzle-smoke payloads by value. The caller transfers those payloads to its
world at their allocated physical slots. Eligibility, targeting, flight/collision, effect updates,
world installation and device firing input are subsequent contracts under active 0065.

## Recovered behavior

The frozen DOS image is `/tmp/wasm-fist-reference-images/fist_dat_image.bin`, SHA-256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`, reference `349ad31`.
Actual instructions were rechecked at physical load-module offsets 17745..17789, b725..b73a,
1ace0..1adcc, 1addb..1ae3b and 9b5c..9bc5. Service CS is `0f69`, DGROUP is `1c00`.

The complete primary handler decrements station-0 ammunition and marks component-relative byte
37 as 3 before normal type-8 allocation. Empty ammunition changes no vehicle or pool state.
Allocation failure consumes the round and marker but preserves reload, recoil and pending trigger.
Success initializes the projectile, sets reload to 20, attempts low-priority type-18 muzzle smoke,
invokes sound dispatch with AX=12, sets recoil to 16 and clears the pending trigger. Smoke failure
never changes successful firing into failure. The sound output is that dispatch request, not a
playback or audibility decision; c047's selected-player/device/sample gates remain open.

| Payload | Complete initialized launch fields |
| --- | --- |
| Projectile type 8 | Firer X/Y; altitude +2048 with dword wrap; absolute turret heading; signed speed 853; shared spatial XYZ rotation of that heading/elevation; collision grace 2; physical origin; absent target and zero target-height offset; only firer side flag 8; M1 collision profile 0; unclassified original byte +2a = 5. |
| Muzzle smoke type 18 | Firer X/Y plus shared planar forward vector of magnitude 800; altitude +2112 with dword wrap; turret heading +32768 with word wrap; projection scale 512; zero animation counter/frame and flags. |

The heights/profile come from actual DGROUP tables e5c0/e5f8/9302; forward distance comes from
92fa. Original a192 only calls spatial rotation 0459; muzzle initialization directly calls planar
03a9. Normal and coarse modes retain their separate numerical contracts. Constructor storage is
zero except type and arena-local slot, so every remaining original short-record byte is zero.
The original flight tail b643..b652 skips the bb1b unit-interaction walk while +23 is nonzero
and decrements it, identifying the two-step collision grace. Terrain contact precedes that gate.
The typed payloads retain all initialized launch fields; they do not embed original raw records,
guest addresses or a second allocator. +2a's flight meaning is deliberately unclassified.

The original shell's owner word +27 is the **physical actor pointer**, not its registry index.
The C payload normalizes it to the extended physical slot. Duplicate snapshot imports may overwrite
that actor's registry binding without freeing its physical allocation; this does not alter the
shell's origin. Allocation identities and saved words remain owned by `sim/object_pool`.

`fist_m1_launch_untargeted` returns 0 for valid EMPTY, CAPACITY or FIRED outcomes. Invalid/null
arguments, malformed metadata, unsupported class/component extent or a missing/wrong physical
origin return -1 and preserve pool, actor and output. Read-only pool validation reuses its existing
owner. A staged transaction prevents partial invalid publication. Optional payload fields are
consumed only for FIRED/has_muzzle; sound request 255 means no dispatch request.

## Verification

```sh
export PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache
bash tools/build.sh all
python3 tools/check_style.py
/tmp/wasm-fist-decoder-oracle/bin/python tests/test_projectile_launch.py --originals --oracle
/tmp/wasm-fist-decoder-oracle/bin/python tests/test_object_pool.py --originals --oracle
```

The required native CTest and WASM Node launch gate includes eight groups. Fixture verification
covers both rotation modes at angular knots/quadrants, signed position/heading wraps, all side
flags, every extended physical origin, empty/one/max ammunition, initial short occupancies
0/118/119/120/149/150, pending mechanical state on failed allocation, repeated shots through full
capacity, optional smoke, holes, saved registry reservations, duplicate/orphan origins, and invalid
requests/state. Malformed/truncated/trailing files fail without publishing observations. Request
storage is released before simulation. Complete vehicle and pool observers reuse existing owners.
Repeated calls and nonzero mechanical fixtures exercise the station handler directly; they do
not claim a complete timed eligibility/fire/reload sequence.

The explicitly requested corpus requires all 47 pinned scenarios and all 179 M1 actors. Each actor
starts from its complete original snapshot with the proved class initializer and explicit station-0,
untargeted handler boundary. The complete mission import sequence supplies its actual pool occupancy
and physical association, including overwritten bindings. This is not an empty-world assumption.
The two shots per actor cover actual exhaustion and smoke-admission states as encountered.

The independent pinned Unicorn 2.1.4 oracle executes actual reset/import/release and complete
17745 handlers without hooks, patches or replacement instructions. It compares every actor byte,
complete pool metadata and all 55 bytes of each new projectile/effect, including constructor zeros
and normalized physical origin. It also proves every unrelated arena payload unchanged. Original
imports provide constructor payloads for other objects at this declared metadata-only boundary;
no unrelated unit AI or complete mission tick is claimed.

Known original unbounded registry-search routes are excluded from original execution and checked
against the already proved 0066 exhaustion repair on both C targets. This includes reserved vacant
entries with free physical storage, and a single reusable identity admitting a projectile while its
optional smoke cannot allocate. Those deliberate repair cases are explicit, not filtered successes.

Production-flags sanitizer reproduction (run after the sequential production build):

```sh
mkdir -p /tmp/wasm-fist-launch-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itests src/assets/scenario.c src/assets/units.c src/sim/random.c src/sim/vehicle_state.c \
  src/sim/rotation.c src/sim/object_pool.c src/sim/projectile_launch.c \
  tests/probe_io.c tests/vehicle_probe_io.c \
  tests/object_pool_probe_io.c tests/projectile_launch_probe.c \
  -o /tmp/wasm-fist-launch-sanitizer/launch_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_projectile_launch.py \
  --originals --oracle --target native --native-probe /tmp/wasm-fist-launch-sanitizer/launch_probe
```

No rendered frame or device binding changes. Flight, hits, smoke progression/removal and audible
PCM remain required before the complete player fire command is delivered. Temporary artifacts
remain under `/tmp`; accepted gate summaries are recorded in closed 0067.
