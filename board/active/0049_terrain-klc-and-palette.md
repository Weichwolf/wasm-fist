Type: Work item
Title: Terrain, colormap and palette decoding for the first real scene
Parent: 0041
Depends: 0048

## Contract

Resolve a decoded scenario's asset names and decode original KLC height/colormap and palette data
into validated typed assets shared by native and WASM, enabling a real softgl terrain scene.

## Evidence

0048 supplies scenario BINF names and position metadata. There are 18 local KLC files and PAL.RES;
47 scenarios use 28 height/colormap/palette/sky combinations. Closed 0051 supplies shared KLC1,
RESOURCE1 and six-bit palette readers: all 22 KLC/SKY planes and all 32 palettes match complete
original instruction output on native/WASM. See `docs/terrain-format.md`. Scenario bundle resolution,
mission palette remapping, terrain geometry/world units/camera and vehicle models remain open.

## Next

Resolve scenario asset names into a complete typed bundle, recover mission palette remapping at
9ec0 and world height/axis units from original map setup/sampling. Recover camera placement from
scenario/player evidence; decoded row order alone does not prove world axes. Render the resulting
real terrain and review actual native/browser output before accepting this scene milestone.

## Accept

Required real height/colormap planes and palette decode completely with no missing/trailing data
or invented constants; native/WASM and independent reference checks cover the encoding and invalid
inputs. First real terrain scene uses these assets and receives actual native/browser visual review.
