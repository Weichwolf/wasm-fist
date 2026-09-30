Type: feature
Title: The shared device clock and browser presentation preserve original timing

## Contract

One shared clock owns PIT/retrace/interrupt/I/O event time on both targets. Device and capture
producers consume it. Browser pacing preserves simulation/audio state and observable ordering.

## Evidence

- Shared PIT clock, ISR register/flag preservation and BIOS chaining landed in 576/582.
  Current coarse model charges one PIT count per port/pump and eight per indirect call
  (`re_out/fist_icall.c:FIST_ICALL_COUNTS`). These are approximations, not fixed-30k Oracle proof.
- Historical mode-13 calibration measured 0x427f; legacy model period is 17025 counts. Earlier poll skipping
  changed reload 0x411c versus Oracle 0x426e. Matching average frame rate does not prove event order.
- With the canonical 0036 fixture, original calibration is 17023 (`0x427f`) but both ports measure
  17022 (`0x427e`). 3064 computes `floor(period×65536/0x4dae)`: increments 56100 versus 56097.
  At counter 317 after KDV callback 79, original fraction is 38188; port fraction is 37075.
  Four IRQs give `38188+4×56100 = 4×65536+444` (counter 321) versus
  `37075+4×56097 = 3×65536+64855` (counter 320). Port needs a fifth IRQ, delaying palette
  upload past frame 405. Original/native/WASM traces 755/756/759 and `palette-405/cause.json`
  live under `scratch/sequence-capture/`. This is a reaching failure, not a corrected clock.
- Temporary instruction/I/O probes `oracle-speaker-instruction-720`/`port-speaker-io-721`
  locate the first speaker event gap upstream of SOUNDDVR: original PIT-0 reload 0x4175,
  port 0x4174; EOI at 404.233733326 versus 404.234224117 ms. Original has 23 reads of PIC
  port 0x21 before driver entry. `src/dos/drive_local.cpp:localFile::Read` reads that mask
  on every file read; do not label this a BIOS polling loop without caller attribution.
- Original first PIT-2 reload: 404.277133346 ms; port: 404.283671728 ms.
  Difference: `404.283671728−404.277133346 = 0.006538382 ms`. Do not fit an audio offset.
- Fixed-30k normal core charges one cycle/instruction. `iohandler.cpp` charges
  `30000/1024 = 29` extra cycles/read and `30000/1365 = 21` extra cycles/write
  (integer division), suppressed when fewer than three delay costs remain in the slice.
  Preserve instruction boundaries and slice remainder when modeling an observable event.
- VGA/PIC ignored writes fell through to speaker port 0x61, changing its state and PIT-2 base.
  `tests/port_io.c` reaches the defect on both targets; the dispatch now isolates port 0x61.
- Verification on base `65d42dc` plus the recorded patch: `bash tools/check_flow.sh` passes all
  178 existing flows on both targets (`scratch/verify/run.we53cO/`). The final port regression
  fails on unmodified base C and passes with the fix on native/WASM; logs retain both results.
  This proves the bounded dispatch fix, not complete original frame/audio parity.
- Scripted mouse timing is menu-relative vblanks. Old COOP_TICK/TICK_HZ/SIGALRM advice is obsolete;
  DOSBox wall-clock XTEST timing differs from port vblank scripts.

## Next

1. Recover 2fd3 calibration/reload/latch instruction and I/O costs; fix 17022 versus 17023 at
   its producer and regress complete captures through palette event 405. Do not force the result.
2. Attribute the pre-speaker reads/calls; recover PIT-0 ISR return and SOUNDDVR I/O costs.
   Repair the shared time contract and compare the first event before extending the trace.
3. Verify retrace/reload/IRQ order through calibration/re-arm. Supply timed events to 0003 and
   frame capture to 0034; neither implements a separate device clock.
4. Audit old mouse-time scripts (r92/9200, debrief). Run `make web`; measure input/presentation,
   background/resume and audio continuity, including Atomics.wait/shared-memory prerequisites.
5. Pace at the presentation boundary; retain event traces. No wall-clock-dependent simulation ticks.

## Accept

Matched original PIT/retrace/ISR effects and sample order, responsive browser play and cross-target
parity. Node matrix passes alone do not establish browser behavior.
