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
| 0084 | Complete nested route/formation goal assignment using canonical orders/physical roster and retained heading/goal ownership delivered; both-target original/domain/world/build/style/memory/scene gates pass. Full 0081 remains open. |
| 0085 | Rewrite development and CI moved to master; ghidra retains the exact frozen decompiled/patched reconstruction. Both refs published without rewriting history. |
| 0086 | Complete shared planar navigation bearing/distance and local numeric scratch delivered; both-target original/domain/corpus/build/style/memory and canonical regressions pass. Existing scene binaries/evidence are unchanged; full 0081 remains open. |
| 0087 | Complete route progress, retained unsigned range and proved full-capacity neighbor-read repair delivered; both-target original/corpus/consuming-world/build/style/memory and current scene gates pass. Full 0081 remains open. |
| 0088 | Complete original mission-ready registry/class reset, all-47 target clearing/releases/tree RNG and reaching TRAIN1 command boundary verified. Canonical C readiness and full 0081 remain open. |
| 0089 | Rewrite-only master layout delivered: concise README, no workflows, root tests/tools, only softgl in deps and pinned /tmp reference images; full build/style/original/presentation gates pass and ghidra remains immutable. |
| 0090 | Complete all-47 saved restoration and shared mission preparation before player control delivered; full original/reset/RNG/corpus/build/style/memory and three real native/browser scene gates pass. Full command/battle/PCM remain open. |
| 0091 | Complete shared terrain visibility using the existing installed-height owner delivered; both-target original/corpus/full-build/style/memory and current SDL/browser gates pass. Runtime aiming/target discovery and full 0081 remain open. |
| 0092 | Complete shared directional target proximity delivered; actual original aim/visibility/scan proves its different nearest target. Both-target original/corpus/build/style/memory gates pass; runtime identities/full discovery and 0081 remain open. |
| 0093 | Complete shared target discovery, physical reference lifetimes and logical voice admission delivered. Both-target original/all-47 prepared-world/release-reuse/build/style/memory and real scene gates pass; full 0081, target acquisition and living battle/PCM remain open. |
| 0094 | Complete shared command bearing/range/retreat and deliberate target-loss navigation delivered. Both-target required original/domain/all-47 canonical, full-build/style/memory and actual scene gates pass; full throttle/gear/0081 parent and living battle remain open. |
| 0095 | Complete shared throttle bank and signed-pitch drive-profile/class/component/display transitions delivered. Both-target required original/domain/all-47 canonical/reached-repair, strict full-build/style/memory and actual scene gates pass; target feedback, full 0081 and living battle remain open. |
| 0096 | Complete original acquisition/aim/consuming throttle and genuine parent entry-eight checkpoint delivered. Required full-domain/all-47 and paired-observer gates pass; shared C consumption is delivered by 0097 and full 0081 remains open. |
| 0097 | Complete shared target installation, aim feedback and selected typed notifications delivered. Both-target required original/domain/all-47 canonical, full-build/style/memory and actual scene gates pass; remaining callbacks/full 0081 and living battle/PCM remain open. |
| 0098 | Complete original physical roster promotion and genuine parent entry-thirteen evidence delivered. All-47 prepared worlds, flag domains, actual allocated orphans and used-domain repair evidence pass; shared C is open 0099 and full 0081 remains open. |
| 0099 | Complete shared canonical physical roster promotion delivered. Required native/WASM original/domain/all-47, full-build/style/memory and actual scene gates pass; remaining callbacks/full 0081 and living battle/PCM remain open. |
| 0100 | Complete original maneuver/idle turret/predictive collision and genuine parent-entry evidence delivered. Full domains/all-47 prepared worlds, retention, release/reuse presence and every search exit pass; shared C is delivered by 0101 and full 0081 remains open. |
| 0101 | Complete shared obstacle maneuvers, idle turret and predictive collision delivered; both-target required original/domain/all-47, full-build/style/memory and actual scene gates pass. Full 0081 and living battle/PCM remain open. |
| 0102 | Complete original af97/afa2, missile readiness/launch and genuine parent entries five/ten/fourteen delivered; required full domains/allocations/all-47/retention gates pass. Shared C is delivered by 0103 and full 0081 remains open. |
| 0103 | Complete shared automatic fire, ordered M3/BMP rack service, real type-15 constructor and logical notifications delivered; required both-target original/canonical, full-build/style/memory and scene gates pass. Full 0081 remains open. |
| 0104 | Complete original ae5c/b0be and genuine parent-entry nine/twelve recovery delivered; all eight required groups and all-47/four-detail coverage pass. Shared C is delivered by0107; full0081 stays open. |
| 0105 | Complete original op-64 queue/return, consumed retreat/support coupling and authored sentinel-overread evidence delivered; all eight required groups pass. Closed 0104 consumes this evidence; shared support repairs are delivered by0107 and PCM remains open. |
| 0106 | Complete original support resets/catalog producer, four-gun consumption and ammunition/lifetime evidence delivered; five support groups and all eight audio regression groups pass. Proved cooldown/stale-resource defects stay reference-only; shared C repairs are delivered by0107. |
| 0107 | Complete shared station selection and retreat support with canonical resources/requester lifetimes, explicit configuration and proved cooldown/audio-dependence repairs delivered; both-target domain/all47, full-build/style/memory/original and actual scene gates pass. Full0081 and living battle/PCM remain open. |
| 0108 | Streaming all47 prepared-world inputs, eight real heights/four details and complete original/current native producer observations delivered for0107; consuming/lifetime acceptance is delivered by0107. |
| 0109 | Complete all47 canonical station/support child inputs and independent predictions verified against 42240 actual DOS returns, real constructor heights and matched-source audio contexts; shared-C consuming/lifetime acceptance is delivered by0107. |
| 0110 | Complete original selected-ground diagnostic, genuine text relocation/frame/label setup and selected/unselected parent tails proved: eight required groups,364288 diagnostic/parent returns and608 setup/96 coupled returns pass. Shared full0081 consumption remains open. |
| 0111 | Complete original ab03 composition through both full callback banks, independent heading/RNG/diagnostic prefix/tail and actual PM operations proved: seven required groups,409792 parent/405600 child returns and all47/four-detail coverage pass. Shared C/full0081 consumption remains open. |
| 0112 | Complete exact op-54 signed EBX transport prediction delivered: eight required groups without skips,475328 parent/471136 child returns and65536 velocity-word constructor transfers pass. Shared full0081 acceptance remains open. |
| 0113 | Independent synthetic full-bank/retained world-event models and reached TRAIN1 M1 command order through update54 proved by three required original groups without skips; full shared C/class acceptance remains open. |
| 0114 | Complete original target-aware turret order for all four ground classes proved by four required groups and279880 full returns, including heading/branch domains, retained moving targets and actual release/reuse. Shared consumption/full0081 remain open. |
| 0115 | Target-motion original gate strengthened to five required groups and311800 full returns, preserving prior digests and adding both angle modes, self aliases and signed coordinate wrap. Shared C gates/full0081 remain open. |
| 0116 | Production renderer numerical repair, fixed WASM shared heap and source-map debug policy delivered; warning-free full native/WASM build, strict style, all 737 dependency tests, actual scenes and all-47/four-detail presentation pass. Full 0081/battle/PCM remain open. |
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
