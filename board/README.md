# Rewrite work queue

Read `../AGENTS.md` and the current WI. Directory is state: `open/active/closed`. One developer
owns implementation, one bounded step at a time. Continue **0041** after preparation **0040**.
The old board is preserved verbatim under `reference/`; its IDs and requirements describe the
frozen reconstruction, not rewrite completion.

## Work order

| WI | Deliverable |
| --- | --- |
| 0040 | Reference freeze, reproducible softgl integration, strict tooling and rewrite boundaries. |
| 0041 | First playable mission through bounded asset/world/control/HUD/audio steps. |
| 0048 → 0051 → 0052 → 0053/0049 | Scenario readers, terrain bundles/palette maps and first real inspection scene delivered. |
| 0054 | Owned unit identity/pose snapshots and original normal-side registry/platoon roster delivered. |
| 0055 | Terrain inspection placed at the roster-zero vehicle's position and heading. |
| 0056 | Owned original model atlases, directional sprite parts and complete family loading. |
| 0057 | Complete model-code catalog and original default ground-vehicle part pose. |
| 0058 | Owned original sprite assembly with stable part order, anchors, mirror and transparent texels. |
| 0059 | Shared ground-vehicle scene, recovered world scale/bearing and bottom-row-first orientation. |
| 0060 | Typed ground-class initialization, owned component templates and deterministic original RNG. |
| 0061 | Shared ground motion, exact speed profiles/rotation and manual turret refresh updates. |
| 0062 | Owned original numerical height expansion/reduction before ground installation delivered. |
| 0063 | Ground height/slope queries and typed independent hull/turret contact state delivered. |
| 0064 | Continuous controlled native SDL2/browser scene, shared PIT/input/live actor rendering delivered. |
| 0065 | Primary player firing/ammunition/reload and visible feedback; next bounded step within 0041. |
| 0066 | Shared runtime arena/registry allocation, low-priority admission and proved exhaustion repairs delivered for 0065. |
| 0067 | Shared complete untargeted M1 primary launch and typed shell/muzzle payloads delivered; consuming flight/world stages remain under 0065. |
| 0068 | Shared complete ordered unit collision, all interaction type rules, encounter randomness and hit aspect delivered for consuming flight. |
| 0069 | Shared untargeted shell advance, explicit damage boundary, normal-priority impact continuation and complete explosion/muzzle retirement delivered; live-world/damage/audio consumers remain under 0065. |
| 0070 | M1 primary damage to all four ground classes, ordered reactions, immediate destruction/effects/wreck/roster/census and four-tick retirement delivered; remaining target/player/world/audio consumers stay under 0065. |
| 0071 | Remaining collision-reachable M1 damage for 5/6/23/26/27, typed short snapshot restoration, exact reactions/census/destruction effects and retained/released bindings delivered; subsequent class/world/input/audio consumers stay under 0065. |
| 0072 | Persistent type-23 wreck, destroyed type-26 and complete type-27 smoke updates, owned type-17 creation/drift/natural release and reaching damage successors; live world/render/audio consumers remain under 0065. |
| 0073 | Retained type-5/6 behavior-12 updates, complete flight/death/effect lifetimes and the proved smoke emitter lifetime repair; living aircraft AI and consuming world/input/render/audio flows remain under 0065. |
| 0074 | Shared world tick prefix and mutable current-registry traversal, complete existing-class pass lifetimes and all-47 occupancy traversal; live payload installation/class dispatch/fire/audio remain under 0065. |
| 0075 | Typed saved-mission installation, physical roster/orphans, retained ordered initialization RNG and conditional tree updates; all 85 TRAIN1 objects and ten supported contexts delivered. Living class dispatch, dynamic combat installation and playable battle remain under 0065. |
| 0076 | Consuming untargeted M1 station-zero fire, shared failure cooldown and canonical shell/muzzle publication delivered. Both-target original/real-TRAIN1/full-build/style/memory gates pass; flight/damage scheduling, command eligibility and playable battle remain under 0065. |
| 0077 | Canonical combat visits and complete ordered payload publication delivered; one physical roster and selected-loss suspension before impact. Both-target original/build/style/memory gates pass; living dispatch, UI/device/PCM and playable battle remain under 0065. |
| 0078 | Canonical player control/render ownership in complete installed worlds delivered, with both-target original/visual/memory/build/style gates; living battle/PCM/outcomes remain under 0041/0065. |
| 0079 | Canonical ground position histories and reached phase consumption delivered; all counter/phase/corpus, original timed, build/style/memory and real scene gates pass. |
| 0080 | Complete per-class movement-word/speed-counter/component maintenance and six-update reached original M1 prefix delivered; both-target full-domain/corpus/timed/build/style/memory/scene gates pass. |
| 0081 | Active heading/RNG/nested command recovery: actual banks and complete all-47 PATH/PINF original I/O/consumer checkpoint verified; closed 0082 supplies the canonical order input; full command C acceptance remains open. |
| 0082 | Complete owned PATH/PINF descriptors/waypoint storage and canonical installation delivered, with both-target original/source-release/atomicity/build/style/memory/scene gates. |
| 0083 | Complete nested ground command selection using canonical orders/RNG, owned saved selectors and conditional extra random consumption delivered; both-target original/domain/world/build/style/memory/scene gates pass. Full 0081 remains open. |
| 0042 | Menus, settings, campaign/profile progression and persistent save/load. |
| 0043 | All missions/maps/vehicles, AI, combat, objectives and resolved outcomes. |
| 0044 | Keyboard/mouse/joystick, devices, audio timing and link behavior on both platforms. |
| 0045 | Editor create → save → reload → simulate and format round trips. |
| 0046 | Continuous visual improvement with before/after review and performance budgets. |
| 0047 | Complete surface inventory, visual review and independent ten-run final WASM gate. |

0046 starts with the first scene; visual changes remain reviewable independently of behavior
changes. Renderer quality/performance settings require an explicit, verified quality decision.

## Workflow and acceptance

1. Reproduce Next. State the bounded behavior/data contract and its original evidence.
2. Implement readable C and meaningful tests for data, state transitions, persistence, input or
   final audio as reached. A triangle proves renderer integration only.
3. Run both production builds and relevant behavior tests. Run strict style/tidy for owned C or
   configuration changes. Historical filtered tests prove only their recorded scope.
4. Review actual native/browser visuals when presentation changes. Record exact build, commands,
   coverage, limitations and compact results in the WI; artifacts remain in `/tmp`.
5. Close only when Accept is proved. Commit/push the bounded success and remove obsolete owned
   artifacts. Missing output, partial runs and absent coverage never pass.

Use RFC 822 headers `Type`, `Title`, optional `Depends`, then **Contract**, **Evidence**, **Next**,
**Accept**. Preserve IDs. Split milestones into independently provable steps as reached; avoid
copying ownership contracts. Inventory all functional surfaces before claiming completion.
Original runs and recovered C are evidence, not replacement implementations.
