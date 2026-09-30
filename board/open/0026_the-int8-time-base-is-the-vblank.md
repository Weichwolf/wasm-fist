Type: feature
Title: The shared device clock and browser presentation preserve original timing

## Contract

One shared clock owns PIT/retrace/interrupt/I/O event time on both targets. Device and capture
producers consume it. Browser pacing preserves simulation/audio state and observable ordering.

## Evidence

- Shared PIT clock/ISR preservation: patches 576/582. One count per port/pump and eight per
  indirect call remain approximations, not fixed-30k Oracle proof.
- PIT modes 2/3 preserve original `counter_latch`: at reload 65536,
  `delay = 1000f/(1193182f/65536f) = 54.92539978027344 ms`;
  after 17022 counts the remaining count is 48513.999574…, truncated to 48513.
  `65536−48513 = 17023`, versus old 17022. This fixes palette 405 and pixels 648/662 (0034).
  `pit_latch_probe.cpp`: 70 cases × two targets; old port fails 80 subtests, fix passes all.
- CPU work, timer baselines and current IRQ wrap model now share fractional PIT phase:
  `elapsed = whole_counts + fraction/30000000`; one CPU cycle adds `1193182/30000000` counts.
  Actual `counter_latch` verifies 2 modes × 4 reloads × 8 intervals × 2 initial phases × 2 targets
  = 256 added cases. The initial 128-case subset had 44 reaching failures before correction.
  `bash tools/check_flow.sh` passes all 43 tests, patches, both builds and 178 cross-target flows:
  `scratch/verify/run.2Oucs7/` (base `b37def0` plus saved patch; current source/binary hashes match).
  Native 781/WASM 782 retain every 762/763 frame byte; event 443 remains unequal.
- Event 443 still differs. Before decoder callback 88, original DOS reads return 8/768/8/16384/3072
  bytes. `dos.cpp:modify_cycles` consumes `4×AX` only when `4×AX+5 < CPU_Cycles`; otherwise
  it leaves five cycles. The cap is the current PIC-event slice, not a fixed millisecond.
  Original GDB traces measure 32/716/32/26644/1406 consumed cycles, plus 29 mask-read cycles
  per call. `CPU_IODelayRemoved` is not elapsed time. Both port DOS 3F paths omit these costs.
  Kernel asm `0008:18fe..1957` splits at `fs:6e4` (16384 here); REP copies at 1938/1945.
  Source: `scratch/sequence-capture/pixel-443/kernel-read.asm`, original 768.
- Queue probe 776 identifies the caps: PanningLatch at 6354.076545715332 ms and VerticalTimer
  at 6355.188701629639 ms. The port omits both as standalone CPU slice boundaries.
  Probe 780 recovers original float delays; its complete frame/PCM bytes match 742.
  Actual `PIC_RunQueue/PIC_AddEvent/TIMER_AddTick` reproduces all five slices and `CPU_CycleLeft`:
  `scratch/sequence-capture/pixel-443/pic-slice-source-proof.json` (VGA queue scope only).
- Bounded original trace: `FIST_CPU_TRACE=<file> FIST_CPU_TRACE_WINDOW=6354:6356` with
  `bash tools/oracle/capture_sequence.sh 10 <fresh-output>`; analyze with
  `python3 tools/oracle/cpu_trace.py <file> --start 2b:11dd --stop 2b:7135`.
  Observe registers/segments after cycle decrement; no extra opcode reads or guest writes.
  Require a complete fixed-cycle window/footer. Ten config/write-error cases fail without an endpoint.
- Callback 88: `1316 + 28975 + 5055 + 191 − 15 + 2 = 35524` cycles between 11dd and 7135,
  hence `35524/30000 = 1.184133… ms`. Terms: instructions, extra DOS callback costs, kernel REP,
  palette REP, zero-cycle POP/MOV SS and empty REP, and exhausted-loop/PIC boundaries.
  Source: `pre-88-attribution-final.json`/`cpu-88-final.txt` under `scratch/sequence-capture/pixel-443/`.
  Original 770 retains all 742 frame/PCM bytes; prior cross-target proof: `scratch/verify/run.gdovpa/`.
- Pre-speaker: 23 mask reads in `localFile::Read` before SOUNDDVR, proved by 720/721.
- Original first PIT-2 reload: 404.277133346 ms; port: 404.283671728 ms.
  Difference: `404.283671728−404.277133346 = 0.006538382 ms`. Do not fit an audio offset.
- Fixed-30k normal core normally charges one cycle/instruction. `iohandler.cpp` charges
  `30000/1024 = 29` extra cycles/read and `30000/1365 = 21` extra cycles/write
  (integer division), suppressed when fewer than three delay costs remain in the slice.
  Preserve instruction boundaries and slice remainder when modeling an observable event.
- Port 0x61 isolation: `tests/port_io.c`. Accepted `3302901`: 178 flows, `scratch/verify/run.lwhih4/`.
- Port mouse scripts use menu-relative vblanks; Oracle XTEST uses wall time.

## Next

1. Model all original queued VGA/PIT events on the shared clock with original float delays,
   millisecond slice limits and exhausted-loop boundaries. Preserve 0036's DOS-load CPU handoff.
   Regress the five source-proved slices before charging DOS packet caps, mask reads, gateway/hook
   instructions and REP. Recover branch counts from asm/trace; never fit a 35524-cycle delay.
   Preserve reaching failures; regress all 10000-ms events on both targets, especially 443.
2. Attribute the pre-speaker reads/calls; recover PIT-0 ISR return and SOUNDDVR I/O costs.
   Repair the shared time contract and compare the first event before extending the trace.
3. Verify retrace/reload/IRQ order through calibration/re-arm. Supply timed events to 0003 and
   frame capture to 0034; neither implements a separate device clock.
4. Audit mouse-time scripts (r92/9200, debrief). Run `make web`; measure input/presentation,
   background/resume, audio continuity and Atomics.wait/shared-memory prerequisites. Pace at
   presentation boundaries; retain traces. No wall-clock-dependent simulation ticks.

## Accept

Matched original PIT/retrace/ISR effects and sample order, responsive browser play and cross-target
parity. Node matrix passes alone do not establish browser behavior.
