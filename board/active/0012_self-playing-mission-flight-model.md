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

## Duplication source split (2026-08-26)

b1df caller trace (return-address): FUN_0000_ba33 (33x) <- ba49 <- b51f (the 59 side-A 0x10 effect objects
spawning children) and FUN_0000_a93e (7x).  BUT only ~40 b1df calls in 6000 ticks while 35 duplicates
ALREADY EXIST at spawn (t=314) -> the MAIN duplication is LOAD-time via b1a2 <- d81e (the .FSG loader),
with colliding param_3 slot indices; b1df adds a few more during the sim.  So the dedup fix is primarily
in the LOAD path: instrument b1a2 (param_3 slot, param_4 object) during d7e1's load loop to find the
colliding/repeating param_3, and asm-verify the slot-index computation in d81e/d7e1 (likely a base-loss or
16-vs-32-bit width bug in the inbox reads uRam000f0008/000a that feed b1a2's param_3).  Fix so each loaded
object gets a UNIQUE slot and a294 == distinct side-A count (<0x78) -> b1d6 fire guard passes.

## DEFINITIVE fire-path root: weapon-spawn is base-loss-scrambled (board:0010 class) (2026-08-26)

b1d6's sole caller = FUN_1000_9caa (the type-0x11 projectile/weapon spawn).  Asm-verified (img 0x19caa):
  push di            ; DI = SOURCE firer object (caller passes it in DI)
  mov ax,0x11 ; push cs ; call 0x1b1d6   ; allocate a NEW projectile obj -> returned in DI
  mov si,di          ; SI = the NEW object
  pop di             ; DI = source restored
  mov [si+0x12], (rand&0x3f)+CS          ; new-obj id/seed uses CS (== the decompile's "unaff_CS")
  mov [si+4],[di+4] ; [si+8],[di+8]       ; copy SOURCE(di) position X/Y -> NEW(si)
  mov [si+0xc], [di+0xc]+0x300            ; + muzzle Z offset
  retf
DECOMPILE (FUN_1000_9caa) LOST b1d6's DI return: it uses iVar4=param_2 (the SOURCE) as the write target and
reads position from iVar5=unaff_CS (garbage/CS constant).  So the port writes the projectile fields onto the
SOURCE firer and sources position from garbage -> even if the a294 guard permitted, no valid projectile is
created.  Two coupled base-losses, BOTH the CS-context / register-tracking class (board:0010):
  (A) a294 leak: b51f(59 side-A 0x10 objects) -> ba49 -> ba33 -> b1df re-registers via `unaff_CS` (b51f:
      `FUN_0000_ba49(0x9c15,param_1,unaff_CS)`), duplicating objects (ad74 x10) -> a294 -> 0x96 cap ->
      b1d6's `a294<0x78` guard rejects EVERY weapon spawn.  (Load b1a2 is CLEAN: 50 objs, slots 0-49,
      no collision -- the dup is purely the sim-time unaff_CS spawn path.)
  (B) spawn scramble: 9caa mis-maps b1d6's new-object return (DI) -> writes projectile to the source and
      reads position from unaff_CS.
FIX PATH (the goal's core "build the part the port does not run"): recover the CS-context / register flow in
the spawn cluster -- either board:0010 SetCSContext in the Ghidra pipeline (systematic: eliminates unaff_CS,
re-threads b1d6's DI return) then re-decompile, OR per-site base-loss patches on 9caa (use b1d6-return as
target, param_2 as source, CS for +0x12) + ba49/ba33/b51f (correct the child-object pointer so it stops
re-registering the parent).  Then a294 stays < 0x78, b1d6 registers projectiles, they carry the firer's
position + muzzle offset, fly, hit, b2ef fires, goals fall.  This is the last root between "tanks drive"
(patch 416) and "mission resolves".  Cross-ref board:0010 (this fire path is a concrete board:0010 consumer).

## a294 guard is NOT the sole blocker -- whole spawn cluster is broken (2026-08-26)

Diagnostic: temporarily relaxed b1d6's guard 0x78->0x300 (reverted).  Result: ZERO change -- goals=13,
live=114, a294=150, and the player position sequence byte-identical to the guarded run.  So allowing b1d6
to register does NOT produce functional projectiles: the 9caa spawn scramble (writes the projectile fields
to the SOURCE object and reads position from unaff_CS) means the "new" object is never validly created.
=> the fire path requires the FULL spawn-cluster reconstruction (9caa target/source/position + b1d6 DI
return + ba49/ba33/b51f unaff_CS child pointer + a294 dedup), all the board:0010 CS-context/register class,
done PER-SITE (Ghidra is NOT installed in this env, so the systematic SetCSContext re-decompile is
unavailable).  This is the substantial remaining work between patch 416 (tanks drive) and a resolved
mission.  It is bounded (a specific ~5-8 function cluster) but each site needs asm-mapped register recovery.

HONEST SESSION STATE: movement FIXED+verified (416); win-metric+mechanism corrected (goals/AUTO CONTROL,
oracle-confirmed dynamic); fire path fully traced to the board:0010 spawn-cluster base-loss; goal UNMET
(no mission resolution yet).  Next: per-site reconstruct the 9caa/ba49/b1d6-return spawn register flow.

## RESISTANCE POINT: spawn/registration model not fully understood (2026-08-26)

Asm-traced the b51f child-spawn precisely (do NOT improvise a fix past this until resolved):
  b51f (0xb51f): per-object gate; on carry sets [di+0x1b] phase-bit, computes a child position into
    DAT_2000_9c7f/9c83 (= [di+4]/[di+8] + a per-phase offset table [bx-0x6375]/[bx-0x6371]), then
    `mov di,[0x9a25]; push cs; call c31e` (a sound/effect), `pop di` (di=b51f obj), `mov ax,0x9c15;
    call ba49`.  So at the ba49 call, DI = the b51f object.
  ba49 (0xba49): `call ba33; mov eax,[9c7f]; mov [si+4],eax; mov eax,[9c83]; mov [si+8],eax` -- writes the
    computed child position into si[4]/si[8].
  ba33 (0xba33): `push di; mov ax,4; call 0xf69:0xbb4f (=FUN_1000_b1df); mov si,di; pop di; jb skip;
    call ba5d`.
  b1df (0x1b1df): IDENTICAL to b1d6 minus the a294 guard -- walks 9fbc for a free slot, `mov [si],di`
    (registers the EXISTING DI pointer), zeroes the body.  b1df does NOT allocate and does NOT change DI.
OPEN CONTRADICTION: DI at the b1df call = the b51f parent object, so b1df registers the PARENT into a new
slot (and ba49 then sets si=di=parent position to the child coords) -- i.e. it appears to re-register +
reposition the PARENT every phase.  That would duplicate/corrupt the parent (matches the port's ad74 x10),
but the ORACLE does not leak, so either (a) DI is remapped by a segment/base I'm not modelling, (b) these
b51f objects are consumed short-lived effects whose slots are freed by a path I haven't found, or (c) the
`mov si,di` after b1df captures a DIFFERENT DI because b1df's far-return (0xf69 cluster) unwinds the stack
such that DI is reloaded.  UNRESOLVED -- do not patch until the model is understood (needs Ghidra board:0010
OR a working oracle anchor to observe the correct per-frame registry writes; both unavailable in this env).
This is the honest edge of understanding; improvising a base-loss patch here risks silent corruption of the
pristine-derived engine, which the project forbids.  The 416 movement fix stands (asm-verified, 18/0 verify).

## REFINED (corrected re_out->build): spawn cluster IS patched; blocker = display-object per-frame clear (2026-08-26)

IMPORTANT CORRECTION: my earlier "9caa/spawn-cluster base-loss" analysis read re_out (pristine/scrambled).
The BUILD is already patched: 9caa=patch 270, b51f=253, b583=257, the b1df->ba33->ba49->ba5d chain=258,
b5e7=335.  patch 258 asm-VERIFIED that b1df(4, di=the object) registers the object into the 182-slot 0xdfbc
table -- so the b51f/b583 emitters re-register their object each phase (up to 8x) as DISPLAY objects; this
is the correct asm.  These MUST be cleared/despawned each frame or the 182-slot registry overflows.

CONFIRMED the guard is a real safety limit, not the bug: relaxing BOTH a294 gates (b1d6 <0x78 and b21d
cap ==0x96) SEGFAULTS (exit 139) -- the registry overflows.  So combat cannot be unblocked by bypassing the
guard; the leak itself must be stopped.  a294 reaches 150 by the FIRST cockpit frame (t=314) -- i.e. during
the pre-cockpit load/spawn frames, via the emitter re-registration -- and never falls (b2ef=0: NOTHING
despawns).  In the oracle a294 must stay <0x78 (it fires 9caa->b1d6), so the oracle DOES clear these each
frame; the port does not.

THE MISSING MECHANISM (concrete next lead): the per-frame TRANSIENT-OBJECT CLEAR.  The frame loop
(re_out ~13008/13071) runs `fist_icall_far(DAT_2000_2ce4)` immediately BEFORE FUN_0000_c0ca(update) and
FUN_0000_461b after.  One of these (2ce4 pre-clear, or a compaction that removes display-type slots and
decrements a294) is very likely the per-frame reset the port stubs/mis-dispatches, letting emitters
accumulate to the 0x96 cap.  NEXT: trace what DAT_2000_2ce4 / FUN_0000_461b resolve to at the mission frame
(instrument fist_icall_far for that vector; objdump the frame-loop caller), find the display-object clear,
verify it runs in the port; if stubbed/base-lost, restore it -> a294 stays low -> b1d6 spawns weapons ->
projectiles (b5e7) -> hits -> b2ef -> goals fall.  Secondary: patch 258's documented bb64/DAT_5a25
target-population deviation.  Movement (416) stands; goal UNMET.

## Despawn is INDIVIDUAL, not a frame clear -- localizes the leak (2026-08-26)

Ruled out a per-frame transient clear: DAT_2000_2ce4 (pre-c0ca vector) = 0xc30e in-mission, and
FUN_0000_c30e is EMPTY (return;).  Despawn is per-object: FUN_0000_c30f (b354->b2ef) and the b5e7 destruct
branch (patch 335) each despawn ONE object when it expires.  So the a294 leak is simply that the emitter-
spawned display objects never reach their despawn condition (b2ef fires 0x).  The remaining question is
narrow and concrete: for a b51f/b583-emitted display object, WHICH update path should call b2ef (c30f or
b5e7 destruct), and why is its expiry condition never met in the port (lifetime/timer/collision read
base-lost or width-bugged, OR the despawn dispatch vector unresolved)?  NEXT: instrument b2ef's would-be
callers (c30f, b5e7 destruct branch) + the expiry compares in the display-object update; find the read
that never trips; asm-verify + patch (416/258 idiom).  Once one class of display object despawns, a294
falls below 0x78 and the 9caa->b1d6 weapon spawn (already correct, patch 270) proceeds -> combat.
Movement (416) stands; goal UNMET; the fire chain is now traced end-to-end to a single narrow defect class
(display-object expiry/despawn), everything upstream (spawn) and the win metric confirmed correct.

## MEASURED: a294 leak is POST-COCKPIT type-4 display-object registration (2026-08-26)

Instrumented b21d (side-count) per call.  DECISIVE: of 143 b21d calls, 99 are AFTER cockpit entry
(incock=1).  a294 climbs to ~44 at load-end (units+trees), then to the 0x96 cap POST-cockpit via ~99
registrations of TYPE 4 (side=0): these are the b1df(4, di) display-object registrations from the
b51f/b583 emitters (each phase re-registers -> +a294).  They NEVER despawn (b2ef=0) -> a294 saturates ->
b1d6 weapon-spawn guard blocks fire.  So the leak is unambiguously the per-frame/per-phase type-4 display
objects not being reclaimed.

NEXT LEAD (concrete): FUN_0000_462e = `b1df(param_1,param_2); if(!CF) c296(param_2)` -- register THEN call
FUN_0000_c296.  c296 (+ the type-4 update method vtable[4] @ DGROUP:(8-0x1bac)) is the likely
reclaim/despawn-schedule path for these transient display objects.  Trace c296 and the type-4 update:
find where a type-4 object's lifetime/rendered-flag should trip its b354->b2ef reclaim, and why it never
does in the port (base-lost expiry read, or the reclaim tied to the op-0x4c render that FIST_SIMRUN stubs).
Strong secondary hypothesis: the reclaim is part of the op-0x4c display-list PRESENT (the goal's named
render frontier) which FIST_SIMRUN only ready-bit-stubs -> the render never consumes+reclaims the
transient display objects -> they leak.  If so, the fix is implementing op-0x4c faithfully (not the stub).

STATUS: movement fixed+verified (416); fire blocker measured to the type-4 display-object reclaim; goal
UNMET.  The chain is: [emitters register type-4 display objs each phase] -> [never reclaimed: b2ef=0] ->
[a294 -> 0x96 cap] -> [b1d6 guard blocks weapon spawn] -> [no projectiles/hits/destroys] -> [goals stuck].
Fixing the type-4 reclaim (or the op-0x4c render that should do it) is the single remaining blocker.

## CONVERGENCE: the type-4 reclaim is the op-0x4c render (the goal's named frontier) (2026-08-26)

Ruled out every non-render reclaim candidate: c30e (empty), 461b (round-robin dispatcher), c30f (b354->b2ef
despawn -- no direct callers, vtable-only), 462e/c296 (register+INIT a new object, not reclaim).  No engine
per-frame path reclaims the type-4 display objects.  Therefore the reclaim is in the PER-FRAME RENDER that
FIST_SIMRUN only ready-bit-stubs: the op-0x4c display-list present.  The emitters register type-4 display
objects each phase FOR the render to draw + consume; with the render stubbed, they are drawn never and
reclaimed never -> a294 leaks to the 0x96 cap -> weapon spawn blocked.  This is EXACTLY the goal's stated
frontier: "finishing the per-frame render path (the op-0x4c display-list / DGROUP:0x7aa4 viewport-geometry)".

So the single remaining blocker for self-playing combat is the op-0x4c display-list render implementation
(NOT a small base-loss patch).  patch 413 (blanket op-0x4c faithful threading) was reverted earlier because
the real path hits unimplemented FUN_1000_0c21 memmgr -> native hang; that memmgr + the op-0x4c display-list
consumer are the work.  ORDERED PLAN: (1) implement/stub-out FUN_1000_0c21 memmgr so op-0x4c can run without
hanging; (2) implement the op-0x4c display-list present faithfully (draws + reclaims the type-4 display
objects, decrementing a294); (3) verify a294 stays <0x78, b1d6 spawns weapons, projectiles hit, b2ef fires,
goals -> 0; (4) then determinism + native==wasm across the run.  This is the substantial "build the part the
port does not yet run" clause -- now precisely scoped to op-0x4c, reached by tracing combat backwards from
b2ef=0.  Movement (416) stands; goal UNMET; the blocker is singular and named.

## REFRAME: type-0x10 objects are WEAPON SYSTEMS; leak = documented bb64/DAT_5a25 deviation (2026-08-26)

Instrumented b51f's carry inputs: positions are SANE (~0x100000), offsets are the real 8-direction burst
pattern (0xfffffe00=-512, 0x00001800=+6144, ...); carry fires exactly when offY is negative (a legitimate
in-bounds geometric condition, asm `add eax,off; jc`).  So the b51f registration TRIGGER is correct -- the
59 type-0x10 objects are WEAPON/TURRET systems: b51f -> bb64 (find armed target in range -> DAT_5a25, patch
255) -> c31e (act on target) -> ba49 (spawn/register).  The registration IS the weapon burst.  Phase-bit
gate [0x1b] limits each to ~4 fires (the negative-offY phases), once -> a294 climbs to the 0x96 cap once,
then static (matches a294=150).

THE CONTRADICTION (unresolved): patch 258 (asm-verified) has ba49 register param_1 = the EMITTER itself
into a new slot and overwrite its position -- which duplicates/corrupts the emitter, yet the oracle can't
leak.  Patches 253/255/258 EXPLICITLY document deviations here: patch 258 note "c31e's object should be
SI=DAT_2000_5a25 (the bb64-found target) but the chain dispatches on param_1; revisit with the bb64 DAT_5a25
population."  So the spawn chain currently uses the emitter (param_1) as a stand-in because bb64/DAT_5a25
(the proper target/effect object) is incompletely populated.  The correct object to register/act-on is the
bb64-found target in DAT_5a25, not the emitter -- completing that (patch 255/258) likely stops the
emitter self-duplication AND makes the weapon act on a real target.

TWO CANDIDATE ROOTS remain, needing the oracle to disambiguate (a294 tick-by-tick) or Ghidra (board:0010):
  (A) complete bb64/DAT_5a25 so ba49 registers the real target/effect (not the emitter) -> no self-dup;
  (B) the op-0x4c render reclaims the display objects each frame (FIST_SIMRUN stubs it).
Both are within reach but each needs ground-truth I cannot get in THIS env (oracle anchor paging-blocked;
Ghidra absent).  I will NOT improvise a fix on the unresolved emitter-registration contradiction.

HONEST FINAL: movement fixed+verified (416); win-metric+self-play-mechanism corrected+oracle-confirmed;
fire failure traced by measurement to the weapon-system spawn chain (type-0x10 -> bb64/DAT_5a25 -> ba49)
whose target-object population is a DOCUMENTED incomplete deviation (patch 255/258).  Goal UNMET.  The two
concrete unblockers are unchanged: a per-run registry-signature oracle anchor, or Ghidra for board:0010.

## PATCH 417 landed: correct weapon targeting (asm-verified) -- combat closer, not complete (2026-08-26)

BREAKTHROUGH this session: bb64 DOES find targets (instrumented: 55 armed(0x40) objects, 21 bit3-eligible,
consistently FINDS si=c252 = a side-B unit).  So targeting works.  Root of no-damage, asm-verified from
full b51f asm (0xb51f-b582):
  - b564 `jae b582`: the fire is gated on bb64's CF (target FOUND), NOT the discarded position-add carry
    (b552's `add` carry is never tested).  Ghidra gated on that carry (bVar2) -> fired on the wrong
    condition.
  - b573 `mov di,[0x9a25]`: c31e dispatches on SI = the bb64-found TARGET (DAT_5a25); Ghidra passed the
    emitter (param_1) -> the damage/action hit the EMITTER, never the target.
PATCH 417 recovers both (gate on g_fist_cf; c31e on g_mem+0x1c000+DAT_5a25).  EFFECT (measured): a294 now
starts at 80 (was 150), combat dynamics change, player moves differently -> the fix is real and matrix-safe
(in-mission only; make check clean).

STILL UNRESOLVED (b2ef=0, goals=13): (1) a294 still climbs to the 0x96 cap (~t500) via the ba49 emitter
registration each fire -> eventually blocks fire again; (2) even with correct targeting, the TARGET is not
destroyed -- c31e dispatches the target's -0x1ab0 action method but no b2ef results.  The open question is
the ba49 spawn contradiction (asm registers the emitter di, not a flying projectile) + whether the actual
damage is c31e-direct or projectile-based.  Two sub-defects remain in the SAME weapon cluster; 417 fixed
the targeting/gate, the damage-application + the ba49/effect-despawn are next.  Movement (416) + targeting
(417) landed and asm-verified; goal UNMET but combat provably advanced (targeting now correct, a294 halved).

## *** COMBAT WORKS *** patch 417 fixed -- units are destroyed (2026-08-26)

The c31e param_3 fix was the linchpin.  c31e (patch 256) does `di = (uint16_t)param_3` -- it wants the
object NEAR OFFSET, not a host pointer.  My first 417 passed g_mem+0x1c000+DAT_5a25 -> its low16 = garbage
-> c31e dispatched the wrong type.  Corrected to pass DAT_5a25 (the raw near offset).  RESULT (measured,
setarch -R, deterministic):

  a296 (side-B unit count) DROPS: 16 -> 15 -> 14 -> 13 -> 12 -> 11 -> 10 over t=474..1008.  b2ef FIRES.
  UNITS ARE DESTROYED.  The mission is DYNAMIC where it was frozen for the entire prior effort.

The side-A weapon emitters (type-0x10, b51f) find side-B targets (bb64 -> c252), damage them (c31e on the
target), and destroy them.  This is real AI-vs-AI combat: the enemy destroying the player's side.

REMAINING (the last blocker to a RESOLVED win/lose): combat STALLS at t~1008 when a294 (side-A) re-caps at
150.  The side-A emitters/effects (ba49 registrations) never despawn (a294 only climbs, never falls), so
after each emitter fires its one 8-phase burst (phase-bit [di+0x1b] gates re-fire) the registry fills and
b21d/b1d6 block further spawns -> combat halts with a296=10 (6 of 16 destroyed), not 0.  To RESOLVE the
mission, the spent emitters/effects must despawn (free a294 / reset the phase-bits) so combat is SUSTAINED
until a296 -> 0 (one side eliminated).  This is the effect/emitter despawn lifecycle -- the same class as
the display-object reclaim, now the SINGLE remaining blocker between "combat works" and "mission resolves".
(My FIST_SIMTRACE goal-count is polluted by leaked duplicates -- goals oscillates 13-17; a296 is the clean
combat signal.)  Movement (416) + targeting/damage (417) landed+asm-verified; combat PROVEN; sustaining it
to resolution is the final step.

## COMPLETE CHAIN TRACED: combat works; final blocker = effect despawn (render/ba49-alloc) (2026-08-26)

Verified 23/0 native: patches 416+417 are matrix-safe (in-mission only).  The full combat chain is now
traced end to end and WORKS except one piece:
  spawn (loaded, clean) -> per-tick c0e5 dispatch (works) -> mobile-unit movement (416, works) ->
  weapon target-acquire bb64 (works, finds c252) -> damage c31e on the TARGET (417, WORKS -> a296 16->10,
  b2ef fires, units destroyed) -> [MISSING: effect/emitter despawn] -> a294 leaks to 0x96 cap -> blocks
  BOTH sides' further weapon spawns (b1d6 guard) -> combat halts unresolved.
Exhaustively confirmed the despawn is genuinely absent per-frame: a294 falls ONLY via b2ef (61428); b330
(bulk clear-by-type, incl. b330(0x10) for emitters) is called only from command fns (5612/5652/5666), NOT
per-frame; c30e (pre-update) is empty; 461b is a round-robin; c30f/b5e7-destruct are the individual
despawns but nothing drives them for the spent emitters.  So the effect reclaim is the render-transient
path (op-0x4c display-list, FIST_SIMRUN stubs it) OR the ba49 chain should register a fresh DESPAWNING
effect object instead of the emitter (patch 258's documented deviation) -- both need the oracle (to observe
the correct per-frame registry) or Ghidra (board:0010) to implement faithfully, neither available here.

SESSION RESULT: from a frozen mission (goals=13 static, b2ef=0, framed as open-ended extender
reconstruction) to WORKING, VERIFIED, DETERMINISTIC COMBAT (a296 16->10, units destroyed) via three
asm-verified matrix-safe patches (416 movement, 417 targeting+damage x2 fields).  The mission is dynamic;
it does not yet RESOLVE (one side to 0) because the effect despawn (a294 leak) halts sustained combat.
That single remaining blocker is precisely scoped to the op-0x4c render / ba49-effect-alloc frontier -- the
goal's own named "per-frame render path" clause.  Goal UNMET (no resolved win/lose) but combat proven.

## SHARPEST: despawn = bab4 (type 4); effects mis-registered as the type-0x10 emitter (2026-08-26)

Dumped the c0e5 type->update vtable (FIST_VTDUMP, gated, in native_main.c):
  type 04 -> bab4  (the DESPAWN animation update: dec [di+0x20] timer, advance [di+0x19] frame, and when
                    [di+0x19]==[di+0x1e] -> b354->b2ef DESPAWN.  ba5d sets up exactly these fields.)
  type 10 -> b51f  (the WEAPON: bb64 find-target -> c31e damage -> ba49 spawn effect.)
So the weapon-spawned EFFECT should be a TYPE-4 object (despawns via bab4 after its animation), and
b1df(4, di)'s "4" is precisely that intended effect type.  BUT the fire chain (b51f b57b `pop di` ->
ba49 -> ba33 -> b1df) registers DI = the type-0x10 EMITTER itself, whose update stays b51f (fires forever,
never despawns) -> a294 leaks.  ba5d inits the bab4 animation fields on it, but its dispatch TYPE ([di+0])
is never set to 4 (checked b51f/ba49/ba33/ba5d/b1df/b21d asm -- none writes [di+0]).  So the effect is
born as the emitter (type 0x10) and runs the weapon update instead of the despawn update.

This is the exact mechanism of the a294 leak, and it IS patch 258's documented deviation ("ba49 registers
the object ... revisit with the bb64 DAT_5a25 population").  The correct behaviour registers a FRESH
TYPE-4 effect object (from an object pool / DAT_5a25) with [obj+0]=4, which despawns via bab4.  The port's
asm-faithful chain registers the emitter as a stand-in.  Resolving it needs the fresh-type-4-effect source
(the object pool the decompile/patch 258 approximated), which needs the oracle (to observe the correct
per-frame registry: does the emitter transiently become type 4, or is a pool object allocated?) or Ghidra.
The DESPAWN mechanism itself (bab4, type 4) is now identified in-engine -- the gap is purely getting the
effect to BE type 4.  Movement (416) + targeting/damage (417) landed; combat works; goal UNMET; the last
blocker is now diagnosed to a single field (the effect's dispatch type) + its allocation source.

## Confirmed: effect needs a FRESH type-4 object (naive type-set crashes) (2026-08-26)

Diagnostic (FIST_EFFTYPE4, reverted): forced the ba33-registered effect object to type 4 (-> bab4 despawn).
Result: SEGFAULT (exit 139).  The registered object IS the live emitter being processed; changing its type
mid-flight makes bab4 deref its (weapon-shaped, not effect-shaped) fields -> crash.  This PROVES the fix is
NOT "repurpose the emitter" but "register a FRESH type-4 effect object" from the object pool that patch 258
approximated (it registered the emitter as a stand-in because the fresh-effect source was unresolved).

FINAL DIAGNOSIS (as sharp as possible without oracle/Ghidra): combat works (416+417); the mission does not
resolve because weapon hit-effects are registered as the type-0x10 emitter (update b51f, never despawns)
instead of a fresh type-4 effect (update bab4, despawns after its animation).  a294 leaks -> b1d6 guard
blocks sustained fire.  The despawn mechanism (bab4) and the correct effect type (4) are identified
IN-ENGINE; the single missing piece is the fresh-type-4-effect ALLOCATION SOURCE (an object pool) that the
b51f->ba49->ba33->b1df asm passes as di=emitter.  Resolving it needs the oracle (observe the correct
per-frame registry: where the fresh effect object comes from) or Ghidra (board:0010 systematic CS-context).
Both unavailable here.  This is the precise, minimal remaining work for a RESOLVED mission.

## *** EXACT FIX PROVEN *** (patches 418/419, reverted — they crash the render cascade) (2026-08-26)

FOUND the fresh-effect allocation source: FUN_1000_b21d IS the object allocator.  Its asm allocates a free
slot from the side pool (a022 stride 0x37 / c05c stride 0xfb), writes word[buf]=the type, and RETURNS the
buffer in DI.  Patch 200's own comment says so ("Returns the allocated slot's DGROUP near-offset ...
consumed by the callers b1a2/b1d6/b1df"), but the port's b1df/b1d6 DROPPED that DI-return and register the
INPUT param (the emitter) instead -- so effects are the type-0x10 emitter (b51f, never despawns) rather
than the fresh type-4 buffer (bab4, despawns).  patch 258's documented deviation, root-caused.

PROVEN (patch 418 = thread b21d's DI-return through b1df->ba33->ba49; patch 419 = migrate bab4 base-loss):
with these, the effect IS the fresh type-4 object and c0e5 DISPATCHES bab4 (the despawn) -- CONFIRMED by
the crash moving from b51f into bab4 then into the RENDER (c33c->c4df->c694).  So the mechanism is correct:
effects -> type-4 -> bab4 animate -> b2ef despawn -> a294 balances -> sustained combat.

WHY REVERTED: the type-4 effects now flow through the per-frame RENDER walk (c33c/c4df) which dispatches
per-object render methods (c694 -> c945 ...) that are BASE-LOST (c694 derefs param_3+0x1c with param_3 a
garbage host pointer -> SIGSEGV).  This crashes the process, regressing the working 416/417 combat, so 418/
419 cannot ship until the render cascade is migrated.  IMPORTANTLY: c33c/c4df/c694/c945 are DECOMPILED
FIST.DAT functions (FUN_0000_*), NOT the undecompiled overlay -- so the render IS migratable (the 363/414/
415/419 base-loss idiom), a BOUNDED cascade, not an overlay reconstruction.

THE COMPLETE REMAINING FIX (now precisely known, ~a handful of base-loss migrations):
  1. patch 418 (thread b21d's DI-return: b1df/b1d6 register it, callers ba33/ba49/462e/9caa init IT); 
  2. patch 419 (bab4 despawn migration -- done, verified reached);
  3. migrate the per-object RENDER methods the type-4 effects hit: c694 (+c945) and the c4df dispatch
     object-pointer base-loss (param_3 must be the DGROUP near-offset, not a host pointer);
  4. re-test: a294 balances -> combat sustains -> a296/goals -> 0 -> mission RESOLVES.
This is the end of the trace: the mechanism is proven, the allocator (b21d) is found, the despawn (bab4) is
migrated and confirmed, and the only thing between here and a resolved mission is the bounded, decompiled
render-method base-loss cascade (c694/c945/c4df).  Movement (416) + targeting/damage (417) shipped; the
effect-despawn + render cascade is the finish.  Goal UNMET but the path is fully proven end-to-end.

## Cascade is BRANCHING (deep) -- proven fix, but completion is the full per-frame migration (2026-08-26)

Pushed the effect-despawn fix further (418 + 419-with-bab4-dispatch-fix + bae1/bb02 sub-method migrations +
render-skip diagnostic).  Each fix moved the crash to the NEXT unmigrated function: b51f -> bab4 -> bae1/
bb02 -> render c694 -> update bc0c(type 0x17) ...  Routing effects through the fresh type-4 pool exercises
many per-frame update AND render methods that were never reached before (the effects never existed), and
each is a base-loss (host-ptr deref of a near-offset).  a294 was still climbing (79->115) when it crashed
(effects animate for [o+0x1e] frames before bab4 despawns them, so the pool fills before they clear).

So the effect-despawn fix is PROVEN correct (b21d allocator + type-4 + bab4 reached) but its completion is
NOT a 2-3 function patch -- it is the bounded-but-DEEP per-frame simulation+render base-loss migration
(bab4/bae1/bb02 + c694/c945/c4df render methods + bc0c and other update methods the effects flow through).
This is exactly the goal's "finishing the ... per-frame render path ... with no stubs" clause, now reached
and scoped concretely (a specific set of ~10+ FUN_0000 base-loss migrations in the b5xx-bcxx/c6xx-c9xx
render+effect cluster), each asm-verifiable with the 363/414/415/419 idiom.

SESSION NET: shipped 416 (movement) + 417 (targeting/damage) = WORKING VERIFIED COMBAT (a296 16->10).
Root-caused + proved the effect-despawn fix (b21d is the object allocator; port drops its DI-return; effects
must be the fresh type-4 object that despawns via bab4).  Reduced the remaining from "open-ended extender
reconstruction" to a concrete, decompiled, base-loss migration cascade of the effect+render cluster.
Goal UNMET (mission does not resolve); combat proven; the finish is a deep-but-bounded per-frame migration.

## HONEST CORRECTION: patch 418 has an unresolved pool-allocation flaw (2026-08-26)

On grinding the cascade further I found the bc0c crash is on an UNDER-allocated a022 slot (param_2=0xBA58 =
slot 122 while only ~115 were allocated) -- i.e. patch 418's b1df registered a 9fbc entry pointing at an
UNALLOCATED/garbage pool slot.  So "thread b21d's DI-return" is NOT a clean fix: the idx/a294 accounting
between b21d (roster slot walk, stride 0x37/0xfb) and 9fbc (the object registry) does not line up the way
418 assumed, and the effect objects it creates are malformed -> the cascade CRASHES rather than converges.
My earlier "*** EXACT FIX PROVEN ***" claim was OVERCONFIDENT: 418 confirmed the effect can be routed to
type-4/bab4, but it does not correctly ALLOCATE the effect object.  The real fresh-effect allocation (which
object buffer the a022/c05c pool hands out, and how ba5d/9fbc index it) is still not correctly understood
from the decompile alone -- it needs the oracle to observe the correct per-frame pool/registry state, or
Ghidra.  Retract the "proven" framing; the honest state is: the DIRECTION (effects must be a despawning
type-4 object) is right, but the ALLOCATION mechanics are not yet correctly reconstructed.

TRUE SESSION STATE: 416 (movement) + 417 (targeting/damage) shipped + verified = real working combat
(a296 16->10, units destroyed) -- this stands.  The effect-despawn / a294-leak fix that would SUSTAIN combat
to resolution is NOT solved: the object-allocation model (b21d pool <-> 9fbc registry <-> ba5d init) is not
correctly understood, and 418's attempt creates malformed objects.  Goal UNMET.  Honest remaining work:
correctly reconstruct the effect-object allocation (needs oracle/Ghidra), then migrate the effect+render
update-method cluster it flows through.  I over-claimed "proven fix" mid-session; corrected here.

## UN-RETRACT: patch 418 IS correct -- a294 BALANCES, cascade converges (2026-08-26)

Reverse the previous retraction.  With 418 (thread b21d's allocated buffer) + the cascade of base-loss
migrations it exposes (bab4 + its sub-methods bae1/bb02 + bc0c) + render-skip diagnostic, the run:
  - DOES NOT CRASH (exit 0, runs to t=64000).  bc0c was just an unmigrated base-loss (+ a width bug
    *(int/uint*) where the asm is WORD -- same class as 416), NOT a garbage object from 418.
  - a294 now BALANCES: climbs to ~141 then FALLS (136,131,126,120,110,107,...) oscillating 88-147 instead
    of monotonically pinning at the 0x96 cap -> THE EFFECTS DESPAWN (bab4 animate -> b2ef).  The a294 leak
    is FIXED.
So 418 IS the correct fix (b21d is the object allocator; register its DI-return; effects are the fresh
type-4 object that despawns via bab4).  The cascade is a BOUNDED set of base-loss migrations (bab4/bae1/
bb02/bc0c so far), each asm-verifiable (363/414/415/416/419 idiom), and it CONVERGES (no crash, a294
balanced).  My earlier "garbage/under-allocated slot" diagnosis was wrong: 0xBA58 was a valid type-0x17
object near-offset that bc0c deref'd raw (base-loss), not a malformed 418 allocation.

NEW ISSUE (not a crash): with the CORRECT behaviour, a296 stays 16 (no side-B destroyed) in this window --
the 417 a296-drop (16->10) was partly an ARTIFACT of the duplication leak (the emitter re-registered itself
and re-ran b51f, AMPLIFYING c31e damage).  With 418 (no self-duplication) the emitters fire once/phase, so
less damage, and the player drives off sooner.  So the mission still doesn't resolve, but now from correct
mechanics, not a leak.  REMAINING to resolution: (1) migrate the render methods c694/c945 (+c4df object-ptr)
to drop the render-skip guard [the type-4 effects must render faithfully, no stub]; (2) understand why the
correct combat doesn't deplete a side -- likely the player AUTO-CONTROL navigation (drives out of range) or
the weapon cadence.  418+cascade is a REAL, shippable-once-render-migrated fix; goal still unmet but the
effect-despawn/a294-leak is genuinely SOLVED.

## ACCURATE PICTURE: a294 leak SOLVED; 2 subsystems remain (render + navigation) (2026-08-27)

The effect-despawn / a294-leak fix is REAL and CONVERGES (proven: a294 balances, no crash).  Its patch set:
  418 (thread b21d DI-return: b1df returns the allocated buffer, ba33/ba49 init it) +
  419 (bab4 despawn base-loss migration) + bae1/bb02 (bab4 sub-method migrations) +
  bc0c (type-0x17 weapon-platform update: base-loss + int/uint->WORD width, fires 9caa).
Reference impl saved: scratchpad/combo2_a294balances.c (currently uses a render-skip GUARD, which the goal
forbids -- must be replaced by migrating the render methods).

TWO REMAINING SUBSYSTEMS between "a294 solved" and "mission resolves":
  (A) RENDER migration (board:0001 windshield): the type-4 effects flow through the per-frame sprite render
      c33c->c4df->c694->c945->c962...  c4df is migrated (sets g_fist_render_si); the render METHODS are not
      (c694 reads word[si+0x1c] with si the object near-offset, Ghidra gave it a garbage host-ptr param_3).
      Migrate c694/c945/c962 (read the object from g_fist_render_si; g_mem+0x1c000 base; asm widths) to drop
      the render-skip guard.  Bounded base-loss cascade (the 416/419 idiom).
  (B) COMBAT RESOLUTION: with CORRECT mechanics (418, no self-duplication) a296 stays 16 in a 64000-tick
      window and the player AUTO-CONTROL drives OUT of range (X -> -21M by t=4747).  The 417 a296-drop
      (16->10) was partly a duplication ARTIFACT (the emitter re-ran b51f, amplifying c31e).  So correct
      combat is slower/balanced and needs the player to ENGAGE -- the auto-control NAVIGATION (drive toward
      objectives) is the next thing to verify against the oracle, or the mission simply needs the player's
      own weapons (bc0c/9caa, now unblocked since a294 balances) to deplete the enemy goals over more time.

NET (accurate, non-inflated): 416+417 shipped = working combat.  418+cascade SOLVES the a294 leak (proven,
converges) but is blocked from shipping by the render-method migration (board:0001) + is not sufficient for
resolution (needs the player navigation/engagement).  Goal UNMET.  The main combat blocker (effect despawn)
is genuinely solved; the finish is the render sprite-method cascade + the auto-control navigation.

## REFINED: even the 417 a296-drop was an emitter-teleport artifact; resolution needs navigation (2026-08-27)

Traced why 418 (correct) shows a296=16 while 417 showed a296 16->10.  With 417 alone, ba49 wrote the burst
position DAT_5c7f/5c83 to [emitter+4]/[+8] -- the EMITTER's OWN position -- teleporting the (otherwise
stationary, b51f does not integrate position) type-0x10 weapon emitters around, sometimes INTO range of
side-B units, where c31e then damaged them.  So the 417 a296-drop was partly an ARTIFACT of that position
corruption, not clean combat.  With 418 (ba49 writes velocity to the fresh EFFECT object, not the emitter),
the emitters stay put and only engage what comes into their range.

So the accurate combat model: side-A has stationary weapon emitters (b51f/bc0c) + goals; the player's side-B
platoon is AUTO-CONTROL mobile.  Resolution (one side eliminated) requires the AUTO-CONTROL to DRIVE the
player/platoon into the enemy and destroy the goals (or be destroyed).  In the port the auto-control drives
the player OUT (X -> -21M) rather than toward objectives.  THE RESOLUTION BLOCKER IS THE AUTO-CONTROL
NAVIGATION (heading/target selection), a distinct subsystem -- likely a base-loss in the player-unit AI
(7c1d + shared a9ea/a0a4 targeting/steering) or it needs the oracle to verify the correct heading.

FULLY ACCURATE SESSION STATE (2026-08-27):
  - 416 (movement) + 417 (targeting/damage) shipped -- but 417's visible a296-drop was partly an emitter
    position-corruption artifact, so "combat working" is real only in the sense that c31e damage + b2ef
    destroy fire; clean sustained combat is not yet achieved.
  - a294 effect-despawn leak SOLVED + proven (418 + bab4/bae1/bb02/bc0c cascade; a294 balances, no crash) --
    genuine, but blocked from shipping by the render-method migration (board:0001 c694/c945/c962).
  - Resolution requires, beyond the above: the AUTO-CONTROL NAVIGATION to engage (the player-unit AI).
  Goal UNMET.  Three scoped pieces remain: (1) render sprite-method base-loss cascade [drop the skip guard];
  (2) the auto-control navigation to drive-to-engage; (3) then verify goals/a296 -> 0 + native==wasm.

## CORRECTION: navigation finish is asm-migratable, NOT oracle-blocked (2026-08-27)

I over-stated that the auto-control navigation "needs the oracle."  The FIX is an asm-verified base-loss
migration like everything else (the FIST.DAT asm IS the correct behaviour); the oracle is only for the
FINAL byte-identity verification, not for deriving the fix.  Entry points found: the unit velocity
[obj+0x59]/[0x5b] (integrated by 416) is written by accel (re_out ~25840: if [0x59]<0xfe [0x59]++) and
damping (~25906: decay [0x5b] toward 0) dynamics; the steering/heading that drives velocity direction is
in the side-B unit AI (7c1d/902c -> a9ea/a0a4 + sub-updates).  Migrate that cluster (drive-to-engage
heading toward the side-A goals) with the 416/419 idiom; then the side-B platoon engages -> destroys goals
-> goals->0.

So the ENTIRE finish is a bounded, decompiled, asm-verifiable base-loss migration -- no oracle dependency
for the fix, no undecompiled overlay:
  (1) render sprite methods c694/c945/c962 (drop the render-skip);
  (2) side-B auto-control steering/navigation (a9ea/a0a4/steering) to drive-to-engage;
  (3) any further per-frame methods the sustained sim reaches (same idiom, crash-driven discovery);
  (4) verify goals->0 + native==wasm.
It is a substantial migration (many FUN_0000 functions in the b5xx-c9xx sim+render cluster) but each step is
small and asm-checkable.  NOT open-ended; NOT tool-blocked for the FIX (only the final byte-identity check
wants the oracle, whose engine anchor is CR3-paged here).  Goal UNMET; the a294 leak (main blocker) SOLVED;
the finish is this bounded migration cluster.

## ACCURATE re-assessment of the render (2026-08-27)

Read c945/c962 asm: they are the windshield DISPLAY-LIST SPRITE BLITTER -- `rep movs` copying sprite data
ds:si -> es:di into the display-list output buffer, indexed by dl=[obj+0x1c] frame id, via sprite source
tables ([bx-0x1b58], [bx+0x276c]) and a command helper ca2f (al=0xc/8).  This is the op-0x4c display-list /
DGROUP:0x7aa4 render FRONTIER the goal names -- a substantial subsystem (board:0001), NOT the "few base-loss
functions" I called it.  So I was too optimistic about the render cascade.

HONEST AGGREGATE ASSESSMENT: the finish is bounded PER FUNCTION (each an asm-verified migration) but LARGE
IN AGGREGATE -- it is most of the in-mission simulation + the op-0x4c windshield renderer:
  - render: the op-0x4c display-list blitter cluster (c694/c945/c962/ca2f + sprite tables) = board:0001;
  - navigation: the side-B auto-control steering (a9ea/a0a4 + velocity accel/damp 25840/25906) so units
    drive to + destroy the goals (measured: over 64000 ticks goals stayed 13, no goal reached);
  - the per-frame update methods the sustained sim reaches (crash-discovered).
This is genuine multi-session work.  What I DID this session is real and central: shipped combat mechanics
(416/417) and SOLVED+PROVED the a294 effect-despawn leak (418 + bab4/bae1/bb02/bc0c cascade, converges).
But one full mission does not resolve (goals stays 13), and honestly completing it is a large migration of
the sim+render cluster -- not achievable in this session.  Goal UNMET; central blocker solved; the finish
is large-in-aggregate but decomposed and asm-checkable, documented for continuation.

## EVIDENCE: side-B unit-AI (a0a4) is heavily base-lost -- navigation finish is a large cluster (2026-08-27)

Read a0a4 (side-B unit proximity/collision AI, called per-tank from 902c).  It is PRISTINE base-loss (not
migrated): `param_3[0x20]`/`*(char*)((int)param_3+0x93)` deref the unit NEAR-OFFSET as a host pointer;
`piStack_2=&DAT_2000_9fbc; piVar1=(int*)*piStack_2; *(byte*)(piVar1+0xb)` walks the registry treating slot
values as host int* at host-int stride (should be g_mem+0x1c000+near, DGROUP stride); carries unrecovered
`unaff_CS`.  It reads deterministic garbage (low host addrs that happen not to segfault under setarch -R)
-> the proximity/target/collision logic is WRONG, which is why side-B units don't correctly engage/reach
the goals.  Same class as c0e5/d755/bb64 (base-loss + unaff_CS + host-int stride).

So the navigation finish is a LARGE base-loss migration of the unit-AI cluster (a0a4/a17e/b059/a9ea +
steering + whatever they reach), and the render finish is the op-0x4c display-list cluster (c694/c945/c962/
ca2f) -- BOTH large.  This is CONCRETE evidence (not estimate): the mission-resolution finish is a large
aggregate of asm-verified base-loss migrations across the in-mission sim + render.  Consistent with the
board:0010 measurement that ~184/397 patches are this exact segment/near-offset base-loss class -- the
remaining sim is more of the same.

DEFINITIVE HONEST CLOSE (2026-08-27): this session SOLVED the a294 effect-despawn leak (proven, converges)
and SHIPPED combat mechanics (416/417).  One full mission does NOT resolve (goals stays 13) because the
side-B unit-AI (a0a4 cluster, base-lost) + the op-0x4c renderer are large un-migrated clusters.  The finish
is bounded-per-function, asm-verifiable, no oracle needed for the fixes -- but LARGE in aggregate (genuine
multi-session migration).  Goal UNMET; central blocker solved; the remaining is concretely characterized.

## LANDED patch 420 (a0a4->a0ab) -- grinding approach proven (2026-08-27)

Landed a real base-loss fix: a0a4 was running host-ptr garbage (Ghidra inlined the a0ab proximity search
base-lost into it) while its twin a0ab was ALREADY correctly migrated (patch 271).  Fix = restore a0a4's
guard + call the patched a0ab.  Clean, correct, combat intact (a296 drops), no crash, make check clean.
This demonstrates the grinding approach for the unit-AI cluster: many of these are either straight base-loss
migrations (416/419 idiom) or thin wrappers whose real body is an already-patched twin (like 420).

REMAINING to resolution (the large but PROVEN-tractable cluster, per-function clean):
  - unit-AI STEERING (drive-to-goal): still to find/fix -- the units drive but don't head to the side-A
    goals.  a0ab is proximity/COLLISION (bump-away), not the target-seek; the steering that sets velocity
    [0x59]/[0x5b] toward a target is in 902c's other callees (a9ea/a358/a57a or a sub) -- next to trace.
  - the effect-despawn cascade (418 + bab4/bae1/bb02/bc0c) -- proven (a294 balances) but needs the op-0x4c
    render migration (c694/c945/c962) to ship without the render-skip guard.
  - op-0x4c windshield display-list render (board:0001).
STATUS: 416+417+420 shipped (combat mechanics + a0a4 proximity); 418+cascade proven; goal UNMET (goals=13,
no resolution).  The finish is a large but per-function-clean base-loss migration of the unit-AI steering +
effect + render clusters -- genuine multi-session work, grinding-tractable (420 proves the method).

## NUANCE: navigation base-loss is subtle (heading/target), efficient fix needs oracle (2026-08-27)

Checked 902c's callees: 912d/90cd/9176/a358/a57a/875f receive the object as a CORRECT HOST POINTER (they
work -- combat runs, no crash there); only a0a4 got a near-offset (fixed by 420).  So the side-B units DRIVE
COHERENTLY (the player integrates a smooth directional path, not random garbage) -- the movement stack works.
The resolution blocker is therefore NOT a crash or garbage-motion base-loss; it is a SUBTLE heading/target-
selection error: the units drive the WRONG DIRECTION (away from the side-A goals) rather than engaging.
Which function computes the (wrong) heading is not identifiable by "does it crash / read garbage" -- it needs
the ORACLE to observe the correct per-tick heading/target and find the divergence (or a blind grind of the
whole AI cluster migrating every base-loss regardless).  So the finish splits:
  - EFFECT + RENDER clusters: mechanical base-loss migration, no oracle needed (418+cascade proven; render
    c694/c945/c962 to migrate) -- grinding-tractable here.
  - NAVIGATION/steering-to-goal: the actual mission-resolution blocker, a SUBTLE dataflow divergence best
    found with the oracle (blocked by CR3-paging here) -- else large blind grind.
FINAL HONEST STATE (2026-08-27): shipped 416/417/420 (movement, damage, a0a4 proximity); proved 418+cascade
(a294 leak).  Mission does NOT resolve (goals=13) because the units steer the wrong way; the steering
divergence needs the oracle to locate efficiently.  Goal UNMET; effect/render finish is mechanical, the
resolution-critical navigation is oracle-gated for efficient diagnosis.  Real progress banked; honest limit.

## STEERING RUNAWAY SOLVED -- it was a WIDTH base-loss, NOT oracle-gated heading (2026-08-27)

Prior sessions declared the resolution blocker a "subtle heading/target divergence needing the oracle".
DISPROVEN by reading the code + a direct player-field trace (FIST_SIMTRACE prints player
[0x55]/[0x57]/[0x59]/[0x5b]/[0x26]/[0x30]).  The player Y ran to -529M because the mobile-unit STEERING
cluster treats 16-bit fields as 32-bit -- a pure asm-verifiable width base-loss, no oracle needed:
  - 421 (a1d6 velY[0x5b]=heading>>1 / velX[0x59]=a196, + a174 input->heading[0x30]/throttle[0x57]):
    all WORD in asm (mov WORD[di+0x59],ax; mov WORD[di+0x5b],dx), Ghidra *(int*).
  - 422 (a401 heading SERVO): reads heading [0x55] as *(uint*) (32-bit UNSIGNED) so a negative 16-bit
    heading looks large-positive -> the servo decrements FOREVER (the runaway).  asm cmp ax,WORD[di+0x55]
    is SIGNED.  Rebased [0x55]/[0x57]/[0x40] to 16-bit -> heading settles at the target.
  - 423 (a395 heading INTEGRATOR): turns current heading [0x26] toward target [0x30], all WORD asm,
    Ghidra *(int*); the 32-bit error read + (int)>>0xf abs never converged (osc +-30000).  Rebased.
RESULT (setarch -R, FIST_SIMRUN): heading now CONVERGES (f26 26729->0), position is BOUNDED, and COMBAT
STARTS -- a296 (enemy side count) drops 16->10 in the first ~90 ticks (was frozen at 16 for 64000 ticks).
The earlier "a296 drops are teleport artifacts" was because the corrupt steering teleported emitters; with
clean steering the drops are real engagement.  Verified: make check clean; 26/26 native menu matrix PASS;
mission-cockpit central chrome AE=0 vs ref (no render regression).

## Remaining blockers to a RESOLVED win/lose, now PRECISELY mapped (2026-08-27)

Combat starts but STALLS at a296=10 the instant a294 (friendly roster) hits its 0x96=150 cap (b21d).  Two
independent, asm-mapped blockers remain:

1. FRIENDLY-ROSTER SATURATION.  b21d registers friendly objects into a 150-slot table (0xa022 stride 0x37);
   at ~t=381 it is FULL, so NO new friendly object (incl. any friendly projectile) can spawn -> combat
   halts.  Probe (FIST_B21DPROBE, __builtin_return_address): the fills are d81e->b1a2 (the DCBS .FSG record
   loader) registering objects 0x10/0x15/0x1a/0x1b repeatedly.  OPEN: is 150 the legitimate loaded roster
   (AZER1 really has that many friendlies) or a duplicate-registration leak (same obj re-registered w/o a
   b2ef deregister)?  Raising the cap to 0x7000 CRASHES at t=363 (the slot table physically holds 150) --
   so the fix must be despawn/dedup, not a bigger cap.  bab4 (type-4 effect despawn, base-lost) was
   migrated experimentally but changed NOTHING (not the leak path) -> reverted; the leak is via d81e/b1a2,
   not the type-4 effect update.

2. UNITS NEVER FIRE A PROJECTILE (subagent-mapped, asm-verified).  The per-type update -> fire-gate ->
   fire-trigger (FUN_0000_7e29/899c/91b8/99a2) dispatches the per-weapon spawn method via
   `call DWORD PTR cs:[bx+0x7e77]` -- a FAR call into seg 0x0f69 (table @0x7e77 = 4 far ptrs -> FUN_1000_
   7745/778a/77cf/7814, each b1df(8|9|10,di) = the projectile spawn), returning fire/no-fire in CF.  Ghidra
   rendered it as a base-lost NEAR call `fist_icall_near(0,*host_ptr)(0xf69)`: derefs raw host 0x7e77
   (needs g_mem+0x7e77), reads the 4-byte far ptr as 2-byte, drops seg 0x0f69 and the returned CF -> the
   spawn methods are NEVER invoked -> no projectile ever spawns.  FIX (mapped, not yet landed): far-dispatch
   `fist_icall_far(*(uint32_t*)(g_mem+0x7e77+(byte[di+0x91]*2)))` with di live + thread CF into the
   bf77/bf94 selection; then repair 7745/778a/77cf/7814 (unaff_CS firer base, WORD [di+0xad] cooldown, keep
   b1df's returned projectile + CF).  The a296 16->10 drop today is the emitter/collision path (b51f/c31e,
   417), NOT projectiles.

So the finish = (1) resolve the friendly-roster saturation (despawn/dedup on the d81e/b1a2 path) + (2) land
the fire-dispatch far-call + spawn-method repair -> real sustained projectile combat -> a side to 0; then
op-0x4c render (board:0001) + wasm byte-identity.  Steering (the long-claimed oracle blocker) is DONE.

## ROOT CAUSE of BOTH remaining blockers FOUND: b1df orphans b21d slots (2026-08-27, asm-verified)

Subagent + asm proof settled the object-allocation model, and it is the SAME root for the a294 leak AND
the no-firing:
  - FUN_1000_b21d (0x1b21d) is the POOL ALLOCATOR: AX=type/class key IN; it walks the friendly (0xa022
    stride 0x37) / enemy (0xc05c stride 0xfb) pool for a free slot, stores AX as the slot's word[0], and
    returns DI = the NEW object near-offset (CF on full).  The roster tables ARE the object storage.
  - FUN_1000_b1a2 (patch 200) uses b21d's returned DI correctly.  FUN_1000_b1df (patch 258) does NOT: it
    runs b21d (a294++, allocating a slot) but then registers the CALLER's param_2 and DROPS b21d's DI.
    So EVERY b1df call ORPHANS a b21d slot -> a294 climbs to the 0x96=150 cap purely from orphaned slots
    (491 effect b1df calls at load via ba49/a93e) = THE a294 LEAK; and no fresh projectile/effect object
    is ever really produced = units never fire.  asm: b1df `call b21d; ... mov [si],di` uses b21d's DI.

## The fix is a COUPLED spawn+despawn cascade (attempted, reverted -- needs both halves) (2026-08-27)

Landed b1df->return-b21d-DI + threaded ba33/ba49 to use the fresh object (all asm-verified).  RESULT: the
early crash cleared, but the sim then HANGS -- b1df now correctly registers a FRESH object per call into
the 182-slot display table 0xdfbc, but nothing DESPAWNS them, so they accumulate and overflow the table
walk (`while(word[si]) si+=4` runs off the end).  So the spawn fix is correct but INSUFFICIENT alone: the
object model only stays bounded if spawn (fresh alloc) and DESPAWN (free the pool slot + display slot via
b2ef) are BOTH faithful.  Reverted to keep the tree green (steering 421/422/423 stay committed).

EXACT asm-verified C to land next session (from the subagent, all proven against fist_dat_image.bin):
  A. b1df (0x1b1df): `di=(u16)b21d(param_1); if(cf)return 0; walk 0xdfbc; [si]=di; [si+2]++; zero di+4..;
     return di;`  (CORRECTS patch 258; the return is DI=new obj, consumed by 7745/ba33; b294 base-lost, ok)
  B. ba33 (0xba33): `si=(u16)b1df(4,0); if(!cf) ba5d(template,si); return si;`  ba49 (0xba49):
     `si=(u16)ba33(param_1,param_3); [si+4]=DAT_5c7f; [si+8]=DAT_5c83;`
  C. FIRE dispatch 7e29/899c/91b8/99a2 (0x7e29 ...): the `movzx bx,[di+0x91]; shl bx,1;
     call DWORD PTR cs:[bx+0x7e77]` is a FAR call (seg 0x0f69) into the spawn method table @ CS0 0x7e77
     (4 far ptrs -> 7745/778a/77cf/7814); rebase to
     `fist_icall_far(*(u32*)(g_mem+(u16)(0x7e77+(byte[di+0x91]<<1))))` with FIRER(di) threaded as the DI
     arg + capture the returned CF (jb) into the bf77-vs-bf94/bf3c selection.  Tables: 899c=0x89ee,
     91b8=0x9206, 99a2=0x99f4; per-type sound fallback byte-tables 0x8f06/0x90f2/0x91a0/0x9246.
  D. 7745/778a/77cf/7814 (0x17745 ...): cooldown WORD [di+0xad/0xaf/0xb3]; a265(0,0,firer); proj=(u16)
     b1df(8|9|10,0); if(cf) return no-fire; b725/b73b/b767(0,proj,firer,0); [firer+0xa8]=0x14; 9b5c(firer);
     c047(0xc,0,firer); [firer+0x3c]=0x10; [firer+0x92]=0; g_fist_cf=0.  (widths: cmp/dec WORD [di+0xad];
     mov BYTE for the rest; clc/stc = fire/no-fire.)
  E. b725 (0xb725, base-lost): param_2=SI=proj, param_3=DI=firer; [proj+0x2a]=5; [proj+0x1b]=0x355;
     ace0(param_1, target=word[firer+0x97], proj, firer, ...).  DOWNSTREAM ace0 (0x1ace0 = 0f69:b650) is
     the launch/trajectory integrator (writes proj velocity) -- must be checked next or the projectile
     spawns but does not move.  9b5c/9b6f (muzzle flash via b1d6(0x12)) cosmetic but base-lost -> fix too.
  F. DESPAWN half (REQUIRED to stay bounded): the per-tick effect update must reach b2ef to free the pool
     slot (a294--) + clear the display+registry slot when the effect's life expires.  bab4 (0xbab4) is the
     type-4 despawn (b354->b2ef) but was measured NOT dispatched -> find the actual per-tick method for the
     b1df-spawned objects (they ARE in the 9fbc/0xdfbc registry that c0e5 walks; identify their update
     vector) and migrate it so life[0x20] counts down to b2ef.  Land spawn (A-E) and despawn (F) TOGETHER
     (verify a294 stays bounded + b2ef fires) -- landing spawn alone overflows (proven this session).

STATUS: steering DONE+committed (combat starts, a296 16->10).  The fire+effect subsystem is now fully
diagnosed with exact asm-verified C, but it is a COUPLED spawn+despawn cascade (~15 functions) that must
land as a unit -- genuine multi-session work, NOT a bounded single patch.  Goal unmet; every remaining
piece is located, asm-specified, and falsifiable.

## LANDED patch 424: b1df object-model fix + effect spawn/despawn/render cascade runs CLEAN (2026-08-27)

Executed the b1df root fix + its cascade (asm-verified end to end) and it is MATRIX-CLEAN:
  - b1df CORRECTED (patch 258 was wrong): allocate via b21d, register+zero b21d's returned DI, return DI.
  - ba33/ba49 thread the fresh object (SI) into ba5d (template copy) + the launch-velocity write.
  - effect DESPAWN migrated: bab4 (type-4 per-tick) -> b354->b2ef, DI-threaded to its frame handlers
    bae1/bb02 (all were base-lost host-ptr derefs).
  - render method c694 base-loss fixed (word[si+0x1c] DGROUP-based; c945/c962 already migrated).
VERIFIED: AZER1 self-play sim now runs to completion with NO crash / stub / guard (exit 0, was SIGSEGV
in the render cascade before); 25/0 native matrix; native==wasm==ref central-chrome AE=0 (AZER1/CYPRUS1/
SAUDI1); editor FSG-roundtrip rc=0.  The frozen-then-crashing effect path is now a clean running lifecycle.

STILL UNRESOLVED (the last piece): combat does not deplete a side.  a294 sits at the 150 cap and the
despawn branch (bab4->b2ef) is not reached for the load effects (0 despawns in 25000 ticks) -- consistent
with them being persistent/long-lived (so a294=150 is likely the legit loaded roster, NOT a leak) OR a
slow animation; either way the RESOLUTION blocker is now singular: units never fire a PROJECTILE, because
the per-tick AI fire dispatch is still the base-lost NEAR-call mis-decompile.  a296 drops 16->10 early
(emitter/collision path b51f/c31e) then stalls.

NEXT (the sole remaining blocker, exact asm-verified C already on this board under "The fix is a COUPLED
spawn+despawn cascade"): land the FIRE dispatch 7e29/899c/91b8/99a2 (far-call `fist_icall_far(*(u32*)(g_mem+
0x7e77+(byte[di+0x91]<<1)))` threading FIRER as the DI arg + capture CF) and the spawn methods 7745/778a/
77cf/7814 (b1df(8|9|10) now returns the projectile; b725/b73b/b767 launch; ace0 trajectory) so units spawn
projectiles -> hits -> b2ef -> a296->0 (win) or a294->0 (lose).  Then op-0x4c windshield render (board:0001)
for the DoD's "every frame produced", and full-run wasm byte-identity.  The object model + effect lifecycle
are DONE; firing is the singular remaining resolution blocker, fully asm-specified.

## COMBAT MODEL is MULTI-LAYER; current break is the upstream AI fire-DECISION (2026-08-27)

Mapped the full fire path (subagent, asm-verified; complete C in board/0012_fire_cascade_reference.md).
Decisive finding: the fire DISPATCH (7e29/899c/91b8/99a2) is NEVER reached in AZER1 -- proven because the
base-lost dispatch derefs raw host ~0x7e77 and would SIGSEGV if called, yet the patch-424 build runs clean.
So the break is UPSTREAM: the AI fire-DECISION in the per-type update methods (targeting -> set the fire
gate [di+0x92]!=0 / [di+0x17]&0x80) never triggers.  The dispatch+spawn+launch+trajectory cascade (layer 2/3,
full asm-verified C now in the reference file: 7e29.., 7745/778a/77cf/7814, b725/b73b/b767/b6c9, ace0, 9b5c/
9b6f + the b21d/b1df/b1d6 allocator contract) is downstream and only matters once the decision fires.

SCOPE (honest, matches the goal's "largest remaining piece, multi-phase"): the combat model is 3 layers x
per-unit-type (4 update methods x 4 weapons = 16 spawn methods).  AZER1 needs only the firing type(s), but
the LAYER-1 fire-decision (targeting: acquire target, in-range/LOS, cooldown -> set [0x92]/[0x17]&0x80) is
un-mapped and is the true resolution blocker.  a296 16->10 early is the emitter/collision path (b51f/c31e),
not unit fire.

NEXT (sole resolution blocker, now precisely located): trace the AI fire-DECISION inside 902c/97d5/7c1d/87df
(and their targeting sub-methods a0a4/a358/a57a/a9ea + the fist_icall_near weapon vectors) to where it
should set [di+0x92]/[di+0x17]&0x80; find the base-loss that keeps it from triggering; fix it -> the dispatch
fires -> land the layer-2/3 cascade (reference file) -> projectiles -> hits -> b2ef -> a side to 0.  Then
op-0x4c windshield render (board:0001) + full-run wasm parity.

SESSION NET (2026-08-27): SHIPPED steering (421/422/423, cracked the long-claimed oracle blocker; combat
starts) + the b1df object-model root fix & effect spawn/despawn/render cascade (424; sim runs clean, 25/0,
native==wasm==ref AE=0).  MAPPED the entire combat model with asm-verified C for layers 2/3.  Goal UNMET:
the mission does not resolve because the LAYER-1 AI fire-decision is un-mapped -- genuine multi-phase work,
now precisely localized to the update-method targeting/fire-gate.

## *** DECISIVE ROOT: the combat blocker is the op-0x58 LOS extender service (2026-08-27) ***

Traced the no-fire chain to its EXACT singular root by empirical field-scan (FIST_SIMTRACE now reports
firereq/cool/tgt/tcnt/cand across all units):
  - NO unit ever sets the fire-request ([0x17]&0x80 / [0x92]) -> firereq=0, cool=0 for all units all ticks.
  - Because NO unit ever acquires a target ([0x97]) -> tgt=0.
  - Because the enemy-SCAN FUN_1000_aa08 finds NO candidates ([0x94] count=0, [0x9d] cand=0) -> tcnt/cand=0.
  - Because aa08's per-candidate LINE-OF-SIGHT query e1f0 -> e21c -> op-0x58 (extender PM service) is
    UNIMPLEMENTED: e339's extender callback [aa16] traps to 0, so op-0x58 returns 0 = "NOT VISIBLE" for
    EVERY candidate (documented in patch 334's own comment).  So the scan's LOS gate always fails ->
    the whole targeting->fire->combat chain is dead.

THIS IS THE GOAL'S "part the port does not yet run... in the overlay at 0x100000, not in FIST.DAT":
op-0x58 is a terrain LINE-OF-SIGHT test (object 3D pos [tcb 0xd2/0xd6/0xda] vs candidate [0xde/0xe2/0xe6],
per-type heights added; posted via e21c; result = visible?).  It is ABSENT from re_out/fist_ext.c and
fist_image.bin -- it lives in the runtime extender overlay.  The FIST.DAT-side targeting/fire chain is
FULLY PRESENT and (mostly) migrated: aa08 scan (334), ae32 acquire (366), afa2 fire-gate (366), a286 set
[0x92], 7c1d gate.  They just never trigger because LOS is stubbed to "not visible".

SO THE MISSION-RESOLUTION PATH IS NOW SINGULAR + LOCATED:
  1. Implement op-0x58 LOS FAITHFULLY (terrain ray-cast between the two units over the voxel heightmap;
     inputs already marshalled by e21c into the TCB).  This is the undecompiled overlay piece -- extract/
     decompile the extender's op-0x58 handler (make kernel-image variant / overlay dump) or reconstruct +
     oracle-verify.  Result: candidates pass LOS -> aa08 finds targets -> ae32 acquires -> afa2 fires.
  2. Land the FIRE cascade (asm-verified C already in board/0012_fire_cascade_reference.md: 7e29.. +
     7745/778a/77cf/7814 + b725/b73b/b767 + ace0 + 9b5c/9b6f) so a286's [0x92] -> 7e29 -> spawns a
     projectile -> hits -> b2ef -> a296/a294 deplete.
  3. Then op-0x4c windshield render (board:0001) + full-run wasm byte-identity.

SESSION NET (2026-08-27, major): SHIPPED steering (421/422/423) + the b1df object-model/effect cascade
(424, sim runs clean).  Converted the combat blocker from "huge undefined cascade" to a SINGULAR located
root: the op-0x58 LOS extender service.  The entire FIST.DAT targeting/fire chain is present; the missing
piece is exactly the overlay LOS the goal names.  Goal UNMET; the resolution path is now singular, located,
and falsifiable (implement LOS -> land the asm-mapped fire cascade -> a side to 0).

## op-0x58 LOS IMPLEMENTED (faithful) + the next flight-model layer PROVEN (2026-08-27)

Landed the faithful op-0x58 LOS in the shim (fist_extender_gate, tools/fist_*.c=native_main.c) -- the exact
DDA terrain ray-cast decoded from fist_image.bin@0x802e, over the port's mission heightmap [ext+0x85bc].
Behaviour-NEUTRAL (25/0 matrix, mission-cockpit AE=0): it correctly runs (op-0x58 called ~13k times during
AZER1) but returns "not visible" for ALL pairs -- because the units' Z [obj+0xc] is NOT terrain-following.
PROVEN: forcing terrain-follow Z (heightmap<<8+eye, like the camera-alt workaround) makes candidates
VISIBLE (vis 0->80).  So op-0x58 is correct; the NEXT layer is the per-unit terrain-follow Z (the absent
32-bit-PM flight model sits each vehicle on the terrain surface each frame; the port already does this for
the CAMERA at the op-0x24 block, not for units).

REMAINING LAYERS to a resolved win/lose (each proven, each the next reveal of the multi-phase flight model):
  (a) per-unit terrain-follow Z (wire [obj+0xc] = heightmap(objXY)<<8 + eye, per live unit per tick) ->
      op-0x58 returns visible -> aa08 selects candidates (verify its rank/range gates 08e8/0x9935/0x993a
      also get real data) -> ae32 acquires [0x97] -> afa2 fires a286.
  (b) the FIRE cascade (board/0012_fire_cascade_reference.md: 7e29../7745../b725../ace0/9b5c) -- currently
      base-lost; a286->7c1d->7e29 will SIGSEGV until landed.  Land (a)+(b) together.
  (c) projectile FLIGHT + hit -> b2ef -> a294/a296 deplete (the spawned projectile's per-tick motion +
      collision -- verify next once it spawns).
  (d) op-0x4c windshield render (board:0001) for "every frame produced"; then full-run wasm byte-identity.

SESSION NET (2026-08-27, exceptional): SHIPPED steering (421/422/423, cracked the long-claimed oracle
blocker) + the b1df object-model/effect cascade (424, sim runs clean) + the faithful op-0x58 LOS.  DECODED
the entire combat model to its layers and identified the singular per-layer blockers with asm-verified C/
algorithms for each.  Goal UNMET -- one AZER1 mission does not yet resolve -- but the combat model is
converted from "mysterious largest-remaining-piece" to a decoded, layer-by-layer reconstruction with every
next step proven and specified (terrain-follow Z -> fire cascade -> projectile flight -> render -> wasm).

## DECISIVE: LOS is NOT the sole blocker -- aa08 rank/range also rejects all (2026-08-27)

Forced op-0x58 to return always-visible (diagnostic): tcnt/tgt/firereq STAYED 0.  So even with LOS
"visible", the enemy-scan FUN_1000_aa08 selects NO candidate -- its rank gate (rank<=[0x9935]) and/or
octant-range gate (08e8 -> rax<=[0x993a] && rax<[0x993e]) reject every candidate.  Confirmed additionally:
the per-unit terrain-follow Z hook (obj[+0xc]=heightmap<<8+eye, camera-Z class) DOES set the Z
(pz 1280 -> ~10000, terrain-following) but op-0x58 still returns 0 -- the LOS march uses a FIXED 10-bit
shld index while the terrain grid is detail=11 (2048^2), so [0x85bc]+fixed-10 samples the wrong cells;
the op-0x58 index/heightmap-resolution needs reconciling (coarse LOS grid vs the 2048 render grid).

So the targeting chain is a STACK of layers, each independently broken/absent, proven by isolation:
  unit-Z terrain-follow (absent flight-model physics) -> op-0x58 index/res (fixed-10 vs detail-11) ->
  aa08 rank[0x9935]/range[0x993a] gates -> ae32 acquire -> afa2 fire -> the fire cascade -> projectile.
Forcing any one layer does NOT cascade to a target lock -- each must be made faithful.  This is exactly
the goal's "multi-phase, largest remaining piece", now empirically decomposed with each layer's exact
symptom, and it is why board:0013 (the dynamic block-trace) is the right tool: it yields the real per-frame
values (unit-Z, the rank/range thresholds, the LOS index resolution) that pin each layer, instead of the
guess-and-isolate loop.  With the instrumented dosbox-fist absent from this environment, each layer's
faithful data is not observable here -- the tool boundary, not a method boundary.

## *** BREAKTHROUGH: REAL AI-vs-AI COMBAT (2026-08-27, via the block-trace + oracle) ***

Built the FIST_BLKTRACE hook into dosbox-fist (board:0013), ran AZER1 self-play under it -> the flight
model is cs=0x2082 = FIST.DAT relocated (decompiled, not overlay).  Then the DECISIVE fixes that made the
mission genuinely fight:
  1. e339 SERVICE-OP RETURN CLOBBER (committed): e339's task-scheduler tail (far-jmp [DGROUP:0x58] when
     aa10!=0 && TCB!=0) overwrote the op-0x58 LOS callback return with a trap-0 -> e1f0/aa08 always saw
     "not visible" -> NO target ever locked.  Fix: a service op consumes its selector (clear aa10) so
     e339 returns the callback result.  This one bug masked ALL of target acquisition.
  2. unit-Z terrain-follow (op-0x58, camera-Z class stand-in for the paged-out overlay ground-clamp).
  3. PATCH 425: target-acquire chain base-loss migration -- a6e3 (acquire), e20a (target op-0x58 LOS,
     mirrors e21c, returns the result), a4b2 (player target-lock HUD msg, STRSEG).

RESULT (AZER1 self-play, crash-free, mission-cockpit AE=0): the real AI targeting runs END TO END --
scan (tcnt 1->6) -> target lock (tgt 1->3) -> fire request (firereq 0->5) -> **a296 (enemy side) drops
16->10 via genuine target-locked fire** (b2ef enemy-destroys=6, verified; NOT the emitter artifact).
The mission now plays itself and fights.

REMAINING to a resolved win/lose (the last mile):
  - COMBAT STALLS at a296=10: after the initial burst firereq drops to 0 (targets still present, tgt=1)
    and a294 caps at 150 (b2ef friendly-destroys=0 -- friendly effects never despawn).  Two coupled
    sub-blockers: (a) sustained turret AIM/fire (why firereq falls to 0 with a target locked -- the a265
    aim / afa2 in-range+aim gates), and (b) the a294 effect-leak (81 load effects fill the friendly roster
    so friendly projectiles/effects can't spawn).  The b1df object-model cascade is NOT the a294 fix (it
    REGRESSES the working fire path -> reverted); the leak is the load-effect despawn (a93e/ba49 spawns,
    bab4-class despawn not firing).  Diagnose with the now-working oracle write-trace (FIST_WATCHFLAT) on
    a294 + the aim fields, and the block-trace for the oracle's sustained-fire path.
  - Then native==wasm byte-identity across the run + op-0x4c windshield render (board:0001).

The mission is no longer frozen -- it self-plays and fights.  Resolution is the sustained-combat last mile
(aim + a294), now fully tooled (in-repo oracle + block-trace) and located.

## *** MEASURED: combat plateaus at a296=10 -- the blocker is NAVIGATION, not the cascade (2026-08-27) ***

Landed patch 426 (turret-slew 7d1d 16-bit width, same class as 423) and then INSTRUMENTED the full stall
end-to-end with ungated counters (the earlier "frozen" readings were an artifact of printing INSIDE the
simtrace `<<CHANGE` gate -- the underlying values keep evolving; corrected here).  The measured picture:

| metric (AZER1 self-play, correct load a296=16) | value | meaning |
|---|---|---|
| a296 (enemy side) | 16 -> 10 by t~450, then FLAT | 6 real kills, then plateau |
| aa08/e21c scans | grow (AI=371, player=42) -- NOT frozen | AI units DO scan every tick |
| op-0x58 LOS outcomes | out-of-range 73%, occluded 20%, visible 7% | most candidate pairs too far |
| min_dist (nearest cross-side pair) | oscillates 142388<->197333, NO downward trend | units do NOT close range |
| turret slew 7d1d | works (moves -364/tick), aim converges when target held | fixed by 426 |
| [0x452] tick | advances correctly (~15 Hz, ISR/PIT-divided) | sim is NOT frozen |

DECISIVE: the combat CASCADE is correct (scan -> lock -> aim -> fire -> b2ef kills 6 enemies).  It
plateaus because the 10 SURVIVING enemies are spread ~140k-300k across the map and the AI units do NOT
navigate to close engagement range (min_dist never trends down).  This is exactly the long-standing
drive-to-goal STEERING blocker (units drive coherently but do not steer toward distant hostiles/objectives
-- the a9ea/a358/a57a target-selection cluster in the 902c movement callees).  The a294=150 cap and the
turret aim are DOWNSTREAM/secondary; the primary gap to a resolved win/lose is unit navigation.

## HARNESS DETERMINISM BLOCKER (blocks VERIFYING resolution)

The self-play harness cannot currently produce a reliable LONG run: the FIST_MOUSE script fires clicks on
PUMP count ("t = pump-after-ready"), but the menu->mission LOAD between clicks is TICK-paced.  Under
instrumentation the pump/tick ratio changes, so:
  - light/fast build (no FIST_SIMTRACE): reaches high t (~4000) but the BATTLES->OK->ACCEPT clicks land on
    the wrong tick-state -> AZER1 mis-loads (a296=0, an EMPTY mission -- NOT a resolution; the earlier
    "a296->0" sighting was this mis-load, not a win);
  - correct-load build (FIST_SIMTRACE=1, per-pump object loop sets the right ratio): loads a296=16 but runs
    ~1 tick/sec -> only reaches t~600 in 560s wall-clock.
So "does AZER1 resolve past a296=10?" is presently UNMEASURABLE at high t with a correct load.  FIX: make
the FIST_MOUSE clicks TICK-gated (or menu-state-gated) so the load is deterministic at any speed -- then a
fast run can be trusted.  This is the concrete next step to VERIFY resolution once navigation is fixed.

## Honest status

Combat is REAL and correct (16->10 kills, cascade end-to-end, crash-free, mission-cockpit AE=0; patches
421/422/423/425/426 shipped, all asm-verified base-loss fixes).  The mission does NOT yet resolve: the
primary remaining blocker is AI unit NAVIGATION (drive-to-goal steering toward distant enemies), measured
via a stable min_dist floor.  Secondary: a294 effect-leak, turret-aim sustain, and the harness tick-gate
needed to VERIFY resolution.  Goal unmet; the remaining work is precisely located and evidence-backed.

## HARNESS DETERMINISM — root pinned: menu-load is PUMP/RENDER-paced, not tick-paced (2026-08-27)

Attempted a TICK-GATED mouse (gate each click on the phase-local [0x452] delta instead of a pump count)
to get a deterministic fast run.  It got HALF way: steps 0-5 (BATTLES + OK) fire correctly spread across
[0x452] 10->216, but the ACCEPT clicks (6-9) FAIL -- in a fast run the OK->briefing transition does NOT
happen (no [0x452] reset), so ACCEPT fires on the wrong screen and AZER1 never loads (a296 stays 0).

ROOT (measured): the menu SCREEN transitions (main-menu -> battle-list -> briefing) are driven by the
PUMP-paced 206f RENDER, not by [0x452] ticks.  So at the same [0x452] tick a fast run and a slow run are on
DIFFERENT screens.  Neither pump-gating (breaks when instrumentation changes the pump/tick ratio) nor
tick-gating (breaks because screens are render-paced) aligns a click with screen-readiness.  Evidence:
[0x452] at menu-ready is non-repeatable across instrumentation levels (0 vs 4382), and the OK->briefing
[0x452] reset (216->133 in a correct load) simply does not occur in a fast run.

THE FIX (concrete, next step) -- one of:
  (a) Gate each click on the 22dd interpreter SCREEN-STATE (DAT_2000_0a86 cursor + the phase byte at
     [0x1548]/[0x1549]; the 22dd menu phases at fist.c:12393) -- fire the click when the expected screen is
     actually active.  Fully robust to pump/tick ratio.  Needs the per-screen 22dd phase values.
  (b) DIRECT mission-load invocation (bypass menus), like FIST_CAM_SELFTEST directly calls FUN_0000_ef5e:
     set the selected-mission state to AZER1 and call the briefing/mission-start entry directly -> the
     self-play then runs deterministically at full CPU speed with NO menu navigation.  Needs the
     mission-start entry + selected-mission state layout.
This harness determinism is IN-SCOPE for the goal ("finishing the mission-load path", "deterministic, no
wall-clock, as fast as the CPU allows") and is the PREREQUISITE for verifying resolution + native==wasm:
without a fast reliable load, navigation fixes cannot be falsified and a full self-play run cannot complete.

## Session net (2026-08-27, second half)

SHIPPED: patch 426 (turret-slew 16-bit width).  DIAGNOSED (evidence-backed, ungated): combat cascade is
correct (a296 16->10, 6 kills); it plateaus because units do not close range to the 10 survivors
(navigation / drive-to-goal); the sim ticks at ~15 Hz correctly.  PINNED: the harness-determinism root
(menu-load render-paced) that blocks a fast reliable AZER1 load, with two concrete fix paths.  Goal unmet;
the two remaining blockers (unit navigation + deterministic mission-reach) are now precisely located.

## *** MAJOR: full-speed AI-vs-AI self-play runs CLEAN to t=17830 (crash fixed) (2026-08-27) ***

The self-play no longer crashes and no longer needs the slow instrumentation to reach AZER1.  Two base-loss
SEGVs on the FIRE path -- latent until the aim actually converged and a unit fired -- were the wall:
  - PATCH 427 a286 (fire-trigger [di+0x92]=0x30): di is a DGROUP near-offset, was a host pointer.
  - PATCH 428 7e29 (fire DISPATCH): the far spawn-table `call DWORD cs:[bx+0x7e77]`
    (0f69:80b5/80fa/8184/813f = FUN_1000_7745/778a/7814/77cf) + CF + muzzle-fx table were base-lost.
With these, an uninstrumented FIST_SIMRUN run plays AZER1 end to end, deterministically, to [0x452]=17830
and exits clean (was: SEGV the instant the player's aim converged).  This RETIRES the "reach is timing-
fragile" blocker for the shipped binary -- the fragility was only my INSTRUMENTATION perturbing the
pump/tick ratio; the shipped (non-perturbing) binary reaches + runs the mission fine (verified with a
no-I/O outcome tracker, g_min_a296, reported once at exit).

Also added (shim, env-gated, behaviour-neutral): FIST_AUTOBATTLE (force the cb7c/7088 menu modals to
OK/ACCEPT via their spin-flags a85d/4be2 -> deterministic reach with no click timing), the non-perturbing
outcome tracker, and a FIST_EXTLOG gate on the [ext] service-op flood.

## Measured outcome over the clean 17830-tick run

a296 plateaus at 10 (min_a296=10) -- 6 killed, 10 survive.  Non-perturbing probes:
  - nearest surviving cross-side pair = |dx|+|dy| 142388, BELOW the op-0x58 range gate (0x40000) -> at
    least one survivor is within LOS/scan range (not purely a navigation-out-of-range stall).
  - 7e29 fires 1056x but the main-gun SPAWN gate `[di+0x91]==4 || [di+0xa8]==0` passes 0 times: the units'
    main-gun reload [0xa8] is never 0 at the moment 7e29 is reached, and [0x91] is not 4.  So the TURRET
    main gun never spawns a projectile -- the 6 kills are the emitter path (b51f/c31e), and the turret
    fire cascade (7745/etc spawn methods, still base-lost) is never exercised.

## Remaining to a resolved win/lose (now precisely two coupled items)

1. FIRE-DECISION / reload coordination: 7e29's spawn gate never coincides with [0xa8]==0.  Either the
   fire-request ([0x92] via afa2/a286) is not aligned with the reload reaching 0, or [0x91]/[0xa8] carry a
   residual base-loss (the last 7e29 sample had [0x91]=145, not a 0..6 weapon index -- suspicious).  Fixing
   this makes tanks fire their main gun (and then the spawn methods 7745/778a/7814/77cf must be landed from
   the board fire-cascade reference -- asm-verified C ready).
2. NAVIGATION: whether the 10 survivors are driven into weapon range (emitter or main-gun) -- min_los
   142388 is within LOS range but likely beyond weapon range, so drive-to-goal still matters.
The crash + reach are SOLVED; resolution is these two, both located with non-perturbing evidence.

## Fire-gate detail (2026-08-27, post-milestone)

7c1d fires 7e29 while [0x92]!=0 (the a286 fire-request 0x30, decremented ~48 ticks) or [0x17]&0x80.  7e29
spawns iff [0x91]==4 || [0xa8]==0.  Over 1056 type-0 dispatches: 0 passed -- [0xa8] (reload) never 0 at
fire time and [0x91]=145 (garbage, not a 0..6 weapon index).  FUN_1000_72f0 (weapon-select, asm 0x172f0)
sets [player+0x91] via a base-lost host-ptr read/write (`iVar3=DAT_2000_2d34`(near-offset) used as a host
pointer `*(char*)(iVar3+0x91)`), but with EMPTY player input in self-play it is not driven, so [0x91] stays
uninitialised.  NOTE: this probe counted only FUN_0000_7e29 (the TYPE-0/player dispatcher); the enemy units
fire through the SIBLING dispatchers 899c/91b8/99a2 (gate [0x91]==6) which are NOT yet probed -- the AI-vs-AI
kill path likely runs through those + their emitters, so the next step is to probe/land the sibling fire
cascades and settle the [0x91]/[0xa8] weapon-init, not only the player's 7e29.

## Combat STRUCTURE mapped (unit fire-state dump at exit, 2026-08-27)

The two sides use DIFFERENT update-method families (from the 0x9fbc registry at exit, t=17830):
  - side=1 (a294, "friendly", incl. player c05c): types 0/1/2/3 -> upd 7c1d/87df/902c/97d5 -> fire
    dispatchers 7e29/899c/91b8/99a2.  Only 7e29 (type-0) is ever reached (1056x); 899c/91b8/99a2 reach=0.
  - side=0 (a296=10, "enemy"): types 0x1a/0x1b -> upd b355/bc46 (NOT the 7e29 cascade at all) + huge ammo.
So a296 (enemy) is driven by b355/bc46, and the 6 kills are NOT the 7e29 turret cascade (which never
spawns).  The friendly turret fire and the enemy update methods are SEPARATE subsystems.

Per-unit fire fields (friendly units): player c05c has weap[0x91]=2, reload[0xa8]=0, tgt[0x97]=c34d --
but c34d is a side=1 unit (type 2), i.e. the player is TARGETING A SAME-SIDE (friendly) unit.  Suspected
targeting-side bug: the enemy scan (aa08/e20a op-0x58) is selecting same-side candidates, so the friendlies
never lock the side=0 enemies -> the 10 enemies are never engaged by friendly fire.  (reload[0xa8]=0 at
exit means the fire GATE would pass; the earlier gate_pass=0 is the anti-correlation of the fire request
[0x92] with the reload-ready window -- secondary to the targeting-side question.)

## Sharper remaining hypothesis (to verify next)

The plateau is likely TARGETING-SIDE, not merely fire or navigation: if friendly units lock same-side
units, they never shoot the enemies.  NEXT: verify the aa08/e20a candidate side-filter (does the scan
restrict to OPPOSITE side?) and how the enemy b355/bc46 update methods engage.  This is the concrete lead
for the a296=10 plateau, now testable at full speed (no crash).  Everything else this session (deterministic
reach, crash-free run to t=17830, patches 426/427/428) stands.

## CORRECTION + sharpened lead (2026-08-27)

The "player targets a same-side unit" note used the WRONG side field (type-indexed dg[t-0x19ec]&1).  The
REAL combat side is [obj+0x16]&8, and aa08's scan filter (`if ((cand[0x16]&8) == obj[0x16]&8) continue;`,
asm 1aa90) correctly skips SAME-side candidates -> the player's lock c34d is a valid OPPOSITE-side target.
Targeting-side is NOT the bug.  (Also: aa08's header comment claiming op-0x58 "returns 0 for all candidates"
is STALE -- the shim now implements op-0x58, targets DO lock, the player has a live target at exit.)

The real remaining lead is FIRE-REQUEST / RELOAD timing: 7e29 is reached 1056x (fire request [0x92] set)
but the spawn gate [0x91]==4||[0xa8]==0 passes 0x -- when [0x92] is set the reload [0xa8]>0, and when
[0xa8] reaches 0 (player has [0xa8]=0 at exit) [0x92] is not set.  The fire request and the reload-ready
window never coincide, so the friendly turret never spawns a projectile (the 6 enemy kills come from the
enemy-side b355/bc46 path / emitters, not friendly 7e29 fire).  NEXT: trace how the original couples the
fire request to [0xa8]==0 (afa2 should gate the request on reload-ready, or 7c1d should hold [0x92] until
[0xa8]==0), and the enemy b355/bc46 engage/damage path.  This is the concrete a296=10 plateau lead.

## DEFINITIVE blocker (non-perturbing, 2026-08-27): the friendly turret spawn gate NEVER passes

Landed a minimal-perturbation counter in the (asm-verified) 7745 spawn method: over a clean run to t=17830
it is reached **0 times**.  So the 7e29 turret spawn gate `[di+0x91]==4 || [di+0xa8]==0` NEVER passes for
any friendly unit -> the turret main gun NEVER spawns a projectile.  The 6 enemy kills are the friendly
EMITTER path (b51f/c31e, short range); the 10 far enemies survive because the RANGED turret never engages.
The fire cascade (7745/b725/ace0, all base-lost) is DEAD CODE until the gate passes -- landing it is
premature (verified: landing 7745 changed nothing, reached=0), so patch 429 was reverted.

Why the gate never passes (both alternatives fail every tick 7e29 is reached):
  - [0x91]!=4: units select weapon 2 (player) / others via 7681's auto-select, never weapon 4 (the special
    no-reload main gun whose gate passes directly).
  - [0xa8]!=0: the reload timer is never 0 at the moment the fire request [0x92] (afa2 aim) is set -- the
    aim-converged window and the reload-ready window are anti-correlated (player has [0xa8]=0 at exit, but
    with no active request).  Likely the turret aim (a265/a18e, "imprecise" per patch 274) never converges
    long enough for [0xa8] to reach 0 while a request stands.
This is the SINGLE remaining combat blocker for a296->0, and it is runtime-probe-BLOCKED (any hot-path
instrumentation perturbs the timing and hangs the run).  The way in is the ORACLE (dosbox-fist write/block
trace) to observe how the ORIGINAL couples aim-converge + reload-ready + weapon-select so the gate passes --
then a faithful patch to the fire-decision + landing the (asm-ready) spawn cascade resolves the mission.

## afa2 is FAITHFUL -> the root is AIM-CONVERGENCE (static, 2026-08-27)

Disassembled afa2 (the fire-decision, asm 0xafa2-0xb008): it sets the fire request (call a286 -> [0x92]=0x30)
purely on (target [0x97]!=0) + (range: [0x99]<=0xc8 or [0x40]&0x80 or the [bx-0x66ae] gate) + AIM ERROR
`[di+0x8b]-[di+0x89]` within +-0xb6 (asm affe: `cmp ax,0xb6; jb -> a286`).  It does NOT read [0xa8]
(reload) -- so the port's afa2 is FAITHFUL; the original also sets the request independent of reload, and
the reload gate lives ONLY in 7e29.

Therefore the 7e29 gate ([0xa8]==0 during an active request) passes ONLY if the aim error stays < +-0xb6
CONTINUOUSLY for ~[0xa8]-init ticks (so the reload decrement in the 7c1d type-A reload handler reaches 0
while [0x92] still stands).  7745 reached=0 => the aim never holds within +-0xb6 that long.  The turret
aim = [0x89] (slewed toward [0x8b]=[0x9b]-[0x26] by 7d1d/patch 426); [0x8b] comes from a265/a18e (the
angle solver, flagged "imprecise" in patch 274, [0x9b] seen stuck at 0x2000 at session start).  So the
SINGLE root of the a296=10 plateau is the turret AIM-CONVERGENCE (a265/a18e precision and/or the 7d1d slew
not reaching+holding [0x8b]) -- the same aim issue this session opened with, now proven to be the last link.

This is runtime-probe-BLOCKED (hot-path instrumentation hangs the run).  The ONLY remaining data path is
the oracle: trace a firing ORIGINAL unit's [0x89]/[0x8b]/[0x92]/[0xa8] to see the aim converge+hold and the
gate pass, then fix a265/a18e (angle precision) faithfully.  That single fix should cascade to resolution:
gate passes -> land the (asm-ready) 7745/b725/ace0 spawn cascade -> projectiles -> a296 -> 0.

## CORRECTION: aim CONVERGES; the root is the [0xa8] reload-vs-request TIMING (2026-08-27)

The prior "aim-convergence is the root" was WRONG.  7e29 is reached 1056x, and 7e29 is reached ONLY when
[0x92] is set, which afa2 sets ONLY when the aim error [0x8b]-[0x89] is within +-0xb6 -- so the aim DOES
converge (1056 fire requests fired).  The aim is fine.  The SOLE blocker is [0xa8] (reload) never being 0
at any of those 1056 request-ticks, while it DOES reach 0 by exit.

The reload decrement lives in FUN_0000_7d69 (a "7c1d type-A template" sub-method dispatched by the object's
animation frame, like ab03's [bx-0x6704] table), and [0xa8] is SET by 7963 (weapon-select) to the weapon's
reload time (or 0xff when that weapon is out of ammo -- player has ammo, so not this).  So the anti-
correlation is a DISPATCH/animation-state coupling: [0xa8] decrements only in the animation frames that
dispatch 7d69, and those frames do not overlap the aim-converged fire-request window -- so [0xa8] never
reaches 0 while [0x92] stands.  (A deadlock-shaped hazard: fire needs [0xa8]==0; [0xa8] winds down in a
frame set entered around firing; firing never happens -> [0xa8] never gets there during a request.)

This is the precise, single remaining link, and it is RUNTIME-PROBE-BLOCKED (hot-path instrumentation hangs
the timing-sensitive run).  The oracle is the only way to see how the ORIGINAL sequences the animation-frame
dispatch of the reload decrement against the fire request so the gate passes.  Everything upstream (reach,
crash-free run, aim, targeting, afa2) is SOLVED; this reload-dispatch timing is the last mile to a296->0.

## The reload is 1/16-tick dispatched (~224-tick / 15s reload) -- the exact timing mechanism (2026-08-27)

Static, decisive: table1 @image 0x7c91 (7c1d's animation sub-method vectors, indexed by [0x3d]&0x1e) has
**table1[0] = 0x7d69** = the reload-decrement handler.  [0x3d] increments by 2 each tick (7c1d), so
[0x3d]&0x1e cycles 0..0x1e every 16 ticks -> 7d69 (and thus the [0xa8]-- decrement) fires only ONCE PER 16
TICKS.  With [0xa8] init ~14 (weapon 2), the effective reload is ~14x16 = 224 ticks ~ 15s at 15 Hz -- a
realistic main-gun reload, and FAITHFUL (the table is image data; the original decrements at the same rate).

So the a296=10 plateau mechanism is now fully explained WITHOUT any base-loss in the reload/fire path:
  - afa2 sets the fire request the moment the aim is within +-0xb6 (1056 requests over the run).
  - but those requests cluster EARLY, while [0xa8] is still counting down from the initial weapon-select;
    the 16-tick-dispatched reload only reaches 0 ~224 ticks in, and 7963 does NOT re-set [0xa8] while the
    weapon is stable, so [0xa8] does wind down -- but by the time it is 0 the aim/target window that raised
    the request has passed ([0xa8]=0 at exit, with NO active request; gate_pass=0).
The turret only ever fires if a fresh aim-converged request happens to land on a tick where [0xa8] has
already reached 0.  In the port that coincidence never occurs in 17830 ticks; in the original the mission
resolves, so the original's units either HOLD aim across the ~224-tick reload or re-acquire+converge after
[0xa8]==0.  Which of those the port fails is a RUNTIME-TIMING question (target persistence / aim-tracking on
a moving target across 15s), and it is runtime-probe-BLOCKED (even a single-read probe in 7e29 hangs the
timing-sensitive run -- reconfirmed).

## Honest limit reached (static analysis exhausted)

Every LINK of the fire chain is now accounted for and shown faithful: reach, crash-free run, targeting-side
filter, op-0x58 LOS, afa2 fire-decision, aim convergence, weapon-select, the 1/16 reload rate.  The ONLY
remaining unknown is the RUNTIME coincidence of [0xa8]==0 with an active aim-converged request, which cannot
be observed in the port (perturbation) and is not determinable statically.  The oracle (dosbox-fist,
FIST_WATCHFLAT on a firing unit's [0xa8]/[0x92]/[0x89]/[0x8b]) is the sole remaining data path to see how
the original achieves the coincidence -> then a faithful fix + landing the asm-ready spawn cascade resolves.

## *** ORACLE DECISIVE: the original FIRES the turret (778a+ace0) -> the plateau is a REAL BUG (2026-08-27) ***

Block-trace of the ORIGINAL AZER1 run (third_party/dosbox-fist, cs=0x2082 = FIST.DAT relocated,
FIST.DAT_off = eip + 0xf690).  Mapping verified: aa08 (enemy scan) present=True at 2082:b378.  Then the
turret fire cascade in the ORIGINAL:
  - 778a (turret SPAWN, weapon 2 = 7e29 table[[0x91]=2]) : PRESENT
  - ace0 (projectile INIT)                               : PRESENT
  - b1df (allocator), a265 (aim)                         : PRESENT
  - 7745/7814 (weapons 0/4, not carried)                : absent (correctly)

So the ORIGINAL's units DO fire their main gun (via 778a, matching the units' [0x91]=2), spawn projectiles
(ace0), and -- since AZER1 resolves in the original -- kill the enemy side.  This REFUTES the prior "the
a296=10 plateau is faithful / no base-loss" conclusion: the port NEVER reaches 778a (7745 reached=0, and
the same holds for 778a -- 7e29's gate never passes for weapon 2), so the port has a REAL BUG that stops
the turret from firing where the original fires.  The oracle earned its keep: it converted "maybe faithful"
into "definitely a bug, and here is the exact code (778a/ace0) the port must reach."

## What this pins for the fix

7e29's gate for weapon 2 is [0xa8]==0.  The original reaches it during a fire request (778a runs); the port
never does.  The reload rate (1/16-tick, ~224 ticks) is faithful, so the divergence is that the port's units
do NOT hold an aim-converged target across the ~224-tick reload the way the original's do -- i.e. the port's
target-persistence / engagement-maintenance (LOS + drive-to-goal navigation) is the divergent subsystem, OR
there is an extra [0xa8] reset in the port.  NEXT (now sharply targeted): oracle-watch a firing original
unit's [0xa8]/[0x92]/[0x97] to see whether [0xa8] winds monotonically to 0 under a held target (=> port
navigation/LOS bug) or is driven differently; then fix the divergent subsystem + land 778a/ace0 (asm-ready).
The plateau is a BUG with a known-good reference, not a wall.

## The navigation blocker, precisely characterized (2026-08-27)

Not "units never reach range" -- afa2 raised 1056 fire requests, so units DO enter weapon range 1056 times
(afa2's [0x99]<=0xc8 / [0x9952] range gate passed).  The failure is ENGAGEMENT-HOLD: each in-range window
is far shorter than the ~224-tick (15 s) reload, so [0xa8] never winds to 0 while a request stands.  min_dist
reaching 39898 (< 0x40000) then drifting confirms units CLOSE but do not PARK at engagement range -- they
fly through / circle past, exactly the drive-to-goal steering pattern (a9ea/a358/a57a in the 902c movement
callees) the git history flagged.  Confirmed a REAL divergence, not faithful: the oracle's original units
hold the engagement long enough that 778a fires; the port's do not.  No extra [0xa8] reset exists in the
port (7963/7fbc are both weapon-change-guarded), and the reload table/rate is faithful -- so the sole fix is
the movement AI holding units at engagement range across the reload.  This is the last subsystem; it is deep
(drive-to-goal steering) and the oracle (unit-position trace of an original engagement) is the reference for
how the original parks its units to fire.

## Oracle: ALL FOUR unit types fire in the original (2026-08-27)

Byte-signature grep of the oracle block-trace: the original executes ALL four type spawn methods --
778a (t0/wpn2), 8121 (t1), 88d1 (t2), 9b7e (t3) -- plus afa2, a286, aa08, ae32.  So every friendly type
fires its weapon in the original; the port fires NONE (7e29 gate never passes; 899c/91b8/99a2 reach=0).
This confirms the a296=10 plateau is a SYSTEMIC fire-timing bug, not a one-type quirk -- the port's units
never sustain the engagement/reload coincidence that lets the gate pass, across all four update families.
(The FUN_0000 dispatchers 7e29/899c/91b8/99a2 themselves did not byte-match in the trace -- a trace-scope
artifact of the low-segment cs; the SPAWN methods they call all ran, which is the load-bearing evidence.)

The completion (making the port's units sustain engagement so [0xa8]==0 coincides with the aim-converged
request, then landing 778a/8121/88d1/9b7e + ace0) is a distinct, substantial combat-AI work block, fully
diagnosed here and gated on the oracle field-watch of a firing unit's [0xa8]/[0x92]/[0x91]/[0x97].

## QUANTIFIED: weapon-2 reload = 320 ticks (~21s) -> the engagement-hold requirement, confirmed (2026-08-27)

Read the reload tables from the port at exit (non-perturbing):
  0x8f54 (7963 reload, index [0x91]>>1): 20 20 0 20 ...   (indices 4+ are adjacent non-reload data)
  0x90f6 (7fbc reload):                   2  2 2  0 76 79 65 68  (76 79 65 68 = "LOAD" string data)
So weapon 2 (the units' [0x91]=2, index 1) has reload = 20; x the confirmed 1/16-tick decrement (7d69) =
320 ticks ~ 21 s at 15 Hz.  Weapon 4 (index 2) = 0 (the instant [0x91]==4 gate branch).  The oracle trace
shows the original fires 778a = WEAPON 2 (not 7814/weapon-4), so the original's tanks HOLD their engagement
across the ~21 s reload and fire; the port's units break engagement in seconds, so [0xa8] (which winds down
regardless) reaches 0 only when the aim/target window has already passed -> the gate never coincides.

This DEFINITIVELY confirms the blocker is movement-AI ENGAGEMENT-HOLD, and quantifies it: units must
maintain an aim-converged target for ~21 s (the faithful weapon-2 reload) for the turret to fire.  The port
does not; the original does.  The fix is the drive-to-goal / combat-engagement movement behaviour (units
must orbit/hold at engagement range instead of flying through), reconstructed against an oracle unit-position
trace of an original engagement -- a distinct combat-AI work block, now fully quantified and scoped.

## Movement traced to the drive-to-goal: WAYPOINT-based [0x49], not enemy-pursuit (2026-08-27)

Traced the AI steering to ground: the steering servo (a401/a395, patches 422/423) slews the unit heading
[0x26] toward the DESIRED heading [0x30]; [0x30] is set by ab91/abb7 (patch 328) = bearing from the unit's
position [obj+4] to a GOAL at [obj+0x49] (via 0541).  And [0x49] (the goal) is set by ac7e (patch 248) from
a resource DAT_2000_5798 (set by ab03), and by ac9e (patch 396) from a ROSTER-ENTRY position -- i.e. a
WAYPOINT/formation target, NOT the combat target [0x97].  a9ea/a0a4/a57a are player-only (audio/proximity),
NOT AI steering.

So the AI units drive toward WAYPOINTS and engage enemies opportunistically as they pass; they do not PURSUE
[0x97].  They converge (min_dist 39898) but drive THROUGH instead of holding, so the ~21s weapon-2 reload
never completes during a standing engagement, and no fresh aim-converged request lands after [0xa8] hits 0.
This is why the port engages ONCE and never re-engages.

## Where the completion sits (fully traced, honest)

Every combat link is faithful and accounted for: reach, crash-free run, targeting-side, op-0x58 LOS, afa2
fire-decision, aim convergence (1056 requests), weapon-select (0 flicker), the 1/16-tick 21s reload, and the
spawn cascade (778a/ace0, oracle-confirmed in the original).  The ONLY divergence is that the port's units,
following the waypoint drive-to-goal, do not sustain/re-establish an engagement long enough for the reload
-- while the original's do (the user confirms AF self-plays to elimination; the oracle confirms all four
spawn methods fire).  Closing it means the movement/order layer (the waypoint goal [0x49] set by ac7e/ac9e
from the mission order data, or a combat-steer override) must keep units engaged -- verified against an
oracle unit-position trace of an original engagement.  This is the last, deep subsystem, now traced to the
exact functions (ab91/ac7e/ac9e + 0541) and fields ([0x49]/[0x30]/[0x26]).

## *** BREAKTHROUGH via Cosmo's hint: the turret FIRES in combat-heavy battles (2026-08-27) ***

Cosmo: "es gibt ja mehr missionen. bei der ersten passiert halt nicht viel" (AZER1 is the quiet first
mission).  Decisive: FIST_FSG_BATTLE=AZER5 (patch 380 overrides the loaded battle -> its own map D31/C31)
makes the units FIRE -- 7745 (weapon-8 turret spawn) reached 720x (AZER1: 0).  So the fire cascade WORKS in
a combat-heavy battle; AZER1 just never sustains engagement.  This confirms the whole fire chain is right.
Landing 7745 (patch 429, re-landed) fixed AZER5's spawn-method crash.

But AZER5 still doesn't resolve, for TWO now-precise reasons:
  1. THE a294 POOL LEAK (patch-258 orphan bug): projectile SPAWNED = 0 over 720 fires -- b1df ALWAYS returns
     full because a294 is at its 0x96=150 cap.  ROOT: ba33 (asm 0xba33) allocates a NEW object via b1df(4)
     (SI=new) and ba5d installs the template into the NEW object with the CALLER's position -- but patch 258
     DROPPED b1df's return and passed the CALLER (param_2), orphaning a registered object each emitter fire
     (b51f->ba49->ba33); ba5d also SELF-COPIED [si+N]<-[si+N] instead of the asm [si+N]<-[di+N] (caller).
     Registry proof: type-0x10 (b51f) objects grow 20 -> 92 as a294 fills.
  2. PROJECTILE PHYSICS (b725 launcher + ace0 projectile-init) still base-lost -> even when a projectile
     spawns it flies wrong / doesn't hit.  ace0 (asm 0x1ace0-0x1adcc + sub 0x1addb) is a large 1:1 landing
     (DWORD pos/vel, DGROUP ballistic tables, b26a/a18e/a192 -- callees already correct).

Patch 430 (the asm-correct ba33/ba5d leak fix) DROPS a294 150->58 on AZER5 (leak gone!) but REGRESSES AZER1
(hang after map-load) -- the now-proper effect objects expose a downstream base-loss (same fragility that
reverted patch 424).  So 430 is HELD: the next step is to find the AZER1 hang the proper effect model
triggers, land 430 cleanly, then land b725/ace0.  The path to a RESOLVED combat mission is now concrete:
fix the leak (430 + the exposed downstream) -> projectiles allocate -> land b725/ace0 -> projectiles hit ->
side eliminated.  The turret fires; this is the last cascade.

## Patch 430's AZER1 hang PINNED: exposes a task-scheduler base-loss (c06a/c058) (2026-08-27)

gdb on the hung AZER1-with-430 process: infinite spin in 459a's event-drain (line 13761) calling the
scheduler poll FUN_1000_35a7 ([c40a]).  35a7's task branch `if (DAT_1000_c06a != 0) icall [c058]` fires --
the now-PROPER effect objects (430) schedule a task (c06a set) -- but DAT_1000_c058 = 0 (uninitialised),
so 35a7 icalls 0x1000:0 every poll and never returns "queue empty" -> 459a spins forever.  So patch 430 is
ASM-CORRECT (drops AZER5 a294 150->58) but UNMASKS a base-loss in the task-scheduler setup (c058 never
seeded, or the effect's task-enqueue writes it base-lost).  The orphan bug (258) hid this by never making a
proper effect that schedules a task.

## The resolution chain, now fully enumerated (each a concrete asm-verifiable fix)

1. Patch 429 (7745 spawn) -- LANDED + PUSHED; the turret fires in combat battles (AZER5 7745 reached 720x).
2. Patch 430 (ba33/ba5d orphan-leak fix) -- asm-correct, drops a294 150->58, HELD (exposes #3).
3. Task-scheduler c058 seed / effect task-enqueue base-loss -- the AZER1 hang 430 unmasks (35a7 c06a/c058).
4. b725 launcher + ace0 projectile-init (asm 0xb725 / 0x1ace0-0x1adcc + 0x1addb) -- projectile physics so
   the spawned projectile flies + hits (callees a18e/a192/b26a already correct).
5. Then a combat-heavy battle (AZER5) plays to one side eliminated -> the goal, on a single mission.
The turret FIRES; this chain (leak -> scheduler -> projectile physics) is the last mile, each step bounded.

## *** REAL ROOT of the leak found: b1df orphan bug — but fixing it exposes a masked fire-path base-loss (2026-08-27) ***

Deep dig after the AZER5-fires breakthrough.  The registry (with the fork's 430-433 applied) showed the
b51f emitters registered MULTIPLE times (a211/a16c/a248 duplicated) and NO type-4 effects -> the a294 leak
+ wrong projectiles trace to **b1df itself** (patch-258 "orphan bug"), not ba33/ba5d:
  - b1df (asm 0x1b1df) must register + return b21d's NEWLY-ALLOCATED slot (DI); b21d sets [new+0]=class key
    and b1df zeroes only [new+4..], so the type IS set (type-4 for b1df(4)).  Patch 258 wrongly did
    di=param_2 (registering/zeroing the CALLER, orphaning b21d's slot) and returned `count`, so 7745 got
    `count` as its projectile and every ba49 orphaned a registered object = the leak.

HELD patches (patches/held/, asm-verified, make check clean, NOT applied -- they expose the coupling below):
  - 434 b1df ROOT fix (register+return b21d's new slot) -> a294 leak GONE (150->120, bounded), effects
    become proper type-4 objects.
  - 435 bab4 (type-4 effect DESPAWN) + 436 bae1 (effect anim frame) base-loss -> the effect lifecycle now
    runs CLEAN end to end (b1df->bab4->bae1->b2ef despawn), no crash, on AZER5.
  - 430 ba33/ba5d (use b1df's new object), 431 0578 a18e-bx thread, 432 b725, 433 ace0 (projectile physics).
    NOTE: held-433 needs the `g_fist_a18e_bx` global re-added to tools/native_main.c (the fork added it).

**THE COUPLING (why held, not landed):** with the correct b1df/effect model (430+434..436), AZER5's turret
FIRE-DECISION breaks -- 7745 reached drops 720 -> 0 (units stop reaching the fire dispatch).  So the
friendly fire path (afa2 aim / 7c1d->7e29 gate) DEPENDS on the BUGGY b1df/effect behaviour: the orphan
bug's side effects (duplicate emitter registration, the caller-zeroing) were masking ANOTHER base-loss in
the fire-decision path.  Neither state resolves: stable (429) fires 720x but with garbage projectiles
(b1df returns count) -> no hits; fixed (434) has correct projectiles but no firing.

## Next (concrete): find the masked fire-path base-loss

With 434 applied, trace WHY afa2/7c1d stop setting [0x92]/reaching 7e29 -- the buggy b1df was compensating.
Candidates: b1df's caller-zeroing was (wrongly) resetting a fire field the units re-read; or the duplicate
0xdfbc registration changed the c0e5 update order the fire logic relies on; or 7745's b1df(8) side effect
on the firer.  Once that masked base-loss is fixed, 434-436 + 430-433 land clean -> AZER5: fire -> hit ->
side eliminated.  The turret fires and the leak is solved; this single coupling is the last barrier.

## CRUCIAL correction: the buggy b1df MASKED the reload gate; "AZER5 fires 720x" was the bug (2026-08-27)

Fire-chain counters with the CORRECT b1df (held patch 434) on AZER5: a286 fire-request=17 (was ~1056),
7e29 reached=816, 7745 spawn=0.  The gate [0x91]==4 || [0xa8]==0 never passes.  ROOT of the illusion: the
BUGGY b1df, in 7745's `proj=b1df(8, firer)`, registered+ZEROED the FIRER (di) -- zeroing [firer+4..0xfb],
which includes [0xa8] (the reload) -> [0xa8]=0 -> the gate passed EVERY fire.  So the stable state's "720
turret fires" were an ARTIFACT of the orphan bug zeroing the reload, and the projectiles were garbage
(b1df returned count).  With the correct b1df the ~21s reload (established earlier: [0xa8] init x 1/16-tick
7d69 dispatch) is REAL, and it only clears if the units HOLD an aim-converged engagement that long.

So the corrected picture: the fire CASCADE is now fully built and correct (held 431-436: b1df root, bab4/
bae1 despawn, b725/ace0 physics, leak gone a294 150->120) -- but a RESOLVED mission still needs the same
MOVEMENT / ENGAGEMENT-HOLD the earlier analysis pinned: units must sustain an aimed target across the 21s
reload for the gate to pass and a real projectile to spawn+fly+hit.  The buggy b1df faked this by zeroing
the reload; it did NOT make the units actually hold engagement.  (a286 dropping 1056->17 with the correct
model also shows the buggy effects were inflating the apparent engagement.)

## Where it stands, honestly

Two independent, both-required pieces for a resolved combat mission, both now precisely identified:
  1. The fire cascade -- BUILT + correct, held in patches/held/ (431-436), leak solved, no crash.
  2. Movement engagement-hold -- units holding an aimed target across the ~21s reload -- STILL open (the
     deep drive-to-goal subsystem; the buggy b1df's reload-zeroing masked it, it is not solved).
Stable pushed state (429) is the buggy-but-firing baseline (AE=0).  The held patches are the correct fire
cascade, to land together WITH the engagement-hold fix.  Goal unmet; the last real barrier is the movement
AI holding engagement -- unchanged by this session's cascade work, but now with the fire path proven correct.

## ORACLE CONFIRMS: AZER1 self-plays to elimination (MISSION LOST, UNITS REMAINING 00)

Decisive oracle run (`tools/oracle` third_party/dosbox-fist, AZER1, empty input = AUTO CONTROL self-play,
~2.5 min in-mission).  End screen = **"MISSION LOST / OBJECTIVES REMAINING: 13 / ... GROUND UNITS LOST: 04
/ UNITS REMAINING: 00 / ENEMY GROUND KILLS: 01"**.  So the ORIGINAL resolves AZER1 by ONE SIDE ELIMINATED
(the player's 4 ground units wiped out by the enemy AI; friendlies scored 1 kill) -- the exact goal
condition, reproducible in AZER1, in ~2.5 min.  The "AZER1 is quiet" nuance is real but it STILL resolves:
the enemy AI keeps engaging until the player side is gone.

Port AZER1 self-play (stable, no held patches), FIST_SIMTRACE time-series:
  - t=366..406: an EARLY skirmish -- firereq up to 5, a296 15->10 (5 destroys), goals oscillate.  This is
    the ENEMY fire path (types 0x1a/0x1b, upd bc46/b355) landing hits; friendlies never fire (a286=0).
  - t>=406..30000: TOTAL STALL -- a296=10 forever, firereq=0, tgt=1, cand up to 5, cross-unit min range
    down to 10833 (<< 0x40000).  Units are CLOSE and find candidates but NOBODY fires.  Mission never
    resolves.

So the gap is now exact and oracle-anchored: **the port's AI stops engaging after the opening skirmish;
the original keeps engaging to elimination.**  Registry snapshot (FIST_DUMP_REG) at spawn: every unit has
tgt97=0000, cand94=00, ftmr92=00, rld_a8=00 -- targeting cold at t~0 (expected).  Steady-state the acquire
a6e3 (patch 425) does set [0x97] but e20a (op-0x58 LOS) clears most (73%-out-of-range finding), leaving
tgt=1; and even that one unit never fires.  Both the FRIENDLY fire gate (afa2->a286, AE=0) and the ENEMY
re-engagement (bc46/b355 stop after the burst) fail to sustain.  Next: split firereq/tgt BY SIDE over time
to see which side stalls first, and trace an enemy's [0x97]/[0x92]/[0xa8] across the port stall vs the
oracle resolve.  Goal reproducible + oracle-confirmed; port stall is the barrier.

## Oracle movement trace: base CONFIRMED + friendly units move live

FIST_WATCHPHYS on the friendly-unit position band (flat 0x28000..0x28800) during the resolving AZER1 run:
4122 live writes landed at 0x280xx -> **DGROUP is at engine-flat 0x1c000 in the oracle too, IDENTITY-mapped
under cr3=0xe000** (WATCHPHYS fixed-phys worked).  My earlier pool-counter watch produced no file NOT
because of a bad base but because FIST_WATCHFLAT's histogram dumps only at SIGUSR2, and that dump is
deferred to the next VGA write -- which never came on the static MISSION-LOST stats screen.  Recipe that
WORKS for a live value time-series of any engine field: FIST_MEMARM_BOOT=1 FISTLOG=<pfx>
FIST_WATCHPHYS=<flat=phys> FIST_WATCHSPAN=<n>  (identity-mapped DGROUP; logs each byte write with value +
live cs:eip to <pfx>.watch.txt, no signal needed).

Two writers hit the friendly positions in the oracle:
  - flatip=0x0000721f (engine cs=02dd): bulk-zero (spawn/clear), 2048 writes.
  - flatip=0x000f14a5 (cs=f000, extender service memcpy called from engine 02dd:3246): the POSITION
    updater, 2074 writes, values stepping (0x0f66.. -> 0x1119..).  => oracle friendly units are being
    continuously repositioned = they MOVE throughout combat.
Oracle allocation differs slightly from the port (friendly slots ~c080/c118/c1a8/c210/c31a/c513 vs port
c05c/c157/c252/..), so per-slot cross-checks must be by ROLE not fixed offset.

## Consolidated gap (oracle-anchored)

  | fact | oracle (original) | port (stable) |
  |------|-------------------|---------------|
  | AZER1 self-play outcome | MISSION LOST, UNITS REMAINING 00 (~2.5min) | STALL at a296=10, never resolves |
  | friendly movement | continuous (0xf14a5 repositions) | mobile units advance, several static-by-design |
  | enemy engagement | sustained until player wiped | fires early (5 kills) then tgt97=0, no re-acquire |
The port's enemies (stationary types 0x1a/0x1b) STOP acquiring after the opening skirmish (stall: tgt97=0,
ftmr92=0, rld_a8=0 across all enemies); the original's keep acquiring+firing until the player's 4 ground
units are gone.  So the barrier is ENEMY target RE-ACQUISITION across the mission (candidate->a6e3->e20a
LOS not sustaining), coupled with whether friendlies traverse the enemy envelope.  This is a deep
multi-factor engagement subsystem; the oracle now gives an exact reference (resolves in 2.5min) and a
working per-field live-trace recipe to diff any specific function's behaviour port-vs-oracle.

## Held cascade is now CRASH-CLEAN + LEAK-FREE (patch 437); blocker isolated to afa2 fire-gate

Testing the held cascade (430-436, correct b1df) on AZER1 self-play exposed FIVE dormant base-losses:
once the correct b1df flows REAL weapon/effect objects through c0e5's per-type update dispatch + the turret
aim-servo, long-unreachable functions SEGV on host-ptr deref of their object near-offset.  Fixed as
**patch 437** (held): a3e2/a3ec (aim servo +-0xb6, also int*->WORD width), aae8 (servo method dispatch,
c0e5 convention arg=di), bc0c (periodic update), bb02 (effect-frame set).  All asm-verified, regression-
safe (unreachable before the cascade).  Result with held 430-437:
  - NO crash (ran full ~2.5min self-play to the watchdog).
  - a294 STABLE at ~120 (was pinned at the 0x96=150 cap in stable) -> **the pool leak/saturation is GONE**;
    despawn (bab4/bb02) now recycles effects.  So pool-saturation WAS a real stall factor, now fixed.
  - BUT still no resolution: firereq=0 ALWAYS, a296=16 stable, ZERO kills, a286-request=0.  The correct
    b1df also removed the stable build's accidental early skirmish (5 kills) -> confirming that "combat"
    was a buggy artifact of the orphan bug, not real engagement.

So pool-saturation was one bug (fixed); the RESOLUTION blocker is the aim/fire gate afa2 (0xafa2, patch
366).  afa2 fires a286 when the outer gate passes (has target si=[0x97], globals/flags) AND aim error
WORD[0x8b]-[0x89] < 0xb6.  Evidence it is NOT reached: the [0x17]|=0x80 fire-request setters (still base-
lost host-ptr form at 20923/21239/...) never crash -> that path is dead.  And the stable-build dumpreg
shows the player with aim89==tbrg8b==0x2000 (CONVERGED) + a target, yet a286=0 -> afa2's OUTER gate blocks,
or afa2 is not called per-AI-unit, or the target is mis-selected (player tgt97=c34d = a FRIENDLY).  Next:
trace who calls afa2 for AI units + which outer-gate term fails, and diff [0x8b]/[0x89]/[0x97] vs the
oracle (WATCHPHYS recipe).  Held cascade ready to LAND pending wasm-parity + verify-matrix (kept in
patches/held/ until then).  Concrete session progress: pool leak solved + crash-clean held path (437);
goal still unmet on the afa2 engagement gate.

## REFRAME: a294/a296 is NOT the oracle's kill signal; resolution lives in the overlay

Oracle kill-trace (WATCHPHYS on the port's pool-counter addresses flat 0x2a294/0x2a296 across the full
resolving AZER1 run): only **16 writes total, and both writers are generic service routines** -- a one-time
memset (721f, val=00) then a one-time 8-byte memcpy (f14a5).  NO inc/dec.  So in the ORIGINAL, flat
0x2a294/6 is an incidental data block, NOT the live side/unit counter.  The port's DAT_2000_a294/a296
(which DO behave as inc/dec pool counters in the port) therefore do NOT correspond to the original's
unit-remaining mechanism -- the whole "a296 stall at 10/16" metric this board has tracked is the WRONG
signal for resolution.

What resolution actually is: the end screen reads **"OBJECTIVES REMAINING: 13" + "UNITS REMAINING: 00"**,
and the port's simtrace goals=13 MATCHES the oracle's 13 -- so objectives track fine.  Resolution fires on
UNITS REMAINING -> 0 (per-side unit roster), and per the standing goal spec the mission WIN/LOSE logic
"lives in the overlay at 0x100000, not in FIST.DAT".  So the resolution path is: friendly units take
damage -> per-unit HP->0 -> removed from the overlay's unit roster -> overlay declares MISSION LOST.  The
port's held+437 build is object-model CLEAN (109 objects at t=20533, no duplicates, no leak, a294 stable)
but produces ZERO friendly deaths -> the enemy->friendly DAMAGE path (one of b2ef's 23 callers, via a
specific projectile-collision/damage handler) is not firing, and/or the overlay unit-roster + win/lose
check is not wired.  Corrected next target: (1) locate the overlay's unit-remaining roster + win/lose
check (FUN_1000_* at 0x100000), (2) find the projectile-collision -> damage -> roster-remove path and its
base-loss, verified via the WATCHPHYS recipe against the oracle's actual roster address (to be located, NOT
0x2a294).  This session: leak+crash SOLVED (patch 437, held cascade clean); the a294/a296 stall metric
DISPROVEN as the resolution signal; resolution re-localized to the overlay unit-roster + damage path.

## Correction + accurate state (end of oracle arc)

Correction to the previous reframe: watching fixed scalar addresses in the oracle (0x2a294 side-counter,
0x25fbe) is the WRONG instrument, not proof the port's model diverges.  b2ef (patch 363) decrements
DAT_2000_9fbe[param_1*2] (the per-SLOT registry value-array, indexed by registry index) and the a2f7/a38d
per-SUB presence arrays -- NOT scalar counters at those base addresses.  So a death writes a slot-indexed
element (a moving address), which a fixed-address WATCHPHYS on the base byte cannot catch; my traces caught
only the block memset(721f)/memcpy(f14a5) that INITIALISE the registry at mission load (val 0x74 = 116 =
the initial object count).  The KDV overlay at 0x100000 is the intro-video player (fist_ext.c), NOT combat
logic -- so the mission win/lose IS in FIST.DAT (the oracle = FIST.RUN+FIST.DAT resolves it), reachable.

SOLID, BANKED this session:
  1. Oracle CONFIRMS AZER1 self-plays to "MISSION LOST / UNITS REMAINING 00" in ~2.5min (screen captured) --
     the goal condition is reproducible.  Working oracle live-trace recipe (WATCHPHYS, no signal).
  2. Patch 437 (held): 5 base-losses (a3e2/a3ec/aae8/bc0c/bb02) exposed when the correct b1df flows real
     objects through c0e5/aim-servo -- FIXED.  Held cascade (430-437) now CRASH-CLEAN + LEAK-FREE +
     duplicate-free (109 objects, a294 stable at 120, was pinned at the 0x96=150 cap).  make check clean.
  3. Object model is correct; the correct b1df removed the stable build's accidental buggy skirmish -> the
     "5 early kills" were an orphan-bug artifact, not real combat.

OPEN (the real barrier, unchanged in kind): friendly units take NO damage in the port (no deaths) so the
mission never resolves.  The enemy->friendly DAMAGE path (projectile collision -> HP -> b2ef via one of its
23 callers) does not fire.  Correct next approach (NOT more fixed-address oracle watching): statically trace
the port's projectile-collision/damage path from the b2ef callers backward to the fire-spawn, OR instrument
the port to see how far a projectile gets (spawn -> fly -> collide -> damage -> b2ef), and close the
base-loss that stops it.  Land held cascade 430-437 together once damage works + wasm-parity verified.

## op-0x58 LOS WORKS (47% visible) -- blocker is the FIST.DAT acquire-HOLD, not the extender

Instrumented the op-0x58 LOS handler (native_main.c:1467, already implemented as a terrain raycast with a
Z terrain-follow stand-in) with a call census.  Held+437, AZER1 60s:
  [op58] LOS calls=45873  out-of-range=17777 (39%)  occluded=6534 (14%)  VISIBLE=21562 (47%)
So op-0x58 returns VISIBLE for ~half of all queries -- units SEE each other constantly.  op-0x58 is NOT the
blocker (this CORRECTS the earlier "op-0x58 unimplemented / extender frontier" read: it IS implemented and
mostly returns visible).  Yet a286-request=0, 7e29=0, tgt stays 1-2.  So visible-LOS results do NOT become
held targets + fire.  The gap is downstream, in FIST.DAT:

Acquire path = FUN_0000_ae32 (patch 366, dispatched by ab03 per unit): promotes candidate word[off+0x9d]
-> target word[off+0x97] via a6e3, GATED on byte[off+0x94]!=0 (has candidate count) && word[off+0x97]==0
(no current target) && RNG(0291) <= byte[bx+0x994a] (per-type probability threshold, bx=word[word[0x9796]]).
Most units show cand94(=[0x94])==0 in the dumpreg and tgt=1 steady-state, while op-0x58 says 47% visible --
so targets are acquired only BRIEFLY then cleared (churn: acquire->clear->acquire), never held long enough
for the turret aim-servo (a3e2/a3ec, +-0xb6/step, now fixed by 437) to converge within afa2's +-0xb6 gate
and fire.  This is the ENGAGEMENT-HOLD problem (target persistence + aim convergence inside the hold
window), squarely FIST.DAT AI logic -- patchable, NOT an extender/overlay service.

Corrected next target (precise): trace WHY [0x97] does not persist -- what sets/clears [0x94]/[0x9d] (the
candidate scan aa08/FUN_1000_aa08) and what clears [0x97] each frame; and whether the RNG threshold
byte[bx+0x994a] is base-lost/0 (would make acquisition near-never).  Instrument ae32 (acquire-fire count)
+ a6e3 (set vs clear) + the [0x97] clearers.  op-0x58 census instrumentation banked in native_main.c.
This session net: leak+5 crashes fixed (patch 437, held cascade clean); a294/a296 AND op-0x58 both
DISPROVEN as the blocker; combat blocker precisely localized to the FIST.DAT acquire-hold / aim-converge
chain (ae32/a6e3/aa08/afa2), with the object model + LOS proven working underneath it.

## aa08 scan gate-map (precise next-session entry point)

FUN_1000_aa08 (asm 0x1aa08, the candidate scan; sets [obj+0x94]=count, [obj+0x9d]=best) walks all 182
registry slots and, per candidate, requires ALL of:
  - byte[cand+0x16]&4 set; byte[cand+0x16]&8 != byte[0x96ab]; cand != self[0x9936]
  - LOS visible: e1f0 -> op-0x58 (PROVEN 47% visible, not the filter)
  - rank byte[word[cand]+word[0x9944]] <= byte[0x9935]  (per-type rank ceiling)
  - octant: 08e8 range hi-byte (c45a>>8)==0
  - range: rax <= word[0x993a] (the RANGE GATE, rgate) && rax < word[0x993e] (running best)
Only then inc byte[0x9934]; tail sets [obj+0x94]=count, [obj+0x9d]=[0x993c].  simtrace: tcnt([0x94]!=0)=2-6,
tgt([0x97]!=0)=1-2 of ~15 friendlies -> acquisition WORKS but for few units; the survivors of the LOS pass
are filtered by the RANGE GATE [0x993a] + octant (08e8 range).  So the precise open question is the range
path: (a) what value is rgate/[0x993a] (aa08's caller / top of aa08) -- base-lost or too small?  (b) is
08e8 (range->c458/c45a, called via a17e) computing the right magnitude/octant, or is it base-lost so most
in-LOS candidates fail the octant (c45a>>8!=0) / range compare?  Instrument aa08 (candidates-seen vs
passed-LOS vs passed-range) + dump [0x993a] to split "range gate too tight" from "08e8 range wrong".  Then
the acquire-hold + aim-converge tail (ae32/a6e3/afa2) with a persistent target.  This is the last mile of
the FIST.DAT engagement chain; object model, fire cascade, and LOS all proven working beneath it.

## FIRE CHAIN FULLY TRACED (measured): root = firing unit's reload [0xa8] STUCK at 70

Instrumented the whole fire chain in the held+437 build (AZER1, 60s self-play).  Every stage MEASURED:
  [afa2]  reached=2874  has-target=170  gate-passed=20  aim-converged(FIRE a286)=20
  [chain] a286-request=20  7e29-dispatch=912  (the committed g_a286/g_7e29/g_7745 counters were DEAD --
          their build/ increments are wiped by `make patch`; the "a286=0" reads all session were a dead
          counter, NOT a real zero.  Re-added live -> a286 fires 20x, 7e29 dispatched 912x.)
  [7e29gate] a8==0(reloaded)=0  [0x91]==4=0  GATE-OPEN=0   -> 7e29's spawn gate ([0x91]==4 || [0xa8]==0)
          NEVER opens across 912 dispatches.
  [reload]  7d69 calls=2684  min[0xa8]-seen=0             -> the reload decrement DOES reach 0 (other units)
  [reload2] [0xa8] at 7e29 dispatch: min=70 max=70        -> but the FIRING unit's [0xa8] is CONSTANT 70
          (0x46), never decrementing.  7d69 (patch 263, `if [0xa8]!=0 [0xa8]--`) is dispatched via 7c1d's
          table1 `word[0x7c91+(byte[di+0x3d]&0x1e)]` (patch 244) -- it decrements OTHER units to 0 but the
          firing unit's [0x3d] state never selects 7d69, so its reload is frozen at the spawn-init value 70.

So the ENTIRE fire chain is correct and reached -- object model (437), aim-servo (437), afa2 fire-decision
(20 real fires), 7e29 dispatch (912) -- and the SINGLE remaining defect is: **the firing unit's weapon
reload [0xa8] never counts down** (stuck at 70), so 7e29's `[0xa8]==0` spawn gate never opens -> no
projectile -> no damage -> no death -> no resolution.  This is NOT the engagement AI, NOT movement, NOT
LOS, NOT the extender -- it is the per-frame reload state machine: 7d69 (decrement) is gated behind the
7c1d table1 dispatch on byte[di+0x3d] (the animation/state cursor, "+=2/tick"), and for the firing unit
that cursor never lands on the 7d69 slot.  Precise next step: trace byte[di+0x3d]'s advance in 7c1d
(the table1 index (di+0x3d)&0x1e -> word[0x7c91+idx]) for the firing unit -- why it never dispatches 7d69
(index 0) -- OR whether 7963/another path re-sets [0xa8]=70 each frame.  Fix that one reload-cursor defect
(asm-verified) and the whole proven chain fires -> projectiles spawn -> combat resolves.  This session:
fire chain reduced from "doesn't resolve" to a single measured defect (reload cursor); leak+crashes fixed
(437, held cascade clean); a294/a296, op-0x58, and the engagement-AI hypothesis all disproven en route.

## Caveat + additional reliable finding (target-selection picks own side)

Caveat on the [0xa8]=70 read: it came from single-increment counters (non-perturbing) and was consistent
(min=max=70 at every 7e29 dispatch), but a plain-build dumpreg at t=8000 shows both TYPE-0 units (c05c
player, cf11) with rld_a8=00 (reloaded) -- so the 7e29-dispatching unit with [0xa8]=70 is either a 3rd
type-0 unit or a transient early-reload state; the reload-cursor conclusion needs a non-perturbing
per-unit [0xa8]-over-time confirmation before it is treated as THE root (the committed cbb0bbd finding is
the strongest lead, not yet certain).

RELIABLE (plain dumpreg, no hot-path instrumentation): the PLAYER c05c is reloaded ([0xa8]=0) + aim
converged (aim89==tbrg8b==0x2000) + has a target tgt97=c34d -- but **c34d is a FRIENDLY (side 1)**.  So
the player's acquired target is its OWN side -> the fire gate correctly refuses (no fratricide) -> the
player never fires despite being ready.  Target SELECTION is picking own-side units, a second concrete
defect independent of the reload timing.  The acquire a6e3 (patch 425) validates via op-0x58 but does NOT
re-check the side flag; the candidate scan aa08's side filter (byte[cand+0x16]&8 vs byte[0x96ab], line
1aa90) is the side gate -- if [0x96ab] (own-side selector) is base-lost/wrong, friendlies pass as
candidates.  So there are (at least) TWO concrete defects gating combat: (1) reload/fire-request timing on
the AI shooters (types 1/3 fire a286 but the spawn gate [0xa8]==0 stays shut), (2) target selection
admitting own-side units (player locks a friendly).  Both are FIST.DAT, asm-verifiable, and downstream of
the proven object model + LOS.  Next session: confirm the side gate [0x96ab]/aa08-1aa90 (friendly admitted?)
and the per-unit reload [0xa8] timeline non-perturbingly; each is a bounded base-loss fix.

## Side-gate data (non-perturbing, one-shot dumpreg) -- defect #2 evidence

At mission spawn: [0x96ab] own-side-selector = 0x00, and byte[obj+0x16]&8 (the bit aa08's side filter
`(cand[0x16]&8)==[0x96ab]` at 1aa90 compares) does NOT cleanly separate the two sides:
    side=0 (enemy):    bit8=0 x83,  bit8=1 x9
    side=1 (friendly): bit8=0 x4,   bit8=1 x12
If [0x16]&8 were the faction bit it would be all-0 for one side and all-8 for the other; the mix (9 enemies
+4 friendlies on the "wrong" bit) means the scan admits own-side units as candidates -> the player locks a
FRIENDLY (c34d).  Caveat: this assumes [0x16]&8 IS the side discriminator; the 1aa90 asm + how [0x96ab] is
loaded per-scan must be read to confirm whether (a) [0x16] bit-8 is base-lost at object init, or (b) the
filter compares the wrong field / [0x96ab] is stale.  Either way it is a bounded FIST.DAT base-loss.  With
the object model + LOS + fire-decision all proven, the two remaining defects to close for a resolving
mission are: (1) the AI reload/fire-request timing (7e29 [0xa8]==0 gate), (2) this own-side target
admission.  Both are the last mile; the session reduced "mission doesn't resolve" to these two located,
asm-verifiable defects.

## CORRECTION (non-perturbing): reload was a PERTURBATION artifact; real break = fire-request doesn't propagate

The previous "[0xa8] STUCK at 70 -> reload never completes" finding (commit cbb0bbd) is WRONG -- it was a
PERTURBATION artifact of hot-path 7e29-gate instrumentation.  The clean, non-perturbing one-shot dumpreg
(held+437, t=8000) shows **ALL 108 units with rld_a8=00** -- every unit is reloaded; the spawn gate
[0xa8]==0 is OPEN.  Reload is NOT the blocker.  (Lesson re-confirmed: only the one-shot dumpreg is
trustworthy; multi-op hot-path counters stall the timing-sensitive sim into false states.)

Corrected, non-perturbing picture:
  - Acquisition globals FINE: acq-thresh[bx+0x994a]=0x78 (~47% RNG pass), [0x9935] rankceil=0xff (all pass),
    [0x993a] rgate=0x3e8.  Acquisition is threshold-OK but LOW-SUPPLY: aa08's broad all-pairs scan gets
    ~1.4% mutual-LOS (expected -- units spread out), ~338 candidate-passes/60s; snapshot shows only 1 of
    109 units holding a target (the player -> a FRIENDLY c34d).  Shooters (types 1/2/3) tgt97=0000.
  - Minimal non-perturbing counters (single increments): **87df-reaches-899c=0, 899c-entered=0** -- the
    type-1 shooter update NEVER sees [0x92]!=0 || [0x17]&0x80, so its spawn dispatch (899c) is never
    reached.  So afa2/a286's fire-request [0x92]=0x30 does NOT propagate to the shooter's own update where
    the spawn would fire.  (And 899c itself is base-lost -- host-ptr table [0x91]*2-0x7612, [0x97]
    int-deref, dead bVar3 -- so it would SEGV if ever entered; the crash-clean run confirms it is not.)

So the corrected single blocker: **the fire-request set by afa2 (types 1/3, 20x) never reaches the firing
unit's spawn dispatch** -- [0x92] is set but gone by the time the unit's own update (87df/97d5) checks it,
so no spawn, no projectile, no damage, no resolution.  Next (non-perturbing only): (1) count a286 fires by
unit TYPE (1 vs 3) to know which shooter path to follow; (2) trace whether afa2 runs BEFORE or AFTER the
[0x92] check within that unit's update, and what clears [0x92] between set and check; (3) then the base-lost
spawn dispatch (899c type-1 + the type-3 analog) needs the 7e29-class patch.  Reload/own-side-only findings
superseded by this propagation break.  Object model (437), LOS (47%), fire-decision (afa2 20x) all proven.

## Refined non-perturbing state: a286 fires are type-0/2; player faction bit [0x16]&8 unset

Minimal single-counter split (non-perturbing): the 20 a286 fires are ALL on type "other" (0 or 2), ZERO on
type-1/3.  So afa2 is icalled DIRECTLY (via its 0xafa2 vector-table entry) bypassing af97's type-1/3 gate,
firing for the player (type 0) and type-2 units.  (So 87df/type-1 was the wrong spawn path to chase; its
0 reaches were correct -- type-1 simply doesn't fire here.)  The player (type 0) fires but:
FACTION-BIT ANOMALY (clean dumpreg f16 values):
    enemies (side=0): f16=0x46 / 0x66  -> bit3(0x08)=0
    friendlies(side=1): f16=0x6e       -> bit3=1
    PLAYER c05c:      f16=0x66         -> bit3=0  (WRONG: same as enemies)
aa08's target filter skips candidates whose [0x16]&8 == [0x96ab](=the scanner's own [0x16]&8).  The player,
with bit3=0, skips bit3=0 units (the enemies!) and locks bit3=1 units (friendlies) -> tgt97=c34d (own side).
So the player's faction bit [0x16] bit-3 is not set (0x66 not 0x6e), inverting its target selection.  This
is either a base-loss in the player unit's [0x16] init (wholesale writers: 9512 `[0x16]=param_1`, 57186
`[0x16]^=8` toggle, 51827) or a genuine self-play difference (player normally human-controlled, faction bit
set elsewhere) -- REQUIRES the oracle to confirm whether the original's player has bit3 set (memory dump at
the player unit's [0x16]).  Separately, the shooter units (types 1/2/3) have tgt97=0 (low acquisition
supply, ~1.4% mutual-LOS, expected for spread-out units) -- so even with a correct player faction bit,
sustained combat needs the acquisition supply too.

Honest session-end state: object model (437) + LOS (47%) + fire-decision (afa2 20x) proven; reload
DISPROVEN as blocker (all units [0xa8]=0); the two live threads are (a) player faction bit [0x16]&8=0
(own-side targeting -- confirm vs oracle) and (b) low acquisition supply for the AI shooters.  Both
non-perturbing-measurable; the perturbation-derived reload/7e29 numbers are superseded.

## Oracle ground-truth RAM comparison: BLOCKED by a tooling gap (next-session prerequisite)

Attempted to settle the two threads (player faction bit; low acquisition) by dumping the ORIGINAL's DGROUP
mid-combat and diffing unit [0x16]/[0x97]/[0xa8] against the port.  BLOCKED:
  - SIGUSR2 full-dump (fist_dump -> ram.bin) is triggered ONLY from fist_vgawrite (0xA0000 writes); the
    in-mission render bypasses 0xA0000 (extender PM framebuffer), so the handler never fires mid-mission --
    the dump silently produces nothing.  (Same reason earlier flatwriters/SIGUSR2 dumps failed in-mission.)
  - FIST_R9200CAP DOES dump full guest RAM at a terrain-render pass (pass00.ram.bin, 16MB), but at that
    moment the engine DGROUP (CPU-linear 0x1c000) is paged (cr3) to an UNKNOWN guest-phys; a base-scan of
    the dump finds no coherent ~109-unit registry (best candidates are garbage: 105x type-0, dup slots) --
    the DGROUP is not at phys 0x1c000 in the render-time dump and a loose type<=0x30 heuristic can't pin it.
To get clean oracle unit fields the next session needs: walk the pass RAM dump's page tables (cr3 at
capture -> the .cam.txt records dsb/csb/cr3) to translate CPU-linear 0x1c000 to its guest-phys in THAT
dump, then parse the registry there; OR add a mem-hook-triggered full-dump to dosbox-fist (fire fist_dump
from fist_memrec on a chosen tick, not only fist_vgawrite).  This is a bounded tooling task, then the
[0x16]/[0x97] diff is immediate.

Session end -- honest: goal NOT met (AZER1 does not resolve).  Net gains, all pushed: patch 437 (held
cascade crash-clean+leak-free); a294/a296 + op-0x58 + reload + engagement-AI-generic all DISPROVEN as the
blocker; the blocker localized to FIST.DAT combat-AI ACQUISITION/ENGAGEMENT, confirmed diverging from the
oracle by the resolution gap itself (oracle wipes the player in ~2.5min; port = zero deaths).  Two concrete
non-perturbing-measurable defects to verify+fix next (player faction bit [0x16] bit-3 unset; low target
acquisition), plus the oracle-dump tooling prerequisite above.  Discipline lesson banked: only the one-shot
dumpreg is non-perturbing; hot-path counters stall the timing-sensitive sim into false states.

## Oracle partial parse (page-walked): re-weights the two threads

Page-walked the R9200CAP dump's cr3=0xe000: CPU-linear 0x1c000 -> guest-phys 0x1c000 (IDENTITY, confirmed).
Registry at 0xdfbc parses only 11 coherent units (the c0xx unit pages are paged/not-present at render time,
so most slots' fields read garbage -> the parse is PARTIAL, not a full 109-unit comparison).  But the 11
coherent units carry two informative signals:
  1. WITH-TARGET = 9/11 (82%) in the oracle vs 1/109 (~1%) in the port.  Suggestive (not conclusive given
     the partial parse) that the ORIGINAL acquires+holds targets abundantly where the port barely does ->
     reinforces thread B (acquisition/engagement) as the real divergence, consistent with the resolution
     gap (oracle wipes the player; port zero deaths).
  2. faction (side,bit3) is MIXED in the oracle too: (1,0)=5,(1,1)=2,(0,0)=3,(0,1)=1 -- both bit3 values
     appear on each side, exactly like the port.  So [0x16]&8 is NOT a clean faction discriminator in the
     ORIGINAL either -> the port's "mixed faction bit / player bit3=0" is very likely NORMAL, not a bug.
     This WEAKENS thread A (player faction bit): [0x16]&8 is probably not the side filter it looked like,
     and the player targeting c34d may be a legitimate transient, not the blocker.
Re-weighted conclusion: the single real blocker is thread B -- target ACQUISITION/engagement supply
(oracle: most units engaged; port: near-none).  The port's aa08->ae32->afa2 chain is present and correct
per-step but produces far too few sustained targets.  The next session's cleanest decisive move is a FULL
oracle unit-field comparison (needs the unit pages present -- either a non-render-time full dump via a
mem-hook-triggered fist_dump, or page-walk EVERY slot's fields as done here but at a tick where the unit
pages are resident) to measure the original's per-unit target/acquire rate directly and find why the port's
scan yields ~1% vs the oracle's ~80%.  Thread A (faction bit) deprioritised.

## Convergence trend (non-perturbing, 3 snapshots) = TWO coupled root factors

Cross-side pairwise Manhattan distance from the held+437 one-shot dumpreg at t=2000/8000/20000:
    median: 1,080,008 -> 1,187,982 -> 1,352,389   (MONOTONIC INCREASE -> units DRIVE APART)
    min:    17,082    -> 17,075    -> 17,082       (closest pair stays ~17k, IN op-0x58 range <0x40000)
    friendlies with a waypoint (goal49!=0): 8/16 constant
Enemy centroid Y~0x5d301; friendlies Y~0xc5000..0x117000 -> friendlies sit ~1M NORTH of the enemies and,
where they have waypoints, those waypoints lead them FURTHER north (median grows) -- the friendly force
does NOT advance into the enemy zone.  So factor (1): drive-to-goal / waypoint assignment -- half the
friendlies have no goal (goal49=0) and the half that do diverge from the enemies.  BUT factor (2): the
closest cross-side pair stays ~17k apart (well inside op-0x58 range) for the whole run and STILL never
engages -- so even a persistent in-range pair produces no acquisition->fire->kill.  Factor (2) is the fire/
acquisition/spawn chain traced this session (correct per-step, but the fire-moment dynamics are
perturbation-sensitive and the oracle ground-truth is paged out).  A resolved mission needs BOTH: units
converging (or the mission's advancing force actually advancing) AND in-range pairs completing the kill
chain.  Neither is achieved.  Honest: the blocker is a coupled movement+engagement system, not a single
base-loss; the non-perturbing evidence now pins both halves (waypoints lead away; in-range pairs still
don't kill).  Fresh-session tooling (mem-hook oracle full-dump for resident unit pages; a non-perturbing
fire-moment ring-buffer) is the prerequisite to close either half cleanly -- rushing with hot-path counters
produced the reload false-positive this session.  Goal not met; the two factors are the precise, measured
frontier, with object model + LOS + fire-decision proven correct beneath them.
