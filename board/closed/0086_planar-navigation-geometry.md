Type: Work item
Title: Own complete planar navigation bearing and distance
Depends: 0084

## Contract

Implement the complete semantic 0541 planar return in shared readable C11: full-turn heading
and unsigned distance from wrapped signed DWORD XY, with fine/coarse atan selection. Recover
the actual quadrant/octant fold, normalized division/interpolation and magnitude-dependent
distance precision. Keep immutable numeric tables and scratch local; preserve all input/RNG
state. This is the full geometry prerequisite for remaining 0081 callbacks, not partial ab88
dispatch, target-reference resolution or parent-phase acceptance.

## Evidence

Frozen image SHA256 d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5.
0541 calls complete 0731 bearing and 0927 distance and returns AX plus distance DX:CX.
SS:2448 has 257 authored atan knots, including the wrapped endpoint; SS:2040 selects fine
interpolation or a lower knot. 077e folds signed wrapped deltas, normalizes them and divides
by the normalized high word. 0b71 drops low coordinate bits at 16/24-bit magnitude boundaries;
0927 selects its root precision at squared sizes 32/40/48 bits. 0baf is integer floor sqrt.
Original early-zero and INT32_MIN fold behavior must be proved rather than hidden by clamps.
Initial complete original comparison passes 10867 signed-edge/deterministic spread cases and
all 257 table knots, with unrelated DGROUP/input/RNG preservation. Scratch 2034..203d and actual
call-stack writes are the only excluded memory destinations.

## Next

Continue full 0081 on master with this shared geometry owner. Recover complete remaining ab88
bearing/range handlers and their caller/target contract, then waypoint, discovery, physical
roster, presentation and complete parent sampling/admission/scheduling. Do not reinterpret
saved target-reference words as physical slots or substitute heading-only dispatch. First
playable battle, PCM/outcomes and the full rewrite surface remain under 0041/0065 and 0042..0047.

## Accept

Both targets pass the complete recorded geometry domain and required original corpus without
skips. Missing/truncated/output runs fail. Production fast-math builds, strict checks and
sanitizers pass; existing command/state/control/visual behavior remains verified. No complete
navigation callback bank, first playable battle, PCM or final ten-run claim follows. Keep
full 0081/0041/0065 and remaining rewrite requirements open; commit/push and clean /tmp.

## Verified acceptance

Required `test_geometry.py --target all --originals --oracle` passes all seven groups with zero
skips in 96.496 seconds: 397671 complete returns per target and 960 saved actor/goal pairs from
all 47 pinned originals. Scope is 131072 quotient-word fine/coarse returns, 16384 dense octant
returns, 131072 axis/diagonal low-word magnitudes, 66248 signed/precision edge combinations,
40960 complete mode-byte/absolute-position cases, 10000 deterministic full-DWORD cases,
15 original table/scratch edge returns and 1920 saved-pair quality returns. All complete
numeric outputs and unchanged original input/RNG/unrelated-memory bytes are required. This
does not claim an exhaustive 2^64 displacement run. The current standalone Native gate passes
the same seven required groups in 104.434 seconds.

ASan/UBSan/LeakSanitizer passes the same seven groups, zero skips and complete 397671/960 scope
in 77.619 seconds with production fast-math flags. Bulk decoding poisons/frees every source
before observations; all truncation/trailing/count/missing/argument failures emit no partial
output. Complete original signed-extreme fold and numeric precision returns are retained.

`CTEST_PARALLEL_LEVEL=4 bash tools/rewrite/build.sh all` exits zero: all 34 Native CTests
(80.19 seconds), all 32 WASM Python suites and both renderer gates. Standard suites omit the
optional original oracle/corpus; separate required geometry and canonical gates have zero
skips. Full strict format/tidy passes all 79 owned C units. Required canonical driving on
both targets passes seven groups / 123 fixtures / 924 complete timed boundaries / 907 installed
objects / 60 explicit rejections in 24.257 seconds. Existing selector/goal/world/control
regressions pass in the complete standard suites.

SHA256 comparison with the accepted 0084 receipt proves both platforms' goal, selector, world,
driving and preview executable/module binaries are byte-identical. This pure geometry helper
is not yet wired into those existing callers. Their previous complete original and actual
Native SDL/Chromium visual/device evidence is retained for those exact binaries; no new scene
render or full navigation/gameplay acceptance is claimed. Platform presentation sources and
the pinned dependency are unchanged. The completed build's existing pixel/render gates pass.

An initial required run failed because a fixture generator enumerated a target before its
X value; the missing case matrix was corrected and the complete final gates include it.
Initial tidy rejected nested conditionals, short names and indirect owner includes. These
were corrected with explicit branches/names/includes; checks and expectations were not weakened.
Reproduction and numeric rules are in docs/planar-geometry.md. Raw logs, prototype and sanitized
build belong under /tmp/wasm-fist-0086-review and /tmp/wasm-fist-0086-sanitized. Retain a compact
source/binary/log/coverage receipt and remove obsolete owned artifacts after commit/push.
The full rewrite goal stays active and the final complete WASM streak remains zero.
