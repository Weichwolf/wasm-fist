Type: feature
Status: open
Parent: 0007
Title: the INT-8 time base is the original's vblank-locked PIT

The engine makes the PIT interrupt its vertical-retrace interrupt: 2fd3 measures one retrace period
in PIT counts (30de polls port 0x3da between two retrace edges while the PIT free-runs) into [0x44c],
3064 derives the [0x452] frame-tick increment d8b8 = [0x44c] << 16 / 0x4dae (60 Hz) and d8bc (50 Hz),
and the ISR 30f8 waits for the retrace and re-programs the PIT ([0x18c4]) on every interrupt so the
next one lands just before the next vblank.  Everything the ISR does per interrupt -- 31c3's retrace
section ([0x5e4] = the driver's DAC service 0be2, 3a00, [0x15b0], [0x2b0]), 335b's event timers, the
sound driver's sequencer -- therefore runs once per vblank on the original.

Measured (DOSBox oracle, `scratch/oracle/player.ram.bin`): [0x44c] = 0x427f (17023 counts = 70.09 Hz,
DOSBox's mode-13h refresh), d8b8 = 0xdb24, [0x18c4] = 0x426e, d8c6 = 0x406c.  The port: the shim's
PIT counter (fist_vga.c in(0x40)) decrements once per read and 0x3da toggles per poll, so 2fd3 sees a
tiny period, [0x44c] = 0x100, d8b8 = 0x34b -- 78 interrupts per frame tick against the original's
1.17.  The frame tick itself stays 60 Hz by construction, so screens and the [0x452]-paced logic look
right; the per-interrupt work does not: palette fades settle within one frame tick (the original's
0x20-step SETTINGS fade takes 0.46 s), and any INT-8-scheduled sim work is ~66x too frequent per frame
-- a candidate cause for the spawn-cadence and freeze findings on board:0007, and for the verdict
arriving at 3:38 on the port against 5:49 on the oracle (board:0017).

This engine can drive its INT-8 from a deterministic VGA/PIT clock model whose retrace period is 17023
PIT counts, so that 2fd3 measures 0x427f, 30f8's re-arm produces the original's [0x18c4] sequence,
and one cooperative tick == one vblank == one INT-8, the same on native and wasm.  The cooperative
pump then advances that clock (not "one interrupt per pump"), in(0x40)/in(0x3da) read it, and the
verify matrix is re-passed on both targets.

## The clock (2026-09-12)

Done in the shim (fist_vga.c holds the clock, native_main.c delivers the interrupt, fist_icall.c and
the extender gate charge their costs; no engine patch beyond 576's ISR carry):

- One time line, in PIT counts (1193182 Hz).  Every port access and every cooperative pump costs one
  count; every dispatched indirect call 8; every windshield render (extender op 0x24) one frame period.
  Channel 0's counter, the latch, the reload and the mode come from the count; the INT-8 is delivered
  at each wrap it steps across, in order, held while the ISR runs (the PIC's single edge).  The BIOS tick
  runs until the engine takes the vector; from then on 30f8 chains it through [0x432] (patch 576).
- The VGA: 17025 counts per mode-13h frame, 449 lines, the retrace pulse on lines 412..414 (DOSBox's
  int10 CRTC values), so 2fd3 measures exactly 0x427f and d8b8 = 0xdb24 -- every full-RAM oracle
  capture has those two words.  A status poll that would spin jumps the clock to one count before the
  next edge (the sequence of observed statuses is unchanged; the poll count is not, so 30f8's PLL settles
  on a reload ~0x411c instead of the oracle's ~0x426e -- the interrupt period is one frame either way).
- Measured on the port: 701 INT-8 in 600 ticks (65536/0xdb24 = 1.168), the SETTINGS fade takes its
  0x20 vblanks, the debrief dim its 60.
- The FIST_MOUSE script unit is now vblanks after the menu entry (was pumps); every script in verify.sh,
  selfplay.sh and asan_selfplay.sh was rescaled by 1/16.7 (the measured old pumps-per-vblank), and each
  step lands on the same [0x452] as before.  FIST_TICK_HZ / FIST_COOP_TICK / FIST_NO_ITIMER /
  FIST_NOMISSIONCOOP are gone with the SIGALRM model; live web play paces each vblank to the wall clock.

Measured with it:

- The oracle machine (this host, the instrumented DOSBox at cycles=max) renders 25.7 fps in-mission
  (`scratch/oracle/fps`: 4754 windshield passes over 11112 ticks, 0 or 1 per tick), i.e. one frame per
  2.3 ticks; the port at one frame per vblank renders 1.17 per tick.  The sim does not care: AZER1
  resolves at [0x452]=8595 with 8000, 17025 or 46500 counts per frame.  The mission loop 459a steps the
  sim once per elapsed tick and drains its event queue before every step, so frames are presentation.
- What the INT-8 cadence DID change: the same AZER1 verdict moved from 12630 (78 interrupts per tick)
  to 8595 (1.17), both DEFEAT -- the per-interrupt work (335b's timers, the sound sequencer's 60 Hz
  advance from [0x452]'s carry) is now at the original's rate.  The oracle's own AZER1 runs spread from
  1:38 to 5:49, so a tick-for-tick match needs the same initial conditions (RNG seed, click ticks) --
  board:0017's business, not the clock's.

Open: the wasm web player's pacing (fist_web_vblank, Atomics.wait) is untested in a browser; the
r92/9200 capture scripts and census_debrief.sh still assume the old FIST_MOUSE unit where they drive
the port (they drive DOSBox with xclick, unaffected).
