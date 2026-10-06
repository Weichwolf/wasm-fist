# Shared ground-vehicle weapon control

`src/sim/weapon_control.c` owns the recovered station selection, pending fire command,
gun/recoil prefix and reload phase for all four ground classes. It consumes the typed actor
from `vehicle_state.c`, preserving all unrelated fields and ammunition. Native and WASM use
the same implementation. These methods are not yet connected to the driving session; firing
eligibility, live projectiles/hits, ready-rack replenishment and audible playback remain open
within WI 0065/0041.

Initialization now preserves original selected station `+91`, loaded station `+a5`, pending
trigger `+92`, recoil `+3c`, gun elevation word `+38` and authored elevation byte `+a7`.
They are copied from the source snapshot, rather than assigned guessed start defaults.

## Selection

The original station codes are even bytes. M1/M3/BMP have 0, 2, 4, 6; T80 also has 8.
Station 8 reads the separate initialized word `+b4` as its ammunition. The four ordinary
counts keep their original slot order; weapon names are not inferred from ammunition sizes.

| Class | Continuous station | Switch countdowns, in station order | Reload completion marker |
| --- | --- | --- | --- |
| M1 | 4 | 20, 20, 0, 20 | `+db` when the selected store is nonempty |
| M3 | 6 | 2, 2, 2, 0 | `+cd` |
| T80 | 6 | 20, 20, 40, 0, 8 | `+cf` when the selected store is nonempty |
| BMP | 6 | 2, 2, 2, 0 | `+d7` |

Re-selecting the selected station changes nothing. Changing to the continuous station preserves
the loaded station and countdown. Returning to the loaded station also preserves its countdown.
Other switches update the loaded station, assign the class's actual countdown and request its
recovered voice cue. Every changed selection marks the class's complete set of HUD components
with 3 and emits a station-change event representing the original global HUD refresh.

M1 switching to an empty store assigns countdown 255 and cue 13. M3 station 0 and BMP station 4
keep their ordinary countdown even when empty: no reserve stock requests cue 13; available
stock requests notice 25 for 90 ticks, alongside the normal selection cue. Selection does not
replenish that stock. T80's setter does not apply either empty-store policy.

Cycling traverses 0→2→4→6→0 on every class, as the actual class cycle methods do. It leaves
T80's separately selectable station 8 out of the cycle. Selecting station 8 then cycling goes
to 0. Invalid arguments preserve the complete typed state and event output.

## Timing and requests

`fist_weapon_request_fire` implements command `a286`: set pending trigger to 48, preserving
ammunition and countdown. This does not consume the request or claim a projectile has fired.
The subsequent class fire gates and allocation/launch/flight stages remain explicit work.

`fist_weapon_begin_tick` implements the class-entry elevation/recoil fragment before motion:
copy the gun elevation's high byte into its authored pose byte and decrement nonzero recoil.
The caller owns phase advancement. `fist_weapon_reload_phase` runs the actual countdown method
only when the current phase masked with `1e` is zero. With the original phase increment of two,
this occurs once per 16 ticks; switch countdown 20 expires after 20 such dispatches.
Odd starting phases retain their low bit and have the same dispatch spacing.

Expiry emits a timer-expired event. M1/T80 mark their completion component and request the
selected ready cue only when its store is nonempty; M3/BMP mark completion regardless of
ammunition. A ready cue of 255 requests no voice. Timer expiry does not create ammunition.
Voice/notice IDs are boundary requests. Original selected-player/side/mute/time gates and
actual sample playback require their later shared owners; these events do not imply sound.

## Original evidence and verification

The pinned DOS image/instruction environment are described in [ground motion](vehicle-motion.md).
Original setters are physical `7963`, `17fbc`, `18b7c`, `1964e`; reload methods are `7d69`,
`8971`, `917d`, `997d`. Class prefixes start at `7c1d`, `87df`, `902c`, `97d5`.
Actual catalog types identify M1, M3, T80 and BMP; historical patch names are not authoritative.

`original_weapon_control_oracle.py` executes complete setters/cycle/request/reload returns and
the complete declared elevation/recoil fragment without instruction hooks or replacements.
It also executes the original phase increment/index instructions for every tested phase;
other phase callbacks remain outside this weapon subset. Actual original mute gates suppress
playback; selected-player notices and global HUD writes still execute and are observed.
The oracle verifies the complete original countdown/cue tables at their DGROUP locations.

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_weapon_control.py --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_vehicle_start.py --originals --oracle
```

Tests cover all class station/loaded/selected/stock/empty boundaries, every countdown and phase
byte, every signed elevation and recoil byte, timed reload/recoil sequences, every pending
trigger byte, malformed/invalid requests and all 960 original ground actors across 47 missions.
Complete 251-byte original states, complete component payloads, HUD refresh bytes and notices
are observed. Source/request buffers are released before C observations. Invalid station bytes
are exhaustively checked on each class inside both probes, without a process per byte.
Missing originals, partial runs, unequal lengths, absent output and sanitizer errors fail.

For the production-flags sanitizer gate:

```sh
clang -std=c11 -O2 -g -Wall -Wextra -Wpedantic \
  -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math \
  -fsanitize=address,undefined -fno-sanitize-recover=all \
  -Isrc -Itools/rewrite \
  tools/rewrite/weapon_control_probe.c tools/rewrite/probe_io.c \
  tools/rewrite/vehicle_probe_io.c src/sim/weapon_control.c \
  src/sim/vehicle_state.c src/sim/random.c \
  -o /tmp/wasm-fist-0065-controls-sanitized-probe
ASAN_OPTIONS=detect_leaks=1 PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  python3 tools/rewrite/test_weapon_control.py --target native --originals \
  --native-probe /tmp/wasm-fist-0065-controls-sanitized-probe
```
