Type: Work item
Title: First playable mission
Depends: 0040

## Contract

Decode one real map and vehicle into typed C state; render and play a full mission on native and WASM with controls, HUD, weapons, objectives/outcome and audible events.

## Evidence

Renderer integration and the original scenario envelope/metadata decoder are delivered. See closed
0048 for complete 47-mission, 4213-record native/WASM evidence. Closed 0051 supplies all 22 original
KLC/SKY planes and 32 resource palettes with complete original-instruction output comparison on
both targets. Models, simulation, interactive native presentation and audio delivery remain open.
Closed 0052 supplies complete owned terrain bundles and original mission palette mappings for all
47 scenarios, with native/WASM instruction-oracle and allocation/error-path evidence.
Closed 0053/0049 supplies recovered world coordinates and a shared C original-terrain inspection
scene reviewed as native output and actual Chromium canvas. This static preview has no playable
controls, vehicle/cockpit, objectives or audio; its sky uses only the mapped average color.
Closed 0054 supplies owned typed unit identities/poses, complete snapshots and normal-side
registry/platoon mappings for all 4213 records, compared with actual original allocation,
pose-copy and roster instructions on native/WASM. Vehicle initialization is still open.

## Next

Install the selected roster vehicle using the owned definitions, recover model catalog and
composed sprite-part layouts; follow with native SDL2/browser
interactive loops and vehicle/terrain rendering, fixed-step controls,
HUD, combat/outcome and shared sound events as separate verified steps. Scenario-state, path,
stamp and player semantics still need typed decoding before object installation.

## Accept

One complete mission runs interactively on both platforms from real provisioned data, with correct movement/combat/objective outcome and audible events. Asset/behavior tests and actual visual checks cover the bounded milestone. This does not close all-mission coverage.
