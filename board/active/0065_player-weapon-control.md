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

The stage-only verification above did not integrate device commands or visible feedback. The
following step connects selection, gun/recoil and reload to the continuous session.

### Shared driving integration and visible weapon panel

The driving session now applies digit 1–5 and Tab press edges through the recovered selectors
and aae8 take-control refresh. Only T80 maps digit 5; its fifth store remains the distinct +b4
word. Held/repeated inputs do not restart selection, paused inputs are ignored without deferral,
and simultaneous edges execute ascending digits then Tab. The rational PIT owner runs gun/recoil
before motion and reload after the phase increment. Complete player/clock/input/feedback
publication is transactional on interval failure.

One read-only weapon query supplies the selected store, class station count, countdown,
continuous-station distinction and reserve. One C/softgl overlay supplies readable station,
ammunition/reserve, mechanical status and bindings on both platforms. It uses an authored font
and opaque panel, preserving complete framebuffer alpha. SELECTED is not firing eligibility.
The session records selection/reload/request counts, last voice ID and notice deadline; these
are feedback observations, not a sound playback queue. See `docs/driving-scene.md` and
`docs/weapon-control.md` for exact contracts and reproduction commands.

Verified on 2026-10-06:

- `bash tools/rewrite/build.sh all`: all 15 native CTest gates and complete WASM gates pass.
- `python3 tools/rewrite/check_style.py`: strict format/tidy pass for all 43 owned C units.
- Pinned `test_driving.py --originals --oracle`: all nine groups pass in 25.934 seconds,
  without skips. Complete typed player/clock/input/feedback/store traces cover all four classes,
  selection/repeat/pause/capability edges, reload partitioning, malformed station rejection and
  all 47 original players over eight installed 2048-square fields on both targets. Original
  setters/control refresh, gun/recoil, phase ADD/index and reload methods join the complete
  motion/turret/contact returns at the explicitly declared manual boundary.
- Production-flags ASan/UBSan/LSan driving probe: all nine groups and the original corpus pass
  in 5.162 seconds, without skips. Actual sanitizer SDL runs in isolated AZER1/INDIA3 copies
  also pass complete-frame, input/pause/weapon/focus and clean-shutdown assertions.
- Actual production SDL and Chromium gates pass with isolated AZER1/INDIA3 originals and
  constructed M1/T80 fixtures. Chromium checks every canvas byte against the current complete
  C RGBA frame, reload expiry, station stores, T80's fifth store, worker startup/failure/teardown
  and input/focus behavior. Native gates inject real SDL keys and check complete changing/stable
  frames and clean exit. Native/browser images were viewed, including the M1 RELOADING→SELECTED
  panel, M3 reserve 10 and T80 station 5/ammunition 20. Compact reviewed captures remain under
  `/tmp/wasm-fist-0065-scene-*-review`; temporary input copies, builds and logs are removed.

Original snapshots may already select/load station 2. The new device tests explicitly switch
stores before timing a reload; they do not change originals or presume a default station.
Sanitizer rendering required longer display settlement before capture. The SDL verifier exposes
that wait without changing pixel/state assertions, after observed captures showed the preceding
unpaused frame rather than the published paused frame.

The scene still has no firing command binding, live projectile, hit, audio or outcome behavior.
This verified integration stage does not satisfy the complete item Accept; 0065 remains active.

### Runtime allocation prerequisite

Closed 0066 delivers shared runtime object occupancy, registry binding/reuse, normal and
low-priority allocation, release and reset. Actual complete M1 primary handlers prove that
ammunition decrements before projectile allocation fails and that muzzle smoke uses the
120-short-object admission boundary. Two reaching original memory corruptions are explicitly
repaired: extended allocation beyond its actual 32 slots and registry search beyond 182 entries.
Both targets pass complete metadata traces for all 4,213 original snapshot imports over 47
missions, every type/admission route and allocator boundary, plus original, strict-tooling and
sanitizer gates. See `board/closed/0066_runtime-object-allocation.md` and `docs/object-pool.md`.
This allocator owns metadata; live typed projectile/effect/world payloads remain open.

### Complete untargeted M1 primary launch

Closed 0067 now supplies the complete already-eligible station-0 launch transaction using the
shared allocator: consumption/marker before allocation, typed shell pose/velocity/origin/profile,
optional low-priority muzzle initialization, reload/recoil/trigger and sound-dispatch request.
The physical origin remains valid when duplicate imports overwrite its registry binding.
Both targets pass 1,308 complete handler-boundary transitions, all 179 M1 actors with full mission
occupancy, complete actual original actor/new-record/metadata comparisons, repaired reservation
exhaustion cases, all 17 native and complete WASM gates, strict tooling and sanitizer checks.
See `docs/projectile-launch.md` and closed 0067 for exact evidence and scope. This transaction
returns initialized payloads to their caller; it does not install live world payloads, advance
flight/smoke, resolve hits, bind fire input or produce audible PCM.

### Ordered collision dependency for consuming flight

Closed 0068 supplies complete bb1b unit collision, inclusive wrapped XY admission, every e518
interaction method, encounter-ordered shared randomness and word-width hit aspect. Borrowed
physical pose views use the existing pool owner; current registry order, overwritten bindings
and orphaned sources remain correct. Type 21 is the actual clc/ret method, and probabilistic
classes 5/6 remain unnamed beyond their proved contract. Both targets pass 35,271 full query
observations, all 47 snapshot worlds/4,213 sources and 179 M1 shell queries, complete original
methods, strict tooling, all 18 native and complete WASM gates, and the production-flags memory
gate. See `docs/unit-collision.md` and closed 0068. Flight must check the first returned hit
against its origin only after the query; it must not search past an origin hit. Projectile
advance, ground impact, damage, retirement, live world/input/audio delivery remain open.

### Consuming shell and transient-effect lifecycle

Closed 0069 delivers shared untargeted type-8 flight/expiry, current-position ground contact,
collision grace and the actual first-hit/origin rule. Ground/unit impacts retain the allocated
shell at a pending boundary; real damage must execute before continuation allocates the exact
normal-priority explosion and releases the shell. Type-4/type-18 complete animation/retirement
uses the same pool owner. Distinct required coverage totals 37,947 valid fixtures and 41,925
updates per target, including the complete reached ground-byte matrix and all 179 M1 launch
positions over 47 complete mission collision worlds. Production gates, strict LLVM and memory
checks pass. See docs/projectile-flight.md and closed 0069 for the staged original boundaries,
expanded-period verification and truthful request-only notice/voice/sound scope. No fire command,
live damage/world/render/audio behavior is claimed by this kernel.

## Next

Recover command eligibility and phase-driven fire from original class/input routines and real
mission state. 0070 supplies M1 primary damage to all four ground classes, including original
aspect/source scaling, ordered reactions, selected-player feedback, immediate destruction,
normal-priority effects/wreck, roster/census and four-update retirement. See
docs/vehicle-damage.md for the explicit selected-player/UI/audio/wreck-update boundaries and
reaching launch→flight→damage→impact cleanup evidence. 0071 supplies the remaining collision-reachable
M1 target actions for 5/6/23/26/27, typed short snapshot restoration, shared word-width arithmetic,
exact reactions/census/effects and retained/released identities. See docs/other-damage.md for
complete original/corpus/reaching evidence. 0072 supplies persistent type-23 wreck updates,
destroyed type-26 modes 4..7, complete type-27 updates and owned type-17 smoke creation, wind
drift, low-word altitude rise and natural release. See docs/destruction-smoke.md for the distinct
artillery counters, disabled/exhausted parameter decay and reaching critical-hit successors.
0073 supplies complete retained aircraft 5/6 behavior-12 updates, post-release motion/emission
and the independently proved emitter lifetime repair; see docs/aircraft-death.md. Living
aircraft AI/other behaviors, live type-26 firing and selected-player flows remain required.
0074 supplies shared world time, scheduled voice production/consumption and resumable current
registry traversal, with complete existing-class mutable-pass lifetimes; see docs/world-step.md.
Its all-47-context corpus proves traversal at class-call boundaries, not living class execution.
0075 supplies owned typed saved-mission payload installation, physical roster/orphans and retained
file-order initialization RNG, including every TRAIN1 record and complete conditional tree updates;
see docs/mission-world.md. Ten complete supported contexts install correctly; 37 other complete
inputs fail explicitly for undelivered classes. Installation does not supply a living dispatcher
or connect the player-only driving baseline to a complete battle.
0076 supplies the consuming untargeted M1 station-zero fire branch, panel/component refresh,
reload blocking and shared failed-attempt cooldown. The mission wrapper publishes complete
shell/muzzle payloads at their actual physical slots while preserving unrelated payloads,
roster and RNG. Both targets compare the original branch and a real owned TRAIN1 selection,
reload and fire sequence. See docs/primary-fire.md for the declared post-behavior boundary;
this does not supply complete living ticks, targeted/other-station fire or audible PCM.
Prioritize the first mission: consume the owned saved world through terrain/contact and
actual take-control installation, then recover complete reached living ground methods and
command eligibility before extending all-mission AI coverage. Preserve the current-entry
iterator and canonical payloads; do not replace unknown methods with no-op dispatch.
TRAIN1 also needs
the shared 9c5d tree-change producer and live type-26 modes 0..3; its delivered conditional 9c4f
tree method and collision/destruction owners do not supply those remaining behaviors.
Preserve the delivered selection/reload clock and truthful display. Replay timed
inputs on both targets and verify complete output/state/error behavior, visuals and memory.
Keep 0041 active until projectiles/hits, objectives/outcomes and audible events form a complete
playable mission; continue subsequent bounded stages without substituting invented rules.

0077 integrates canonical shell flight, actual reached damage and post-damage impact with all
delivered dynamic/death/tree visits. Every returned effect/smoke/wreck installs at its actual
physical slot before further current-entry traversal; combat and installation share one
physical roster. Selected fatal hits suspend visits/fire until the explicit player-loss
consumer acknowledgement, then continue impact once. See docs/mission-combat.md for the
complete state/raw-record gates and explicit living-method, UI/device and constructed-height
boundaries. Full living dispatch, actual command eligibility, battle drawing, audible events and mission
outcomes remain required. Selected controlled contact/take-control installation is delivered
by 0078 below; complete world class/terrain scheduling remains open.


0078 connects the continuous native/browser scene to the selected physical ground actor in the
complete installed world. Contact/take-control installation and timed manual control, camera
and HUD consume that canonical actor directly; final initialization RNG and every other object
are retained. The ten supported installations and 37 explicit unsupported inputs, old all-47
standalone diagnostics, original stage returns, actual devices/pixels, strict tooling and memory
all pass. See docs/driving-scene.md and closed 0078. Complete living dispatch and command
eligibility, canonical battle scheduling/drawing, audible PCM, selected-loss UI and outcomes
remain required; this controlled subset does not complete a playable mission.


Closed 0079 adds the six owned saved ground position samples and original phase-selected
sampling counter to the canonical controlled actor. Both targets pass complete original counter,
phase and 960-snapshot comparisons, reaching TRAIN1 prefix returns, timed control regressions,
strict builds/style, memory and actual native/browser scenes. See docs/vehicle-history.md.
Closed 0080 consumes the complete per-class movement-word, speed-counter and component stage,
with every signed speed/counter/phase, subtraction edge, all 960 saved ground states and
canonical timed/paused behavior verified against original returns on both targets. Full
build/style, sanitizers and actual TRAIN1 devices/frames pass. See docs/vehicle-maintenance.md.
The actual M1 prefix now agrees through six updates. Active 0081 owns the next tick-seven,
phase-46 heading-history/command callback and shared RNG difference. Its verified all-47
original PATH/PINF checkpoint led to closed 0082: complete owned descriptors/waypoint data now
load into the canonical world, with both-target original/build/style/memory/scene gates. See
docs/mission-orders.md. Complete command dispatch remains unimplemented. Other phase/behavior,
contacts, living world scheduling, visible battle, audible PCM and outcomes remain required;
this does not complete a mission.

## Accept

Both running platforms handle primary player weapon commands, ammunition and reload timing
correctly for the recovered bounded contract, with visible verified feedback, meaningful timed
behavior tests, strict tooling and actual native/browser evidence. This item alone does not
claim complete combat, all weapons/classes, mission objectives/outcomes or audio delivery.
