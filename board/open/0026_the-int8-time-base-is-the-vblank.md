Type: feature
Title: The shared device clock and browser presentation preserve original timing

## Contract

One clock owns PIT/retrace/interrupt/I/O time on both targets. Device/capture producers consume it.
Browser pacing preserves simulation/audio state and observable ordering.

## Evidence

- PIT modes 2/3 match original `counter_latch`. Mode 2 at reload 65536 uses
  `1000f/(1193182f/65536f) = 54.92539978027344 ms`. After 17022 counts the remaining
  48513.999574… truncates to 48513: `65536−48513 = 17023`, versus old 17022.
  `pit_latch_probe.cpp`: 70 cases × 2 targets; old port fails 80 subtests. Fix resolves three 0034 events.
- CPU work, timer baselines and the current IRQ-wrap model preserve fractional PIT phase:
  `elapsed = whole_counts + fraction/30000000`; each CPU cycle adds `1193182/30000000` counts.
  Actual `counter_latch`: 2 modes × 4 reloads × 8 intervals × 2 phases × 2 targets = 256 cases.
  Initial 128-case subset had 44 reaching failures. Accepted `839ae5e`: 43 tests, patches, both builds,
  178 flows; `scratch/verify/run.2Oucs7/`. Full original PIT event/IRQ ordering remains unproved.
- `fist_clock_cpu_slice` projects the next PIC dispatch budget from shared clock/VGA state:
  PanningLatch, retrace, vertical interrupt, chained bands and millisecond limits. The calendar runs
  without capture; `FIST_OPENLOG` reports both DOS 3F paths' read budgets. `pic_slice_probe.cpp`
  compiles actual `PIC_RunQueue/PIC_AddEvent/TIMER_AddTick`: (16 cases + 1 capture case) × 2 targets
  = 34 passing comparisons. Old projection failed 14/26; the previous origin's creation mode fixes
  six further failures. Transition proof covers post-resize queued state, not resize timing.
- Mode 13 uses `25175000/8 = 3146875` pixels/s; text uses `28322000/9 = 3146888`.
  `SetupDrawing` retains cached vtotal for changes below 0.0001 ms while other delays change.
  Original probes 776/780/785 recover caps/geometry; their complete frame/PCM bytes match 742.
  This projection does not track running `CPU_Cycles` or prove the complete PIT queue/IRQ contract.
- Accepted projection: `bash tools/check_flow.sh` passes 44 tests, exact patches, both builds
  and all 178 existing flows; `scratch/verify/run.EKVpOw/` (base `839ae5e` + saved patch).
  Matched native 789/WASM 790 retain all prior frame bytes; 434 read-budget records agree.
  Frame 443 and missing final PCM remain open in 0034. Proofs: `scratch/sequence-capture/pic-slices/`.
- Before decoder 88, original reads 8/768/8/16384/3072 bytes; the port reads 19456 unsplit.
  `dos.cpp`: subtract `4×AX` iff `4×AX+5 < CPU_Cycles`, otherwise set five cycles.
  GDB proves costs 32/716/32/26644/1406 plus 29 mask-read cycles each, capped by the active
  PIC slice. `CPU_IODelayRemoved` is not elapsed time; below-five budgets can gain cycles.
  Actual `dos.cpp/iohandler.cpp` probe: 303 boundary cases plus all five observed callbacks
  (`pic-slices/io-dos-source-proof.json`). Original 786 sees no below-five DOS cap in this run.
  Both port 3F paths omit costs.
  Kernel `0008:18fe..1957` splits at `fs:6e4` (16384 here); REP copies at 1938/1945.
- Callback 88: `1316 + 28975 + 5055 + 191 − 15 + 2 = 35524` cycles, or `35524/30000`
  = 1.184133… ms: instructions, DOS, kernel REP, palette REP, zero-cycle SS/REP, loop boundaries.
  Source: `pixel-443/{kernel-read.asm,pre-88-attribution-final.json,cpu-88-final.txt}` under
  `scratch/sequence-capture/`. Original 770 retains all 742 frame/PCM bytes. Trace with
  `FIST_CPU_TRACE=<file> FIST_CPU_TRACE_WINDOW=6354:6356` during an original 10-second capture;
  analyze with `python3 tools/oracle/cpu_trace.py <file> --start 2b:11dd --stop 2b:7135`. Require footer.
- Original 792/793 retain all 742 frame/PCM bytes. Decoder 88 → palette entry costs
  `290708+3+3 = 290714`: decoder, caller, three exhausted-loop cycles at queued draw bands.
  Palette → copy: `(4×768+11)+756×21 = 18959`; 13 of 769 writes suppress extra delay.
  Copy → return: `16000+5 = 16005` cycles; REP resumes across a millisecond boundary.
  Proofs: `pic-slices/{decoder-88-retirement,palette-copy-88-source}-proof.json`.
- Original normal core costs one cycle/instruction with SS/REP exceptions. I/O extra costs:
  `30000/1024 = 29` per read, `30000/1365 = 21` per write; suppress below three delay costs.
  Port per-I/O/pump and indirect-call charges remain approximations (patches 576/582).
- Pre-speaker: 23 `localFile::Read` mask reads before SOUNDDVR (720/721). First PIT-2 reload:
  port 404.283671728 minus original 404.277133346 = 0.006538382 ms late. Do not fit an offset.
  Port mouse scripts use menu-relative vblanks; original XTEST uses wall time.

## Next

1. Track active CPU slices and normal-core retirement/exhausted-loop decrements before
   charging DOS costs, including below-five credits. Prove the three decoder-boundary cycles.
   Regress both targets. Recover packet splits, gateway/hook instructions and REP from asm/trace;
   preserve 0036's handoff and match every 10000-ms event, especially 443. Never fit 35524 cycles.
2. Attribute pre-speaker reads/calls, PIT-0 ISR return and SOUNDDVR I/O. Verify reload/re-arm/IRQ order.
   Supply device events to 0003 and scanout to 0034; neither owns a separate clock.
3. Audit mouse scripts (r92/9200, debrief). Run `make web`; measure input/presentation,
   background/resume, audio continuity and Atomics.wait/shared-memory prerequisites. Pace at
   presentation boundaries; no wall-clock-dependent simulation ticks.

## Accept

Matched original PIT/retrace/ISR effects and sample order, responsive browser play and cross-target
parity. Node matrix passes alone do not establish browser behavior.
