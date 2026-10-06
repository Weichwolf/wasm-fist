# KLC planes and resource palettes

The shared C11 asset library decodes complete `KLC1` images, looks up `RESOURCE1` members and
validates original VGA DAC palettes. It accepts immutable byte spans without platform I/O or
generated engine code. KLC pixels and palette bytes are owned by the decoded image; resource
views borrow the archive. Every decode/lookup failure preserves the caller's output.

## Original evidence

The runtime-kernel instruction image `re_out/fist_image.bin` has SHA-256
`102a5465e03f3b0397041b1bd783d33992ac2c14bb6e42a713af610ec08c3ab1` at the frozen reference.
Recheck the evidence with:

```sh
objdump -D -b binary -m i386 -M intel \
  --start-address=0x643c --stop-address=0x686c re_out/fist_image.bin
objdump -D -b binary -m i386 -M intel \
  --start-address=0x6250 --stop-address=0x643c re_out/fist_image.bin
```

`646d` recognizes KLC1. `659a/65a0` read dword width/height; `65be–65d6` copy 768 palette bytes.
`65f3/65f4` traverse block rows/columns, `6606–6610/683e–683f` consume control pairs across row
boundaries, and `6623`, `672e`, `67fb/6819`, `682c` implement the four modes below. Row writes
use EDI plus zero, one, two or three times the decoded width; `683b/684a` advance block/row bases.
The decoded row order is preserved. World axes, height units and camera placement are a separate
contract; they are not inferred from a picture of the image.

`6250–6289` folds a basename, pads it to 12 bytes and XORs three dwords with `0xACEDDEAD`.
`62dc–6304` reads the resource header/count and includes one sentinel entry. `6331–635f` searches
the encoded directory; `636a` selects its offset; `6397–63a3` subtracts the next entry's offset
to obtain the member length. Local `PAL.RES` contains 32 complete 768-byte palettes with every
component in the six-bit VGA DAC range. The shared palette decoder preserves those DAC bytes.

## KLC1 layout

| Offset | Data |
| --- | --- |
| 0 | ASCII `KLC1` |
| 4 | Little-endian uint32 width |
| 8 | Little-endian uint32 height |
| 12 | 256 × three embedded RGB bytes |
| 780 | Compressed 4×4 blocks |

Dimensions must be positive multiples of four and their product must fit `size_t`. Blocks are
row-major. One control byte supplies four successive two-bit modes, least-significant pair first.
Pairs continue across block-row boundaries; unused pairs in the last byte are ignored.

| Mode | Payload | Sixteen pixels, in row order within the block |
| --- | --- | --- |
| 0 | Three index bytes, then LE uint32 selectors | Two bits per pixel, low first; indices `[0,a,b,c]`. |
| 1 | Two index bytes, then LE uint16 selectors | One bit per pixel, low first; indices `[a,b]`. |
| 2 | One byte `v`; if zero, another 16 bytes | Nonzero `v` fills the block; zero introduces raw pixels. |
| 3 | None | All zero. |

The decoder requires exactly the complete stream, without truncated or trailing bytes. It validates
using the same block reader before allocation, then materializes the pixels. It does not allocate
from unproven dimensions or cap an otherwise valid image to a guessed game size. The embedded
palette is preserved without applying VGA scaling or mission palette remapping.

The current corpus contains eight 256×256 heightmaps, eight 512×512 colormaps, two 24×24 stamp
planes and four 128×1024 sky planes. All use this same format. They total 3,146,880 decoded pixels.

## RESOURCE1 and PAL layout

The archive starts with `RESOURCE1\r\n\x1a` (12 bytes), then a LE uint32 member count. Each directory
entry has a 12-byte encoded basename and a LE uint32 absolute payload offset. There are count+1
entries; the last supplies the end offset. Decode names by XORing with repeating bytes
`AD DE ED AC`; names were DOS folded and NUL padded before encoding.

Lookup uses the original DOS folding, including its punctuation mapping: bytes at least `0x60`
subtract `0x20`. The public adapter accepts ASCII basenames of at most 12 bytes without spaces,
path separators or drive prefixes. It returns the first matching member, as the original does.
It validates the entire directory before returning: the first payload begins after the directory,
offsets never decrease or escape the input, and the sentinel ends the file. Zero-length generic
members are permitted. A palette member must contain exactly 768 bytes, each at most 63.

Scenario bundle loading and original mission palette preparation/remapping are now supplied by
`src/assets/terrain.c` and `palette.c`; see [terrain loading](terrain-loading.md). Terrain rendering
still needs world placement. These readers do not yet install terrain, stamps, sky or vehicles in a scene.

## Verification

Constructed fixtures run in native CTest and the WASM build gate. They exercise all modes and
selectors, row orientation, cross-row control grouping, unused control pairs, raw/fill/zero blocks,
every truncated synthetic KLC prefix, unchanged output on failure, invalid dimensions/overflow,
trailing data, resource count/directory/offset errors, DOS folding, missing names, duplicate-first
selection, zero-size members, six-bit palette bounds and repeated destruction.

The optional original gate executes actual original x86 instructions through pinned Unicorn 2.1.4.
No instruction hook implements or replaces decoding. It begins with buffered files and stops before
DOS close; the resource oracle executes original name encoding, header reads, directory search,
offset selection and length subtraction. The expanded gate also executes original mission palette
preparation/mapping as documented in terrain loading. DOS transport/refill, paging, terrain resampling
and full gameplay are outside that oracle's scope. The pinned manifest contains only
names, dimensions, sizes and hashes; original bytes remain ignored and read-only.

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
python3 -m venv /tmp/wasm-fist-decoder-oracle
/tmp/wasm-fist-decoder-oracle/bin/pip install -r tools/rewrite/oracle_requirements.txt
# If the ignored instruction image is absent, regenerate it from provisioned originals:
make kernel-image
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_terrain_assets.py --originals
```

On 2026-10-06 all eleven groups passed on both targets without skips in the original gate. Every
pixel, embedded palette byte and selected resource byte matched the original instruction output;
missing, short, different or skipped required output fails. All 22 image files and 32 palettes
were covered, with original hashes checked again after use. An AddressSanitizer/UndefinedBehaviorSanitizer
native build passed the same synthetic and complete original contracts. All 47 original scenario
contracts also passed after extracting shared endian/view and probe-I/O owners.
