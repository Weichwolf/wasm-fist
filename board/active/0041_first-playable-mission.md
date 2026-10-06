Type: Work item
Title: First playable mission
Depends: 0040

## Contract

Decode one real map and vehicle into typed C state; render and play a full mission on native and WASM with controls, HUD, weapons, objectives/outcome and audible events.

## Evidence

Only renderer integration exists. No asset decoder, simulation, interactive native window or audio delivery is implemented.

## Next

Inventory original map/model/mission formats and their recovered readers. Record a bounded decoder contract and fixtures, then implement its validated output. Follow with terrain/model rendering, native SDL2/browser loops, fixed-step controls, HUD, combat/outcome and shared sound events as separate verified steps.

## Accept

One complete mission runs interactively on both platforms from real provisioned data, with correct movement/combat/objective outcome and audible events. Asset/behavior tests and actual visual checks cover the bounded milestone. This does not close all-mission coverage.
