# Asset ownership

Decode validated original map/model/texture/palette/mission/audio data into typed host structures.
Original files remain ignored under `armoredfist/`. Tests use reproducible synthetic fixtures or
local provisioned originals. `scenario.c` now decodes complete FSG envelopes, typed metadata and
bounded unit/chunk views; see `docs/scenario-format.md` and WI 0048. Terrain, palette, models and
gameplay-state decoding remain open; continue WI 0049 under milestone 0041.
