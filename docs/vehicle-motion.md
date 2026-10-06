# Ground-vehicle motion

`src/sim/vehicle_motion.c` implements the complete motion stage used by the four ground
classes, followed by manual turret slew. It consumes owned `fist_vehicle_state` from WI 0060
and returns explicit speed/hull/turret refresh events. No platform input, clock or allocation
participates. Invalid class, component size or pointers preserve state and event output.

This is the original 7cd5/88e4/912d/98c3 stage plus untargeted 7d0f/8917/90cd/9911. It does
not replace the full vehicle tick: suspension/terrain installation, collision, resolved target
aiming, input dispatch, weapon updates, damage/animation countdowns and mission scheduling
remain open. Callers supply current throttle/headings/phase and own phase advancement. The
tests advance the active-object phase by two between observations, as the original class tick
does after its movement/terrain methods. No simulation frequency is guessed here.

## Speed and hull control

The original methods call service 1a401 (speed), 1a395 (hull), then 1a1d6 (velocity) and
integrate both signed velocity words into 32-bit X/Y. Complete near/far returns and class
component markers are verified with the frozen original instructions, without hooks or patches.

Speed updates only when phase +3dh has its lowest two bits clear. At such an update, motion
flag 10h, both direction flags 06h together or a zero movement gate force throttle to zero.
Otherwise throttle is preserved. Signed pitch divided down by 512, rounded toward negative
infinity, plus 16 gives the slope index; the original clamps it to 0..31. Control mode +90h's
lowest two bits select one of four full 32-byte profiles. These pointers and bytes live in
**GS STR:2b84**, with DS:0070 holding the STR segment. Reading them from DGROUP gives wrong
limits. The C table contains the recovered complete data:

| Profile | Complete limits |
| --- | --- |
| 0 | Indices 0..16: 64; then 61,58,54,50,45,32,24,16,8,0,0,0,0,0,0 |
| 1 | Indices 0..25: 32; then 28,24,20,16,12,8 |
| 2 | All 32 indices: 64 |
| 3 | All 32 indices: 32 |

Target speed is the lesser of that unsigned limit and signed throttle divided down by four.
Actual signed speed moves one unit toward the target. Direction flags with phase bits 0ch
set delay acceleration in the current sign direction, but allow braking across zero. Any
nonzero changed speed clears control flag 10h. When speed does not change, that flag stays untouched.

The hull servo is blocked by direction flags 06h, motion flag 10h or a zero movement gate.
Otherwise the wrapped signed difference from hull heading to requested heading is limited
to 182 turn units per call. A half-turn tie goes in the negative direction. Operating flag
80h instead recenters the hull toward the relative turret offset, by up to 910 units, adding
the same step to hull/requested hull and subtracting it from both relative turret offsets.
It clears flag 80h when the offset is within that step. This path reports a hull refresh
even for zero displacement, matching the original carry-return contract.

## Rotation and position

`src/sim/rotation.c` owns reusable planar rotation 03a9. Heading zero follows +Y and quarter
turn +X. The original quarter-wave table at SS:2042 has 257 words, including its wrapped
endpoint; normal mode interpolates the two neighboring knots with the original rounding.
Temporary coarse mode SS:2040 selects the lower knot. Cardinal extrema return a full-scale
coefficient of 65536; an interpolated word of **65535 without carry** remains 65535. Treating
every 65535 result as full scale incorrectly changes near-cardinal velocities. The gate
specifically covers headings 32 and 33 with magnitude 321, where Y is respectively 321 and 320.

The independent reference reconstructs the original knot values as rounded samples of
65536*sin(index*pi/512). The instruction oracle runs complete 03a9 returns for every heading
in both math modes, signed extrema/cardinal/interpolation cases and every signed magnitude at
quarter turn. Both returned velocity lanes are observed, including the -32768 word wrap.

Drive magnitude is signed speed divided down by two. Exactly one of direction flags 02h/04h
adds -8/+8 times that magnitude to hull heading and copies it to requested heading. The rotation
then overwrites **both** velocity words. X/Y integrate those signed words with defined 32-bit
wrap; C uses wider arithmetic to avoid signed overflow. Missing Y output cannot pass tests.

## Manual turret and component refresh

After motion, manual turret slew limits the wrapped requested-relative minus actual-relative
offset to 364 units, and stores absolute turret heading as hull plus relative offset. It
restores selectors +a9h/+aah/+abh to 80h/0/0 on every call. The typed start state now preserves
all three selector bytes too; the initialization regression observes them after releasing
every source definition/snapshot.

| Class | Speed component byte | Hull/turret component byte |
| --- | --- | --- |
| M1 | +cbh | +cah |
| M3 | +c5h | +ech |
| T80 | +c9h | +c8h |
| BMP | +c4h | +ceh and +d1h |

Changed speed marks its component with 3; hull refresh or changed turret marks the class's
heading component(s) with 3. The C state addresses these within its owned complete component
payload. The full original write-footprint comparison includes every preserved byte, selector,
component marker and global scene-refresh notification. Targets are explicitly absent at this
manual-stage oracle boundary; no target solver is replaced or claimed implemented.

## Verification

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_vehicle_motion.py --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_vehicle_start.py --originals --oracle
```

The optional oracle uses pinned Unicorn and the frozen DOS image, with original STR data at
2d74. Actual complete speed/hull far returns are observed for their carry flags; the complete
class motion method then executes from the same unchanged source, followed by its manual turret
wrapper. Stack/segments and original scene-refresh output are checked. No substitute services,
instruction hooks, missing calls or modified instruction bytes are used.

Both production targets test all 256 motion flag/phase values, independent phase/gate/direction
cases, all four profiles across the signed pitch bins and all 256 profile bytes, slew and
recentering boundaries, 256-step drives to all 128 actual slope/profile caps,
1024-step sustained drives for every class, braking/reverse driving,
coordinate wrap, complete ownership, empty/malformed/missing inputs and atomic API failures.
`--originals` requires all 47 hash-pinned FSGs and 960 ground snapshots, with no skips. Default
gates explicitly skip only the separately requested original-corpus group. Every observation
follows release of the request/snapshot storage. Original file hashes must remain unchanged.

For the same native behavior under memory instrumentation:

```sh
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function \
  -fno-strict-aliasing -ffast-math -Werror -O1 -g -fsanitize=address,undefined \
  -fno-omit-frame-pointer -Isrc -Itools/rewrite \
  src/sim/random.c src/sim/rotation.c src/sim/vehicle_state.c src/sim/vehicle_motion.c \
  tools/rewrite/probe_io.c tools/rewrite/vehicle_probe_io.c \
  tools/rewrite/vehicle_motion_probe.c -o /tmp/wasm-fist-motion-sanitized
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 \
  tools/rewrite/test_vehicle_motion.py --target native \
  --native-probe /tmp/wasm-fist-motion-sanitized --originals
```

This shared simulation stage changes no displayed frame yet. Reviewed WI 0059 native/browser
captures remain the static presentation baseline; installed motion and controls will enter
the interactive scene next.
