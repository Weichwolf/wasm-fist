# Untargeted M1 shell flight and transient effects

`src/sim/projectile_flight.c` consumes the typed type-8 shell returned by the shared M1 launcher.
The environment borrows a complete physical collision view, its writable shared pool, installed
height image and original random state. The source view must borrow the owning shell's actual pose
and agree with its flags/mode. A transactional local pose replaces only that source view during
collision, so targets are tested against the integrated position rather than the previous tick.
All other bodies remain borrowed and unchanged. No original engine code enters the runtime.

The explicit update protocol is flying → pending ground/unit impact → retired, or flying → retired
on expiry. Pending unit impact returns the first actual registry hit, saved binding word and
aspect while retaining the allocated shell. The caller must dispatch real unit damage with that
hit and the shell's retained collision profile/launch parameter before finishing impact. Damage
may change targets, random streams or pool occupancy. There is no placeholder damage callback.
The post-damage continuation uses the then-current pool, returns the optional initialized effect,
releases the shell and marks its deletion flag. Scheduling a pending/deleted/stale/orphaned shell
through this untargeted update fails atomically; the world owner must stop dispatching it.

`fist_object_pool_find` owns read-only current physical-to-registry lookup (original b354).
Flight/effect retirement reuses the existing pool release owner. All runtime payloads carry their
actual allocation; malformed/stale bindings fail rather than freeing a different object.
Original physical orphan imports remain valid pool metadata and collision targets are still only
current registry bindings. They are outside the registered-object update schedule.

## Original evidence

The frozen DOS image is SHA256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`;
the protected-mode image is SHA256
`102a5465e03f3b0397041b1bd783d33992ac2c14bb6e42a713af610ec08c3ab1`.
The original oracle pins both images and Unicorn 2.1.4, and checks actual method/table/template
addresses before executing them. Reproduce the instruction evidence:

```sh
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xb5e7 --stop-address=0xb6c9 re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xba33 --stop-address=0xbb1b re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x9bc6 --stop-address=0x9be8 re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xe1a6 --stop-address=0xe1d1 re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel \
  --start-address=0x11a6 --stop-address=0x11cb re_out/fist_image.bin
```

Actual e454 dispatch resolves types 8/4/18 to b5e7/bab4/9bc6. Shell age at +2d increments modulo
65536; updated unsigned age ≥480 expires before position, terrain, grace or impact handling.
An age of 65535 wraps to zero and continues normally. For the explicitly untargeted shell,
tracking at ages divisible by 16 or secondary flag 1 returns without writes because target +2f
is zero; clearing target at age 40 is likewise already satisfied. Directed projectiles and other
projectile classes remain separate behavior, never implicitly executed as type 8.

The original integrates signed velocity words into all three wrapped DWORD coordinates. The
terrain gate reads the **word** at altitude+1: `(altitude >> 8) mod 65536 < 128`. Higher altitude
bits do not enter that comparison. It samples the current XY and stores the returned byte at +18,
then branches on the sign bit of the **wrapped byte** altitude-minus-ground subtraction. This
is not interchangeable with an integer less-than comparison for all possible byte inputs. Terrain
is checked before the grace counter; a ground impact retains the still-positive grace. A valid
nonimpact decrements positive grace and returns; zero grace invokes the complete shared bb1b
query. The first origin hit is ignored without searching later targets. Other hits compute the
actual aspect and stop before b673 calls bbb7/c31e damage.

The own-position service contract is directly recovered: DOS e1a7 sets DI to shell+4; protected
op 54 at 11a8 zero-extends DI and adds its installed DGROUP base, then reads X/Y, shifts by 13,
negates Y and calls the installed 8480 sampler. It preserves DI on return. The EBX inbox store in
DOS e1b4 is not this handler's coordinate source. Frozen patch 515's argument about that store
was an inference; the new evidence executes the actual DI-consuming handler instead. Runtime
sampling reuses the existing shared ground sampler rather than adding a second height convention.

Ground impact selects DGROUP 9bdd[type*2], which points type 8 to template 9c1d; unit impact selects
9c4d. These exact templates are:

| Impact | Model | Last frame | Period | Extent | Callback selector | Projection scale |
| --- | --- | --- | --- | --- | --- | --- |
| Ground | 16 | 22 | 6 | 768 | 0 | 2048 |
| Unit | 20 | 10 | 5 | 256 | 4 | 2048 |

Creation at ba33 uses **normal** service bb4f (physical 1b1df) before deleting the projectile.
Muzzle smoke uses low-priority bb46/1b1d6; applying its 120-object gate to explosions would drop
valid effects. At 150 short objects including the shell, the explosion fails to allocate even
though releasing the shell subsequently leaves a free slot. At 149 it succeeds, taking the last
slot before shell release. Constructor/initializer leave flags/frame/height offset zero and copy
all XYZ from the impact; type 4 interprets the common heading word as its model code.

Type-4 updates decrement the countdown byte modulo 256. Zero reloads its period; equality of
current and last frame then retires the effect, otherwise the frame increments modulo 256 and
executes the original callback selected by `selector & 6`. Ground frames 13/16/19 set the height
offset to 768/512/256; unit frames 6/8 set 512/256. Selectors 2/6 have actual empty callbacks.
Last frames remain for a full period: naturally created ground/unit effects retire after 138/55
updates. Original type-18 muzzle smoke increments its word counter, resets it at unsigned ≥8,
then increments the byte frame and retires at ≥7. Naturally created smoke lasts 56 updates;
word/byte wrap cases are retained rather than clamped.

An impact returns sound-dispatch request 15 and a typed ground/unit notice request. Unit impact
also requests the separate hit-voice path. This preserves dispatch intent only: original selected
player notice, 120-tick display, 420-tick hit-voice rate limit, further player/side/speech gates,
be8b sound mapping/device gates and final PCM mixing are future consuming owners. The rate clock
must run before later voice gates just as bfce does. None of these requests claims audible output.

## Verification scope and commands

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_projectile_flight.py --originals --oracle
```

The new mandatory native CTest/WASM Node gate has seven groups. The separately requested original
corpus fails on any missing input, hash mismatch, incomplete output or skipped required group.
It includes all 179 M1 authored launch positions with the complete import/registry occupancy of all
47 missions. Those are explicit snapshot collision worlds, with a supplied test height plane and
velocity; they do not claim installed mission dynamics, actual turret elevation or full mission
play. Original mission inputs are read and rehashed, never mutated.

The oracle executes untouched age/tracking/integration instructions, exact terrain gate, full
actual op-54 height handler, height publication/subtraction, grace, complete ordered query and
aspect arithmetic. Its declared DOS/protected-mode service boundary transfers the actual DI and
returned height register between independently mapped original executors; the DOS e339 device
gate itself is outside this staged evidence. Unit impact stops before actual damage dispatch.
Continuation explicitly resumes after that boundary, executing complete ba33 allocation/template
initialization and the original b6be retirement tail. Original notice/voice/audio consumers are
not run by this oracle. Complete type-4/type-18 methods return through their actual release calls.
Different staged stops require invalidating Unicorn's cached translation blocks; no source bytes
or instruction hooks are modified. Source fields outside each method's declared writes and all
unrelated physical body records are checked unchanged, including orphans. Complete metadata and
random observations accompany every update.

Tests cover every 128 altitude-byte ×256 terrain-height combination for the reached terrain gate,
high-gate cases for every height, current-position cell crossing, altitude word/high-bit behavior,
480-tick natural expiry and age wrap, grace, signed coordinate/velocity extrema, post-integration
hit, first-hit/origin/overwritten-target order, every encounter random byte at all four cursors,
allocation priority/fullness and saved-word deletion wrap. Effect coverage includes complete
natural lifetimes, every frame/countdown/period byte, all original callback choices, extreme word
counters and explicit atomic invalid world/payload/identity/output rejection.

Sanitizer reproduction with the same production flags (after the main build is terminal):

```sh
mkdir -p /tmp/wasm-fist-0069-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itools/rewrite src/assets/units.c src/assets/scenario.c src/assets/vehicle.c src/assets/klc.c \
  src/sim/random.c src/sim/rotation.c src/sim/vehicle_state.c src/sim/ground.c \
  src/sim/object_pool.c src/sim/collision.c src/sim/projectile_launch.c src/sim/projectile_flight.c \
  tools/rewrite/probe_io.c tools/rewrite/object_pool_probe_io.c \
  tools/rewrite/projectile_flight_probe.c -o /tmp/wasm-fist-0069-sanitizer/projectile_flight_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/rewrite/test_projectile_flight.py \
  --originals --target native --native-probe /tmp/wasm-fist-0069-sanitizer/projectile_flight_probe
```

The shared continuous driving scene remains without a fire command. Typed live-world scheduling,
actual damage/components/destruction, effect/projection drawing, notifications, audible audio and
outcomes remain required under 0065/0041. This kernel alone supplies neither a complete playable
mission nor the final ten full WASM acceptance runs. No presentation change is claimed.

Verified on 2026-10-06: all 19 native CTest and complete WASM build gates pass. The expanded native
flight gate separately passes after the full byte matrix/period cases were added. Strict LLVM
19.1.7 format/tidy passes all 52 owned translation units. The complete seven-group original gate
passes 37,691 fixtures/41,158 updates per target in 210.790 seconds with no skips; a subsequent
complete effect-group oracle run covers all added period bytes on both targets (3,075 fixtures,
5,895 updates, 10.195 seconds), including the 256 new fixtures/767 new updates. Combined distinct
coverage is 37,947 fixtures and 41,925 updates per target. Production-flags ASan/UBSan/leak detection
passes the current complete seven-group corpus and those same distinct totals in 7.181 seconds,
without skips. The sanitizer command includes the scenario reader dependency required by the
complete unit decoder. No presented frame changes, so the earlier reviewed driving scene remains
the presentation evidence; this verification supplies no new combat visuals or audible PCM.
Temporary transcripts, fixture files, build logs and sanitizer binaries are removed after commit;
only a compact verification summary remains under /tmp/wasm-fist-0069-flight-review.
