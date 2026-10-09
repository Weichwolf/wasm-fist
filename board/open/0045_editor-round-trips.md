Type: Work item
Title: Editor round trips
Depends: 0042, 0043

## Contract

Editor create, edit, save, reload and simulate work with validated owned mission/map formats and JSON-parameterized procedural heightmap/colormap generation, without original files.

## Evidence

Not implemented. Historical editor flow checks are reference evidence only.

## Next

Inventory original editor operations and persisted records; implement bounded create/edit/load/save behavior with malformed-input and round-trip fixtures before simulation integration.

## Accept

Complete editor inventory passes create→save→reload→simulate on both targets without original files, preserving owned mission semantics, map-generation parameters/seeds and complete required editor functionality. Original format compatibility is optional import/research, not a runtime prerequisite.
