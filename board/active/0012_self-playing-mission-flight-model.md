Type: feature
Area: flight-model
Tags: mission simulation extender oracle transaction-log

One AZER1 campaign mission plays itself to a resolved win/lose under the port --
the engine's AI drives both sides with empty player input, deterministic
cooperative tick, no wall-clock, native first then wasm byte-identical -- by
reconstructing the extender's 32-bit protected-mode flight/combat simulator
(unit AI, movement, weapons, damage, mission win/lose logic) that lives in the
overlay at 0x100000, NOT in FIST.DAT, so that the port's guest-memory
transaction log matches the DOSBox oracle's write-for-write.

## Foundations PROVEN (2026-08-26)

1. THE ORACLE PLAYS AZER1.  Instrumented DOSBox (third_party/dosbox-fist, cycles=max,
   headless xvfb + xclick BATTLES->OK->ACCEPT) loads AZER1, spawns the cockpit, and
   the engine's AI drives the battle with ZERO player input: HUD "GOALS REMAINING"
   went 13 -> 11 over ~120s wall (two objectives destroyed), radar tracks enemies,
   windshield renders live (14-22KB frames, continuously changing).  It resolves to
   a win/lose given time.  Screenshot: scratchpad/playout2/frame_15.png.

2. THE PORT CANNOT (baseline).  Under coop tick (FIST_COOP_TICK=1, no wall-clock),
   AZER1 spawns to d549=0x1c (cockpit) and the PIT timer advances, but the SIM is
   FROZEN: player@3ae0 and the object registry@9fbc are byte-identical across a
   32000-tick span, unit-count@530a=0.  The render shell exists; the simulator behind
   it is stubbed.  Confirmed by a two-tick g_mem diff (only 616/65536 DGROUP bytes
   move, all timer/render cursors).

3. THE TRANSACTION LOG WORKS (the spec mechanism).  The instrumented DOSBox live-
   appends every guest write in a watched window to <prefix>.watch.txt --
   "WATCH ph=<phys> val=<vv> flatip=<..> cs=<..> eip=<..> ss/sp + 20-word stack" --
   via FIST_MEMARM_BOOT=1 + FIST_WATCHPHYS=<phys>/FIST_WATCHSPAN (physical) or
   FIST_WATCHFLAT=<flat>/FIST_WATCHFLATSPAN (CR3-aware, for the engine-flat DGROUP at
   flat 0x10000000+off -- REQUIRED in-mission since the engine runs under the
   extender's PM paging cr3=0xe000, so DGROUP is NOT at a fixed guest-phys).  A test
   watch captured 40000 writes/region during AZER1 play (hit the 40000 cap).  The
   SIGUSR2 full-RAM SNAPSHOT (.ram.bin) does NOT fire in this env (its fist_req poll
   is inside the VGA-A0000 write hook the extender's voxel writer bypasses) -- use
   the live watch/writeflat trace, not the snapshot.

## Scope (honest)

This is the LARGEST remaining piece of the port.  Everything to date reconstructed
FIST.DAT (the 16-bit UI/render-driver engine).  The flight/combat SIMULATION is a
separate 32-bit PM program in the extender overlay (0x100000), currently a shim stub
-- which is why the port's mission is a frozen scene with a ticking clock.  Building
it = decompiling/porting that overlay the same disciplined way FIST.DAT was done,
verified against the oracle transaction log.  Multi-phase, not a bounded change.

## Next steps (ordered)

1. Capture the flight-model transaction log with CORRECT arms: FIST_WATCHFLAT on the
   engine-flat DGROUP (0x10000000+) for the mission-state fields (goals counter, unit
   list, player) AND on the extender flight-model region (guest-phys 0x131000+, ext-
   flat) -- windowed to stay under the 40000-hit cap; identify where units/AI/goals
   live and how they evolve tick-to-tick.
2. Locate + decompile the flight-model overlay code (the writers' cs:eip in the trace
   point at it) as a new target, or reconstruct its behaviour from the trace.
3. Reconstruct incrementally in the shim/port so the port's DGROUP + sim state evolve
   write-for-write to the oracle trace, one subsystem at a time (spawn -> unit update
   -> AI -> weapons/damage -> win/lose), native==wasm at each step.
4. Finish the mission-load render frontier (op-0x4c / DGROUP:0x7aa4 viewport geometry,
   board:0001) so the simulated frames render.
5. Done when AZER1 plays itself to a resolved win/lose, deterministic, native==wasm.

## Sim-state LOCALIZED (2026-08-26, decompile + port memdump)

The mission simulation state is the OBJECT REGISTRY DAT_2000_9fbc (engine DGROUP is
0x20000-based: DAT_2000_XXXX = g_mem+0x20000+XXXX, per build/fist.c macros; NOT the
shim's 0x1c000).  Registry = 182 slots stride 4 {obj-nearptr:word, val:word} at
g_mem+0x29fbc; each object lives at g_mem+0x20000+nearptr with a flags byte at +0x17
(bit3=0x08 => "goal" unit).  The HUD goals count DAT_2000_578e (g_mem+0x2578e) is
RECOMPUTED each frame by FUN@fist.c:60248: `578e=0; for 182 slots: if(slot.ptr &&
obj[+0x17]&8) 578e++;` then FUN_1000_65c2 formats it into the "GOALS: " HUD string
(DAT_2000_46db).  So goals 13->11 in the oracle = the flight model DESTROYING
goal-flagged objects (clearing their slot/flag), and the engine re-counting.

PORT BASELINE (from a coop-tick AZER1 run, two memdumps t[0x452]=8000 vs 40000):
  - mission-LOAD WORKS: registry has 114 occupied slots, 33 goal-flagged objects.
  - simulation does NOT run: object-data [0x2a000..0x2d000] only 77/12288 bytes move
    over 32000 ticks; a sampled object is byte-identical; DAT_2000_578e stays 0 (the
    count fn is base-broken/not-run in the port).  Units are spawned but FROZEN.

So the reconstruction target is sharp: the flight-model SIM LOOP that walks the
registry and updates each object (position/physics -> AI/targeting -> weapons/damage
-> destruction), driving DAT_2000_9fbc + object data write-for-write to the oracle
trace, until goal objects are cleared and DAT_2000_578e reaches 0 (win) or the player
platoon is eliminated (lose).  VERIFY handle: watch DAT_2000_9fbc + object region
(engine-flat, CR3-aware FIST_WATCHFLAT) in the oracle and match the port's writes.
Refines next-step 1: the sim state is the registry, not a scalar; capture its trace.

## BREAKTHROUGH: the flight model RUNS (2026-08-26) -- the sim was one handshake away

The flight-model sim is NOT missing code -- it is FUN_0000_c0ca (per-tick), which walks the object
registry DAT_2000_9fbc and calls each object's update method (FUN_0000_c0e5: fist_icall_near vector at
obj*2-0x1bac), then dde2->op-0x1c.  c0ca IS reached in the port (guard DAT_2000_2dab=0, not gated), BUT
only ~3 times at spawn ([0x452]=314) then it STOPPED while the timer kept advancing.

ROOT: the in-mission 459a frame loop presents via the op-0x4c gate, whose completion is the d548 (0x1548)
handshake -- the engine sets d548=1 ("waiting"), the flight model OR-s bit7 (d548=0x81="frame ready"),
then FUN@fist.c:13440 presents + FUN_1000_65c2 (HUD) and resets d548=0.  The port faked bit7 ONLY in the
PIT pump (native_main.c:711), but the present-poll spins on op-0x4c WITHOUT re-entering fist_timer_pump,
so after a few frames d548 never flips -> infinite op-0x4c spin -> the loop never returns to c0ca -> the
sim freezes.

FIX (proven, env-gated FIST_SIMRUN experiment in fist_extender_gate): complete the present INSIDE the
op-0x4c gate (d548: 1->0x81 when d549==0x1c), emulating the flight model's frame-ready signal at the point
the loop actually polls.  RESULT: c0ca now runs CONTINUOUSLY (~1/tick, #8000 at tick 8275), the mission
runs CLEAN to tick 30000 (rc=0), and the object data EVOLVES: 3176/12288 bytes change t2000->t8000 (vs 77
frozen).  The flight model is executing -- units update every tick.  THE SIM IS ALIVE.

REMAINING to reach a resolved win/lose native==wasm:
  1. Make the present-complete FAITHFUL + default (not FIST_SIMRUN-gated): the real signal is the flight
     model OR-ing bit7 into d548 via the TCB+8 pointer d99b installed -- wire that, or drive it from the
     op-0x1c handler, so it is the correct mechanism not an always-ready fake.
  2. Fix the goals-count base-loss (fist.c:60248 `*(byte*)(*piVar4+0x17)` derefs the raw DGROUP offset as
     a host ptr -> DAT_2000_578e stuck at 0) so goals decrement is visible + the win test works.
  3. Verify the sim is FAITHFUL: capture the oracle's registry/object transaction log (live FIST_WATCHPHYS)
     and match the port's object writes tick-for-tick; fix divergences (host-ptr class) until identical.
  4. op-0x1c (dde2) is still stubbed -- determine what the extender op-0x1c must do and whether the sim
     needs it (physics?) vs c0e5 alone.
  5. Run to a resolved win/lose; then wasm byte-identical.

## Visual confirmation + the render frontier (2026-08-26)

Captured the port windshield with FIST_SIMRUN (scratchpad/simfb.png): the COCKPIT renders (dashboard,
MPH/compass gauges, AUTO CONTROL, FIRE, READY) and the RADAR shows LIVE unit blips -- the sim state is
visibly driving the HUD.  So the flight model runs and feeds the instruments.  The WINDSHIELD (top) is
garbage blue static: the op-0x24 / 9200 voxel-terrain render is still broken (board:0001) -- a RENDER
frontier, independent of the now-working sim.  The "GOALS REMAINING" HUD text is also not painting (HUD
text render + the goals-count base-loss).

So the two remaining fronts are now cleanly separated:
  A. SIM (this board): runs; make the present-complete faithful+default; fix goals-count base-loss; verify
     object writes tick-for-tick vs the oracle transaction log; reach a resolved win/lose; wasm identical.
  B. RENDER (board:0001): the windshield voxel terrain (op-0x24 -> 85d0 camera -> 8120 proj -> 9200 texel
     walk) + viewport geometry, so the simulated frames actually draw.
The DoD needs BOTH (every simulated frame produced), but they are now independently attackable, and the
hard blocker (a totally frozen sim) is CLEARED.

## HONEST REFINEMENT: unblocked != faithful (2026-08-26)

Long-run check (FIST_SIMRUN, AZER1): the sim runs but does NOT yet reproduce the oracle's battle.
  - tick 5000: d549(view)=0x20, tick 30000: d549=0x00 (left cockpit 0x1c) -- view-state transitions fire.
  - occupied units ~114->139 (stride8) / 114 (stride4): STABLE -- units are NOT being destroyed, whereas
    the oracle depletes goals 13->11 (units dying).  So combat attrition is not happening faithfully.
  - the present-complete is gated on d549==0x1c, so once the view transitions the hack stops driving it.
CONCLUSION: the op-0x4c present-complete UNBLOCKS c0ca (the sim executes, objects churn, HUD/radar live),
but the always-ready fake is NOT the faithful frame-ready timing, so the sim advances at the wrong cadence
/ takes wrong transitions and does not converge to the oracle's outcome.  "Sim runs" is achieved; "sim
runs FAITHFULLY (oracle-matching)" is the next substantial step and is what the goal requires.
NEXT (sharpened): (1) capture the oracle's per-tick object-registry + d548/d549 transaction log (live
FIST_WATCHPHYS at the in-mission guest-phys) and the port's, diff tick-for-tick; (2) replace the fake
present-complete with the FAITHFUL frame-ready signal (the flight model OR-s bit7 into d548 via the TCB+8
pointer d99b installs -- reconstruct that trigger + its timing) so the cadence matches; (3) fix the
goals-count base-loss so attrition is visible + the win test works; (4) drive to the oracle's resolved
outcome; (5) wasm byte-identical.  The frozen-sim blocker is cleared; faithful reproduction is the work.

## op-0x1c mapped; faithful-combat requirements (2026-08-26)

The extender PM op-table (fist_image.bin:0xcb3, byte-indexed dword -> trampoline 0x10xx -> call handler):
  op-0x18 -> [0xccb]=tramp 0x10ca -> call 0x89b0  (MAP-LOAD; wired in port as m_ext_FUN_0000_89b0)
  op-0x1c -> [0xccf]=tramp 0x1109 -> inline: edi=[0xc99]; ecx=0x20; walk 32 tasks; per task call 0x7fa0
             (transform) storing screen coords [edi+0x22/0x24/0x32/0x34] -> op-0x1c is PROJECTION
             (world->screen for the radar/HUD/render), NOT the damage/combat resolver.
  FUN_0000_1109 / 0x7fa0 are NOT in re_out/fist_ext.c -- the extender decompile only covers the KDV +
  map-load cluster, so the sim/projection functions are in fist_image.bin but NOT yet in the build.

So faithful in-mission play needs, concretely:
  1. The REAL frame-ready cadence, not the always-ready op-0x4c fake: the current hack unblocks c0ca but
     drives d549 into wrong states (0x1c->0x20->0x00) and the wrong tick rate, so combat never resolves.
     Reconstruct how the flight model OR-s bit7 into d548 (the d99b TCB+8 pointer) at the right time.
  2. Decompile + wire the missing extender sim functions: op-0x1c projection (0x1109 + 0x7fa0) and whatever
     resolves weapons/damage/death (units do NOT deplete today -> the damage path is absent/stubbed).  These
     are in fist_image.bin; extend the extender decompile (make kernel-image / assemble_fist) to cover them.
  3. The engine-side object-update methods (c0e5's per-type vectors at type*2-0x1bac) DO run (objects churn)
     but movement/AI without the extender projection+damage loop don't produce attrition.
  4. Verify tick-for-tick vs the oracle registry/d548 trace; reach the resolved win/lose; wasm identical.
STATUS: frozen-sim blocker CLEARED (c0ca runs).  Faithful combat = real frame-ready cadence + decompiling
the missing extender sim/projection/damage functions + oracle-verified attrition.  Substantial but mapped.

## d549 derail confirms: the per-frame extender pipeline must actually run (2026-08-26)

d549 (DAT_1000_d549 @ g_mem+0x1d549) is the view/game state: 0x14/0x16 (menu-ish), 0x1c (cockpit),
0x1e/0x20/0x22 (external/map/etc views).  Under the FIST_SIMRUN present-hack the port's d549 goes
0x1c -> 0x20 -> 0x00, and 0x00 is INVALID (fist.c:9339 `DAT_2000_0a86 = word[&0a88 + d549]` indexes a
table by d549 -> d549=0 reads out of range).  So the mission STATE DERAILS by ~tick 30000, not a clean
phase change.  Root: the present-hack completes the frame WITHOUT the flight model's real per-frame work
(op-0x1c projection 0x1109/0x7fa0 stubbed; op-0x24 windshield render partial/garbage), so the frame data
the engine's game logic reads (camera, projected object positions, render results) is stale/wrong and the
state machine walks into an invalid d549 -> no faithful combat, no attrition, no resolved win/lose.

So "make the present faithful" is not a one-liner: it requires the extender's per-frame pipeline to
actually PRODUCE correct frame data each tick -- op-0x1c projection + op-0x24 render (board:0001) both
running faithfully -- so the frame the engine presents is real, the d548 bit7 is set at the right point
(after that work), the game state stays valid, and c0e5's object updates + the projected combat drive real
attrition matching the oracle.  This is the "finishing the mission-load and per-frame render path" clause
of the goal, and it is genuinely multi-step reconstruction (decompile op-0x1c/0x7fa0 + finish the 9200
windshield render + faithful frame-ready), not a bounded patch.

TURN NET: cleared the frozen-sim blocker (c0ca/c0e5 run).  Proved the flight model is decompiled engine
code + missing extender per-frame functions, not a from-scratch simulator.  Mapped the exact remaining
pipeline to a resolved, oracle-matching win/lose.  The reconstruction of that per-frame pipeline (and then
wasm parity) is the substantial, well-defined work that remains.

## Oracle-derived faithful handshake; combat = cs=0x2082 (2026-08-26)

Captured the oracle's d548/d549 live trace (FIST_WATCHPHYS at the CORRECT guest-phys: engine base 0x11190,
so DGROUP:0x1548 = 0x2e6d8, and DAT_2000_9fbc registry = 0x3b14c -- addressing SOLVED).  Findings:
  - d548 frame-ready = the flight model ORs bit7 into the WAITING state, which is 1/2/3 -> 0x81/0x82/0x83
    (not only 1->0x81).  The port hack handled only 1 -> incomplete handshake -> d549 DERAILED to invalid 0.
  - FIX: OR bit7 into any waiting state (1/2/3).  RESULT: d549 now STAYS VALID and cycles 0x1c<->0x20 like
    the oracle (tick5000 d549=0x20 d548=0x83; tick20000 d549=0x1c d548=0x81) -- the derail is GONE.
  - the frame-ready is written by cs=0x2082 (the EXTENDER flight model) at eip 0x82e4/0x831e/0x8382/0x8402/
    0xb17e/0xb247; the "waiting" 1/2/3 is written by cs=0x1119 eip=0x23d1 (562x, the engine present setup).
  - oracle d549 spends most time in 0x1c (cockpit) cycling to 0x20/0x22 -- so the port's view-cycling is
    faithful; only the stuck-0x00 was the bug (now fixed).

STILL: units do NOT deplete (133 stable) -- COMBAT/DAMAGE is absent.  It runs in cs=0x2082 (the extender
flight model), which the port STUBS.  So the next concrete target is cs=0x2082's per-frame combat: the
functions at eip 0x82e4../0xb17e.. (and what they call) resolve weapons/damage/death and write the object
registry.  These are in fist_image.bin (the extender), NOT in fist_ext.c -- decompile + wire them (the
proven pipeline), verify the port's registry writes tick-for-tick vs the oracle (FIST_WATCHPHYS=0x3b14c),
until goal units deplete to a resolved win/lose.  Handshake: FAITHFUL.  Combat: the mapped remaining work.

## Combat code located: flat 0x28000+ (cs=0x2082) -- a NEW decompile target (2026-08-26)

The cs=0x2082 flight-model writers (frame-ready + combat) live at flat linear 0x28000..0x2c000
(flatip 0x28b04, 0x28b3e, 0x2b9dc, 0x2ba6c, ...).  This is ABOVE the engine (0x10000..0x1c000) and BEYOND
re_out/fist_image.bin (size 0xbf90) -- so the flight-model combat/physics code is a SEPARATE region NOT in
either decompile (fist_dat_image.bin = FIST.DAT engine; fist_image.bin = FIST.RUN kernel).  It is the
extender's PM flight model proper, loaded at runtime (the overlay the goal calls "the part... at 0x100000").
CONCRETE NEXT: extract the flat 0x28000..0x2c000 (and its callees) from the running extender / FIST.RUN,
add it as a decompile target (make image variant), decompile -> assemble -> wire it as the op-per-frame
flight-model step, and verify the object-registry writes (FIST_WATCHPHYS=0x3b14c) tick-for-tick vs the
oracle until goal units deplete to a resolved win/lose.

TURN NET (major, oracle-verified): (1) frozen-sim blocker CLEARED (c0ca/c0e5 run); (2) faithful d548
handshake (OR bit7 into waiting 1/2/3) -> d549 no longer derails, cycles 0x1c<->0x20 like the oracle;
(3) oracle-verification addressing SOLVED (DGROUP:0x1548=guest-phys 0x2e6d8, registry=0x3b14c, live trace
works); (4) combat PRECISELY located (flat 0x28000+, cs=0x2082) and confirmed a new decompile target.
The goal (faithful win/lose native==wasm) now reduces to: decompile+wire the flat-0x28000 flight-model
combat, finish the op-0x24 windshield render (board:0001), verify vs the oracle, reach the outcome, wasm
byte-identical.  Substantial, but every piece is now located, tooled, and falsifiable.

## The combat cluster is cs=0x2082, MISSED by the decompile (2026-08-26, decisive)

Oracle registry-writer trace (FIST_WATCHPHYS=0x3b14c during AZER1 play): the object/unit registry
DAT_2000_9fbc is written overwhelmingly by cs=0x2082 (eip 0xbb0f 1536x, 0xbb6e/0xbb26/0xbc77 ...; base
SegPhys=flatip-eip=0x20820, so the code runs at guest-phys 0x2c000..0x2c500).  The writer's stack operates
on DAT_2000_3ae0 (player) + unit slots -> this IS the flight/combat model that moves and destroys units.
CRITICAL: the decompile has ONLY CS clusters 0x0000 (7663 fns), 0x1000 (4708), e000 (4) -- cs=0x2082 has
ZERO functions.  So the combat cluster is NOT decompiled (not merely unwired): the Ghidra pipeline
(PrepAnalysis/SegWrapFixup discover 0x1000/0xf69/e000) never threaded the 0x2082 code segment.  That is
exactly the goal's "flight/combat model... in the overlay... not in FIST.DAT".

CONCRETE NEXT (the reconstruction, proven pipeline):
  1. Determine cs=0x2082's source: is guest-phys 0x20820.. inside FIST.DAT's loaded image (a cluster Ghidra
     skipped) or the FIST.RUN extender overlay?  Capture the code bytes at guest-phys 0x2c32f (the 0xbb0f
     writer) and grep them in FIST.DAT / FIST.RUN / fist_dat_image.bin to identify the binary + offset.
  2. Thread the cs=0x2082 cluster into the Ghidra pipeline (extend SegWrapFixup/PrepAnalysis to discover +
     analyze it, or add it as an image target) -> decompile -> assemble into a new TU.
  3. Wire it as the per-frame flight-model step (it is what op-0x1c/the c0ca chain should reach) and verify
     the port's registry writes tick-for-tick vs the oracle (0x3b14c) until goal units deplete.
  4. Finish op-0x24 windshield render (board:0001); reach the resolved win/lose; wasm byte-identical.

TURN NET: sim runs; handshake faithful (no derail, d549 cycles like the oracle); oracle addressing solved;
combat cluster IDENTIFIED as cs=0x2082 and PROVEN absent from the decompile -> the precise, falsifiable
next decompile target.  The goal is a bounded reconstruction of one missed code cluster + render + verify,
not an open-ended "write a simulator".
