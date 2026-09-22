Type: feature
Title: Every battle renders the complete windshield byte-identically to the original
Depends: 0002

## Contract

Compare the complete presented windshield, including terrain, sky, objects and HUD, against the
original under matched input, RNG and time. Cover every battle on native and WASM.

## Evidence

- Historical matrix: five terrain flows match native↔WASM. This does not prove oracle fidelity.
- The isolated 9200 writer matches its captured inputs; upstream geometry remains board:0002.
- Live browser rendering exists. The historical HUD crawl was fixed by restoring the rect-fill
  register arguments (patch 410); do not restart the disproven a2e9 performance investigation.
- A spawn capture inside op 0x24 precedes HUD painting and is not the final presented frame.

## Next

1. Finish board:0002's isolated geometry/projection proof on AZER1.
2. Capture a matched, fully presented frame on the original and both targets. Compare framebuffer
   indices and DAC separately; locate the first differing producer before patching.
3. Extend to every battle and detail/night mode. Use board:0027 for vehicle-specific consoles,
   board:0026 for browser pacing, board:0014/0015 for dispatch defects exposed by those flows.
4. Add live movement, firing and cockpit-switch captures. A static spawn flow cannot cover them.

## Accept

Every required battle/view has a reproducible oracle comparison with zero differing pixels and
palette entries on both targets. Live browser play reaches and maintains the same rendered state.
Do not use a dashboard-only match as proof that camera, RNG and simulation time match.
