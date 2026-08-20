Type: feature
Area: io

Game save/load round-trips byte-identical: saving a mission/campaign state and
reloading reproduces the exact engine state (same framebuffer + RNG on resume),
verified by a create->save->reload->compare flow in tools/verify.sh, mirroring
the editor .FSG round-trip that already passes.

## Comments

Survey (baseline established): Armored Fist has NO separate savegame subsystem
-- there are no .SAV/.GAM files and no "savegame" strings in the decompile. The
persistence mechanism IS the 230-byte fixed-size player profile (.FPL, one per
pilot: D/JOE/JO/KKR/PP/TRT/GAMESWIN, all exactly 230 B). Handlers:
  - ec0d / ec5e (fist.c:34612/34653): profile-name entry + extraction -- the
    CREATE/SELECT-PLAYER UI (builds "<name>.FPL" at DAT_2000_37bc from the typed
    name buffer s_b7da / length b7d8).
  - 6a9c / 6ade: profile checksum-read + byte-identical rewrite -- ALREADY
    matrix-verified (verify.sh:148, the SELECT PLAYER / CAMPAIGNS OK path).
This NARROWS 0004 sharply: no subsystem to port, and the write path already
passes bit-identically. The sole remaining 1:1 obligation is the CAMPAIGN-
PROGRESSION MUTATION round-trip: completing/advancing a mission must update the
profile's progress field and a reload must reproduce the advanced state byte-
identically. That needs a matrix flow driving a mission to completion (Depends:
a deterministic mission-completion/score path -- relates to the mission-sim
surfaces). The 230-byte layout is the next concrete artifact to map (which bytes
hold campaign position / promotions / stats) by diffing D.FPL before/after a
completed mission under the DOSBox oracle.
