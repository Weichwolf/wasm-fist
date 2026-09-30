Type: feature
Title: The shared device clock and browser presentation preserve original timing

## Contract

One clock owns PIT/retrace/interrupt/I/O time on both targets. Device/capture producers consume it.
Browser pacing preserves simulation/audio state and observable ordering.

## Evidence

Proof paths are under `scratch/sequence-capture/`; matrix runs are under `scratch/verify/`.

- Phase is `whole_counts + fraction/30000000`; each CPU cycle adds `1193182/30000000` counts.
  PIT modes 2/3 retain source float latch/rounding. VGA calendar: retrace, PanningLatch, vertical
  interrupt, chained bands, cached vtotal and tick limits. Mode 13/text clocks: `25175000/8` / `28322000/9`.
  `fist_clock_cpu_slice` returns tracked budget or next dispatch. Start phase is fixed (0036).
  Production resize, PIT event/IRQ order, SS/REP and guest effects remain unproved.
- Retirement keeps empty budgets across calls. The failed normal-loop decrement survives intra-tick
  dispatch; `TIMER_AddTick` resets it. Raw elapsed cycles are separate. Actual-source regressions:
  `test_port_io.py`, `pic_slice_probe.cpp`; `12×2 = 24` retirement cases, old logic fails 20.
  Current: `bash tools/check_flow.sh`, 46 tests, exact patches, both builds, `178` flows, exit 0;
  `run.6eoNh3/` (base `fcf12a2` + saved patch), `start-epoch/acceptance-proof.json`.
  All 2100 native/WASM frame bytes match the previous clock; original parity stays open (0034).
- DOS subtracts `4×AX` iff `4×AX+5 < CPU_Cycles`, otherwise sets five cycles, including credits.
  Buffer/register writes precede the cost, including failed reads' AX error. PIC resumes after callback;
  credits can postpone an event. Keep PIC tick distinct from virtual index: 30000 can return to 29995
  before `TIMER_AddTick`. `CPU_IODelayRemoved` is not elapsed time. No below-five hit through 30 s.
  Proofs: `pic-slices/io-dos-source-proof.json`, `cpu-retirement/{pic-dos-credit,pic-dos-tick-boundary}-proof.json`;
  `101×5×2 = 1010` synthetic budget/AX/I/O cases plus six tick-end cases; guest-file integration is open.
- Kernel `0008:18fe..1957` splits reads at `fs:6e4` (16384), with REP copies at 1938/1945.
  Pre-88 reads 8/768/8/16384/3072 cost 32/716/32/26644/1406 plus 29 mask-read cycles each.
  Port reads 19456 unsplit. Full pre-decoder cost: `1316+28975+5055+191−15+2 = 35524` cycles,
  `35524/30000 = 1.184133… ms` (instructions, DOS, kernel/palette REP, SS/REP and loop boundaries).
  Proof: `pixel-443/{kernel-read.asm,pre-88-attribution-final.json,cpu-88-final.txt}`.
  Reproduce with `FIST_CPU_TRACE_WINDOW=6354:6356`; require the complete footer, then
  `cpu_trace.py <file> --start 2b:11dd --stop 2b:7135` (capture recipe 0036).
- Decoder → palette: `290708+3+3 = 290714` cycles. Palette → copy: `(4×768+11)+756×21 = 18959`;
  13 of 769 writes suppress delay. Copy → return: `16000+5 = 16005`, REP resumes across a tick.
  Original 792/793 preserve all 742 frame/PCM bytes. Proofs:
  `pic-slices/{decoder-88-retirement,palette-copy-88-source}-proof.json`.
- Normal core costs one cycle/instruction with SS/REP exceptions. I/O extra costs:
  `30000/1024 = 29` per read, `30000/1365 = 21` per write; suppress below three delay costs.
  Port per-I/O/pump and indirect-call charges remain approximations (patches 576/582).
- Speaker logs 811/812 reproduce 803/804's 27 counter/54 type commands and all 2100 frames.
  First counter: `404.283671728−404.277133346 = 0.006538382` ms late. Logs omit CPU fractions;
  read-only probe 813 gives `min(port_phase−original) = −0.123809266` ms, with unchanged frame bytes.
  Proof: `start-epoch/speaker-phase-proof.json`. Synthesis belongs to 0003; never fit offsets.
- Original warm-up: `46×2−2 = 90`; native 808: `44×2−2 = 86`. The 26 delay calls lose
  `4×16×2×26 = 3328` instructions. Original 07b7 → first 07f4 fetch:
  `390 + 76×29 + 43×21 + 1 = 3498` cycles; 18 reads/eight writes suppress delay, one failed-loop cycle.
  `start-epoch/warmup-original-proof.json`. Isolated native/WASM `prototype/*-warmup` derives 90
  from the original warm-start fixture and shared clock; production prefix and CLI/STI effects are open.

## Next

1. Restore 0036's DOS-load handoff with real MZ reads and active budget. Reuse `start-epoch/prototype/`:
   288 source-budget cases, real images/relocations, 32-KiB boundary and short EOF pass both targets.
   Wire production, then implement DOS caps/credits, packet splits, gateway instructions and REP.
   Preserve callback ordering and PIC tick state. Match every 10000-ms event, especially 443.
2. Recover production pre-speaker work (23 mask reads, 720/721), 07b7's polling/CLI/STI and ISR return.
   Derive calibration arguments from operations and budget; never inject 90 or a measured total delay.
   Prove reload/re-arm/IRQ order. Supply events to 0003/0034; neither owns another clock.
3. Audit mouse scripts (r92/9200, debrief): ports use menu-relative vblanks, original XTEST wall time.
   Run `make web`; measure input/presentation, background/resume, audio continuity and
   Atomics.wait/shared-memory prerequisites. Pace at presentation boundaries, never host time.

## Accept

Original PIT/retrace/ISR effects and sample order, responsive browser play and cross-target parity.
Node matrix passes alone do not establish browser behavior.
