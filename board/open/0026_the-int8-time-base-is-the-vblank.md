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
  Remaining ports/pumps still charge whole PIT counts (`fist_vga.c`, `native_main.c`).
  Decoder instructions are charged by patch 617, startup/sound calibration by 618/620.
  Full instruction, REP and interrupt timing remains open; the old 576/582 references were incorrect.
- Current base `7426275`: PIC data ports 0x21/0xa1 retain their independent mask bytes, initialized
  from DOSBox's enabled timer/keyboard/cascade/RTC IRQs. Their I/O delay consumes the active CPU
  budget before the register access and suppresses below three delay costs. Remaining ports still
  use the old pump; IRQ delivery, ICW/command handling and production file-read integration remain open.
  Actual-source `pic_slice_probe.cpp` now links `io_delay_probe.cpp` and constructs PIC_8259A;
  15 budget/read-count cases × unmasked/masked IRQ2 × both targets = 60 matching cases. The old
  code fails all 30 unmasked cases. `test_file_read_mask_io_matches_original_budget_and_suppression`
  includes the full 1151-read loader count, read/write suppression and IRQ2 unmasking.
- Fresh read-only GDB capture confirms all 1151 original FIST.DAT reads: first budget 1186 at
  cycle 628814, 38 costed reads, then 1113 suppressed reads at cycle 629916/budget 84/mask 0xf8.
  Application fetch remains cycle 629918. All complete 10000-ms frames/palettes/times and PCM match
  `resume-a469760-original/`. Proof: `pic-mask/{read-mask.gdb,dosbox-gdb,original-read-mask.json,
  original-start-cpu-complete.txt,original-complete/,source-proof.json}`. The broad first probe timed
  out; its incomplete capture is excluded. The successful probe arms only at DOS_Execute(FIST.DAT)
  and disarms after the loader's 1151 reads.
- The old unrelated-port PIT2 assertion subtracts whole PIT timestamps; fractional I/O exposes
  that invalid expectation on both targets (`run.e6degJ/`). Its replacement retains all 17 ports
  and both enabled speaker states, comparing complete latched PIT2 words against actual original
  `timer.cpp` at the recovered relative times. Speaker-state assertions remain intact.
  `bash tools/check_flow.sh '^(intro|mainmenu)$'`: 52 tests, exact patches, native/WASM builds,
  2 selected flows pass / 0 fail, exit 0; `run.JYGlHR/`. New complete 10000-ms native/WASM frames
  match each other and the prior native capture bytewise; first original pixel failure remains
  event 443 / byte 32004 (13 vs 14). Evidence: `pic-mask/{native,wasm}/`. This is bounded PIC data
  port evidence, not complete timing or PCM acceptance.
- Speaker logs 811/812 reproduce 803/804's 27 counter/54 type commands and all 2100 frames.
  First counter: `404.283671728−404.277133346 = 0.006538382` ms late. Logs omit CPU fractions;
  read-only probe 813 gives `min(port_phase−original) = −0.123809266` ms, with unchanged frame bytes.
  Proof: `start-epoch/speaker-phase-proof.json`. Synthesis belongs to 0003; never fit offsets.
- Original warm-up: `46×2−2 = 90`; native 808: `44×2−2 = 86`. The 26 delay calls lose
  `4×16×2×26 = 3328` instructions. Original 07b7 → first 07f4 fetch:
  `390 + 76×29 + 43×21 + 1 = 3498` cycles; 18 reads/eight writes suppress delay, one failed-loop cycle.
  `start-epoch/warmup-original-proof.json`. Isolated native/WASM `prototype/*-warmup` derives 90
  from the original warm-start fixture and shared clock; production prefix and CLI/STI effects are open.
- Current base `cbbda64`: `fist_load_mz` is the shared original-backed application/overlay loader.
  It reads the 28-byte MZ/ZM header, uses DOSBox's 0x7ff page mask and page rounding rather than e_cblp,
  reads 32-KiB blocks (including short/zero EOF results), then reads each four-byte relocation after
  one seek. Only actual file bytes are written; PhysMake's 16-bit segment addition is preserved.
  The obsolete load_seg >= 0x3400 guard is removed: original DOS overlays accept segment zero.
  The overlay registry retains actual loaded extent rather than claiming unwritten EOF padding.
  Every read performs localFile::Read's PIC access after fread, before guest-memory copying.
  This internal DOS_Execute path has no INT 21h 4*AX budget cap.
- `test_mz_loader_matches_original_reads_relocations_and_short_eof` fails all 12 target/file cases
  on the old loader (`mz-load/red.log`). It now checks complete FIST.DAT/MGAVIDEO/SOUNDDVR memory,
  real relocations, a 32-KiB boundary, short EOF and ZM/high-page-mask cases. Separate regressions
  cover segment wrapping (f000 + 2000 -> 1000) and application clock/BIOS initialization on both targets.
  Original read-budget provenance is `pic-mask/source-proof.json`; all 1151 individual production
  read budgets match it, and both application entries are cycle 629918/budget 82 (0036).
  `bash tools/check_flow.sh '^(intro|mainmenu)$'`: 55 tests, exact patches, native/WASM builds,
  2 selected flows pass / 0 fail, exit 0; `scratch/verify/run.y8zSMD/`.
- `mz-load/{final-native-10s,final-wasm-10s}/` retain 698 complete frames/endpoints and match each
  other and their preceding port captures bytewise. Every original palette/time matches; only pixel
  event 443 differs over these 10000 ms. `mz-load/proof.json` records the complete diagnostic scope.
  Full mixed port PCM remains absent. The loader/start-phase step is proved; pre-decoder DOS caps,
  kernel packet/gateway/REP work, remaining port I/O and full timing acceptance remain open.


- Current base `ce89956`: `fist_clock_charge_dos_transfer` implements DOSBox `dos.cpp`
  `modify_cycles(reg_ax)` on the shared active CPU budget: strict `4*AX+5 < remaining`, otherwise
  five cycles, including exact fractional-clock credits from budgets zero through four. Credits
  retain the current slice and do not call TIMER_AddTick. The actual original helper is extracted
  verbatim for `dos_delay_probe.cpp`, linked beside the original PIC queue and I/O helper.
  `test_dos_caps_and_credits_match_original_active_slice`: 109 cases on both targets (218),
  including empty budgets, intra-tick deadlines, tick-end credits and subsequent retirement.
- Ordinary 16-bit disk reads now share `local_file_read` with MZ reads: fread into DOS scratch,
  PIC mask access, guest copy, AX/CF writes, then callback cap; invalid handles return AX=6/CF=1
  and still charge the error AX without a PIC access. EOF/zero-length reads still access PIC.
  `test_dos_file_reads_match_original_buffer_register_and_budget_contract`: 11 cases on both
  targets (22), covering full/short/zero EOF, 65535 bytes, tiny budgets and invalid handles.
  The old production code fails all 22 cases (`dos-read-red.log`); both targets now pass.
  Extender packet/gateway/REP work, write callbacks, other file/device errors and complete PIC
  event/IRQ ordering remain open; this is the disk-read primitive, not complete DOS acceptance.
- `scratch/sequence-capture/dos-transfer/{check-proof.py,proof.json,native-10s/,wasm-10s/}`:
  both complete 10000-ms streams contain 698 frames and match the previous port bytewise.
  All original times/layouts/palettes match; pixel event 443 remains the only differing frame.
  All 1151 application read-budget records retain the previously proved trajectory; entry remains
  cycle 629918/budget 82. Mixed port PCM is absent.
- Fresh `dos-transfer/{original-30s/,native-30s/,wasm-30s/,30s-proof.json}` extends the diagnostic
  to 30000 ms: 2100 complete frames on each target, 1324058 mixed original PCM samples. Native/WASM
  frame bytes match, all original times/layouts/palettes match, and 23 pixel events differ (first 443).
  The final menu frame matches bytewise and was visually inspected. Port mixed PCM remains absent;
  this does not accept complete original parity.
- `bash tools/check_flow.sh`: 57 tests, exact patches, sequential native/WASM builds and the
  entire existing matrix pass (178 flows / 0 failures), exit 0; `scratch/verify/run.KtGi9K/`.
  No filter was used. Complete source replay patch (including the new probe wrapper) is
  `scratch/sequence-capture/dos-transfer/production.patch`; source hashes were checked unchanged
  after the gate. This validates the bounded primitive and existing matrix scope, not complete
  original frame/PCM parity or the final ten-run gate.
- Resident kernel provenance for the next Extender step: `FIST.RUN` has a 48-byte MZ header;
  module offsets 2578..2615 equal runtime CS=8 IP 18de..197b bytewise (delta c9a).
  Paging-aware GDB memory reads confirm 16-bit code, FS=10 base 26e0, fs:6e4=16384,
  fs:1f0=a000, and cs:1612=1614 (gateway module offset 22ae). The original file's initial
  packet field at module c8e is already 4000; memory-limit initialization can reduce it.
  Do not use the alternative DPMI gateway 168d or the decoded high-module `fist_image.bin`.
  Snapshot occurs at tick 6882, after callback 88, and establishes resident bytes/fields only.
  `kernel-map/{map.gdb,check-map.py,proof.json,phase-original/}`; the complete probed original
  10000-ms frame and PCM streams match `pic-mask/original-complete/` bytewise.

## Next

1. Preserve 0036's now-proved MZ load/application-fetch phase. Preserve the shared DOS cap/credit and
   16-bit disk-read primitives; implement Extender packet splits, gateway instructions and REP
   from the now-mapped original resident kernel.
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
