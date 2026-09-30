Type: feature
Title: The shared device clock and browser presentation preserve original timing

## Contract

One clock owns PIT/retrace/interrupt/I/O time on both targets. Device/capture producers consume it.
Browser pacing preserves simulation/audio state and observable ordering.

## Evidence

Proof paths below are under `scratch/sequence-capture/`; matrix runs are under `scratch/verify/`.

- Phase is `whole_counts + fraction/30000000`; each CPU cycle adds `1193182/30000000` counts.
  PIT modes 2/3 retain original float latch/rounding. One VGA calendar includes retrace, PanningLatch,
  vertical interrupt, chained bands and tick limits without capture. `fist_clock_cpu_slice` returns
  tracked budget or next dispatch. Source regressions: `test_port_io.py`, `pic_slice_probe.cpp`.
  Mode 13/text clocks are `25175000/8` / `28322000/9`; retain source's cached vtotal tolerance.
  Accepted phase/projection: `839ae5e` / `09bc527`, `run.2Oucs7/` / `run.EKVpOw/`.
  Production resize timing, PIT event/IRQ order and guest effects remain unproved.
- Retirement retains an empty budget across calls. Normal-loop extra decrement survives
  an intra-tick PIC boundary; `TIMER_AddTick` resets it. Raw elapsed cycles are a separate operation.
  Actual PIC/no-op loop: `12 cases×2 targets = 24` pass; old logic fails 20. Decoder endpoints match 793.
  SS/REP, DOS, latch effects and guest IRQ work remain open.
  Accepted: `bash tools/check_flow.sh`, 45 tests, exact patches, both builds, 178 flows, exit 0;
  `run.iWxrKB/` (base `09bc527` + saved patch). Matched 10000/30000-ms frame bytes are unchanged (0034).
  Proofs: `cpu-retirement/{acceptance,baseline-retirement,capture}-proof.json`.
- DOS subtracts `4×AX` iff `4×AX+5 < CPU_Cycles`, otherwise sets five cycles (including credits).
  `CPU_IODelayRemoved` is not elapsed time. Pre-88 reads 8/768/8/16384/3072 cost
  32/716/32/26644/1406 plus 29 mask-read cycles each; port reads 19456 unsplit.
  `pic-slices/io-dos-source-proof.json`: 303 cases + five observed reads. No below-five hit through 30 s.
  Kernel `0008:18fe..1957` splits at `fs:6e4` (16384); REP copies at 1938/1945.
- DOS buffer/register writes precede `modify_cycles(AX)`, including failed reads' AX error code.
  PIC dispatch resumes after callback return; credits can postpone an already due event.
  `cpu-retirement/{pic-dos-credit,pic-dos-tick-boundary}-proof.json`: `101×5×2 = 1010` budget/AX/I/O
  cases + six tick-end cases with synthetic markers. Virtual index 30000 can return to 29995
  before `TIMER_AddTick`. Keep PIC tick state distinct from virtual index; no guest-file proof yet.
- Pre-decoder 88: `1316+28975+5055+191−15+2 = 35524` cycles, `35524/30000 = 1.184133… ms`:
  instructions, DOS, kernel/palette REP, SS/REP exceptions and loop boundaries. Source:
  `pixel-443/{kernel-read.asm,pre-88-attribution-final.json,cpu-88-final.txt}`. Require complete footer:
  `FIST_CPU_TRACE_WINDOW=6354:6356`,
  `cpu_trace.py <file> --start 2b:11dd --stop 2b:7135` (capture recipe 0036).
- Decoder → palette: `290708+3+3 = 290714` cycles (decoder, caller, three queued-band loop cycles).
  Palette → copy: `(4×768+11)+756×21 = 18959`; 13 of 769 writes suppress extra delay.
  Copy → return: `16000+5 = 16005`; REP resumes across a millisecond boundary. Original 792/793
  preserve all 742 frame/PCM bytes. Proofs: `pic-slices/{decoder-88-retirement,palette-copy-88-source}-proof.json`.
- Normal core costs one cycle/instruction with SS/REP exceptions. I/O extra costs:
  `30000/1024 = 29` per read, `30000/1365 = 21` per write; suppress below three delay costs.
  Port per-I/O/pump and indirect-call charges remain approximations (patches 576/582).
- Pre-speaker: 23 file mask reads (720/721). Current 803/804/805 match 27 counter/54 type commands;
  first reload is `404.283671728−404.277133346 = 0.006538382` ms late, later events 0.124481111 ms early.
  Original 807/native 808 calibration starts at 90/86; all 26 arguments differ by four,
  removing `4×16×2×26 = 3328` instructions. Proofs: `cpu-retirement/{speaker-current,sound-calibration-current}-proof.json`.
  Do not fit offsets/arguments. Synthesis belongs to 0003; frame bytes remain unchanged.
  Port mouse scripts use menu-relative vblanks; original XTEST uses wall time.

## Next

1. Implement mask-I/O delays and DOS caps against active budget; distinguish PIC tick/dispatch
   from virtual time inside callbacks.
   Preserve below-five credits and buffer/register writes before callback dispatch.
   Recover packet splits, gateway/hook instructions and REP from asm/trace; preserve 0036's handoff.
   Regress both targets and match every 10000-ms event, especially 443. Never fit 35524 cycles.
2. Recover SOUNDDVR 07b7's warm-up polling/retirement; derive calibration arguments, never set 90.
   Attribute pre-speaker reads/calls and PIT-0 ISR return; prove reload/re-arm/IRQ order.
   Supply events to 0003/0034; neither owns a separate clock.
3. Audit mouse scripts (r92/9200, debrief). Run `make web`; measure input/presentation,
   background/resume, audio continuity and Atomics.wait/shared-memory prerequisites. Pace at
   presentation boundaries; no wall-clock-dependent simulation ticks.

## Accept

Matched original PIT/retrace/ISR effects and sample order, responsive browser play and cross-target
parity. Node matrix passes alone do not establish browser behavior.
