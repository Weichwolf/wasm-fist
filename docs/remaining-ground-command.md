# Remaining ground command boundaries

Closed WI 0104 proves complete unchanged `ae5c` station selection, `b0be` retreat/support
and genuine automatic parent entries nine/twelve. Controlled entries execute the
original `b111` RET. The `ab03` prefix always stores the actor and consumes RNG,
even when global admission rejects. Admitted calls increment the retained byte
counter, resolve both platoon tables and dispatch according to control bit zero.
The separate selected `b152` diagnostic remains under WI 0081; this boundary
uses diagnostic-unselected caller state while testing selected UI/audio separately.

Station selection reads the target's type, or the type-zero preference when no
target exists. Type 26 uses an unmasked variant byte. Each class tries three
literal preference bytes in order and tests its station's ammunition word. If
all three are empty, it still selects the third byte. Repeated preferences are
intentional original data, so three tries do not imply three distinct stations.

A changed station invokes the real class setter: selected/loaded bytes, reload
countdown, empty-ammunition/reserve notice, component invalidation and all four
global HUD markers follow their existing weapon-control owner. Already-selected
returns do not dirty the HUD. Voice admission executes real `bf3c`; admitted
requests execute the complete kernel with original read-only WV/EV voice banks.
Full general-register and kernel-byte predictions prove the returned sample
address. The actual DOS actor/CS/DS/SS/SP return, whole DGROUP, code, text and
mailbox are checked. Scratch general registers outside the declared consuming
ABI are not new simulation fields.

Support admits the side-eight actor only with a nonnull target, unsigned range
at least 20 and wrapped global age at least 1800. It then consumes one RNG draw.
Mode four conditionally invokes the actual class smoke service, pool allocator,
type-20 constructor and installed-map height service. Stock is spent before an
allocation failure; BMP has no stock gate. The call's actual AL is consumed at
its original comparison, including the real op-64 EAX response when source audio
is admitted. This preserves the bank-placement defect proved in WI 0105 for
observation; it does not establish an intended gameplay repair.

Air support preserves the actual cooldown, first-free queue order, capacity
failure, notices and selected display. Artillery spends ammunition and records
cooldown before later busy/capacity rejection, copies the currently addressed
physical target position, and preserves its text/notice/queue writes. Queue,
stock, request and constructor contracts reuse WI 0105's already accepted owner;
they are not replaced by fake service returns or private runtime queues.

The following defect proofs matter for shared consumption. Variants 4..255 read
adjacent preference pointers and can select odd/out-of-range station bytes. The
complete callback is observed with explicitly already-loaded caller state so
this proof does not pretend additional reload/voice table entries are valid.
A valid third/fourth available artillery gun exposes another original defect:
the SHL/scan/SHR sequence writes its cooldown one byte late, corrupting the high
byte of the side cooldown and the adjacent dispatch byte. Actual complete four-gun
resource/queue/selected-notice tests prove every choice and exit; the reference
model preserves this behavior, while shared C must repair it explicitly.
Actual release leaves the old target reference nonzero; a later smoke allocation
can reuse that same physical slot. Subsequent artillery then copies the smoke
position. Live, released and reused station calls also read the currently present
type. Existing canonical target-lifetime ownership must prevent silently binding
a released target's successor. Out-of-domain platoon-byte table reads at these
parents do not prove valid platoon descriptors or a safe later dereference.

The required suite covers literal preferences for all 28 target types and all
four authored variants on every class, every station-availability mask, full
selected/loaded byte pairs and ammunition words per class. It checks reserve/
notice byte domains in actually reached M3/BMP branches, all voice-gate/age/
selected-pointer words, support RNG/range/age words, full flag/mode byte pairs,
and real parent control words, counter high nibbles, RNG cursors, platoon bytes
and global admission. Shared-predicate word domains distribute classes; the
class-dependent station domains and parent counter/cursor/platoon cross products
exercise every class. Complete inherited WI 0105 stock/queue/audio-domain evidence
remains pinned unchanged rather than being relabeled as a new PCM test.

The corpus provisions all 47 pinned scenarios and eight real height files,
executes original decoding/scaling, saved allocation, PATH/PINF loading and
mission preparation at 512/1024/2048/4096. Across 188 worlds and 3840 ground actors
it checks 57600 complete returns: unchanged child inputs, both genuine parent
banks, and explicitly constructed reaching target/retreat inputs through direct
support and real automatic entries nine/twelve. Reaching cases retain the real
actor's position, heading and velocity, original allocator state, support
resources and installed height field; effects-bank offsets 0/32/128 expose the
actual heap-dependent outcomes. These are declared consuming-boundary fixtures,
not an untouched whole-battle acceptance claim.

Reproduce with:

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_remaining_ground.py \
  --originals --review-dir /tmp/wasm-fist-0104-review
```

The suite requires all eight groups, zero skips and a terminal successful run
before writing `remaining-ground.json`. Original image pins come from
`tests/reference_images.py`; original assets remain ignored/read-only.

The exact required eight-group run passes with zero skips in 2759.320 seconds:
2238296 complete DOS callbacks, 874888 actual kernel audio returns, 108828 real
constructor/height returns and 7510 actual typed target release/allocation pairs.
The genuine parent boundary supplies 370176 complete ab03 returns, including 4080
global-admission rejects. Automatic entry nine/twelve each accounts for 97284;
controlled entry nine/twelve each accounts for 85764. Main output digest:
`197b7ef0517c25be83537c5d55cc9cb92b8d213946a7aa8df13506f1ca4de9f5`.
The corpus includes exactly 23040 constructed smoke/height returns. All 47 mission
and per-detail actor/child/parent/reaching totals are checked before acceptance.

Closed WI 0106 supplies the complete original reset/catalog/ordered-resource and
ammunition/lifetime prerequisites. Its five support groups and eight audio
regression groups pass; the corrected cooldown model preserves the previously
accepted audio hashes/counts/coverage. See [support state inputs](support-state-boundary.md)
and [consumed audio returns](audio-request-boundary.md). This evidence includes
invalid retained resources after actual release and same-type binding reuse;
canonical resources must retain allocation lifetimes, without a private list.

The accepted WI 0103 production hashes are reaudited: all 261 tested sources and
three programs are unchanged, as are all 419 original files/read-only permissions,
frozen ghidra/reference tag and softgl pin. Build/style/memory/presentation evidence
is carried forward from accepted 426ab57; no fresh C build or visual check is claimed.
The exact required result, source/original/reference pins, commands and cleanup
receipt remain under `/tmp/wasm-fist-0104-review`. Obsolete owned logs/drafts and
superseded pilots are removed after recording compact evidence. No C/configuration/
dependency/presentation change is accepted by this reference-only checkpoint.

The subsequent shared C consumer must cover both complete children using one
canonical actor/registry/RNG/height/notification owner, preserve the existing
weapon-control state owner, prove used-domain validation and explicitly justify
stable audio-independent support semantics from the recovered evidence. Full
parent/diagnostics/class dispatch, living battle, PCM, outcomes, menus/editor and
the independent ten complete WASM runs remain open; the final streak is zero.
