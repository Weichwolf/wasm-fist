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
objectives, HUD and audio remain open.

## Next

Continue with 0065: recover and deliver primary player weapon firing, ammunition/reload and
visible feedback, then projectile/hit rules, mission objectives/outcome and shared sound events
in verified bounded steps. Preserve the working continuous driving baseline. Recover targeting,
other-unit class/AI behavior and complete world/object installation as reached; current sprites
intentionally retain MAL colors. Scenario path/stamp/player mission semantics still need typed
decoding. Every stage requires real native/browser behavior and visual evidence before claiming
a complete playable mission.

## Accept

One complete mission runs interactively on both platforms from real provisioned data, with correct movement/combat/objective outcome and audible events. Asset/behavior tests and actual visual checks cover the bounded milestone. This does not close all-mission coverage.
