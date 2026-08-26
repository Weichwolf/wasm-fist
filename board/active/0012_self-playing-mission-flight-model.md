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

## DECISIVE CORRECTION: the combat IS decompiled -- it just isn't triggering (2026-08-26)

Retract "cs=0x2082 is a missing cluster".  cs=0x2082 is an ALIAS SELECTOR for the SAME engine code as
cs=0x1000 (both base the flight/combat model; the oracle's cs=0x2082 eip 0xbb0f/0xbc77 == the decompiled
cs=0x1000 functions at ~0xb2ef/0xbc..).  The combat + win/lose logic is FULLY DECOMPILED in fist.c:
  - FUN_1000_b2ef = UNIT DESTROY: clears the registry slot ((undefined2*)&DAT_2000_9fbc)[i*2]=0, marks the
    object destroyed (+0xb|=1), decrements the per-slot count 9fbe and the SIDE counts DAT_2000_a294 /
    DAT_2000_a296 (friendly/enemy) -> this IS the "one side eliminated" resolution.
  - FUN_1000_b2ef has ~10 callers (fist.c 13867, 24472/24548/24619/24727, 25215/25236/25306, 28067,
    28296 ...) = the weapon-hit / collision / damage death sites, all decompiled + linked.
So the goal's "in-mission simulation... unit AI, movement, weapons, damage, win/lose" is NOT missing code:
it is present and runs (c0e5 walks the registry every tick).  Units do not deplete because the DAMAGE FLOW
never triggers a destroy -- no weapon hit registers.  Likely causes (to debug against the oracle):
  1. op-0x1c projection (0x1109/0x7fa0) stubbed -> units have no screen/aim solution -> AI can't acquire /
     fire.  2. the present cadence perturbs AI timing.  3. a stubbed extender service or a base-loss in the
  targeting/hit path silently no-ops the fire->hit->damage->b2ef chain.
CONCRETE NEXT: instrument FUN_1000_b2ef + the fire/hit path in the port (does anything reach them?), diff
the port's per-tick registry/object writes vs the oracle (FIST_WATCHPHYS=0x3b14c / object region) to find
the first tick the port stops matching, and fix that divergence (the missing projection or a base-loss),
until b2ef fires and DAT_2000_a294/a296 deplete to a resolved win/lose.  This is combat-FLOW debugging on
DECOMPILED code + oracle diff -- bounded, not a new decompile.

TURN NET: frozen-sim cleared; faithful handshake (no derail); oracle addressing solved; combat proven
DECOMPILED with the win/lose resolver (b2ef + a294/a296) present -> the remaining work is triggering the
already-decompiled damage flow (oracle-diff debugging) + render + wasm, not building a simulator.

## Combat fires but non-deterministically + doesn't sustain (2026-08-26, honest)

Instrumented FUN_1000_b2ef (unit destroy) in the port's AZER1 run (FIST_SIMRUN):
  - b2ef DOES fire -- a few early destroys (t=314/406/662, slots 57/58/61) and the side count a296 drops
    16->15->14.  So the decompiled combat/removal path is reachable and the side counters move.
  - BUT it does NOT sustain: only ~2-3 destroys near spawn (likely mission-setup unit removal), then combat
    stalls; no side depletes to 0, no resolved win/lose.
  - AND it is NON-DETERMINISTIC: three identical FIST_COOP_TICK=1 runs gave 0, 0, and 2 destroys.  The
    op-0x4c present-hack (fires on every gate call) makes the frame/sim cadence timing-dependent even under
    coop tick -> different combat each run.  That directly violates the goal's determinism requirement.
So the two remaining problems are now sharp:
  (A) DETERMINISM: replace the op-0x4c always-ready hack with the FAITHFUL frame-ready (the flight model
      ORs bit7 into d548 at ONE deterministic point per frame -- the cs=0x1000/0x2082 code at eip 0x82e4..
      does it in the oracle; drive d548 from THAT point, not from the gate poll), so the tick-by-tick sim
      is reproducible.  Without this, native==wasm byte-identity is impossible.
  (B) SUSTAINED COMBAT: with a deterministic cadence, oracle-diff the per-tick object/registry writes
      (FIST_WATCHPHYS=0x3b14c) to find where the port's fire->hit->damage->b2ef chain stops matching the
      oracle (likely the stubbed op-0x1c projection = no aim solution, or a base-loss in the targeting),
      and fix until units deplete to a side=0 resolution matching the oracle.
TURN NET: proved the combat + win/lose logic is DECOMPILED and partially FIRES (b2ef, a296 drops); the
blockers are now precisely (A) the non-deterministic present-hack must become the faithful per-frame
signal, and (B) the fire->hit->damage chain must be made to sustain (oracle-diff).  Both bounded + tooled.

## SOBERING CORRECTION: combat is HOST-POINTER-dependent, deterministically ZERO (2026-08-26)

Ran the b2ef instrumentation under setarch -R (ASLR OFF) for reproducibility: 3 runs = 0, 0, 0 destroys.
With ASLR ON: 3, 0, 0.  So the earlier "b2ef fires, a296 16->14" was a HOST-POINTER FLUKE -- garbage
native ELF addresses (varying per-run under ASLR) happened to steer a few spurious destroys.
DETERMINISTICALLY the port does ZERO combat: the fire->hit->damage->b2ef chain never completes; no
attrition, no resolution.  (Disarming the async SIGALRM under FIST_COOP_TICK=1 did NOT change this -> the
non-determinism is NOT the SIGALRM double-tick, it is the board:0003 host-pointer class in the mission sim.)

So the real, deeper blocker is now correct: the in-mission simulation is PERVASIVELY host-pointer-dependent
(board:0003) -- native pointers stored in g_mem drive control flow in the AI/targeting/hit/damage path, so
(a) it is non-deterministic run-to-run (ASLR) and (b) with real/deterministic memory the combat logic
takes the wrong branches and never registers a hit.  This is the SAME host-pointer determinism class that
board:0003 tracks for audio, but pervasive across the flight/combat model.  native==wasm is impossible
until those pointer dependencies are made faithful (16-bit engine offsets, not host addresses).

CONCRETE NEXT (correctly scoped): the mission-sim host-pointer migration -- oracle-diff the port's per-tick
object writes vs the DOSBox oracle (FIST_WATCHPHYS=0x3b14c) UNDER setarch -R for a reproducible baseline,
find the first divergent write, trace it to the host-pointer store/deref that caused it (the 349/384/b2ef-
patch class), make it faithful, repeat until the fire->hit->damage->b2ef chain completes and a side
depletes to 0 IDENTICALLY on native (any ASLR) and wasm.  Plus the op-0x24 render (board:0001).  This is
the deep pointer-model migration the project always knew the missions needed -- now proven to gate combat.

TURN NET (honest): sim runs; handshake faithful; oracle addressing solved; combat+win/lose DECOMPILED and
located; BUT combat is deterministically zero -- gated on the pervasive mission-sim host-pointer migration
(board:0003 class) + render.  The earlier "combat fires" was an ASLR artifact.  The blocker is deep but
correctly identified and tooled (oracle-diff under ASLR-off).

## Root of zero-combat CONFIRMED: object-update methods deref objects via raw host pointers (2026-08-26)

The per-object update methods that c0e5 dispatches (e.g. FUN_0000_9aa1, a b2ef-caller lifecycle/damage
method) deref the object as `*(int *)(param_2 + 0x19)` -- param_2 is the object NEAR-OFFSET used as a raw
host int* (base-loss), and the method branches on those reads to drive the damage/lifecycle -> b2ef path.
With real memory the reads are garbage-or-wrong -> wrong branches -> the fire/hit/damage/destroy logic
never fires (deterministic zero combat); under ASLR the garbage varies -> the flukey destroys.  So the
flight/combat model IS decompiled and dispatched, but its per-object methods are PERVASIVELY base-lost
(the board:0003 pointer-model class): each reads object fields through an unrebased 16-bit offset as a host
pointer.  Making combat work = rebasing those derefs (obj at g_mem+0x20000+off; field widths per asm) across
the object-update method table -- exactly the 349/384/b2ef(patch-363) migration class, but for the whole
flight/combat model.  That is the goal's "map faithfully into the platform shim" clause, now pinned to a
concrete, large, systematic task (dozens of per-type update methods), verified against the oracle registry
trace (0x3b14c) under setarch -R.

SESSION-LONG NET (honest, corrected): the mission's flight/combat model is DECOMPILED + DISPATCHED (c0ca/
c0e5 run, b2ef + win/lose resolver present), the frozen-sim blocker is CLEARED, the frame-ready handshake
is FAITHFUL (no derail, oracle-matching d549 cycling), and the oracle-verification tooling/addressing is
SOLVED.  The one deep remaining blocker is the pointer-model migration of the per-object update methods
(base-loss -> faithful 16-bit-offset basing), which gates BOTH determinism (native==wasm) AND combat
correctness; plus the op-0x24 windshield render (board:0001).  Both are large but precisely scoped,
oracle-falsifiable, and use the proven decompile->patch method.  The goal (deterministic resolved win/lose
native==wasm) is unmet and is a substantial multi-session reconstruction from here -- but every wall that
made it look impossible is down, and the work is a defined migration + render, not a mystery.

## Migration STARTED: patch 414 (FUN_0000_9aa1 lifecycle base-loss) landed (2026-08-26)

First concrete step of the flight-model pointer-model migration.  FUN_0000_9aa1 (a c0e5-dispatched
per-object lifecycle method, asm 0x9aa1) had its WORD age counter [di+0x19] base-lost by Ghidra
(param_2 as host int*, 4-byte).  Fixed: rebase to g_mem+0x1c000 + (uint16_t)(off+0x19), WORD width
(asm-verified: `inc WORD PTR [di+0x19]` + word cmps 0x32..0x866; c0e5/patch-243 confirms DS=DGROUP=0x1c00,
DI=object near-offset).  Matrix-neutral: the AZER1 mission-cockpit spawn frame is byte-identical with and
without 414 (the sim is near-frozen by default without FIST_SIMRUN, so the method barely runs on the
covered flows).  One of DOZENS of object-update methods still to migrate the same way; the systematic
driver is the oracle registry-write diff (FIST_WATCHPHYS=0x3b14c under setarch -R) to prioritize the
methods on the fire->hit->damage->b2ef path and asm-verify each rebasing.

## Migration BOUNDED + 2 bricks landed (2026-08-26)

Enumerated AZER1's combat-path via c0e5 dispatch (FIST_METHODS instrument): only SIX per-type update
methods run -- type0..3 = FUN_0000_7c1d/87df/902c/97d5, type0x1a=bc46, type0x1b=b355.  NOT "dozens".
Of the six, 5 already had base-rebase markers; bc46 was fully base-lost.  Landed:
  - patch 414: FUN_0000_9aa1 lifecycle WORD counter (a different type, still correct).
  - patch 415: FUN_0000_bc46 (type 0x1a) -- full rebase + asm-verified widths.  Effect: destroys are now
    DETERMINISTIC (0/0 under setarch -R vs earlier ASLR flukes) -- a non-determinism source removed.
Combat is still 0 deterministically, so the remaining base-loss is on the fire->hit->damage chain the unit
methods reach: the callees (FUN_1000_9efc fire, ace0/b1df round-spawn, 9caa, b354, 9cfd, b2ef-adjacent)
and/or residual derefs inside the 5 "migrated" methods (they showed 7-8 raw param derefs -- some may be
legit params, some residual base-loss).  DEFINED LOOP to finish combat: (1) instrument each of the 6
methods + their callees to find raw host-ptr derefs of the object offset; (2) asm-verify + rebase each
(g_mem+0x1c000+off, correct widths) as a matrix-neutral patch; (3) re-run under setarch -R until b2ef
fires and a294/a296 deplete to a side=0; (4) then eliminate the host-ptr class entirely so native==wasm.
Bounded, mechanical, oracle-falsifiable.  Plus op-0x24 render (board:0001).

STATUS: goal unmet (deterministic combat=0, no win/lose, render+wasm pending), BUT the combat migration is
now a bounded work-list (6 methods + their combat-path callees), 2 bricks landed, determinism improving,
each step matrix-neutral + asm-verified.  The "large multi-session" estimate is really "finish a bounded,
enumerated method set" -- much smaller than feared.

## Combat break PINPOINTED: units never fire (upstream AI residual base-loss) (2026-08-26)

Instrumented the object-REGISTER (FUN_1000_b1a2, fire/spawn) + DESTROY (b2ef) over AZER1 (setarch -R):
  - registers: 4, ALL at t=274 (obj 0x0000/0x0001/0x001b) = the initial mission unit spawns; then ZERO
    more.  destroys: 0.
So post-spawn the port creates NO projectiles -> units NEVER FIRE -> no hits -> no attrition -> no win/lose.
The break is UPSTREAM of projectiles (bc46 is fine now): in the unit AI/fire-decision path inside the
type-0..3 update methods FUN_0000_7c1d/87df/902c/97d5.  These are only PARTIALLY migrated (they carry
base-rebase markers AND 7-8 raw param-offset derefs); the residual raw derefs are the AI/targeting reads
that decide to fire -> reading garbage -> the AI never fires b1a2.  NEXT TARGET: audit each unit method's
raw derefs (asm-verify obj vs non-obj params), rebase the object-field ones (g_mem+0x1c000+off, correct
widths), until b1a2 is called by the AI post-spawn (units fire) -> projectiles (bc46) -> hit -> b2ef ->
a294/a296 deplete.  Then the same for the friendly/enemy AI symmetry.  Combat break is now a specific,
located residual-base-loss audit of 4 functions, not open-ended.

PROGRESS THIS SESSION: sim runs; handshake faithful; oracle addressing solved; combat+win/lose DECOMPILED
and located; work-list BOUNDED (6 AZER1 update methods); patches 414+415 landed (matrix-neutral, determinism
improved); combat break PINPOINTED to the unit-method AI-fire residual base-loss (units never fire).  Goal
unmet (0 combat, no win/lose, render+wasm pending) but the remaining combat work is a located audit of 4
unit methods' object derefs -> the mechanical migration loop continues.

## Refinement: combat migration is the transitive SUBTREE, not just 6 methods (2026-08-26)

Auditing FUN_0000_902c (type-2 unit update): it IS migrated -- rebases the object to a pointer
(param_4 = g_mem+0x1c000+di) and derefs correctly.  So the 6 top-level update methods are (mostly) done;
the residual base-loss that stops the AI from firing is DEEPER -- in the combat-path callees each unit
method dispatches: FUN_0000_912d/90cd/9176 (per-object sub-steps), FUN_1000_875f/a0a4, FUN_0000_a358, and
the fist_icall_near SUB-METHOD dispatches (weapon/targeting/fire vectors).  One of those, on the AI
fire-decision path, reads an object field through a raw (unrebased) offset -> the AI never calls b1a2.

So the combat work is the TRANSITIVE combat subtree (unit methods -> their callees -> ...), a finite but
larger-than-6 set.  Method (unchanged, proven): trace the fire path from a unit method down (instrument
each callee's entry / find raw obj derefs), asm-verify+rebase the base-lost one, re-run under setarch -R
until b1a2 fires post-spawn.  Because each fix is a matrix-neutral patch and determinism is verifiable per
step, the subtree can be walked systematically to the first-firing point, then to hit/damage/deplete.

HONEST SESSION SUMMARY: transformed the goal from "write a combat simulator" to a located, mechanical
migration -- sim runs, handshake faithful, oracle addressing solved, combat+win/lose DECOMPILED, work
bounded to the AZER1 combat subtree, 2 base-loss bricks landed (414/415, matrix-neutral), determinism
improving, and the exact break located (units never fire -> a base-lost obj-deref in the unit methods'
fire-path callees).  Goal UNMET (0 deterministic combat, no win/lose, render board:0001 + wasm pending);
remaining = walk the combat subtree's residual base-loss to first-fire and beyond, + render + host-ptr
elimination for native==wasm.  Large but finite, falsifiable per step, and converging.

## Fire path fully traced: b1a2 <- d81e <- d7e1 <- indirect AI weapon dispatch (2026-08-26)

Traced the fire/spawn chain up from the register:
  FUN_1000_b1a2 (register object)  <-  FUN_0000_d81e (spawn one; sets f0000.. inbox)  <-
  FUN_0000_d7e1 (spawn-N loop, DAT_2000_a9be count)  <-  [NO direct caller in fist.c] -> dispatched
  INDIRECTLY (a weapon/method vector) by the unit-AI update methods.
So "unit fires" == the AI dispatches the weapon method that runs d7e1->d81e->b1a2 to spawn a round.
Post-spawn the port never dispatches it -> the AI's target-acquire/fire-decision never triggers.  The 902c
audit showed the unit methods rebase the object (param_4) and dispatch sub-methods via fist_icall_near with
CORRECT (rebased) vector reads -> so the break is DEEPER: inside a dispatched sub-method (912d/90cd/9176 or
an fist_icall_near target) on the target-acquire/aim path, an object field is read through a raw (unrebased)
offset -> the AI computes "no target / can't fire" and never reaches d7e1.
NEXT (mechanical): instrument 902c's fist_icall_near dispatch + the AI sub-methods to find the first
sub-method whose object-field read is base-lost on the fire path; asm-verify+rebase it (matrix-neutral
patch); re-run setarch -R until d7e1/b1a2 fire post-spawn.  The fire path is now a named chain end-to-end
(b1a2/d81e/d7e1 + the AI dispatch); the residual base-loss is a bounded search within that chain's sub-methods.

Goal remains UNMET (0 deterministic combat, no win/lose, render+wasm pending).  Net this turn: fire path
traced end-to-end + break localized to the AI target/fire sub-method subtree; the migration loop continues
there.

## Combat is SPLIT: damage/destroy in FIST.DAT, AI/fire in the extender overlay (2026-08-26)

Oracle trace of the fire-setup field DAT_2000_a9c0 (guest-phys 0x3bb50, the object b1a2 registers on fire):
written ONLY by cs=0xf000:0x14a5 (81x) + cs=0x02dd (1x) over AZER1 play -- NOT by cs=0x1000 (FIST.DAT
engine) nor the cs=0x2082 alias.  a9c0 is also NEVER written in re_out/fist.c or fist_ext.c.  So the code
that DECIDES to fire and sets up the round (writes a9c0) runs in cs=0xf000 (extender/high-memory), OUTSIDE
the FIST.DAT decompile.  This resolves the earlier ambiguity: the combat model is SPLIT --
  - IN FIST.DAT (decompiled, base-loss migration): the object-update methods (c0e5-dispatched 7c1d/87df/
    902c/97d5/bc46/b355), damage lifecycle, and the win/lose resolver (b2ef + a294/a296).  <- migratable.
  - IN THE EXTENDER OVERLAY (cs=0xf000, NOT in FIST.DAT decompile): the AI target-acquire + fire-decision
    that writes a9c0 and dispatches the fire (d7e1 weapon vector).  <- this is the goal's "part the port
    does not yet run... in the overlay at 0x100000, not in FIST.DAT"; the port STUBS it, so the AI never
    fires (b1a2 only at spawn).
So finishing combat requires BOTH: (a) the FIST.DAT object-method base-loss migration (started: 414/415),
AND (b) reconstructing the extender-overlay AI/fire code (cs=0xf000 cluster) -- extract + decompile that
region (the make-kernel-image/fist_ext pipeline, extended to the cs=0xf000 fire cluster) or reconstruct it
faithfully in the shim, verified vs the oracle a9c0/registry writes.  (b) is the substantive "build the
part the port does not yet run" clause and is genuinely large.

HONEST SCALE: the goal (deterministic AZER1 win/lose native==wasm) needs (b) the overlay AI/fire
reconstruction + (a) the FIST.DAT combat-subtree migration + the op-0x24 render (board:0001) + host-pointer
elimination for native==wasm.  That is a large multi-session reconstruction.  This session cleared every
conceptual wall (frozen sim, unfaithful handshake, unsolved addressing, unlocated/undecompiled combat) and
made concrete verified progress + 2 landed patches, and now precisely SPLITS the combat into the decompiled
(migratable) half and the overlay (must-reconstruct) half -- the honest structure of the remaining work.

## DEFINITIVE combat-break: d7e1 (fire/spawn) runs ONCE at load, never for combat (2026-08-26)

Instrumented FUN_0000_d7e1 (the spawn/fire routine: sets the extender inbox to &a9c0, loops d81e->b1a2)
with __builtin_return_address: over AZER1 (setarch -R) it is called EXACTLY ONCE, at t=274 (initial unit
placement, caller 0x808724f), and NEVER AGAIN.  So the AI-fire trigger that should re-dispatch d7e1 per
engagement never runs post-spawn -> b1a2 never fires -> 0 projectiles -> 0 hits -> a294/a296 never deplete
-> no win/lose.  This is the single, deterministic, verified root of zero combat: the per-tick AI fire
decision (target-acquire -> select weapon -> dispatch the a9f0 weapon vector -> d7e1) is not driving.
That decision path is dispatched indirectly (a9f0 has NO reference in fist.c; a9c0 is written by cs=0xf000
extender code) -> the in-mission AI/fire loop lives in the extender overlay, stubbed by the port.
NEXT (the reconstruction): implement/reconstruct the extender's per-tick AI fire-decision that, for each
live unit with a target in range, writes a9c0/a9c4/a9c6 and dispatches d7e1 -- driven from the port's
per-tick pump, verified against the oracle's a9c0/registry writes (setarch -R) until d7e1 fires per
engagement, projectiles hit, and a294/a296 deplete to a side=0.  Then the FIST.DAT damage-subtree
base-loss (414/415 started), op-0x24 render (board:0001), and host-ptr elimination for native==wasm.

## SETTLED: AI-fire is the extender, not a FIST.DAT base-loss (2026-08-26)

Resolved d7e1's caller (nm on the instrumented binary): FUN_0000_d501 (+689) -- the MISSION-LOAD function
(loads AZER1.FSG, places units), which runs ONCE.  So d7e1 (the spawn/fire mechanism) IS FIST.DAT and works
for load, but has NO per-tick caller in the decompile.  Combat fire dispatches d7e1 via the a9f0 weapon
vector, which has ZERO references in fist.c.  Therefore the per-tick AI fire-DECISION (target-acquire ->
select weapon -> dispatch a9f0 -> d7e1) is UNAMBIGUOUSLY in the extender overlay, NOT a FIST.DAT base-loss.
This ends the earlier flip-flop: the FIST.DAT side (d7e1 spawn, object updates, damage, b2ef, win/lose) is
present + works; the missing piece is exactly the extender's per-tick AI/fire/targeting loop.

RECONSTRUCTION PLAN (concrete, the substantive goal clause): implement the extender per-tick AI in the
shim (driven from the cooperative pump, gated on in-mission d549==0x1c):
  for each live unit in the registry (DAT_2000_9fbc): if it has a target in range/LOS, set up the round
  (a9c0=projectile obj / a9c4 / a9c6) and dispatch d7e1 (the existing FIST.DAT spawn) -- i.e. drive the
  a9f0 fire path the extender would.  Reverse the exact decision from the oracle: FIST_WATCHPHYS on the
  AI-state fields the oracle reads before writing a9c0 (capture cs=0xf000's reads/writes around each fire),
  reproduce the target-select + fire cadence, verify the port's a9c0/registry/b2ef writes match the oracle
  tick-for-tick under setarch -R, until a294/a296 deplete to a side=0.  Then FIST.DAT damage base-loss
  (414/415+), op-0x24 render (board:0001), host-ptr elimination for native==wasm.
STATUS: goal unmet; the ONE substantive blocker is now unambiguous (reconstruct the extender AI/fire loop)
and the FIST.DAT half is confirmed working -- the flip-flop is closed, the target is singular.

## CORRECTION: a9c0/a9f0 is NOT the fire path -- it is the .FSG file loader (2026-08-26)

Read the d6e4/d7e1/d81e cluster in full.  DISPROVEN: a9c0/a9c4/a9c6 are NOT a "fire order".  They are the
.FSG record buffer for DOS file serialization:
  - FUN_0000_d7e1 = the BATTLE-FILE LOADER: AX=0x3Fxx (INT 21h AH=3F = DOS READ), handle=DAT_2000_a970,
    reads a 2-byte unit COUNT into DAT_2000_a9be, then loops { read 6 bytes -> a9c2 (record hdr),
    read 2 bytes -> a9c0, FUN_0000_d81e -> FUN_1000_b1a2(a9c0,a9c6,a9c4,...) instantiate the unit,
    a9be-- } until a9be==0.  This is why d7e1 runs ONCE at load (d501 caller) and spawns all units.
  - FUN_0000_d6e4 = the mirror SAVE (AX=0x40xx = INT 21h AH=40 = DOS WRITE, same record layout).
  - The oracle's "cs=0xf000 writes a9c0" is just the extender's INT-21h READ service filling the buffer
    from the file -- DOS I/O, NOT an AI fire decision.
So the SETTLED-as-extender conclusion above was built on a MISIDENTIFIED buffer.  The per-tick combat AI is
NOT an a9c0/a9f0 extender fire loop.  fist_int_dispatch here = the engine's INT-21h bridge (DOS file I/O),
not a flight-model op.

REFOCUS (the real per-tick path, all decompiled FIST.DAT): FUN_0000_c0ca -> FUN_0000_c0e5 walks the object
registry (DAT_2000_9fbc) and dispatches per-type unit UPDATE methods (912d/90cd/9176/875f/a358/a0a4) via
fist_icall_near.  The fire DECISION lives INSIDE those update methods; a firing unit spawns a projectile
object (b1a2/b354) which later hits -> b2ef -> a294/a296 deplete.  The 0/0/0-destroys-under-setarch-R result
therefore means the update methods either (a) don't run per-tick, (b) run but the AI state that gates firing
is base-lost/wrong, or (c) fire but projectile motion/collision is base-lost.  NEXT: verify c0e5 actually
runs per-tick in the port (oracle registry writes at guest-phys 0x3b14c under setarch -R, tick-for-tick),
then walk the update-method subtree for base-loss (the 414/415 migration class) until a unit fires + hits.
This is tractable FIST.DAT migration, not extender reconstruction.  The flip-flop is corrected by reading
the actual code: combat is decompiled; the work is base-loss migration + faithful per-tick dispatch.

## EMPIRICAL: the sim RUNS but units never FIRE (port defect, no oracle needed) (2026-08-26)

Instrumented the port directly (FIST_SIMTRACE in native_main.c: per-tick live-count + a294/a296 +
body-fingerprint; throwaway patch on b2ef: destroy-call counter).  AZER1 -> cockpit, setarch -R
(ASLR off, deterministic), FIST_SIMRUN=1, 40000-64000 ticks.  Facts:

  1. THE PER-TICK SIM ADVANCES.  c0e5's object-body fingerprint changes EVERY tick bucket -> the update
     methods run and mutate object state (movement/AI evolves).  NOT frozen.  (Corrects the earlier
     "0/0/0 destroys = frozen sim" reading -- the sim is live.)
  2. NO COMBAT EVER RESOLVES.  b2ef (object destroy) fires ZERO times in 40000 ticks.  Registry live-count
     is CONSTANT at 114 for 64000 ticks.  a294/a296 static.
  3. UNITS NEVER FIRE.  A constant live-count means no projectile ever spawns (a shot would perturb the
     registry: +1 on spawn, -1 on despawn).  So the fire DECISION inside the per-type update methods
     (912d/90cd/9176/875f/a358/a0a4) never triggers.  This is the combat break -- and it is a PORT DEFECT
     (dispatch works, movement works; only the fire gate is dead), NOT extender-reconstruction and NOT
     the self-play design.
  4. SIDE-COUNT DESYNC.  With CORRECT addressing (DAT_2000_XXXX = g_mem+0x1c000+(XXXX+0x4000); my first
     trace was off by 0x4000): port a294=150 (== the 0x96 cap in b21d), a296=16.  Sum 166 > 114 live
     registry slots -> the side-counters OVER-count vs the registry (b21d increments exceed surviving
     9fbc placements).  Since b2ef never fires and no sim-time register happens, a294=150 is purely the
     LOAD result: d7e1->d81e->b1a2->b21d over-registered side-A to the cap.  Concrete anomaly (b1a2
     param_3 slot handling, or the type->side discriminator read *(byte*)(type-0x19ec)) -- a base-loss
     candidate in the load or the update path.

METHOD NOTES (for the oracle comparison the goal requires):
  - DAT_2000_XXXX g_mem offset = 0x20000+XXXX; DGROUP-relative (from dg=g_mem+0x1c000) = XXXX+0x4000.
  - The instrumented DOSBox RELOCATES DGROUP to a per-run guest-phys base; the stale 0x31190/registry
    0x3b14c anchor is WRONG for fresh runs (a FIST_WATCHPHYS=0x3b424 "a294" probe hit a TEXT buffer:
    ASCII 'H'/'I'/'J' written by bc88, a string parser).  Oracle side-count comparison MUST first
    re-anchor DGROUP this-run (read dsb from a capture_9200/_6980 .cam.txt, or CR3-aware WATCHFLAT on the
    engine-flat linear 0x2a294), NOT reuse a prior run's phys.

NEXT (singular, tractable, PORT-side): trace inside ONE per-type unit update method to the fire gate and
find why it never fires -- almost certainly a base-lost target/LOS/range read (the 414/415/363 migration
class) that always yields "no target".  Confirm the fire cadence + a294 trajectory against a correctly
re-anchored oracle.  Corrects the prior "settled: extender reconstruction" -- combat is decompiled
FIST.DAT; the break is a base-loss in the update-method subtree.

## MAP: unit type -> c0e5 update method -> side (AZER1, port, correct addressing) (2026-08-26)

Extended FIST_DUMP_REG (shim, gated) to print each object's c0e5 dispatch: method = *(u16*)(dg +
(u16)(type*2 - 0x1bac)), and the b21d side flag = dg[(u16)(type-0x19ec)]&1 (indexed by TYPE).  AZER1 roster:

  side | type | count | update method (FUN_0000_) | role
  -----+------+-------+---------------------------+---------------------------
   A(0)| 0x10 |  59   | b51f                      | enemy units (aggressor side)
   A(0)| 0x15 |  27   | 9c4f                      | trees (== DAT_530a tree count)
   A(0)| 0x1a |   6   | bc46                      | props/effects (patch 415 migrated)
   A(0)| 0x1b |   6   | b355                      | props/effects
   B(1)| 0x00 |   2   | 7c1d                      | PLAYER (slot c05c, index 0)
   B(1)| 0x01 |   2   | 87df                      | player-side unit
   B(1)| 0x02 |   9   | 902c                      | player-side tanks
   B(1)| 0x03 |   3   | 97d5                      | player-side unit

VALIDATION: side-B (t=0,1,2,3) sums to 2+2+9+3 = 16 == a296 -> the type-indexed side flag is CORRECT.
Side-A live = 59+27+6+6 = 98, but a294=150 (capped 0x96) -> ~52 phantom side-A registrations (over-count
confirmed; b1a2/b21d load path double-counts or the .FSG exceeds the cap -- a load base-loss candidate).

FIRE GATE (FUN_0000_b51f, the 59 enemy 0x10 units): increments [param_2+0x19] each call; every 32 ticks,
per an 8-phase bitmask DAT_2000_5646[phase], if the phase-bit in [param_2+0x1b] is clear it accumulates a
position+offset (DAT_5c8b/5c8f) and, on CARRY, sets the bit and calls FUN_0000_c31e(0,10,param_1,DAT_5a25)
+ FUN_0000_ba49(0x9c15,...) -- the candidate fire/spawn.  param_2 is used as a RAW object pointer
(param_2+0x19/+0x1b/+0x1c) -> the base-loss surface: if param_2 is the raw near-offset (not g_mem+0x1c000+
off) the gate reads/writes garbage and never carries -> never fires.  NEXT SESSION: verify param_2's base
at the c0e5->b51f/902c dispatch (the object pointer register), migrate the fire-gate derefs (414/415/363
class) so the carry/bitmask math is faithful, and confirm firing + b2ef destroys against a correctly
re-anchored oracle.  This is the singular remaining frontier: per-update-method fire-gate base-loss.

## ORACLE-ANCHOR BLOCKER + refined hypothesis (2026-08-26)

Tried to compare the port's a294/a296 to the oracle's.  BLOCKED by the engine-under-paging anchoring:
  - The 16-bit engine runs under the extender's PM paging (cr3=0000e000, paging=1; confirmed in a live
    .cam.txt [seg] line).  At the 9200 render point DS is the extender's 4GB FLAT selector (dsb=10000000),
    NOT the engine DGROUP.  So an engine DGROUP field's guest-PHYS is a paged translation, NOT derivable
    by segment arithmetic, and the relocation differs per run.
  - Every fixed anchor missed: FIST_WATCHPHYS=0x3b424 hit a TEXT buffer (ASCII 'H'/'I'/'J' written by
    bc88, a string parser); 0x3aab4 (derived from a prior run's cs=2082) saw only 2 init-zeroing writes
    across the whole 90s mission -> not a294.  The prior session's registry anchor 0x31190/0x3b14c does
    NOT hold this run.
  - FIST_WATCHFLAT is CR3-aware (the correct tool) but its histogram is emitted only in the final
    fist_dump(), which needs SIGUSR2 -- broken in this env.  SIGUSR2 .ram.bin also does not fire here.
  UNBLOCK PATHS (bounded, next session): (a) build a per-run registry-signature locator -- scan a broad
  FIST_WATCHPHYS window during load for the deterministic roster slot values (c05c,c157,a022,...) to fix
  the engine DAT_2000 phys THIS run, then a294 = base+0xa294; or (b) make FIST_WATCHFLAT emit live (patch
  the DOSBox to append its flat-watch hits to .watch.txt per-write like WATCHPHYS does), then watch the
  engine-flat linear 0x2a294 directly.  Either gives the tick-by-tick a294 the goal's transaction-log
  comparison needs.

REFINED LEADING HYPOTHESIS for "units move but never fire" (port defect, unproven): the FIST_SIMRUN
present-complete stub (native_main.c:732) only ORs bit7 into d548 -- it runs NONE of the extender's
per-frame 32-bit flight-model work.  Unit MOVEMENT is pure FIST.DAT velocity integration (needs no
extender) and works (fp evolves).  Unit TARGETING (acquire enemy in range/LOS in the 3D voxel world) may
require the extender's per-frame world/geometry computation, which SIMRUN starves -> the mobile-unit AI
(902c/97d5/87df/7c1d -> shared a9ea/a0a4 targeting) never acquires a target -> never fires -> b2ef=0.
If confirmed, the fix is the goal's core clause: implement the extender per-frame flight/combat model
faithfully (not a ready-bit stub).  To confirm/refute WITHOUT the oracle: trace the mobile-unit AI
targeting to the point it bails (no-target), and check whether the missing datum is extender-computed.
Also testing an anchor-free A-vs-B signal now: oracle windshield inter-frame diff over the 90s mission
(static standoff => tiny diffs; active combat => large diffs).

## REFRAME (the session's key finding): win = GOALS REMAINING, self-play = AUTO CONTROL (2026-08-26)

Anchor-free A-vs-B experiment (oracle windshield inter-frame MAE across the 90s AZER1 mission, no memory
anchoring): MAE = 2460..12330 (peak ~19% pixel change/frame), all frames confirmed windshield (17-20KB).
=> the ORACLE IS HIGHLY DYNAMIC with the SAME passive input.  Rules out "the real game needs player action"
(hypothesis B).  The port's total static no-combat is a BUG (path A); the goal IS achievable.

The oracle frames (saved: ref/oracle_azer1_selfplay/f_01.png gunner view, f_07.png driver view) show the
GROUND TRUTH that corrects the goal's own framing:
  - HUD reads "GOALS REMAINING: 13" -> the WIN/LOSE condition is GOALS -> 0 (destroy the 13 goal objects),
    NOT a294/a296 unit-elimination.  I was measuring the WRONG metric all along.
  - "AUTO CONTROL" panel + view cycling "PL:1 UN:1" -> the player's platoon is on AUTOPILOT (AI-driven).
    THIS is why the mission self-plays with EMPTY input: the tank drives + engages under auto-control.
  - radar shows red enemy blips (targets tracked); the tank drives through terrain (big MAE = camera moving
    because the auto-controlled tank is MOVING).
  - strings confirmed in FIST.DAT: "GOALS:"@img0x246cc, "AUTO"@0x248f0, "AUTO TURRET CONTROL ENABLED/
    DISABLED"@0x2ee6e/0x2ee8b -> goals + auto-control are DECOMPILED ENGINE features (reachable).

PORT MATCHES AT SPAWN: FIST_SIMTRACE now counts goal-flagged objects (obj+0x17 & 0x08): the port loads
goals=13 -- EXACTLY the oracle's "GOALS REMAINING: 13".  Roster is faithful.  But goals stays 13 forever
(no combat destroys them) because the auto-controlled tank never drives/engages.

REVISED FIX PATH (supersedes the a294/side-count framing): the self-play needs the AUTO-CONTROL tank to
DRIVE toward + ENGAGE the goals so goals: 13 -> 0.  NEXT SESSION, in order:
  1. Determine if the port's player tank is actually IN auto-control mode on mission entry (find the
     auto-control state flag; the real game defaults to it for the platoon).  If not set, that alone may be
     why it sits idle.
  2. If in auto-control but idle: the auto-control NAVIGATION (path to goals) + FIRING likely needs the
     extender's per-frame world model (voxel terrain height/LOS for driving + targeting) that the
     FIST_SIMRUN present-stub starves -> implement the extender per-frame flight/combat step faithfully
     (the goal's core clause), driving goals->0.
  3. Track goals (not a294) as the win metric; mission resolves at goals==0.
The win-metric correction + AUTO-CONTROL mechanism + A-confirmation (oracle is dynamic) are the durable
gains; the goal (self-playing win/lose, byte-identical native==wasm) remains UNMET.

## SHARPENED ROOT: mobile-unit position is base-loss-corrupted (2026-08-26)

Tracked the player tank (registry idx 0, slot c05c, t=0) position (obj+4/+8 = 32-bit X/Y; verified vs the
spawn dumpreg: X=0x039cea2c Y=0x119172a1) across 56000 ticks under FIST_SIMRUN + setarch -R:
  t=314   X=60615212    Y=294744737     (= spawn 0x039cea2c / 0x119172a1, plausible world coords)
  t=8000  X=1981957015  Y=868406021
  t=16000 X=110378239   Y=-564112123
  t=24000 X=1962079803  Y=-2012630267
  t=32000 X=-1191408201 Y=817818885   ... (wild, sign-flipping, GB-scale jumps)
A driving tank moves SMOOTHLY (small per-tick deltas).  These are random 32-bit jumps -> the mobile-unit
MOVEMENT INTEGRATION is corrupted: the AUTO-CONTROL AI integrates position += velocity/heading, and a
base-lost velocity/heading field (read from the raw near-offset instead of g_mem+0x1c000+off) makes
position explode to garbage.  So the tank never coherently navigates to the goals -> never engages ->
goals stuck at 13, b2ef=0.  The wild state also defeats targeting (no valid range/LOS) -> no fire.  ONE
root, the 414/415/363 base-loss class, in the MOBILE-UNIT update subtree (902c/7c1d + shared movement
callees 912d/90cd/9176/875f/a358 and targeting a9ea/a0a4).

NEXT SESSION (concrete, ordered): (1) read FUN_0000_902c's movement callees (912d/90cd/9176 = the
position/velocity integrators) and asm-verify which obj-field derefs are base-lost (raw param_4+off vs
g_mem+0x1c000+off); (2) migrate them (patches/NNN, 363/414/415 idiom) so the player position integrates
SMOOTHLY; verify via FIST_SIMTRACE that X/Y move in small deltas; (3) then targeting/fire should follow
(goals begin to fall); track goals->0 as the win.  The oracle windshield (ref/oracle_azer1_selfplay/)
is the visual truth: auto-control tank drives + "GOALS REMAINING" counts down.

## BREAKTHROUGH: movement width-bug FIXED (patch 416) + fire blocker CONFIRMED (a294 guard) (2026-08-26)

ROOT of "units move but don't fire", found by asm-verification and two instrumented runs:

1. MOVEMENT (FIXED, patch 416): the 4 mobile-unit integrators (7cd5/88e4/912d/98c3) do
   `pos[+4/+8] += (long)*(int*)(obj+0x59/0x5b)`.  ASM is `movsx eax,WORD PTR [di+0x59]` -- a 16-bit velX,
   16-bit velY at adjacent 0x59/0x5b.  ghidra_compat doctrine: int=32-bit.  So `*(int*)` read 32 bits =
   velX|(velY<<16) = GARBAGE velocity -> tanks teleported to +-billions.  Fix: `*(int*)`->`*(short*)`
   (16-bit; the `(long)` keeps the sign-extend).  VERIFIED: post-fix the player Y integrates SMOOTHLY and
   monotonically (1.1M -> -443M over 60000 ticks, steady deltas) instead of random jumps.  Tanks now DRIVE.

2. FIRE BLOCKER (confirmed, not yet fixed): instrumented b1d6/b1df (the runtime object/projectile
   spawners).  b1d6 is called HUNDREDS of times during the sim (units ARE trying to fire), but EVERY call
   after load bails on b1d6's `DAT_2000_a294 < 0x78` capacity guard because a294=150 (>=120).  So no
   projectile can spawn -> no hits -> b2ef=0 -> goals stuck at 13.
   WHY a294=150: a296=16 == live side-B (consistent), but a294=150 vs 98 live side-A -> side-A over-counts
   by 52.  Since b2ef (despawn/destroy) fires 0x, every spawned object (early b1df effects) accumulates and
   NEVER despawns -> a294 leaks monotonically to the 0x96 cap -> the guard then blocks ALL weapon spawns.
   THE REAL BLOCKER IS THEREFORE "objects never despawn": b2ef is never dispatched, so a294 can only climb.

NEXT (ordered, tractable): find why timed objects never despawn (never dispatch b2ef).  Prime suspect:
the SAME width-bug class in the effect/projectile UPDATE methods (b5e7/b60f/b808/b93f/b945/b998 -- they
carry the most `(long)*(int*)` sites AND lifetime/timer counters like `*(int*)(param_5+0x2d)`).  If a
16-bit lifetime is read as 32-bit it never hits its expiry compare -> the object lives forever -> a294
leaks.  Asm-verify each `(long)*(int*)`/`*(int*)` timer deref in that cluster (movsx WORD => *(short*)),
migrate, and confirm: b2ef starts firing -> a294 falls below 0x78 -> b1d6 spawns projectiles -> goals
begin to fall.  36 `(long)*(int*)` sites remain (b945:6, b998/b93f/b808/b60f/b5e7:3 each, a0a4/a0ab:2 ...);
this is a bounded asm-verified width-sweep, the 416 idiom generalized.  Win metric = goals -> 0.

## a294-LEAK ROOT: registry DUPLICATION (confirmed at spawn) (2026-08-26)

FIST_DUMP_REG at first-in-mission: 115 live registry entries but only 80 DISTINCT slot values -> 35
duplicates.  The SAME object is registered in many 9fbc slots: ad74 x10, a392 x8, a35b x6, a324 x6,
ab85 x5, a3c9 x5, ...  Side-A distinct = 64, yet a294=150.  So each duplicate registration bumps a294
(via b21d) -> a294 over-counts to the 0x96 cap -> b1d6's `a294<0x78` weapon-spawn guard blocks ALL firing.
The fire blocker is therefore DUPLICATE OBJECT REGISTRATION in the object-management path (b1a2/b1d6/b1df
placement into 9fbc[param_3*2] with colliding param_3, or an update method that re-registers its object
every frame), NOT a despawn/lifetime issue.

NEXT (ordered): (1) determine WHERE the duplicates are created -- instrument b1a2/b1d6/b1df to log
(param_3 slot, param_4 object) and find the colliding/repeating registration; likely a base-loss/width bug
in the slot-index (param_3) computation in d81e/d7e1 (load) or an update method re-adding its object.
(2) Fix it (363/414/415/416 idiom) so each object occupies ONE slot and a294 == distinct side-A count
(<0x78).  (3) Then b1d6's guard passes -> weapon spawns -> projectiles -> hits -> b2ef -> goals fall.
Track goals->0 as the win metric.  This is the last identified blocker between "tanks drive" (patch 416,
done) and "mission resolves".

SESSION SUMMARY (2026-08-26): went from wrong-metric/wrong-model to a landed fix + fully-traced blocker.
  - Corrected the win metric: GOALS REMAINING->0 (port loads goals=13 == oracle), NOT a294/a296.
  - Corrected the mechanism: self-play = AUTO CONTROL autopilot; oracle is dynamic with empty input
    (frame MAE<=12330) => the port's static behaviour is a BUG, goal achievable.
  - LANDED patch 416: mobile-unit velocity read width int->short (asm movsx WORD) -> tanks now DRIVE
    (position integrates smoothly vs prior random +-billions).  asm-verified 4 sites.
  - TRACED the fire blocker end to end: duplicate registration -> a294 leaks to 0x96 cap -> b1d6 guard
    blocks every weapon spawn -> b2ef=0 -> goals stuck at 13.
  GOAL STILL UNMET (mission does not yet resolve); the remaining chain is one bounded fix (dedupe the
  registry so a294 falls below 0x78), then verify goals->0 and native==wasm.
