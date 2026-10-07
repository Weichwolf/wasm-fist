# Untargeted M1 primary fire dispatch

WI 0076 adds shared `sim/primary_fire`, consuming the pending/automatic branch at the actual
post-behavior class boundary and dispatching the complete untargeted M1 station-zero handler.
`fist_mission_world_fire_untargeted` consumes the canonical physical actor and publishes complete
shell and optional muzzle payloads before another world visit. The existing allocation owner
remains authoritative; overwritten registry bindings do not invalidate a physical origin.

This is a consuming fire stage and payload publication, not a complete living-class tick or
playable battle. The caller must establish an untargeted M1 with station zero selected and
supply the actual class state/current simulation tick. Living ground methods, target resolution,
other stations/classes, input/link eligibility, dynamic flight/damage/effect dispatch, battle
drawing, audible events and mission outcomes remain required under 0065/0041. The device fire
command remains unbound until its live flight/damage/update consumer is connected.

## Original evidence

The unchanged original load module is `re_out/fist_dat_image.bin`, SHA-256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
Addresses below are raw load-module offsets. Near code is CS zero, service CS is paragraph f69,
and DS/SS are paragraph 1c00. A service offset must be added to **f690**, not f790.

```sh
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x7c65 --stop-address=0x7c7b re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x7e29 --stop-address=0x7e77 re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x170d6 --stop-address=0x170eb re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x17745 --stop-address=0x1778a re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0xbf3c --stop-address=0xbfb1 re_out/fist_dat_image.bin
```

The first full original comparison disproved an initial mode-reset hypothesis: its first
payload difference was preserved behavior +3e, followed by preserved byte +63. The incorrect
hypothesis read physical 171d6 instead of the actual f69:7a46 at 170d6. The real complete service
writes 3 to all four weapon-panel globals 8e62/66/6a/6e and changes no actor field. Tests retain
the original preserved behavior/flags expectations after the correction; no rule is weakened.

7c65..7c7b calls 7e29 when the pending byte +92 is nonzero, decrementing it first. Otherwise
secondary flag +17 bit 80 requests the call. A pending value of one therefore still attempts
the shot on the transition to zero. No request preserves the actor and pool.

7e29 first refreshes all four weapon plates, then marks M1 component +db with 3. Station zero
returns without calling a handler while reload byte +a8 is nonzero. The station far table
at CS:7e77 maps station zero to f69:80b5, physical 17745. Its complete launch owner retains
empty/ammunition/capacity ordering, shell/muzzle admission, velocity and mechanical defaults;
see [primary launch](projectile-launch.md). A blocked request does not consume ammunition or
advance reload. Both pending and automatic requests use the same actual body.

A successful untargeted launch loads the voice byte at DGROUP 8f06, which is 12, and calls bf3c.
A failed handler calls bf77: unsigned word `(tick - failed_at) mod 65536` below 240 preserves
the shared failure timer and requests no cue; otherwise store the current tick and request 13.
The timer is shared among actors, never per-vehicle. Blocked and idle branches do not enter
this failure gate. The sound-dispatch request returned by launch remains separately 12; a voice
candidate and a sound-dispatch candidate are distinct producers, not two PCM buffers.

bf3c separately gates the candidate by selected-player state, side, sentinel/time and devices.
This fire stage exposes the candidate before those gates and does not duplicate the future
audio owner. The original oracle declares the nonselected presentation boundary using 6da2=0:
actual bf3c returns before the device call, while full handler/cooldown instructions still run.
No protected-mode/device return is fabricated.

## State and publication

`fist_fire_history` owns shared word 9fce; the mission caller supplies it alongside the current
world tick. `fist_fire_result` distinguishes request, handler dispatch, weapon-panel refresh,
launch outcome and voice candidate. An idle/blocked result has no dispatched handler; its empty
launch value does not represent a failed attempt. Byte +63 is now retained in the typed saved
vehicle owner and common state observations; this fire stage preserves it and behavior +3e.

`fist_m1_launch_source_valid` is the shared physical-source validity contract used by both the
launch and consuming fire owners. Invalid pointers, classes, stations, components, physical
slots or metadata preserve actor, pool, history and result. No registry-current guard hides an
orphaned origin. The pool's physical type remains the mission payload union tag.

The mission wrapper installs returned type-8 and type-18 values directly into their actual
physical slots. Unrelated payloads, roster and RNG remain unchanged. The caller must supply
one shared history; class/world timing and global audio consumers do not belong to the
publication function. Other transient tags are not installed by this function. Saved import
support remains explicit: unknown saved tags still reject the whole transaction rather than
inventing restored projectile state.

## Verification

The fire mode of the existing launch probe reuses the owned fixture decoder, pool serializer,
vehicle observations and launch payload writer. Requests and source bytes are freed before
firing. It serializes canonical installed shell/muzzle values, checks their complete fresh
zero/default fields, captures unrelated world bytes from the same actual object and fails any
unexpected mutation. Null/invalid direct API and world API cases preserve every owner/output.
Large verification worlds/captures use owned heap buffers. The first WASM run proved the old
stack allocation was invalid: main reserved 64,624 bytes, with a further 32,320-byte rejection
frame, exceeding the default 65,536-byte stack. It also corrupted the old launch mode's globals.
The corrected frames are 1,920 and 208 bytes; the stack setting and all assertions remain intact.

The independent golden model reuses existing initialized state, allocation, launch payload and
weapon-selection/reload models. Tests cover every pending, secondary, reload, behavior and
behavior-flag byte independently, cross pending/automatic/reload branches, expiry/retry, unsigned
cooldown wrapping, empty/capacity/optional smoke ordering, overwritten origins and explicit
repaired-exhaustion C-only errors. Full original comparisons include actual complete actor and
new 55-byte records, untouched arena payloads, pool metadata, four panel bytes and failure timer.

`original_projectile_launch_oracle.py` executes actual 7c65..7c7b and every reached near/far
return, including 7e29, its real service and station handler. The declared outer fragment ends
before joystick/manual consumer a57a; it does not claim complete class execution. The successful
voice table is pinned, actual candidate AL and complete panel writes are checked, and the original
shared cooldown executes in full. Original instructions are neither patched nor replaced.

The required corpus hashes all 47 complete FSGs and checks all 179 M1 actors with full mission
occupancy at the declared untargeted station-zero boundary. A separate reaching C flow installs
every TRAIN1 record through the actual owned loader, releases all source storage, selects station
zero/takes control, runs 320 declared phase/reload boundaries, requests fire and publishes the
canonical shell/muzzle. These are selected stage calls, not 320 complete mission ticks. Its
post-selection/reload actor and full occupancy also enter the complete original fire comparison;
unrelated complete typed TRAIN1 payloads are checked for preservation in C.

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_primary_fire.py \
  --target native --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_primary_fire.py \
  --target wasm --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_projectile_launch.py \
  --target all --originals --oracle
```

Build memory instrumentation sequentially after any previous compile has returned:

```sh
mkdir -p /tmp/wasm-fist-0076-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-sanitize-recover=all \
  -fno-omit-frame-pointer -g -O1 -Isrc -Itools/rewrite src/assets/*.c src/sim/*.c \
  tools/rewrite/probe_io.c tools/rewrite/object_pool_probe_io.c \
  tools/rewrite/vehicle_probe_io.c tools/rewrite/projectile_launch_probe.c -lm \
  -o /tmp/wasm-fist-0076-sanitizer/projectile_launch_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/rewrite/test_primary_fire.py \
  --target native --originals --native-probe /tmp/wasm-fist-0076-sanitizer/projectile_launch_probe
```

Required corpus/original/memory gates require zero skips and complete observations. Default build
gates explicitly omit their two additional original groups. Missing output, unequal traces,
incomplete processes and sanitizer errors fail. This changes no presentation/input/PCM/persistence
code. Final timings and bounded acceptance belong in WI 0076; compact review evidence belongs
in `/tmp/wasm-fist-0076-fire-review`. On 2026-10-07, all 26 native CTest gates, all 24 WASM
Python gates and both renderer probes pass in the complete production build. Strict LLVM 19.1.7
format/tidy passes all 70 owned C units. Required native/WASM original fire gates pass all seven
groups in 28.996/36.382 seconds; the memory gate passes in 11.332 seconds. Each has the complete
2,828/4,480 totals and zero skips. The old launch both-target original regression also passes
all eight required groups in 29.761 seconds, without skips.

Obsolete owned logs and memory binaries are removed after
verified commit/push. Final complete-mission WASM acceptance remains outstanding.
