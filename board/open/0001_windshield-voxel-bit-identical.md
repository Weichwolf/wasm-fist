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

CROSS-REF from board:0003 (2026-08-25) -- AUDIO TIMING IS UNIFIED WITH THIS FRAME-DETERMINISM FRONTIER:
The board:0003 audio investigation concluded (measured, asm-verified) that the menu music tick (SOUNDDVR
0a28) is called ONCE PER ENGINE FRAME at the VGA vsync-limited rate -- measured exactly at 70.086 Hz
(mode-13h vsync, intervals 14.268 ms) with ~17% frame-skips (mean 60 Hz), driven by the engine's per-frame
main loop (via 0x1d2->[cs:0x5c2]=0xa28), NOT by any ISR or PIT subdivision (the retrace ISR 0x0b1f is
video-only; asm-checked).  THEREFORE sample-bit-identical AUDIO and the bit-identical live WINDSHIELD are
the SAME underlying requirement: the port's per-frame present/loop must iterate INSTANT-FOR-INSTANT like
the original (70 Hz vsync + the exact frame-skip pattern).  A deterministic frame-cadence model built here
delivers BOTH: the windshield voxel render AND the OPL audio stream fire at the original's exact frame
instants.  The audio rider is a small bounded shim change once this lands: drive fist_snd_seq_advance from
the port's per-frame VGA-present hook (retire the fist_opl.c sample-clock reconstruction).  So this
frontier's payoff is doubled -- it closes board:0003's audio bit-identity too.  (board:0003 landed
MUSIC_DIV=120.536 = the measured 60 Hz-mean as the best uniform approximation until this lands.)
