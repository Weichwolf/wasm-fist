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

FIX ATTEMPT 2 + KEY ENTANGLEMENT FINDING (2026-08-26, matrix-guarded):
  Added a SURGICAL single-slot installer fist_apply_reloc_slot(0x174, 0x378) called from be0e first-use
  (installs ONLY the c378 loader vector, not the whole section).  RESULT: it FIXED the render corruption
  (about/settings/settings-sky PASS again) AND loaded INTRO.MS3 (intro music from adv=51) -- so the
  single-slot approach is the right shape for cause (1).  BUT the full matrix -> 170/177 with 7 NEW fails:
    - audio-intro nat!=wasm(2354949), audio-opl-init nat!=wasm(44293)  [audio-menu-content still PASSES]
    - terrain-azer1/saudi1/cyprus1/india1/syria1 nat!=wasm(188)
  CRUCIAL: audio-intro was native==wasm ONLY BECAUSE THE INTRO WAS SILENT -- the es=0 garbage started at
  ~[0x452]=4972 (adv 4262), AFTER audio-intro's [0x452]=4000 window, so its window was clean.  Loading real
  INTRO.MS3 makes music play from ~[0x452]=60 -> INSIDE the audio-intro window -> and that playback is
  native!=wasm.  So CAUSE (1) AND CAUSE (2) ARE ENTANGLED: fixing the intro-music LOAD (cause 1) exposes the
  audio TICK-CADENCE non-determinism (cause 2) within the intro, breaking native==wasm.  Cause (1) CANNOT
  land alone.
  REORDERING: cause (2) -- the deterministic per-frame audio cadence (this board) -- is the PRIMARY blocker
  and must be fixed FIRST; then the single-slot INTRO.MS3 load (cause 1, attempt-2 shape) can land on top
  and the intro/mission audio becomes native==wasm end-to-end.  The whole intro+mission audio native==wasm
  rides on this board's per-frame determinism -- confirming the "doubled payoff" (windshield + all audio).
  Reverted attempt 2 (patch 100 + the helper); tree green (make check clean).  Negative results banked;
  the primary path is now unambiguous: land cause (2) cadence determinism, then cause (1).

AUDIO DETERMINISM DECOMPOSED INTO 3 LAYERS (2026-08-26); cause-2 LANDED:
  cause-2 (re-entrant cadence) -- FIXED + COMMITTED (g_in_isr atomic-tick guard, native_main.c): keeping
    g_in_isr=1 across the snd/opl ticks stops out()->fist_timer_pump from re-entering and generating extra
    fist_opl_tick samples.  Measured: [0x452]=8000 menu-music WAV native/wasm sample gap 5026->711; with
    INTRO.MS3 loaded the intro WAV LENGTHS became native==wasm identical (5696650 both).  Matrix 177/177.
    (The SIGALRM-under-coop async tick was NOT the residual -- disabling it had zero effect; reverted.)
  cause-1 (es=0 intro garbage) -- fix shape = single-slot fist_apply_reloc_slot(0x174,0x378) at be0e
    first-use (loads INTRO.MS3, render-safe).  Blocked by cause-3.
  cause-3 (NOTE-CONTENT divergence) -- NEW, the deep one: with cadence fixed, native and wasm still emit
    DIFFERENT NOTES for the same playback.  First divergence is the es=0 chord (adv=4262: native splits it
    ch1/5/8 @4262 + ch0 @4263; wasm plays ch0/1/5/8 all @4262) -- the delta-0 goto-fetch loop terminates one
    note earlier on native.  The es=0 song reads g_mem[0:0xa024+] = ENGINE IMAGE (loaded identically), so the
    divergence is in SEQUENCER STATE (cursor [ds:0xc] / voice chain), not the song bytes.  With INTRO.MS3
    loaded the intro WAV content still differed 4M bytes -> real-song playback is also content-nondeterministic.
    ROOT (to find): what sequencer/driver state differs native<->wasm at the first chord despite identical
    song memory + fixed cadence -- needs cross-target (native AND wasm/node) gdb of [ds:0xc]/voice-chain at
    adv=4262.  This is THE remaining audio determinism blocker (gates cause-1 landing + all mission audio).

CAUSE-3 DIAGNOSIS (2026-08-26), via FIST_MEMDUMP g_mem diff native vs wasm at [0x452]=5000:
  Only 143 bytes of the whole 16MB g_mem differ (both targets internally deterministic).  The differing
  bytes are host-address-shaped (native ..2d 08 = 0x082d.. ELF, wasm ..11 01 = 0x0111.. wasm linear mem),
  heaviest in the extender region 0x100000+ (88B), plus the sound-driver DS 0x3ce90+ and DGROUP.  The 177
  VIDEO flows pass because the render path never reads these; the AUDIO path reads a few driver-DS words:
    ds:0x1c1 = native 0xfab vs wasm 0xf80  (the 0cfb device-slot dispatch vector -> different FM loader)
    ds:0x14  = native 4    vs wasm 5       (the sequencer delay counter -> chord splits differently)
    ds:0x12  = native 0x1ec vs wasm 0      (the play-function vector)
  These are set by the SOUNDDVR device-SELECT (patch 353, 0872: copy device-3's 7 method vectors from the
  per-device table driver_ds:0x17d.. into the live slots).  TESTED the 64KiB-g_mem-alignment fix (patch
  384's "host-ptr-truncated-to-near-offset" class): it aligned native g_mem + cut g_mem diffs 143->108, but
  the AUDIO reglog was UNCHANGED (native/wasm md5 identical to before) -> cause-3 is NOT that truncation
  class; it is a SPECIFIC device-select divergence where the copied method-vector / slot values come out
  different native<->wasm (a value derived from a host pointer or a divergent copy in the 0872 select /
  patch 353).  Reverted the alignment (no effect on target).  NEXT: trace the writer of driver_ds:0x1c1 /
  the 0872 method-vector copy source, find the native-vs-wasm-divergent input, make it faithful (16-bit).
  cause-2 (re-entrant cadence) is LANDED; cause-3 (this device-select host-value divergence) is the last
  audio determinism blocker + gates cause-1 (INTRO.MS3 load) and all mission audio.

CAUSE-3 TRACED DEEPER (2026-08-26): the divergent audio driver-DS words are, in the method-vector table
ds:0x17b..0x1d1, EXACTLY ONE of 44 entries: ds:0x1c1 = native 0fab vs wasm 0f80 (all others identical).
But the more fundamental divergence is ds:0x12 = native 0x01ec vs wasm 0x0000 (0x1ec = the 01ec play-fn
offset) and ds:0x14 = 4 vs 5 (delay).  The device-SELECT copy (patch 353: dev=byte[D+0x12]; if dev in
1..5 copy row dev*2) is SKIPPED on both (native dev=0xec !<6, wasm dev=0) -> so ds:0x1c1 is written
divergently ELSEWHERE in the driver init, and ds:0x12 itself is a driver-init value that comes out
0x01ec native / 0 wasm.  The method table is loaded from SOUNDDVR.DVR (identical file) so these single
entries are RUNTIME-OVERWRITTEN with a native-vs-wasm-divergent value -- a driver-init host-value leak
(board:0003 class), NOT the g_mem-base truncation (alignment had no audio effect).  NEXT: watchpoint the
writer of driver_ds:0x12 (and 0x1c1) to find the divergent input (likely a host-pointer low-16 or a
call-arg the __allregs vector site drops, cf. patch 384) and thread the faithful 16-bit value.

AUDIO DETERMINISM -- 2 OF 4 LAYERS LANDED (2026-08-26):
  cause-2 (re-entrant cadence) -- LANDED (g_in_isr atomic tick); 10x DoD gate 10/10.
  cause-3 (dropped AX command word) -- LANDED (patch 385: thread be58->c530 AX; asm 0xbe4f-0xbe66).
    Result: note COUNT + driver-DS state converged native==wasm; matrix 177/177.
  cause-1 (es=0 intro garbage) -- fix ready (single-slot INTRO.MS3 load); with cause-2+3 fixed the note
    count AND the WAV LENGTH are now native==wasm IDENTICAL (17760300 both) when INTRO.MS3 loads -- but
    blocked by cause-4.
  cause-4 (instrument-APPLY level divergence) -- NEW residual + the last blocker: with all above, the OPL
    LEVEL registers (0x40-0x55) still differ -- native writes the real instrument levels, wasm writes the
    base (0x80/0x00).  0fab's level write is (record_byte ^ 0x3f); native record_byte=0x22 / wasm 0x3f, so
    wasm reads a level of 0.  The g_mem diff is down to 128 bytes: driver 0x39d66 = native 0x91fd / wasm 0
    (a level/timbre source populated on native, 0 on wasm) + the extender region 0x100000+ (88 host-ptr
    bytes).  So cause-4 is 0fab/0f99 reading an instrument-level source that is host-populated on native and
    empty on wasm -- another board:0003 host-value site.  This breaks audio-intro if INTRO.MS3 loads (the
    level divergence is inside the [0x452]=4000 window), so cause-1's INTRO.MS3 load is reverted until
    cause-4 lands.  NEXT: watchpoint driver_ds region 0x39d66 (and 0fab's puVar6/level source) to find the
    host-populated-only value.  Two of four audio-determinism layers are now LANDED + matrix-clean.

CORRECTION (2026-08-26): 0x39d66 is written by the MGAVIDEO driver (m_mga_04f1 -> 1774 -> 184b -> 0a31,
value 0x91fd from a video param), NOT the sound path -- it is video memory near the driver, dormant for
the passing video flows.  So cause-4's instrument-LEVEL source is elsewhere (the extender 0x100000+ or the
engine 0x10000 g_mem diffs).  cause-4 (wasm OPL levels read as 0, native real) remains the last audio
blocker; the level write is 0fab's `(record_byte ^ 0x3f)` so the divergent input is the instrument record
byte the FM-apply reads -- next: watchpoint the exact OPL reg-0x54 write's source under the INTRO.MS3-loaded
build to find the host-populated-only level table.  cause-2 + cause-3 (patch 385) are LANDED + committed +
matrix-clean; the audio note-count and WAV-length are now native==wasm identical -- only instrument levels
diverge.  Two of four audio-determinism layers landed this session.

CAUSE-4 REFINED (2026-08-26): the divergent OPL level is NOT a static g_mem byte -- searching the
INTRO.MS3-loaded memdumps for the expected source byte (native 0x22 / wasm 0x3f, from level = byte^0x3f)
found ZERO hits.  So the level is COMPUTED from host-pointer-divergent state, not a raw record byte.  The
remaining g_mem diffs are all host-address-shaped: engine 0x10000+ high-words native `09 08`/`0a 08`
(=0x0809../0x080a.. ELF pointer high words) vs wasm values, and the extender 0x100000+ (88 host-ptr bytes).
So cause-4 is a stored HOST POINTER (board:0003 class) whose value feeds the OPL instrument-level path on
one target but reads 0 on the other.  Fixing it needs the cross-target (native+wasm/node) gdb trace of the
exact reg-0x40..0x55 level write's data source -- deferred until the 10x gate (validating cause-2 + patch
385) frees the build.  Two of four audio-determinism layers landed + gate-validating; cause-4 is a
board:0003 host-pointer site feeding the level path (not a single fixable byte, so no quick thread).

CAUSE-4 MECHANISM (2026-08-26, gdb+static): the divergent OPL level is written by 0f99 (instrument loader),
`level = puVar6[6] ^ 0x3f` where puVar6 = D + param_1*0x10 + 0x1dd (param_1 = program number).  For a VALID
program (small param_1) puVar6 lands in the driver-DS instrument-record region, which is native==wasm
IDENTICAL -> level matches.  The divergence therefore comes from a LARGE/garbage param_1 that indexes
puVar6 PAST the records into the host-pointer-divergent region (extender 0x100000+/engine 0x10000+).  That
garbage program number is the es=0 pre-song (cause-1): its unregistered "song" yields garbage program-change
events -> wild instrument records -> native!=wasm levels.  So cause-4 UNIFIES WITH cause-1: fixing the es=0
garbage (load INTRO.MS3, real programs 0..N) keeps puVar6 in the identical record region and the levels
converge.  (The residual seen with INTRO.MS3 loaded needs a re-check with all of cause-2+3 + the load in
place; the earlier test predated confirming cause-3's ds:0x1c1 fix fully propagates.)  So the audio native==
wasm chain is: cause-2 (LANDED) + cause-3/patch385 (LANDED) + cause-1 (INTRO.MS3 load, ready) -- with all
three, the garbage program indices are gone and cause-4 should resolve.  NEXT: re-apply cause-1 on the
cause-2+3 base and re-measure the level convergence (the earlier INTRO.MS3 test was on the pre-patch-385
base).

CAUSE-4 HYPOTHESIS DISPROVEN (2026-08-26) -- SELF-CORRECTION: re-applied cause-1 (INTRO.MS3 load, valid
programs) ON TOP of cause-2 + cause-3/patch385 and re-measured -- audio-intro [0x452]=4000 STILL differs
(2.17M bytes).  So cause-4 (instrument LEVEL divergence) is NOT the es=0 garbage-program-index issue: it
persists with VALID INTRO.MS3 programs.  0f99 reads level = puVar6[6] from D+program*0x10+0x1dd (the driver-
DS record region, which the memdump showed IDENTICAL) yet the level still diverges -> the real cause-4 is
either (a) the INTRO.MS3 instrument records are loaded to a location that IS native!=wasm (not the region I
sampled), or (b) 0f99's reg-base uVar3/uVar5 = D[uVar4+0xbef/0xbf8] differ (uVar4 = D[(param_2>>8)+0xc01]),
or (c) a wasm codegen/FP path.  DECISIVE NEXT: native+wasm gdb of ONE 0f99 call (same param_1/param_2) --
dump puVar6[0..10], uVar3, uVar5, bVar2 on BOTH targets and diff, to see which input actually differs.
cause-1 reverted (does not reach native==wasm alone).  cause-2 + cause-3 remain LANDED + 10x-gate-validated;
cause-4 is the genuine last menu/intro audio blocker and its input has not yet been pinned.

CAUSE-4 FULLY PINNED (2026-08-26, gdb): the divergent OPL level (reg 0x44 native 0x8f / wasm 0x80) is
written by 10e3 (note key-on velocity path) <- 0aa7, NOT 0f99.  10e3's level = (D[uVar3+0xbdd] | ((vel>>2)
+0x20)) ^ 0x3f; back-solving -> D[channel+0xbdd] = native 0x80 / wasm 0x8f AT THE WRITE.  D[+0xbdd] is set
by 0f99 as `puVar6[1]` = instrument-record byte 1, puVar6 = D + PROGRAM*0x10 + 0x1dd.  The memdump at
[0x452]=5000 shows D[+0xbdd] + the records IDENTICAL -> the divergence is TRANSIENT: at the key-on (adv
~4262) D[+0xbdd] held a wild value that later converged.  MECHANISM: the es=0 pre-song (cause-1) feeds 0f99
GARBAGE program numbers -> puVar6 indexes PAST the identical records into host-pointer-divergent memory
(extender 0x100000+) -> the transient per-channel base level D[+0xbdd] diverges -> the key-on level diverges
native<->wasm.  So cause-4 == cause-1 (the es=0 garbage) via the TRANSIENT base-level, resolving the earlier
apparent contradiction (the persistent records are identical; the transient wild read is the divergence).
OPEN: the earlier INTRO.MS3-load test still diverged -> either the es=0 song still plays a window before
INTRO.MS3 registers (a key-on in that window latches the wild base level), or the c378 single-slot load
lands INTRO.MS3 a few ticks later than the first es=0 key-on.  NEXT (decisive): with INTRO.MS3 loaded, gdb
whether 0f99 EVER sees a garbage (>N) program; if yes, tighten cause-1 so INTRO.MS3 registers BEFORE the
first key-on (or gate 0f99 to skip out-of-range programs faithfully per asm 0x99 `if (8<bVar2) return`,
which already guards voice but not program).  cause-2 + cause-3 LANDED + 10x-gate-validated; cause-4 is the
es=0-garbage transient, now understood end-to-end.

CAUSE-4 -- "WILD RECORD" HYPOTHESIS ALSO DISPROVEN (2026-08-26, asm 0xf99): the asm computes puVar6 =
program*0x10 + 0x1dd with program = AL (a BYTE, 0..255), so puVar6 in 0x1dd..0x11cd -- ALWAYS within the
driver-DS region that the memdump showed native==wasm IDENTICAL.  It never indexes into the divergent
extender.  So cause-4 is NOT a wild-record read.  It is a genuine TRANSIENT divergence of D[channel+0xbdd]
(the per-channel base level 10e3 reads at key-on): at the write it is native 0x80 / wasm 0x8f, but the
records, the program, the voice->channel map D[voice+0xc01], and the reg-base D[+0xbf8] all read IDENTICAL
in the memdump, and g_mem is BSS-zeroed identically.  So the source of the transient is not yet found by
static/memdump analysis.  DECISIVE NEXT (bounded): a temporary env-gated dump of D[ch+0xbdd] + puVar6[1] +
program INSIDE 0f99 and 10e3 (a diagnostic patch, rebuild BOTH targets, run, diff the per-call trace) to
catch the exact call where D[+0xbdd] first differs native<->wasm -- static analysis has hit its limit here.
STATE: cause-2 + cause-3 LANDED + 10x-gate-validated (menu/editor matrix); cause-4 pinned to one transient
driver byte (D[ch+0xbdd]) but its divergence source needs live cross-target instrumentation, not memdump.
Two of four menu/intro audio-determinism layers fixed; the goal (mission audio, WAV-vs-original, DD2
missions) remains a large multi-session effort beyond this.
