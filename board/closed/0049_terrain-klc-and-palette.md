Type: Work item
Title: Terrain, colormap and palette decoding for the first real scene
Parent: 0041
Depends: 0048

## Contract

Resolve a decoded scenario's asset names and decode original KLC height/colormap and palette data
into validated typed assets shared by native and WASM, enabling a real softgl terrain scene.

## Evidence

0048 supplies scenario BINF names and position metadata. There are 18 local KLC files and PAL.RES;
47 scenarios use 19 case-normalized height/colormap/palette/sky combinations (28 spelling variants).
Closed 0051 supplies shared KLC1,
RESOURCE1 and six-bit palette readers: all 22 KLC/SKY planes and all 32 palettes match complete
original instruction output on native/WASM. Closed 0052 supplies complete scenario bundle loading,
original mission palette sorting and color/sky mapping, verified for all 47 scenarios and 38 mapping
pairs. Closed 0053 supplies recovered world axes/scale and a shared C/softgl original-terrain
inspection scene, with complete native/WASM frame contracts and actual native/Chromium visual
review. See `docs/terrain-format.md`, `docs/terrain-loading.md` and `docs/terrain-scene.md`.
Vehicle models, playable camera and simulation remain under 0041.

## Next

Terrain data and the first real inspection scene are complete through 0051/0052/0053. Continue
0041 with player/unit semantics, vehicle geometry and interactive native/browser presentation.
The preview makes its camera/quality choices explicit; it does not promise vehicle/cockpit behavior.

## Accept

Required real height/colormap planes and palette decode completely with no missing/trailing data
or invented constants; native/WASM and independent reference checks cover the encoding and invalid
inputs. First real terrain scene uses these assets and receives actual native/browser visual review.
