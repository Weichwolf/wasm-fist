Type: feature
Title: The shared device clock and browser presentation preserve original timing

## Contract

One shared clock owns PIT/retrace/interrupt/I/O event time on both targets. Device and capture
producers consume it. Browser pacing preserves simulation/audio state and observable ordering.

## Evidence

- Shared PIT clock, ISR register/flag preservation and BIOS chaining landed in 576/582.
  Current coarse model charges one PIT count per port/pump and eight per indirect call
  (`re_out/fist_icall.c:FIST_ICALL_COUNTS`). These are approximations, not fixed-30k Oracle proof.
- PIT modes 2/3 now preserve original `timer.cpp:counter_latch` float delay and count truncation.
  At reload 65536: `delay = 1000f/(1193182f/65536f) = 54.92539978027344 ms`;
  after 17022 counts the remaining count is 48513.999574…, truncated to 48513.
  Calibration therefore reads `65536−48513 = 17023`, versus old integer result 17022.
  3064 computes `floor(period×65536/0x4dae)`: original increment 56100, old port 56097.
  The old port needed a fifth IRQ where the original needed four, causing palette event 405.
  Recovering the arithmetic fixes that palette and pixel events 648/662 in complete captures (0034).
  Direct source probe: `tools/oracle/pit_latch_probe.cpp`; 70 cases × two targets, 80 failures at
  unmodified base, all pass with the fix. Evidence: `scratch/sequence-capture/pit-calibration/`.
- Event 443 still differs. Before decoder callback 88, original DOS reads return 8/768/8/16384/3072
  bytes. `dos.cpp:modify_cycles` consumes `4×AX` only when `4×AX+5 < CPU_Cycles`; otherwise
  it leaves five cycles. The cap is the current PIC-event slice, not a fixed millisecond.
  Original GDB traces measure 32/716/32/26644/1406 consumed cycles, plus 29 mask-read cycles
  per call. `CPU_IODelayRemoved` differs from actual consumption in capped calls; do not use it
  as elapsed time. Port real/flat DOS 3F paths currently charge neither transfer nor mask-read time.
  `scratch/sequence-capture/pixel-443/read-cycles.gdb` uses offsets from the saved DOS disassembly;
  capture `oracle-read-cycles-766` matches all 742 frame/PCM bytes. Pre-decoder probe
  `pixel-443/pre-88.gdb` arms only the diagnostic counter: 1,316 instructions between 11dd and
  7135 at callback 88, with separate DOS/REP gaps in `stages-pre-88.txt`. Capture 767 retains
  every original frame/PCM byte. Packet/copy/slice accounting remains open.
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
- Dispatch fix: all 178 existing flows pass on base `65d42dc` plus patch (`run.we53cO/`).
  PIT fix: 38 tests and all 178 flows pass with unfiltered `bash tools/check_flow.sh` on `68bf531`
  plus patch (`scratch/verify/run.lwhih4/`); binary/script hashes still match after completion.
  Neither establishes complete original frame/audio parity.
- Scripted mouse timing is menu-relative vblanks. Old COOP_TICK/TICK_HZ/SIGALRM advice is obsolete;
  DOSBox wall-clock XTEST timing differs from port vblank scripts.

## Next

1. Recover FILEMGR's 16384-byte DOS packet splitting and callback 88's instruction/copy/slice
   boundaries. Reproduce measured transfer costs on the shared clock and regress event 443 on
   both targets. Do not fit the 1.1-ms gap or treat combined `fread` as one original DOS call.
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
