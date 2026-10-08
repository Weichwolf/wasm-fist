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

Remaining class tables, secondary gun/rack callbacks, phase admission, contacts,
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
The remaining secondary rack callbacks and every pre-engine global write still
need independent proof.

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
