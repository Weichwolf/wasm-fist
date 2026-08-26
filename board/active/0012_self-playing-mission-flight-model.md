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
