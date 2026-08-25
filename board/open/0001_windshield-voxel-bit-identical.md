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

MISSION-AUDIO REACHABILITY FINDINGS (2026-08-26): empirically scoping mission music vs this frontier.
  - Under COOP ticking (FIST_TICK_HZ=1000 FIST_COOP_TICK=1) + the MC_MOUSE mission nav, the port DOES reach
    the mission (opens MSPRITE1.BIN, M1CON.MRL = mission-1 console) and the op-0x2c spawn (rc=0, 6/6 runs).
  - Mission MUSIC (MSN*.MS3) loads DURING GAMEPLAY, AFTER the spawn -- the FB-capture harness exits at spawn
    (FIST_MISSFB_N=1) so it never reaches the music.  Running past spawn (FIST_MISSFB_N=300) times out:
    sustained in-mission gameplay does not progress far enough headlessly -> exactly this board's gap.
  - Under fast-forward (FIST_TICK_HZ=25000, the verify.sh mission mode) + audio, the run is too slow (audio
    per-tick) to reach the mission in-budget.  So mission music needs the COOP in-mission sim to progress
    -- which is this board's deterministic per-frame loop.
  - RARE INTERMITTENT SEGFAULT (~1/8) in the coop+mission+audio path (once at op-0x1c display-list, clean
    the other 7 runs incl. under gdb) -- a non-determinism symptom (uninit state / ordering) in the same
    in-mission path this board addresses.  When the deterministic per-frame loop lands, re-test for the crash.
  NET: mission audio bit-identity is gated on THIS board (the in-mission per-frame determinism), confirming
  the "doubled payoff" above -- the deterministic frame loop unblocks the windshield AND mission audio.

CORRECTION (2026-08-26) -- MISSION GAMEPLAY + MUSIC ARE REACHABLE HEADLESSLY (earlier "stalls" was harness):
  - The in-mission windshield render ADVANCES frame-to-frame: op-0x2c posts #1/#5/#20 have distinct md5s
    (motion), rc=0.  Gameplay progresses under FIST_TICK_HZ=25000; the N=300 "timeout" was just slowness.
  - MISSION MUSIC LOADS + PLAYS: a coop run (FIST_TICK_HZ=1000 FIST_COOP_TICK=1 FIST_DUMPTICK=15000 + the
    MC_MOUSE mission nav, NO FIST_MISSFB2C) navigates menu->LOADING.MS3->MISSION and opens MSN2.MS3, then
    emits 2241 note-ons / 4471 key-ons (11762 OPL writes) continuously across the whole sequence, ending
    in-mission.  So the re-entrancy fix (d7bd0aa) plays mission music too, as predicted (same 0a28->0c39
    driver proven note-for-note on the menu).
  - The earlier "sustained sim doesn't progress" was a HARNESS artifact: FIST_MISSFB2C (op-0x2c video
    capture) + audio + nav stalls at OPL-init (169 writes); dropping the FB machinery (audio-only reglog)
    runs clean.  A separate rare ~1/8 segfault remains in the coop+mission path (uninit/ordering) -- retest.
  NET: mission audio is NOT board:0001-blocked for PLAYBACK; the open piece is native==wasm determinism of
  the mission path + an oracle mission-music reference.  WAV-sample tempo/phase identity vs the original
  still rides this board's per-frame cadence.

MISSION AUDIO native==wasm STATUS (2026-08-26, measured): the coop mission run (menu->loading->MSN2.MS3)
PLAYS on both targets but is NOT yet native==wasm deterministic:
  - native 2241 note-ons / 11762 writes (WAV 11.997M samples) vs wasm 2764 / 14035 (WAV 14.01M samples).
  - LONGEST CONTIGUOUS COMMON note-run = 1062 (the core music matches native<->wasm), BUT the common
    PREFIX from t=0 is 0: they diverge at note-on #0 -- native=(ch1,768,7) vs wasm=(ch0,768,7) -- the
    SAME pitch on a different channel.  That is the es=0 PRE-SONG artifact (0b5d reads a garbage "song"
    from segment 0 before the real song registers; board:0003 noted it) reading channel state that differs
    native<->wasm.  And the TAIL differs: wasm runs ~17% more fist_opl_tick calls to reach [0x452]=15000
    (in-mission pump/tick cadence divergence), so it plays further into the music.
  TWO DISTINCT DETERMINISM CAUSES to close for mission audio native==wasm:
   (1) the es=0 pre-song garbage (a sound-driver init-state bug; likely also latent in the menu but masked
       there because audio-intro pins a window where both targets read the same value) -- fix: never let
       the sequencer play before a real song is registered ([ds:0x6]!=0 / [ds:0xe] gate).
   (2) the in-mission tick cadence: native and wasm reach [0x452]=15000 with different fist_opl_tick counts
       even under FIST_COOP_TICK=1 -> the per-frame/pump cadence this board must make instant-for-instant.
  So mission music CONTENT is largely faithful (1062-note shared core, same proven 0a28->0c39 driver), but
  the hard native==wasm invariant for mission audio rides (1) a pre-song gate + (2) this board's cadence.

es=0 PRE-SONG ROOT FOUND (2026-08-26, gdb playing-flag watch): the native<->wasm mission-audio divergence
begins at a delta-0 CHORD (adv=4262, fnum=768 on ch0/1/5/8) that wasm fetches whole in one advance while
native splits ch0 to the next advance -- because the two read a DIFFERENT delta for the 4th chord note,
i.e. they read DIVERGENT SONG MEMORY.  Root: `be0e(0)` (track 0) -> 01ec -> 0af4 registers a song with
[ds:0x6]=0 (es=0) and [ds:0xe]=0xffff (PLAYING) during the intro, BEFORE the real menu song be0e(4)
(es=0x4c61).  Track 0 is an UNLOADED song (segment 0), so 0b5d streams es:cursor = 0:0x9ff4+ = g_mem low
region, which is runtime-divergent native<->wasm -> the chord's 4th delta differs -> the goto-fetch loop
terminates one note earlier on native.  (Chain: be0e(0) <- e584 <- e446 <- cae6.)
FAITHFUL FIX DIRECTION (needs asm verification vs 0xaf4/0x1ec/0xbe0e): the driver must not PLAY a song
whose data segment is 0 (unloaded) -- treat es=0 as "no song / silence" (do not set [ds:0xe]=0xffff), which
is what the original must do since segment 0 holds no MS3.  This is a ROOT fix (never play an unloaded
song), NOT a skip-the-symptom guard.  Confirm against the DOSBox oracle that the intro (be0e(0) period)
produces no MS3 music, then patch the be0e/0af4 chain.  Closing this removes cause (1); cause (2) (in-
mission tick cadence) remains this board's per-frame determinism.

es=0 ROOT NAILED (2026-08-26): NOT an empty track -- track 0's name IS 'INTRO.MS3' (present on disk, 1523
B), but it NEVER OPENS (openlog: only MAINMENU.MS3).  gdb at the two be0e calls (correct addr g_mem+0x1c378):
  be0e(0) [intro]: c378(loader vector)=0x00000000 (NOT installed) -> the (*c378)() screen-resource load is
                   a null-vector no-op -> INTRO.MS3 never loads -> descriptor seg [0x9f1c]=0 -> es=0.
  be0e(4) [menu ]: c378=0x0f692e7d (seg 0xf69 installed) -> loads MAINMENU.MS3 -> es=0x4c61 (plays right).
So the intro-music load fails because the c378 loader vector is installed only AFTER be0e(0) runs; the
sequencer then streams es=0 (0:0x9ff4+) garbage that is runtime-divergent native<->wasm -> the chord split
at adv=4262 and the whole mission-audio native!=wasm.  This is a VECTOR-INSTALL ORDERING root (the 0xf69
service cluster that owns c378 is set up later than the intro's first be0e), NOT a band-aid case.
FIX (two parts, needs asm + DOSBox-oracle): (a) determine whether the ORIGINAL installs c378 before the
intro's be0e(0) (=> the port's vector-install ordering is early-off, fix it so INTRO.MS3 loads and plays)
OR the original also has no intro MS3 (=> the driver must treat es=0/unloaded as SILENCE, faithfully); (b)
re-run the native<->wasm mission note-seq -> the 1062-note common core should extend to a full match once
the es=0 garbage window is gone, leaving only cause (2) (in-mission tick cadence) for this board.

es=0 CAUSE-(1) FULLY ROOTED + FIX SCOPED (2026-08-26):
  ORACLE CONFIRMS the original PLAYS intro music: 225 OPL note-ons in the intro window (t<26s, first at
  t=1045ms) in the DOSBox capture.  So the port MUST load+play INTRO.MS3 during the intro; the es=0 garbage
  is a genuine bug (faithful fix = load INTRO.MS3, NOT gate es=0 to silence).
  ORDERING ROOT: the intro's be0e(0) fires BEFORE the c378 loader vector exists.  c378 is installed by the
  shim fist_ensure_dlist_vecs() -> fist_apply_reloc_section(si=0x174) (native_main.c:964/1217), called from
  the engine's e714 path -- but the intro-music be0e(0) comes from the earlier e584<-e446<-cae6 path.  So
  the display-list module's method vectors (incl. c378 = the screen-resource/.MS3 loader) are relocated too
  LATE: be0e(0)'s (*c378)() load is a null-vector no-op -> INTRO.MS3 never opens -> es=0.
  FIX (next, bounded engine work; guard the 177-flow matrix): make the display-list vector install / reloc
  of the c378 loader happen BEFORE the intro's first be0e(0) (the original clearly has it available then).
  Then INTRO.MS3 loads, the intro plays the real music (native==wasm deterministic), and the mission
  note-seq native<->wasm 1062-note common core should extend to full -- leaving only cause (2) (in-mission
  tick cadence) on this board.  This is a load/vector ORDERING fix, asm/oracle-grounded, not a guard.

DEEPEST ROOT of cause-(1) (2026-08-26): the c378 loader install is late because f842 is INERT.
  Per fist_ensure_dlist_vecs()'s own comment (native_main.c:1201): the ORIGINAL installs the display-list
  method vectors (incl. c378 = the .MS3/screen-resource loader) AT LOAD TIME via the far applier f842 from
  FUN_1000_223c.  In the port f842's C body is INERT -- Ghidra DROPPED ITS STRING-OP SEGMENT BASES (the
  board:0003 far-pointer-basing class) -- so it can't apply at load time; AND applying the reloc at the
  223c point corrupts the not-yet-ready DGROUP (breaks FUN_1000_5c3a; patch 091).  So the port defers the
  whole install to e714 (menu-enter), which is AFTER the intro's first be0e(0) -> INTRO.MS3 can't load.
  So cause-(1) is a board:0003 far-ptr-basing instance (inert f842) forcing a deferred vector install whose
  timing misses the intro music.  FIX candidates (delicate; must keep the 177-flow matrix green):
   (i) install ONLY the c378 loader vector (or call fist_ensure_dlist_vecs) at the intro screen-enter,
       after DGROUP is ready but before be0e(0) -- needs verifying DGROUP is ready at that point; OR
   (ii) restore f842's dropped string-op segment bases (the real board:0003 fix) + resolve the 223c
       DGROUP-not-ready corruption so the ORIGINAL's load-time install works verbatim.
  This is careful ordering/basing work for a focused session -- NOT a rushed end-of-session change (a wrong
  install point corrupts DGROUP and breaks the validated matrix).  Cause-(1) is now rooted end-to-end.

FIX ATTEMPT 1 -- RULED OUT (2026-08-26, matrix-guarded): calling fist_ensure_dlist_vecs() at the be0e
first-use (before the c378 dispatch) DOES make INTRO.MS3 load (openlog: INTRO.MS3 x2; intro note-ons now
start at adv=51 = real music, not the adv=4262 es=0 garbage) -- BUT it BREAKS rendering: full matrix ->
FAIL about / settings / settings-sky at ref-AE~63000 (nearly the whole screen wrong).  Confirms the
fist_ensure_dlist_vecs comment: applying the reloc section (DGROUP:0x344..0x394) before DGROUP is ready
corrupts it; the intro be0e(0) is still too early (DGROUP:0x344 region not yet the installable placeholder).
Reverted (patch 100 restored; tree green).  So the WHOLE-section early install is out.  REMAINING FIX PATHS
(narrower): (a) install ONLY the c378 slot (DGROUP:0x378) if that single far-ptr's target region is ready
at intro time, leaving the rest for e714; (b) make the DGROUP:0x344 block ready earlier so the section can
install pre-intro without corruption; (c) the deep f842 fix (restore its dropped string-op seg bases so the
original's LOAD-time applier works verbatim).  Each needs the 177-flow matrix as the guard, as this attempt
demonstrated.  Negative result banked: don't re-try the blanket early install.
