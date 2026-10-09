Type: Work item
Title: Deliver complete canonical shared ground physical contacts
Depends: 0121, 0126, 0127

## Contract

Implement both selection wrappers and the complete ordered physical contact
transition in shared C11 over the canonical mission world. Preserve admission,
cooldown, directional proximity, wrapped radius, near-obstacle prediction,
first/repeated/no-hit behavior and complete tree damage/release/RNG/display
consequences. Own saved contact/cooldown/tree-health fields explicitly; reuse
existing simulation, geometry, damage arithmetic, pool and notification owners.
Terrain contact remains separate. Do not install a partial living-class bank.

## Evidence

Closed 0127 proves the unchanged original with nine required groups, 434112
counted contact/retention observations and genuine constructor/source/flight/
kernel/height coupling. It records domain/corpus counts, output pins and strict
comparison scopes. Its source retirement/reuse sequence proves a stale cached
source-side defect; bit identity does not require reproducing that defect. The
reference tests continue to specify the original behavior. The complete 0126
shared manual owner and its production gates remain the last accepted C.

The current draft is fist_mission_world_ground_contacts in sim/ground_contacts.c.
It composes the existing proximity, obstacle prediction, damage arithmetic/RNG
and pool owners in an atomic canonical-world transaction. Explicit vehicle
contact_flags/contact_cooldown and tree damage restore their proved saved bytes.
The canonical damage caller captures source side as a value; later retirement
and reuse cannot change it. Initial null-source side is clear, as proved by0127.
Collision sound borrows the two real c047 records and the independent 9fdf owner,
requiring configuration only for an admitted first hit from the matching source.
Requests/selector history and complete damage-display events are logical output;
kernel transport, PCM and full engine/configuration producers remain separate.

Development evidence passes 272417 independent native contact observations and
whole typed write-footprint checks, with original coupling explicitly skipped in
that initial development run. Separate saved-field gates pass 6144 ground and
6144 tree restoration/initialization/readiness/variant observations. The genuine
shared C source sequence passes eight contexts, 24 flight updates, eight actual
impacts/retirements, eight same-slot neutral smoke constructors and eight
consuming contacts through all four ground classes. It captures both source sides
and retains the captured value across reuse. These are declared caller inputs,
not complete battle scheduling. The 520-case guard pilot passes 56 atomic used-
input/late failures and 464 genuinely unused malformed inputs, including rollback
after an earlier predictive obstacle write. These pilot runs are development evidence; final acceptance is recorded below.

The first synthetic tree pilot omitted the real tree damage record and retained
arbitrary saved pool indices. Its mismatched expected tree damage/occupancy was
rejected; fixtures now supply the pinned (100,0) record and proper allocated
physical indices. Compiler/style development findings are corrected without
weakening warnings, assertions or checks. Reproduction fixtures remain versioned;
disposable programs, logs and compact results stay under /tmp.

The first frozen complete gate passed style and all three builds, then was
interrupted after an independent Valgrind failure. Whole-byte transaction checks
consumed undefined padding copied from vehicle restoration's local aggregate.
Restoration now explicitly initializes the representation before decoding its
members. No comparison or memory check is suppressed. The corrected native
development gate passes all four focused groups under Valgrind without errors
or lost blocks: saved fields, genuine source retirement/reuse, all 520 guards
and 128 retained 32-call contact sequences (4096 consuming calls). The retained
sequences cover both wrappers, all four classes, cooldown/repeated/no-hit
transitions, recoil departure, retained/released trees and byte-wrapped damage.
The strict style gate subsequently rejected memset and the new indirect waypoint
type dependency. The final draft uses a bounded unsigned-character loop and
direct goal-member stores; no check was weakened. Complete LLVM19.1.x style
(103 translation units) and native/WASM/production-fast-math sanitizer builds
pass on the current frozen source. The complete required contact gate now passes
nine tests without skips: 705001 observations per actual program, including
427968 unchanged original contacts, 15360 canonical prepared-world contacts,
all 520 guards and 128 held sequences/4096 consuming calls. Its output digest is
da5aeec5e6ba77603163ed53f8069ffb1f0134abc5e24923e983e79589834ace.
The actual gate exit is 0 after 2350.460 seconds. All final bounded gates now pass, as recorded below.

The current sequential supervisor is wasm-fist-0129-complete-v4.service. Its
573-file immutable source is /tmp/wasm-fist-contact-integration-source-v3; exact
commands, source hashes, actual terminal receipts and logs are under
/tmp/wasm-fist-0129-review/v4. Explicit leak-checking ASan and fail-fast UBSan
settings govern the complete three-program contact gate. The five-process
production Memcheck gate now passes all four groups/five actual processes with
no errors, suppressions, lost blocks or remaining heap bytes. It covers saved
fields, genuine source lifetime, 520 guards and 4096 held consuming contacts.
All 54 native production CTests pass (1379.91 seconds). Complete WASM production
regression also passes: 52 Python scripts, 50 unittest suites/369 tests, two
plain behavioral gates and two actual Node checks. The required original-contact
gate has zero skips; optional historical original groups in the ordinary
regression remain separate. All 13 sequential stages have actual exit code zero.
The contact/memory audit verifies all nine actual terminal receipts,
573 unchanged source files, original model/output pins and every Memcheck log.
Earlier interrupted and failed runs remain excluded. The branch histogram
labels only each encoded input's first call; all held successor outputs are
independently compared and included in the 705001-call digest.

## Final acceptance evidence

The complete audit verifies every exact command/terminal receipt/log hash,
all 573 frozen source files, LLVM19.1.7 style across 103 translation units,
production flags and three actual contact programs. The scene audit verifies
13 canonical native and six canonical Chromium captures, complete opaque
frames, terrain/vehicle content, actual held input, independent turret motion,
pause/weapon/reload/focus stability, failure cleanup and normal 5/30-second
presentation deadlines. All 19 actual captures were visually inspected:
terrain and the assembled player remain visible, and HUD states are readable.
Terrain softness and small vehicle framing remain explicit quality work in0046;
the independent 31-image visual review is recorded in docs/visual-review.md.

Compact accepted evidence is under /tmp/wasm-fist-0129-review/v4:
complete-acceptance-audit.json, scene-capture-audit.json, contact-memory-audit.json,
complete-memcheck.json and contacts/c-ground-contacts.json, with actual terminal
receipts and reproducible fixture/program/source pins. The exact full gate
commands are in complete-gates-spec.json; docs/ground-physical-contacts.md gives
the required contact reproduction command and comparison scope.

The source-side value repair, saved fields and complete canonical physical
contact owner are accepted. No partial living-class bank is installed. Original
comparisons are optional development references under the expanded owned-content
scope; this original-based diagnostic scene does not accept original-file-free
assets, first playable battle, PCM or full-game quality. Full-game WASM streak
remains zero.

## Next

Reuse this complete owner in0121's complete living-class transaction. Follow
0130 and the requested JSON heightmap/colormap generator first; deliver owned
content and original-file-free first-playable integration without losing the
proved gameplay contracts. Complete class/world/battle/PCM and full0047 remain
open.

## Accept

Both targets consume the complete canonical contract with correct selection,
write footprints, geometry, RNG, lifetimes, resource release and notifications.
Saved fields restore/retain correctly. Documented source-lifetime repair is
proved independently of the original model; invalid used input fails atomically,
and genuinely unused input stays ignored. Missing cases/output, partial runs or
substituted runtime providers fail. Full0121/class/world/battle/PCM/game and the
final independently verified ten-run gate remain open after this bounded owner.
