# Original audio request and consumed return

Closed WI 0105 proves complete unchanged protected-mode operation `0x64` and its
consuming retreat/smoke/support return. This is original reference evidence. PCM
decoding, mixing, timing, full device/DPMI initialization and shared C consumption
remain open. No host sample address or inferred support repair enters the rewrite.

## Bank and queue contract

The pinned DSOUNDS bank contains 15 records, followed by a two-byte zero paragraph
sentinel. WVSOUNDS and EVSOUNDS each contain 43 records with no trailing sentinel.
Each record starts with a paragraph-count word; the next record is at
`offset + 2 + paragraphs * 16`. Its body starts two bytes after the record offset
and contains frame-count and default-step words before the PCM bytes. Several
original frame counts exceed their paragraph payload by one to three bytes.
Request/header tests do not establish PCM boundary behavior; a later decoder must
recover it before accepting playback.

Original handler `786a` leaves a disabled request unchanged. Otherwise AL selects
an effects record, or a voice record after clearing bit 7. AH selects direct
channels 0/1/2 or queued channels 128/129/130. Absent banks return without queuing;
AL=255 selects the original silence header. All admitted paths return the selected
sample address in EAX. Direct `22ab` replaces the active pointer/count/step and
mixer immediates. Queued `2377` stores a pending pointer/step/attenuation/volume;
it also starts immediately when the active pointer is silence. Both paths
zero-extend DL into EDX, including a queued request on a busy channel.

The real effects loader at `781a..7832` requests alignment four from allocator
`36bf`. Complete original allocations prove all 64 possible aligned low bytes.
Tests count the additional 192 unaligned handler inputs separately, rather than
claiming those are loader-produced bank placements.

## Reached support dependency

Original `b0be` calls the class smoke service through `a28c` in retreat mode and
then compares the returned AL. The actual type-20 constructor allocates a normal
short object, rotates the hull offset, adds signed actor velocity and queries the
real op-54 height handler. Source-matched `c047` then issues sample 11 through
op-64. Stock/capacity failure returns AX=0x3031; source-unmatched success returns
AX=39. The actual audio response is consumed by the subsequent support decision.

The following paired observations use identical complete DOS world inputs and
change only the effects-bank placement or sound admission:

| Audio context | Bank offset | Consumed AL | Complete support result |
| --- | --- | --- | --- |
| Enabled | 0 | 8 | Air confirmed |
| Enabled | 32 | 40 | Artillery confirmed |
| Enabled | 128 | 136 | No support call |
| Disabled or absent bank | 0/32/128 | 11 | Air confirmed |

All three enabled placements satisfy the actual allocator's alignment. The proof
covers all four classes, selected/source contexts, complete stock domains,
actual pool exhaustion, support admission/cooldowns and every queue index/full
exit. Constructors use declared, actually allocated worlds and installed flat
height fields. The full all-47 mission and genuine parent acceptance remains in
WI 0104. The later shared consumer needs an explicit evidenced rule to remove
this heap/device dependency; no gameplay repair is inferred here.

## Authored sentinel overread

Original c047 selector 51 contains packet `0f 01 00`. The real producer admits
AX=0x010f, although effects sample 15 is beyond the 15 valid records. Handler
786a walks through the zero paragraph sentinel and treats the next allocation
as a sample header. Two complete original allocator calls place an actual
WV/EV bank there. Its paragraph and frame words become the effect's apparent
frame count and step, respectively: WV yields 218/3487, EV yields 186/2975.

Complete c047 and op-64 returns prove the one-past-bank sample pointer, with
read-only original bytes and whole kernel/DGROUP/code/text/mailbox checks. The
test's explicitly marked invalid-sentinel model entry describes this overread;
it is not a valid sixteenth PCM record or permission to read past an owned bank.
PCM handling and the intended missing/invalid cue repair remain unproved.

## Reproduction and evidence

Use the pinned optional Unicorn environment under `/tmp`:

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_audio_request.py \
  --originals --review-dir /tmp/wasm-fist-0105-review
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_audio_tail.py \
  --originals --review-dir /tmp/wasm-fist-0105-review
```

Both commands are required: six primary groups and two sentinel groups, with no
skips. Primary coverage is 349012 complete kernel requests, 123260 complete DOS
returns, 83291 coupled kernel returns, 89453 real constructors/height returns and
256 complete allocator returns. The sentinel gate adds 216 complete kernel
requests, four complete c047/kernel pairs and four complete allocator returns.
Every pending/active channel and unrelated mapped buffer is guarded. Actual AL
is read at its original consumer before the unchanged comparison and final return.

Independent fixtures recycle emulator instances after 256 cases to bound
translator storage. Sequential requests preserve their actual retained state.
The strengthened run has the same normalized output hash as the preliminary
full run: `7f04b24f3da9596c80c0cf838028e7c71129384b212f28672366f587c68f3be0`.
The sentinel result hash is
`4b6ca963878f50afd891127ae42fe065ae2bfb7a193b14e992fd344442ee2666`.

Compact result, bank metadata and pin/reproduction receipts remain under
`/tmp/wasm-fist-0105-review`. Runtime, existing tests, production configuration,
softgl and presentation are unchanged from accepted 426ab57. Its native/WASM,
strict tooling, memory and scene evidence is carried forward after source and
program hash verification. Full battle/PCM/outcomes and the independent complete
game WASM gate remain open; the ten-run streak is zero.
