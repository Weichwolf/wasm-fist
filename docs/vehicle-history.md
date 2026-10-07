# Ground position history

The canonical `fist_vehicle_state` owns six position word pairs, newest first. Restoration copies
all saved pairs at +6e..+85 without interpreting their bits as signed coordinates. Initialization
retains them and seeds the sampling counter through the existing original random-byte owner
`random_phases[0]` (+6d). There is no second timer or retained raw-snapshot alias.

`fist_vehicle_history_phase` consumes the current phase without advancing it. All four original
class index sequences select the same aa37 callback when phase & 1eh == 6. The shared driver
calls this owner after its existing phase increment; reload and contact keep their existing
owners. Other phase-bank callbacks, behavior updates, inter-object contacts, world scheduling
and engine PCM remain required before a complete living-class or playable-mission claim.

## Original evidence

Frozen DOS image SHA256 `d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
Near wrapper aa37 calls f69:b038, raw instructions 1a6c8..1a721. The method increments +6d as
an unsigned byte. After wrap, values below 12 return immediately; other values reset to zero
and shift the six pairs in descending order. The new pair contains words read at +5 and +9,
precisely bits 8..23 of the current 32-bit X/Y coordinates. C casts through uint32_t before
shifting and retains the resulting uint16_t words. No world-position sign or boundary clamp
changes those lanes. Initialized counter 255 wraps to zero without shifting; counter 11
inserts on its next admitted callback.

The reaching TRAIN1 test installs all 85 records through actual original constructors and
loader initialization, takes control of physical slot 151 and installs declared flat contact.
It executes the real M1 prefix 7c1d..7c7e for three updates, stopping before the engine-audio
PM service. Phase 38 reaches the history callback. Both the actual initialized counter and
an explicitly constructed counter-11 input produce complete raw actor agreement with the
independent manual-stage expectation. Other payloads and installation RNG remain unchanged.
This proves the previously missing counter transition and the reaching sample write.

Stopping before audio is a declared device boundary. An exploratory full call with no PM
sound descriptor reached reserved address-zero return scaffolding, so it cannot establish
full-class completion. The reaching test observes and rejects reserved address-zero/one
entries. It does not replace original instructions or driver results.

## Verification

```sh
bash tools/rewrite/build.sh all
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/rewrite/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_vehicle_history.py \
  --target all --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_driving.py \
  --target all --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_mission_driving.py \
  --target all --originals --oracle
```

The existing motion probe's history mode restores owned state, frees source bytes before
updates and compares every output boundary. A byte-preservation check rejects changes to any
other typed field or padding. Atomic invalid-type/component/null and malformed-batch cases
publish no partial output. The original oracle executes actual class index instructions and
complete near/far callback returns, comparing complete 251-byte snapshots including unmodeled
bytes. All counter/phase bytes, four classes, coordinate lane extrema, repeated six-sample
windows and all 960 original ground snapshots are required; original corpus requests fail on
missing files, hash differences or skipped groups. Native and WASM use identical shared C.

A growing world exposed nested world-size automatic snapshots in the mission-world probe.
The emitted WASM main/invalid frames reserved 37888 + 37264 bytes; the failed gate trapped in
`invalid` before any mission work. One heap snapshot now serves initialization, invalid-input
and tree preservation checks, retaining every compared byte and freeing it on every exit.
The corresponding emitted frames reserve 656 + 16 bytes. Runtime stack limits are unchanged.

Sanitizer reproduction uses the production warning/fast-math configuration:

```sh
cmake -S . -B /tmp/wasm-fist-history-sanitized -G Ninja \
  -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug \
  '-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -g -O1'
cmake --build /tmp/wasm-fist-history-sanitized \
  --target fist_vehicle_motion_probe fist_driving_probe fist_mission_world_probe
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_vehicle_history.py \
  --target native --originals --oracle \
  --native-probe /tmp/wasm-fist-history-sanitized/fist_vehicle_motion_probe
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_mission_world.py \
  --target native --originals --oracle \
  --native-probe /tmp/wasm-fist-history-sanitized/fist_mission_world_probe
```

Exact accepted results are recorded in WI 0079. Temporary binaries,
requests and logs stay under `/tmp`; original scenario hashes remain unchanged.
