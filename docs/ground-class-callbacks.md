# Ground class callbacks

This is original-only recovery for active0081. Complete shared class scheduling,
combat and PCM are not accepted by these callback observations.

## Conditional ammunition assignment

All four class phase tables select the following complete callback when the
advanced phase byte masked with0x1e equals12 (table slot6).

| Class | Table | Callback | Actor word | Assigned value |
| --- | --- | --- | --- | --- |
| M1 | 7c91 | 7cb2 | +ad | 15 |
| M3 | 8858 | 8879 | +b1 | 75 |
| T80 | 90a0 | 90c0 | +ac | 90 |
| BMP | 984e | 986e | +ad | 400 |

Each complete callback tests actor byte+16 bit0x08. If set, it overwrites the
listed word with the immediate value; otherwise it returns without changing
memory. This is assignment, not saturation or an increment. Preserve this exact
condition before giving the bit a gameplay name. These stores are separate from
the existing reload timer and surface-to-air rack owners.

The unchanged original dispatch-index instructions pass all256 phase bytes for
each class (1,024 prefixes): BX equals phase&0x1e and memory is unchanged. The
pinned tables select the listed callback at byte index12. Every callback passes
all256 flag bytes and eight ammunition words (0,1,14,75,90,400,32768,65535):
2,048 complete near returns per class,8,192 total. Whole0x60000 memory matches
an independent prediction outside the explicitly written two-byte return slot;
actor, DI, DS, SS and final stack position are checked. No original instruction
patch, hook or substituted callback is used.

Output digest: d237edffcb2cff8cabffd5a0158ef794515883ed47f2dd878fdf87a56f8e3e3a.
Original image digest:
d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5.
The pinned image comes from immutable reference/reconstruction-v1 via
tests/reference_images.py. Receipt and callback disassembly are under
/tmp/wasm-fist-0119-class-research/ammunition-refill.json and ammo-0..3.txt.
Reproduce with:

```sh
PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  /tmp/wasm-fist-original-ground-ammunition-refill.py
```

## Reached class difference

The real saved AZER7 world supplies actual allocated and selected actors for
each class. Complete original class-entry execution through the pre-engine
boundary agrees with the existing composed model for the initial M1/M3 update.
T80 differs only at+ac (15 predicted,90 observed), and BMP only at+ad (44
predicted low byte,144 observed; the complete word is400). Both reach phase12.
All other payloads remain unchanged. The independent callback proof above
explains these first differences; this four-context discovery does not prove
all class phases or target-aware class integration.

The first-difference fixture and receipt are
/tmp/wasm-fist-original-ground-class-first-step.py and
/tmp/wasm-fist-0119-class-research/first-step-development.json. Its targetless
contexts must not be substituted for the required targeted-class gate.

Remaining complete class composition, phase admission, contacts,
fire/input and engine queue composition still need complete class/world proof
before shared C acceptance. In particular M3/BMP callbacks8886/987b inspect
simulation tick6cde with mask0x0ff0; that global is not a device-input word.

## Reached complete instruction cycles

The widened real AZER7 class test now executes all four allocated classes through
128 consecutive updates at four constant-height details, with saved and explicitly
constructed caller states: 32 contexts, 4,096 complete original class returns,
512 command parents, every controlled entry in every context, and 4,096 retained
actual motor-audio kernel returns. Actor/RNG predictions, unchanged other saved
payloads, genuine contacts and visibility operations all pass. The motor tail
uses each actual class callsite and the existing independent queue predictor.

The first widened T80 attempt stops at update 6, phase 22, on an unconfigured
op-58 visibility call. Observational tracing identifies common callback a904.
When the phase's high bits are zero and actor byte+16 bit0x08 is set, this callback
decrements nonzero byte+36, increments byte+37 modulo 256, then reads physical
roster slot (new byte+37)&15. Null and type-23 candidates return immediately.
Otherwise the existing source/target-height visibility owner is called; a visible
result sets byte+36 to255. The independent prediction and genuine PM return now
cover this reached branch alongside existing parent visibility: 448 transfers total.
The original missing-provider attempt remains failure evidence; address-zero/one
returns are still forbidden, and no original instruction is replaced.

Combined digest:
d1ce72f85ad453f37d291f9ff355bbbae793e65899cf77b19717cd5b92d4ff77.
Receipt: /tmp/wasm-fist-0119-class-research/reached-ground-class-cycle128-development.json.
Reproduce with PYTHONPATH=tests:/tmp and the pinned oracle environment using
/tmp/wasm-fist-ground-class-cycle128.py; the bootstrap/engine adapters and their
hashes are recorded in the receipt.

These complete instruction returns check the declared actor/RNG/payload and PM
contracts, not every pre-engine DOS global or every possible class branch.
Independently predict remaining global writes and full callback domains before
class C acceptance. No shared-class scheduling or PCM playback is accepted here.

## Shared status timers

All four pinned class tables select a9a0 at byte index24. The complete callback
returns unchanged when actor phase byte+3d has any high bit0xe0 set. Otherwise it
processes byte+19 and two distinct byte counters in this order:

1. If flags&0x06 is nonzero, increment counter+a6 modulo256. When the new value
   is at least64, clear that counter and flag bits0x06. Request logical voice27
   through bf3c only when the resulting flags do not contain0x10.
2. If the resulting flags contain0x10, increment counter+52 modulo256. When the
   new value is at least112, clear that counter and flag0x10, then request voice35.

The comparisons follow the byte increment:255 wraps to0 without expiring. The
second branch observes the first branch's flag changes. At most one logical
voice is requested in a complete return. These offsets are separate state owners;
do not infer timer units or gameplay names from the constants alone.

Independent predictions match149,504 complete unchanged original returns:
18,432 phase/flag/counter-edge combinations,65,536 complete flag/turn-counter
combinations and65,536 complete flag/immobile-counter combinations. The fixture
observes43,392 voice27 and52,000 voice35 entries. It predicts all0x60000 memory,
including the near/far call scratch words, and checks DI/DS/SS/SP/CS and the voice
input/stack. Each actual bf3c admission branch executes its real far return in an
explicitly muted device context; no instruction replacement, PC skip or fabricated
return is used. These logical requests do not prove admitted PCM playback.

Output digest:
87c867141d5fe3ecd618fafa2cfc6e28772dfe9f76233715ad65ec9d2b5262f1.
Fixture digest:
74d72e1b49db472b3782181d669b1b1b6fa936775bc534a10f2f1283e14f475a.
The original image digest is the same as the ammunition proof above. Receipt:
/tmp/wasm-fist-0119-class-research/status-timers.json. Reproduce with:

```sh
PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  /tmp/wasm-fist-original-ground-status-timers.py
```

This original-only checkpoint accepts neither shared class C nor world scheduling.
Complete class composition and every pre-engine global write still need
independent proof.

## Complete physical-roster visibility

The complete a904 domain now passes81,120 unchanged original returns with actual
normal allocations:65,536 phase/flag pairs,12,288 counter/cursor/candidate
combinations,1,024 self candidates,224 all-type/range combinations and2,048
type-26 variant/range combinations. A live candidate in the upper half of the
physical roster proves those slots are not consulted. Null and wreck candidates,
byte-counter wrap and every actor class are checked. Genuine visibility returns
include5,228 visible and4,204 non-visible results using the existing height owner.

The predictor checks all DOS/image/other-object memory outside the PM mailbox;
the original child provider independently checks every mailbox/external/code
write and the real numerical kernel result. Near-call scratch words and the
return ABI are exact. The initial failed test predicted an extra stack write at
8ffa. Actual e21c/e25c/e286 pushes reuse8ffc; the declared PM provider stops before
CALL e339. Correcting that independent expectation preserves the whole-memory
guard. Failure evidence is retained in roster-stack-expectation-failure.json.

Output digest:
201149198fa6eae42a89901bdd5cf15f1a78cbee073e06948ee87f8d89dfaa85.
Fixture digest:
b549acf72cf830511454e0d64a46736c8fc677763ad8003def444c05633f4541.
Receipt: /tmp/wasm-fist-0119-class-research/roster-visibility.json. This complete
callback proof does not accept shared class C, full scheduling or the game.

The exact verified status/roster fixtures are versioned under
tests/fixtures/ground_class_callbacks. They deliberately execute every required
group without filters and write receipts under /tmp. With the pinned oracle
environment, reproduce directly from the repository:

```sh
mkdir -p /tmp/wasm-fist-0119-class-research
PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  tests/fixtures/ground_class_callbacks/status_timers.py
PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  tests/fixtures/ground_class_callbacks/roster_visibility.py
```

## Counter restoration and retention

The recovered byte fields+37 (roster cursor),+a6 and+52 are retained by both
complete c296/class initialization and each ground readiness method. The existing
+36 owner is distinct: readiness clears it. A required4,096-return proof covers
all four classes, both declared link inputs0/2 and every byte value of each new
field. Complete251-byte payload predictions, original return ABI and RNG match:
2,048 class-start returns and2,048 readiness returns. No field is inferred zero
merely because the current saved scene happens to contain zero.

Output digest:
e1c4c6eb2a4649a51852bc992172d813d8d3d240269951de46486f1ca7f45881.
Fixture digest:
5afa7339acb0166f82c2774b0a212d3d4eb0741c8421158a727a385aebdb06cd.
Receipt: /tmp/wasm-fist-0119-class-research/callback-retention.json. This payload
retention proof does not accept every class/global write or shared scheduling.
Reproduce with the same pinned environment and review directory above:

```sh
PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  tests/fixtures/ground_class_callbacks/callback_retention.py
```

Shared consumption must add the three missing bytes to typed saved restoration
and preserve them through initialization/readiness. Use drive.motion_flags for
+19 and the existing reset_state for+36. M3/BMP+bb already belongs to
weapons.ready_stock; their secondary rounds are existing weapon slots0/2. A
second pending-flag or reserve owner would duplicate canonical state.

## M3/BMP phase refill

The pinned M3 table selects8886 at byte index20; BMP selects987b at index16.
Both complete callbacks return unchanged when simulation tick6cde&0x0ff0 is
nonzero. The admitted branches use byte+19 bit0x01, word ammunition and byte+bb:

| Class | Ammunition | Capacity | Component refreshed to3 | Canonical rounds slot |
| --- | --- | --- | --- | --- |
| M3 | +ad | 2 | +d6 | 0 |
| BMP | +b1 | 4 | +e4 | 2 |

If the pending bit is clear, nonzero reserve and ammunition unequal to capacity
set it and return without loading a round. If it is already set, the original
loop loads until ammunition equals capacity or reserve reaches zero. Each round
clears the pending bit, increments the word modulo65,536, decrements the byte
reserve, requests logical voice23 through bf3c and refreshes the listed component.
The loop continues after clearing the bit. Equal-capacity/empty-reserve exits
retain a previously set bit; an over-capacity word is not clamped. Tick inhibition
also applies to an already pending refill. These methods do not service the
existing surface-to-air rack state/reserves at+b5..b8.

Independent whole0x60000 memory predictions match291,328 complete unchanged
original returns without filters: every tick word for both classes (131,072),
every ammunition word (131,072),26,624 full-flag/admission/ammunition/reserve-edge
combinations and2,560 complete reserve-byte/ammunition-edge combinations. The
fixture observes210,906 real voice23 near/far return paths in explicitly muted
device contexts. Exact call scratch, voice inputs, actor/segment/stack return ABI
and all other memory are guarded. No original instruction or return is replaced;
PCM playback and complete shared class scheduling remain open.

Output digest:
ecff15dc4a8b4ce7838011358914890712e3fb5d9f9d8b955a1db655a12bac0a.
Fixture digest:
36e4f8e94fd223d0054561569c074d73d96a832ae38c8734eef301fbaf4a357d.
Receipt: /tmp/wasm-fist-0119-class-research/secondary-rack.json. Reproduce with
the same review directory and pinned environment:

```sh
PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  tests/fixtures/ground_class_callbacks/secondary_rack.py
```

## Smoke phase admission and status decay

All four pinned class tables select a46e at byte index28. It invokes the existing
19caa smoke creator with extent base768 when phase byte+3d has no bits0x60 set
and operating byte+1a has bit0x40 set. Phase0x80 therefore admits this callback;
the0xe0 inhibition used by the other common timers does not apply here. After
the conditional call, every return shifts unsigned status byte+96 right by one.
The constructor belongs to fist_drifting_smoke_create, not a new damage owner.

Independent whole0x60000 memory predictions pass69,632 complete unchanged
original returns:65,536 phase/operating-flag pairs and4,096 status-byte/class/phase
combinations. The explicit smoke setting is0. All11,264 admitted paths execute
the actual producer's disabled-setting comparison and far return. Actor state,
AX and DI/DS/SS/SP/CS, near/far return scratch, unchanged RNG/pool and all other
memory are checked. No instruction replacement or substituted return is used.

This proves admission and decay with smoke explicitly disabled. Enabled
constructor coupling, pool exhaustion, source lifetimes, shared class C and
world scheduling remain required; this fixture accepts none of those scopes.

Output digest:
dcd343dedaf9b8a9951e68b01f70bee48354fba6d8bc50e2157cfb619ef3010c.
Fixture digest:
491b5792c7dcb1fba42ca92bc7631ae63792a04babeab5f1d7c6a7dd83468825.
Receipt: /tmp/wasm-fist-0119-class-research/smoke-phase-disabled.json. Reproduce
with the same review directory and pinned oracle environment:

```sh
PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  tests/fixtures/ground_class_callbacks/smoke_phase_disabled.py
```

## Smoke constructor coupling

A second required original fixture uses actual allocated ground actors in all
four classes, rather than a standalone payload pointer. It independently predicts
the complete smoke payload, pool/registry state, RNG and caller status shift.
It checks3,860 complete a46e returns:1,024 setting-byte cases,1,024 phase-byte
cases,604 short-pool occupancy cases,1,024 status-byte cases,128 RNG contexts,
40 coordinate/wrap cases and16 extended-pool occupancy contexts. The exercised
paths include3,092 actual constructor calls,1,948 creations and124 low-priority
refusals. Disabled/refused calls preserve RNG; every other actor payload remains
unchanged. Creator output captures source XYZ, adds768 to altitude with signed
word-pair wrap, starts heading at0 and retains the existing random-extent owner.

Full0x60000 memory also matches a paired invocation of the unchanged constructor
from its declared far-child entry snapshot, followed by the independently
predicted byte+96 shift. This proves caller composition, including exact stack
scratch, rather than independently predicting every constructor scratch byte.
Original code/external memory and DI/DS/SS/SP/CS are checked; no original code or
return is replaced. The existing constructor's semantic predictor remains the
single test owner; class composition does not invent another smoke algorithm.

An initial attempt to feed the existing destruction C probe fails because that
probe requires a short-pool source while actual ground actors occupy the extended
arena. The valid probe guard is retained. The failed input/fixture/log and compact
failure receipt remain under/tmp; this original-only checkpoint does not claim
native/WASM class acceptance, complete class scheduling or PCM playback.

Output digest:
c3834baad51db3cb91bc5234baf69791355b41f6052849d9d461c59c737e9ab7.
Fixture digest:
947b486c4a314aae9c21a403ccf94113e6a5b35ffba003216c1966a317a86e60.
Receipts: /tmp/wasm-fist-0119-class-research/smoke-phase-coupled.json and
smoke-phase-coupled-short-probe-failure.json. Reproduce with the pinned oracle:

```sh
mkdir -p /tmp/wasm-fist-0119-class-research
PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  tests/fixtures/ground_class_callbacks/smoke_phase_coupled.py
```

## Complete phase and manual elevation family

All four phase tables select a202 at slots1/4/9/13 (byte indices2/8/18/26).
Motion byte+19 bit0x20 raises; otherwise bit0x40 lowers; neither bit returns
unchanged. Raise takes precedence when both bits are set. The phase callback
saves shared step word9602, substitutes held-input word9604, executes the complete
raise/lower helper and restores9602. Shared previous-clock word9600 still updates.

The common acceleration helper first updates9600 from clock word0452. Unsigned
wrapping elapsed values below20 increment the active step only when it is below364.
Elapsed values of20 or more reset it to18. Already larger step words are retained
on rapid input. The phase helper's temporary increment does not persist in9602.
This recovery does not infer physical units or replace the original clock producer.

Before changing elevation, a2a8 clears nonzero target word+97 and resets elevation
word+38 to0. With no target, it retains the old elevation. Raise adds the step with
16-bit wrap, then clamps signed results above9100. Lower subtracts with16-bit wrap,
then clamps signed results below-5460. The other bound is not clamped. Manual
a1e9/a26d retain the adaptive step. Quick lower a265 sets9602 to728, skips clock
acceleration and performs the same lower path. Center a25b clears the target in
the same manner and always clears requested relative turret offset+8b; it leaves
clock/steps unchanged. No allocation, voice, RNG or target dereference occurs.

Independent whole0x60000 memory/AX/BX/DI/DS/SS/SP/CS predictions pass365,056
complete unchanged original returns using actual allocated actors:24,576 complete
flag/context cases,131,072 elevation-word cases (both directions),131,072 held-step
cases (both directions),65,536 wrapping elapsed-clock values and12,800 manual
helper/edge cases. All required groups run without filters or substituted returns.
Exact original stack scratch, other actor payloads, pool, RNG and input words are
guarded. This accepts original behavior; shared C/native/WASM/full-class/input
and canonical clock consumption remain separate gates under0122/0121.

Output digest:
4121a05727e9831f30d7ac3655987c25fc64c636db9b40d177f88021c4a1aec0.
Fixture digest:
e4f1635160926b3174190e10fa7bb8b0aec88c2827d90a8ec63d446dadde5942.
Contract digest:
6f9ea76c5847c7ed05496e34d4b5fdf368b9310b4e7476657831f06c5a20cbbe.
Receipt: /tmp/wasm-fist-0122-review/original-elevation.json. Reproduce with:

```sh
PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  tests/fixtures/ground_class_callbacks/elevation.py
```

The separate elevation_shared.py fixture runs nine actual original calls across
the four allocated actors with one retained controls owner. It never resets the
shared controls between calls, and predicts all memory and every return ABI.
Phase/manual/quick-lower/center calls, unsigned clock wrap and target cancellation
produce shared controls(0,728,18) and elevations(37,0,-585,-709). The C probe repeats
the same sequence through one shared controls pointer. This does not accept the
complete class or the device-clock producer.

Output digest:
4edda9f63c35cc2093bc77b82aba21f66dfe8c2d5edc5beb0d746b9861e037a1.
Fixture digest:
7d5147fb75ebb8d4403111d0c3f6472a9ba67c053b15fe889b5ffd12394a7246.
Receipt: /tmp/wasm-fist-0122-review/elevation-shared-original.json. Reproduce with
the same pinned environment and PYTHONPATH using
tests/fixtures/ground_class_callbacks/elevation_shared.py.
