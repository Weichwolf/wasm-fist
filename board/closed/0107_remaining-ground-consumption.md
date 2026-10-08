Type: Work item
Title: Consume complete ground station selection and retreat support in shared C
Depends: 0104, 0106

## Contract

Implement complete ae5c/b0be children for all four ground classes in shared owned C11,
using prepared canonical worlds and proved source data. Preserve used admission, ordered
station choices, reload/component/HUD/voice writes, real support resources/queues/cooldowns,
conditional RNG, smoke allocation/constructor/height and retained physical lifetimes.
Do not install an incomplete ab03 parent bank, replace the weapon-control owner, duplicate
artillery resources/ammunition or turn queued support into a fabricated completed strike.

Use existing weapon_control for mechanical selection/reload/cues, existing shared voice
history/admission, target reference lifetimes, world random, pool allocation, height and
preparation.artillery[side] order/count. Original +1f is type27.animation_counter in the
existing C payload, not debris_parameter (+1d); rename/document that one member for its
proved ammunition role and preserve preparation/damage/reset consumers. No second counter.
Canonical resource entries also capture pool allocation lifetimes;
never recapture a lifetime from a possibly reused slot at consumption. Skip stale resources
without binding their successors, and exercise actual same-type reuse in shared tests.

Establish typed owned support configuration and state from recovered 0106 inputs: air
stock, per-side delays, per-side clocks and sixteen-entry air/artillery queues. Require an
explicit configuration/reset boundary; manual-FSG stock three and catalog stock/delay
producer have different sources. Do not invent campaign parsing or silently configure
support in a saved-only initializer. Canonical world storage owns all retained state;
platform code handles presentation/devices/storage only. Queue entries own lifetime-safe
requester references and proved coordinate/tick/phase fields; no saved near pointers or
borrowed records survive. Copy target coordinates before a slot can acquire a successor.

Complete smoke creation publishes a typed type-20 payload with actual original pose and
sampled height, using normal-priority pool semantics. M1/M3/T80 spend existing class stock
before allocation failure; BMP has no stock spend. Keep created payload projection/type
ownership explicit; full type-20 subsequent methods are separate recovered work, never a
silent success/no-op in the world dispatcher.

Explicit repairs, supported by paired original evidence, must be tested and documented:
- Reject used target variants outside 0..3 atomically rather than reading adjacent tables.
- Clear stale target references and avoid matching their reused physical successor.
- Write the typed side clock for every selected gun, repairing the proved index2/3
  unaligned original write; preserve original spend-before-busy/full ordering.
- Remove audio/bank-address/device/source identity from the gameplay support decision.
  Preserve the proved source-unmatched smoke branch as the deliberate stable gameplay
  rule: after a smoke attempt both success (original return39) and stock/capacity failure
  (0x3031) request artillery. This is an intentional repair, not a claim about original
  intent. Implement typed outcomes, rather than expose guest register returns in simulation.
  Emit logical sound separately and preserve original RNG selection when no smoke is called.
  Test this repair against enabled/disabled/absent/source-unmatched contexts and all bank
  placements. Requests/playback/device failures must never choose a different support kind.

Do not add mode/deleted-flag restrictions to the ordered gun scan without a separately
justified intentional repair: original b0be only tests the registered gun's ammo word.
Actual prepared variant1 is registered and initialized to five too. Existing destruction
sets +1f to zero; preparation sets five. The required ownership proof describes
this producer chain but does not establish a new intended dead-gun policy.

## Evidence

Accepted on 2026-10-08 against the exact candidate inputs retained in
`/tmp/wasm-fist-0107-review/acceptance-final-receipt.json`. Shared C owns complete
station selection, explicit support configuration, transactional support requests and
type-20 construction. Canonical preparation captures resource lifetimes, type27.rounds
is the single ammunition owner, and support queues retain physical requester identities.
See docs/ground-support-consumption.md for boundaries and deliberate repairs.

Closed0104 proves complete original children and genuine parent entries nine/twelve;
closed0105/0106 prove audio coupling, queue/reset/catalog inputs, four-gun ordering and
ammunition ownership. Their current source graphs/results and immutable references were
checked in original-evidence-current-audit.json. The older0105 cooldown model is explicitly
superseded by accepted0106 evidence; no stale source pin is treated as current proof.
Closed0108/0109 provide independently verified prepared-world inputs and predictions,
including 42240 complete actual DOS returns and matched-source audio contexts.

The required LLVM19.1.7 style/tidy gate passes all94 translation units. The complete
corrected sequential production `bash tools/build.sh all` finishes successfully: both
builds, all46 native CTests, all44 WASM Python suites and both Node renderer probes pass.
Default optional original skips remain explicitly scoped; the required original and
canonical gates below run separately without skips. Two earlier probe diagnostic failures
for inconsistent type metadata were corrected without changing expectations; that terminated
run is excluded. Candidate inputs remained unchanged throughout the corrected full run.

Both production targets pass twelve station/support groups and 247147 complete transitions,
with zero skips and identical output SHA256
`d1ead219bf39febfce22281102ca10c63c2c3a6247d5f4403fc9039cd8f75c04`.
Coverage includes authored station choices/ammunition, selected/loaded bytes, complete RNG
words/cursors, variant rejection, wrapped admission, stock/capacity/audio independence,
ordered guns/typed cooldowns, all queue exits, actual release/reuse/orphans, retained requests,
used/unused invalid-input ordering and complete atomic failure/world preservation.

The actual prepared-world replay passes both targets for all47 missions, eight real height
maps and four details: 188 worlds, 3840 actors, 26880 ordinary children and 944 prepared
resources per target. It also covers 476 each resource release/reuse/orphan cases, 1480
admitted requester lifetime cases across all four ground classes, and five malformed-input
rejections. Twelve groups complete without skips; native/WASM complete output SHA256 is
`d878e5a82b52257c1fe29fab23a8d8c1858ca55de0a4868366a162021d3d9010`.
The probe destroys source storage and preserves every unrelated world byte.

Production-fast-math ASan/UBSan passes the complete domain and canonical gates with the same
digests, plus actual TRAIN1 SDL input/pause/focus/shutdown and complete-frame checks.
Current production TRAIN1 native/browser scenes and inputs pass; before/after frames were
reviewed and retained under `/tmp/wasm-fist-0107-scene/`. The browser checks complete C/canvas
agreement, startup failure and clean shutdown. Separate existing preparation/driving gates
pass on both targets with `--originals --oracle`, seven groups each and no skips: preparation
covers 847 fixtures, 11124 saved records, 1168 passes/twelve rejections per target and 656
original DGROUP returns; driving covers 123 fixtures, 1368 boundaries, 4449 installed objects
and twenty-three explicit rejections per target.

The additional required original ownership gate passes two groups without skips: sixteen
complete preparations, twenty-four support returns, eight actual releases and four same-type
allocation/saved pairs. These prove the declared producer/consumer lifetime contract, not
natural battle reachability. All419 original files remain unchanged/read-only; frozen ghidra,
reference tag, engine/kernel images and pinned softgl are unchanged. Compact receipts retain
commands, complete result hashes, source/program pins and the requirement-to-evidence audit.
Completed owned raw logs and obsolete snapshots are removed after preserving their evidence;
current builds, compact results and reviewed frames remain under `/tmp`. The checkout is348MiB.

Reproduce with LLVM19.1.x, the original assets and provisioned pinned images. Use
`PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache` for Python commands:

- `python3 tools/check_style.py`
- `bash tools/build.sh all`
- `/tmp/wasm-fist-decoder-oracle/bin/python tests/test_remaining_ground_canonical.py --target all --result-json /tmp/wasm-fist-0107-review/canonical-current.json`
- `/tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_support_ownership.py --originals --review-dir /tmp/wasm-fist-0107-review`
- `/tmp/wasm-fist-decoder-oracle/bin/python tests/test_mission_ready.py --target all --originals --oracle`
- `/tmp/wasm-fist-decoder-oracle/bin/python tests/test_mission_driving.py --target all --originals --oracle`

Exact sanitizer build/behavior/scene and production scene commands remain in the final receipt
and linked memory/cleanup receipts. No full parent bank is installed by this milestone;
queued strike dispatch, selected diagnostics, subsequent type20 methods, living battle,
PCM/outcomes and complete-game acceptance remain open. Complete-game WASM streak is zero.

## Next

Continue active0081 with complete selected diagnostics and the remaining caller/global,
heading/RNG and canonical parent/class consumption contracts. Keep queued support dispatch,
type20 lifetime and PCM/device work explicit under the living-battle goal. This WI is closed;
its complete bounded station/support contract does not establish a playable full battle.

## Accept

Both-target production behavior tests cover all authored preferences/masks/ammunition,
selected/loaded station and used variant validation; complete reload/voice/notice ordering;
side/target/range/global/per-side age wrapping; mode/RNG/stock/allocation/height; ordered
0..4 guns and every queue/capacity/busy/notice exit. Include retained sequential requests,
actual release/reuse, overwritten registry orphans, all47 prepared worlds/eight heights at
four details, complete atomic failure and source-release/queue ownership. Test corrected
cooldown for every gun and support independence across all logical audio contexts. Preserve
unrelated complete world state, including both canonical resource/weapon owners and random.

Run python3 tools/check_style.py and bash tools/build.sh all sequentially with LLVM19.1.x
and production fast-math. Require meaningful production-native/WASM exact behavior probes,
existing canonical regressions, memory checks and actual native/browser visuals. Retain
compact pins/reproduction/evidence, clean obsolete completed owned artifacts, commit/push.
Full parent/selected diagnostics/living battle/dispatch/PCM/outcomes/fullgame and independent
ten-run WASM gate remain open unless separately completed and proved; streak is zero.
