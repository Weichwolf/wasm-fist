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
