Type: Work item
Title: Primary player weapon control and ammunition/reload feedback
Depends: 0064

## Contract

Extend the real shared driving session toward its first playable mission with recovered primary
player weapon selection, firing eligibility, ammunition consumption and reload timing. Show
verified weapon/ammunition feedback in both continuous scenes. Recover actual weapon slot/type
and input contracts before choosing behavior; do not infer them from initialization counts or
invent the meaning of the common trailing parameter 20. Share state, input and event ownership.
Projectile/hit rules and sound must follow their recovered contracts, not silent placeholders;
retain explicit scope until those stages are delivered.

## Evidence

0064 supplies a continuous real player/terrain/model scene on native SDL2 and browser, shared
rational PIT clock, manual controls and current render inputs. 0060 retains class ammunition,
cycle/stock and trailing parameters with their still-unrecovered firing/reload identities.
The driving scene has no weapon command, feedback, projectile, hit, audio or outcome behavior.

### Spatial launch prerequisite

Shared `fist_rotate_spatial` now recovers complete original 0459 direction arithmetic using the
existing quarter-wave coefficient owner. Complete original M1 station-0 firing reaches the
new direction contract: weapon speed 853 produces horizontal speed 852, because original
0487 clears full-scale carry before the horizontal multiplication. Planar vehicle rotation
retains its distinct full-scale contract. See `docs/projectile-direction.md` for reaching
cases, complete handler evidence, reproducible coverage and the explicit implementation scope.

Verified on 2026-10-06:

- `bash tools/rewrite/build.sh all`: all 14 native CTest gates and complete WASM gates pass.
- `python3 tools/rewrite/check_style.py`: strict format and tidy pass for all 40 owned C units.
- Pinned `test_vehicle_motion.py --originals --oracle`: all 12 groups pass in 98.745 seconds,
  without skips, including 333,690 spatial observations and complete shot handlers for 49
  constructed poses plus all 179 original M1 actors. Existing planar/motion regressions pass.
- A final independent-original primary-handler rerun also proves empty ammunition preserves
  the complete actor and allocates nothing; both C targets observe complete launch velocity.
- ASan/UBSan/LSan native motion probe with production fast-math: all 12 groups and original
  corpus pass in 3.949 seconds, without skips. Reproduction commands are in the linked document.

This changes numerical simulation only; scene pixels and device behavior are unchanged.
Temporary verification logs and the sanitizer executable are removed after commit/push.

### Weapon state, selection and reload stages

Typed start state now retains selected/loaded station, pending trigger, recoil, signed gun
elevation and its authored high-byte pose. Shared `sim/weapon_control` supplies complete
M1/M3/T80/BMP setters, cycling, the 48-step fire request, the class-entry gun/recoil prefix
and phase-driven reload dispatch. T80's separately selectable fifth station reads its actual
word +b4 ammunition. Class-specific empty-store/reserve behavior and complete component marks
are retained, with explicit voice/notice requests. See `docs/weapon-control.md` for ownership,
actual instruction/table locations, bounded original comparisons and reproduction commands.

Verified on 2026-10-06:

- `bash tools/rewrite/build.sh all`: all 15 native CTest gates and complete WASM gates pass.
  The new weapon gate is required on both targets; its eight groups cover every class.
- `python3 tools/rewrite/check_style.py`: strict format/tidy pass for all 42 owned C units.
- Pinned `test_weapon_control.py --originals --oracle`: all eight groups pass in 148.616
  seconds, without skips. 606,611 complete state transitions include all 960 ground actors
  across all 47 original missions, full byte countdown/phase domains, all signed elevations,
  recoil/trigger bytes, class station/stock boundaries and long timed sequences. Actual original
  phase ADD/index instructions, complete method returns, HUD bytes and notices are observed.
- Native ASan/UBSan/LSan weapon probe with production flags: all eight groups and the original
  corpus pass in 33.571 seconds, without skips. No scene/presentation code changes in this step.
- Pinned `test_vehicle_start.py --originals --oracle`: all seven groups pass in 85.861 seconds,
  without skips. The extended typed fields survive source release and match complete original
  initialization, alongside unchanged full RNG/class/corpus regression coverage.

Temporary verification logs and the sanitizer executable are removed after commit/push.

The continuous scene still has no firing command, ammunition/reload feedback, live projectile,
hit, audio or outcome behavior. These stage implementations do not satisfy this item's Accept.

## Next

Connect the delivered selection/pose/reload stages to the shared driving clock and input owner
with visible verified feedback and explicit event ownership. Recover command eligibility and
phase-driven fire from original class/input routines and real mission state. Use the complete
M1 primary-handler oracle to reproduce ammunition,
allocation and launch boundaries before implementing the owned live projectile state. Implement
shared state transitions and truthful visible ammunition/weapon feedback, replay timed inputs
on both targets and verify complete output/state/error behavior, visuals and memory.
Keep 0041 active until projectiles/hits, objectives/outcomes and audible events form a complete
playable mission; continue subsequent bounded stages without substituting invented rules.

## Accept

Both running platforms handle primary player weapon commands, ammunition and reload timing
correctly for the recovered bounded contract, with visible verified feedback, meaningful timed
behavior tests, strict tooling and actual native/browser evidence. This item alone does not
claim complete combat, all weapons/classes, mission objectives/outcomes or audio delivery.
