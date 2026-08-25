Type: bug
Title: The test matrix verifies engine audio CONTENT against the original, not just native==wasm
Parent: 0003

The DoD audio flows (`audio-opl-init`, `audio-intro` in tools/verify.sh) assert only that the NATIVE and
WASM builds emit a byte-identical OPL/WAV stream. That invariant is trivially satisfied when BOTH targets
are silent: the vsync-drive gate passed 10/10 while the port emitted ~4 note-ons in 190 s of menu music
(the original emits ~99 in the menu window alone). native==wasm byte-identity is necessary but NOT
sufficient for the goal ("the same audio stream as the original") -- a mute port sails through.

This item is closed when the matrix carries a flow that compares the port's engine-generated OPL note
stream to the ORIGINAL's (DOSBox oracle), so a regression to silence (or wrong notes) FAILS the gate:

- Oracle reference: `tools/oracle/trace_opl.sh` (FISTOPLLOG) captures the original's menu OPL writes;
  reduce to a note-on sequence (channel, fnum, block) over one loop.
- Port capture: `FIST_OPL_REGLOG` over the same menu window -> same reduction.
- Compare with `tools/oracle/noteseq_compare.py` (longest contiguous note-run, phase/timing-invariant):
  gate threshold e.g. >=85% of the shorter sequence == FAITHFUL. Bank a checked-in oracle note-seq
  reference (like ref/audio_menu_oracle_clean.wav) so the flow is hermetic.
- Extend beyond the menu once mission music plays (MSN*.MS3), per the DD2 completeness standard.

Rationale lives in board:0003: the note-sequence harness already exists and is self-validated; wiring it
into verify.sh as a first-class flow is what turns "native==wasm" into "bit-identical to the original"
for audio. Until then, every audio "pass" is provenance-blind to the oracle.
