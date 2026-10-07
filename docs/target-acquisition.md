# Target acquisition and later aim feedback

Closed WI 0096 recovers complete automatic ae32 and direct a6e3 from unchanged original instructions.
It is an original behavioral checkpoint for the shared C consumer required by 0081. Existing
0093 discovery publishes a candidate; acquisition separately decides whether to install it.
A complete living parent, playable battle and PCM are still pending.

## Installation and conditional randomness

Automatic ae32 first requires nonzero discovery counter byte +94 and a null current target
word +97. Only then does it draw one original 0291 value. Its low byte is compared unsigned
with the PINF behavior's actual DS:994a byte: 120, 200, 40 or 0. Equality admits an attempt.
Earlier rejection does not read the behavior or consume RNG. Valid behavior choices are the
four original UI values already recovered by command selection.

An admitted attempt calls a6e3 with candidate +9d and sets control bit 128 after it returns,
even if the candidate is null, deleted, ineligible or invisible. It does not test automatic
control bit 1 itself; caller scheduling supplies that distinction.

Direct a6e3 rejects null, target secondary flag 64 or target deletion flag 1, retaining an
existing target on those branches. It otherwise installs the candidate first and executes
complete e20a visibility through the real protected-mode op-58. Failure then clears +97,
including an earlier valid target. There is no side, preference, discovery-range or registry
membership filter at this direct boundary. A live physical orphan and self can be installed.
Class/variant source and target heights are the same actual tables used by discovery; reuse
that shared aim-position/visibility owner in C.

## Selected display and logical voice

Only a selected actor with a visible installed target publishes a message. Actual f69:ae12
(physical 1a4a2) stores the text handle at DS:96a0 and duration 120 at DS:969e. It does not draw
the message or call a device. Friendly target messages come from GS:2de7; enemy target messages
from GS:2daf. Ground labels distinguish M1, M3, T80 and BMP; aircraft distinguish APACHE/HIND;
artillery has a lock message, other types use TARGET LOCK. Both banks' type-26 sentinel selects
GS:2e1f's four FUEL TANK/SATELLITE DISH/PROPANE TANK/BUNKER messages. The actual strings reside
in the original GS data segment at physical 2d740; code-segment zero is not that data owner.

Enemy target voices come from DS:9702: 15/16/15/16 for the four ground classes, 17 for artillery
and 18 for the other types. Friendly targets use 20. Complete bf3c applies the existing shared
voice gate, selected actor, actor side and unsigned wrapped 30-tick cooldown, then emits op-64
with AX=0280 OR voice, DX=clock high byte and ECX=0. The observer verifies the actual request
and resumes its caller's tail, which consumes no device result. Playback/mixing remains open;
C must use the existing voice-history owner rather than a second cooldown.

Visibility masks the type-26 mode by 3. The message dispatcher instead uses the whole mode
byte as a word-table index. Constructed live values above 3 read adjacent GS data. The
required checkpoint covers the complete byte domain and records this operand distinction.
The six actual saved mode-7 objects in TRAIN1 already have deletion flag 1 and are rejected;
complete bcbf readiness also marks modes with bit 4 deleted. This is an unchecked operand
proved on constructed inputs, not an observed bad message during an untouched saved mission.
A safe typed C message must not copy arbitrary adjacent text addresses as gameplay data.

## Later geometry and throttle consumption

The complete f69:abd5 method (physical 1a265) returns immediately for a null target. Otherwise
it prepares source/target aim XYZ and calls real a18e/0578. That helper returns horizontal
bearing in AX, elevation in BX and the complete planar distance in CX:DX. Elevation uses the
signed wrapped Z difference and retained planar distance through the existing angle owner.

The class helper stores target bearing at +9b, elevation at +38 and packed unsigned range
at +99. The range is distance bits 8..23: actual SHL EDX,8 followed by MOV DL,CH. This is
independent of navigation +53. It is not produced by acquisition or weapon-selection callback
ae5c; the latter selects an available class weapon before calling its class setter.

Reached mode-6 ad2f consumes that +99 through the accepted 0095 owner, including -48/0/80/240
bands at 45/60/90. C must reuse the existing turret.elevation owner for +38, retain target bearing +9b and
reuse the established
geometry, physical reference, descriptor, RNG, throttle and profile owners.

## Verification

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_target_acquisition.py --originals --review-dir /tmp/wasm-fist-0096-required
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_acquisition_parent.py --review-dir /tmp/wasm-fist-0096-parent
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_acquisition_observer_cache.py --review-dir /tmp/wasm-fist-0096-cache

The required gate checks complete near/far returns, all 65536 DGROUP bytes outside exact
actor/global/scratch/bounded-stack destinations, unchanged instructions and GS text/tables,
mailbox preservation and exact RNG. Real visibility runs the unchanged installed PM kernel.
A filtered --test invocation is supporting evidence only and cannot produce acceptance.
The observer evicts Unicorn translated blocks when entry/stop/segment changes; identical
boundaries reuse immutable instructions. The separate required cache gate compares complete
results and full DGROUP/code/GS/mailbox against the existing unconditional-eviction observer,
including repeated boundaries, real visibility, admitted voices and near/far changes.

Required coverage includes all RNG words/four behavior thresholds/cursors, discovery and
old-target gates, all ground/target classes and both target flag bytes, all variant bytes,
selected/side/voice/cooldown boundaries, wrapped spatial geometry, actual release/reallocation
and orphan candidates. Corpus coverage requires all 47 pinned missions on all eight real
original height maps at 512/1024/2048/4096 details, with actual preparation and complete
discovery -> acquisition -> aim -> throttle child returns. These are declared reaching child
boundaries, rather than untouched complete parent/living ticks.

A separate required parent gate executes complete ab03 at the genuine low-four-bit entry
eight in both automatic and controlled banks, all four classes/behavior choices and all RNG
bytes. It checks the unconditional parent draw, conditional acquisition draw, descriptor/
route resolution, actor counter, visibility and complete return. The diagnostic-selected
pointer is explicitly zero; other parent entries and actual diagnostic rendering remain open.

The complete required child gate passes eight groups without skips in 422.341 seconds:
276848 acquisitions, 12825 aim returns, 5804 actual visibility transfers, 248 admitted requests
and 8504 consuming throttle returns. The all-47 corpus has 188 prepared worlds/3840 ground
actors. Output SHA256: a7010fad9d8ddce1c4daf7a1f5fa64a8c2720c36ff756db995a10dd29d7b7817.

The genuine-parent gate passes 16384 complete returns without skips in 18.646 seconds; the
paired-observer gate passes 7168 complete sequences without skips in 39.075 seconds. The first
terminated full attempt and failed/filtered pilots are excluded. Exact hashes, commands and
results remain in /tmp/wasm-fist-0096-review; see board/closed/0096_original-target-acquisition.md.

Shared C acquisition/aim is open 0097; complete 0081 remains pending. Production source,
configuration, current scene artifacts, originals, immutable reference and pinned softgl are
unchanged. No new build/style/rendering or audible PCM acceptance is claimed by this original
checkpoint. The independent complete-game WASM streak is zero.
