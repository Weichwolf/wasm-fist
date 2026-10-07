# Shared ground contact

`src/sim/ground.c` supplies installed numerical-height sampling and ground contact for the
four ground vehicle classes. `fist_ground_sample()` returns height, roll and pitch for a
map position and heading. `fist_vehicle_ground_update()` publishes those fields separately
for the hull and absolute turret. Both are shared C11, without original engine code at runtime.

`src/sim/world.h` owns map/turn constants shared by simulation and rendering. Original map
positions are shifted by 13 into 32-bit sampler coordinates, so the map repeats every 524288
position units. Render positions divide by 256; headings cover 65536 units per turn.

## Installed field and four-point footprint

The caller supplies a complete owned square power-of-two numerical height image, resampled
with `fist_heightfield_resample()` to its selected installed dimension. The query borrows it
only for the call. It accepts up to 16 index bits per axis, subject to `size_t` plane-size
representability; the two original SHLD index contributions share 32 bits. Actual original
map sizes 512, 1024, 2048 and 4096 are all verified. A one-cell field returns its constant
height and zero slopes. Missing pixels, zero/non-square/non-power-of-two/overflow dimensions
and null arguments fail without publishing output.

Kernel `1109..11a4` forms `x = map_x << 13`, `y = -(map_y << 13)` with 32-bit wrapping.
The height kernel `8480..848d` uses the top detail bits of y as the row, then the top detail
bits of x as the column. This is an installed height lookup, separate from the inspection
renderer’s exact triangle interpolation.

Kernel `7fa0..801c` selects the original Q31 sine table at `9450`. The index is
`((-heading) modulo 65536) / 128`; cosine is 128 entries later. Both coefficients are
arithmetically shifted right by six to get the fixed sampling offsets. The original tail
repeats the first 128 sine values. Shared C keeps all 512 resulting offsets. The original
Q31 table has small asymmetries and cannot be replaced with a symmetric quarter-wave table.
Its complete 640-word SHA256 is recorded in the C source and independent `ground_basis.json`.

For center (x,y), sine s and cosine c, the four samples are:

| Contact axis | First sample | Second sample |
| --- | --- | --- |
| Roll | (x-c, y+s) | (x+c, y-s) |
| Pitch | (x-s, y-c) | (x+s, y+c) |

Subtract heights in an 8-bit lane, sign-extend the wrapped difference and multiply by 128.
This reproduces the high word of the original signed-byte difference shifted left by 23.
Slopes range from -16384 to 16256 signed turn units. A difference of 255 wraps to -1 before
scaling. No floating atan, clamping, artificial terrain normal or guessed sample distance is
used to determine gameplay slope.

Cardinal offsets are +33554431 and -33554432, not an exactly symmetric +/-4096 world-unit
footprint. On the asymmetric 512-square fixture at (0,0), complete original returns are
height 0, roll 14848 and pitch 512. A symmetric footprint instead gives roll 12288 and pitch
-4096; those pitch values reach different speed-profile bins. The fixed baseline is included
in default tests, and the full original oracle proves it alongside all headings/boundaries.

## Vehicle state and phase boundary

The protected-mode op 1c pass walks 32 near pointers, skipping null/unsupported units. For
a ground unit, it samples hull heading +26h and absolute turret heading +10h independently.
It writes only:

| Original field | Shared typed owner |
| --- | --- |
| +1dh byte | `vehicle.ground_height` |
| +32h word | `vehicle.drive.terrain_roll` |
| +34h word | `vehicle.drive.terrain_pitch` |
| +22h word | `vehicle.turret.terrain_roll` |
| +24h word | `vehicle.turret.terrain_pitch` |

The per-vehicle C API rejects unsupported classes or wrong component sizes and preserves the
whole state on failure. The future roster/controller owner chooses participating vehicles;
this helper does not invent a different global roster. Initialization preserves these added
fields from the definition, just as complete original c296/class initialization does.

Contact publication preserves altitude. Original ground class entries begin by copying byte
+1dh into altitude byte +0dh before their motion stage (for example 7c1d..7c23). This does not
replace all four altitude bytes. The mission/update functions 4308/4330/4354 queue op 04, 10,
1c and 28, then flush through 1664 after their class dispatch. The owned contact helper advances
no phase/clock, transfers no altitude and replaces no complete class tick. Class altitude
transfer, exact mission/control ordering and interactive integration are subsequent work.

## Verification

```sh
bash tools/build.sh all
python3 tools/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_ground.py --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_vehicle_start.py --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_vehicle_motion.py --originals --oracle
```

Default gates test every heading value, all 65536 byte pairs in the roll footprint, signed
byte differences, cell/seam boundaries, extreme/wrapped 32-bit coordinates, tiny/constant
fields, independent hull/turret headings, all four classes and complete state preservation.
The request is overwritten and freed before sampling/contact; the height image is destroyed
before observations. Public invalid field/class/component cases publish nothing. Full shared
state serialization has one owner and includes the added ground fields in initialization and
motion regressions.

`--oracle` executes complete 7fa0/8480 and op 1c returns, checking the stack and complete 251-byte
original actor results. It executes actual map-loader stores at 8aa7..8b0b to install sampler
immediates. No hooks, replacement calls or externally patched instructions are used; these are
the original self-modifying setup instructions. Direct query results and every preserved byte
of original actors must agree with the independent fixed-data reference and both C targets.

`--originals` requires all 47 pinned FSGs and eight pinned Dxx height maps. Original KLC and
height-resampler routines provide installed fields at each of the four runtime sizes. The
gate covers all 960 ground snapshots at every size, plus all 512 heading bins on every field:
3840 complete contacts and 16384 direct samples per target. Missing files/outputs, changed
hashes, incomplete runs or requested skipped groups fail. All originals remain read-only.

For native memory instrumentation:

```sh
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function \
  -fno-strict-aliasing -ffast-math -Werror -O1 -g -fsanitize=address,undefined \
  -fno-omit-frame-pointer -Isrc -Itests \
  src/sim/ground.c src/sim/vehicle_state.c src/sim/random.c src/assets/klc.c \
  tests/probe_io.c tests/vehicle_probe_io.c tests/ground_probe.c \
  -o /tmp/wasm-fist-0063-ground-sanitized
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_ground.py --target native \
  --native-probe /tmp/wasm-fist-0063-ground-sanitized --originals
```

This simulation step changes no displayed frames. WI 0059 remains the reviewed static native/
browser baseline. It does not implement collision, the full suspension/class tick, mission
clock/controllers, live rendering, combat, HUD or audio; the next step connects this owned
state and shared motion to the first controlled moving scene.
