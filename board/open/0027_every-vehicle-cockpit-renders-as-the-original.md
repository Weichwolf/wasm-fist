Type: feature
Title: Every original vehicle cockpit renders correctly through live switches
Parent: 0001

## Contract

Render M1, M2/M3 Bradley, T-80 and helicopter views, including instruments, radar, palette and the
switch after vehicle loss. Compare at matched simulation/presentation boundaries.

## Evidence

Patches 586/587 repaired Bradley allocation and fill/label/needle/glyph contracts. The static console
then largely matched a different-time oracle frame; that was not a complete synchronized proof.
Dynamic needles, compass, map/radar and pointer remained incomparable at those differing times.
The stale “blocked on MEMMGR swap before Bradley can render” instruction is superseded.

## Next

1. Capture the same AZER1 vehicle-loss switch and following Bradley frames in the original and port.
   Compare full presented framebuffer and DAC, including dynamic instruments and radar sweep.
2. Add T-80 and helicopter starts/switches from missions that actually select them. Trace cockpit
   asset loads and element methods; use 0014 for lost argument lanes, 0025 for damaged source data.
3. Verify both the rendering surface and presented framebuffer receive the new console. Avoid
   fixing decoder/paint logic until the loaded .MRL/.BIN data itself is confirmed intact.

## Accept

All vehicle classes and transitions have zero-diff original/native/WASM frame+palette checks at
matched state. Include instrument movement and radar changes, not only static cockpit chrome.
