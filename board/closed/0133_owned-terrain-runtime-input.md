Type: Work item
Title: Owned high-precision terrain runtime input
Depends: 0130, 0131

## Contract

One shared C11 owned map decoder and periodic triangle sampler prepare native/WASM
terrain consumption without original files for first playable 0041. Implement
FMAP1 as documented in docs/owned-map-format.md, with complete validation before
allocation, independent 16-bit height/RGB ownership and atomic decode failure.
Preserve height-range/water metadata; do not quantize through original palettes
or 8-bit height formats. Package verified owned PNGs with the offline standard-
library tool, validating the entire batch and recording reproducible provenance.

## Evidence

Delivered src/assets/owned_terrain.c and assets/generator/pack_maps.py. All eight
lossless bundles and adjacent package manifests are versioned under
assets/generated/runtime/maps: 16 files, 41947750 bytes. Complete repeated packer
output is byte-identical to these files. Generation/loading needs no original
images, palettes, game files or PNG/deflate runtime dependency.

The immutable /tmp/wasm-fist-owned-terrain-source snapshot records 687 file
hashes, source base 075e9f5 and unmodified softgl 1d17a94, without armoredfist/.
Every stage checked frozen source hashes before/after execution. The final audit
also matched current owned C/tests/tools/assets/configuration and library bytes
to the tested snapshot; later root changes concerned documentation only.

All nine sequential commands finished with actual exit 0. Audited evidence is
under /tmp/wasm-fist-owned-terrain-review, including exact commands, source
hashes, terminal receipts, log hashes and acceptance-audit.json:

- Native configure/build and strict LLVM 19.1.7 format/tidy: 105 owned C units.
- Complete production bash tools/build.sh all: 55/55 native CTest regressions;
  53 WASM Python scripts, including 51 unittest suites and 380 groups, plus both
  Node renderer checks. Owned terrain runs all 11 groups without skips on each
  target. Explicit optional original/reference skips in other suites remain
  development-only; this run does not establish complete-game independence.
- Original-free generator: all 13 groups without skips.
- Whole eight-map packaging: all 16 files match the versioned bundles/manifests.
- Production-fast-math ASan/UBSan: 10 explicitly named groups, no errors/leaks.
- Memcheck: 3 named groups, 10 processes, zero errors and zero remaining blocks.

The 11 owned groups check every possible height word and RGB byte after source
poison/free; fractional metadata; complete malformed header/length/CRC/nonfinite
rejection; both cell triangles, diagonal, knots and periodic edges/corner;
negative/multiple-period and huge finite coordinates; actual allocation failure
and reclamation on native/WASM; all eight complete real owned maps; corrupted
PNG/manifest/deflate input; and actual late-batch preflight failure before any
write plus repeated complete success. Failure output/ownership remain unchanged.

Production fast math initially optimized ordinary representation tests into
always-true floating-class checks. Volatile representation reads preserve the
integer exponent validation with compiler flags unchanged; nonfinite metadata
and coordinates now fail on both production targets and the sanitizer build.
Memory-pressure fixtures reach partial decoder allocation and prove recovery,
rather than failing while reading input. Both production targets retain this
pressure group. ASan's virtual reservations prevent using the same native address
limit, so its ten-group subset excludes only that explicitly covered case.

## Next

Configure the actual MSAA/three-helper renderer profile in 0134 using existing
softgl APIs and owned code only. Consume the owned maps for near-ground native/
browser visuals and motion; continue ridge/gully/material refinement in 0131.
Own mission installation/gameplay, water/sky geometry, Blender assets/cockpits,
new audio and actual 60-FPS frame measurements remain required. This boundary
establishes lossless owned terrain input, not complete runtime rendering, final
visual approval or full-game independence. The final ten-run WASM streak stays 0.

## Accept

Complete strict validation/ownership and canonical periodic surface behavior;
all eight actual owned bundles packaged and fully decoded on both targets;
meaningful failure/source-lifetime/precision/geometry/pressure coverage without
owned skips; strict LLVM style and complete production builds/regressions; and
clean bounded sanitizer/Memcheck results. These checks are satisfied for this
asset boundary. Full 0130/0131/0041/0047 acceptance remains open.

Reproduction uses an original-file-free checkout and external build root:

```sh
FIST_REWRITE_BUILD_ROOT=/tmp/wasm-fist-owned-terrain-production bash tools/build.sh all
python3 tools/check_style.py --build-dir /tmp/wasm-fist-owned-terrain-production/native
python3 tests/test_map_generator.py
python3 assets/generator/pack_maps.py --output-dir /tmp/wasm-fist-owned-map-packages
```

Exact sanitizer compilation, ten selected test names and the Valgrind wrapper/
three selected groups are retained in the review config/terminal receipts.
