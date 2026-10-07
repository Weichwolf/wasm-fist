# Ground command bearing and target-loss evidence

Closed WI 0094 owns the recovered complete original ab88 near return and its eight-entry
DS:9810 bank. The shared C callback is `fist_mission_world_bear_command`, using canonical orders,
physical target lifetimes and the existing planar geometry owner. It is not yet installed
in the complete 0081 parent bank; living battle and playable-mission acceptance remain open.

## Complete callback bank

The bank uses the even command byte +43 directly as a byte offset, without doubling it.
Mode 4 is retreat; mode 6 is approach. The three final entries are genuine single RETs.

| Command byte | Actual entry | Persistent behavior |
| --- | --- | --- |
| 0 | ab91 | With goal-valid control bit 2, measure actor-to-goal; update range in manual and automatic control, requested heading only in automatic control. |
| 2 | abb7 | Require automatic bit 1 and goal-valid bit 2; update heading and range toward the owned goal. |
| 4 | ac09 | Automatic retreat: start a reverse bearing/range once, then consume its byte continuation counter and behavior-dependent duration. |
| 6 | abde | Automatic approach: measure the current target and subtract the original unsigned range offset. |
| 8 | ac5d | Automatic maneuver: only maneuver byte +45 equal to 4 copies saved word +47 into requested hull heading +30. |
| 10/12/14 | ac72/ac73/ac74 | Actual returns; preserve the complete actor. |

The real 0541 service receives source actor XY at +4/+8 and either goal XY at +49/+4d or
target XY at saved near reference +97 plus 4/8. Its actual AX bearing goes to hull request
word +30. The caller executes SHL EDX,8 then MOV DL,CH and stores DX at +53, retaining only
distance bits 8..23. This is a packed unsigned word, not a clamped 32-bit distance. Target
approach/retreat subtract 30 from that word and saturate unsigned underflow to zero. Retreat
adds 32768 to the bearing with word wrap. Fine/coarse angle selection uses original byte 2040.

On the first automatic retreat call with control bit 64 clear, ac09 sets that bit, zeros
saved byte +44 and computes its reverse heading/range. With the bit already set, it never
reads target coordinates: it increments +44 modulo 256 and clears bit 64 when the resulting
unsigned byte is at least the duration. Actual DS:994e contains bytes 8,4,12,0. The PINF
behavior word indexes these bytes directly; original UI cycles prove choices 0..3. The
zero-duration choice still starts a retreat and ends on the next admitted continuation.
These are callback counts, not inferred seconds or simulation ticks.

Complete class-start observations preserve +44 and +47. C retains explicit independent fields;
neither is the existing +42 parent/heading counter, +45 maneuver selector or +8e secondary
bearing. Unused duration/target/heading fields must not cause blanket validation failures.

## Reaching unsafe targets

Original ab88 never validates a target allocation or registry binding. A null +97 in an
admitted target branch sends DI=4 to 0541. Observed real 0731/b71 loads read DS:0004/0006/
0008/000a as separate 16-bit words; SUB/SBB carry across each coordinate. Two declared
unrelated DGROUP inputs produce different actual heading/range outputs. Observed and
unobserved complete returns agree; the hook only records operand addresses, never substitutes
an instruction, read, callback or result. Those bytes are not a valid rewrite world position.

Actual release preserves the source's old near target and the deleted payload's XY. Complete
ab88 keeps deriving the old heading/range. Actual same-type 0->0 and different-type 0->2
allocation reuses that physical pointer, so the next complete callback silently steers toward
the successor. Actual duplicate import instead preserves a physical orphan; its target stays
valid despite the newer current registry binding. The rewrite must distinguish these cases
using the existing lifetime owner, not a repeating saved registry word or current binding alone.

A lifetime check alone does not settle consuming target-loss behavior. A target can disappear
between command selection and this later callback. Recover and verify the complete caller's
navigation/throttle continuation before accepting a fallback; do not read foreign DS data,
manufacture a successor target, suppress a reaching failure or call a missing-target guard
a completed battle repair. An ongoing retreat continuation genuinely needs no live target.

## Deliberate target-loss repair

Only an admitted new targeted measurement checks reference representation/lifetime. A live
physical orphan or in-place type-19 retirement remains a valid target; release, same/different
class reuse and an empty reference do not. Malformed used references or live payloads fail
atomically. Manual modes, genuine return entries and ongoing retreat continuation do not
validate unused targets. Saved near words remain opaque, even when nonzero.

On loss, clear the runtime target, exit retreat control bit 64 and select the ordinary member
navigation mode (leader 0, follower 2). Invalidate the previous goal before invoking the existing
route/formation goal owner, then run ordinary navigation bearing. Empty routes and absent/wreck
leaders therefore leave goal validity clear. Heading/range and unrelated saved fields stay
retained when no new goal is supplied. No additional random draw or invented target position
is used. Subsequent ordinary navigation throttle sees this mode and goal validity; full C
throttle/gear and parent-bank consumption remain later work.

The independent observer in tests/original_ground_navigation_oracle.py executes complete
unchanged ad2f returns for both navigation entries, including its actual ad3b gear callback.
Declared manual gear +90=2 is an explicit boundary that preserves gear, not a replaced callback.
Leader mode without a valid goal sets throttle to zero; valid ranges <=8 use 80, larger ranges
select DS:992c words 96/160/208/224. All 64 actual PINF +6 increment/decrement UI tails prove
four choices. Follower mode stops without a goal, at ranges <=3 or 65535; subsequent boundaries
8/32/48/80 select 16/32/128/240, and larger ranges use 272. These are retained original throttle
words, not newly inferred physical units.

The complete original continuation sweep covers all unsigned range and control words in both
modes, 262528 ad2f returns including 384 explicit repaired goal -> bearing -> throttle sequences.
Of those repaired sequences, 176 stop without a valid navigation goal. All DGROUP bytes outside
throttle and the existing bounded stack remain unchanged, including RNG and other objects.
This proves ordinary navigation consumption of the declared repair; it does not accept the
original unsafe null/released/reused target reads.

The C batch probe releases/allocates/imports/retypes actual canonical pool objects before the
complete callback. It checks full-world preservation outside exact owned destinations and
transactional rejection. Restored sources are poisoned/freed; initialization and readiness
retain +44/+47. The canonical probe loads and prepares all 47 missions on all eight original
height maps and four detail sizes, then runs fine and coarse callbacks in current registry
order. This is complete child-callback consumption, not a partial replacement parent bank.

The next full throttle/profile recovery must distinguish operand widths and destinations:
ad3b admits byte +90 <=1, compares signed word +34 with 0x0e00 and calls the actual a19e
far setter on a transition. Word +34 is retained as drive.terrain_pitch; original speed is
word +55. The four DS:965e setters write +90 and class component bytes +d6/+c8/+ca/+d2,
then call actual f69:7a5b. None of these consuming setter effects is replaced by a return.

## Prepared original parent consumption

The corpus observer actually loads/restores all 47 pinned missions, including real PATH/PINF,
and runs d755 on all eight decoded/resampled original height planes at 512/1024/2048/4096 detail.
It then declares +42=15, admission byte 978a=0 and diagnostic selected pointer 7ae0=0 before
three complete ab03 calls for each current ground actor. Genuine bank entries 0->1->2 execute
heading sampling, command selection, goal assignment and ab88, with the actual world RNG.

This is a constructed reaching counter boundary in a real prepared world, not an untouched
saved-actor or whole living-tick claim. The independent model checks signed heading sampling,
complete actor state and exact RNG after every parent return. All other DGROUP bytes stay
unchanged outside the recovered RNG/caller/scratch/stack destinations. Other actors, physical
roster, orders and allocation metadata are preserved. Diagnostic presentation is explicitly
outside this unselected-parent boundary; no diagnostic callback is replaced by a return.

## Reproduction

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_ground_bearing.py --target all --originals --oracle
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_ground_bearing.py --originals --review-dir /tmp/wasm-fist-0094-review
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_command_boundary.py --originals
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_geometry.py --target all --originals --oracle

The required research gate has six groups, with no optional skips. It covers every control
word in every command mode, all packed range words for approach and retreat, complete counter/
maneuver/heading domains, all-class admission, signed/wrapped/coarse geometry, target-prefix/
self/orphan/null/release/reuse paths, actual class starts/UI and all prepared parent contexts.
Original files and frozen code remain unchanged. The separate required C gate passes eight
groups with zero skips on both production targets: 771911 scalar cases, 7413 atomic rejections,
764498 complete scalar original bearing returns, 188 prepared worlds and 7680 fine/coarse
canonical callbacks per target. The full ASan/UBSan required gate has the same output digest;
full production builds/style and actual SDL/browser/sanitized scenes also pass. Compact evidence
and the two visually reviewed production frames remain under /tmp/wasm-fist-0094-c-review;
see board/closed/0094_ground-command-bearing.md for exact acceptance and exclusions. The full
C throttle/gear and parent bank, living battle/device/PCM/outcomes and final independent WASM
streak remain open; the streak is zero.
