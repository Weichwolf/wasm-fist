Type: Work item
Title: Deliver complete shared decoded ground-axis consumption
Depends: 0121, 0122

## Contract

Own complete f69:aae4 as one readable shared driver operation. Restore and retain
the two signed saved axis bytes in canonical vehicle state. Reuse the existing
drive-profile/component/display event owner; preserve heading dead zone, signed
throttle dead zone/asymmetric -24 boundary, wrapping heading and profile ordering.
Return an explicit no-refresh event when the original skips the profile setter.
Reverse speed always refreshes, including when mode3 is already set. Validate the
complete used ground actor and output before mutation; late failure preserves
actor/output. Do not consume device records or install a partial class/manual bank.

## Evidence

Published ecea3cc provides exact original model/fixtures and whole-memory/ABI
evidence:528,384 independent full original axis/profile returns;384 retained
four-actor calls through all three pure analog manual callbacks;4,096 complete
class-start/readiness axis-retention and4,096 complete decoded-record copies.
The consumer does not read clock, target, pool/RNG or devices. Unknown channel
semantics and full a57a banks remain under0121, without guessed behavior.

## Next

After0122 complete publication, start an isolated exact candidate from current
master. Add the two canonical signed saved bytes with initialization/readiness
retention tests. Implement full consumer in driver.c using
fist_vehicle_set_drive_profile. Add a complete probe with original domain,
all47 snapshot, retained-event/heading and whole typed write-footprint tests;
invalid pointer/type/component/output/late records fail atomically before output.

## Accept

Exact required original predictions agree on native/WASM without skips.
All47 saved ground snapshots restore the original axes and retain them through
initialize/prepare. Repeat calls and mode3 refresh events are checked. Complete
strict LLVM19.1 owned format/tidy, sequential production all, fast-math sanitizers
and actual native/browser gates pass. Audit exact source/program/index pins,
commit/push complete consumer and clean obsolete owned logs. Keep full class,
manual device producers, battle/PCM/outcomes and complete-game gate open.
