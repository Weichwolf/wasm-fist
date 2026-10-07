Type: Work item
Title: First playable mission
Depends: 0040

## Contract

Decode one real map and vehicle into typed C state; render and play a full mission on native and WASM with controls, HUD, weapons, objectives/outcome and audible events.

## Evidence

Renderer integration and the original scenario envelope/metadata decoder are delivered. See closed
0048 for complete 47-mission, 4213-record native/WASM evidence. Closed 0051 supplies all 22 original
KLC/SKY planes and 32 resource palettes with complete original-instruction output comparison on
both targets. Simulation, interactive native presentation and audio delivery remain open.
Closed 0052 supplies complete owned terrain bundles and original mission palette mappings for all
47 scenarios, with native/WASM instruction-oracle and allocation/error-path evidence.
Closed 0053/0049 supplies recovered world coordinates and a shared C original-terrain inspection
scene reviewed as native output and actual Chromium canvas. This static preview has no playable
controls, vehicle/cockpit, objectives or audio; its sky uses only the mapped average color.
Closed 0054 supplies owned typed unit identities/poses, complete snapshots and normal-side
registry/platoon mappings for all 4213 records, compared with actual original allocation,
pose-copy and roster instructions on native/WASM.
Closed 0055 uses the roster-zero vehicle's position and heading for the inspection view, with
missing-player failures and reviewed original TRAIN1/AZER1 native/browser frames.
Closed 0056 supplies owned original directional sprite model families: all 34 families/170 files,
612 records and 1679860 texels pass native/WASM original-instruction, ownership and memory checks.
Closed 0057 supplies all 34 model-code names and default ground-vehicle visual selection for
all 960 original ground snapshots, with independent hull/turret direction and complete 16-bit
facing-rounding proof. Closed 0058 supplies owned authored-resolution model bitmaps with original
part/piece order, anchors, column-major texels, mirroring and zero transparency: all 6432 corpus
bitmaps pass native/WASM and original-instruction checks, with reviewed actual C vehicle assets.
Closed 0059 draws the roster-zero ground vehicle in the shared terrain scene using recovered
world texel scale and direction, alpha/depth/fog and an explicit follow inspection camera.
Actual native/browser AZER1/TRAIN1/INDIA3 views and complete both-target original/constructed
scene, instruction-oracle, strict-style and sanitizer checks pass. It also corrects the earlier
asset-preview row orientation: original sprite rows run upward, bottom row first.
Closed 0060 supplies typed ground-class c296 initialization, complete owned component templates
and explicit original four-stream random stepping. All 960 ground snapshots and complete original
class/template returns are compared on native/WASM. This initializes one stage; full terrain/
suspension installation, target references and interactive controls remain open.
Closed 0061 supplies complete shared ground-motion and manual turret stages: speed/hull servos,
four complete slope profiles, both velocity lanes, wrapped XY integration and component/part
refresh. Both targets pass sustained driving, every speed cap, all 960 original ground snapshots,
complete original near/far returns, strict tooling and memory checks. This stage owns no mission
clock, terrain/collision installation, targeting or platform input loop.

Closed 0062 supplies owned periodic numerical height expansion and exact knot reduction.
Both targets compare complete original resampler returns for all square original planes, all
eight height maps at four runtime sizes through 4096 and repeated size changes. Ground
height/slope sampling and contact publication are delivered by closed 0063.
Closed 0063 supplies shared installed contact for all four classes, retaining independent hull/
turret slopes and preserving altitude until the class transfer stage. Both targets pass all
headings, byte differences, all 960 original ground snapshots at four field sizes, full original
returns, initialization/motion regressions, strict tooling and memory checks. This contact stage changes
no displayed frame or mission/control clock. Closed 0064 connects those stages to an owned
continuous controlled native SDL2/browser scene with current model/camera inputs, rational PIT
clock, held keys/pause/focus, all-47-player manual stage traces, actual visual and sanitizer
evidence. Only the player is displayed; targeting, full class tick, other-unit AI, combat,
objectives and audio remain open. Active 0065 now supplies recovered station selection,
gun/recoil and mechanical reload stages in that shared clock, with press-edge digit/Tab input
and a C/softgl weapon/ammunition/reserve panel. Complete timed traces for all four classes and
all 47 original players, original method comparisons, actual native/browser visual/device gates
and sanitizer checks pass. Live firing/projectiles/hits and audible event delivery remain open;
this does not yet make the mission playable to completion.
Closed 0066 supplies the shared runtime arena/registry metadata owner and normal/low-priority
allocation boundaries, with original corpus/reuse/exhaustion, corruption repair and memory
evidence. Typed live projectile/effect/world payload installation remains under active 0065.

Closed 0067 supplies the complete typed untargeted M1 station-0 launch transaction, including
ammunition/capacity ordering, optional muzzle initialization, physical origin, mechanical state
and sound-dispatch request. Both targets pass actual original-method and full-mission-occupancy
corpus comparisons, required builds/style and memory checks. Consuming world installation,
projectile flight/hits, smoke lifecycle, command eligibility and audible PCM remain under 0065.

Closed 0068 supplies complete shared ordered unit collision and hit aspect for consuming flight.
All per-type rules, wrapped XY admission, encounter RNG, registry/physical association and
first-hit ordering pass complete original-method and all-47-snapshot-world verification on both
targets, required builds/style and memory checks. Projectile advance, ground impact, damage,
effect retirement and full live mission/input/audio behavior remain open under 0065.

Closed 0069 consumes untargeted type-8 shells through age, wrapped flight, current-position ground
contact, grace, ordered collision and expiry. Pending impacts keep the shell alive for actual
damage before original normal-priority explosion allocation and release. Complete type-4/type-18
animation/retirement also passes both-target original-method, full terrain-byte matrix, all-47
collision-world, strict-tooling and memory gates. See docs/projectile-flight.md for the explicit
DOS/PM service and damage boundaries. Live scheduling, actual unit damage/destruction, drawing,
fire commands, audible events and outcomes remain required; no complete mission is claimed.

## Next

0070 supplies shared M1 primary damage to ground classes 0..3, ordered random reactions,
selected-player feedback and immediate destruction/effects/wreck/roster/census plus type-19
retirement. Independent original comparisons and a reaching launch/flight/damage/impact/cleanup
sequence cover this bounded kernel. See docs/vehicle-damage.md for the required remaining target
actions, selected-player loss/UI/takeover, later wreck updates and audio consumers. 0071 delivers
the remaining collision-reachable M1 actions for 5/6/23/26/27 with typed snapshot restoration,
shared source/word-width arithmetic and exact census/reaction/effect/retained-release semantics.
Both targets compare complete original methods and reaching collision/damage/impact/natural
effect cleanup; all 47 occupancy/roster contexts include every current remaining target snapshot.
0072 supplies persistent wreck, destroyed type-26 modes 4..7, complete type-27 updates and
low-priority type-17 smoke admission, drift and natural release. Reaching critical-hit successors
retain distinct emission counters and all original parameter/flag rules; see docs/destruction-smoke.md.
0073 supplies complete retained aircraft 5/6 behavior-12 updates through airborne spin,
ground destruction, post-release emission/motion and natural effect cleanup, with the original
emitter lifetime defect proved and repaired. See docs/aircraft-death.md for complete state and
caller boundaries. Other aircraft behaviors, live type-26 firing and selected-player flows
remain open. 0074 supplies the common world tick prefix and current registry traversal, proved
against original instructions and full existing-class mutable-pass lifetimes. See docs/world-step.md
for the separate voice producer and traversal-only mission corpus boundaries. This does not
install live mission payloads or execute unknown living methods. Closed 0075 now supplies typed
saved-mission installation, physical roster/orphans, retained file-order initialization RNG and
complete conditional tree updates. All 85 TRAIN1 records and ten supported contexts install
correctly; 37 other complete inputs are explicit unsupported failures. See docs/mission-world.md
for original, ownership, reload and memory evidence. Closed 0078 connects this world owner to the controlled scene described below. Input eligibility, visible battle, AI, audible events and mission outcome
remain open.

Continue with 0065: recover and deliver primary player weapon firing, ammunition/reload and
visible feedback, then projectile/hit rules, mission objectives/outcome and shared sound events
in verified bounded steps. Preserve the working continuous driving baseline. Prioritize
actual mission integration of the delivered combat owners before later all-mission
coverage. Consume the saved world through terrain/contact and take-control installation, recover
living ground methods and command eligibility without no-op dispatch. 0076 now consumes the
untargeted M1 station-zero fire branch and publishes canonical shell/muzzle payloads into the
owned world, including shared cooldown and actual TRAIN1 source-release evidence on both targets;
see docs/primary-fire.md. Canonical combat consumption is described below. Recover targeting,
other-unit class/AI behavior, the shared tree-change producer and live type-26 updates as reached;
current sprites
intentionally retain MAL colors. Scenario path/stamp/player mission semantics still need typed
decoding. Every stage requires real native/browser behavior and visual evidence before claiming
a complete playable mission.

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
original PATH/PINF loader/consumer checkpoint identifies owned mission orders under 0082 as
a prerequisite; complete command dispatch remains unimplemented. Other phase/behavior,
contacts, living world scheduling, visible battle, audible PCM and outcomes remain required;
this does not complete a mission.

## Accept

One complete mission runs interactively on both platforms from real provisioned data, with correct movement/combat/objective outcome and audible events. Asset/behavior tests and actual visual checks cover the bounded milestone. This does not close all-mission coverage.
