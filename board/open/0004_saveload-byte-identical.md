Type: feature
Area: io

Game save/load round-trips byte-identical: saving a mission/campaign state and
reloading reproduces the exact engine state (same framebuffer + RNG on resume),
verified by a create->save->reload->compare flow in tools/verify.sh, mirroring
the editor .FSG round-trip that already passes.
