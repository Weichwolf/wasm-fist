# Automatic ground fire and surface-to-air readiness

Closed WI 0102 proves complete original af97/afa2, 8711/96c0, type-15 allocation,
b8d1 construction and c047 request admission. Both required original gates pass without
skips. Closed 0103 supplies complete shared C consumption; complete parent/class/battle/PCM
acceptance remains open. Frozen engine SHA256:
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.

## Admission and pending fire

af97 returns immediately for classes other than M3/BMP (types 1/3); those two branch directly
into afa2. Automatic parent entries five/fourteen select af97 and entry ten selects afa2.
All three controlled-bank entries select the genuine b111 RET. The common afa2 body reads
the actual platoon descriptor at DS:9796 and rejects behavior word three before reading any
target payload. A null retained target word +97 also returns before payload access.

When actor flag 8 is clear, phase-random low-byte bits 0/1 must both be zero. Flag 8 bypasses
that gate alone. Retained target range +99 at most 200 or control bit 128 bypasses the next
threshold test. Otherwise the phase-random low byte must be below the authored probability
byte indexed by descriptor behavior at DS:9952: 80, 140, 40, 0. Behavior three already returned.
No new random value is drawn here. The raw range is the existing aim owner's scaled word;
these values are not newly inferred physical distances.

A target with flag 16 invokes the genuine M3/BMP readiness service. Other classes continue
to the ordinary pending-fire branch even for that flag. The branch compares wrapped requested
turret offset +8b minus current offset +89. Values 0..181 and 65354..65535 admit; positive 182
rejects while negative 182 admits. A symmetric absolute-value comparison would differ.
Admitted ordinary fire executes complete a286, setting pending byte +92 to 48, then clears
control bit 128 only. It consumes no ammunition and creates no projectile at this boundary.
Missile branches leave that pending byte and control word unchanged.

Malformed descriptor behavior words are preserved by the decoder. The reference tests cover
every word when the threshold index is unused. Shared consumption validates a
used probability index before publication, without inventing an early clamp on unused choices.

## Ordered missile racks

M3/BMP retain two reserve bytes +b5/+b6 and two rack-state bytes +b7/+b8. State four selects a
ready rack and state zero can begin arming by writing state two. Other byte values retain
their actual branches. This command does not advance arming timers or complete rack reload.

| Selection | M3 | BMP |
| --- | --- | --- |
| First ready rack | State four and nonzero first reserve | State four regardless of reserve |
| Second ready rack | State four if first was not selected | State four if first was not selected |
| First rack to arm | State zero regardless of reserve | State zero and nonzero first reserve |
| Second rack to arm | State zero if first was not selected | State zero if first was not selected |
| Dirty components after an arming/ready service | +e0/+e2 = 3 | +c6/+c8 = 3 |

Both racks busy return without component writes. A selected ready rack executes its complete
far launch service even if empty; failures still dirty both components. An empty first BMP
rack remains selected and prevents fallback to a ready second rack. An empty first M3 rack
can instead select the second. Sequential original returns verify this difference.

Complete c296 resets both reserves to one while retaining both rack-state bytes, selected/
loaded station bytes +91/+a5 and pending byte +92. Mission readiness retains those five bytes
and both reserves. The existing `weapons.cycle[2]` already owns the reserves. Shared C gives
the newly recovered rack states one typed owner and reuses the existing reserve storage.

## Complete launch, display and sound request

The selected far service checks reserve, target presence and target flag 16. A valid launch
decrements its reserve before the genuine normal-priority type-15 allocation attempt. Full
short capacity therefore consumes reserve and creates no missile. It uses the existing
first-free physical slot and the first registry entry whose pointer and generation are both
zero; a vacant entry with retained generation is unavailable. Freed holes and the last short
slot follow that ordering. The constructor clears the complete remaining 51 bytes.

b8d1 installs the firer's XY, wrapped altitude plus 3072, absolute turret heading, target
pointer +1a and origin pointer +2b. It writes speed +1c = 160, age +26 = 0, steering word
+28 = 364, stage byte +2a = 0 and secondary flag 1. Primary flags stay zero: the source side
flag is not copied. Flight, steering consumption, collision/damage and retirement are later
methods and remain separate work.

On failure the actual selected display producer (firer equals DS:6d34) stores duration 120
and an original text handle: NO SAMS ARMED, NO AIRBORNE TARGET! or MISFIRE!. Other firers
preserve the entire display state. Successful launch calls complete c047 with selector 24.
Only a firer matching sound-source pointer DS:9fdf records selector 24 in DS:9fdd and emits
the authored op-64 packet: AX = 7, DL = 0, ECX = 0, mailbox source EBX = 24. The reference
adapter also checks the inherited high DX byte from its declared caller register fixture.
This is c047 admission, separate from bf3c's voice/side/mute/cooldown rules. No audio result
is consumed after the device call. PCM playback is still required by the full rewrite goal.

The original near target pointer has no lifetime check here. Actual release/reuse tests show
a deleted airborne target slot can become a missile targeting itself; a reused slot can be
read as the successor. Shared consumption resolves the exact live canonical reference
only when its payload is used. Existing acquisition/aim/clearing owners remain responsible;
release/reuse never grants permission to borrow the successor's payload.

## Verification boundaries

`automatic_fire_contract.py` predicts the complete DGROUP result before original execution.
`original_automatic_fire_oracle.py` runs unchanged instructions and verifies every byte outside
the bounded 64-byte call-stack scratch, full code/text/mailbox and actor/CS/DS/SS/near return.
No instruction hook or nested gameplay replacement is installed. The only host boundary is
the actual admitted unconsumed op-64 device request. Controlled bank RETs execute unchanged.

The required main suite passes six groups without skips in 697.877 seconds: complete word/
flag admission domains; both complete rack-state byte pairs and reserve/target gates;
real allocated targets of all 28 types and
actor registry orphans; genuine capacity/hole/reserved-binding/wrapped constructors; both
genuine parent banks and counter/phase RNG/global admission; and all 47 pinned missions/eight
height maps at four actual prepared details. Constructed stimuli and the original unsafe
release/reuse demonstration are distinguished from ordinary prepared mission returns.

The separate required retention gate passes without skips in 7.037 seconds. It checks
whole-DGROUP class initialization and readiness in 1536 contexts: both classes, three link
modes, full retained byte domains and all four RNG cursors.
It verifies 3072 complete returns, code/text/mailbox and genuine registers, using the existing
initialization/readiness models. Production C, programs, softgl and original files are unchanged.
No full-game acceptance or consecutive complete-game WASM run follows from this research.

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_automatic_fire.py --originals --review-dir /tmp/wasm-fist-0102-review
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_automatic_fire_retention.py --originals --review-dir /tmp/wasm-fist-0102-review

The main gate verifies 608968 complete returns, 29439 genuine parents and all 15 required
branches, including 22275 real missile allocations and 16110 actual audio request boundaries.
The all-47 prepared corpus contributes 30720 child/parent returns through 188 worlds and
3840 ground actors. Main output SHA256:
`4cdde6d200e53e379fee17725faff7fc6a1bdb28ea5124a3e4a7cb6f49fe6221`.
Retention SHA256: `ce97ae530fdfd31862076c143c2e6078e76936b2a3b5963d6a6ebea6dc05284d`.
Exact commands, complete corpus/source/program/original/reference pins and terminal results
remain in `/tmp/wasm-fist-0102-review/receipt.json`; obsolete owned logs/helpers are cleaned.
Initial pilots are supporting only, superseded by the complete required run. No C, build
configuration or dependency changed; accepted production/style gates remain those of 0101.
Closed 0103 delivers the complete shared C consumer below; full-game WASM streak remains zero.

## Verified shared C delivery

WI 0103 owns complete af97/afa2 and direct M3/BMP readiness in
`src/sim/automatic_fire.c`. The child consumes the parent's already-drawn phase word;
it advances neither RNG nor the parent counter. Rack states have one owner beside the
existing reserve bytes and survive saved restoration, class defaults and preparation.

The transaction publishes only proved actor/component writes, an actual normal-priority
type-15 allocation, the selected typed notice and admitted c047 logical request. Used
invalid descriptors or stale/reused target identities preserve the entire world and output.
Unused malformed fields retain their original early returns. Successful construction owns
complete pose/velocity/projection/flags/stage state and exact live origin/target references;
`fist_mission_world_view` borrows that canonical payload. The sound-source slot comparison
is distinct from selected-display ownership and bf3c voice history. PCM remains pending.

Required paired native/WASM verification passed six groups with no skips in 866.222 seconds:
579544 complete callbacks per target, 579531 unchanged original returns, 7680 canonical
returns through all 47 missions, eight height maps/four details, 188 prepared worlds and
3840 ground actors. Full word/flag admission contributes 410880 callbacks, rack/reserve
coverage 146432, actual target types/orphans 14392, allocation/wrapping/sequences 141,
invalid/ignored dependencies 19 and the canonical corpus 7680. There are 21119 actual missile
constructions. Output SHA256:
`6da10bd3a0e3971cca1930f87a75472be53a7e9de084c5ed265c80fe9940e673`.
Source records are poisoned; complete actor/components/pool/results are compared and all
other world bytes checked. Each fire batch also verifies 1536 initialization/readiness
retention contexts and real release/reuse atomic failures, including unused stale targets.

The complete ASan/UBSan required gate passed the same six groups/counts/branches/hash
without skips in 1153.407 seconds. Strict LLVM 19.1.7 format/tidy passed 91 owned translation
units. The full production build passed 45 native tests, 43 WASM Python suites and both
WASM Node renderer checks with requested flags and owned-code Werror. Optional corpus
omissions in default regression suites do not replace the separate required no-skip gate.

Actual TRAIN1 SDL/browser and sanitized SDL scenes pass their complete device/input/frame/
pause/focus/shutdown checks; native/browser after frames were visually reviewed. A separate
explicit inspection constructs a real missile from each ready class, borrows its canonical
mission view and renders the original SHOT model with the existing compositor and softgl.
All six native/WASM/sanitized fixtures show the actual constructor pose and 2656 changed
pixels over the same-camera terrain baseline. Inspection camera/texel settings are declared
visual choices, with source and reproduction preserved. This proves constructor presentation;
parent scheduling, arming timers, missile flight and live-game entity-renderer integration
remain separate work. No cross-target pixel identity is required.

    CTEST_PARALLEL_LEVEL=4 bash tools/build.sh all
    python3 tools/check_style.py
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_automatic_fire.py --oracle --originals --review-dir /tmp/wasm-fist-0103-review

Compact source/program/original/reference pins, complete terminal results, sanitizer/scene
reproduction and reviewed frames remain in `/tmp/wasm-fist-0103-review/receipt.json` and
`constructor-scene.json`; obsolete owned builds/logs/captures are removed after acceptance.
The original oracle/retention files and immutable reconstruction remain unchanged. Continue
complete original ae5c/b0be recovery under WI 0104, then selected b152 and the full parent/
class contract. Full 0081, living battle/PCM/outcomes and independent complete-game WASM
acceptance remain open; the full-game streak is zero.
