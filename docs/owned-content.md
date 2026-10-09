# Owned content and original-file independence

This scope extension replaces original-file runtime requirements. The original
game and frozen reconstruction remain optional development references for
behavior and format research. The completed native and WASM builds, packages,
startup, all gameplay, persistence and editor flows require only owned content.
No shipped asset or mandatory provisioning step may depend on original files.
Current original-based previews and decoder evidence remain historical progress;
they do not establish this new acceptance requirement.

## Repository ownership

Use `assets/` for versioned authoring sources, recipes and final runtime content:
Blender models and scenes; procedural texture scripts and parameters; JSON map
generation recipes and owned mission definitions; audio generation sources and
parameters; and generated meshes, textures, maps, UI/cockpit content and audio.
Use `/tmp` for intermediate exports, preview renders, build products and captures.
Keep release assets compact without deleting required final content. Blender and
other pinned authoring tools run offline; softgl remains the only dependency in
`deps/`, and both runtime implementations remain shared C11.

Every content item needs a stable identity, source/recipe, generator/tool version,
seed where relevant, generation command and output manifest. Validate references,
dimensions, geometry, material/texture data, audio and generated output before
loading. A missing content item must fail required acceptance explicitly.

## Complete content inventory

Inventory every original functional content category and its owned replacement:
all vehicles and their articulated parts, interiors/cockpits and instrumentation;
aircraft, weapons/projectiles, wrecks, buildings, trees and environment objects;
terrain, skies, effects and lighting; menu/editor/HUD graphics, labels/fonts and
icons; and every sound, ambience, music or voice category present in the recovered
game. Include all missions/maps, objectives, spawn/route definitions, campaign
content and editor templates. The inventory must identify missing work rather
than treating the currently rendered subset as the whole game.

Models are rebuilt in Blender with clear silhouettes, detailed surfaces and
working articulation. Cockpits need complete geometry, readable instruments and
the required interactions. Generate textures procedurally from owned recipes;
upscaling or recoloring original images does not meet the requirement. Generate
new audio with reproducible sources/recipes and review real playback quality.
Complete category coverage and a substantial quality improvement are both
required; merely replacing file names or achieving numerical compatibility fails.

## Maps, missions and editor

Implement a JSON-parameterized map generator with explicit schema/version, seed,
terrain/environment parameters and heightmap/colormap outputs. Record exact
generation inputs and preserve reproducibility. Own mission definitions and
their spawn, route, objective and campaign relationships. Terrain is deliberately
new; verify routes, placements, traversability, collision and objective behavior
against the intended mission experience rather than original pixel identity.

Load owned outputs through shared validated C11 asset contracts. Native and WASM
must consume the same content and simulation rules. Editor creation, edits,
save/reload and simulation must preserve owned map parameters and mission
semantics without needing FSG/KLC, original palettes, sprites or sound banks.
Optional original-format import/research must remain separate from required flows.

## Acceptance and work order

Complete the currently running bounded contact proof without claiming asset or
game independence. Then add the owned-content inventory and generation/runtime
contracts alongside first-playable integration. Deliver an original-file-free
first mission with owned terrain, vehicles, cockpit/UI and audible feedback;
extend every remaining category and function until the inventory is complete.
Original decoders are research tools, not the release content pipeline.

For each replacement, review actual native/browser images and movement, asset
detail, cockpit/HUD usability, audible output and performance. Compare quality
with development references where available; record concrete improvements and
remaining defects. Physics tests and frame hashes cannot establish quality.
Do not accept a placeholder as a completed replacement.

Finally build, package and run both targets from a clean checkout without
`armoredfist/`, reference images/tools or an original download. Exercise the
complete feature/content inventory, persistence and editor round trips with no
skipped required coverage. Ten consecutive complete clean original-file-free
WASM runs require independent subagent confirmation; any failure resets the
streak. The independent final gate does not authorize delegated implementation.
