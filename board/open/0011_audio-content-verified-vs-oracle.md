Type: bug
Title: Audio content regressions fail the oracle-backed matrix
Parent: 0003

## Contract

Reject silence, wrong notes and missing mission effects even when native and WASM agree.
This owns content fixtures and regression wiring; sample-exact audio implementation is board:0003.

## Evidence

`audio-menu-content` compares against `ref/audio_menu_noteseq.txt`: the captured 239-note menu
sequence was fully contained in the working stream; a silent stream matched none. The gate was
verified in the 177-flow historical matrix. Sustained missions are now reachable; the old blocker
“capture exits at spawn” is a harness limitation, not an unavailable simulation.

## Next

1. Capture original mission music/effects at fixed scenario boundaries; retain raw trace, extraction
   command and hash alongside each checked-in reference.
2. Extend `tools/oracle/noteseq_compare.py` and the matrix to the relevant mission/device streams.
   Require complete reference containment as the menu gate does; do not revive the proposed 85% gate.
3. Prove dispatch to the intended content checker. Intentionally silent, missing and wrong-note
   fixtures must fail; native↔WASM WAV equality must not intercept the content-flow branch.
4. Include framebuffer assertions in audio flows: overlay-range bugs once left video uninitialized
   while WAV-only checks passed. Add sample-exact comparisons as 0003 makes them available.

## Accept

Menu and mission content failures are caught on both targets with hermetic original references.
A note-order pass is labeled as such, never reported as sample-level bit identity.
