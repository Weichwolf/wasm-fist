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

--- PROGRESS (2026-08-25) ---
LANDED: the MENU audio-content gate is wired into the matrix (commit follows).
  - ref/audio_menu_noteseq.txt -- 239-note DOSBox-oracle MAINMENU.MS3 reference.
  - tools/oracle/noteseq_compare.py --gate -- contiguous-containment check (fixed build 239/239 PASS,
    silent build 0/239 FAIL).
  - tools/verify.sh `audio-menu-content` flow -- gates the port's native AND wasm menu OPL note stream
    against the reference.  Verified PASS both targets (native 4412 / wasm 4397 note-ons contain all 239).
  A regression to silence now FAILS the matrix -- the gap the silence bug hid behind is closed FOR THE MENU.
STILL OPEN (why this item stays open):
  - MISSION audio content (MSN*.MS3) is NOT yet gated: sustained in-mission gameplay -- where mission music
    loads/plays -- is not reachable headlessly today (the FB-capture harness exits at spawn, before MSN*.MS3
    loads).  Gating mission audio depends on the in-mission cooperative sim (board:0001).  The re-entrancy
    fix (d7bd0aa) is structurally general (all music shares the 0a28->0c39 sequencer), so mission music will
    play faithfully once reachable -- but that must be VERIFIED, not assumed, when the sim path lands.
  - WAV-SAMPLE bit-identity vs the original (tempo/phase, not just note ORDER) remains the ultimate target;
    the note-seq gate is a necessary content invariant, not the full deliverable -- it depends on the
    frame-timing determinism (board:0001) to become a sample-exact WAV compare.

CORRECTION (2026-08-25): the menu content gate initially had two wiring bugs that made it PASS without
actually gating -- (1) flow-dispatch: 'audio-menu-content' matched the generic 'audio-' native==wasm WAV
branch first (fixed: excluded it); (2) set -u: run_audio_reglog used $t in the same `local` that defines
it, so under verify.sh's set -u the function errored and returned an empty reglog path -> the gate saw ""
and FAILED (fixed: split the local onto two lines).  Both caught by NOT trusting a green result (the wrong
detail string, then the in-matrix FAIL).  NOW GENUINELY ENFORCED: full matrix 177/177 with the real content
gate showing detail "menu OPL note stream contains oracle MAINMENU.MS3 note-seq" and PASSing on native+wasm.
A regression to silence now truly FAILS the matrix (self-checked: silent build 0/239).

DoD GATE PASSED (2026-08-26, 00:48): 10/10 consecutive clean wasm runs on the 177-flow matrix -- the real
audio-menu-content gate is now part of every DoD run (177 pass / 0 fail each).  Menu audio content is a
genuinely-enforced, DoD-validated invariant: the port plays MAINMENU.MS3 note-for-note vs the oracle, on
native AND wasm, and a regression to silence fails the gate.

MISSION AUDIO -- empirically confirmed board:0001-gated (2026-08-26): the port reaches the mission spawn
under coop ticking (rc=0, 6/6) but MSN*.MS3 music loads post-spawn during gameplay, which the spawn-capture
harness does not reach; running past spawn stalls (sustained in-mission sim = board:0001).  A rare
intermittent segfault (~1/8) in the coop+mission+audio path is logged on board:0001 as an in-mission
non-determinism symptom.  So the mission audio-content gate (analogous to audio-menu-content) waits on
board:0001 making in-mission gameplay deterministically reachable headlessly.
