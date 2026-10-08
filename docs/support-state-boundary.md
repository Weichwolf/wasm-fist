# Original support state inputs

Closed WI 0106 proves original support initialization and ordered artillery consumption.
This reference-only prerequisite leaves full WI 0104 parent/domain/corpus acceptance open.
Shared support C, campaign parsing, later dispatch/flight and PCM remain unimplemented.

## Reset and catalog producer

Unchanged `9d3c` sets air clocks `9460/9462` to world clock `6cde - 480`, wrapping
as a word. It sets air types `944c/944e` to 5/6, swapping for nonzero side word
`6db4`, and clears only the first word of each of 16 twelve-byte entries at
`9464/9524`. Queue tails are retained, rather than silently zeroed.

Unchanged `b2a2` clears resource counts `9ccb/9ccd` and `9c89`, sets artillery
clocks `9f17/9f19` to world clock minus 480, and clears only the first word of
each of 16 fourteen-byte entries at `9ce7/9dc7`.

Far producer `1bf45` selects a 253-byte record at `6dda + 253 * WORD[6db6]`.
The test declares an already-decoded record at index zero. It establishes the
producer's complete behavior for that input, not record discovery, campaign
file decoding or complete mission startup.

| Record input | Original destination/operation |
| --- | --- |
| `4c/3c/5c/6c`, 16 bytes each | Mailbox `7a/8a/9a/aa` respectively |
| `21`, 16 bytes | Filename at `79a9` |
| `fc`, byte | Link setting `6dae` |
| `31`, word | `6dbc` |
| `33/34`, bytes | Words `e3ae/e3b0`, each input plus one |
| `35/36`, bytes | Artillery delays `9ce3/9ce5`, multiplied by word `fa8a` |
| `37/38`, bytes | Air delays `9454/9456`, multiplied by word `fa8c` |
| `39/3a`, bytes | Air stocks `9450/9452`, widened to words |

Delay multiplication wraps to a word. Frozen factors `fa8a/fa8c` are both 480;
tests also cover every possible factor word. The producer executes its original
CLD and complete far return, with seeded upper halves of general registers.

## Valid four-gun cooldown defect

The side-eight artillery service scans the ordered list at `9cd7`, bounded by
count `9ccd`. It selects the first gun with ammunition at object offset `1f`.
It spends ammunition and writes the clock before testing busy/full queue exits.
The list test uses four real original type-27 allocations and a real target.

Original instructions shift BX left twice before scanning, increment it by two
per skipped gun, then shift it right twice before writing `WORD[9f17 + BX]`.
With zero-based resource indices 0/1, that destination is `9f19`. Indices 2/3
instead write an unaligned word at `9f1a`, changing the high cooldown byte and
the adjacent byte `9f1b`. For previous cooldown 1320 and clock 1800, the observed
cooldown becomes 2088 and the adjacent byte becomes 7. This occurs with valid
allocated resources; it is not a malformed-file hypothesis.

The common reference model preserves the original write at
`9f19 + resource_index // 2`. A future shared consumer must explicitly repair
this through typed per-side clocks. No unaligned DOS-memory behavior is added
to the C runtime by this checkpoint.

Canonical C already owns ordered artillery resources in
`world.preparation.artillery[side]`. Existing per-class weapon fields own smoke
stock; BMP's real smoke service has no stock gate/spend. Future consumers must
use these owners and establish explicit support configuration/queue/clock state.

## Existing ammunition and retained lifetimes

Original preparation `b445` sets object word `1f` to five; artillery `1aeda/1af03`
tests and spends it. Destruction `b3e9` clears that same word. Existing shared C
already stores it as `type27.animation_counter`, including preparation and damage
writes. A later consumer must rename/document and use that member; the separate
`debris_parameter` is word `1d`. Creating another ammunition counter would break
the established producer/destruction contract.

Sixteen complete prepared/support pairs cover four classes, both authored gun
variants and saved ammunition 0/65535. Each original preparation produces five;
each admitted support request produces four. Prepared variant one is registered
and consumable too. This does not justify silently adding a mode/deleted-flag gate.
Whole DGROUP, independent height predictions and unchanged code/text are checked.
The real height wrapper's complete EBX mailbox packet is predicted before execution,
including the seeded upper register half; other mailbox bytes remain unchanged.

Eight additional complete support returns explicitly declare an invalid retained
resource: a real release leaves its list word unchanged, and a real same-type
allocation/saved continuation can reuse the same address and registry/value tuple.
The original then spends the released storage or successor's ammunition. These
cases prove the failure of tuple-only identity checks; they do not establish
ordinary battle reachability. Canonical resource storage must retain allocation
lifetimes through the existing pool reference owner and prevent successor binding.
The new two-group gate's case digest is
`00f44bd8b68e4b0594ab3c3abd21d8b5868ee17d6aa70c8e3af46b9213248777`.

## Required evidence and reproduction

The initialization gate completes two groups and 459008 returns: 196608 air
resets, 131072 artillery resets and 131328 catalog producers. It covers all clock
words, all air side words, every byte value throughout the declared record and
both complete delay-factor word domains. Whole DGROUP has no scratch exclusion;
code/text/mailbox, seven general registers, segments and stack returns are checked.
Output digest: `d896ad5b6d72bf8097b6ade0a98846fd54f19e7fed03a92e7ed5f68fe3293993`.

The ordered-list gate completes one group and 130560 b0be returns across four
classes, counts 0..4, all 16 availability masks, ammunition 1/65535, busy 0/1,
occupied entries 0..16, selected/unselected and notice bytes 0/2/255. Whole state
outside bounded stack scratch is checked by the existing support/audio observer.
All four first-available indices are reached. Branch totals are: not in place
26112, empty 24480, busy 39984, confirmed 37632 and queue full 2352.
Output digest: `9fec9f211074069e9cb571b2305697d28523f22976afb127310729d4e93192b7`.
The same fixtures also produced this digest with the separate remaining-parent
observer; that comparison does not accept its unfinished full WI 0104 suite.

All eight original audio regression groups were rerun after the model correction.
Their output digests, counts and coverage match closed WI 0105 exactly.

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_support_initialization.py \
  --originals --review-dir /tmp/wasm-fist-0106-review
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_support_list.py \
  --originals --review-dir /tmp/wasm-fist-0106-review
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_support_ownership.py \
  --originals --review-dir /tmp/wasm-fist-0106-review
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_audio_request.py \
  --originals --review-dir /tmp/wasm-fist-0106-review
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_audio_tail.py \
  --originals --review-dir /tmp/wasm-fist-0106-review
```

All five commands are required, with zero skips and complete group counts 2/1/2/6/2.
Initialization/list/audio results were copied from `/tmp/wasm-fist-0104-review`;
the ownership gate ran directly under `/tmp/wasm-fist-0106-review`. Its compact
receipt retains exact source/reference/original pins and cleanup inventory.
The live full 0104 run retains its own inputs and evidence.
No production C, configuration, softgl or presentation changes occurred: accepted
426ab57 evidence is carried forward after verifying 261 sources and three programs.
The complete-game WASM streak remains zero.
