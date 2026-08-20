Type: feature
Area: render
Tags: voxel oracle

The windshield voxel-space terrain view (rows 0-95 of the 320x200 mission
framebuffer) renders bit-identical to the original for every battle: a
first-person perspective — sky above, a horizon, receding terrain below —
matching the original's DOSBox framebuffer pixel-for-pixel, on native and wasm.

Reference method: tools/oracle/capture_battle_burst.sh grabs the ORIGINAL spawn
under stock DOSBox; a frame whose DASHBOARD matches the port at AE=0 gives a
provenance-verified windshield reference (e.g.
tools/oracle/samples/oracle_azer1_windshield_dashAE0.png for AZER1).

Done = the windshield region of a matrix flow reaches 0-diff vs that reference
for AZER1, then generalised across battles and added to tools/verify.sh.
