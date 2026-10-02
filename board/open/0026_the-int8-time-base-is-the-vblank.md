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
- Current base `159ea34`: Extender AH=3f now follows the resident VCPI kernel's 16384-byte
  packet loop, reached real/protected-mode gateway and installed INT 21h wrapper path.
  Each packet uses the existing local-file PIC access and DOS budget cap, then copies dwords
  and remaining bytes before testing the original unrequested remainder/short-read condition.
  Full EAX, residual ECX, EDX and CF follow 18de..197b; EBX is preserved. The packet buffer
  belongs to private kernel memory, not the caller's flat-module offset a000.
- `fist_kernel_read.h` records original instruction blocks by IP and module offset, rather
  than a measured total delay. MOV SS refunds its normal-core fetch. Shared REP MOVS reserves
  a budget chunk and completes its writes before PIC dispatch, including the original
  zero/one-count budget-one exception and sequential forward/backward overlap behavior.
  `rep_probe.cpp` executes original `core_normal/string.h` verbatim alongside the original PIC.
  Four complete pre-88 syscalls from the independently captured trace are versioned in
  `kernel_read_cases.json`: 8/768/8/19456 bytes, exact start/end cycle and budget, EAX/ECX/EDX.
  All eight target cases fail on the parent shim (`kernel-read/red-proof.json`) and now pass.
  Original-source REP memory/budget regressions cover 84 target cases; another 16 cover
  zero/short/packet-boundary EOF, 70000-byte EAX results and invalid handles.
- Scope is the reached 16-MiB VCPI boot, normal wrapper flags and clear-DF disk-read ABI.
  Memory-limit packet reduction, other gateways, critical-error branches, guest interrupt
  effects and complete generated caller register marshaling remain open. Source provenance:
  `kernel-map/trace-code-proof.json` matches 115 reached kernel opcode locations; the far JMP
  at 1432 has its observed relocated selector 02dd instead of file selector 006f.
  Its initialization remains unattributed; no operand mask is used to accept parity.
- `bash tools/check_flow.sh '^(intro|mainmenu)$'`: 60 tests, exact patches, sequential
  native/WASM builds and two selected flows pass / zero failures, exit 0;
  `scratch/verify/run.YW9ejP/`. This filtered gate proves its stated scope only.
  `kernel-read/{check-proof.py,proof.json,native-10s/,wasm-10s/}`: all 698 complete frames,
  indexed pixels, palettes and times now match the original, including event 443.
  All 1151 MZ read budgets and application entry cycle 629918/budget 82 are retained.
- Fresh `kernel-read/{native-30s/,wasm-30s/}`: all 2100 native/WASM frames match bytewise;
  every original time/layout/palette matches. There are 27 pixel-difference events, first 817
  at 11704156 us, byte 8754 (original 16 / port 212), 965 unequal pixels. Event 443 stays fixed.
  The corrected frame, first remaining difference and final menu were visually checked.
  Mixed port PCM is still absent; complete comparison fails. Frame diagnostics are not acceptance.
- Current base `5b8f0fd`, patch 621: reached KDV 7120 clears DF, retires its four setup
  instructions, copies 16000 dwords through the shared original-backed REP path, then retires RET.
  The old C loop writes all pixels without time. At callback 168, original before-call state is
  cycle 350841173/budget 8827 and before the next caller fetch is 350857178/budget 22822.
  REP splits into 8823/7177 dwords at TIMER_AddTick. `kdv_blit_cases.json` records the complete
  original CPU-trace hash and copy opcode bytes, independently confirmed through guest paging
  in original resident memory. No fitted elapsed delay is charged.
- `test_kdv_frame_blit_matches_original_copy_budget_and_aperture` compiles the actual 7120 body
  after every ordered Extender patch, without touching `build/`. Original DoString/PIC supplies
  the full 262144-byte memory image and cycle/budget results for five initial states on both
  targets, including budget-one and tick boundaries. Parent code fails all ten cases; patch 621
  passes all ten. `bash tools/check_flow.sh '^(intro|mainmenu)$'`: 61 tests, exact patches,
  sequential builds and two selected flows pass / zero failures, exit 0; `run.deIWNB/`.
- `pixel-817/{check-proof.py,proof.json,native-blit-10s/,wasm-blit-10s/}`: complete 10000-ms
  streams retain all 698 original frame/palette/time records. Both new 30000-ms streams contain
  2100 identical cross-target frames and preserve every original time/layout/palette; 27 pixel
  events differ, first still 817. This fixes the reached copy contract; upstream timing is open.
  Loader budgets/app entry are retained. Port mixed PCM remains absent; strict comparisons fail.
- The fresh complete original 11686:11696 trace attributes a missing Sound Blaster IRQ path
  before callback 168: protected handler 14e0 calls software mixer 2630, whose reached path
  emits 1024 bytes, with 12 setup + 44 instructions/sample + RET = 45069 instructions.
  Another 609 instructions belong to the nested timer interrupt; the combined interval consumes
  46373 cycles. No channel rollover is reached here. Port `fist_sb_set_irq_cb` has no caller.
  Do not inject this measured total: recover mixer/device state and its actual IRQ cadence in 0003.
  The read-only IRQ-7-armed GDB snapshot captures paging, cursors, pitches, buffers and actual
  self-modified channel SHR/ADD operands; its complete 13000-ms frame/PCM stream matches the
  unprobed original (908 frames / 574358 samples). Evidence: `pixel-817/{mixer-snapshot.gdb,
  mixer-registers.json,mixer-physical.bin,mixer-original-13s/,proof.json}`.

- Current base `bb25d36`, 0003's bounded active-mixer step restores the reached 2630 byte
  normalization/feedback/lookup contract. Its paired original state/output regression passes on
  both targets; shared extender operand resolution is reused. The 62-test/two-flow gate is
  `run.woiVQD/`. All 1151 loader budgets and application entry cycle 629918/budget 82 remain
  unchanged in fresh complete 10/30-second captures (`mixer-2630/proof.json`). This step adds
  no mixer timing: SB completion IRQ/device lifecycle and actual interrupt work remain open.

- Byte-operand step (0003, patch 623) preserves all loader budgets/app entry and the
  complete existing 10/30-second video results (`mixer-rollover/proof.json`). The 63-test builds
  and all 178 existing flows pass, exit 0 (`run.N3Lzlt/`). New original SB provenance belongs to 0003:
  DSP 1024-byte blocks differ from the DMA 2048-byte ring, and 11111-Hz/Q14 mixer demand gives
  92/93-ms IRQ tick intervals. Recover demand/DMA/PIC ordering, not a periodic measured delay.

- Default-RET callback step (0003, patch 624) fixes a reached native wrong-buffer/WASM-signature
  failure. The original CALL/RET preserves seven GP registers and pops four ESP bytes; do not
  assign decompiler extraouts to the live pitch/partial sample. The private 64-test/two-flow gate
  passes (`scratch/callback-worktree/scratch/verify/run.ZRagJp/`); canonical source inputs match.
  Fresh complete 10/30-second captures retain all loader budgets/app entry and frame bytes
  (`mixer-callback/proof.json`). This adds no timing: `fist_icall`'s old pump, full instruction
  retirement and actual SB DMA/PIC/ISR work remain unresolved.

## Next

1. Preserve the proved MZ/application phase, disk-read cap and reached VCPI packet/SS/REP contracts.
   Continue 0034's first remaining pixel difference at event 817 in the 30000-ms capture.
   Recover 0003's now-reaching SB IRQ/mixer state and instruction work; preserve the proved 7120
   REP copy. Do not inject elapsed delays or change capture phase.
2. Recover production pre-speaker work (23 mask reads, 720/721), 07b7's polling/CLI/STI and ISR return.
   Derive calibration arguments from operations and budget; never inject 90 or a measured total delay.
   Prove reload/re-arm/IRQ order. Supply events to 0003/0034; neither owns another clock.
3. Audit mouse scripts (r92/9200, debrief): ports use menu-relative vblanks, original XTEST wall time.
   Run `make web`; measure input/presentation, background/resume, audio continuity and
   Atomics.wait/shared-memory prerequisites. Pace at presentation boundaries, never host time.

## Accept

Original PIT/retrace/ISR effects and sample order, responsive browser play and cross-target parity.
Node matrix passes alone do not establish browser behavior.
