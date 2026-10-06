# Asset ownership

Decode validated original map/model/texture/palette/mission/audio data into typed host structures.
Original files remain ignored under `armoredfist/`. Tests use reproducible synthetic fixtures or
local provisioned originals. `scenario.c` now decodes complete FSG envelopes, typed metadata and
bounded unit/chunk views; see `docs/scenario-format.md` and WI 0048. KLC/resource/palette readers
and owned terrain bundles are delivered (0051/0052); see `docs/terrain-format.md` and
`docs/terrain-loading.md`. `units.c` owns typed snapshot identity/pose and the original normal-side
roster (0054, `docs/unit-definitions.md`). `model.c` owns complete directional sprite families,
atlases and part variants (0056, `docs/model-format.md`). The complete model-code catalog and
default ground-vehicle part pose are supplied by `vehicle.c` (0057,
`docs/vehicle-model-selection.md`). Owned authored sprite composition is in
`render/model_bitmap.c` (0058, `docs/model-composition.md`). Model mission-palette translation
and scene projection/drawing, other-class selection, gameplay initialization, paths/stamps/PINF
and audio decoding remain open
under milestone 0041. Source views are ephemeral; owned loaders copy before the next read.
