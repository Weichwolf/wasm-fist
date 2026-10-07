# Ground obstacle maneuvers and idle turret

Closed WI 0100 proves complete original ae66, af1c/f69:b2a0, b059 and b017. Required full
original/domain/corpus/parent, retention and every-search-exit gates pass. Closed WI 0101 supplies
complete shared C consumption; full 0081 remains open. Frozen image SHA256:
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.

## Maneuver state

ae66 masks saved +45 with 6 and calls four genuine entries: ae7b, aea8, af0b and aea2.
The original table at ae73 is data, despite the decompiler's spurious executable interpretation.
Separate saved bytes +46 (remaining maneuver count) and +51 (blocked count) are required;
both initialization and readiness retain them. Closed 0101 supplies their C ownership.

For state zero, a cleared control bit 8 resets +51 only. Otherwise +51 increments with byte
wrapping; values below three select state two with count three. Other values decrement +46
and clear +45 when that byte reaches zero. Count zero decrements to 255. State six performs
that same countdown; state four can restart its four-count continuation while bit 8 remains
set. Unused selector bits remain intact unless a real state write occurs.

State two counts down before its full direction search. On expiry it tests fifteen original
offsets around independent hull heading +26. Each uses complete original 03a9 rotation at
magnitude 32 and multiplies both signed components by eight. The physical registry is scanned
in slot order for every attempt. A clear direction ends the search; exhausted search retains
index fifteen even though that direction was not tested. The stored +47 heading uses the
offset eight entries beyond that index. It is not the tested heading. State four/count four
are then published, preserving the original control bit and blocked byte.

## Ordered obstacle prediction

af1c skips null/current-actor pointers, type 21 and objects without flag 64. It retains
registry order and reads current physical pointers, including a valid actor whose logical
registry binding was overwritten. Generation values do not govern this scan. Complete b112
executes 0731 and adds a half turn before comparison with the actor's absolute turret heading
+10. The rejected angular sector is [16384, 49152). It is independent of hull heading +26.
The scan then requires wrapped unsigned Manhattan separation at most 7680.

f69:b2a0 starts eight movement vectors ahead, advances before each of 24 distance samples and
compares the low distance word with the wrapped sum of both +14 extents and the margin. af1c
uses margin 1024. Original a19a/0927 Euclidean distance, quantization and ten-byte scratch
execute unchanged. Predictive endpoints and scalar scratch are compared explicitly; no
geometry or collision result is replaced.

Reaching b059 uses the same angular admission and predictor with margin 3584. It initially
uses saved signed velocity +59/+5b multiplied by eight. Any nonzero +45 instead selects
complete heading rotation at magnitude 64, independently of ae66's masked selector. A hit
sets control bit 8 without clearing other control bits. ae66 subsequently consumes that bit.
No altitude, side, generation or inferred nearest-object rule is added to this boundary.

## Idle turret and parent entry

b017 early returns when control bit 4 or saved target word +97 is nonzero. Otherwise it draws
from the canonical four-stream RNG. Ordered masks 0x3f, 0x3c0 and 0x1c00 select requested
turret offset +8b: 32768, zero, or a conditional second draw masked to 0x3fff then reduced by
8192. If no branch applies, the request is retained. The target word is a presence gate here;
it is not dereferenced or interpreted as a new runtime identity.

Genuine ab03 entries seven/fifteen call ae66 in the automatic bank; entry eleven calls b017.
Their controlled-bank b111 entries are actual RET instructions. ab03 still consumes its
unconditional phase draw before global admission and retains the separate phase value.
Full heading sampling, firing callbacks, selected b152 diagnostics, class/battle and PCM
remain required before full parent acceptance.

## Shared C ownership

Closed 0101 implements these complete callbacks in `src/sim/ground_maneuver.c` through
`fist_mission_world_maneuver`, `fist_mission_world_observe_obstacle` and
`fist_mission_world_idle_turret`. Their required complete gates pass. The command
owner retains `maneuver_count` (+46) and `blocked_count` (+51); existing selector, turn,
control word and requested turret offset remain their sole owners.

Prediction borrows current `fist_mission_world_view` bodies and uses shared rotation and
`fist_planar_distance`, whose quantization/root implementation is the existing geometry
owner. Type-8 +14 is now retained by its projectile payload; the delivered M1 constructor
initializes it to zero. Candidate generation, side, target lifetime and logical roster do
not become extra obstacle filters. All coordinates and byte/word boundaries wrap explicitly.

Maneuvers calculate a local command transaction before publication; invalid used physical
bodies preserve the complete world. Idle work draws into a local copy of canonical RNG and
publishes only its state and requested offset after success. Its target-presence gate never
resolves a physical successor. The tests poison source records and verify every unrelated
world byte, actual canonical sequences, retention, release/reuse and branch-specific early
returns. Remaining firing/diagnostics and the complete parent/class scheduler remain open.

## Verification

The first all-blocked pilot exposed an incorrect bare-0731 interpretation of b112. The model
now includes its actual half-turn tail without changing comparisons. Complete empty/blocked
searches and 288 wrapped numeric, ten RNG and six genuine parent pilot returns pass. These
pilots are excluded from acceptance; the complete gates below supersede them.

The five-group gate passes without skips in 303.195 seconds: 528022 complete original returns,
including 38376 genuine parent returns. All 47 actual prepared missions at four details produce
188 worlds, 3840 ground actors and 42240 complete canonical returns, including motion-producer
consumption and constructed full searches in the real registry. Output SHA256:
`0df24b58abe1665452fb757eaf9a00562b320da3b46cfb006ba608769311c8f8`.

A separate two-group gate passes without skips in 4.263 seconds: 3072 class/link/cursor contexts,
6144 complete start/readiness returns and 512 actual live/released/reused/explicit-clear target
returns. It verifies all maneuver byte values and proves the unused RNG cursor is ignored.
Release/reuse does not clear the retained target or trigger idle randomness; target clearing
remains its existing owner's job. Output SHA256:
`7330a93d4f8a9684b59e1f500ea50b684d770966cb8f18705d4110b5aba2f27e`.

The separate search gate passes without skips in 14.235 seconds: 192 actual allocation contexts
cover all search exits 0..15 for every ground class at coarse bytes 0/1/255; eight additional
competing-body contexts prove registry ordering. Declared uint16 radius-wrap inputs isolate
rays, without claiming authored object sizes. Output SHA256:
`c650016c4aade27c0a5fd1b5ff732e4a32dd53804d002816d336c2f464555bc6`.

The required gate compares every DGROUP byte outside bounded call-stack scratch, immutable
instructions, GS text/tables, mailbox and complete actor/segment/stack returns. It includes
selector/count/blocked/control byte/word domains, all first/second RNG and target presence
words, actual allocated orphans, ordered search/filter/radius/coordinate boundaries, genuine
parent returns and all 47 actual prepared worlds at four details. Missing output or partial
runs fail. Current production C/configuration/programs are unchanged from accepted 0099;
this research adds no native/WASM/style/scene/PCM acceptance. Final full-game streak is zero.

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_ground_maneuver.py --originals --review-dir /tmp/wasm-fist-0100-review

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_ground_maneuver_retention.py --originals --review-dir /tmp/wasm-fist-0100-review
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_ground_maneuver_search.py --originals --review-dir /tmp/wasm-fist-0100-review

Compact results, source/program/original/reference hashes, commands and exclusions are retained
in `/tmp/wasm-fist-0100-review/receipt.json`; obsolete owned logs/helpers/runner metadata are
removed. Closed 0101 consumes this complete contract in shared C; full parent/class/battle/PCM
and independent complete-game acceptance remain required.

## Shared verification

Closed 0101 passes six required groups plus eight nested original groups without skips on
both production targets and ASan/UBSan: 954287 complete cases and 19200 sequential actual
prepared-world callbacks per target. All 47 missions/eight height maps/four details, every
search exit, full byte/word/RNG domains, retained counters, actual released/reused targets,
ordered early-hit/type-21 bypasses and atomic used-input failures are covered. Complete
source records are poisoned; every unrelated world byte is checked. Shared output SHA256:
`9961628d3ac7d4a14f0364b36b7d26f8003ab782aea9ee5ffb53aeb3203ebeae`.

Strict LLVM 19.1 style, the complete 44-native-CTest/42-WASM-suite/two-Node build, actual
SDL/Chromium scenes and sanitized SDL all pass. Two after images were visually reviewed.
An externally terminated partial build is excluded and superseded by the terminal complete
replay. Compact evidence, exact commands, source/program/corpus/original/reference hashes
and exclusions live in `/tmp/wasm-fist-0101-review/receipt.json`; obsolete owned artifacts are
cleaned. Continue af97 under open 0102, then remaining callbacks and full parent/class/battle/
PCM/outcomes. No complete-game acceptance follows; final WASM streak zero.
