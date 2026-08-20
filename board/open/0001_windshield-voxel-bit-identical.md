Type: feature
Area: render
Tags: voxel oracle

The windshield voxel-space terrain view (rows 0-95 of the 320x200 mission
framebuffer) renders bit-identical to the original for every battle: a
first-person perspective — sky above, a horizon, receding terrain below —
matching the original's DOSBox framebuffer pixel-for-pixel, on native and wasm.

Reference method: tools/oracle/capture_battle_burst.sh grabs the ORIGINAL spawn
under stock DOSBox; a frame whose DASHBOARD matches the port at AE=0 gives a
provenance-verified windshield reference (e.g.
tools/oracle/samples/oracle_azer1_windshield_dashAE0.png for AZER1).

Done = the windshield region of a matrix flow reaches 0-diff vs that reference
for AZER1, then generalised across battles and added to tools/verify.sh.

*** CROSS-SURFACE META-FINDING (surveyed 0002/0003/0004/0005 this session) ***
All remaining, unshipped work on EVERY surface bottoms out in the same two dependencies -- the
easy parts (menus/dialogs, profile-WRITE, editor .FSG round-trip, note-on) are already done and
matrix-verified; what is left is uniformly the hard core:
  - 0002 windshield: gated on the 3a24 base-ray-curve PRODUCER (needs an oracle watchpoint --
    dosbox rebuild or QEMU gdb) + then the projection/6980/Layer-2 chain (deep).
  - 0003 audio: gated on the SONG-LEVEL program/patch-change in the sequencer parse (0f99 never
    called during playback) -- the engine's indirect-dispatch/sequencer core.
  - 0004 save-load: the WRITE path is verified; the only remaining 1:1 obligation (campaign-
    progression mutation round-trip) is gated on a deterministic MISSION-COMPLETION path (deep
    mission-sim) + the DOSBox oracle for the before/after .FPL diff.
  - 0005 controls: menu mouse/keyboard verified; the remainder (in-mission drive/turret/weapons
    mapping, joystick, serial) is gated on the in-MISSION sim running (deep).
CONCLUSION: there is NO autonomous-loop-tractable landable win left -- the remaining surfaces all
require focused RE work on the shared ENGINE CORE (the mission-sim + the hand-written indirect-
dispatch/sequencer machine) and/or oracle tooling. The single HIGHEST-LEVERAGE direction is that
shared core, not per-surface probing: completing the mission-sim + indirect dispatch unblocks the
windshield render, the audio program-change, the save-load mutation, AND the in-mission controls
at once. This session's landed/verified win remains patch 407; the durable next step is a FOCUSED
session on (a) the oracle watchpoint for 3a24, or (b) the sequencer song-level program-change, or
(c) the mission-completion sim path -- each a dedicated effort, all feeding the same core.
