# Scenario envelope and metadata

The shared C11 decoder in `src/assets/scenario.c` reads original `.FSG` mission files without
guest registers, memory images or generated engine code. Input stays immutable and caller-owned;
the returned chunk/unit views borrow it. Decode failure leaves the caller's output unchanged.

## Original evidence

Machine code was rechecked from `re_out/fist_dat_image.bin`, SHA-256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`, at frozen reference `349ad31`:

```sh
objdump -D -b binary -m i386 -M addr16,data16,intel \
  --start-address=0xd501 --stop-address=0xd5f9 re_out/fist_dat_image.bin
objdump -D -b binary -m i386 -M addr16,data16,intel \
  --start-address=0xd7b5 --stop-address=0xd81e re_out/fist_dat_image.bin
```

`d501` reads six bytes per chunk at d51b/d53b, checks first tag SHDR at d529/d531, dispatches
the seven tags through e9e6 and seeks over unknown chunks by their declared 16-bit length.
`d7b5` reads the header into e982. `d599` consumes byte e984, eight dword copies at d59f–d5db
consume e988 through e9a4, and d5df consumes byte e987. These are payload offsets 2, 6–37 and 5.
The decoder exposes the raw mode/limit bytes and signed two's-complement 32-bit position values;
it does not invent units or reinterpret zero-limit gameplay behavior.

`d7e1` reads a 16-bit unit count, then a six-byte record header; d808/d81e consume the declared
state body. The writer d6e4 and reference patch 360 identify header words as state size, catalog
index and catalog value. The reader validates lengths/counts and returns state views, without
claiming to implement vehicle/AI state semantics.

## Wire layout

Each chunk has a four-byte ASCII tag and little-endian uint16 payload length. Payload follows
immediately with no alignment padding. Published missions contain each of these once:

| Tag | Payload | Current interpretation |
| --- | --- | --- |
| SHDR | 54 bytes | Version uint16 at 0; mode byte 2; limit byte 5; eight int32 positions at 6. |
| DCBS | Variable | Count uint16, then count × (three uint16 header words + declared state bytes). |
| PATH | 2144 bytes in originals | Borrowed full payload; path semantics remain open. |
| STMP | 258 bytes in originals | Borrowed full payload; stamp semantics remain open. |
| PINF | 176 bytes in originals | Borrowed full payload; player semantics remain open. |
| BINF | 70 bytes | Four 16-byte fields: heightmap, colormap, palette, sky; final six bytes retained. |
| TERM | Zero bytes | Complete termination, with no trailing bytes. |

All 47 provisioned originals contain these seven chunks in the table order. The decoder requires
the complete known envelope, rejects duplicate known chunks, requires SHDR first and empty TERM
last, and accepts reordered middle chunks or unknown chunks between them. PATH/STMP/PINF are
retained as bounded opaque views, not declared semantically validated. Header reserved/extension
bytes and BINF's tail are preserved in their chunk views.

BINF fields may terminate with NUL or fill all 16 bytes with DOS space padding: INDIA4's sky is
`5.SKY` followed by eleven spaces without NUL. Spaces are removed like the reference DOS filename
layer; names must remain nonempty and fit the 8+dot+3 basename length. Case is preserved; future
platform/resource lookup must be case-insensitive. Embedded directory lookup is not implemented.

DCBS bodies in the current corpus have lengths 55 or 251 bytes. The envelope decoder follows the
declared length and requires at least the original first state word, rather than guessing a unit
kind from size. Typed state decoding, model catalog resolution and object installation are next
contracts. No objects are fabricated from metadata alone.

## Verification

```sh
bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
python3 tools/rewrite/test_scenario.py --originals
```

Synthetic contracts run through native CTest and the WASM build gate. The explicit original gate
requires all 47 pinned files and rejects skipped coverage. A manifest stores only filenames,
sizes and SHA-256, not game contents. Complete native/WASM transcripts are compared with an
independent Python format interpretation, including every position, asset name, record header,
state byte and known-chunk byte; missing/short/different output fails. The originals are checked
again for unchanged hashes after use. Every truncated prefix of the synthetic complete file
must fail and leave output unchanged; malformed counts/lengths/names/termination also fail.

On 2026-10-06 all eight test groups passed on both targets without skips in the original gate,
covering 574215 original scenario bytes and 4213 unit records across 28 asset combinations.
This proves the documented envelope/metadata contract, not a playable mission or full game.
