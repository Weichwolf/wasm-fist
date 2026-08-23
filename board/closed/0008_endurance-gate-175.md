Type: milestone
Title: The WASM build passes the 10x-consecutive endurance gate on the 175-flow matrix
Date: 2026-08-23
Gate: tools/wasm_gate.sh (DD2-standard: 10 consecutive clean tools/verify.sh wasm runs; one failure resets to 0)

This engine can pass the DD2 endurance gate on its current test matrix: ten consecutive full-matrix
WASM runs, each 175 flows PASS / 0 FAIL, zero failures across all ten -- the count never reset.

RUN LOG (tools/wasm_gate.sh, node 22.16.0, /tmp/wasm_gate.log):
  run #1..#10 CLEAN, 175 pass / 0 fail each, 01:05 -> 02:47 CEST (~1h45, ~11 min/run), consecutive 10/10.
  DONE -- 10 consecutive clean wasm runs achieved.  grep -c FAILED = 0.

WHAT THIS PROVES:
- The WASM build is deterministic and stable: 10 independent full-matrix passes, no flakiness, no
  timing-dependent divergence across ~2 hours of grinding.  The 175 flows include every menu/screen/
  settings-toggle, the editor create->save->reload->simulate + add/rem tank + add-enemy + edit-sim
  round-trips, 46/47 mission cockpits (op-0x24 and op-0x2c), 5 mission terrains, and OPL audio init --
  all native<->wasm byte-identical (verify.sh's hard invariant), re-verified 10x.
- The gate floor was corrected 159->175 this round so a silently-shrunk matrix can no longer count as
  clean (the failed==0 guard is the real gate; the floor catches shrinkage).

WHAT THIS DOES *NOT* CLAIM (honesty -- the DD2 "done" bar is NOT met):
- The matrix is NOT yet the full exhaustive matrix the goal requires.  Terrain covers 5/47 missions
  (board:0007 -- the object-reference representation cascade blocks the other 42), audio is not yet
  full-duration native==wasm (board:0003 -- state-transition-onset determinism), INDIA3 cockpit is
  excluded (board:0006 -- spawn OOM), and terrain oracle-fidelity is tooling-gated (board:0002).
- Therefore this is the endurance gate passing on the CURRENT matrix, a real and necessary milestone,
  NOT "the port is done".  Per CLAUDE.md, re-passing this gate AS THE MATRIX GROWS to cover every
  surface is what will eventually turn "the matrix passes" into "the port is done".  This item records
  the first clean 10x; it must be re-run after each material matrix expansion.

RE-GATED on the EXTENDED 176-flow matrix (2026-08-23, after patch 411 landed + the audio-intro flow):
10 consecutive clean tools/wasm_gate.sh runs, 176 pass / 0 fail each, 0 resets (05:51 -> 07:57 CEST).
The matrix grew 175 -> 176: audio is now native==wasm bit-identical ACROSS the intro->menu transition
(audio-intro, [0x452]=300), not just the [0x452]=120 init window.  wasm_gate floor raised 175 -> 176.
Still NOT the full-exhaustive matrix (terrain 5/47 board:0007, INDIA3 board:0006, audio not yet across
mission transitions board:0003, oracle-fidelity board:0002) -- the endurance gate re-passing on each
expansion is the mechanism, not the completion.
