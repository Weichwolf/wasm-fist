# Target discovery and reference lifetimes

The original-only observer in tests/original_target_discovery_oracle.py executes the complete
b011 near return and f69:b378 service at physical 1aa08..1ab7e. It uses actual saved allocation,
registry order, aim wrappers and unchanged PM op-58 visibility on the installed height plane.
It does not substitute a visibility matrix, patch instructions or hook individual instructions.
Shared native/WASM discovery is implemented in src/sim/target_discovery.c. The complete parent
heading/command bank and a playable battle remain undelivered under 0081/0041.

## Selection and secondary state

The scan visits all 182 current registry entries in ascending order. Null entries, the actor,
objects without flag 4 and same-side objects are excluded. Deletion flag 1 is not an additional
scan filter. A physical orphan is not visited, but an old target can still point to it.

Ground types 0/2 prefer ground classes (rank 0), then aircraft 5/6 (rank 1). Ground types 1/3
reverse these ranks. Types 26/27 have rank 2; all other types have rank 99. All 28 entries in both
actual byte tables at 98a4/98c0 are checked against explicit expectations. The three class range
tables at 972e/9736/973e contain 1000 for ordinary link modes, 150 when link >=2, or 625/625/450/450
when link >=2 and actor operating bit 16 is set.

The e1f0/e21c visibility wrapper adds source heights 2048/2560/2048/1920 for ground actors.
Target heights come from all 28 e588 entries. Type 26 instead uses mode byte +19 masked by 3:
3840/4352/2560/3072. Addition wraps at DWORD width. The observer checks every actual transferred
XYZ and sends back the real protected-mode EAX before continuing the original scan.

For visible candidates, a better rank resets both distance words to ffff and retains both
previous pointers. Equal rank continues; worse rank skips primary distance calculation.
Directional a17e proximity receives candidate as source and actor as target. Bits 8..23 form
the compared range. Primary selection requires range <= limit and strictly smaller than the
current distance. Byte 9934 increments on each improved winner, rather than counting all
eligible or visible objects. Thus registry order resolves ties.

The secondary test always follows these branches. Candidate secondary flag 8 admits the
current AX only when it is strictly smaller than the retained secondary operand:

| Previous branch | Actual secondary operand |
| --- | --- |
| Invisible candidate | 0 |
| Visible but worse priority | ff00 OR candidate priority |
| Proximity high byte DH nonzero | Unpacked proximity low word |
| Distance calculation with DH zero, including range rejection | Proximity bits 8..23 |

No fabricated visibility is used to cover DH rejection. Real op-58 permits XY differences
only in [-262144,262144). The opposite-direction proximity lanes are each at most 262144;
their maximum plus half-minimum is at most 393216, below 2^24. Hence real visible candidates
cannot reach nonzero DH. Large-displacement fixtures instead execute actual invisible returns.

At the tail, motion byte +19 loses bit 128; a retained secondary pointer sets it again and
produces bearing word +8e through actual b112/0731 plus 32768. This bearing uses the caller's
existing coarse/fine angle mode. No secondary preserves the old bearing. Counter byte +94
becomes the winner-update count and candidate word +9d becomes the retained primary pointer.
Automatic control bit 1 clears a different nonzero old target +97 only when its class rank
is worse than the scan's winning rank. The scan does not install a new primary target.
Later ae32/a6e3 acquisition and the complete parent bank remain separate consuming work.

Constructed reaching cases retain an earlier lower-priority primary pointer after a better
rank resets the distance but fails range. Invisible secondary objects can win with operand
zero. These are observed original behavior; silently replacing them with a nearest-visible
search would change the contract.

## Original lifetime defect and owned repair

Actual release 1b2ef clears a registry binding, decrements its saved word and marks the payload
deleted. It does not clear another ground actor's target pointer. A complete b011 with no
remaining candidates preserves that old target. Actual dynamic allocation 1b1df then reuses
the same physical slot and registry entry, restoring the same saved word 1 with both same-type
replacement (0 to 0) and different-type replacement (0 to 2). The retained target now silently
refers to the successor.

This is a reaching allocation-lifetime alias, not a hypothetical concern or a monotonic
generation counter. The current fist_object_pool_is_current compares type, physical slot,
registry index and saved value; saved values repeat after release/allocation. That metadata
cannot supply an allocation-lifetime guarantee: all four fields repeat in the proved same-type
case. Preserve both reaching regressions when implementing the repair.

The canonical pool now stores one opaque 64-bit lifetime per live physical slot. A single
process-local issuer in object_pool.c assigns a fresh nonzero identity on allocation/import;
exhaustion fails rather than wrapping. Release and reset invalidate references. In-place
retirement and duplicate-binding orphaning preserve the physical lifetime. Copies retain their
own valid references, while newly initialized/reloaded pools cannot revive references to a
previous pool, including identical type/slot/registry/value tuples. Discarded transactions can
leave identity gaps; identities are neither saved gameplay data nor RNG input. Pool mutation
remains on the simulation's single writer.

Ground command state owns target/candidate runtime references separately from opaque saved
near words. Preparation clears both forms. Complete discovery clears stale old references
even in manual control, while retaining the original conditional clearing for live targets.
It records the primary candidate without installing a new target. The prepared command
selector consumes live runtime presence and preserves its conditional RNG contract; saved
word presence remains only at the previously accepted pre-preparation boundary. Reaching C
tests release/reuse both same-type and different-type successors, consume the repaired target
in discovery and command selection, and check retype/orphan/reset semantics.

mission_view.c owns borrowed pose/flags/mode projection for both discovery and collision.
Ground poses use caller storage; projectile and other payload poses retain their actual
canonical address. Explicit common-field projections support query fixtures for all 28 types
without pretending their undelivered living methods exist. Production restoration/preparation
is independently checked against original occupancy, registry values, poses, flags, mode,
secondary heading/count and RNG before every canonical mission query.

## Logical notification boundary

When old +94 is zero and the new count is nonzero, actual bf3c receives voice byte 14.
Admission requires word 6da2 == ffff, actor == selected pointer 6d34, clear actor side bit 8,
and unsigned wrapped word (clock 0452 - last admitted clock 9fca) >=30. It updates 9fca, sets
AX=028e, retains clock's high byte in DX with DL=0, zeros ECX, and calls device operation 64.
The observer verifies the actual request at e2db and resumes the caller's tail, which does
not consume a device result. Gate 6da2's broader device meaning is not inferred here.

This boundary proves admission, cooldown and logical request emission. It does not execute
the PCM mixer or prove audible playback, full device initialization or complete engine audio.
voice.c now owns this reusable logical admission contract and the canonical world's shared
cooldown history. Discovery publishes an observable request through that owner. Consuming
fire/damage producers and device playback must use the same history; final PCM remains open.

## Reproduction

The required research gate checks complete returns and all 65536 DGROUP bytes outside exact
actor destinations, recovered scratch, bounded call-stack writes and an admitted cooldown
word. It also checks unchanged code, mailbox bytes outside actual transfers, and canonical RNG.
The PM observer independently checks its complete kernel, stack, mailbox and DGROUP.

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_target_discovery.py --originals --review-dir /tmp/wasm-fist-0093-review

Seven required groups cover all class/type tables, complete type-26 mode bytes, flag/link/
operating domains, range/tie boundaries, priority/secondary paths, old targets/orphans,
notification gates/wrap and real release/reallocation. The original corpus gate runs actual
preparation and complete discovery for every current ground actor in all 47 pinned missions
using all eight original height maps at 512/1024/2048/4096 detail. Missing input, incomplete
returns or any skipped group fail.

The consuming C gate adds 1595 independent constructed queries and compares every original
scan with production C, including 3840 queries through actual all-47 C restoration/preparation.
It poisons borrowed raw inputs before scanning, checks complete world preservation outside
the exact owned actor/request destinations, checks the complete height plane and exercises
malformed/truncated/trailing batches and null API inputs. Runtime lifetime values are opaque;
complete observable output is normalized to physical slots for cross-target comparison.

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_target_discovery.py --target native --originals --oracle --review-dir /tmp/wasm-fist-0093-c-native-final
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_target_discovery.py --target wasm --originals --oracle --review-dir /tmp/wasm-fist-0093-c-wasm-final

Use --native-probe with the same production flags plus address/undefined-behavior sanitizers
for the required memory gate. Regular builds deliberately report the optional original gate
as skipped; acceptance requires the commands above with zero skips. The full parent command
bank, target acquisition, living battle, device/PCM consumers and complete final WASM streak
are separate requirements; discovery alone does not complete them.

Use --target all to compare both production targets in one invocation with a single original
replay. Closed WI 0093 records the accepted separate-target and memory results, complete general
build/style regressions and actual native/browser scene checks.
