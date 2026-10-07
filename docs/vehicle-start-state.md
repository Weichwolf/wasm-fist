# Ground-vehicle start state

`src/sim/vehicle_state.c` initializes owned typed state for the four original ground classes.
`fist_vehicle_initialize` borrows an immutable `fist_unit_definition`, takes explicit four-stream
random state and a link-mode byte, and publishes vehicle/random output together after success.
It allocates nothing and retains no snapshot pointers. Invalid pointers, type/snapshot identity,
length or random cursor fail without modifying either output. The caller's decoded identity and
32-bit position/altitude remain authoritative; original saved pool addresses are not runtime IDs.

This is the c296 initialization stage. Terrain/suspension installation, target references,
controller ticks, damage/animation semantics, save-state restoration and mission selection of
the seed/link configuration remain open. An initialized vehicle is not yet a playable mission.
Fields not modeled in this stage remain available through the immutable asset definition;
this API does not pretend to produce a complete restored 251-byte runtime object.

## Original execution and field contract

The frozen original DOS image is pinned by `OriginalUnitOracle`. Original CS0:c296 calls 0291
twice, sets shared controls and dispatches through DGROUP:e4e0 to 7b91/8744/8f9f/973a for types
0/1/2/3. The optional oracle executes this entire routine, class call, 1e27 template copy and
RET to the supplied return address. It uses the actual DS/SS=1c00 and checks restored segments,
DI and stack. There are no instruction hooks, patches or substitute calls.

| Typed field | Original source or initialization |
| --- | --- |
| Hull heading / requested heading | Preserved words +26h / +30h |
| Absolute turret heading / relative offset | Preserved words +10h / +89h |
| Requested turret offset | Word +8bh = 0 |
| Speed / throttle / terrain pitch | Preserved signed words +55h / +57h / +34h |
| Velocity X/Y | Preserved signed words +59h / +5bh |
| Motion flags / update phase | Preserved bytes +19h / +3dh |
| Movement gate | Word +5dh = ffffh |
| Projection extent / scale | Words +12h = 40 / +14h = 1024 |
| Control flags | Word +40h OR 1 |
| Object flags | Byte +16h OR 66h, then clear bit 3 for types 0/1 or set it for 2/3 |
| Secondary flags | Byte +17h OR 34h |
| Operating flags | Byte +1ah = 1, OR 10h only when DGROUP:6dae = 2 |
| Random phases | Low bytes of successive 0291 results, at +6dh / +42h |
| Control mode / turret-view mode / hull-view mode | Bytes +90h = 0 / +86h = 1 / +8dh = 1 |
| Behavior / reload countdown | Bytes +3eh = 0 / +a8h = 0 |
| Animation selectors | Preserved bytes +a9h/+aah/+abh, added for WI 0061 motion updates |

Original 7d0f/8917/9911 set +10h = +26h + +89h. The oracle additionally checks
27 complete wrapper returns with distinct directions, relative offsets and turn wrap. This establishes the independent hull and
turret meanings and corrects the reversed labels in WI 0057. Numeric visual selection was
already correct. Original service 1a84c uses +86h for the turret view and +87h as the height
added to altitude; 1a80b uses +8dh for the hull view. These are not vehicle speed limits.

| Type / class | Four initialized weapon slots | Class parameter | Cycle bytes / ready stock | Camera height | Components |
| --- | --- | --- | --- | --- | --- |
| 0 / M1 | 15, 20, 2000, 5 at +adh | Byte +b5h = 20 | No initialization in this class | 2048 | 57 bytes at +bfh |
| 1 / M3 | 2, 500, 400, 2000 at +adh | Byte +fah = 20 | +b5h/+b6h = 1/1, +bbh = 10 | 2560 | 62 bytes at +bch |
| 2 / T80 | 15, 16, 5, 2000 at +ach | Word +b4h = 20 | No initialization in this class | 2048 | 59 bytes at +beh |
| 3 / BMP | 300, 400, 4, 2000 at +adh | Byte +f9h = 20 | +b5h/+b6h = 1/1, +bbh = 12 | 1920 | 61 bytes at +bch |

Camera height is the original word +87h. Weapon slots retain original class order; their firing
identities and the meaning of the trailing parameter require the firing rules. The typed cycle
and ready-stock fields are zero for classes without those initialized fields; this does not assert
that overlapping physical bytes in those classes were zeroed. No reload-period guess is made
from the common parameter value 20.

The full component payloads come from descriptors 2c7f/2c9f/2cc2/2ce4, pointing to segments
2d59/2d5d/2d61/2d65. Original 1e27 copies every byte, including odd trailing lengths. The C
state owns all 57/62/59/61 bytes. Individual component damage and animation behavior will be
decoded before use. The oracle compares complete 251-byte original results with an independent
expected write footprint, including preserved bytes outside the currently typed fields.

## Deterministic random state

`fist_vehicle_component_size` exposes the one owned class-size contract for motion-state
validation; unsupported types return zero. WI 0061 also moves probe output into shared
`vehicle_probe_io.c` and observes all three preserved animation selector bytes. The full
original initialization/ownership gate remains required after these additions.

`src/sim/random.c` implements complete original 0291 stepping. Four caller-provided 16-bit
LFSR words are visited in round-robin order. Advance the selected word by a right shift,
XOR b400h when the old low bit was set, store the new word and return that word minus one
modulo 65536. Zero seeds are valid and return ffffh while preserving zero words. The cursor
advances even with zero state. No platform clock or host RNG participates.

The original uses **SS**:1f82 for its last-used address and SS:1f84/86/88/8a for the words. The
typed state instead stores the next stream index 0..3. Conversion belongs only to the oracle.
Original AX and its stored result SS:0342 must agree after every complete return. Two draws per
ground initialization advance the cursor by two; unsupported non-ground records consume none.
Mission boot seeding and random use by later simulation methods remain separate work.

## Reproduction

```sh
bash tools/build.sh all
python3 tools/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  tests/test_vehicle_start.py --originals --oracle
```

The optional oracle uses the existing pinned `oracle_requirements.txt`; Unicorn is not linked
into the game. Default build tests cover constructed behavior and explicitly skip the requested
original-corpus group. `--originals` requires all 47 hash-pinned FSGs and fails on any skip.
Both targets observe state only after the probe frees FSG and owned definition/snapshot storage.

The gate sweeps every 16-bit input word in each of four streams, verifies three complete
65536-draw sequences including zero seeds/cursor wrap, all 256 object/secondary flag bytes in
all four classes, independent preserved hull/turret/motion fields, every link-mode byte, empty
and mixed non-ground sets, invalid requests/files/API calls, and all 960 original ground snapshots.
Direct API checks reject every unsupported 16-bit type and invalid snapshot length, preserving
the complete output bytes and all random fields. Production fast-math builds exercise the same C.

This state-only change requires no new displayed frame. The reviewed vehicle scene from WI 0059
remains the static presentation baseline; runtime state will enter rendering with interactive
movement/control integration.
