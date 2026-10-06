Type: Work item
Title: First playable mission
Depends: 0040

## Contract

Decode one real map and vehicle into typed C state; render and play a full mission on native and WASM with controls, HUD, weapons, objectives/outcome and audible events.

## Evidence

Renderer integration and the original scenario envelope/metadata decoder are delivered. See closed
0048 for complete 47-mission, 4213-record native/WASM evidence. Closed 0051 supplies all 22 original
KLC/SKY planes and 32 resource palettes with complete original-instruction output comparison on
both targets. Terrain installation/rendering, models, simulation, interactive native presentation
and audio delivery remain unimplemented.
Closed 0052 supplies complete owned terrain bundles and original mission palette mappings for all
47 scenarios, with native/WASM instruction-oracle and allocation/error-path evidence.

## Next

Continue 0049 with terrain placement/rendering and a supported inspection camera.
Follow with model catalog
and geometry, actual terrain/model rendering, native SDL2/browser loops, fixed-step controls,
HUD, combat/outcome and shared sound events as separate verified steps. Scenario-state, path,
stamp and player semantics still need typed decoding before object installation.

## Accept

One complete mission runs interactively on both platforms from real provisioned data, with correct movement/combat/objective outcome and audible events. Asset/behavior tests and actual visual checks cover the bounded milestone. This does not close all-mission coverage.
