Type: Work item
Title: Restore and prepare every original saved mission before player control
Depends: 0075, 0088, 0089

## Contract

Extend the canonical saved-object owner to restore every record in all 47 original missions,
using the complete normal-side d84a/43c1 boundary. Own common saved-format fields for 11/13/16/25
and every modeled type-18 muzzle field. Reuse the existing arena/registry, pose and muzzle owners.
Add a separate shared mission preparation owner for the delivered d755 class bank, with actual
ground reset, tree RNG, center heights, target modes, bounded artillery lists and temporary release.
Consume preparation after terrain installation and before canonical player control.
Preserve input order, ground-only participant initialization, all physical orphans, roster,
RNG, source-release ownership and whole-world failure atomicity. Do not run d755 inside c296,
translate saved target pointers, or supply missing living methods as successful no-ops.

## Evidence

The current native AZER1 probe returns status 2 on the first unsupported type 16; the actual
original loader completes that record. The 47 pinned FSGs contain 4213 records: missing common
saved classes are 673 type-16, 22 type-25 and one each type-11/13; one saved type-18 is also
present. Every short record is a nonparticipant. Complete original d84a preserves all common
fields and consumes no RNG for these types. Existing physical constructors preserve their
new pool index while the DOS saved-byte transfer restores the remaining bytes. Type-18 frame
and animation word are actual +19/+1a fields, proved by the existing complete muzzle oracle.

These saved common fields are format data, not simulated class methods. The separate original
readiness bank counts 16, samples 25 and releases 11/13/18. CYPRUS4 and INDIA5 selected players
retain nonzero saved target words after loading. The old canonical test fixture cleared +97
itself; that cannot establish the reached original mission boundary. Remove that fixture write
and execute actual d755 before control. Ground reset also owns saved +36 and candidate +9d.
Types not restored here continue to fail installation explicitly. Later living
class/collision dispatch remains explicitly unsupported where undelivered.

## Next

Compare full native/WASM saved-world output against actual original constructors/loader returns
for all 47 missions and every new type's complete byte/pose/identity domains, including orphans.
Require all 4213 objects, unchanged originals, complete RNG/roster/order outputs and source release.
Regress canonical timed driving and complete command worlds across the expanded corpus. Run both
production builds, strict format/tidy, memory checks and actual SDL/browser previews. Keep compact
/tmp receipts, clear obsolete artifacts and commit/push the complete reached start boundary.

## Accept

All 47 original saved worlds and all 4213 objects install and prepare completely; no skipped original
records or silent type fallbacks. Both targets agree with complete original restoration for
all owned fields and state. All required build/style/original/memory/presentation regressions
pass. Compare complete actual original DGROUP returns outside only actual stack writes, complete
owned state, RNG, census/lists and registry mutations. Require invalid-input atomicity, source
release, retained orphans and repeated passes. Living battle, audio and complete game acceptance
remain open; final streak zero.

## Verified original and presentation checkpoint

The new shared preparation gate passes seven groups without skips on both production targets:
847 fixtures, 11124 saved-record observations, 1168 complete successful passes and 12 atomic
rejections per target. Its independent original observer executes 656 complete d755 returns,
checking all 65536 DGROUP bytes outside only actual stack writes 8ff0..9001. C observations
check complete owned metadata, live payloads, roster, orders, RNG, census and typed artillery
lists. The probe separately checks every byte of released payloads apart from their deletion
flag and every orphan payload, after releasing all saved source storage.

Coverage includes all 256 flag/link bytes, all four ground classes, all four tree variants,
all eight target modes, both delivered artillery lifecycle modes, four entries per side,
repeated passes, reversed registry order, deleted nonparticipants, duplicate-binding orphans,
release-word underflow and synthetic runtime explosion/shell/retiring releases. Used variants
outside delivered tables, five artillery entries, invalid heights and incomplete requests fail.
All 47 pinned missions/4213 records prepare completely and clear all 14 saved ground targets.

Required original saved-world restoration passes seven groups on both targets without skips:
1156 fixtures, 10057 installed object observations including reload, 269 explicit rejections
and 65569 tree requests per target. Complete original record comparisons include all new
common/muzzle fields and extreme coordinates/flags/projection/identity values. Required
canonical driving passes seven groups without skips, 123 fixtures, 1368 full timed boundaries,
4449 installed objects and 23 explicit rejections per target. It now executes complete actual
preparation before control/contact; it never clears fixture target words itself.

Required both-target original goal and route regressions each cover all 960 ground records
in all 47 complete worlds with zero unsupported corpus cases. Goal coverage is 184948 fixtures,
47268 rejections and 2128 complete world boundaries. Route coverage is 281524 fixtures,
75076 rejections, 656 full-capacity repairs, 205792 unchanged original returns and 2691 complete
world boundaries. Earlier primitive/loader boundaries remain separate from preparation.

Actual SDL and Chromium input/pixel gates pass against isolated original TRAIN1, AZER1 and
CYPRUS4 assets. Reviewed AZER1 browser and CYPRUS4 native paused captures show terrain,
the selected original model and the weapon/ammunition/pause HUD. Full changing/stable opaque
frames, actual movement/steering/turret, weapon/reload, pause/focus, shutdown and startup failure
checks pass. All 419 provisioned original files retain the preceding checkpoint's hashes.

Native production passes all 36 CTest gates in 456.26 seconds. Strict LLVM 19.1.x format/tidy
checks all 81 owned C units; the header filter now includes moved tests headers. The full
regular WASM gate passes all 34 Python suites/260 cases and both Node pixel probes. Its 42
optional original groups retain their declared skips; every required original gate above
passes without skips. No compiler/style check or existing behavior requirement is weakened.

The production-flags AddressSanitizer/UndefinedBehaviorSanitizer/LeakSanitizer build passes
all seven groups without skips for each required preparation, saved-world and canonical driving
gate, with the same complete corpus and counts. Preparation completes in 13.202 seconds,
saved-world in 29.236 seconds and canonical driving in 7.710 seconds. Configuration and the
three probe targets build sequentially after the terminal full production build.

TRAIN1 with seeds (1, 2, 32768, 65535)/cursor 0 retains 85 records and its 49 trees consume
98 additional RNG values: loader (23040, 46080, 16384, 52223)/cursor 2 becomes preparation
(5829, 11658, 45, 54666)/cursor 0. Full original canonical boundaries check that shared state.
Frozen ghidra/peeled reference tag remain 349ad31a9fd21b350d435651bb2e90afda40cf60 and softgl
remains 7963be1d5b5e1bebbe97ece2c655228c8bc0a838. Compact hashes, commands and latest reviewed
native/browser frames remain under /tmp/wasm-fist-0090-review; obsolete logs, duplicated input
copies and the memory build are removed after recording their evidence.

## Reproduction and remaining scope

```sh
bash tools/build.sh all
python3 tools/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_mission_ready.py --target all --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_mission_world.py --target all --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_mission_driving.py --target all --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_ground_goal.py --target all --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_ground_route.py --target all --originals --oracle
```

The receipt records exact sanitizer configuration, commands, environment and complete results.
Only the declared preparation bank/domain is accepted. Full e006/device setup, runtime target
discovery/reference lifetimes, command/class dispatch, living battle, PCM, outcomes and the
remaining game surfaces stay open. No game-completion or independent final WASM run is claimed;
the final complete-game streak remains zero. Continue unchanged parent 0081 with the prepared
canonical world rather than translating saved near references or wiring a partial callback bank.
