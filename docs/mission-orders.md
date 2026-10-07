# Owned mission orders

`src/assets/orders.c` decodes complete PATH/PINF input into the shared C11
`fist_mission_orders`. The canonical mission world owns the only runtime copy; the mission
caller installs it before control/contact/model loading. No borrowed scenario view survives.
Failed loading preserves the caller session, options and initial RNG, releases the temporary
world and publishes no partial orders. Standalone inspection remains an explicitly separate
actor subset. Saved-object-only world installation leaves `orders_loaded` zero; it does not
fabricate a complete mission input. World reset clears both the flag and all retained data.

This delivers input and ownership for WI 0081. It does not implement command callbacks,
waypoint advancement, living AI, PCM or a playable battle.

## Format and original evidence

Frozen original DOS image SHA256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
The actual near loaders d87f/d8a9 read complete blocks into DS:7d40/85b6. Their eight-entry
pointer tables at 7d2a/85a0 prove the record strides. The complete unchanged DOS handlers,
read/seek register contracts, whole-DGROUP preservation and unchanged RNG are independently
verified by `original_mission_orders_oracle.py`; only DOS file I/O is supplied by the host.

| Input | Exact extent | Owned fields per platoon |
| --- | --- | --- |
| PATH | 8 × 268 = 2144 bytes | Count byte, eleven retained header bytes, all 32 signed int32 X/Y pairs beginning at +12. |
| PINF | 8 × 22 = 176 bytes | All eleven little-endian uint16 words, including unknown and unused words. |

The original editor append at 4de6..4e08 looks up the selected platoon's route, compares its
count unsigned with 20h and refuses an append when count >=32. For count <32 it addresses
`route +12 +count*8` before copying XY and incrementing count. Therefore decoded counts 0..32
are admitted, and 33..255 are rejected. Every saved coordinate slot beyond count remains data,
including an empty route's slots. Eleven other header bytes are retained without assigning
invented semantics. Both branches' actual admission/address prefix is executed for all eight
platoons and all 256 counts, stopping before the XY-copy/UI call or rejected return. This
bounded fragment proves capacity/addressing, not complete editor functionality.

Actual command consumers use the first four PINF words as behavior, waypoint mode, formation
and throttle selector. Decoder admission does not infer valid selectors from the shipped
corpus or silently clamp these words. Their full-width values survive; consumer-specific rules
belong to complete command recovery. Exact lengths, nonnull views and every route count are
validated before publication. Signed coordinate decoding uses the shared endian reader and
preserves both int32 extrema. The owned value structure has no input pointers or RNG owner.

## Verification

```sh
export PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache
bash tools/build.sh all
python3 tools/check_style.py
/tmp/wasm-fist-decoder-oracle/bin/python tests/test_orders.py --originals --oracle
/tmp/wasm-fist-decoder-oracle/bin/python tests/test_mission_driving.py --originals --oracle
/tmp/wasm-fist-decoder-oracle/bin/python tests/test_mission_world.py --originals --oracle
/tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_command_boundary.py --originals
```

The existing scenario probe's `--orders` observes every owned field after source overwrite/release;
`--orders-checks` requires API invariants without publishing a transcript. Each valid fixture
also checks all 2048 count/platoon combinations, every 2320 truncated block length, oversized
and null views, null arguments and complete unchanged outputs on failure. Constructed fixtures
cover all admitted counts, signed extrema, the complete retained-header byte domain and
full-width unknown descriptor words. Required corpus coverage pins all 47 files and compares
109040 complete order bytes per target with the actual DOS loader, without skipped inputs.

Complete canonical timed transcripts add loaded orders to every world boundary. Nonzero
constructed routes/descriptors, unused slots, input release, pause and repeated updates must
retain every field; malformed input must preserve the entire existing session/options.
All ten supported original worlds consume their real file input; all 37 unsupported worlds
still fail explicitly. Saved-object and canonical combat tests explicitly observe unloaded
orders at their declared subset boundary. Their prior payload/roster/RNG expectations are
preserved. Actual native SDL and Chromium TRAIN1 scene/device gates remain required.

Sanitizer reproduction keeps the production warning and fast-math settings:

```sh
cmake -S . -B /tmp/wasm-fist-orders-sanitized -G Ninja \
  -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug \
  '-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -g -O1'
cmake --build /tmp/wasm-fist-orders-sanitized \
  --target fist_scenario_probe fist_mission_world_probe fist_driving_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_orders.py --originals --oracle \
  --target native --native-probe /tmp/wasm-fist-orders-sanitized/fist_scenario_probe
```

Repeat the world and canonical gates with their corresponding sanitized probes. Exact terminal
acceptance and limitations are recorded in WI 0082. Artifacts stay under `/tmp`; original files
remain read-only and ignored. Complete command/world/PCM/outcome and final acceptance gates
remain open under 0081/0041/0065/0047.
