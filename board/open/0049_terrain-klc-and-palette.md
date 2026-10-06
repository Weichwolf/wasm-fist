Type: Work item
Title: Terrain, colormap and palette decoding for the first real scene
Parent: 0041
Depends: 0048

## Contract

Resolve a decoded scenario's asset names and decode original KLC height/colormap and palette data
into validated typed assets shared by native and WASM, enabling a real softgl terrain scene.

## Evidence

0048 supplies scenario BINF names and position metadata. There are 18 local KLC files and PAL.RES;
47 scenarios use 28 height/colormap/palette/sky combinations. KLC parsing, resource palette lookup,
terrain geometry/camera and vehicle models are not implemented yet.

## Next

Read the original KLC1 reader at extender 643c/6481/6542 and resource lookup around 5c98/6032;
verify machine code against real height/colormap and palette files. Define the bounded decoded-plane
contract and independent synthetic/original fixtures before writing C. Recover dimensions, byte
encoding, palette representation and row orientation; do not guess from filenames. Deliver decode
before rendering, then review an actual native/browser scene with a proven camera position.

## Accept

Required real height/colormap planes and palette decode completely with no missing/trailing data
or invented constants; native/WASM and independent reference checks cover the encoding and invalid
inputs. First real terrain scene uses these assets and receives actual native/browser visual review.
