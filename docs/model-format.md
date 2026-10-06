# Directional model sprite data

The shared C11 loader in `src/assets/model.c` owns a complete original model family: DAC6 palette
`.MAL`, base atlas `.M00`, and directional streams `.M08`, `.M16`, `.M32`. These are composed
sprite assets, not triangle/vertex meshes. Runtime unit-to-model catalog selection, mission
palette translation, sprite placement/rendering and vehicle simulation are subsequent contracts.

The source callback may reuse/free its view at the next read. Each file copies all input bytes;
record, pixel, part/variant and piece views borrow that owned storage. The family owns every file
and its complete 32-direction record map. Failed decoding/loading/getters leave outputs unchanged.
Destroy resets all state and is idempotent. Original game files remain read-only and ignored.

## Original evidence

The frozen DOS image is `re_out/fist_dat_image.bin`, SHA-256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5` at reference `349ad31`.
Addresses below are raw load-module offsets, CS zero. DS is DGROUP paragraph `0x1c00`.
Frozen patches 471 and 573 locate the relevant routines; original instructions were rechecked.

```sh
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x183f --stop-address=0x1bd9 re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M intel,addr16,data16 \
  --start-address=0x2f91 --stop-address=0x309b re_out/fist_dat_image.bin
```

`183f` loads MAL/M00/M08, then M16 and M32 at increasing directional quality. `1a45` streams
16-byte record headers followed by their declared remaining bytes. A final zero uint16 is EOF;
its DOS read can return only two bytes. The new decoder requires exact termination, no incomplete
record, and no trailing bytes. An EOF-only stream is representable; a complete family requires
one base record and nonempty directional streams.

| Header byte offset | Field |
| --- | --- |
| `00` | uint16 complete record byte length, including header. |
| `02`, `04` | uint16 two facing byte offsets, or first field `fffe` for the base atlas. |
| `06`, `08` | uint16 offsets to the two part tables. |
| `0a`, `0c` | Retained raw; no common semantic claim. |
| `0e` | uint16 offset to the sprite directory. |

For directional records, faces must be even offsets 0..62. `1a91..1ab3` rotates each by -32
modulo 64. The C API exposes the resulting direction ordinal 0..31. Base marker `fffe` skips
rotation; original code copies it into the second facing field regardless of the saved value.
The base has no directional part tables; unused header words may contain stale values and remain
uninterpreted. For example M1_A.M00 is an empty atlas, length 20 followed by the EOF word.

## Sprite directory and part tables

Each sprite directory descriptor is uint16 pixel offset, uint8 width, uint8 height. The first
pixel offset gives the end of the directory, whose final descriptor is `(record length, 0, 0)`.
The new reader validates this sentinel, four-byte descriptor alignment, monotonic bounded
pixel offsets, and exactly `width*height` bytes between adjacent offsets. Texels retain original
palette indices, including zero; `.MAL` is exactly 768 DAC components, each 0..63.

`1b91..1b99` reads the directory's first pixel offset as the retained metadata extent.
Actual sprite selection at `302d..3045` computes descriptor offset `reference*4`, loads dimensions
into CX (CL width/CH height), and adds the uploaded-record address to its pixel offset. The kernel
entry `ad1e` in `re_out/fist_image.bin` multiplies CL by CH to obtain the complete texel extent.
No compressed pixel codec or vertex positions are invented by the model reader.

The part count is `(first variant-table offset - part-table offset)/2`, the exact `1b35..1b45`
calculation. A part table contains one uint16 variant-table offset per part. Variant tables are
packed between this part table and the first list; their physical extents end at the next higher
variant-table offset or first list, with whole uint16 entries. Each entry points to a list:

- uint8 piece count, uint8 unsigned drawing priority;
- that many four-byte pieces: uint16 sprite reference, int8 X offset, int8 Y offset.

`2fb7..2fd4` fetches the table, per-part variant and priority/count. The node's variant byte
uses bit 7 for a separate facing and doubles the low bits in BL, giving at most 128 variants
per part. Larger tables fail validation; tests exercise index 127 and reject 129 entries.
All part counts agree across both orientations and all directional records in a loaded family.
Every list and descriptor reference is bounded before the model becomes visible to callers.

The frozen DGROUP `3a8c` selectors are +8 for directions 0..15 and +6 for 16..31. C chooses the
corresponding part table. Lists expose priority unchanged; final drawing order/animation/turret
pose selection are separate runtime work. Local/base atlas choice and mirroring come from the
reference word: low 14 bits sprite index, bit 14 selects M00, bit 15 mirrors. `301d..3045` and
`307b..3099` independently confirm these fields. Both local and base indices are validated against
the actual atlas counts. Signed offsets preserve every bit, including -128 and 127; the renderer's
final anchor/projection convention is not claimed here (kernel `ad47` negates DH).

## Complete family direction map

The loader reads M00, M08, M16, M32 and assigns each directional record's two normalized facing
slots in order. Later assignments replace earlier entries. A full family must populate all
32 slots; missing data cannot produce a default or fabricated model. The original default/neighbor
fill routines `197b`, `1996`, `19b9` are executed by the oracle. With all three quality streams
loaded, the final original map agrees with the complete C map. Runtime reduced-detail loading,
resource residency and quality settings are not implemented by this full-data loader.

All 34 original families have five files, 612 records (34 base, 170 M08, 136 M16, 272 M32),
1,679,860 texels and 1,761,940 complete input bytes. This includes tank variants, destroyed models,
helicopters, trees, targets, smoke, explosions, muzzle flashes and shots. Names, sizes and SHA-256
are pinned in `tools/rewrite/model_originals.json`; no original game contents are tracked.

## Verification

```sh
export PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
python3 tools/rewrite/test_models.py --originals
# Optional independent original-instruction gate; missing requirements fail:
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_models.py --originals --oracle
```

The optional `/tmp` virtualenv uses `tools/rewrite/oracle_requirements.txt` (Unicorn 2.1.4).
The native/WASM model probe observes complete raw records, palette and every sprite texel after
input overwrite/free or source destruction. It exercises every part variant/piece and direction
map, getter/null/failed-output contracts and repeated destruction. The six test groups cover
shared part tables, multiple variants, signed extrema, empty piece lists, direction replacement,
every truncated prefix, invalid record/facing/table/list/atlas/dimension/reference data,
incomplete directions, missing required files and invalid DOS model basenames.

`original_model_oracle.py` reuses the pinned DOS-image/executor checks and executes actual facing
rotation/descriptor assignment, original load-order neighbor fills, part count/list fetches and
sprite selection. It checks every sprite descriptor and complete texel output, including those
not used by a particular list, plus every variant's priority, local/base choice, mirror flag and
signed placement. Uploaded-record addresses and buffered input are provided at original I/O
boundaries; no hooks or patched instructions replace logic. DOS, allocator/resource relocation,
mission palette mapping, projection, final sorting/rasterization and gameplay are outside this
oracle's scope. The new runtime never links or executes frozen engine code.

ASan/UBSan reproduction with production compiler flags:

```sh
mkdir -p /tmp/wasm-fist-model-sanitizer
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Werror \
  -fno-strict-aliasing -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Isrc -Itools/rewrite src/assets/model.c src/assets/palette.c tools/rewrite/probe_io.c \
  tools/rewrite/probe_source.c tools/rewrite/model_probe.c \
  -o /tmp/wasm-fist-model-sanitizer/model_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  python3 tools/rewrite/test_models.py --originals --target native \
  --native-probe /tmp/wasm-fist-model-sanitizer/model_probe
```

Verified on 2026-10-06: both production build gates, strict LLVM 19.1.7 format/tidy and all six
model groups pass on both targets with the complete original-instruction oracle and no skips.
ASan/UBSan/leak detection passes all six groups and every original family. Normal fixture build
gates deliberately skip the separately requested original corpus. This is model-data/ownership
coverage; existing terrain presentation is unchanged and no vehicle rendering or playable mission
is claimed. All temporary fixtures, build logs and sanitizer binaries stay under `/tmp` and
obsolete scratch is removed after acceptance.

The complete catalog and default ground-vehicle selector/part-pose contract are documented in
[vehicle-model-selection.md](vehicle-model-selection.md) (WI 0057). Sprite projection, palette
translation and vehicle drawing remain separate steps.
