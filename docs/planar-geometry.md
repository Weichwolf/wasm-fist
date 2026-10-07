# Planar navigation geometry

WI 0086 owns the complete semantic original 0541 return in shared C11
`sim/geometry.c`: a full-turn heading and unsigned DWORD distance between two signed DWORD
XY positions. Inputs use the existing owned waypoint value type. Subtraction wraps modulo
2^32 before the original quadrant and distance rules. The helper is pure: numeric scratch is
local, inputs are values and no world/RNG state is mutated. It does not dispatch ab88, resolve
saved target references or update the parent heading phase.

The frozen image is pinned at SHA256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
Actual 0541 calls complete 0731/077e bearing and 0927 distance, saves the bearing and returns
it in AX, with unsigned distance high word DX and low word CX. These are semantic numeric
outputs; DOS registers and scratch do not become C game state.

Heading zero follows +Y; a quarter turn follows +X. The original first rotates negative-X
and negative-Y wrapped words into a quadrant. It folds an octant using
`tan(a - pi/4) = (x - y)/(x + y)`, normalizes both lanes until Y's top bit is set, drops Y's
low word for division and saturates the numerator at the last legal word quotient. Retain
this normalization precision rather than assuming an exact mathematical ratio.

SS:2448 contains 257 authored atan knots. Each agrees with the independently calculated
`round(atan(i/256) * 65536 * 8 / tau) mod 65536`, including the wrapped endpoint zero. Fine
rotation interpolates a wrapped word difference with an eight-bit fraction and round 128;
nonzero SS:2040 selects the lower knot. Octant composition rounds the final turn by four
before division by eight. The immutable C table is independent of the original image at runtime.

Original 0b71 computes the squared magnitude after discarding these coordinate bits:

| Wrapped absolute lane magnitude | Retained lane before squaring | Restored squared scale |
| --- | --- | --- |
| below 2^16 | complete magnitude | 1 |
| 2^16 through 2^24 - 1 | magnitude >> 8 | << 16 |
| at least 2^24 | magnitude >> 16 | << 32 |

Add the two resulting unsigned squared lanes. Original 0927 then selects its integer-root
precision from that sum:

| Squared sum | Returned distance |
| --- | --- |
| below 2^32 | floor_sqrt(sum) |
| 2^32 through 2^40 - 1 | floor_sqrt(sum >> 8) << 4 |
| 2^40 through 2^48 - 1 | floor_sqrt(sum >> 16) << 8 |
| at least 2^48 | floor_sqrt(sum >> 32) << 16 |

The C integer root uses radix-four subtraction; it adds no floating-point dependency under
production fast-math. Wrapped signed magnitudes cannot exceed 2^31, so the squared sum cannot
exceed 2^63. The original ninth scratch byte remains zero for every input to this complete
two-lane helper; its extra-overflow branch is unreachable, not replaced with a fake return.

Keep the original zero/signed-extreme contract explicit. The octant sum can wrap to zero for
INT32_MIN extremes, and actual 07a5 then returns heading zero. Signed negation also retains
INT32_MIN's word pattern before the next quadrant check. Tests execute these original returns;
no clamp or mathematical atan2 shortcut hides their behavior. This documents the helper's
recovered rules, not general mathematical accuracy or complete mission navigation acceptance.

`original_geometry_oracle.py` executes unchanged complete 0541 and all actual nested calls.
It verifies input identity and all 65536 DGROUP bytes outside the actual numeric scratch
2034..203d and call stack 8fc0..9001. Both input positions, SS:2040, RNG and unrelated bytes must
remain unchanged. No code hook, guessed function result or original instruction is substituted.
The bulk probe copies every input before poisoning/freeing the source and rejects incomplete
or trailing batches before emitting any output.

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_geometry.py \
  --target all --originals --oracle
CTEST_PARALLEL_LEVEL=4 bash tools/rewrite/build.sh all
python3 tools/rewrite/check_style.py
```

Required coverage includes every division quotient word in both fine/coarse modes, all octants,
every low-word magnitude on axis/diagonal, precision and signed-wrap boundaries, all mode bytes,
deterministic full-DWORD spread and both quality returns for all 47 files / 960 saved ground
actor/goal pairs. This is the recorded numerical domain, not an exhaustive 2^64 displacement
claim. Complete callback/target/waypoint/roster/parent scheduling and first playable battle/PCM
remain under active 0081/0041/0065. Renderer view-bearing remains its deliberate continuous
visual calculation. Exact accepted results and cleanup live in WI 0086; final complete WASM
streak remains zero.
