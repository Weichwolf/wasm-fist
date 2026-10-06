Type: Work item
Title: First playable mission
Depends: 0040

## Contract

Decode one real map and vehicle into typed C state; render and play a full mission on native and WASM with controls, HUD, weapons, objectives/outcome and audible events.

## Evidence

Renderer integration and the original scenario envelope/metadata decoder are delivered. See closed
0048 for complete 47-mission, 4213-record native/WASM evidence. Terrain/model decoding, simulation,
interactive native presentation and audio delivery remain unimplemented.

## Next

Continue 0049 with original KLC terrain/colormap and palette decoding. Follow with model catalog
and geometry, actual terrain/model rendering, native SDL2/browser loops, fixed-step controls,
HUD, combat/outcome and shared sound events as separate verified steps. Scenario-state, path,
stamp and player semantics still need typed decoding before object installation.

## Accept

One complete mission runs interactively on both platforms from real provisioned data, with correct movement/combat/objective outcome and audible events. Asset/behavior tests and actual visual checks cover the bounded milestone. This does not close all-mission coverage.
