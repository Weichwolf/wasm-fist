# Ground movement maintenance

The shared `fist_vehicle_state` owns the saved movement word at +5d and speed-threshold counter
at +5f. Restoration copies both; original participating initialization resets the movement
word to 65535 and retains the counter. The existing motion owner uses a zero movement word
to suppress hull slewing and clear throttle on its admitted speed step. Neither field is
assigned a physical fuel or temperature unit without evidence from its other consumers.

`fist_vehicle_maintenance_phase` consumes the current phase without advancing it. The shared
driver calls it after phase progression, alongside the existing reload and history owners,
before ground contact. Invalid class or component extent leaves the whole actor unchanged;
driver/session failures retain their existing transactional actor, feedback, clock and input
contracts. The current driving subset preserves all nonplayer payloads and installation RNG.

## Original evidence

Frozen DOS image SHA256 `d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
The actual index instructions of every ground class use phase & 1eh. The maintenance bank
entry and complete wrapper differ by class:

| Class | Bank byte index | Near wrapper | Raw component bytes, in write order | Owned component indices |
| --- | --- | --- | --- | --- |
| M1 | 10 | 7cbf | cc, cd | 13, 14 |
| M3 | 10 | 88ce | c6, c7 | 10, 11 |
| T80 | 16 | 9160 | f1, f0 | 51, 50 |
| BMP | 20 | 98fb | f8, f7 | 60, 59 |

Each wrapper calls f69:a96c, raw 19ffc..1a02c. The service takes the absolute signed speed
word using word NEG, then unsigned SHR by four. The 8000h case retains magnitude 32768.
Subtracting this amount from +5d saturates to zero only on actual unsigned carry. For signed
speed >=60, +5f increments only when below 248; saved values 248..255 remain unchanged.
For other signed speeds, a nonzero counter decrements. Reverse speed contributes to movement
consumption but takes the decreasing counter branch. There is no counter overflow or invented
speed clamp. After the service, phase & e0h ==0 marks the two class components as 3; higher
phase bits suppress only those writes, never the movement/counter update.

The reaching TRAIN1 observation installs all 85 original records, takes control of physical
player 151 and supplies a declared flat-contact field. Before this change, the actual M1
7c1d..7c7e prefix first differed at tick five, phase 42: original +5d was 65533 while the manual
subset kept 65535. After maintenance, complete 251-byte actor records agree through six
updates, including constructed movement/counter boundaries (65535,0), (1,1) and (0,255).
Every other payload and the final file-order initialization RNG remain unchanged. This prefix
stops before engine PCM; it is not a complete class or mission return.

The next first difference is tick seven, phase 46, at ab03: heading-history words +28/+2a/+2c,
average +2e and byte +42 change, and actual RNG 0291 advances stream cursor 2 to 3. Its nested
controlled/AI callback and complete caller contract remain required under WI 0081. No callback
is substituted with a return. Observation hooks flush Unicorn's existing translation cache
after installation so reused constructor/RNG code is observed; guest instructions remain
unchanged. Reserved address-zero/one return entries explicitly fail the prefix gate.

## Verification

```sh
bash tools/build.sh all
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_vehicle_maintenance.py \
  --target all --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_vehicle_history.py \
  --target all --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_mission_driving.py \
  --target all --originals --oracle
```

The existing motion probe's maintenance mode restores owned state and frees input bytes before
updates. It checks every unrelated typed byte and padding; complete serialized expectations
also compare every component. The independent oracle executes actual class index instructions
and complete near/far wrappers, comparing all 251 raw bytes. Every signed speed word, every
counter/phase byte, all subtraction boundaries for consumption 0..2048, repeated exhaustion,
counter limits, exact component writes and all 960 saved ground snapshots are required.
Per target: seven groups, 321424 fixtures and 328128 complete boundaries. Requested originals
fail on absent files, hash differences or skipped coverage. History shares the protocol and
reaching-world owner without weakening its separate five-group contract.

Canonical timed regressions add 16 valid participating starts across all four classes, two
speed branches and two counter ranges, with both maintenance calls and paused-time preservation.
The constructor's real movement-word reset is required; clearing participation also removes
the actor from the roster and cannot establish a controlled saved-player path. Complete
canonical state/world streams and byte lengths, original manual-stage returns and unchanged
nonplayer/RNG state are compared. The standalone 47-player/eight-map gate remains separate.

ASan/UBSan/LSan reproduction keeps production fast-math flags:

```sh
cmake -S . -B /tmp/wasm-fist-maintenance-sanitized -G Ninja \
  -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug \
  '-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -g -O1'
cmake --build /tmp/wasm-fist-maintenance-sanitized \
  --target fist_vehicle_motion_probe fist_driving_probe fist_driving_preview
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_vehicle_maintenance.py \
  --target native --originals --oracle \
  --native-probe /tmp/wasm-fist-maintenance-sanitized/fist_vehicle_motion_probe
```

Repeat the history, standalone and canonical driving gates with their corresponding sanitized
probe. Actual native SDL and Chromium TRAIN1 device/frame checks use isolated provisioned
assets as described in [driving-scene.md](driving-scene.md). Exact acceptance belongs to WI 0080;
temporary outputs stay in `/tmp` and original files remain read-only. Complete living dispatch,
world battle presentation, audible PCM, objectives/outcomes and the final ten-run gate remain
open under 0041/0065/0047.
