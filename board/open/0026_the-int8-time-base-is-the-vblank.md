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

## Next

1. Preserve the proved MZ/application phase, disk-read cap and reached VCPI packet/SS/REP contracts.
   Continue 0034's first remaining pixel difference at event 817 in the 30000-ms capture.
   Consume the proved shared PIC/reset timing contract. Recover
   0003's actual 133a producer, CLI/STI and protected SB IRQ/mixer state/instruction
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
