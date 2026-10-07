# Ground command phase: recovered input and pending consumption

WI 0081 remains open. The native/WASM controller currently consumes motion, reload, position
history and maintenance, but not the complete heading/command callback. This document records
the verified original input and dispatch contracts needed for that implementation. Closed
WI 0082 supplies owned orders; WI 0083 implements the complete nested mode selector and WI 0084
the complete route/formation goal return. These milestones do not replace full 0081 acceptance.

Closed WI 0088 proves the later [original mission-ready reset](mission-ready-boundary.md):
saved target clearing, class reset, temporary registry release and tree RNG. TRAIN1 adds 98
RNG calls. WI 0090 supplies the separate shared C preparation and canonical player-start
consumer. The standalone saved-loader/prefix remains an earlier declared boundary; target
discovery and the complete command/class caller still require actual runtime identities.

## Original phase and callback banks

Frozen DOS image SHA256 `d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
All four actual class tables select ab03 at byte indices 14 and 30 of phase & 1eh.
The shared original method calls RNG 0291 before testing the global admission byte +978a,
increments saved byte +42 and, on its low-four-bit boundary, shifts signed heading-history
words +28/+2a/+2c. It adds those three signed words and current hull word +26 with 32-bit
MOVSX arithmetic, logically shifts right two and stores the resulting word at +2e.

It resolves the actor's platoon descriptor through DS:85a0 and route through DS:7d2a.
The incremented low-four-bit counter selects one of sixteen actual near callbacks. Control
flag 1 selects the automatic bank at 98dc; a cleared flag selects the controlled bank at 98fc.
The possible b152 tail is a separate diagnostic panel producer for global selected pointer
7ae0; it also needs a declared/configured presentation boundary before full acceptance.

| Counter low bits | Automatic entry | Controlled entry |
| --- | --- | --- |
| 0 | ab82 | ab82 |
| 1 | ac75 | ac75 |
| 2 | ab88 | ab88 |
| 3 | ad2f | ad3b |
| 4 | ad08 | ad08 |
| 5 | af97 | b111 |
| 6 | b011 | b011 |
| 7 | ae66 | b111 |
| 8 | ae32 | b111 |
| 9 | ae5c | b111 |
| 10 | afa2 | b111 |
| 11 | b017 | b111 |
| 12 | b0be | b111 |
| 13 | b053 | b053 |
| 14 | af97 | b111 |
| 15 | ae66 | b111 |

Entry b111 is an actual single RET. The other callbacks are not replaced with returns.
ab82 calls f69:b4ef, raw 1ab7f, and chooses the +43 command state from control/target/platoon
rules; a controlled leader selects zero and a controlled follower selects two. ac75, ab88
and ad08 dispatch through +43 into additional goal, bearing/range and waypoint handlers.
b011 calls f69:b378, raw 1aa08, and scans the actual ordered registry for candidates using
opposing-side flags, visibility, class preference and distance. This work is required even
with control flag 1 cleared; treating it as an empty manual callback would omit gameplay.
b053 calls f69:b329, raw 1a9b9, and can reorder physical roster members and mutate another
member's index. A complete implementation therefore needs actual canonical world state,
not only an isolated actor and a private random stream.

## Mission orders missing from the old isolated prefix setup

The FSG tag table at DS:e9e6 maps PATH to d87f and PINF to d8a9. With caller read mode
DS:e975 ==1, these complete original handlers issue DOS 21h/AH=3fh reads to 7d40 and 85b6.
Other mode bytes seek forward by the chunk's declared length and leave those blocks unchanged.
Their exact destinations are the records addressed by ab03's platoon tables:

- PATH: eight records of 268 bytes, totaling 2144. Record byte zero is the route count,
  the first goal XY starts at +12, and the allocation fits 32 eight-byte coordinate slots.
  Each slot has two signed 32-bit coordinates. All eleven remaining header bytes and all
  unused coordinate slots must survive ownership and later save/load; their unused semantics
  are not invented. Original corpus route counts range 0..21. Recover original append bounds
  before accepting malformed count domains in a consuming decoder.
- PINF: eight descriptors of 22 bytes, totaling 176. All eleven words must be retained.
  Current original consumers read behavior word +0, waypoint mode +2, formation index +4
  and throttle-command selector +6. Mode-specific validation and remaining word semantics
  belong to recovery, not guessed clamps. The corpus uses values 0..3 for words 0/1/3,
  0..5 for word 2 and zero for words 4..10.

The existing saved-object oracle installs DCBS, actual constructors and class initialization;
its default PATH/PINF storage remains the zero-filled original image. That boundary suffices
for the six accepted early TRAIN1 prefix updates, before those order consumers are reached.
It cannot establish later command/goal correctness. The real TRAIN1 saved actor already has
its first goal and goal-valid control bit, so merely comparing that unchanged goal would
also miss whether the mission route was loaded.

`original_mission_orders_oracle.py` supplies the real missing DOS I/O boundary and executes
both complete original chunk handlers with their original near returns. It validates the
actual tag dispatch and pointer strides and compares all 65536 DGROUP bytes after each return;
only the declared destination may change. Loader globals and stack inputs are set before that
comparison. Full byte counts, actual read/seek requests and RNG preservation are required.
No guest instruction, command callback or caller result is substituted.

The reaching test uses the real 85-object TRAIN1 installation and physical player 151, with
an explicitly constructed stale goal (zero XY and cleared goal-valid control flags). Both
original machines receive that same actor input; one loads real PATH/PINF, the other retains
the declared empty orders. All original M1 prefix instructions execute through 7c7e, stopping
before the engine PCM device service. At update 15, loaded orders restore the real first
route goal and set the goal-valid bit through the complete ac75/ac7e callback. At update 23,
the complete ab88/ab91 direction callback consumes that goal and computes a different range.
All other 84 payloads remain unchanged; three actual ab03 calls advance RNG cursor two to one.
This is a constructed reaching consumer proof, not a claim about an untouched saved actor.
Reserved address-zero/one driver-return scaffolding still fails the prefix observation.

## Reproduction and scope

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_command_boundary.py \
  --originals --review-dir /tmp/wasm-fist-command-review
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_vehicle_history.py \
  --target all --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_mission_driving.py \
  --target all --originals --oracle
```

The new original-only gate requires all 47 pinned files, both complete order blocks for every
mission and read mode 1 plus seek modes 0/2/255: 376 original chunk returns, 109040 bytes of
loaded order input, no skipped references. It also verifies both full callback banks and
actual all-class ab03 admission before the reaching consumer trace. It is outside production
native/WASM gates because it verifies original-input evidence, not implemented C commands.
Exact verified results and the next owned-input requirements are recorded in active 0081.
No C runtime, production build, dependency or original file changes in this recovery step.
Complete phase/world dispatch, PCM, battle, outcomes and final independent WASM acceptance
remain required; the complete ten-run streak stays zero.

The shared owned decoder and canonical mission installation are documented in
[mission orders](mission-orders.md) (WI 0082). Saved-object-only worlds remain explicitly unloaded;
complete command consumption must use the canonical order owner rather than private empty input.

The complete ab82 mode selector and its conditional additional RNG call are documented in
[ground command selection](ground-command-selection.md) (WI 0083). Saved mode/maneuver/target
presence now survive owned restoration and initialization. The existing +42 counter remains
the sole owner; no parent-phase heading update or partial callback bank is installed. Remaining
bearing/range, navigation, target-discovery, roster and presentation calls still require
complete consumption before accepting 0081.

The complete ac75 route/formation goal helper is documented in
[ground command goals](ground-command-goals.md) (WI 0084). It shares canonical orders and the
physical roster, retains saved heading history/average and signed goals, and preserves RNG and
all unrelated world state. Parent heading sampling and callback scheduling remain pending;
no partial bank or new living-prefix acceptance follows from the isolated complete helper.

The complete semantic 0541 planar bearing/distance prerequisite is documented in
[planar geometry](planar-geometry.md) (WI 0086). It owns original numeric rules and local scratch;
it does not implement ab88 target resolution or its full dispatch. Use that shared owner when
recovering remaining command handlers. Parent/callback/battle/PCM acceptance remains open.

The complete ad08 route progress and proved count-32 storage repair are documented in
[ground route progress](ground-route-progress.md). This callback shares canonical route and
retained unsigned range ownership. It does not consume target references, produce bearing/range
or supply a partial parent callback bank. Full 0081 acceptance remains unchanged.
