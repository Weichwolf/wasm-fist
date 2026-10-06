# Ground-vehicle model selection

`src/assets/vehicle.c` provides the original immutable visual selection for ground-vehicle
snapshots, a complete model-code catalog and independent per-part facing/variant selection.
It consumes owned `fist_unit_definition` values from `units.c`; it copies the visual fields and
retains no source or snapshot views. The catalog names are static immutable strings.

This is a visual-data contract. It does not run unit initialization, recover a bearing from
world positions, install objects, remap model palettes, project sprites or draw a vehicle.
Other classes and editor wreck construction remain separate work under WI 0041.

## Catalog and default class dispatch

The original render dispatcher at DOS CS0:c4fb..c507 reads the type from the object and obtains
an even byte-offset model code from DGROUP:e48c. The pristine default table selects:

| Ground type | Model code | DOS basename |
| --- | --- | --- |
| 0 | 4 | M1_C |
| 1 | 16 | M3_C |
| 2 | 28 | T80_C |
| 3 | 40 | BMP_C |

The letters are verified filenames, rather than a guess based on the first family file. The
catalog includes all 34 even codes 0..66. Each ground family has A/B/C/D/E/DEAD in order: M1
occupies codes 0..10, M3 12..22, T80 24..34 and BMP 36..46. The remaining codes are EXPLODE (48),
APACHE (50), HIND (52), SMOKE (54), MUZZLE (56), TREES (58), GSMOKE (60), TARGETS (62), SHOT (64)
and ARTILL (66). Odd codes and values outside this complete list return NULL.

The names follow the original STR loader list: its first complete list begins at frozen DOS
image 2df4a (STR 2d74:080a), after its 8-byte header. Entries contain LOD flags, MGA filename
index, model code and loader method. Duplicate LOD entries agree on the name. The MGA service
0197 copies the indexed DOS filename from its private data segment to DGROUP:0740. The oracle
executes that service for every entry, rather than inferring names from directory order.
The MGA module image hash is 54bb7b7b61371c63d8109ac658ee9cd4ffb2cb1c3ab09fae04673a0f3b09c037;
the DOS image uses the existing `OriginalUnitOracle` version/hash pin.

## Visual snapshot fields and direction selection

All four supported types have 251-byte snapshots. The original c5fc wrapper calls c91c, which
constructs a two-part kind-1eh node through ca6b. It temporarily replaces snapshot heading +10h
with turret heading +26h, then restores +10h and appends that hull heading after the part bytes.
The source snapshot is unchanged after construction.

| Visual field | Original source |
| --- | --- |
| Primary heading | Little-endian word +26h, turret heading |
| Secondary heading | Little-endian word +10h, hull heading |
| Scale | High byte of little-endian word +14h |
| Two part selector bytes | Snapshot bytes +a9h/+aah, each XOR 80h |

The source offset a9h is the first four words of DGROUP:e4a8; the count of two is verified from
all living ground-model families decoded by WI 0056. XOR 80h reproduces the c91c node builder.
In the emitted byte, bit 7 selects the secondary heading. The remaining seven bits address the
part's variant table. Renderer instructions 2fa0..2fb5 select the heading; 2fc5..2fcc double BL
as an 8-bit value, which removes the selector bit before the variant pointer lookup.

`fist_vehicle_part_pose` returns the corresponding `fist_model_pose`; use that pose with
`fist_model_variant_get` to validate that the chosen variant exists in the loaded model.
The reader preserves every variant, including values absent from a particular model. It does
not clamp or substitute a variant. Zero scale is also retained, with no fabricated default.
Unsupported types, missing snapshots, wrong snapshot lengths and inconsistent snapshot/type
identity fail atomically. Invalid part indices and absent output/visual pointers preserve the
caller output. Heading values come from the immutable definition and its copied snapshot.

`fist_model_facing` accepts a typed heading/bearing pair. Heading and bearing use the original
65536-unit turn. It subtracts the supplied view bearing modulo one turn, rounds to the nearest
of 32 equally spaced directions and resolves half-step ties forward. Direction 31 wraps to 0
at the last midpoint. The supplied bearing belongs to the original angle convention; converting
world positions to it is outside this helper.

The oracle executes 05a9..05b6, including the subtraction and all byte/word rounding operations.
It enters after 059a's world-position bearing computation and heading pop. Every one of the
65536 relative angles is compared, with complete native/WASM output for three different
bearings. Additional node tests execute independent primary/secondary headings and the actual
renderer selector/variant instructions.

## Verification

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_vehicle.py --originals --oracle
```

The optional instruction oracle uses pinned Unicorn from `tools/rewrite/oracle_requirements.txt`;
it is not a runtime dependency. The production builds add the vehicle contract to native CTest
and the WASM Node gate. Default fixture runs explicitly skip the separately requested original
corpus. `--originals` requires complete coverage and fails if any test skips.

All six groups pass on both targets with no skips in the complete gate: all 34 catalog names,
all 170 referenced model files pinned by size/hash, all 960 ground snapshots across 47 FSGs,
all 65536 relative angles, all 256 stored selector bytes for both parts and four types, distinct
headings and bearing wrap, zero/max scale, complete source ownership, empty/unsupported sets,
malformed/missing inputs and atomic API failures. The probe overwrites and frees the FSG input, then destroys all owned unit definitions/snapshots
before observing any visual fields or part poses. Original instructions build complete nodes without hooks or
patches and verify that the entire original snapshot is restored. Original file hashes remain
unchanged. Address/undefined-behavior sanitizers with leak checking also pass all six groups and
the full corpus. This step changes no displayed frames; vehicle rasterization remains open.
