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
typed support configuration/queues, transactional station/support children, canonical
artillery ammunition/lifetime ownership and type-20 projection. A preliminary native
compile completed all 69 build steps. Strict style verification failed on missing direct
includes and inconsistent enum initialization; consuming behavior probes, full production
build/test gates, memory checks and scene verification have not run for this implementation.
The implementation remains uncommitted until a verified bounded success is established.
Compact source pins, failed diagnostics and reproduction commands are retained under
`/tmp/wasm-fist-0107-review/checkpoint.json`; completed raw logs were removed after recording
their hashes and results. Prior accepted runtime evidence does not cover these local changes.

Use closed0104 complete original source/required result pins, closed0105
consumed audio return and closed0106 full producer/list gates. The additional required original
producer/consumer proof tests/test_original_support_ownership.py has sixteen
complete d755/b0be pairs across four classes, two prepared variants and saved stock
0/65535, with independent whole-state reset/support models. Original instructions at
b445 initialize +1f to5, b3e9 clear it on destruction, 1aeda tests it and 1af03 spends it.
Closed0104 supplies the separate complete eight-group parent/domain/all47 acceptance.
See docs/remaining-ground-command.md and docs/support-state-boundary.md.

## Next

Fix the direct-include and enum diagnostics without weakening checks, finalize the typed
state/API and explicit repair documentation, then add required consuming
production-native/WASM probes for the complete transactional shared children. Use existing tests/scene harnesses and
/tmp evidence. Keep full parent/class integration and later dispatcher/PCM dependencies
visible in the board; no selected diagnostic, battle or complete-game claim from this WI.

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
