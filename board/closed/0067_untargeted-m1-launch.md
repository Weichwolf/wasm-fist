Type: Work item
Title: Shared complete untargeted M1 primary launch transaction
Depends: 0065, 0066

## Contract

Recover the already-eligible untargeted M1 station-0 handler in readable C. Deliver ammunition
and component consumption, normal projectile allocation, complete typed launch payload, optional
low-priority muzzle initialization, reload/recoil/trigger publication and the original sound-dispatch
request. Use the existing metadata/rotation owners and actual mission occupancy. Keep eligibility,
input binding, flight/collision, effect lifecycle, live world installation and audible PCM explicitly
open under 0065; this bounded item does not replace its complete acceptance requirements.

## Evidence

Actual original physical 17745..17789 consumes ammo before allocation, initializes its type-8 shell
through b725/1ace0, attempts low-priority type-18 smoke through 9b5c/9b6f, dispatches AX=12 and
publishes reload 20/recoil 16/trigger zero only on shell success. Original owner +27 is a physical
actor pointer even when a duplicate import has overwritten its registry binding. Typed payloads
normalize that pointer to a physical slot; no raw guest memory becomes runtime state. See
`docs/projectile-launch.md` for all launch fields, instructions, ownership and reproduction commands.

Shared `fist_m1_launch_untargeted` now implements the transaction with EMPTY/CAPACITY/FIRED outcomes.
Read-only validation reuses the pool owner; invalid input preserves pool/actor/output. The caller
receives initialized payloads by value and owns their world transfer. Missing optional smoke,
including identity exhaustion below the occupancy admission gate, does not invalidate a shot.
The previous pool transcript writer is shared with the launch probe rather than copied.

Verified on 2026-10-06:

- `bash tools/build.sh all`: all 17 native CTest gates and complete WASM gates pass.
  The new eight-group launch gate is mandatory on both targets.
- `python3 tools/check_style.py`: strict LLVM 19.1.7 format/tidy passes all 48 owned C units.
- Pinned `test_projectile_launch.py --originals --oracle`: all eight groups pass on both targets
  in 28.476 seconds, without skips. 837 complete valid fixtures yield 1,308 launch transactions
  per target, including every side byte, every physical extended origin, both rotation modes,
  position/angle wraps, empty/max ammunition, capacity/failure ordering, optional smoke, holes,
  reserved identities and overwritten origin bindings. All 179 original M1 actors run two shots
  with complete mission occupancy from all 47 pinned scenarios. The independent original oracle
  executes 1,301 of those transactions and checks every actor byte, complete metadata and every
  new 55-byte record, plus unchanged unrelated arena payloads. Seven transactions in three
  explicit fixtures cover the proved original unbounded-registry repair without invoking its
  known corrupt route; both C targets must pass those repaired exhaustion cases.
- Production-flags ASan/UBSan/LSan launch probe with the same complete corpus/original gate:
  all eight groups pass in 23.182 seconds, without skips or memory errors.
- Pinned `test_object_pool.py --originals --oracle`: all seven groups pass on both targets in
  17.585 seconds, without skips, covering all 4,213 snapshot imports and existing constructor,
  release, allocation/admission, corruption and complete-handler contracts after observer reuse.

No rendered frame, input binding or audible playback changes. Active 0065/0041 remain open.
Temporary verification logs and the sanitizer binary are removed after commit/push; compact
results remain under `/tmp/wasm-fist-0067-launch-review`.

## Next

Continue 0065 with consuming flight/hit/effect lifecycle and real world installation before
connecting the device firing command. Preserve the complete original mission occupancy and
physical origin association; keep sound dispatch requests separate from audible PCM delivery.

## Accept

Both production targets implement complete original untargeted M1 launch semantics at the stated
handler boundary, including every initialized field and failure ordering, actual mission occupancy,
physical origin/overwritten bindings, malformed requests/state, complete reaching original-method
comparisons, strict tooling and memory checks. No claim of audible playback, flight, hits, live
firing input or complete playable mission follows from this acceptance.
