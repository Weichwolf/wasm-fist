# Complete original ground command parent

`tests/test_original_ground_phase.py` executes unchanged `ab03` through its genuine
final return. This is the reference gate for consuming the complete parent in shared
C. It does not accept the current C candidate or the complete living-class scheduler.

The frozen engine image is provisioned by `tests/reference_images.py` under `/tmp`.
Its SHA-256 is `d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
Both callback tables, every indirect call and the selected diagnostic execute without
instruction patches, fake returns or instruction hooks.

## Parent boundary

`ab03` saves the physical actor in `9a08`, executes the real `0291` RNG before testing
`978a`, and stores the returned word in `978c`. Every nonzero inhibition byte skips
the bank but retains this random consumption. A skipped bank does not read the actor's
platoon. The independent model in `tests/ground_command_phase_contract.py` checks the
whole prefix, including all four random words, cursor and stored random result.

An admitted call increments byte `+42` with byte overflow. A new low nibble of zero
samples hull word `+26` and old history words `+28/+2a/+2c`. Each operand is sign-extended
before the four additions; a logical 32-bit right shift by two supplies average word
`+2e`. The old second/first history and current hull become the new third/second/first
history. The heading test sweeps all 65,536 values of each of the four input words,
with mixed signed boundary values in the other lanes. It retains the real `ab82`
selector after each sample and covers both low-nibble and whole-byte wraps. This is
not an exhaustive enumeration of all four-word combinations.

The admitted prefix reads the eight-entry descriptor/route pointer tables and sets
`9796/9798`. The following complete banks are verified against the pinned image:

| Index | Automatic | Controlled |
| --- | --- | --- |
| 0 | `ab82` command selection | `ab82` |
| 1 | `ac75` route/formation goal | `ac75` |
| 2 | `ab88` bearing/range | `ab88` |
| 3 | `ad2f` throttle/profile | `ad3b` profile |
| 4 | `ad08` route progress | `ad08` |
| 5 | `af97` missile-class automatic fire | genuine `b111` RET |
| 6 | `b011` ordered target discovery | `b011` |
| 7 | `ae66` maneuver | genuine RET |
| 8 | `ae32` acquisition | genuine RET |
| 9 | `ae5c` station selection | genuine RET |
| 10 | `afa2` all-ground automatic fire | genuine RET |
| 11 | `b017` idle turret | genuine RET |
| 12 | `b0be` retreat support | genuine RET |
| 13 | `b053` physical roster promotion | `b053` |
| 14 | `af97` missile-class automatic fire | genuine RET |
| 15 | `ae66` maneuver | genuine RET |

## Complete composition and device observations

The observer checks the independently predicted prefix immediately before the
original indirect CALL. It then executes that CALL and every nested child instruction
through the actual parent continuation. A separate original machine runs the complete
expected child with the same prefix state, general/segment registers and stack depth.
All 65,536 DGROUP bytes and external memory must agree between child and parent.
The previous complete child contracts supply independent behavior evidence; this
additional comparison proves their composition and retained caller context. It does
not replace every child with a new independently authored behavioral model.

Discovery/acquisition actor state, fire allocations and station/support whole-state
predictions are additionally compared with their existing independent models.
Every child independently guards immutable DOS code/text and external memory. The
mailbox may change only to the complete predicted visibility operands and actual
pre-thunk EBX transport. `e291/e2cc` store EBX before `e299/e2d4` replace its low word
with operation `58/64`; sampling EBX after that replacement would test the wrong value.

Actual op-58 visibility executes the pinned kernel on the real installed height plane
and must agree with the independent visibility model. Smoke construction executes
actual op-54 height and its guarded kernel boundary. Admitted op-64 requests execute
the complete kernel queue operation with pinned read-only sound banks; every returned
kernel byte/register is checked by its existing audio oracle. Both shadow and genuine
parent must produce identical complete PM transcripts. This verifies queue/return
coupling, including consumed smoke returns; final PCM mixing/playback remains open.

The existing child stack guards cover `8fc0..9004`; the parent adds a two-byte indirect
return. Only their bounded union `8fbe..9004` is excluded from byte equality. Actor,
DS/SS, CS and final SP are checked explicitly. After the child, the actual parent
executes the complete selected `b152` diagnostic when `7ae0` matches the actor. The
independent panel model checks all characters/attributes and persistent text globals.
The genuine near relocation and text service setup are supplied by closed WI 0110.

## Required run and scope

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_ground_phase.py \
  --originals --review-dir /tmp/wasm-fist-0111-review
```

Missing original files, wrong image/dependency versions, missing output, incomplete
returns, skipped groups and absent coverage fail. The seven required groups include:

- All counter bytes, four ground classes, eight platoons and both complete banks.
- Every heading input word value with complete selection and both wrap boundaries.
- Every nonzero inhibition byte, four random streams and unused invalid platoons.
- Retained complete-bank sequences and conditional additional RNG consumption.
- Consumed support with actual height and enabled/disabled/missing/busy audio contexts.
- Discovery/acquisition/station voices and source-matched missile audio tails.
- All 47 actual prepared missions, eight real heights and four details, executing
  all 32 parent entries for every one of the 3,840 prepared ground actors.

The required run passes all seven groups without skips in 1,913.405 seconds. It checks
409,792 complete parent returns, 405,600 complete separate child returns, 271,132 heading
samples and 134,008 selected diagnostics. The corpus accounts for 188 complete prepared
worlds and 122,880 canonical parent returns. There are 140,424 actual visibility, 32
constructor-height and 88 kernel audio returns; the separate shadow replay repeats
these operations and is not counted as another parent. Output SHA-256 is
`68dd2ee3df2c8b10d6c8dafd9c233127e087c5783fef463f859c6f2636115e90`.
Closed WI 0111 records exact scope. Development pilots and interrupted runs are not
acceptance. Reference-native/WASM C consumption remains a separate required gate.

Next, compare the complete shared C parent against these original observations on
both production targets, including typed reference lifetimes, proved child repairs,
logical events and atomic late failures. Keep the existing single actor/orders/RNG
owners. `97ee` remains an explicit displayed caller value with an unknown producer;
do not invent its meaning. Complete living-class scheduling, battle, UI/device/PCM,
mission outcomes and the final independent ten-run WASM gate remain required.
