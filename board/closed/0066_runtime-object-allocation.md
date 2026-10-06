Type: Work item
Title: Shared runtime object allocation and bounded identity reuse
Depends: 0054

## Contract

Supply one readable shared C metadata owner for original object arena occupancy, live counts,
dynamic allocation, explicit snapshot registry binding, release and reset. Recover admission,
identity/value reuse and complete valid capacity behavior before live weapon/world payloads use
it. Original memory corruption is diagnosed and explicitly repaired rather than reproduced or
concealed. Keep payload initialization, deletion semantics, firing and frame/audio output under
their simulation owners; this item does not substitute an allocator for those functions.

## Evidence

`sim/object_pool` supplies the complete bounded metadata contract with 150 short and 32 extended
slots and 182 registry entries. `assets/units` owns the shared original type-to-state-size
classification. Normal admission uses the available arena; low-priority admission rejects at
120 short objects before type dispatch. Dynamic allocation requires a vacancy with saved word
zero. Imported bindings preserve requested words and retain overwritten physical allocations.
Release decrements the saved word modulo 65536; retained words reserve vacancies. Those values
are not monotonic generation IDs. Invalid/exhausted requests preserve complete state/output.

Actual complete original allocation/release routines and constructor/deletion payload writes
were checked without instruction hooks. Two separately reproduced original defects justify
the port's exhaustion behavior: the 33rd extended allocation writes over the registry, and an
exhausted reserved registry scan returns index 184 outside its 182 entries. These repairs derive
from the actual arena ends/reset extents; they do not introduce guessed gameplay limits.
See `docs/object-pool.md` for addresses, field ownership, corruption proofs and exact commands.

Verified on 2026-10-06:

- `bash tools/rewrite/build.sh all`: all 16 native CTest gates and complete WASM gates pass.
  The new allocator gate is mandatory on both targets.
- `python3 tools/rewrite/check_style.py`: strict format/tidy pass for all 45 owned C units.
- Pinned `test_object_pool.py --originals --oracle`: all seven groups pass in 17.063 seconds,
  without skips. Complete typed metadata/output traces cover every type and admission route,
  holes/reuse/reservations, duplicate bindings, reset, full arenas/registry, wrapped saved words,
  malformed requests/state and all 4,213 original bindings from 47 missions on both targets.
  Complete M1 primary handlers at 118/119/149/150 initial short objects independently prove
  muzzle admission and ammunition consumption before failed allocation. The initial actor has
  a valid selected/loaded station zero, no reload/recoil/target and the actual pending command 48.
- Native ASan/UBSan/LSan allocator probe with production flags and the same complete original
  gate: all seven groups pass in 10.791 seconds, without skips or memory errors.
- Pinned `test_units.py --originals --oracle`: all seven groups pass in 9.511 seconds, without
  skips, verifying the shared classification alongside all original definition/roster behavior.

No presentation or existing driving state changes. This module is not yet used by live scene
payloads. Temporary logs and the sanitizer executable are removed after commit/push; compact
gate results remain under `/tmp/wasm-fist-0066-pool-review`.

## Next

Continue active 0065 with owned live projectile/effect payloads, original M1 eligibility and
allocation/launch failure behavior. Import mission occupancy and registry identity through this
owner; do not create a second allocator or presume the scene starts with empty short storage.
Keep firing input unbound until the consuming launch/flight stages are functional and verified.

## Accept

One shared allocator handles original valid arena/identity/admission behavior and explicitly
reported exhaustion repairs on native and WASM, with complete requested metadata/error/corpus
coverage, original reaching proofs, production builds, strict tooling and memory verification.
Acceptance covers metadata allocation only; live payloads, gameplay and audio remain open.
