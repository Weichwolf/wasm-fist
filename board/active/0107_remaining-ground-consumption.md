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

Implementation checkpoint (2026-10-08, not accepted): the local working tree contains
transactional station/support children, explicit configuration/queues, canonical artillery
ammunition/lifetime ownership and type-20 projection. The complete strict style gate passed
after direct-include, enum and probe readability fixes. A development native run passed ten
groups and 247100 transitions with no skips; subsequent guard/failure additions require the
fresh full production run. That development run is not final acceptance evidence.

`tests/remaining_ground_probe.c` and `tests/test_remaining_ground.py` compare complete actor,
RNG, queue, history and constructor observations against the independent original contracts,
including documented repairs. They cover all authored target-type preferences and ammunition
masks, complete selected/loaded bytes, voice-gate/RNG words, wrapped admission, queue exits,
actual release/reuse/orphans, unused-invalid-input ordering and complete transactional failure.
See docs/ground-support-consumption.md. Declared prepared fixtures do not prove the all47
canonical-world gate or the real preparation-to-consumption lifetime producer chain.

The full sequential production native/WASM build/test run is in progress; current command,
source pins and process handle are recorded under `/tmp/wasm-fist-0107-review/progress.json`.
The C implementation remains uncommitted until a verified bounded success is established.
Compact previous checkpoints remain there; completed obsolete raw logs are removed only after
recording their results/hashes. Prior accepted runtime evidence does not cover these changes.

Cleanup/push checkpoint (2026-10-08): the current production native build and all46 CTest
tests pass, including twelve station/support groups and 247147 transitions; the complete
output digest is `d1ead219bf39febfce22281102ca10c63c2c3a6247d5f4403fc9039cd8f75c04`.
The WASM build completed; its sequential behavior suites are still running. Neither these
partial production results nor the unregistered canonical-contract draft close this WI.
All pinned runtime/test/build inputs match the running gate. The checkout occupies 348MiB;
sixteen ignored Python bytecode files and the superseded probe-only compile log were removed
(207702 bytes), with hashes and compile output preserved in
`/tmp/wasm-fist-0107-review/cleanup-push-receipt.json`. Live logs, the current required style
result, compact reference evidence and unfinished implementation are retained. The build
script already directs Python caches and disposable build output to `/tmp`.

Closed0108 now supplies `tests/remaining_ground_corpus.py`: all47 real-height/four-detail
original preparations and current native producer observations pass for 188 worlds, 3840
ground actors and 944 ordered five-round artillery resources (both authored variants).
This proves the prepared producer observations; consuming children and captured-lifetime
acceptance are still pending. Compact pins/results are in prepared-corpus-receipt.json.

Closed0109 supplies independent canonical child inputs and typed predictions for all47
prepared missions/four details: 26880 stimuli, 42240 actual DOS returns, 23040 real
constructor-height returns and 19200 matched-source audio contexts pass. The reference
verifier has no dependency on the unfinished consuming test or shared-C program.
These proved inputs now support the pending C producer-to-consumer/lifetime gate.
The current production consuming fixture gate also passed on WASM: twelve groups,
247147 transitions and the same complete digest as native. The remaining default
WASM production suites are still running; neither result closes this WI.

Use closed0104 complete original source/required result pins, closed0105
consumed audio return and closed0106 full producer/list gates. The additional required original
producer/consumer proof tests/test_original_support_ownership.py has sixteen
complete d755/b0be pairs across four classes, two prepared variants and saved stock
0/65535, with independent whole-state reset/support models. Original instructions at
b445 initialize +1f to5, b3e9 clear it on destruction, 1aeda tests it and 1af03 spends it.
Closed0104 supplies the separate complete eight-group parent/domain/all47 acceptance.
See docs/remaining-ground-command.md and docs/support-state-boundary.md.

## Next

Finish the current complete production build/test run without restarting it on observation
timeouts. Then extend consuming probes to the real preparation-to-consumption resource chain
and all47 canonical prepared worlds/eight heights/four details. Complete the required original
coupling, current memory and actual native/browser scene gates; retain exact source/program
pins and compact results. Keep full parent/class integration and later dispatcher/PCM
requirements visible; no selected diagnostic, battle or complete-game claim from this WI.

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
