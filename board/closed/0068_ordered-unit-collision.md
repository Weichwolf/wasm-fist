Type: Work item
Title: Shared ordered unit collision and hit aspect for consuming projectile flight
Depends: 0065, 0066, 0067

## Contract

Recover and implement complete original bb1b unit collision, inclusive XY admission, all
per-type interaction methods, deterministic encounter-ordered randomness and the following
hit-aspect arithmetic in typed C. Borrow actual occupied world poses through one view and the
existing pool/random owners. This is the collision dependency of 0065 flight; it does not replace
flight, terrain impact, damage, effect lifecycle, firing eligibility/input or audible delivery.

## Evidence

Actual e454 dispatch sends type-8 projectiles to b5e7: it advances age, retires at 480, integrates
stored velocities, checks terrain, then decrements its initial two-step unit-collision grace or
executes bb1b. The returned hit is compared with the physical origin only after the walk. Flight
must retain this ordering rather than filtering the origin out of the collision query.

Actual bb1b walks all 182 registry entries in order, excludes absent/self/noncollidable entries,
uses 0ea9's inclusive independent XY bounds and tail-dispatches c14f through e518. The complete
source/type threshold table, all class-specific height methods and type-26 modes were rechecked.
Type 21's method is the clc/ret at 9c97, not the adjacent height-table method. Type-5/6 gameplay
identity remains unclaimed; their proved rule consumes RNG before height/threshold acceptance.
See `docs/unit-collision.md` for all instructions, fields, rules and scope.

Shared `fist_collision_find` now delivers those rules and physical/current registry hit identity,
using borrowed typed pose views. Common `fist_object_pose` lives in `sim/world.h` so collision
has no weapon-launch dependency. Pool/body state is read-only. Invalid input prevalidates the
complete world before random/output publication. Complete original-method execution confirms
native behavior, including every source-type/random-byte pair at all four stream cursors.

Verified on 2026-10-06:

- `bash tools/rewrite/build.sh all`: all 18 native CTest gates and complete WASM gates pass.
  The seven-group collision gate is mandatory on both targets.
- `python3 tools/rewrite/check_style.py`: strict LLVM 19.1.7 format/tidy passes all 50 owned C units.
- Pinned `test_collision.py --originals --oracle`: all seven groups pass in 132.596 seconds on
  both production targets, without skips. 35,271 query observations per target cover every
  source/target type, all eight type-26 modes, all collidable flag bytes, inclusive XY/height
  edges, dword/word wraps, every source-type/random-byte pair at all four cursors, encounter
  randomness, current/overwritten/orphan bindings and first-hit/aspect ordering. The corpus
  includes all 4,213 source queries in all 47 pinned snapshot worlds plus 179 M1 launch-position
  shell queries. Actual complete bb1b/0ea9/c14f methods and following aspect instructions match
  complete outputs/RNG without hooks or replaced routines; all arena/metadata bytes stay intact.
- Production-flags ASan/UBSan/LSan collision probe: all seven groups and the same complete
  original snapshot corpus pass in 1.884 seconds, without skips or memory errors. Reproduction
  commands live in the linked document; this memory gate uses the behavioral expectations
  independently proved by the complete original gate above.

No displayed frame, device firing command, damage, effect or PCM changes. Active 0065/0041 remain
open. Temporary logs, sanitizer binary and the generated repository Python cache are removed
following commit/push; compact evidence remains under `/tmp/wasm-fist-0068-collision-review`.

0071 corrects the former type-26 helicopter label using actual byte-indexed render table `e48c`:
type 26 selects `TARGETS`, while types 5/6 select `APACHE`/`HIND`. Height values, query ordering,
coverage and acceptance above are unchanged. See docs/other-damage.md for the pinned assignments.

## Next

Continue active 0065 with consuming projectile flight, original ground contact, damage/effect
lifecycle and live world installation before device fire binding. Use this query's first result
before the physical-origin check; preserve encounter-ordered shared randomness and actual mission
occupancy. Complete firing/audio/objective acceptance remains required for the playable milestone.

## Accept

Both targets implement all original valid unit-collision/type/random/order rules and the hit
aspect at this stated boundary, with complete physical/registry/pose ownership, error atomicity,
source/type/mode/range/height coverage, every pinned snapshot world, complete original-method
comparison, required production builds/style and memory checks. This prerequisite does not claim
live firing, projectile advancement, damage, effect progression/removal, audible PCM or a complete
playable mission.
