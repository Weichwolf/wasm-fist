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

- Current base `a88d3e2`: enabled SB/DMA byte ports now use the existing shared
  CPU I/O delay before device access, alongside PIC mask ports. Ownership is
  resolved once per access. Normal instruction fetch remains the caller's work;
  no second clock, fixed reset-loop count or fitted elapsed delay is added.
  Other ports still use the cooperative pump and remain open.
- `tools/oracle/sb_io_clock_case.json` records all 68 original 133a instructions
  through the reset-high phase, their opcodes/cycles and all 20 reads of 226=ff.
  The independently captured complete 525:526-ms CPU trace and 600-ms original
  frames/PCM/endpoint are revalidated by `sb-io-clock/check-proof.py`.
  This phase ends before the falling reset; reset readiness and IRQ/IF are not
  accepted. The original normal-core fetch and actual `IO_*_Delay`/PIC source
  independently supply all remaining-budget results.
- `test_sb_io_clock.py` links real port/SB/DOS owners on both targets. The reached
  phase plus four thin-budget/tick-boundary starts produce ten matching full
  68-row timelines, device reads and active budgets, without cooperative pumping.
  Parent code fails all ten cases: its first OUT ends at 15778287 instead of
  original 15778283 and routes 21 accesses through the unrelated PIT pump.
  Existing tests reuse one `build_pic_probe` owner; the original timing endpoint
  does not emulate SB data, whose reads are checked against the actual capture.
- `scratch/sequence-capture/sb-io-clock/{red-final-reaching.log,green-unit.log,
  check-proof.py,proof.json}` retains the bounded reaching proof. `bash
  tools/check_flow.sh '^(intro|mainmenu|audio-.*)$'` passes 77 tests, exact patches,
  sequential native/WASM builds and all five selected flows, with separately
  recorded exit 0; `scratch/verify/run.rjEjwk/`. Fresh complete 30000-ms captures
  retain all 2100 prior port frames and 43 register packets bytewise on both
  targets. All 418 original files remain unchanged. This filtered gate does not
  accept the full matrix, original video parity or absent final mixed port PCM.
  Generic PIC events, delayed SB reset, protected IRQ/IF, producer instruction
  work and final mixer integration remain open (0003).

- Current base `b70ebd4`: the shared CPU clock now owns
  generic PIC callbacks, with the source's 512 float-index entries, stable
  equal-time ordering, per-tick float subtraction, callback-relative re-arming,
  cancellation and active-budget requeue. Callbacks run at CPU dispatch after
  instruction effects. Existing PIT/VGA calendars are retained; interleaving,
  IRQ/IF and raw cooperative advancement with pending callbacks remain open.
- `test_sb_io_clock.py` now links the actual original PIC queue with the actual
  `DSP_DoReset`/`read_sb` through `sb_dma_probe.cpp`. Its complete 133a fixture
  contains all 218 instruction/I/O rows, all 55 reads and their remaining CPU
  budgets. Reset AddEvent returns 1052 cycles to CycleLeft at index 28948;
  the float deadline is 0.9849333167076111 and service occurs at index 29549.
  Read-only GDB snapshots preserve the complete 600-ms source outputs bytewise
  (39 frames / 27518 mixed PCM samples). Initial pointer/float-cast scaffold
  failures are retained and excluded from source proof.
- Six regressions cover 50 target phases: the existing high/thin-budget phases,
  complete reached reset, rising-bit cancellation/read latches/write status,
  stable equal-time callback insertion, specific/all cancellation, late relative
  re-arming, tick boundaries and queue capacity. The parent device owner fails
  all eight complete-reset/cancellation target phases. A separate native reaching
  failure at full capacity exposes x87 excess precision: store the original
  Float32 product before comparison/subtraction, as the source oracle does.
  Both targets now retain all 512 accepted events and match every timeline.
  An initial long capacity fixture crossed an uninstalled oracle PIT event;
  it is excluded and the isolated queue fixture stays inside its stated scope.
- `scratch/sequence-capture/sb-reset-event/{check-source.py,red-final-reaching.log,
  red-native-float-width.log,green-unit.log,tested-source-hashes.json,
  capture-configuration.json,check-proof.py,proof.json}` retains provenance and
  regressions. `bash tools/check_flow.sh` passes 81 tests, exact patches, both
  sequential builds and all 178 existing flows, with separately recorded exit 0;
  `scratch/verify/run.WZhm7Z/`. Every tested source/binary hash is retained.
  Separate read-only verification of the same frozen binaries passes all three
  existing audio flows with recorded exit 0. Existing masked/no-reference flows
  retain their limited scope; this is not the final full original-parity gate.
- Fresh complete SB-enabled 30000-ms native/WASM captures retain all 2100 prior
  frame bytes, endpoints and 43 sound-register packets. The first attempts with
  missing start/device settings are excluded from matching-source claims; the
  explicit manifest fixes the compared configuration. Device-off matched parent
  captures are additional diagnostics only. Final mixed port PCM, actual 133a
  producer ABI/CLI/STI, protected vectors/IRQs, complete PIT/VGA/event interaction
  and sound startup remain open; no full original-parity acceptance is claimed.

- Current base `6ed1aa0`: patch 629 retires actual 132f's normal-core
  PUSH/IN/JMP/OR/JS/POP/OUT/RET fetches at their original I/O boundaries through
  the shared PIC owner. The ready path has eight fetches and each busy iteration
  adds four; the live signed status branch determines polling. Full DWORD EAX
  is retained. No fixed loop count or measured aggregate delay is charged.
- `tools/oracle/sb_writer_132f_cases.json` records fourteen complete original
  calls: seven reached ready calls and seven from a controlled device fixture
  that sets only the first entry's write-busy counter to seven. Its eight ff
  reads then 7f produce forty fetches. All GP registers are preserved; RET
  advances ESP by four. Actual original `flags.cpp` resolves all twenty-eight
  captured lazy states to EFLAGS 3202, including IF enabled; raw cached CF is
  stale. Do not assume the earlier 138d CLI survives protected kernel services.
  Both complete 600-ms original captures retain all 39 frames, 27518 mixed PCM
  samples and endpoints bytewise. Source code/assets stay unchanged.
- `test_sb_writer.py` compiles the complete ordered-patched Extender module with
  real I/O/PIC/DSP owners. The independent oracle compiles original PIC/DSP/I/O
  source, replays every captured fetch and first matches every original byte,
  timestamp and remaining budget. All fourteen producer calls and eight
  ready/busy thin-budget/tick-boundary phases preserve full EAX on both targets.
  The parent fails all 44 target phases; first IN begins at 16067116 instead of
  16067118. Both targets now match each complete leaf timeline and active budget.
  This proves byte I/O/fetch/EAX, not guest-stack/caller flag transport or IRQ/IF.
- `scratch/sequence-capture/sb-writer-132f/{check-writer-source.py,
  source-contract-proof.json,parent-red.log,green-unit.log,check-proof.py,
  proof-final.json}` retains the reaching proof. `bash tools/check_flow.sh
  '^(intro|mainmenu|audio-.*)$'` passes 83 tests, exact patches, both sequential
  builds and all five selected flows with recorded exit 0;
  `scratch/verify/run.Bir92a/`. Initial compiler-flag, patch-header and conditional
  breakpoint scaffolds are explicitly excluded. Full frozen-source/binary
  hashes and the matched device/start-state capture configuration are retained.
- Fresh complete SB-enabled 30000-ms captures retain all 2100 prior port frames,
  endpoints and 43 sound-register packets bytewise on both targets; all 418
  original assets stay unchanged. All 27 original frame differences remain,
  first at event 817 / 11704156 us / pixel byte 8754. Strict original mixed-PCM
  comparison still fails because the final port stream is absent. This filtered
  regression does not accept the full matrix or final original parity. Actual
  133a/138d/startup, protected services, CLI/STI/IRQ and mixer integration remain
  open; restore their original producers, not their observed total elapsed time.

- Additional read-only reset producer ABI evidence, current base `fa57275`:
  `scratch/sequence-capture/sb-reset-producer/{check-source.py,abi-proof.json}`
  validates both complete 218-row original 133a branches. The normal source
  trace/output stays byte-identical. A controlled isolated fixture changes only
  the queued DSP response AA to AB before actual IN22A; the unchanged original
  executes its failure branch and returns EAX=0 instead of FFFFFFFF. Both return
  ECX=1f1e, EDX=22a, ESP+4 and effective EFLAGS=3246. This is an EAX result;
  neither branch returns success/failure through CF.
- Seven normal and eight controlled entry/CLI/STI/branch/RET/caller snapshots
  recover actual lazy flags through original `flags.cpp`. Entry EFLAGS=3293;
  CLI changes it to 3093. Immediately before STI it is 3002 and immediately
  after STI 3202. Success CMP and failure XOR both end at 3246. CPL=3, IOPL=3
  permit these original protected instructions. PIC_IRQCheck=0 and no IRQ is in
  service at every snapshot: this capture cannot establish pending IRQ delivery.
  Both captures complete 600 ms / 39 frames / 27518 mixed PCM samples. The wrong
  response changes frame bytes; PCM/end remain byte-identical. Never use that
  controlled failure as a normal-reference capture. The initial missing trace-
  window scaffold is retained and excluded. No port reset producer, caller flag
  transport, privilege/IF owner or protected IRQ integration is accepted here.

- Additional original completion-IRQ evidence, current base `1afdf78`:
  `tools/oracle/pic_irq_case.json` records the complete reached 14e0..153e
  prefix: 36 original fetch rows and all six byte I/O boundaries, with the
  original module-image, trace, PIC-source and DSP-source hashes. The prior
  SB acknowledgement fixture omitted immediate-port IN20 (E4); this case
  retains it. OUT20=0b selects ISR, IN20 returns 80 for the real IRQ7 service,
  OUT224=82/IN225=01 select PCM8 status, IN22e=7f acknowledges it, and
  OUT20=20 issues EOI. Command PIC ports have the same original I/O costs as
  data ports: 21 cycles per write and 29 per read at these reached budgets.
- `scratch/sequence-capture/pic-controller/{check-source.py,source-proof.json}`
  independently rechecks every captured opcode location, register-derived I/O
  byte, timestamp and before/after budget. Entry is cycle 350700134; the 153e
  boundary is 350700319. The complete original 13000-ms endpoint, all 908
  indexed frames/palettes/times and 574358 mixed PCM samples remain bytewise
  equal to the unprobed capture. Originals stay unchanged. This proves the
  original PIC query/acknowledgement prefix, not port controller state, CPU
  interrupt-frame construction, handler execution or complete output parity.

- Additional read-only IRQ-frame provenance, current base `28d01da`:
  `tools/oracle/sb_irq_frame_case.json` and
  `scratch/sequence-capture/sb-irq-frame/guarded/{check-source.py,proof.json}`
  recover the actual IRQ7 surrounding the first remaining video difference.
  Hardware vector 0f arrives at cycle 350700000 in real-mode BIOS 1119:e66f,
  EFLAGS=3216, SS:SP=2d19:fe88. The original CPU pushes a six-byte frame and
  enters IVT target 2dd:53ac with IF cleared. Its resident stub calls 2c3c;
  runtime stub creation remains unproved, not replaced by an invented vector.
- At cycle 350700133, resident 8:378f executes operand-32 IRET from CPL0 with
  a 16-bit stack. Its five DWORDs are 14e0/2b/3016/586a/37f6001b; the CPU
  selects SS=1b and enters CPL3, 32-bit CS=2b/base10000000, retaining a 16-bit
  stack. The full SS DWORD comes from packed BF82/BF84 fields. The handler's
  return CS DWORD is 0033002b, from packed resident CS/DS fields at 1016.
  Keep their complete original bytes; do not normalize upper words in evidence.
- Handler 2b:1562 executes IRET at cycle 350746707 and restores BF15/2b/3016,
  advancing ESP by 12. BF15 calls far through BF84: its stored pointer is
  78:37f6, but selector 78 is a DPL3 386 call gate targeting 8:3791. The
  descriptor supplies the destination offset. The resident return eventually
  performs real-mode IRET at cycle 350746805 to BIOS 1119:e66f/2d19:fe88.
  All eight GP registers, six segments and effective flags are restored.
  Cached CR3 changes 58aa→588a with CR0=0; complete CPU-state restoration is
  not claimed. All six IRET transitions, including the nested timer route,
  are retained in 15 source boundary snapshots and matched to actual fetches.
- The 207-byte resident entry region equals original FIST.RUN bytes at file
  offset 438c; the BF15 trampoline and BF84 pointer match the original flat
  module and paging-aware runtime reads. The complete new 13000-ms capture,
  all 908 frames, 574358 mixed PCM samples, endpoint and bounded CPU trace are
  byte-identical to the previous unprobed original. The initial debugger probe
  captured boot INT21 because its Python callbacks lacked explicit condition
  guards; it is retained and wholly excluded. This proves one original route,
  not other real/protected/V86 interrupt contexts, a target CPU/IF owner,
  actual ISR execution or complete video/audio parity.

- PIC controller implementation, frozen producer base `1afdf78`:
  handwritten `fist_pic.c` now owns both 8259 controllers, IRQ masks/requests/
  service state and vector programming on both targets. PIC command ports
  20/a0 use the same shared I/O budget as data ports 21/a1. The old duplicate
  mask bytes, constant-zero command reads and ignored EOI writes are removed.
  Mask/OCW3 changes return the active budget to the shared clock, as original
  `pic.cpp` does. Eligibility retains actual IF/trap gating, all 16 priorities,
  cascade masks, special masks, ICW single/auto-EOI and nested EOI order.
- Real PCM8 completion now activates the matched hardware IRQ7; reset high and
  a pending PCM8 acknowledgement deactivate its request. The wrong-width
  acknowledgement leaves it pending. This is request/service state; taking an
  eligible vector does not construct a CPU frame or execute a handler. The
  cooperative PIT route and legacy completion callback still need the actual
  CPU/IF/privilege/vector/IRET owner. Other hardware IRQ selections, mixer80/81,
  device variants and the legacy PCM16 producer remain unproved.
- `test_pic_controller.py` links actual original PIC/DSP/DMA/I/O producers.
  Eight methods cover 24 target phases, including all priorities, five thin
  budgets and the complete 36-row reached 14e0 PIC/SB I/O prefix. Every byte,
  fixture timestamp/budget, demanded device byte and pump count is compared.
  The prefix reproduces captured original I/O bytes/times; its empty event
  fixture has a different deadline calendar, so original captured active
  budgets are not claimed as matched. Three old-producer methods fail all
  14 target phases; first failures include ICW mask 01 instead of f8 and real
  PCM8 completion IRR=00 instead of 80. Both targets now pass.
- The PIT2 regression still checks all 17 ports and both speaker states. Its
  command-port expectations now use actual original 21-cycle write delay and
  `timer.cpp` float latches, proved by original I/O source and reached OUT20.
  The earlier eight obsolete expectation failures are retained; no port or
  speaker assertion was removed. `scratch/sequence-capture/pic-controller/`
  contains `parent-final-red.log`, `green-unit.log`, source provenance,
  the tracked/untracked frozen source archive and `check-proof.py`/
  `proof-final.json`. Separate short-transfer END_DMA_Event failure remains
  open in 0003; it is not excluded from reaching evidence or counted as fixed.
- `bash tools/check_flow.sh`, without a filter, passes 91 tests, exact patches,
  both sequential builds and all 178 existing flows with separately recorded
  terminal exit 0 (`scratch/verify/run.YUVdxO/`). The isolated browser build
  also compiles and links the same new PIC unit, exit 0, with JS/WASM/data hashes
  retained; browser execution/pacing is not accepted. The producer source
  hashes stayed unchanged throughout; intervening source-evidence commits
  28d01da/4fd0a54 do not alter these tested inputs.
- Complete matched SB-enabled 30000-ms captures contain 2100 identical
  cross-target frames/endpoints and identical 43 sound-register packets. All
  original times/layouts/palettes match. The PIC command I/O correction changes
  parent frame 1555: a visually checked scanline split, 1793 pixels from byte
  32000, adds one original pixel-difference event (28 total, first still 817 /
  11704156 us / byte 8754). All packet payloads match the parent; 22 timestamps
  change by -9..+21 cycles. Do not claim previous complete frame/packet bytes
  are retained or fit a delay to restore them. All 418 original assets remain
  unchanged. Strict comparison still rejects missing final mixed port PCM.
  This proves the bounded controller contract and existing matrix coverage,
  not full original output parity or the final ten-run WASM acceptance.

- Current base `21a422f`: consume 0003's short PCM8 `END_DMA_Event` recovery
  in `scratch/sequence-capture/sb-short-event/phases/`. Sixteen actual source
  phases and 32 target phases match every fetch budget, first sample boundary,
  complete diagnostic samples, remainder and request/ACK. The existing shared
  queue supplies the source float delay, requeue and cancellation; no private
  device clock, periodic IRQ or fitted instruction interval is introduced.
  All 100 tests, exact patch checks, sequential builds and all 178 existing
  flows pass in `run.bpay5o/`, zero failures and terminal exit0. Consume
  `sb-short-event/proof.json` for the frozen 886-source and capture checks.
  CPU privilege/IF/frame/IRET and actual startup remain open.

- Original CPU helper contract additionally executed from complete `cpu.cpp`
  in `scratch/sequence-capture/cpu-cli-sti/source.cpp`: all 256 synthetic
  CLI/STI caller contexts preserve whole CPU registers, segments and CPU block
  except the IF bit or prepared exception. The 72 denied contexts preserve
  flags and set GP13/error0; the 184 permitted contexts change only IF and
  preserve the prior exception payload. `source-proof.json` records original
  source/harness/output hashes. This is helper evidence, not guest exception
  delivery or a port CPU owner. The actual normal-core STI source branches to
  `decode_end` immediately when IF and PIC_IRQCheck are set under CPU_PIC_CHECK1;
  do not invent a one-instruction inhibit. Startup/CPU/IRQ integration stays open.

- Base `965beb8`, consume0003's actual early CRT relocation pair:
  `engine-resource-relocation/paired/` proves140 actual fetches for f842 withBX0,
  including f7c3's table resolution, PUSHF/CLI,16 vector pairs and POPF/RETF.
  The complete16-MiB memory delta, GP/segments and saved/restored flags match.
  BX0 does not skip installation. These fetches and real inherited IF/privilege
  state belong to future production integration, not an aggregate elapsed charge.
  The startup153c interval additionally attributes1410 fetches/16043 cycles,
  including14633 original callback cycles and854 descriptor-loop fetches. These
  measured totals are diagnostic; never add them as fitted delays. All complete
  source39frames/27518mixed samples/end600ms remain byte-identical. No port CPU,
  protected IRQ, final mixer or first817 acceptance follows from these pairs.

- Base `1472e73`, consume 0003's actual boot `222f -> 26fc` source pair:
  all 115 boundaries account for 114 fetches and three missing BACKLAND
  variants with inherited BP `1718`, unchanged segments and complete
  normal-return raw/lazy flags. The retained whole 16-MiB source pair has
  18 changed bytes; all 39 source frames/27518 mixed samples/end600ms match.
  Patch 632's two-target WORD bridge/data regression repairs DI aliasing BP;
  it does not execute those fetches on the shared clock or own CPU flags,
  stack, IF or guest IRQ delivery. The unfiltered canonical `run.dkymjy/`
  completed 101 tests, exact patches, both builds and 178 existing flows,
  durable exit0. Production startup, first817 and final mixed PCM remain open.

- Patch 633 consumes0003's actual original early CRT0174 install and
  boot222f live operands. Four reaching parent failures now pass across eight
  Native/WASM phase cases with complete16-MiB comparisons. Existing clock/DOS
  owners execute; no total140-cycle charge or incomingBP constant is inserted.
  All 892 frozen inputs match; unfiltered `run.Hw6Cpn/` completes 103 tests,
  exact patches, both builds and 178 existing flows, durable exit0.
  The shared relocation helper owns data writes, not original CPU flags,
  guest stack, fetch retirement/CLI/IRQ delivery or final mixer timing.

- Original153c controlled branch boundaries belong to0003's
  `resource-return-branches/`. Empty/CF-consumer cases take14/5 fetches and
  return normally; first/second table-overflow cases take14/35 but RET consumes
  savedDS=`2d19`, leaving DS at the resource and SP two bytes below the normal
  return. Complete GP/segments/raw+lazyflags/control and16-MiB pairs are kept.
  No total delay, repaired guest stack or full-output success is injected.
  All four intentional debugger stops explicitly fail complete stream/end
  validation; actual loader failure and full CPU/event integration stay open.

- Original task-mode get354/put358 startup proof belongs to0003:
  `task-mode-registers/` accounts for15 fetches and every stack/task write,
  all GP/segments/raw+lazyflags/control fields and unchanged600-ms frame/PCM/end.
  Far CALL/RETF materialize the pending original WORD ADD/OR flag state;
  the byte getter/exchange do not justify clearing unrelated flags. Actual
  SS resolves task1000:0. The port still discards the getter and uses stale
  SS literals in23bf. No total cycle charge or production CPU/IRQ proof follows.

- Consume0003's controlled original mode/pointer paths in
  `task-mode-controlled/`:15/15/13 fetches account for the exact BYTE
  getter/exchange and nullable task-store branches. Full raw/lazy flags,
  GP/segments/control and16-MiB writes are retained; the null path keeps its
  two-cycle advance without a fitted delay. Architectural test state is
  explicitly restored after observation, while time/budget remain untouched.
  Complete600-ms frames/PCM/end then match. This is controlled source evidence,
  not unchanged-state startup or port CPU/IRQ/time/mixed-output acceptance.

- Patch634's bounded BX/full incoming EBX data transport belongs to0003.
  Its frozen895-source gate `run.9hkvAE/` passes103 tests, exact patches,
  sequential builds, six complete startup cases and all178 existing flows,
  durable exit0. Complete30000-ms captures keep all2100 frames/end bytes,
  43 full sound-register rows and the first817 original pixel difference.
  No clock, IRQ/IF, guest-stack overflow or CPU flag claim follows; absent
  final mixed PCM still fails strict sequence acceptance. Consume
  `resource-bx/proof.json`; retain the unequal op68 full packets.

- Patch635's accepted task-mode BYTE/SS/caller contract belongs to0003.
  The frozen1005-input unfiltered112-test/sequential-build/six-startup/full178
  gate passes, exit0. Actual Native getter/setter leaves retain port clock
  individually, but the inherited resolver between them charges8 PIT counts;
  no whole-pair clock or original CPU/IRQ/IF/farstack/flags acceptance follows.
  Complete30000-ms captures retain every parent634 frame/end byte and43
  sound-register rows, with first817/28 original pixel errors, unequal full
  op68 packets and absent final mixed PCM. Consume the compact portable
  receipt `tools/oracle/task_mode_production_case.json`; do not add a measured
  aggregate delay or infer time correctness from the passing target matrix.

- Original sound138d vector initialization is now reproducible with
  `python3 -B tools/oracle/capture_sound_vector_init.py --repo /home/cosmo/Git/wasm-fist --output /tmp/wasm-fist-sound-vector-source`;
  receipt: `tools/oracle/sound_vector_init_case.json`. From0003's actual
  first138d entry, CLI clears IF. DWORD[12c4]=7 and the actual CL test/add
  select vector0f. AX2503 returns the original real-IVT DWORD in fullEBX
  (f0001060 here), preserves other GP/segments and leaves IF cleared. The
  caller saves that DWORD at12c0; PUSH DS/MOV CS,EAX/MOV DS,AX present
  original CS2b and fullEDX14e0 to AX2506. Actual resident1fba POP WORD
  installs the protected selector without overwriting its upper padding;
  resident1fbe writes the fullDWORD handler offset. Its real guest paging
  resolves that slot to physical13f878. Resident1fd6 OR BYTE activates
  the original existing stub at physical817c+3. The reached resident
 2237/223b/223e/2241 sequence builds the real-IVT pointer with DWORD SHL,
  WORD MOVZX/SHL/ADD preserving the segment high WORD; its base comes from
  the actual ES:0206 table, not a fitted stub address. AX2506 eventually
  installs that computed2dd:53ac in the real IVT. Crucially, actual
  resident225b STI enables IF before the service returns; all caller GP,
  including full savedEBX, remain intact. The remaining initial creation
  of the stub's CALL bytes is still unproved: it exists before this setter,
  while the original asset has zero bytes at that region. Do not invent it.
  Thirty complete16MiB/GP/segment/raw+lazyflag/control/time boundaries and
  all507 observed fetches supply23 direct instruction/whole-RAM transitions.
  Actual stack masks and GDT/IDT base/limit are also captured. Paging/state
  reuse the file-error owner; resident code provenance consumes the previous
  IRQ-frame asset offset and verifies each complete reached instruction.
  Fresh independent baseline/observer pairs, verify-only replay and the
  prior effects source match every39 frame/27518 mixed-sample/end600 byte.
  All419 original files and their inventory remain unchanged. Five isolated
  negatives reject missing handler RAM, a widened selector write, omitted
  stub activation, coherently lost kernel IF and coherently lost upperEBX.
  Evidence: `/tmp/wasm-fist-sound-vector-source-public` and
  `/tmp/wasm-fist-sound-vector-source-negative/proof.json`.
  This accepts the reached default IRQ7 original vector/IF prefix only.
  Other vectors/errors, initial stub creation, port protected DOS services,
  CPU/IRET/IF ownership, actual138d/DMA/IRQ/time execution and final mixedPCM/
  complete original parity remain open. The separate642 candidate's1078
  frozen runtime inputs/archive remain unchanged while its full gate runs.

- Original resident CALL-stub creation is now reproducible with
  `python3 -B tools/oracle/capture_resident_stub_creation.py --repo /home/cosmo/Git/wasm-fist --output /tmp/wasm-fist-resident-stub-source`;
  receipt: `tools/oracle/resident_stub_creation_case.json`. From actual4242,
  the resident derives ES from its DS:0114 stack size and actual SS, saves
  WORD DS:0204, computes the table's code-relative base and saves WORD
  DS:0206/CS:2c3a. One shared resident-image owner recovers the nested MZ
  header from the existing IRQ asset anchor and applies all33 WORD
  relocations using actual code base and initialCS. In particular425d's
  raw6f operand is relocated to2dd by actual load segment26e; no code-byte
  mask substitutes for relocation. The actual non-C0 branch forms its
  initial DWORD via WORD SUB/WORD MOV and DWORD SHL/ROR. The loop at429b
  executes256 STOS DWORD/SUB EAX,400h/LOOP operations with real16-bit DI/CX.
  Upper EDI remains1:0000 through1:0400 rather than being cleared. Complete
  table RAM equals all256 modeled writes; every CALL rel16 reaches2c3c
  with a zero activation byte. The derived table is here physical8140 /
  code-relative5370, explaining vector0f's prior e88dd800 stub at817c
  without fitting that address or those bytes. Registration/activation
  remain owned by the preceding sound-vector proof.
  All794 actual fetches are checked against complete relocated code;
  793 direct instruction transitions preserve full GP/segments/raw and lazy
  flags/control state and retire one cycle each. Eleven complete16MiB
  snapshots supply ten whole-RAM transitions, including initial pointer
  stores and the complete table. No aggregate793-cycle delay is supplied.
  Fresh independent baseline/observer captures and verify-only replay
  preserve every39 frame/27518 mixed-sample/end600 byte, also matching the
  earlier vector source. All419 original files/inventory remain unchanged.
  Five negatives reject missing terminal RAM, a narrowed first STOS,
  coherently lost upper DI, coherently lost SUBD lazy flags and coherently
  unrelocated code operands. Evidence:
  `/tmp/wasm-fist-resident-stub-source-public` and
  `/tmp/wasm-fist-resident-stub-source-negative/proof.json`.
  This accepts original default resident creation only. Alternate C0/error
  branches, port resident/MZ/CPU/IRQ/time execution and complete original
  frame/mixedPCM parity remain open. The separate643 candidate's1089
  frozen runtime inputs remain unchanged while its full178-flow gate runs.

- Consume0003's `device_reset_case.json`: the actual original23c4 tail JMP
  and133a reset match all218 existing PIC/SB/I/O clock rows, with full224
  register/segment/raw-lazyflag/control/CPU-budget transitions. The real
  falling reset write requeues CPU_Cycles into CPU_CycleLeft and exits the
  normal core; FillFlags materializes XORb. Later natural exhaustion after
  TESTb causes a second FillFlags before PIC_RunQueue. The next observed
  budgets are598/Left452 and450/Left0, derived by the existing original
  clock owner and normal-core post-decrement, not fitted elapsed delays.
  CLI clears IF and STI restores it; neither discards lazy flags. Seven
  behavioral verifier negatives and fresh39-frame/27518-PCM/end600 pairs
  pass; all419 originals remain unchanged. The existing six clock tests
  pass Native/WASM in15.080s. Port producer/CPU/IRQ/IF execution and full
  original frame/audio parity remain open; source proof is not integration.

- Accepted644 consumes0003's complete reset source states with one
  `FistCpuState` owner. Binding borrows the actual caller context; it seeds
  no registers or flags. The existing shared clock materializes lazy flags
  at a bound normal-core exit before PIC_RunQueue, preserving all dirty
  operand words, prev_type, oldcf and unrelated raw flags. Both reset exits
  and all225 dispatch/219 direct fetch states match the original on both
  targets. A causal missing-FillFlags negative fails four complete traces
  first at1358, with preceding registers and budgets unchanged; actual
  original flags.cpp verifies1029 primitive cases per target. Full gate
  `run.9fdN3p` passes140 tests, exact patches, both sequential builds,
  six startup cases and all178 flows. Additional full captures terminate0
  and retain the prior first817/missing mixedPCM diagnostics. Consume
  `tools/oracle/device_reset_production_case.json` for immutable1353-source,
  419-original, binary and capture bindings. Legacy unbound C paths and
  actual77e2/138d, privilege exceptions, IRQ/IRET/IF transport and complete
  original frame/audio parity remain open.

- Consume0003's `device_start_prefix_case.json` for the real77e2 caller
  around the accepted reset. All242 complete states include raw/lazy flags,
  full GP/segments/control fields and exact CPU budgets. The two DWORD near
  CALLs and1280 RET have actual guest-frame transitions; the additional17
  caller/configuration instructions retire individually. The225 reset rows
  reuse the existing source owner without narrowing any architectural bits.
  Post-reset OR AL,AL creates lazyORb/type4, which must survive the caller's
  next fetch and any later core exit. Nine whole-RAM boundaries and fresh
  39-frame/27518-PCM/end600 pairs pass; six coherent negatives are rejected.
  Six clock, three reset and three configuration regression methods pass
  both targets, while every1353 accepted644 runtime input retains its bytes.
  This is original-source evidence; full startup CPU/guest-address/IF/IRQ
  execution and complete frame/mixedPCM parity remain open.

- Consume0003's accepted645 full configuration/resident-RAM contract in
  tools/oracle/device_config_cpu_production_case.json. Configuration and
  reset borrow one actual full CPU context and use one segment/paging RAM
  owner, including real nearRET. All nine configuration and225/219 reset
  fetch states and complete16MiB physical RAM match on Native/WASM; no
  synthetic guest address supplies the clock. Thin-budget configuration
  cases consume original PIC reference timing. The immutable1154-input/
  248-reference/419-original gate passes140 tests, exact patches, both
  sequential builds, six startup cases and178 existing flows; additional
  complete captures and post-cleanup replays finish0. This preserves the
  reached reset core exits. Post-reset ORb, actual77e2/138d/IRQ/IRET/IF,
  unsupported memory/privilege paths and complete original time/output
  remain open; full flow coverage is not original sequence acceptance.

- Consume0003's device_checkpoint_case.json for126 additional actual
  startup/checkpoint instructions beyond the242-state prefix. All368 full
  CPU states and eleven whole-RAM boundaries are verified; the decoder
  uses actual original ALU/condition producers for all75 flag operations.
  INC/DEC materialize incoming CF before changing the lazy tag, retaining
  untouched var2/oldcf/prev_type. Raw CF and the INC/DEC operand state must
  survive later normal-core exits. Eight coherent/missing-output negatives
  reject invalid evidence, and fresh original39frame/27518PCM/end600 pairs
  plus post-cleanup replay retain all output. Six clock/three reset/three
  configuration methods pass both targets; accepted645 runtime is unchanged.
  No new production gate, full caller/IRQ/IF timing or sequence acceptance
  is claimed. The reached DWORD arithmetic/logical and INC/DEC flag types
  still require integration in the shared CPU owner.

- Accepted startup/checkpoint CPU flag dependency from0003:
  tools/oracle/cpu_startup_flags_case.json supplies seven reached missing
  lazy types with actual original CF/ZF/FillFlags and ALU references.
  Native/WASM pass2426 flag records, all368 complete37-word CPU contexts
  and75 checkpoint ALU/condition calls. INC/DEC preserve incoming CF and
  untouched var2/oldcf/prev_type; a var2 mutation fails both reaching paths.
  This is a primitive/context dependency, not new CPU/event integration.
  Consume cpu_startup_flags_production_case.json for the immutable143-test/
  178-flow gate and complete additional captures. No new clock work is
  claimed. All fresh2100-frame output,43 sound rows and47 unmasked service
  packets retain645; original first817, missing final mixedPCM and
  missing-HIGH error output remain unequal. Post-cleanup capture/source
  verification passes. The six prior one-cycle PIT budget differences
  remain a separate unresolved clock diagnostic.

- Original PIT0 event lifecycle source receipt at parentc71c533:
  tools/oracle/pit_event_case.json and capture_pit_events.py replay actual
  timer.cpp/PIC producers, not port timer math. All736 complete default
  budget states and nine synthetic counter/control/latch sequences retain
  all15 stored PIT fields plus output level, queue float bits/deadlines, PIC request state, CPU debt
  and read returns. The forwarding IRQ observer leaves every baseline state
  unchanged;23 complete IRQ observations are retained. Mode2 count changes
  are deferred until the old event, mode0 is one-shot/reloads remove old
  events, and mode3 can retain its first queued deadline after a count write.
  Partial writes, mode aliases, BCD-zero, value/status latches and low-output
  control are captured. The latter activates IRQ0 and raises a21-cycle
  budget to25; the source virtual index moves25186 to25182. No guest IRQ
  construction is executed: the timing reference has rawFLAGS0; port
  diagnostics receive the368 complete original37-word CPU contexts.
  This is a device/time property fixture, not a synchronized caller replay.
  Native/WASM each expose the same six one-cycle failures, first case53/
  step1 at1620001 cycles: port27761 versus original27760. Original queue
  deadline27761.994140625 minus index1 truncates27760. The original timer
  schedules its float delay through the PIC queue and callback-relative
  residual; port cpu_next_slice instead limits by the rational counter wrap.
  Both budget diagnostics explicitly record six attempted legacy INT8
  deliveries; missing guest IRQ effects are not emulated or accepted.
  The strict --require-budget-parity command exits1. Five verifier negatives
  reject missing original/port output, a coherent one-cycle adjustment,
  a changed float deadline and changed IRQ request. Existing24 port-I/O
  methods in217.736s and six clock methods in24.613s pass. Source captures
  are under/tmp/wasm-fist-pit-events-source-public; superseded preparations
  are retired. Runtime/generated engine/patches remainc71c533; no new
  production gate or full clock/IRQ/IF/IRET/frame/PCM acceptance is claimed.

- Original IRQ0 frame/return source receipt at parentf320fcc:
  `pit_irq_frame_case.json`, `pit_irq_frame.gdb` and `capture_pit_irq_frames.py`
  retain128 actual hardware entries,2196 complete handler fetch states and18
  full16MiB boundaries over2000ms. The first distinct BIOS, loader and
  protected IDT destinations are selected from actual PIC/vector state;
  no fitted selector, handler address or interrupt time selects them.
  Nine reached CPU_Interrupt/IRET transitions preserve all CPU/system words
  and compare every RAM byte. Protected IRQ0 at1089ms enters from CPL3,
  reads the actual TSS stack, constructs a32-bit inward frame on a16-bit
  stack and returns through two real16-bit IRETs plus the protected32-bit
  outer return to CPL3. The complete original137-frame/89258-mixed-sample/
  end2000 output matches the unobserved control and an independent fresh
  replay;148 original producer inputs and419 unchanged game files bind it.
  SHELL_Init's actual128-byte CommandTail copy leaves108 suffix bytes
  uninitialized. Its raw source/guest copy is observed; all cross-replay
  RAM differences are attributed to that input. Per-run full-RAM hashes
  remain visible, and every API transition stays strict without normalization.
  This is a CPU/event/output property reference, not identical whole boot
  RAM across runs or interpretation of every handler instruction.
  Eight negatives reject missing fetch/completion/boot-producer output,
  unfinished returns, changed TSS/frame state, coherently changed register
  records and recursive negative destinations. Initial preparation failures
  and the corrected reference scope are retained in preparation-errors.json.
  Evidence: /tmp/wasm-fist-pit-irq-frame-source-accepted; capture, independent
  replay, negatives and post-cleanup verification exit0. Superseded prototypes,
  duplicate replay and isolated games retired1876048815bytes. The current
  full check_flow.sh run.pTyl5c finishes0:143 tests in225.200s, exact patches,
  both sequential builds, six resource-start cases and all178 existing flows
  with zero failures. Immutable1409 source inputs and their archive remain
  unchanged; full-matrix-receipt.json binds the tested binaries and logs.
  Completed matrix captures and isolated resource-start data retired
  4823073684bytes; compact logs, producer summaries, source archive and tested
  binaries remain. All1409 source inputs, logs and retained binaries verify
  unchanged; the source fixture verifies0 again after cleanup. Existing
  filter/no-reference scopes retain their limits. Runtime remains
  c71c533. The six strict PIT budget failures still reproduce per target;
  no port IRQ/IF/IRET/time, first817 or final mixedPCM fix is claimed.

- Original system-load source extension at parent83bca05:
  The source observer and `pit_irq_frame_case.json` retain four reached system loads
  plus nine IRQ/IRET transitions,26 complete16MiB boundaries and all58 CPU/system
  words, including cached TSS descriptors and exception fields. At15ms the actual
  resident calls CPU_LGDT/LIDT/LTR and later restores the real IDT. LTR reloads
  the descriptor, preserves raw TSS kind8 and writes its busy bit to the cache
  and actual GDT. No LLDT call is reached. The complete137-frame/89258-sample/
  end2000 output is unchanged. Eleven verifier negatives reject incomplete
  system output, missing busy writes, cache corruption and the previous IRQ
  defects. Evidence: /tmp/wasm-fist-cpu-irq-system; source/control capture,
  fresh test-driven source replays and negatives finish0. Actual128-byte boot
  tail input and per-run raw RAM hashes retain the previous attributed-input
  scope; no cross-run RAM normalization or identical boot RAM is claimed.
  The capture wall timeout is configurable and80s for this observer; emulated
  endpoint remains2000ms. A preparatory32s wall timeout failed and is superseded,
  not accepted. Superseded unmanifested system captures and old private output
  RAM retired1649678584bytes; compact records remain. Current source inputs and
  originals verify unchanged and the source fixture verifies0 after cleanup.
  A private13-pair CPU/system prototype matches all58 words and
  every16MiB on Native/WASM. Shared production helper integration and the full
  existing matrix are separate pending work; no complete original video/PCM
  improvement, actual caller/handler transport or timing acceptance is claimed.

- Shared CPU/system helper dependency at parent5a7507d:
  `re_out/fist_interrupt.h` and the existing `fist_cpu.h` implement the reached
  system/IRQ/IRET operations with one RAM/descriptor/stack owner. CPU ABI remains
  37 words; system state has21. Complete thirteen-pair CPU/RAM regression and
  four causal mutants pass on both targets with NDEBUG. Frozen1412 inputs bind
  run.g2L0Tw:147 tests in357.144s, exact patches, sequential Native/WASM builds,
  six resource-start cases and all178 existing flows pass with zero failures
  and terminal0. Source150 producers and419 originals remain unchanged.
  Additional complete30s Native/WASM captures preserve all2100 preceding
  frame/end bytes,43 sound-register rows and47 unmasked service packets.
  Every original layout/palette/time and first10s video remain equal; the same
  28 pixel differences begin817/11704156us/byte8754. Final mixed portPCM is
  absent and strict original comparison fails. The first extra WASM attempt
  failed automatic Node discovery before execution; explicit NODE=/usr/bin/node
  completes the fresh retry. No partial run is accepted. Completed matrix raw
  captures/games retired4823073685bytes, with compact startup records, bound
  logs, frozen tested binaries and source archive retained. Additional obsolete
  isolated games retired48832310bytes. Source verifies0 after cleanup.
  Receipt: tools/oracle/cpu_interrupt_production_case.json; evidence:
  /tmp/wasm-fist-cpu-irq-system. This accepts the shared helper dependency only.
  Actual caller/PIC/handler integration, task/V86/fault/MMIO and cold-page
  paths remain open; no first817/time/mixedPCM improvement is claimed.
  A separate private original-byte LGDT-to-LTR decoder matches nine reached
  normal-core fetches, three API exits, all58 CPU/system words, four clock
  fields and every intermediate/final16MiB on both targets. Its source/control
  output remains137 frames/89258 samples/end2000. Two causal mutants per target
  prove the original MOV-SS budget credit and fully-linked read dirty-bit
  effects. Evidence:/tmp/wasm-fist-cpu-system-caller-preparation. Its CPL0/MIXED
  writable/user-page prototype is not a general paging owner or production
  startup integration. The repeated protected-IRQ busy-TSS
  boundaries are now supplied by the accepted source extension below.


- Accepted resident bootstrap source extension at parent531549e:
  The existing IRQ observer has an optional bootstrap mode; the default keeps
  its previous source contract. First startup and the actual selected protected
  IRQ retain eighteen complete16MiB fetch boundaries and eight system API pairs,
  alongside all128 hardware entries/2196 handler fetches/nine IRQ-IRET pairs.
  Complete original137-frame/89258-PCM/end2000 output and419 originals remain
  unchanged. Relocated resident bytes, complete reached SS AND CPU/RAM effects
  and MOV-SS budget credit verify; other instruction interpretations remain open.
  `resident_bootstrap_case.json` consumes the shared IRQ case rather than copying
  its full trace. Reproduce with `python3 -B tools/oracle/capture_resident_bootstrap.py
  --repo . --output /tmp/wasm-fist-resident-bootstrap-replay`; its --verify-only
  mode verifies the same source. Five negatives finish0 and reject incomplete
  fetch/RAM output, omitted busy clear, wrong AND tag and omitted MOV-SS credit.
  Evidence:/tmp/wasm-fist-resident-bootstrap-source and
  /tmp/wasm-fist-bootstrap-negatives-retry. A private decoder matches every58-word
  CPU/system, four-clock and intermediate/final16MiB boundary on both targets
  for both contexts. Omitting only the AND RAM write passes first startup but
  fails repeated LTR with GP on both targets; final RAM alone still agrees.
  Evidence:/tmp/wasm-fist-cpu-system-repeat-preparation. Duplicate private RAM
  and games retired1563036796bytes after canonical replay. Prototype paging
  remains CPL0/MIXED/present/writable/user only and is not a production bus.
  Frozen1415 inputs bind run.7hF5Di:147 tests in260.263s pass, exact patches and
  both sequential builds, six startup cases and all178 existing flows pass
  with zero failures and terminal0. Completed matrix/build/game data retired
  4862223653bytes; bound logs, compact startup records, frozen binaries and source
  archive remain. Source verifies0 after cleanup. Receipt:
  tools/oracle/resident_bootstrap_production_case.json. No production
  startup/PIC/handler/first817/PCM improvement is claimed.
  A private cache-aware InitPage observer now records six actual bootstrap
  reads and twelve whole16MiB API boundaries. First startup sets PDE accessed
  and PTE accessed/dirty on reads; repeated PG enable leaves those RAM bits
  unchanged but starts with an empty linked-page list and invalid read/write
  handlers, then relinks the actual RAM pages. Raw host pointers, physical
  slots, lists and handler types remain per-run diagnostic evidence. Complete
  CPU/time/RAM/cache transitions and original137/89258/end2000 output verify0.
  Evidence:/tmp/wasm-fist-resident-paging-cache-preparation/page-proof.json.
  Scope remains MIXED/CPL0/present writable user pages. Other permissions,
  architectures, faults/MMIO and the mutable production bus remain open.
  Older no-cache source and completed mutant RAM retired1764363388bytes;
  current canonical source boundaries remain. This preparation is not runtime
  integration or full original-output acceptance.


- Accepted paging-control source extension at parentddc0363:
  `capture_paging_control.py` reuses the accepted bootstrap observer and retains
  only eight new whole16MiB CPU_WRITE_CRX entry/return boundaries. The actual
  non-inlined entry is required: CPU_SET_CRX resolves optimized inline/clone
  locations and did not provide complete pairs in the discarded first attempt.
  Four actual CR3/CR0 pairs preserve every CPU/system/time field and whole RAM.
  CR3 with PG disabled preserves the complete linked-page list; PG enable clears
  read/write pointers and replaces handlers for all161 first-startup and20
  repeated-bootstrap linked pages, while retaining physical slots. Raw host
  pointers/types/flags remain per-run evidence; the source fixture compares
  portable validity, slots, handler properties and lists. All shared128/2196
  IRQ and18 bootstrap fetch metadata and complete137-frame/89258-sample/end2000
  output remain unchanged;419 originals verify unchanged. Five negatives reject
  incomplete CRX/RAM, omitted invalidation, lost physical slots and premature
  CR3 invalidation. Reproduce with `python3 -B tools/oracle/capture_paging_control.py
  --repo . --output /tmp/wasm-fist-paging-control-replay`; use --verify-only
  to check the same source. Evidence:/tmp/wasm-fist-paging-control-source and
  /tmp/wasm-fist-paging-control-negatives. Frozen1421 complete source inputs
  bind run.q4wSw6:147 tests in535.083s and exact patches pass. Initial native
  compilation failed2 because the supplied OBJDIR was absent; after creating
  staging directories, resumed sequential Native/WASM builds, six startup cases
  and all178 existing flows pass with zero failures and terminal0. Sources remain
  unchanged; the failed initial build and exact resume command are retained.
  Completed matrix/build/game raw data retired4862222187bytes; bound logs,
  compact startup records, frozen tested binaries and source archives remain.
  Source verifies0 after cleanup; isolated source games retired19532924bytes.
  Receipt:tools/oracle/paging_control_production_case.json. This is source
  observation only; no production RAM/startup/IRQ/first817/PCM fix is claimed.
  Private typed mutable RAM preparation now shares one CPU/system/cache owner
  across resident accesses and IRQ/system/stack helpers. Both targets match all
  seventeen actual API pairs, six InitPage pairs and both complete original-byte
  LGDT-to-LTR paths, including58 CPU/system words, four clocks where observed,
  all intermediate/final16MiB, touched InitPage cache slots/list and all initially
  linked CRX cache slots/list. Three causal CRX mutants reject omitted cache
  invalidation, lost physical slots and premature CR3 invalidation even though
  CPU/clock and RAM still agree. A separate omitted-read-dirty mutant fails all
  three first-startup RAM pairs on both targets; repeated pairs alone still pass
  because input PTEs are already dirty. Evidence:
  /tmp/wasm-fist-typed-ram-preparation/{trace,owner,crx,dirty-mutant}-proof.json.
  Completed duplicate output RAM retired905969664bytes; canonical original RAM,
  compact proofs/logs, code and binaries remain. Unsupported permissions/models,
  faults/device paths and production RAM/startup/IRQ integration remain open.
  This preparation does not fix first817 or supply final mixedPCM.
  A reaching probe against the unmodified production hot-only RAM helper now
  aborts all three actual first-bootstrap cold-page inputs on Native/WASM; the
  original InitPage succeeds and sets full-RAM accessed/dirty. Repeated inputs
  alone pass its address/RAM check because bits are already set, while required
  cache relinking is not represented. Evidence:
  /tmp/wasm-fist-typed-ram-preparation/cold-legacy-proof.json. This establishes
  the reached migration defect, not an attribution of first817 or mixedPCM.
  Actual device-prefix system-context preparation now records all21 system
  words at every242 original startup/configuration/reset fetch. The existing
  complete CPU/code/time/RAM contract is reverified unchanged in a metadata view
  containing its original fields; the added system words are independently
  recorded, not a full58-word port regression. Paired39-frame/27518-sample/end600
  output and419 originals remain unchanged. MPL is3, cached TSS kind is8 and
  exception.error remains2 throughout this reached prefix; transport the actual
  recorded inputs rather than synthesize a zero system context. Evidence:
  /tmp/wasm-fist-device-system-preparation/source-proof.json. Nine current full
  RAM boundaries remain for additional shared-owner consumer regression.
  Private direct typed-RAM migration of1280 configuration,23c4 dispatch and
  133a reset now accepts one RAM context with actual37 CPU and21 system words.
  Both targets match all9/225/219 complete58-word fetch states, original times
  and full result16MiB through shared resident/RET access. Input generated C
  is frozen privately; repository generated sources remain pristine. Evidence:
  /tmp/wasm-fist-device-ram-consumer-preparation/producer-proof.json. Duplicate
  input/output buffers retired50331880bytes. An additional actual controlled
  source retains all21 system words and restored39/27518/end600 output. Both
  targets now match all58-word controlled configuration fetches and complete
  RAM at its original phase plus three thin budgets supplied by the existing
  source-backed PIC owner. Alternate budgets are composition evidence, not new
  independent hardware captures. Evidence:controlled-proof.json beside the
  consumer proof. Physical RAM/ROM/device providers, outer startup transport
  and public adoption remain open; it is not production or complete original acceptance.

Eight original CRX boundaries now retain all4096 physical provider slots,
actual RAM/ROM/VGA handler classes and flags, the complete272-entry first-MB
map, separate A20 enabled/controlport and LFB handler/range state. The first
context has4064 RAM/24 ROM/8 mapped VGA pages; the repeat context has4056
RAM/24 ROM/16 chained VGA pages. All four first-MB remaps224..227 to304..307
persist. A20 is enabled while its controlport is0. Source CPU/system/time,
all eight16MiB CRX buffers, every initially linked cache entry/list and
137 complete frames/89258 mixed samples/end2000 reverify unchanged. Seven
negatives reject missing boundaries/slots/map entries, writable ROM, a
cache/provider mismatch, A20 derived from controlport and an invented identity
first-MB map. Evidence:/tmp/wasm-fist-physical-provider-source/proof.json and
/tmp/wasm-fist-physical-provider-negatives/proof.json. This is observation only;
physical read/write execution, A20 transitions and production adoption remain
open. Physical PDE/PTE primitives bypass handlers through MemBase; logical
same-page word/dword accesses retain handler width, while page-crossing
accesses split into bytes. Recover and regress those actual routes rather than
flatten all accesses into writable RAM or byte-only device callbacks.
The original provider observation is accepted with the complete existing
147-test/178-flow matrix, exact patches, sequential Native/WASM builds and six
resource-start cases, terminal0 on unchanged1464 frozen inputs. Receipt:
tools/oracle/physical_provider_production_case.json. Matrix raw captures and
disposable builds retired4862223374bytes; current canonical eight source RAM
boundaries and compact evidence remain. Source/control game copies and
completed private result buffers also retired; all three current original
source verifiers replay0 after cleanup. Originals419 unchanged.
Private physical-owner preparation now retains independently initialized
handler mappings even when direct pointers are zero; ROM writes remain ignored,
first-MB maps are restored rather than synthesized and A20 changes only its
enabled/map state. Direct physical PDE/PTE primitives bypass providers, and
logical same-page word/dword device calls retain width. The first/repeated
bootstrap, seventeen API pairs, six cold-page pairs and four CRX pairs match
both targets. Five source-composition mutants reject writable ROM, identity
first-MB, byte-only callbacks, coupled A20 controlport and guarded physical-ROM
primitives. These controlled operations compose extracted original routines;
they are not independent whole-DOSBox captures. Evidence:
/tmp/wasm-fist-physical-ram-preparation/{trace,owner,crx,provider}-proof.json.
The actual outer77e2->7809 caller now privately executes its original instruction
bytes through the one CPU/system/physical owner and the three typed device
producers. Both targets match all242 complete58-word fetch states, original
clocks and nine whole16MiB boundaries. Widened CMP, synthetic zero system and
eager JE flag-filling mutants reach the complete prefix but reject even though
all RAM still matches. An additional actual source observation records physical
providers at all nine prefix boundaries, preserving39/27518/end600 and the
existing CPU/RAM/system proof. Controlled configuration and three existing PIC
thin budgets still pass both targets. Evidence:
/tmp/wasm-fist-device-physical-consumer-preparation/{producer,controlled,outer}-proof.json.
An additional original prefix observer now retains the actual VGA aperture,
banks, wrap,2MiB linear storage and4MiB fastmem at all nine boundaries; buffers
and fields remain unchanged across this reached prefix. Private extracted-source
VGA_ChainedVGA_Handler and controlled VGA_Map_Handler composition match both
targets on complete16MiB RAM/all VGA buffers/read/cache/state/clock outputs.
Original whole-width first-line replication at start offsets318/319 is required:
byte-only writes can match RAM and linear storage but differ in fastmem.
Four mutants reject RAM aliasing, byte-only callbacks, omitted first-line
replication and a lost mapped-page base in the modes that reach each failure.
This combines explicitly declared source substates from independent boundaries;
it is not an independently captured whole VM state or visual/frame acceptance.
Evidence:/tmp/wasm-fist-vga-memory-preparation/proof.json. Private provider,
outer-caller and VGA preparation remains unadopted; other VGA/LFB/MMIO handlers,
register transitions, faults and first817/final mixedPCM remain open.

Current base `eea3cf6`, patch646: one mutable
CPU/system/physical-memory context now replaces the previous hot-page-only RAM
helper in descriptor/stack/IRQ/IRET and the three typed1280/23c4/133a producers.
Original provider/cache/VGA inputs are captured at every selected full boundary,
including52 first/repeated bootstrap/IRQ boundaries and nine device boundaries.
The original complete output/CPU/code/time/RAM verifiers remain unchanged;
only added metadata is removed from their view. No zero system, identity
first-MB or all-writable physical-map fallback is supplied.

The stricter device cache regression reaches an omitted normal-core opcode
Fetchb at23c4: complete CPU/RAM still match while page65538/physical307 is
missing from the final linked list. Restoring the actual CS opcode read makes
both targets match; an omitted-read mutant retains the original reaching
failure. Earlier byte-store claims for LTR's busy descriptor are disproven:
original CPU_LTR -> SaveSelector -> GDT SetDescriptor -> Descriptor::Save
uses two mem_writed calls (cpu.cpp:128), not MEM_BlockWrite/eight bytes.
A controlled relocation of the actual available descriptor into the captured
chained VGA aperture reaches the width defect. Both widths preserve all58
CPU/system words and full16MiB RAM, but eight byte callbacks produce different
VGA fastmem. The corrected owner uses two DWORD stores; the callback regression
binds the actual original Descriptor::Save source. This controlled API case is
not an independently captured changed whole-VM execution.

Selected candidate regressions passed seven CPU/system/interrupt tests and
six device tests on Native/WASM. First/repeated original-byte bootstrap also
matches all58 words/four clocks/nine intermediate and final full RAM/cache/VGA
boundaries. Independent extracted-source provider and VGA compositions were
rerun with current public headers:24 provider results/five causal mutants and
20 VGA results/four mutants pass. These compositions retain their declared
controlled-input scope, not full application acceptance. Evidence:
/tmp/wasm-fist-cpu-memory-adoption/{interrupt-unit.log,provider-replay/provider-proof.json,vga-replay/proof.json}.
Canonical current captures use tools/oracle/capture_memory_context.py;
temporary proofs retain exact generated probes, producer hashes and raw inputs.
Remaining task/V86/fault/other-device routes, LFB/MMIO/register transitions,
actual outer77e2/op6c state transport, first817 and final mixedPCM stay open.

Complete gate `run.VBHRax` under
`/tmp/wasm-fist-cpu-memory-adoption-gate`: `bash tools/check_flow.sh` with
that root's production Native/WASM binaries and sequential builds passed151
tests in290.404s, exact patches, six resource-start cases and178 existing flows
with no failures and durable terminal0. All1474 frozen source inputs remained
unchanged, and the separate419-original inventory matches. See the versioned
`tools/oracle/cpu_memory_production_case.json` for exact command, implementation,
archive/binary/log hashes and source reproduction. Canonical irq/device/config
captures were reverified after the full gate. Completed matrix/build/object
artifacts retired4862221095bytes; compact receipt and tested binaries remain.

Complete fresh SB-enabled30000-ms Native/WASM captures preserve all2100 parent
frame/end bytes,43 sound rows and47 unmasked service packets. Every original
layout/palette/time matches;28 pixel failures remain, first817/11704156us/
byte8754. Final mixed portPCM is absent and strict original comparison fails.
This dependency adoption does not establish actual application startup/IRQ
transport or any first817/finalPCM improvement.

Actual first DOS software entry and outer return adoption, parent `2fdaeee`:
`tools/oracle/capture_software_dos.py` captures380 complete startup fetches,
158 complete handler/caller observations and20 whole16MiB RAM/provider/cache/VGA
boundaries. Original INT21 atCS2b:5cdd passes type1 and explicit return5cdf;
the final protected handler instruction is66cb atCS8:1b41, invoking
CPU_RET(use32=1,bytes=0), not IRET. Shared software entry materializes the
reached lazy DWORD XOR and saves the explicit post-operand return. Shared
RETF restores callerCS2b/CPL3/SS23/ESP3d006 without touching flags. Both API
pairs match all58 CPU/system words, complete RAM and provider/cache/VGA on
release Native32/WASM. Hardware substitution saves5cdd instead of5cdf at
physical7f8c while all58 output words match; near-return substitution leaves
CS/CPL/ESP wrong while complete RAM matches. Four selected regressions pass
(19.243s), including missing RAM/VGA and coherently wrong return/clock
rejection. Reproduce source: `python3 -B tools/oracle/capture_software_dos.py
--repo . --output /tmp/wasm-fist-software-dos-source`. Complete original
39-frame/27518-PCM/end600 output remains equal to its unprobed baseline;
419 original assets remain unchanged. Evidence:
`/tmp/wasm-fist-software-dos-entry/public-original/proof.json` and
`/tmp/wasm-fist-software-dos-entry/software-unit.log`. The full existing matrix
passes155 tests (310.186s), exact patches, sequential Native/WASM builds, six
resource-start cases and178 flows with no failures and terminal0. All1480
frozen source inputs and419 originals remained unchanged at terminal. Consume
`tools/oracle/software_dos_production_case.json` for command, hashes, source
reference and fresh30-second diagnostics. Both production targets retain all
2100 prior frames/end,43 sound rows and47 unmasked service packets. Original
layout/palette/time match;28 pixel errors begin817/11704156us/byte8754 and
strict comparison rejects missing final mixedPCM. Observing158 handler fetches
and replaying these two public APIs does not execute the handler or transport
production startup, fix first817 or supply final mixedPCM. Recover its real paging/control,
DOS service, REP and MOVSS execution next; no fitted handler budget.

Original PIC binary32 rounding, parent4aa0cc3: the optimized Native32 x87
path retained excess precision in PIC_TickIndex, event-index multiplication
and subtraction before integer conversion. The reached DSP-reset budget at
CS2b:1358/cycle15778950 became597 instead of the original598. The shared clock
now rounds at the original float return/expression boundaries, using one
helper for queue service and next-slice deadlines. Existing expectations are
unchanged. All six source-backed SB/PIC tests run Native32 O0/O2 and WASM O2;
the parent fails the optimized reset case and the candidate passes all six.
The full existing matrix run.RZjRij passes155 tests in310.614s, exact
patches, sequential Native/WASM builds, six resource-start cases and178 flows
with no failures and terminal0. All1481 frozen source inputs and419 originals
remained unchanged. Consume `tools/oracle/pic_float_production_case.json` for
command, hashes, source provenance and complete30s production diagnostics.
Both targets retain all2100 prior frame/end bytes,43 sound rows and47 unmasked
service packets. Original layouts/palettes/times match;28 pixel failures begin
817/11704156us/byte8754 and strict comparison rejects missing final mixedPCM.
Full original video/PCM and actual startup/IRQ transport remain open.

Complete startup/both-DOS observation and shared narrow flags, parent2d67006:
`tools/oracle/capture_software_startup_dos.py` now reproduces the actual77e2,
CALL3322, first INT21/AH1a/handler/RETF, following caller and second
INT21/AH4e/FindFirst/handler/RETF through actual callerCS2b:5df6. Its versioned
`software_startup_dos_case.json` references the unchanged first software-DOS
fixture instead of duplicating its contract. Strict verification preserves
380+158+257+219 fetches,20 first-owner plus34 added full RAM/provider/cache/VGA
contexts, four complete host states and39frames/27518PCM/end600 against the
unprobed original. Seven additional negatives reject missing/short FindFirst
RAM, missing VGA, truncated following fetches, wrong directory allocation,
changed raw host-name tail and wrong explicit software return. Initial drive2,
CWD FISTDATA and occupied directory slots0/1 are observed before startup;
FindFirst allocates2 and advances nextFree to3. Real isolated file metadata
and all13 original host name-buffer bytes are matched inputs; the final byte
is uninitialized by localDrive::FindNext and is never normalized to zero.

One public CPU flag owner now handles the reached narrow OR/AND/XOR/SUB/TEST,
INC/DEC and SHL contracts. INC/DEC preserve raw CF and untouched upper lazy
storage; even word SHL writes only the byte count. Original flags.cpp and
instructions.h supply2814 boundary lazy states,3738 instruction calls and1014
whole-capture lazy states. Materialization preserves all37 CPU words over1382
original checkpoint/startup/DOS states. INCw is an extracted-source composition,
not a reached operation in this whole-DOSBox capture. The optimized release
parent reaches396 observations then rejects original SHLw atCS8:19ab;
both corrected release targets match all1014 original lazy-state outputs.

Fresh private continuous preparation now consumes the public flags and new
public source. One CPU/system/RAM/provider/cache/VGA input matches1014 fetches,
1034 complete CPU/PIC-time observations,52 complete memory contexts and four
host contexts on Native32/WASM O2 NDEBUG, without intermediate reseeding.
Lost upper INC storage and word-sized shift-count substitutions both reach
the complete path and retain all52 memory/four host states, but reject CPU
flags first at observations468/819 respectively. Earlier MOVSS/REP/PSP/LOOP/
DTA, parent PIC and missing initial directory-slot proofs remain historical
preparation with their recorded inputs; superseded raw captures are retired.
The interpreter, DOS provider and runtime startup/IRQ transport remain private.

The frozen1485-input gate run.UGEIUm passes159 tests in468.130s, exact patches,
sequential Native/WASM builds, six resource-start cases and all178 flows with
no failures and durable0;419 originals remain unchanged. Command, source,
binary/proof hashes, causal scope and cleanup receipts live in
`tools/oracle/software_startup_dos_production_case.json`. Fresh complete
30000-ms production streams retain all2100 prior frames/end,43 sound rows
and47 unmasked service packets. Original palettes/layouts/times match;
28 pixel errors still begin817/11704156us/byte8754 and strict parity rejects
missing final mixedPCM. This accepts source/flag ownership, not application
startup/IRQ execution, first817, complete audio/video or any broader WI.


### Shared actual-byte instruction execution

The shared `re_out/fist_exec.h` decodes the reached original instructions against
one CPU/system/physical-memory owner. I/O, budget and callback adapters carry
actual external owners; observer snapshots, source addresses, stopping and DOS
host-service composition remain in tests. No captured state initializes runtime
production. Unsupported instruction/control/fault paths fail explicitly.

The public continuous regression executes1014 actual fetches through both DOS
handlers/caller5df6 without intermediate reseeding. Both optimized Native32 and
WASM match1034 complete CPU/time observations,52 whole RAM/provider/cache/VGA
states and four host states. The actual shared clock owns MOVSS/REP fetch credits;
its existing DOS-floor credit uses the same state update. The legacy translated
SS entry still owns no decoded fetch. Losing MOVSS credit first differs at
observation446/CS02dd:143f: time+1,budget-1,all58 CPU/system words unchanged.

Verbatim original branch bodies/macros and DoString prove2112 operand-width/
sequential-IP compositions and336 REP budget/address-wrap/direction/overlap
compositions, including both REP prefixes. All19 observed words and whole256KiB
physical memory match both release targets. A code-segment-sized sequential EIP
substitution reaches10002->0002 with every other byte equal; a whole REP instead
of one budget chunk loses the original remaining count4 and restart2000.
Controlled budgets are not production PIC/IRQ evidence.

The frozen1494-input gate run.X6VJuk passes165 tests in383.821s and exact patches.
Two custom-output-directory setup failures are retained; frozen source remains
unchanged through the recorded resumption. Sequential Native/WASM builds, six
resource-start cases and all178 flows finish with no failures and durable0;
419 originals remain unchanged. Full commands, source/binary hashes, causal
scope and cleanup live in `tools/oracle/cpu_execute_production_case.json`.
Fresh complete30000-ms streams retain all2100 previous frame/end bytes,43 sound
rows and47 unmasked service packets. Original palettes/layouts/times match;
28 pixel errors still begin817/11704156us/byte8754 and strict parity rejects
missing final mixedPCM. This adopts the instruction owner into reproducible
regression, not real application startup/IRQ execution or complete output.


### Original normal-core exits and PIC frame ordering

The shared actual-byte producer now returns the original instruction-specific
PIC/trap/core-return checks. CLI/ordinary instructions do not acquire STI's
pending-IRQ check. Decoder selection remains machine metadata distinct from TF.
The PIC owns one raw pending mask; a blocked lower-priority request still causes
STI/POPF/IRET to leave the core. The shared early-return clock materializes flags
and requeues at the same CPU time, without the failed-loop fetch decrement.

The actual first reaching IRET at original PIC tick531 retains seven complete
58-word CPU/system and16MiB RAM/provider/cache/VGA/PIC observations through the
first IRQ0 handler fetch. Both optimized Native32 and WASM match continuously
from one observed input seed. The original binary32 calendar is input, with no
event reached in this bounded chain; fixture events fail if reached. The actual
IRQ request is cleared before CPU frame construction; the in-service mark is
applied after it. One PIC selector supplies dispatch and the existing vector-only
API. Full39-frame/27518-sample/end600 original output equals the unprobed run.

Verbatim original control bodies, flag operations and PIC match480 controlled
real-mode CLI/STI/POPF/IRET width/stack/IF/TF/DF/pending/priority/budget cases on
both release targets. These end at vector selection; guest trap execution and
protected paths remain unproved. The extra-fetch substitution preserves every
58-word/memory/context/PIC observation but first after-core time+1,budget-1 fails.
Marking service before frame construction preserves complete CPU/time/RAM and
final controller state but fails both original hardware-boundary PIC records.

The frozen1505-input unfiltered gate passes169 tests, exact patches, sequential
builds, six resource-start cases and all178 flows with durable0 and419 unchanged
originals. Fresh complete30s captures preserve all2100 previous frame/end bytes,
43 sound rows and47 unmasked packets. Original palettes/layouts/times agree;
28 pixel failures still begin817/11704156us/byte8754 and mixed final portPCM is
absent. Commands, source/binary hashes and complete scope are recorded in
`tools/oracle/core_exit_production_case.json`. This adopts the reaching core/PIC
frame owner in regression, not real application startup or full device time.


### First reached IRQ0 instruction prefix

The actual-byte producer now supports original ADDb lazy width/CF/FillFlags and
NOP retirement. Byte ADD updates only var1/var2/res low bytes, preserves their
upper bits, oldcf and prev_type, and leaves raw flags lazy. The following INCb
loads that original carry before replacing its tag. NOP changes only the
already charged fetch/EIP; it does not acquire a PIC/trap check.

One original initial IRET seed now runs continuously through core return, PIC
hardware frame and21 first-IRQ0-handler fetches, stopping before IN AL,DX.
Optimized Native32 and WASM match all8 complete58-word CPU/system and16MiB
RAM/provider/cache/VGA/PIC/calendar boundaries plus every fetch. Original
39frame/27518PCM/end600 output equals the unprobed run. The first raw memory
changes are actual handler stack/data writes; no guest DGROUP is rebased.

Verbatim original ADDB/INCB/get_CF/get_ZF/FillFlags agrees with131072 complete
byte-operand/raw-flag cases,524288 full lazy observations on both targets.
Version5f13624 headers match all preceding7 boundary/16 fetch CPU states and
then fail at ADD DL,6 /2082:3a82. Omitting only NOP matches7 boundaries/20
fetches and fails at2082:3a8f. The public capture/fixture/build/serialization
owners are reused; raw snapshots remain test inputs, never runtime startup.

The frozen1511-input unfiltered gate passes173 tests, exact patches, sequential
builds, six resource-start cases and all178 flows with durable0 and419 unchanged
originals. Complete30s production preserves all2100 parent frame/end bytes,
43 sound rows and47 unmasked packets. Original palettes/layouts/times agree;
28 pixel failures still begin817/11704156us/byte8754 and final mixedPCM is absent.
Commands, complete source/binary hashes and scope are recorded in
`tools/oracle/cpu_irq_prefix_production_case.json`. First I/O, whole IRQ handler,
queued devices/PIT/mixer and actual runtime CPU/state transport remain open.


### Bound original PIT producers and first actual timer writes

The shared timer owner implements all3 original timer.cpp counters, raw latch/
status/global-lock state, binary32 frequency/delay, binary32 PIC phase promoted
to double, counter2 gate, retained TIMER construction, callback-relative rearm,
mode2 deferred reload and original control cancellation/IRQ/budget behavior.
Binding supplies the CPU/PIC clock and actual counter/type speaker callbacks;
it never initializes a runtime from snapshots or installs a second event queue.
TIMER detachment retains the actual PIC calendar and removes owned callbacks.
Original failed-fetch debt and29/21 I/O costs retain complete budget/IO metadata.
Real-mode E6 OUT transports the actual first control/low/high write. Port61
uses its existing byte owner: toggle bits4/5 on read; on changed low bits, gate
first, speaker type second, byte assignment last. Speaker bodies remain open.

One original before-control seed reaches all6 complete58-word CPU/system,
16MiB RAM/provider/cache/VGA/PIC/calendar/all3 PIT/IO boundaries continuously
on Native32/WASM. All13 source observations preserve39frames/27518PCM/end600.
Controlled source agrees on736 full budget states and15 complete programs:
all256 controls,240 counter2 mode/access/BCD/count combinations, retained
constructor/detach, all256 port61 bytes from each of4 prior low-bit states and
all240 combinations through actual port61. Silent request observers preserve
all original states/reads. Complete endpoints1/60/110ms and markers agree.
Reaching substitutions distinguish decoder, calendar cancellation, latch
clearing, phase/frequency rounding, fetch debt, detached calendar and port61
gate defects. Original480 control-exit cases retain their expected results;
their setup now prepares PIC_RunQueue without an invented MOV-SS fetch.

The frozen unfiltered gate passes189 tests, exact patches, sequential builds,
six resource-start cases and all178 flows with419 unchanged originals. Complete
30s captures preserve every2100 parent frame/end byte,43 sound rows and47 full
packets. Original palettes/layouts/times agree;28 pixel failures still start
817/11704156us/byte8754 and final mixedPCM remains absent. Commands/hashes/scope
are in `tools/oracle/cpu_pit_production_case.json`. Actual application startup,
remaining IRQ instructions/VGA/device callbacks, protected/V86 I/O exceptions,
speaker bodies and final mixer remain open; this is not full sequence acceptance.


### Bound original VGA callbacks and continuous first-handler prefix

At parent `f6b1b5d`, the shared device now supplies original status reads,
64-bit host drawing arithmetic, LinearLine/ProcessSplit/DrawPart, vertical
interrupt and display-start latch through the existing CPU/PIC calendar.
AND AL,Ib and JCXZ use the existing ALU/branch owners. One matched original
seed reproduces all21 complete CPU/system/RAM/provider/cache/VGA/PIC/PIT/
calendar/drawing/service boundaries,2968 fetches and50 full line/end requests
on both release targets; the probed original retains39frames/27518PCM/end600.

Eight targeted tests pass. Controlled coverage includes all428044 CPU quanta
of the actual status period,3474 full drawing programs,131072 byteAND flag
programs,8192 AND instructions and5472 JCXZ cases. Five additional actual
PIC_RunQueue/DrawPart programs finish all four parts or explicitly retire the
owner. They expose and regress premature CPU budget assignment inside re-arm:
the original budget stays0; the prior port assigns29699 at the first request.
Replace/detach removes the three owned callbacks and preserves a reaching
unrelated event. Positive comparisons include complete state/RAM/calendars
and every request; six deliberate re-arm/removal faults are distinguished.

Reproduce with `python3 -B -m unittest discover -s tests -p test_cpu_vga.py -v`.
Fresh source and bound-PIC commands are `tools/oracle/capture_cpu_vga_callbacks.py`
and `tools/oracle/capture_vga_pic_rearm.py`, each with `--repo . --output /tmp/...`.
The frozen1539-input unfiltered gate passes197 tests, exact patches, sequential
Native/WASM builds, six resource-start cases and all178 flows, exit0, with419
unchanged originals. Complete30s captures retain all2100 parent frame/end bytes,
43 sound rows and47 unmasked packets. Original palettes/layouts/times agree;
28 pixel failures still start817/11704156us/byte8754 and final mixedPCM is absent.
Commands, original contracts, packet/trace hashes, controlled scope and complete
matrix/production results are in `tools/oracle/cpu_vga_production_case.json`.
Actual startup, remaining handler/callbacks, renderer/scaler bodies and final
mixedPCM remain open. These proofs do not establish complete sequence acceptance.


### Reached PUSH CS and complete segment-push contracts

At parent `cbaee6d`, the first unsupported instruction after the accepted VGA/
PIT/JCXZ prefix is PUSH CS at `2082:3abb` (opcode `0e`). The new decoder route
uses the existing operand-width stack owner and cached CS selector. Original
CASE_W/CASE_D bodies and CPU_Push16/32 define the contract; address-size prefixes
never select stack width. The source before/after pair changes only ESP/EIP,
one CPU quantum and the two bytes at cached SS plus the decremented SP. Raw
and lazy flags, other CPU/segment state and the rest of the full16MiB RAM survive.

The fresh versioned source command
`python3 -B tools/oracle/capture_cpu_vga_callbacks.py --repo . --output /tmp/wasm-fist-push-cs-source --through-push-cs`
retains25 full source boundaries and2972 fetches, including before/after PUSH.
Complete39-frame/27518-PCM/end600 output equals the unobserved original.
Both optimized targets continuously match23 complete states,2972 full fetches
and50 full line/end requests from one original initial seed. The previous
decoder reaches the actual missing PUSH CS after22 equal states/2971 fetches.
All3072 controlled programs and12 deliberate selector/operand/address/write/
flag/upperESP fault comparisons pass, including silent original observations.

Reproduce controlled coverage with
`python3 -B tools/oracle/capture_cpu_segment_push.py --repo . --output /tmp/wasm-fist-segment-push`;
run the reaching regression with
`python3 -B -m unittest discover -s tests -p test_cpu_segment_push.py -v`.
The controlled fixture explicitly supplies all cached segments and preserves
all19 CPU/budget/lazy words,12 segment words, three original-width64-bit stack
metadata words, all2MiB RAM and byte-fetch trace. Its original stack masks use
the actual cpu.cpp constructor values, including zero DWORD notmask. Default
controlled formats remain unchanged. The frozen full gate passes200 tests,
exact patches, sequential Native/WASM builds, six startup cases and all178 flows,
exit0, with419 unchanged originals. Complete30s production retains2100 parent
frame/end bytes,43 sound rows and47 unmasked packets; original palettes/layouts/
times agree. The same28 pixel failures begin817/11704156us/byte8754 and final
mixedPCM is absent. Commands, original provenance, complete states and coverage
are in `tools/oracle/cpu_segment_push_production_case.json`. Actual startup/whole
handler/device/renderer/final mixer acceptance remains open.


### Reached SHR and complete shift contracts

The next reaching instruction is WORD SHR DS:0450,1 at2082:3b38
(`d1 2e 50 04`). SHRB/W/D use the existing CPU/lazy-state owner: var1/res
assign at operand width, var2 only at byte width, zero count preserves all
state, and INC preserves the resulting carry. Original `get_OF` uses a
strict sign comparison while `FillFlags` includes the exact sign value;
these distinct original contracts are retained rather than normalized.
The D0/D1 decoder uses byte or decoded operand width and the existing
segment/address owner. Its code-fetch observer distinguishes displacement
fetches from data reads through CS; every RAM access remains observable.

The versioned flags command
`python3 -B tools/oracle/capture_cpu_shr_flags.py --repo . --output /tmp/wasm-fist-shr-flags`
compares148672 programs/594688 complete records, including full original-width
64-bit raw flags/type/prev/oldcf, six boolean flag queries and following INC.
Both optimized release targets match the original and distinguish10 causal
fault results. The encoded command
`python3 -B tools/oracle/capture_cpu_shr_instructions.py --repo . --output /tmp/wasm-fist-shr-instructions`
checks actual original D0/D1 CASE/GRP2/EA/ModRM owners, complete CPU/cache/stack/
2MiB RAM/code-fetch/ordered RAM read-write records and deliberate causal faults.
All7936 encoded programs and16 causal fault results pass on both release
targets. The original cached segment/address width contracts and every code
fetch plus every physical RAM read/write/address/width/value match completely.
The shared code-fetch observer is also used by the previous controlled tests;
their formats and full original comparisons remain intact.

Fresh source reproduction extends the existing unchanged-output owner:
`python3 -B tools/oracle/capture_cpu_vga_callbacks.py --repo . --output /tmp/wasm-fist-shr-source --through-shr-word`.
The fixture retains27 source boundaries/2976 fetches, including before/after
first SHR. Both targets match25 complete continuous boundaries and50 line/end
requests from one initial seed. Complete39frames/27518PCM/end600 remain equal
to the unobserved original. Parent07b34ff reaches the missing SHR after24
matching full states and2975 fetches.
Run `python3 -B -m unittest discover -s tests -p test_cpu_shr.py -v`.
The frozen unfiltered gate passes204 tests, exact patches, sequential
Native/WASM builds, six startup cases and all178 flows, terminal exit0, with419
unchanged originals. Fresh30s production retains2100 parent frame/end bytes,
43 sound rows and47 unmasked service packets. Original palettes/layouts/times
match; the same28 pixel failures begin817/11704156us/byte8754 and final mixedPCM
is absent. Complete commands, provenance, states and controlled/full coverage
are in `tools/oracle/cpu_shr_production_case.json`. Actual startup/whole handler/
device/renderer/final mixer and full original sequence acceptance remain open.

### Reached A0 and shared A0/A1 load contract

At parent `8f13c93`, the next missing instruction is `A0 3B 07` at2082:3b43:
MOV AL,DS:073b. Original CASE_B/W/D and `GetEADirect` fetch the offset at
address width, use the selected cached data-segment base and load at byte or
operand width. MOV preserves every lazy/raw flag field and the untouched
accumulator bits. A0 now shares the existing A1 execution and memory owner.

The versioned controlled command is
`python3 -B tools/oracle/capture_cpu_moffs.py --repo . --output /tmp/wasm-fist-moffs-instructions`.
It compares1120 original programs with complete CPU/cache/64-bit stack/2MiB
RAM/code-fetch/ordered RAM records. Both code/address/stack sizes, all segment
overrides, unaligned/16-bit edge offsets, dirty upper EAX, byte66 independence
and all65 incoming lazy tags are covered. Four causal substitutions cover
widened byte loads, operand-sized offsets, lost CS override and eager flags.

Fresh source capture extends the existing unchanged-output owner:
`python3 -B tools/oracle/capture_cpu_vga_callbacks.py --repo . --output /tmp/wasm-fist-moffs-source --through-moffs`.
It requires29 complete source boundaries/2980 fetches and50 line/end requests,
including the actual before/after A0 CPU, time and full16MiB RAM contract.
The continuous release replay requires27 full states; parent8f13c93 must fail
at A0 after26 equal states/2979 fetches. Run
`python3 -B -m unittest discover -s tests -p test_cpu_moffs.py -v`.
All1120 complete original programs and8 causal results pass both release
targets. Fresh source retains29 full boundaries/2980 fetches/50 line requests;
both targets match27 complete states continuously from one initial seed, and
parent8f13c93 fails A0 after26 equal states/2979 fetches. Original39frames/
27518PCM/end600 remain unchanged. The frozen unfiltered gate passes207 tests,
exact patches, sequential Native/WASM builds, six startup cases and all178
flows, exit0, with419 unchanged originals. Fresh30s production retains2100
parent frame/end bytes,43 sound rows and47 unmasked packets. Original layouts/
palettes/times agree; the same28 pixel failures start817/11704156us/byte8754,
and final mixedPCM is absent. Commands, provenance and complete coverage are
in `tools/oracle/cpu_moffs_production_case.json`. Actual startup/remaining
handler/devices/renderer/final mixer and full sequence acceptance remain open.


### Reached JNS and original release sign queries

Parent `aa4b1ddd5dbbe50299ff466316031fc23d43ac60` reaches JNS at2082:3b48
following the proved A0. The candidate adds opcode79 to the existing
conditional/IP owner and queries original lazy sign without materializing
flags. Original `get_SF` reads raw SF for UNKNOWN;49 result-type branches
use byte/word/dword sign; DIV/MUL and the release diagnostic fallback return
false. The13 rotate/NOTDONE tags and LASTFLAG sentinel are included. Original
`config.h` disables C_DEBUG and its logging overloads have no effect. The
shared result-width owner supplies this query; existing CF/OF/ZF/FillFlags
retain their previously proved tag scope.

Reproduce the complete original source and controlled contracts:

```
python3 -B tools/oracle/capture_cpu_vga_callbacks.py --repo . --output /tmp/wasm-fist-jns-source --through-jns
python3 -B tools/oracle/capture_cpu_jns.py --repo . --output /tmp/wasm-fist-jns-instructions
python3 -B -m unittest discover -s tests -p test_cpu_jns.py -v
```

Require31 source boundaries/2982 fetches/50 complete line/end requests and
unchanged original39frames/27518mixedPCM/end600. Both targets must match29
complete CPU/system/RAM/provider/cache/VGA/PIC/PIT/calendar/drawing/service
states continuously from one fresh IRET seed; parentA0 must fail at JNS after
28 matching states/2981 fetches. Controlled coverage requires2236720 complete
sign-query records,8448 actual CASE_W/D/TFLG_NS programs and8 causal results.
All2236720 complete sign-query records,8448 actual original JNS programs and
8 causal results pass both release targets. Fresh source retains31 full
boundaries/2982 fetches/50 line/end requests; both targets match29 complete
states continuously from one initial seed. Parentaa4b1dd fails JNS after28
equal states/2981 fetches. Original39frames/27518PCM/end600 remain unchanged.
The frozen unfiltered gate passes210 tests, exact patches, sequential Native/
WASM builds, six startup cases and all178 flows, exit0, with419 unchanged
originals. Fresh30s production retains2100 parent frame/end bytes,43 sound
rows and47 unmasked packets. Original layouts/palettes/times agree; the same
28 pixel failures start817/11704156us/byte8754 and final mixedPCM is absent.
Commands, provenance and complete coverage are in
`tools/oracle/cpu_jns_production_case.json`. Actual startup/remaining handler/
devices/renderer/final mixer and full sequence acceptance remain open.


### Reached OUTSB and complete hardware/renderer palette owners

Parent `e18a05b95838cada8ac9a017e6dc0581f1bf8dd7` reaches REP OUTSB at
4ec3:0bff after the proved JNS. The shared MOVS/OUTSB owner reserves REP
work before I/O, retains SI/DI until cleanup and uses original address/operand/
segment/direction/count/budget contracts. One complete hardware DAC and one
renderer-palette owner serve default and bound production ports3c6..3c9.
The renderer receives original ordered color notifications; forced browser/
dump palette injection is removed. Generated engine C and419 originals stay
pristine. Protected I/O permission faults,actual BIOS CPU costs,complete
caller/reset/frame-consumer transport and whole runtime acceptance remain open.

Reproduce the versioned contracts:

```
python3 -B tools/oracle/capture_cpu_vga_callbacks.py --repo . --output /tmp/wasm-fist-outsb-source --through-outsb
python3 -B tools/oracle/capture_cpu_outsb.py --repo . --output /tmp/wasm-fist-outsb-programs
python3 -B tools/oracle/capture_dac.py --repo . --output /tmp/wasm-fist-dac-programs
python3 -B tools/oracle/capture_render_palette.py --repo . --output /tmp/wasm-fist-render-palette-programs
python3 -B -m unittest discover -s tests -p test_cpu_outsb.py -v
python3 -B -m unittest discover -s tests -p test_dac_palette.py -v
python3 -B -m unittest discover -s tests -p test_dac_startup.py -v
```

All6048 complete OUTSB programs/12 causal results,671224 complete DAC
programs/8 causal results and259316 renderer-palette programs/8 causal results
match both release targets. One fresh source seed matches33 complete CPU/
system/RAM/provider/cache/VGA/PIC/PIT/calendar/drawing/service/DAC/renderer
states,2998 fetches,50 line/end requests and1544 before/after I/O states.
Parente18a05b fails OUTSB after32 equal states/2997 fetches/8 I/O states.
A deliberate duplicate production I/O charge differs at observation7 only in
time/budget while every complete palette state stays equal. The actual
`fist_text_clock_init` constructor matches all3635 original startup palette
bytes on both targets; its fixture contains no initialization recipe (0036).
Original39frames/27518PCM/end600 remain unchanged. The first host120-second
source attempt ended124 without the output footer and is excluded; the larger
GDB observation set now has a300-second host timeout with unchanged guest
endpoint/input contracts. The fresh complete source run supplies acceptance.

The frozen unfiltered gate passes217 tests,611 exact patches,sequential Native/
WASM builds,six startup cases and all178 flows,exit0,with419 unchanged originals.
Fresh public30s production retains2100 parent frame/end bytes,43 sound rows
and47 unmasked packets. Original palettes/layouts/times agree;the same28 pixel
failures start817/11704156us/byte8754 and final mixedPCM is absent.
Commands,full scoped provenance and cleanup are in
`tools/oracle/cpu_outsb_production_case.json`. This proves this bounded adoption,
not complete original frame/audio parity.

Whole-handler diagnosis retains44 source boundaries,3383 fetches and1546 I/O
states through actual interrupted-code resume,with unchanged39frame/27518PCM/
end600 output. The coupled candidate matches3045 initial fetches before missing
CMP3b at4ec3:2f3b;remaining whole-handler memory/device execution is unaccepted.
Private source/diagnostic reproduction is
`/tmp/wasm-fist-outsb-whole-handler-capture.py` and
`/tmp/wasm-fist-whole-handler-diagnostic.py`. Recover that reaching contract next.

### Reached CMP register/r/m adoption

Parent `b96686e92a5d4b4b3cfcf3bde774a6291c2d3a4c` reaches missing
`3b06ba15` at `4ec3:2f3b` after the complete OUTSB prefix. Actual
`CASE_W/D(0x3b)`, `RMGwEw/RMGdEd` and `CMPW/CMPD` subtract r/m from
the register operand without register/RAM stores or eager flags. The
existing ALU owner preserves dirty upper lazy words and raw flags/prev/oldcf.
Generated engine C and all419 originals remain pristine.

Reproduce the versioned contracts:

```
python3 -B tools/oracle/capture_cpu_vga_callbacks.py --repo . --output /tmp/wasm-fist-cmp-source --through-cmp
python3 -B tools/oracle/capture_cpu_cmp.py --repo . --output /tmp/wasm-fist-cmp-programs
python3 -B -m unittest discover -s tests -p test_cpu_cmp.py -v
```

All9760 complete original programs and14 controlled causal results match
both release targets. Every31 CPU/cache words,three64-bit stack metadata
words,all2MiB RAM and every ordered code/RAM access are compared. Coverage
includes both code/operand/address/stack sizes,all segment overrides,all
direct pairs/SIB forms,dirty lazy upper words and65 incoming lazy types.
One fresh public source seed matches35 complete CPU/system/RAM/provider/
cache/VGA/PIC/PIT/calendar/drawing/service/DAC/renderer states,3046 fetches,
50 drawing requests and1544 I/O states on both targets. Parentb96686e fails
the actual missing CMP after34 equal states/3045 fetches. Eager result flags
first change only lazy type at the actual next fetch;all complete RAM/device
states remain equal. Original39frames/27518mixedPCM/end600 remain unchanged.

The frozen1580-input unfiltered gate passes221 tests,611 exact patches,
sequential Native/WASM builds,six startup cases and all178 flows,exit0.
Fresh complete30s production retains all2100 parent frame/end bytes,
43 sound rows and47 unmasked packets. Original palettes/layouts/times
agree;the same28 pixel failures start817/11704156us/byte8754 and final
mixedPCM is absent. Commands and complete scoped provenance are in
`tools/oracle/cpu_cmp_production_case.json`. This proves the bounded
execution contract;actual application startup/whole IRQ transport,
protected faults,renderer/scaler timing and original frame/audio parity
remain open. The execution owner has no production consumer yet.

Historical full-handler diagnosis supplies its own original seed and3383
fetches through interrupted-code resume,with unchanged39frame/27518PCM/
end600 output. The coupled diagnostic after CMP matches3051 initial
fetches before missing JS78 at4ec3:2f50. This next instruction is excluded
from CMP acceptance;recover its original contract before runtime adoption.

### Reached JS conditional branch adoption

Parent `cc748762fef2cbd9fb92ee09ce67c593e6229684` reaches missing
`7816a0b5` at `4ec3:2f50`. Original CASE_W/D78/TFLG_S uses the existing
lazy sign getter and shared conditional branch retirement. It preserves
raw flags and lazy operand/type fields; no new sign owner is introduced.

Reproduce the versioned contracts:

```
python3 -B tools/oracle/capture_cpu_vga_callbacks.py --repo . --output /tmp/wasm-fist-js-source --through-js
python3 -B tools/oracle/capture_cpu_js.py --repo . --output /tmp/wasm-fist-js-programs
python3 -B -m unittest discover -s tests -p test_cpu_js.py -v
```

All8448 original encoded programs/8 causal results match both release
targets: every19 CPU/budget/lazy words,all256KiB RAM and all13 code-read
words are compared. The original release default remains false for
unhandled lazy tags. Shared conditional programs cover both polarities;
existing JNS still proves2236720 sign records/8448 programs/8 faults.
Every previous observer program remains byte-identical.

One fresh source seed matches37 complete CPU/system/time/RAM/provider/
cache/VGA/PIC/PIT/calendar/drawing/service/DAC/renderer states,3052 fetches,
50 drawing requests and1544 I/O states. The published CMP predecessor
fails JS after36 equal states/3051 fetches. Eager flags first differ only
in actual raw EFLAGS/lazy type;all complete RAM/device states remain equal.
All39 original frames/27518 mixedPCM samples/end600 remain unchanged.

The frozen1583-input unfiltered gate passes225 tests,611 exact patches,
sequential Native/WASM builds,six startup cases and all178 flows,exit0.
Fresh complete30s production preserves2100 parent frame/end bytes,43
sound rows and47 unmasked packets. Original palettes/layouts/times agree;
the same28 pixel failures start817/11704156us/byte8754 and final mixedPCM
remains absent. The complete scoped receipt is
`tools/oracle/cpu_js_production_case.json`. Actual startup/whole IRQ/mixer,
protected faults,renderer/scaler timing and complete original parity stay
open. The instruction owner still has no application production consumer.

Historical whole-handler diagnosis supplies its own complete original
seed and3383 fetches;after JS it matches3053 initial fetches before
CMPB3a at4ec3:2f55. This diagnosis is excluded from JS acceptance.


## Next

Transport the actual initial CPU/system/RAM/provider/cache/VGA/PIC and device
calendar from real application startup before replacing legacy outer77e2/op6c
transport. Preserve the continuous startup/DOS and first reaching IRET/core/PIC
frame proofs. Consume `cpu_irq_prefix_production_case.json` for the actual21-fetch first
IRQ0 prefix. Consume `cpu_vga_production_case.json` for the continuous21-state/
2968-fetch/50-line VGA/status/PIT prefix and actual service-relative re-arms.
Consume `cpu_segment_push_production_case.json` for the reached PUSH CS and
complete segment-push contract. Consume `cpu_shr_production_case.json` for
reached SHR word DS:0450,1 at2082:3b38 and complete shift contracts. Recover
following handler instructions after the JS contract in
`cpu_js_production_case.json` (next observed CMPB3a at4ec3:2f55),
panning/vertical setup and renderer
bodies, far-transfer/trap retirement and pending device-event
paths before runtime adoption. Keep the raw pending mask separate from eligibility and mark service
only after the CPU frame owner returns. Snapshots, stopping and serialization
belong in tests; captured states are never runtime initialization. Regress
complete CPU/RAM/device/output boundaries and both targets for each adoption.

Consume physical_provider_case.json together with paging_control_case.json.
Adopt the one CPU/system/physical owner only after actual callers, provider
execution and full required cross-target regression are proved. Preserve the
reached outer77e2->7809 byte/word/CALL/flag contracts; recover alternate branches
and the following CALL3322. Transport real system state instead of creating a
zero fallback. Connect actual VGA provider/register transitions to the same
owner before whole-address-space acceptance; other VGA/LFB/MMIO and fault
routes require reaching original evidence.

Consume paging_control_case.json for the real nonempty cache before CR3/PG
changes and retained physical slots. Complete the one mutable CPU/system/RAM
owner, then regress the actual first/repeated bootstrap and existing device
producers together. The device system-context preparation supplies actual MPL,
TSS and exception inputs; never construct a zero system record for legacy
37-word callers. Source first-startup cache includes158 RAM and3 ROM handler
entries; a16MiB byte buffer alone does not prove every physical page writable.
Recover physical RAM/ROM/device providers and first-MB/A20 state before whole
address-space acceptance. Private success remains a hypothesis until production callers,
complete CPU/RAM/output regression and the existing full matrix are proved.

Transport the complete CPU/system state through the actual startup callers,
descriptor-table construction and handler execution before connecting timer
dispatch. Consume pit_irq_frame_case.json for actual GDT/IDT/TSS loads and
IRQ/IRET operations, including cached descriptors, lazy flags, CPL, direction
and stack width. The reached tss.is386 is8, not a normalized1. Legacy scalar
callers and zero-argument ISR calls do not supply that context. Cold paging,
remaining handler instructions, task/V86/exception routes and alternate system
loads need their original paths and reaching evidence. Regress complete CPU/RAM
and timed full-output boundaries on both targets.

Consume `cpu_pit_production_case.json` for the shared original PIT producers,
complete budget/lifecycle/port61/endpoint proofs and first actual E6 writes.
Recover the next reached instruction and original VGA/PIC device callbacks,
then transport actual startup CPU/system/memory and IRQ/IRET state before
runtime adoption. Deliver requests through the existing PIC and CPU IF owner;
recover speaker bodies/final mixing without another clock or fitted delays.

Consume `sound_vector_init_case.json` before implementing the reached AX2503/2506
services or sound CPU/IF integration: getter leaves IF clear, actual setter kernel
STI enables it. Preserve original full EBX, WORD selector/DWORD offset and BYTE
stub activation. Consume `resident_stub_creation_case.json` and the shared
`resident_image.py` owner for prior stub creation, actual MZ relocations and
real16-bit address/loop widths; implement their actual instruction work with
the shared CPU/IRQ state rather than planting a fitted CALL grid. Alternate
resident setup branches, other vectors and errors need their own reaching proof.

1. Preserve the proved MZ/application phase, disk-read cap and reached VCPI packet/SS/REP contracts.
   Continue 0034's first remaining pixel difference at event 817 in the 30000-ms capture.
   Consume the proved shared PIC/reset timing contract. Recover
   0003's accepted644 actual133a producer and shared full CPU context:
   retain both normal-core flag exits and CLI/STI when connecting the
   actual startup caller. Consume `device_start_prefix_case.json` for its
   full1280/guest CALL/RET/TCB binding and post-reset ORb contract. Recover
   protected SB IRQ/mixer state/instruction
   work; preserve the proved 7120
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
