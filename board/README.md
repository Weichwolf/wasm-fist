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
| 0048 → 0049 | Scenario metadata/framing delivered → original KLC planes/palette and real terrain. |
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
