# Ground command bearing and target-loss evidence

Active WI 0094 recovers the complete original ab88 near return and its eight-entry DS:9810
bank. The shared C callback remains pending. This is input for the complete 0081 phase,
not a heading-only implementation or a playable-battle claim.

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

Complete class-start observations preserve +44 and +47. They need explicit retained C fields;
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

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_ground_bearing.py --originals --review-dir /tmp/wasm-fist-0094-review
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_command_boundary.py --originals
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_geometry.py --target all --originals --oracle

The required research gate has six groups, with no optional skips. It covers every control
word in every command mode, all packed range words for approach and retreat, complete counter/
maneuver/heading domains, all-class admission, signed/wrapped/coarse geometry, target-prefix/
self/orphan/null/release/reuse paths, actual class starts/UI and all prepared parent contexts.
Original files and frozen code remain unchanged. Production C, its artifacts and accepted
scene evidence are unchanged; C discovery is already delivered but this callback, full parent
bank, living battle/device/PCM/outcomes and final independent WASM streak remain open.
