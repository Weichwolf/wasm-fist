# Spatial projectile direction

`src/sim/rotation.c` owns both planar motion and the spatial direction calculation required
by weapon launch. `fist_rotate_spatial` takes heading, elevation, signed magnitude and the
original optional coarse math mode. It returns all three signed velocity components. Heading
zero follows +Y; a quarter turn follows +X. Elevation zero is horizontal, a quarter turn +Z.
Both platforms use this C implementation and the same immutable quarter-wave coefficient data.

This is a delivered launch prerequisite within active WI 0065. The driving scene does not yet
fire, allocate projectiles, integrate their flight, resolve hits or play weapon audio.

## Original behavior

The pinned original DOS image and instruction runner are documented in
[ground motion](vehicle-motion.md). Complete routine `0459..04c7` computes the Z component
from elevation, truncates the horizontal magnitude, then rotates that magnitude into X/Y.
Interpolation/coarse selection reuse coefficient routines `05dd` and `05e2`. The horizontal
sign is retained separately from its unsigned magnitude until the final outputs.

The horizontal stage differs from planar rotation at full-scale coefficients: `0487` XOR
clears the carry returned by `05dd` before the branch at `048d`. This stage therefore multiplies
by the returned word 65535. Z and final X/Y preserve the full-scale carry and use 65536.
This is observable gameplay arithmetic, rather than an invented correction or rounding guard.

Complete original execution proves these reaching cases:

| Heading | Elevation | Magnitude | X | Y | Z |
| --- | --- | --- | --- | --- | --- |
| 0 | 0 | 853 | 0 | 852 | 0 |
| 0 | 0 | 1 | 0 | 0 | 0 |
| 8192 | 32768 | -32768 | 23169 | 23169 | 0 |
| 8192 | 8192 | 2 | 0 | 0 | 1 |
| 0 | 16384 | 853 | 0 | 0 | 853 |
| 0 | 49152 | 853 | 0 | 0 | -853 |

Using planar rotation for a horizontal shot incorrectly produces 853 rather than 852.
Keeping an untruncated real-valued horizontal magnitude also changes diagonal shots.
The production fast-math flag does not affect this shared integer calculation.

## Reaching a real weapon launch

`OriginalVehicleMotionOracle.m1_primary_shots` executes the actual complete M1 station-0
handler at physical `17745` (`f69:80b5`) from a complete initialized, untargeted actor.
It runs the real pool reset, ammunition decrement, allocation, launch, spatial rotation,
muzzle creation and event routines without instruction hooks, patches or substituted calls.
It retains complete actor and allocated object records, plus the carry result.

For a successful initialized shot, the actual handler reduces the primary count from 15 to 14,
allocates type 8 and muzzle type 18, stores projectile speed 853, sets reload countdown 20 and
recoil 16, and clears the pending trigger. The three projectile velocity words are compared
with both C targets. An empty primary store returns failure and preserves the complete actor,
without allocating an object. This evidence covers the already-eligible manual station handler;
it does not claim the class's fire eligibility, input dispatch or targeted aiming is implemented.

## Verification

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_vehicle_motion.py --originals --oracle
```

The mandatory native/WASM motion gate observes full heading and elevation turns in both math
modes, every signed magnitude, cardinal/interpolation boundaries and malformed requests. The
independent reference derives the sine knots mathematically; optional pinned original execution
confirms complete routine returns and actual M1 shot velocities. `--originals --oracle` includes
every one of the 179 M1 actors across all 47 pinned scenario files, with targets explicitly absent
at the tested manual handler boundary. Missing original files or incomplete results fail.

The sanitizer gate compiles the actual rotation/state/motion and probe owners with production
flags, then runs complete native behavior tests and the original corpus:

```sh
clang -std=c11 -O2 -g -Wall -Wextra -Wpedantic \
  -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math \
  -fsanitize=address,undefined -fno-sanitize-recover=all \
  -Isrc -Itools/rewrite \
  tools/rewrite/vehicle_motion_probe.c tools/rewrite/probe_io.c \
  tools/rewrite/vehicle_probe_io.c src/sim/rotation.c src/sim/vehicle_state.c \
  src/sim/vehicle_motion.c src/sim/random.c \
  -o /tmp/wasm-fist-0065-sanitized-probe
ASAN_OPTIONS=detect_leaks=1 PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  python3 tools/rewrite/test_vehicle_motion.py --target native --originals \
  --native-probe /tmp/wasm-fist-0065-sanitized-probe
```
