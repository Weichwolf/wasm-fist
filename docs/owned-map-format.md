# Owned terrain wire and sampling contract

FMAP version 1 is a complete, lossless terrain input for shared C11 native/WASM
loading. It contains only owned unsigned-16 heights, RGB8 surface colors and
explicit height-range/water metadata from the JSON authoring recipe. It needs
neither a PNG/deflate runtime dependency nor original palettes, images or files.
PNG remains the compressed offline authoring/review output. Runtime planes are
uncompressed: a 1024-square bundle is 5,242,944 bytes, with bounded direct decode.
All eight final bundles and adjacent package manifests are versioned under
`assets/generated/runtime/maps`; a release needs those runtime files, not the
offline review PNGs. Their total binary footprint is approximately40MiB.

All integers and IEEE-754 binary64 metadata are little endian. The 64-byte header:

| Offset | Bytes | Meaning |
| --- | --- | --- |
| 0 | 8 | Exact magic `FISTMAP` followed by a zero byte. |
| 8 | 4 | Version 1. |
| 12 | 4 | Header length 64. |
| 16 | 4 | Square side: power of two, 16 through 4096. |
| 20 | 4 | Height plane length: side squared times 2. |
| 24 | 4 | RGB plane length: side squared times 3. |
| 28 | 4 | Flags, zero in version 1. |
| 32 | 8 | Minimum world elevation. |
| 40 | 8 | Maximum world elevation. |
| 48 | 8 | Water level, allowed outside the terrain range. |
| 56 | 4 | Reserved, zero. |
| 60 | 4 | Standard CRC32 of header bytes 0..59 followed by the entire payload. |

The payload begins with row-major little-endian unsigned-16 height samples,
followed by row-major RGB8 colors. Rows increase in authoring map Y. There is no
palette, premultiplied alpha, row reversal, padding or trailing data. Decode height
as `minimum + sample / 65535 * (maximum - minimum)`. Elevation metadata must be
finite, within -10000..20000, and minimum must be strictly below maximum. A decoder
checks the complete header, lengths and CRC before allocating independent planes.
Failure leaves the output untouched; destroy releases both planes and resets it.

Normalized authoring X/Y wrap over a unit period. The shared sampler splits each
cell along the top-right to bottom-left diagonal, then interpolates its two
triangles, including both periodic edges and their shared corner. This contract
must also own geometry sampling for render/contact consumers; a separately
rounded 8-bit height copy is not acceptable. World coordinate conversion remains
owned by `src/sim/world.h` and its consumers, not by this serialization format.

`assets/generator/pack_maps.py` checks recipe identity/hash, generated manifest,
complete PNG dimensions/encoding/chunk CRC/deflate/rows and both output hashes
before packaging a batch. It preserves every sample/color byte and records recipe,
generator-manifest, packer and final-output SHA256 in an adjacent package manifest.
It uses only Python's standard library. Reproduction:

```sh
python3 assets/generator/pack_maps.py --output-dir /tmp/wasm-fist-owned-map-packages
```

This is an asset boundary, not complete runtime scene or game acceptance. Render
integration, near-ground native/browser visual review, mission placement, collision,
water rendering and the requested MSAA/thread/60-FPS measurements remain required.
