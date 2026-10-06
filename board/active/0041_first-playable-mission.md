Type: Work item
Title: First playable mission
Depends: 0040

## Contract

Decode one real map and vehicle into typed C state; render and play a full mission on native and WASM with controls, HUD, weapons, objectives/outcome and audible events.

## Evidence

Renderer integration and the original scenario envelope/metadata decoder are delivered. See closed
0048 for complete 47-mission, 4213-record native/WASM evidence. Closed 0051 supplies all 22 original
KLC/SKY planes and 32 resource palettes with complete original-instruction output comparison on
both targets. Vehicle rendering, simulation, interactive native presentation and audio delivery remain open.
Closed 0052 supplies complete owned terrain bundles and original mission palette mappings for all
47 scenarios, with native/WASM instruction-oracle and allocation/error-path evidence.
Closed 0053/0049 supplies recovered world coordinates and a shared C original-terrain inspection
scene reviewed as native output and actual Chromium canvas. This static preview has no playable
controls, vehicle/cockpit, objectives or audio; its sky uses only the mapped average color.
Closed 0054 supplies owned typed unit identities/poses, complete snapshots and normal-side
registry/platoon mappings for all 4213 records, compared with actual original allocation,
pose-copy and roster instructions on native/WASM. Vehicle initialization is still open.
Closed 0055 uses the roster-zero vehicle's position and heading for the inspection view, with
missing-player failures and reviewed original TRAIN1/AZER1 native/browser frames.
Closed 0056 supplies owned original directional sprite model families: all 34 families/170 files,
612 records and 1679860 texels pass native/WASM original-instruction, ownership and memory checks.
Closed 0057 supplies all 34 model-code names and default ground-vehicle visual selection for
all 960 original ground snapshots, with independent hull/turret direction and complete 16-bit
facing-rounding proof. Closed 0058 supplies owned authored-resolution model bitmaps with original
part/piece order, anchors, column-major texels, mirroring and zero transparency: all 6432 corpus
bitmaps pass native/WASM and original-instruction checks, with reviewed actual C vehicle assets.
These steps do not install/draw vehicles into the scene or execute gameplay initialization.

## Next

Recover vehicle world scale/view bearing and model mission-palette selection; project and draw
the verified origin-relative composed ground-vehicle bitmaps in the shared scene. Recover
animation updates and other-class selection;
initialize/install the selected vehicle using the owned definitions. Follow with native SDL2/browser
interactive loops and vehicle/terrain rendering, fixed-step controls,
HUD, combat/outcome and shared sound events as separate verified steps. Scenario-state, path,
stamp and player semantics still need typed decoding before object installation.

## Accept

One complete mission runs interactively on both platforms from real provisioned data, with correct movement/combat/objective outcome and audible events. Asset/behavior tests and actual visual checks cover the bounded milestone. This does not close all-mission coverage.
