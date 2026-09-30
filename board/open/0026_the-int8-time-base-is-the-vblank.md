Type: feature
Title: The shared device clock and browser presentation preserve original timing

## Contract

One shared clock owns PIT/retrace/interrupt/I/O event time on both targets. Device and capture
producers consume it. Browser pacing preserves simulation/audio state and observable ordering.

## Evidence

- Shared PIT clock/ISR preservation: patches 576/582. One count per port/pump and eight per
  indirect call remain approximations, not fixed-30k Oracle proof.
- PIT modes 2/3 now preserve original `timer.cpp:counter_latch` float delay and count truncation.
  At reload 65536: `delay = 1000f/(1193182f/65536f) = 54.92539978027344 ms`;
  after 17022 counts the remaining count is 48513.999574…, truncated to 48513.
  Calibration reads `65536−48513 = 17023`, versus old integer result 17022; original increment
  56100 versus old 56097. This fixes palette 405 and pixels 648/662 (0034).
  `pit_latch_probe.cpp`: 70 cases × two targets; old port fails 80 subtests, fix passes all.
- Event 443 still differs. Before decoder callback 88, original DOS reads return 8/768/8/16384/3072
  bytes. `dos.cpp:modify_cycles` consumes `4×AX` only when `4×AX+5 < CPU_Cycles`; otherwise
  it leaves five cycles. The cap is the current PIC-event slice, not a fixed millisecond.
  Original GDB traces measure 32/716/32/26644/1406 consumed cycles, plus 29 mask-read cycles
  per call. `CPU_IODelayRemoved` is not elapsed time. Both port DOS 3F paths omit these costs.
  The DOS-Extender kernel owns packet splitting, not FILEMGR: asm `0008:18fe..1957` limits reads
  by `fs:6e4` (16384 here), then copies dwords at 1938 and residual bytes at 1945.
  Executed bytes: `scratch/sequence-capture/pixel-443/kernel-read.asm`, original 768.
- Bounded original trace: `FIST_CPU_TRACE=<file> FIST_CPU_TRACE_WINDOW=6354:6356` with
  `bash tools/oracle/capture_sequence.sh 10 <fresh-output>`; analyze with
  `python3 tools/oracle/cpu_trace.py <file> --start 2b:11dd --stop 2b:7135`.
  Writer observes registers/segments after the loop's cycle decrement; it never fetches extra
  opcode bytes or changes guest memory. Reader requires a complete fixed-cycle window/footer.
  Invalid/missing configuration and write failure stop before a capture endpoint.
- Callback 88: `1316 + 28975 + 5055 + 191 − 15 + 2 = 35524` cycles between 11dd and 7135,
  hence `35524/30000 = 1.184133… ms`. Terms: instructions, extra DOS callback costs, kernel REP,
  palette REP, zero-cycle POP/MOV SS and empty REP, and exhausted-loop/PIC boundaries.
  Source: `pre-88-attribution-final.json`/`cpu-88-final.txt` under `scratch/sequence-capture/pixel-443/`.
  Original 770 retains all 742 frame/PCM bytes; native 771/WASM 772 retain their PIT-fix baselines.
  All 42 tests pass (`pixel-443/cpu-trace-tests-final.log`); three cross-target flows pass in
  `scratch/verify/run.gdovpa/`. Ten invalid/missing-config/write-error cases fail without an endpoint.
- Pre-speaker attribution: original has 23 mask-port reads before driver entry;
  `drive_local.cpp:localFile::Read` performs one per file read. Probes 720/721 attribute the gap
  upstream of SOUNDDVR; do not label it a BIOS polling loop.
- Original first PIT-2 reload: 404.277133346 ms; port: 404.283671728 ms.
  Difference: `404.283671728−404.277133346 = 0.006538382 ms`. Do not fit an audio offset.
- Fixed-30k normal core normally charges one cycle/instruction. `iohandler.cpp` charges
  `30000/1024 = 29` extra cycles/read and `30000/1365 = 21` extra cycles/write
  (integer division), suppressed when fewer than three delay costs remain in the slice.
  Preserve instruction boundaries and slice remainder when modeling an observable event.
- VGA/PIC writes now isolate speaker port 0x61 (`tests/port_io.c`). Current port `3302901` has
  all 178 existing flows passed in `scratch/verify/run.lwhih4/`; binaries/scripts remain byte-identical
  after the trace change. Complete original frame/audio parity remains open.
- Port mouse scripts use menu-relative vblanks; Oracle XTEST uses wall time.

## Next

1. Recover shared CPU phase and current PIC slice; one PIT count spans `30000000/1193182`
   CPU cycles. Implement DOS/Extender packet costs: original slice cap, mask read,
   gateway/hook instructions, REP counts and exhausted-loop boundaries. Recover dynamic branch
   counts from asm and the bounded trace; never charge a fitted 35524-cycle constant. Preserve
   reaching failures and regress every event through 10000 ms on both targets, especially 443.
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
