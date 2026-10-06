Type: Work item
Title: Owned original height-field enlargement and reduction
Depends: 0061

## Contract

Provide shared C owned square height-field resampling before terrain installation. Reproduce
the original map-loader bc06 doubling and bed2 halving: periodic neighbors, horizontal-then-
vertical integer averages, preserved source knots and exact even-coordinate reduction. Copy
the embedded palette, retain no source views and publish nothing on invalid input/allocation
failure. This is height data; colormap palette interpolation uses a different original method.

## Evidence

Original 89b0 height load 8bbd..8c16 repeats bc06 or bed2 until the desired runtime dimension.
bc06 first doubles columns and then rows, preserving even coordinates and averaging the next
wrapped neighbor with ADD/RCR (floor of the full 9-bit sum). bed2 selects even columns/rows.
All complete routines and their dimension updates are available in the pinned kernel image.
Ground op 1c consumes this expanded plane; the static renderer currently uses the base plane.

## Next

Complete. Closed 0063 supplies height/slope queries and typed ground contact. Continue 0064
with installed motion in the controlled scene. Colormap resampling, terrain collision and full
vehicle/mission timing remain separate work within parent 0041.

## Accept

Complete output pixels, dimensions and palette agree on both targets with original height-field
resampling. Periodic seams, floor rounding, step order, reduction, source independence and
atomic failures are covered. All requested original coverage and required checks pass. No
expanded-field rendering, terrain collision, installed vehicle or playable-mission claim is made.


## Verification

2026-10-06: `bash tools/rewrite/build.sh all` passes all twelve native CTests and every WASM
asset/scene/pixel gate. `python3 tools/rewrite/check_style.py` passes all 33 owned C units with
unchanged LLVM 19.1 rules and requested production compiler flags.

`test_heightfield.py --originals` passes all eight groups on both production targets, no skips
(131.799 s). The independent oracle executes complete bc06 and bed2 returns with their in-place
buffers, dimension globals and a checked return stack. There are no instruction hooks, patches
or replacement calls. Each returned dimension and every final pixel agrees with the independently
calculated separable fixture and the owned C output; palettes preserve all source bytes.

The pinned square corpus includes 18 planes and 2622592 source pixels: eight numerical height
maps, eight color planes exercised only numerically and two 24-square stamps. Every plane has
separate complete doubling and halving outputs plus repeated up/down steps. Every height map
is additionally checked at 512/1024/2048/4096 and in a 4096-to-original-side round trip. The gate
compares 94 complete corpus outputs totaling 192550816 pixels per target, plus synthetic cases.
Original and decoded hashes match their pins; original files remain unchanged. Four rectangular
SKYs are explicitly outside the square-height contract.

Synthetic coverage includes all 65536 byte pairs on both axes, ninth-bit carries, wrapped seams,
one-cell constant maps, asymmetric corners, exact even-knot reduction, non-power-of-two sides,
unchanged-size independent copies and repeated ownership. The 0,1 / 1,2 fixture proves that a
single four-corner mean is wrong: original staged integer rounding gives zero at odd/odd.
Missing/truncated/trailing inputs, bad ratios/shapes, null/alias arguments and 32-bit size overflow
fail without publication. All source pixels and palette bytes are verified preserved, then
overwritten and freed before observation or the next stage.

Actual intermediate allocation failure is tested on both production targets with an 8192-square
request: native RLIMIT_AS and a bounded non-aborting WASM probe heap make malloc fail. The
shared routine releases its private intermediate buffers and preserves source and output.
The 64 MiB budget belongs only to the probe, not an implemented game quality decision.

ASan/UBSan with leak checking passes all eight native groups, including the original corpus,
all four runtime sizes and large round trips (114.955 s). Its one explicit skip is RLIMIT_AS,
which conflicts with sanitizer shadow memory; both production targets pass that case separately.
The final production gate additionally publishes the separate doubling/halving outputs for all
original planes; sanitizer coverage already exercises the same allocation/sample paths through
all-byte-pair fixtures, all original planes and every large runtime size.

Code/contracts and reproduction commands are in `docs/heightfield-resampling.md`. Review of
actual loader instructions confirms detail comes initially from TCB +59h and marker-file
existence overrides for 4.MEG/8.MEG/16.MEG/40.MEG; these are not host-memory probes. The caller
selects the requested side explicitly. The shared renderer continues to use its decoded base
plane, so this step changes no displayed frames; WI 0059 remains the reviewed native/browser
visual baseline. No installed contact state, collision, controls or playable claim is made.
Commit/push the verified prerequisite and remove its owned logs and sanitizer binary under /tmp.
