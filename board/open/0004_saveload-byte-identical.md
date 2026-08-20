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

Layout map (port-only, diffed across all 7 profiles): .FPL = 230 B, 50 varying /
180 fixed.
  - 0x00..0x1f  header/format (bytes 0,1,4,6,8,0xc vary; NO ascii name in body ->
    pilot name lives only in the filename, consistent with ec0d building
    "<name>.FPL" from s_b7da; the body is pure binary state).
  - 0x24..0x53  FIVE 7-8-byte records -- the CAMPAIGN-PROGRESS entries; in the
    empty default D.FPL these are all 0xFF ("nothing completed"). This is where
    mission-completion writes. <-- the mutation target for the round-trip.
  - 0x54..0xdb  fixed 0xFF (unused/reserve in an empty profile).
  - 0xdc / 0xde / 0xe4-0xe5  tail flags/stats (D.FPL: 0xe4=0x40).
Concrete next datum: diff one profile before/after a completed mission under the
DOSBox oracle -> the delta lands in 0x24..0x53, confirming the field semantics
and giving the exact bytes the progression-mutation round-trip must reproduce.
