Type: Work item
Title: Shared ground-vehicle motion and manual turret updates
Depends: 0060

## Contract

Implement the complete motion stage shared by all four ground classes: original speed and
hull servos, both heading-derived velocity lanes, position integration, component refresh
markers and untargeted turret slew. Preserve original cadence gates, slope/profile limits,
turn wrapping and hull recentering. Simulation callers own phase advancement and resolved
target aiming; this stage does not replace the full vehicle/mission controller.

## Evidence

Actual original 7cd5/88e4/912d/98c3 call service 1a401, 1a395 and 1a1d6 then integrate signed
velocity words into 32-bit XY. Speed profiles are GS STR:2b84, not DGROUP. The planar rotation
03a9 calls 05dd/05e2 and uses SS:2042 quarter-wave data, with interpolation and a separate full
coefficient carry result. Manual turret wrappers 7d0f/8917/90cd/9911 clamp to 364 turn units,
mark components and restore absolute turret heading. Original instructions and frozen data
will be the oracle for complete motion/untargeted turret returns.

## Next

Complete. Continue parent 0041 with terrain/suspension installation, original input/controller
rules and the first moving native/browser scene. Then integrate full vehicle timing, targets,
weapons, collisions, HUD, objectives and sound without claiming these from a motion-stage test.

## Accept

All four classes produce correct complete motion state, refresh flags and manual turret updates
on native and WASM. Tests exercise actual original returns, coefficient/speed-profile boundaries,
phase/flag/gate behavior, signed/wrapped state and sustained movement. Input failures preserve
state/events. Full corpus, strict tooling, production and sanitizer checks pass. No collision,
terrain installation, target solver, mission input loop or playable completion claim is made.


## Verification

2026-10-06: `bash tools/build.sh all` passes all eleven native CTests and every WASM
asset/scene/pixel gate. `python3 tools/check_style.py` passes all 31 owned C units with
unchanged LLVM 19.1 rules and production compiler flags. Default gates explicitly skip only
the separately requested original groups.

`test_vehicle_motion.py --originals --oracle` passes all eight groups on both production
targets, no skips (54.194 s). The gate compares 196800 complete planar rotations per target:
every 16-bit heading in both math modes, signed/cardinal/interpolation boundaries and every
signed magnitude at quarter turn. The independently calculated knot/interpolation reference
matches actual complete 03a9 returns. It distinguishes coefficient 65535 without carry from
the full-length result; near-cardinal 32/33 with magnitude 321 proves the reaching difference.

The gate compares 52892 complete motion observations per target against independent expected
writes and full original class motion/manual turret returns. It observes actual speed/hull
carry returns separately, then executes the complete class method from the same unchanged
source, including component refresh and signed position integration. Every byte of the 251-byte
original result, global scene refresh, stack/segments and typed C output agrees. Targets are
explicitly absent at this manual-stage boundary; no original services are replaced or patched.

Coverage includes all 256 motion flag/phase bytes, independent gate/phase/direction/sign cases,
all four profiles over signed pitch bins and every profile-selector byte, wrapped normal-hull
and manual-turret slew limits, recenter bounds/zero refresh, all 128 slope/profile caps reached
by 256-step drives, four 1024-step sustained drives, braking/reverse drives and 32-bit XY wrap.
All 960 ground snapshots across 47 hash-pinned FSGs are included; hashes remain unchanged.
Invalid requests/files/class/component sizes preserve state/events. Request/snapshot storage
is freed before any motion/observation. ASan/UBSan with leak checking passes all eight complete
native groups, no skips (2.441 s).

The added typed animation selectors are preserved from the definition and reset by original
manual turret updates. Shared probe serialization has one owner. The initialization regression
`test_vehicle_start.py --originals --oracle` still passes all seven groups on both targets,
including its full corpus and every random-word/stream comparison (85.944 s). No generated
engine code, host RNG, guessed frequency or meshoptimizer runtime dependency is introduced.

An early reaching fixture failure exposed a wrong assumption in the test request reader: the
four serialized component templates have distinct start/end offsets. Correct the reader to
use the actual class starts, preserving complete payloads; expectations remain based on original
execution. Strict checks also require initialized padding for byte-preservation diagnostics.

Fields, class marker offsets, exact limits, math details and reproduction/sanitizer commands
are in `docs/vehicle-motion.md`. This simulation-only step changes no displayed frame; reviewed
WI 0059 native/browser captures remain the presentation baseline. Full terrain/collision,
target solving, mission timing/input, combat, damage/animation, HUD and audio remain open.
Commit/push the verified bounded step and remove its owned temporary logs and sanitizer binary.
