Type: feature
Title: Menu and mission audio match the original sample-for-sample

## Contract

One final stereo mixer owns WAV, browser and sequence PCM for PC speaker, OPL and digital effects.
Match original and both targets across intro/menu, mission, cockpit switch/loss and debrief,
at the same rate, devices, time and scenario boundary. Device streams are diagnostic inputs.

## Evidence

- Menu note content is owned by 0011. Sequencer re-entry through port I/O was fixed in
  `d7bd0aa`; missing-program-change/vector-base theories were disproven. Note order is not PCM parity.
- Source audit at `22dfff2`: OPL/SB independently open the same `FIST_AUDIO_WAV` with `wb`;
  browser `fist_web_post_audio()` drains only OPL. Equal WAVs can omit digital effects.
- First nonzero original sample at the 0036 boundary is stereo index 18,887 (-860/-860),
  source `SPKR`. Original `SetType(0/1)` queues -5000 even with speaker output disabled;
  its integrated ramp is audible. The port has PIT-2 state but no speaker PCM producer.
- In `oracle-speaker-711`, counter 512/mode 3 starts at 404.277133346 ms; the 405-ms
  callback first emits -860 at offset 12. Preserve DOSBox `ForwardPIT`, queued transitions
  and ramp integration from `src/hardware/pcspeaker.cpp`; do not fit a waveform/start offset.
  `SetType(0/1/2/3)` selects OFF/PIT_OFF/ON/PIT_ON; raw port-61 bits are not the `SPKR_MODES` enum.
- Original 25-ms prebuffer: `floor(44100×25/1000)+1 = 1103` initial samples. Q14 tick increment
  `floor((44100<<14)/1000) = 722534`; `1103+floor(403×722534/16384)+12 = 18887`.
  Preserve fill, remainder, interpolation and final clipping. Event-clock discrepancies belong to 0026.
- Digital mixer: op `0x64 → 786a → 22ab` assigns channels; `2630` advances cursor `0x15ef`
  by pitch `0x15cb`, releasing it when `cursor>>16` reaches length `0x15e3`.
  Old unreachable-mission investigations predate PIT/patch 610; they are stale.
- Reached by 0034's first remaining video difference (event 817): original Sound Blaster IRQ 7
  invokes protected handler 14e0 and its 2630 software mixer before KDV callback 168.
  The port defines `fist_sb_set_irq_cb` but never calls it. The full original 11686:11696 CPU
  trace proves 1024 mixer outputs without channel rollover: 12 setup + 44/sample + RET = 45069
  instructions, with a 609-instruction nested timer interrupt. The combined interval consumes
  46373 cycles. These are attribution counts, not a permitted fixed-delay replacement.
- `scratch/sequence-capture/pixel-817/{mixer-snapshot.gdb,mixer-registers.json,mixer-physical.bin,
  proof.json}` captures original state immediately before 2630: DS=33/base 10000000,
  IRQ 7/base 220, channel pitch/cursor/length/pointers and self-modified normalization operands
  (SHR 1/2/3, ADD 40/60/70 hex). Read all guest data through paging; physical pages are not
  contiguous flat-module offsets. The probe's complete 13000-ms frame and PCM streams match
  the unprobed original bytewise (908 frames / 574358 samples); this is source-state provenance,
  not port synthesis or final-mixer acceptance. The existing generated 2630 byte sums omit
  these actual SHR/ADD operations. Preserve their width/flag contract and rollover callbacks.

- Current base `bb25d36`, patch 622: the reached active-channel 2630 block now preserves
  self-modified byte SHR/ADD normalization, DS-relative feedback storage and byte lookup/output
  through the actual mutable 2716 operand. The existing extender operand resolver has one shared
  implementation; its DOS callers retain their previous behavior. Generated engine C stays pristine.
- `tools/oracle/mixer_2630_case.json` comes from paired paging-aware original snapshots before
  2630 and after its return, independently checked against the complete original CPU trace.
  The original mixed 1024-byte output SHA-256 is
  `af7fdff52ebb047412d65148e6b88a39cf75d1fe8319de9415c273ef5276f908` (1016 non-128 bytes).
  Three actual sample blocks match provisioned DSOUNDS.BIN bytewise; asset hashes/offsets are
  recorded without adding game samples to Git. The paired read-only probe retains all original
  13000-ms output: 908 frames, 574358 mixed PCM samples and the complete endpoint.
- `test_2630_active_channels_match_complete_original_buffer_and_state` compiles the actual
  ordered-patched producer and compares the entire isolated one-MiB module state, every original
  mixed byte and the untouched DMA half on both targets. The parent fails on both targets:
  native SIGSEGV at the unbased feedback address, WASM wrong state. Both now pass.
  `scratch/sequence-capture/mixer-2630/{red.log,green.log,make-case.py,check-proof.py,proof.json}`
  preserves provenance, the reaching failure and full production capture scope.
- `bash tools/check_flow.sh '^(intro|mainmenu)$'`: 62 tests, exact patches, sequential native/WASM
  builds and two selected flows pass / zero failures, exit 0; `scratch/verify/run.woiVQD/`.
  Fresh 10/30-second frame diagnostics retain the prior results (0034). All 418 provisioned
  original files remain unchanged. Scope is this active no-rollover block: rollover byte writes,
  callback register contracts, SB DMA/IRQ lifecycle, elapsed mixer work and final mixed PCM
  remain open. No aggregate measured delay was added.

- Current base `7eeb072`, patch 623: original MOV moffs8,AL writes at 25f0/25fa,
  2750/275a and 27b0/27ba establish byte-only widths for all six normalization operands.
  Correcting their shared symbol accessors protects each following opcode across all entries
  into the original rollover blocks. Generated engine C stays pristine.
- `mixer_2630_rollover_case.json` records a read-only paging-aware original invocation at tick
  11782, before 2630 through its final RET at 2729. The independent complete 11780:11790 trace
  proves 1024 writes and one third-channel rollover to the same sample, without a callback.
  The original selected DMA half's SHA-256 is
  `01983e5c6c20450e6a49bde2a7772f2648c0e5cd2a8a8872ffe98923ed7d327f` (1008 non-128 bytes).
  Whole normalization instructions are included in the fixture. The regression compares the
  entire isolated one-MiB state, every mixed byte in the selected half and the untouched other
  half. It fails on both parent targets due to opcode corruption and passes with patch 623.
  Both source captures retain all original 13000-ms frames/PCM/endpoints bytewise.
- `scratch/sequence-capture/mixer-rollover/{red.log,green.log,make-case.py,reaching-proof.json,
  check-proof.py,proof.json}` records this scope. `bash tools/check_flow.sh` (no filter): all 63
  tests, exact patches, sequential native/WASM builds and the entire existing matrix pass
  (178 flows / zero failures), exit 0; `scratch/verify/run.N3Lzlt/`. The complete replay patch
  and tested binary hashes are retained; all 418 provisioned original files remain unchanged.
  Fresh complete 10/30-second frame diagnostics retain the prior video results (0034).
  This validates byte-only storage; callback/IRQ/DMA/timing and complete original PCM stay open.
- Current base `105026b`, patch 624: the reached channel-two rollover invokes the actual
  original default callback 2294 (RET). Its seven non-stack GP registers are preserved and
  ESP advances four bytes; the rollover separately saves/restores EAX. All three channel
  vectors contain this original default callback. Calls now use its actual void(void) C ABI
  and retain live ECX/EDX values instead of assigning uninitialized decompiler extraouts.
  The callback is dispatched, not skipped or replaced with a stub; generated C stays pristine.
- `mixer_2630_callback_case.json` records the paired paging-aware invocation at tick 13257
  and the complete independent CPU trace: one channel-two silence rollover, one RET callback
  and 1024 stores. Both source captures retain all 1119 frames / 706658 PCM samples over
  16000 ms. The selected original mixed-buffer SHA-256 is
  `54a44a1cb1590fb87a9303b635ac18d63362bd3eb62b6518e4f5fe003caf52a4` (984 non-128 bytes).
  Samples match provisioned DSOUNDS.BIN and the silence sentinel matches the original image.
  Earlier non-protected-page-root probe failed and remains excluded.
- `test_2630_default_callback_preserves_original_registers_and_mixed_bytes` binds the actual
  translated 2294 body, counts callback invocations and compares full isolated one-MiB state,
  every original mixed byte and the untouched DMA half. The reaching parent regression fails
  on native (unequal PCM bytes) and WASM (function signature mismatch). The new producer and
  both previous complete mixer cases pass on both targets. Evidence under
  `scratch/sequence-capture/mixer-callback/` includes source snapshots/trace, production red/green,
  `check-proof.py`, `proof.json`, complete 10/30-second captures and canonical source hashes.
- A separate checkout and separate executables preserve the byte-fix's full-matrix run.
  `NATIVE=.../mixer-callback/native OUTJS=.../mixer-callback/wasm/fistrun.js
  bash tools/check_flow.sh '^(intro|mainmenu)$'`: 64 tests, exact patches, sequential native/WASM
  builds and two selected flows pass / zero failures, exit 0;
  `scratch/callback-worktree/scratch/verify/run.ZRagJp/`. All 734 relevant source/build inputs
  are identical in the canonical checkout, whose three reaching mixer regressions also pass.
  All 418 provisioned originals in both checkouts remain unchanged. New full frame diagnostics
  retain the previous results (0034); strict comparison still fails missing mixed port PCM.
  Custom register-mutating callbacks, other switch variants, actual IRQ/DMA setup and elapsed
  instruction/event work stay open. No guessed register value or aggregate delay was added.
- Read-only original device provenance: `scratch/sequence-capture/sb-lifecycle/{probe-final.gdb,
  device-events-final.jsonl,proof.json,final-original-30s/}` preserves every complete baseline
  30000-ms frame/PCM/endpoint byte. Commands are timeconstant A6, speaker enable, block size
  03ff and auto-init 1c: 11111 Hz, a 1024-byte DSP block and a separate 2048-byte DMA ring.
  The 319 IRQs alternate DMA halves; Q14 mixer increment is 4127. Tick intervals are 92/93 ms,
  not a fixed 92-ms period. The port incorrectly conflates DSP and DMA lengths. No post-transfer
  finish snapshot is claimed (the finish breakpoint was not reached). Restore actual mixer
  demand/DMA/PIC lifecycle; these observed intervals must not become a fitted timer.


- Current base `b3d4e72`, PCM8 device ownership/consumption correction: DSP 48/14/1c
  retain separate block total/remaining state and no longer overwrite the programmed DMA
  ring or its auto-init flag. Starting the captured PCM8 transfer consumes no bytes.
  `fist_sb_read_pcm8()` supplies actual mixer demand in DMA bytes; current address/count,
  controller-wide flip-flops, mask callbacks and terminal-count reload follow original
  `dma.cpp`. DSP block completion latches an IRQ independently of DMA terminal count;
  single-cycle completion is delivered even after playback stops, and only DSP ACK clears
  the latch. Pause/resume and exit-auto-init retain the original remaining block.
  The device diagnostic ring/WAV is not the final stereo PCM owner.
- `tools/oracle/sb_pcm8_demand.gdb` and `sb_pcm8_demand_case.json` record every original
  request over 1000 ms: 470 demands, first tick 536 / five bytes after the channel's
  25-sample silence fill, and IRQs at 628/720/812/905/997. All 67 complete frames,
  45158 final mixed samples and the endpoint match an independent unprobed original.
  The tracked probe reproduces every event and output byte in a separate isolated run.
  A concurrent 31-second host-timeout capture was incomplete and remains excluded;
  the complete tracked-probe replay uses a 90-second host limit, with the same 1000-ms
  emulated endpoint. Observed request times are fixture provenance, not an injected clock.
- Four reaching tests compile the actual original `DmaChannel::Read`, controller registers,
  `DSP_PrepareDMA_Old`, `DSP_DMA_CallBack` and `GenerateDMASound` producers, with a PCM8
  mixer-input recording endpoint. Native and WASM match every captured pre-demand DMA/DSP
  state and every synthetic DMA input byte. Additional cases cover shared flip-flops,
  physical page addressing, terminal status clearing, separate auto-init modes, mask/pause,
  IRQ acknowledgement and single-cycle/exit-auto completion. Other sample formats and
  scheduled PIC events fail the source test endpoint; final mixer fill/interpolation and
  PIC event-clock behavior are explicitly outside this device-input proof.
- `scratch/sequence-capture/sb-demand/{check-red.py,red.json,green.log,check-proof.py,
  proof.json,production-proof.json}` retains the parent failure on both targets: 1024 PCM
  samples emitted before any mixer demand, and missing DMA counter readback. All four new
  tests and the demand-driven standalone selftest pass. All 418 provisioned originals
  remain unchanged. `bash tools/check_flow.sh` at `scratch/verify/run.RDIm2R/` has passed
  68 tests, exact patches, sequential native/WASM builds and all 178 existing flows pass
  / zero failures, exit 0. The four reaching device tests were also rerun with scheduled
  PIC events required to fail the source endpoint. Tested binary/script hashes remain
  unchanged. This full existing-matrix gate does not establish full original PCM parity.
- Fresh complete 10/30-second production captures retain every previous port frame byte:
  698 original frames match at 10 seconds; all 2100 cross-target frames, original times,
  layouts and palettes match at 30 seconds, with the same 27 pixel-difference events,
  first 817. Strict comparison still fails missing mixed PCM. The production intro
  currently starts no digital DSP and has no SB IRQ callback registration: actual device
  initialization, mixer demand/IRQ dispatch and instruction/event ordering remain open,
  as do the legacy PCM16/SB16-command path and final shared mixed output. This change is
  device-input progress, not first-817 or full audio acceptance.

- Original startup/channel provenance at `1d050a8`: op 64 dispatches to 786a, op 68 to
  76fd (effects mode), and op 6c to 77e2 (DSOUNDS load/device initialization). The first
  scripted call retains AL=0a, AH=02, DL=03 and ECX=0 through e60b/e637 and e2c2; EBX
  alone is the task inbox. Patch 075 currently transports only the script's pitch-table
  byte offset, losing the live sound/channel/normalization/pitch register contract.
  The production gate has no op-64 handler, and 23ec's initial mixer call still resolves
  to an empty generated 2630 placeholder. These missing producers remain open.
- `scratch/sequence-capture/digital-init/{paired.gdb,paired-proof.py,paired-first-channel/}`
  proves the first actual 22ab assignment using same-process paging-aware before/after
  snapshots and an independent complete instruction trace. At tick 553, channel two
  selects module sample 679d6, length 29999 and default pitch 65535; all sample bytes
  match DSOUNDS.BIN at asset offset 79830. The entire 134240-byte asset is mapped at
  54200. Exactly 25 instructions precede RET 2376: nine module-byte changes plus four
  saved-ESI stack bytes, with no interrupt. Unallocated DS pages are neither read nor
  zero-filled. Separate-process snapshots have additional host-address differences
  and are not used to establish this write footprint.
- Both probed 600-ms runs retain all 39 frames, 27518 final PCM samples and the endpoint;
  the complete 30-second register trace also retains the unprobed original output.
  `init-cpu-proof.json` attributes 1024 initial mixer stores and 45120 instruction rows
  (one channel rollover, no callback), leaving the physical 2048-byte DMA ring at 128.
  These counts are source attribution, not an injected delay. The reached 22ab setter
  reads DS:(1e8b+DL); its C currently omits DS. A reaching port regression/fix is next,
  followed by the actual register transport, device initialization and IRQ producer.
  `python3 scratch/sequence-capture/digital-init/paired-proof.py` and `analyze.py`
  reproduce the provenance checks; no port implementation or parity acceptance is claimed.

- Current base `f54bc8c`, patch 625: direct channel setter 22ab now resolves its
  normalization byte through the shared extender operand owner. Original instructions
  233c/2354/2367 use DS:(1e8b+zero_extend(DL)); the previous C read the bare host address.
  Existing patch 623 retains byte-only operand stores. Generated engine C remains pristine.
- `tools/oracle/channel_22ab_case.json` records the actual first assignment from the
  same-process tick-553 pair above. The reaching test compiles the ordered-patched original
  setter, reconstructs its sample from provisioned DSOUNDS.BIN, checks the returned pointer,
  every isolated one-MiB module byte, complete normalization instructions and all untouched
  2048 DMA bytes on both targets. Parent native faults at 1e8e; WASM stores unequal state.
  The setter and all three previous complete mixer cases now pass. Shared fixture loading
  preserves the existing mixer expectations and source hashes.
- `scratch/sequence-capture/digital-init/{red.log,red-producer/,red-address.log,
  green-channel-mixers.log,check-channel-proof.py,channel-proof.json}` reproduces source
  provenance, reaching failure, fixture equality and production diagnostics. All 418
  provisioned original files remain unchanged. `bash tools/check_flow.sh '^(intro|mainmenu)$'`
  passes 69 tests, exact patches, sequential native/WASM builds and both selected flows
  / zero failures, exit 0; `scratch/verify/run.AGmXvE/`. This is a filtered gate, not a new
  full-matrix or original-parity acceptance.
- Fresh complete 10/30-second captures retain every previous port frame byte and endpoint.
  All 698 original frames match at 10 seconds. At 30 seconds all 2100 cross-target frames
  match, all original times/layouts/palettes match, and the same 27 pixel events differ,
  first 817. Strict comparison still fails absent final mixed port PCM. The production
  gate still does not dispatch op 64 or initialize the DSP. Direct setter storage is
  proved; queued/automatic allocation, register transport, initialization, IRQ/IF,
  elapsed instruction work and final shared mixer output remain open.

- Current base `aec8f0c`, patch 626: the actual intro loop now publishes its consumed
  AX/ECX/DL lanes before e2c2 enters the PM gate. Original e61e/e626 loads sound, channel
  and normalization from separate ES bytes; e62f loads full ECX from the DWORD table at
  DGROUP:(f772+byte[cur+5]). The script byte is a table offset. One shared shim retains
  these widths for both targets. The alternative translated entry shares this decoder
  and the original six-byte cursor advance. e684/e6a5 queues stops 82ff/81ff/80ff with
  zero ECX/DL. Generated engine C remains pristine. The full EBX inbox model, original
  register returns/flags and other control-loop paths are outside this bounded proof.
- `tools/oracle/sound_script_case.json` records all 43 consumed packets from the complete
  original 30000-ms trace: 40 script requests plus three stops. Every complete original
  frame/PCM/endpoint byte matches the unprobed original (2100 frames / 1324058 PCM samples).
  Read-only lane capture on the unchanged parent engine reaches all 43 gates on both
  targets but records zeros in every packet. Initial failed native compilation/capture
  is excluded; the corrected diagnostic builds and complete red runs prove this failure.
- The source-backed decoder regression replays all original packets into a recording
  poster endpoint; unexpected inbox values or request counts fail. Separate complete
  production captures verify all 43 packets at the real PM gate on both targets, with
  byte-identical native/WASM logs. Shared ordered patch preparation remains exact and
  now handles timestamped engine diff headers. The regression driver rejects partial
  input records. `FIST_SOUND_REGLOG` records CPU cycle, AX, ECX and DL; it is diagnostic
  metadata, not digital PCM or a sound-producer substitute.
- `scratch/sequence-capture/sound-script/{red.json,red-native-complete-30s/,red-wasm-30s/,
  green-unit.log,check-proof.py,proof.json}` retains source provenance and red/green
  evidence. `bash tools/check_flow.sh '^(intro|mainmenu)$'` passes 70 tests, exact patches,
  sequential native/WASM builds and both selected flows / zero failures, exit 0;
  `scratch/verify/run.XkKiXu/`. The sound regression is rerun after requiring partial
  test-input records to fail. All 418 original files remain unchanged. This filtered
  gate is not a new full existing-matrix or original-parity acceptance.
- Fresh complete 10/30-second frames retain every previous port byte and endpoint,
  including all 698 matching original frames at 10 seconds and the same 27 original
  pixel differences at 30 seconds, first 817. Strict sequence comparison still fails
  absent mixed PCM. First original 786a trace cycle is 16611902; port gate cycle is
  16579231. The original trace has fetched its instruction while the port records the
  service gate; this is clock attribution, not a permitted aggregate delay. Original
  startup/device/instruction work must supply the missing progression. The port still
  has no actual op-64 producer dispatch, initialized DSP or SB IRQ callback.
- Remaining caller provenance: be8b reads sound/channel and normalization from descriptor
  9fe1/9fe3, overriding DL with 3 when byte 6ce6 exceeds 1; befb/bf15/bf3c select secondary
  sound bank/channel two, zero ECX/DL; c00c uses (zero_extend(CX)<<9)+f000 and DL=2;
  c035 stops the descriptor's channel; c047 halves descriptor normalization and uses
  zero ECX. These asm contracts guide their future reaching regressions; their missing
  register transport is not fixed or accepted here.

- Current base `2b4bbe6`, patch 627: the reached 2810 DMA initializer now resolves
  its WORD port-table reads through the shared extender operand owner. Original
  281b/282a/2837/2854/2866/2897 use DS-relative data; the count table is 12ef,
  not a host function address derived from 12eb. The existing module-based page
  access at 12cc+3 was already correct. Byte OUT widths, word-channel address/count
  conversion and AX return remain intact; generated engine C stays pristine.
- `tools/oracle/dma_2810_case.json` records all nine original byte OUTs, AX=1 and
  immutable port tables from the complete 534:536-ms CPU trace and paging-aware
  original snapshot. The 2810-to-28a4 interval has 48 instructions before RET;
  its measured cycle interval is attribution only. The probe retains all original
  600-ms output bytes: 39 frames, 27518 mixed PCM samples and the endpoint.
- The reaching regression invokes the actual ordered-patched 2810 and shared
  SB/DMA owner. An independent endpoint compiles DOSBox's original DMA/DSP owners
  and receives the captured original OUTs. Both targets compare every OUT, AX,
  the entire untouched isolated module, both complete 1024-byte PCM8 input halves,
  DMA registers/reload, DSP state and IRQ acknowledgements. Parent native faults;
  parent WASM emits eight wrong port addresses. Both now pass, as do the previous
  four device regressions. This is device input/state evidence, not final mixed PCM.
- `scratch/sequence-capture/dma-init/{red-reaching.log,green-unit.log,
  check-proof.py,proof.json}` preserves provenance and the reaching failure/fix.
  `bash tools/check_flow.sh '^(intro|mainmenu)$'`, with separate outputs, passes
  71 tests, exact patches, sequential native/WASM builds and both selected flows,
  with an independently recorded exit 0; `scratch/verify/run.alvyDo/`.
  The earlier unfiltered log reports all 178 existing flows passing, but its outer
  process returns 143; `scratch/verify/run.LPAVCP/`. It is retained as diagnostic
  evidence and excluded from clean full-gate acceptance. No new full-matrix gate
  or original-parity acceptance is claimed.
- Fresh complete 30000-ms captures retain every previous port frame byte and all
  43 sound-register packets; native/WASM register logs remain byte-identical.
  All 2100 frames and endpoints are complete; strict comparison still rejects
  absent final mixed port PCM. All 418 provisioned originals remain unchanged.
  Production startup/op-64 dispatch, protected IRQ/IF, other device variants,
  instruction/event timing and the final shared mixer remain open.
- Additional read-only startup diagnosis under `digital-init/paired-initial-mixer/`
  captures the first actual 2630 before DSP start through RET in one process.
  It proves 45119 instructions before RET (45120 rows including RET), 1024 stores,
  one rollover, no callback and six changed module bytes. The physical ring remains
  entirely 128. Complete original 600-ms frame/PCM/endpoint bytes are unchanged.
  The existing translated producer matches this isolated initial state on both
  targets; 23ec's generated call still resolves to its empty 2630 placeholder.
- `digital-init/file-loader/{registers.txt,proof.json,capture/}` independently
  retains the same complete original 600-ms outputs. Original 6032 queries and
  loads the entire 134240-byte DSOUNDS.BIN; its read/return EAX is 00020c60 and
  ECX/EDX are zero after the resident kernel read. Reuse this file/memory ownership
  when restoring startup; verify the generated caller's DWORD result and CF,
  rather than adding a separate raw sample loader.

- Current base `89c6ba6`, patch 628: successful FILEMGR 6032 reads now retain the
  resident kernel's full EAX shadow. Original 608d stores DWORD EAX at 937;
  60a3/60a8 return that saved DWORD after closing the file. The generated caller
  combined AX with an uninitialized high word. The existing shared DOS file/read
  owner supplies the result; no separate sample loader is added, and generated
  engine C stays pristine.
- `tools/oracle/file_loader_6032_case.json` records the original HIGH.DTL return
  (2052 bytes), DSOUNDS.BIN query/full read (134240 bytes, EAX=00020c60), raw
  original instructions and image/asset/register-trace hashes. The read-only
  source probe retains all 39 frames, 27518 mixed PCM samples and the complete
  600-ms endpoint bytewise. Originals remain provisioned and ignored by Git.
- The regression compiles the complete ordered-patched extender with actual
  6032/5cc2/5d50 and the real DOS/VGA owners. Parent native returns garbage high
  words for both assets; WASM returns only 3168 for DSOUNDS. Both already copy
  the entire DSOUNDS asset correctly. Both targets now match full query/load
  counts, the saved DWORD, every file byte and all destination guards. The
  query leaves the entire destination untouched. Fixture endpoints outside
  file service abort if reached; no interrupt vector is installed and timing
  is not asserted by this isolated regression.
- `scratch/sequence-capture/file-loader/{red-reaching.log,green-unit.log,
  check-proof.py,proof.json}` preserves source provenance and reaching failure.
  `bash tools/check_flow.sh '^(intro|mainmenu)$'`, using separate outputs, passes
  73 tests, exact patches, sequential native/WASM builds and both selected flows,
  with separately recorded exit 0; `scratch/verify/run.ThgEmA/`. This is a filtered
  gate, not a new complete existing-matrix or original-parity acceptance.
- Fresh complete 30000-ms captures retain every prior port frame byte and all
  43 sound-register packets: 2100 frames and the endpoint on each target, with
  byte-identical native/WASM register logs. Strict original comparison still
  fails absent final mixed port PCM. All 418 original files remain unchanged.
  Error CF/other file-service register contracts, actual sound startup and
  service dispatch, protected IRQ/IF, device variants, instruction/event timing
  and the final shared mixer remain open.

- Current base `3d548a3`: the handwritten SB owner now retains the mixer index,
  exposes register 82's separate PCM8/PCM16 completion bits and acknowledges only
  the selected width. DSP read status preserves the complete original 7f/ff byte;
  reading status clears the PCM8 completion without consuming available data.
  Original 14e0's 152a/152c select mixer 82; 152f/1530 choose the acknowledgement
  port from bit 1. The old default ff would select the wrong acknowledgement width,
  and either port incorrectly cleared the same completion flag.
- `tools/oracle/sb_irq_ack_case.json` records the complete original handler bytes,
  image/device-source hashes and both reached SB reads from the independent
  11686:11696-ms trace: 152f reads 225=01; 1539 reads 22e=7f. Rechecking the
  historical read-only source capture retains all 908 frames, 574358 mixed PCM
  samples and the complete 13000-ms endpoint bytewise.
- The oracle endpoint now invokes DOSBox's actual `read_sb`/`CTMIXER_Read` and the
  real version-byte producer. Its prior synthesized zero acknowledgement only
  established DMA input/state, not complete hardware status bytes. Existing test
  expectations change from 00 to 7f because both original code and the actual
  reached trace prove that value. Unreached mixer/audio/MIDI endpoints abort;
  no event timing or protected PIC delivery is supplied by this device fixture.
- Two new regressions fail on both parent targets and pass on both current
  targets. Three complete 1024-byte demands preserve the original DMA state,
  status reads and IRQ counts 1/1/2: the unrelated 16-bit acknowledgement leaves
  PCM8 pending, PCM8 acknowledgement clears it, and the next completion is raised.
  The second regression retains every available version byte (4/5) while the
  IRQ acknowledgement changes status from ff to 7f only after data is consumed.
  All seven SB/DMA regressions pass. Initial oracle link/unconfigured-base
  scaffold failures are retained and excluded from reaching proof.
- `scratch/sequence-capture/sb-irq-status/{red-final-reaching.log,green-unit.log,
  check-proof.py,proof.json}` retains this scope. `bash tools/check_flow.sh
  '^(intro|mainmenu)$'` passes 75 tests, exact patches, sequential native/WASM
  builds and both selected flows, with separately recorded exit 0;
  `scratch/verify/run.S3PCy0/`. Fresh complete 30000-ms captures retain all 2100
  prior port frames and 43 sound-register packets on each target. All 418 original
  files remain unchanged. This filtered gate is not a complete-matrix or original
  output acceptance: strict comparison still rejects absent final mixed PCM.
  Actual startup/service dispatch, protected IRQ/IF, reset and event timing,
  other mixer registers/device variants, the legacy PCM16 producer and final
  shared mixer remain open. No measured instruction interval is inserted as a delay.

- Current base `a88d3e2`: 0026 owns the SB/DMA I/O timing correction and its
  full original-backed 68-instruction reset-high regression. Both targets retain
  all 20 original 226=ff reads and exact active CPU budgets at the reached phase
  and four thin-budget/tick boundaries; all ten parent phases fail and now pass.
  Consume `scratch/sequence-capture/sb-io-clock/proof.json`; do not add a device
  clock or charge normal fetch twice. The 77-test/five-selected-flow gate exits
  zero (`scratch/verify/run.rjEjwk/`), retaining complete 2100-frame/43-packet
  production captures and every original asset byte. Original output acceptance
  remains open, including absent final mixed PCM.
- Revalidated original 133a executes 217 instructions before RET and returns
  full EAX=ffffffff on ready AA, not a CF success result. Its reset-low phase
  reads 22e=7f 33 times, then ff and 22a=aa. Original DSP reset clears state on
  high bit 0 and schedules `DSP_FinishReset` at 0.020 ms after the low write;
  PIC may requeue the active CPU slice before that callback. Evidence:
  `scratch/sequence-capture/sb-reset-probe/{check-source.py,proof.json}`; the
  complete 600-ms original outputs are unchanged. Current port reset still
  queues AA immediately, and the actual producer body is missing. Recover
  shared event ordering/cancellation and instruction polling before integrating
  startup. The observed iteration counts are diagnostic, never fixed delays.

- Current base `b70ebd4`: the handwritten DSP owner now resets
  on rising bit zero, cancels an outstanding readiness callback, enters WAIT
  on the falling edge and queues AA from the shared clock callback. Empty data
  reads retain the original last-byte latch; reset/WAIT write status is ff and
  NORMAL follows the original 7f/ff busy counter. One shared event owner and its
  50 target-phase proof belong to 0026; consume `sb-reset-event/check-source.py`
  and `check-proof.py`, not another sound clock or a fixed polling count.
  The actual source PIC/DSP functions match all 218 reached reset rows and reads.
  The old device owner fails both complete reset phases and all six reset-variant
  phases. Source capture scaffolding is explicitly excluded.
- `bash tools/check_flow.sh` passes 81 tests, exact patches, both sequential
  builds and all 178 existing flows, with separately recorded exit 0;
  `scratch/verify/run.WZhm7Z/`. All three existing audio flows additionally pass
  on the same frozen binaries with recorded exit 0. Consume the strict bounded
  proof in `scratch/sequence-capture/sb-reset-event/proof.json`. Explicit matched
  SB-enabled full 30000-ms captures retain all 2100 frame bytes/endpoints and
  43 sound-register packets. The actual 133a startup routine is still missing,
  as are protected CLI/STI/IRQ/vector services and remaining initialization.
  Source reset scheduling is a dependency proof, not complete sound startup,
  final mixed PCM or original video acceptance.

- Current base `6ed1aa0`: consume 0026's patch-629 132f instruction/I/O contract.
  Fourteen complete original ready/controlled-busy calls recover the leaf's
  fetches, full EAX result and actual lazy flags; 44 reaching target phases fail
  on the parent and pass after the correction. `sb-writer-132f/check-proof.py`
  verifies all 83 tests, exact patches, both builds and five selected flows
  (`scratch/verify/run.Bir92a/`, exit 0). Complete matched 30000-ms captures retain
  all 2100 prior frames and 43 register packets. Final port PCM is still absent
  and all 27 original frame differences remain, starting at event 817. The leaf
  is a dependency proof; actual startup, protected vectors, caller flags and
  completion IRQ/mixer integration remain open. Reuse the shared clock owner.

- Consume 0026's original 14e0 PIC prefix in `tools/oracle/pic_irq_case.json`
  and `scratch/sequence-capture/pic-controller/source-proof.json`: actual
  immediate-port IN20 reads ISR=80 before the PCM8 acknowledgement and EOI.
  All six reached I/O bytes and budgets are retained, as are the complete
  13000-ms original frame/PCM/end bytes. Restore actual request/service state;
  do not replace that byte with a fixed 80 or bypass the protected IRQ path.

- Consume 0026's `sb_irq_frame_case.json` original route: the reached IRQ7
  interrupts real-mode BIOS, enters the resident kernel, and uses an outer
  operand-32 IRET to invoke 14e0/CPL3 with a 16-bit stack. Handler IRET returns
  through BF15 and a call gate before the resident real-mode BIOS return.
  A direct protected-ISR callback omits this reached frame/segment/flag work.
  All 15 boundary snapshots/six IRETs and complete original 13-second outputs
  are revalidated; target CPU ownership, actual startup/mixer dispatch and
  other IRQ contexts remain open. Reuse 0026's time/interrupt owner.

- Consume 0026's shared PIC controller proof in
  `scratch/sequence-capture/pic-controller/proof-final.json`: real PCM8
  completion asserts IRQ7, wrong-width acknowledgement preserves its request,
  matching acknowledgement/reset clear it, and actual PIC masks/service/EOI
  retain original register bytes. The 91-test/unfiltered 178-flow gate exits
  zero (`run.YUVdxO/`); native/WASM complete 30000-ms frames/end and 43 packet
  logs match each other. Original first pixel failure remains 817; corrected
  command-port I/O adds differing frame1555. Final mixed port PCM is absent.
  CPU privilege/IF/frame/IRET, actual ISR/startup/mixer work and device variants
  remain open; a returned eligible vector is not protected delivery.
- A separate reaching short-transfer regression, `pic-controller/`
  `reproduce-short-event.py` and `current-short-event-red.log`, uses actual
  original four-byte auto-init PCM8 at 11111 Hz, then 12000 normal fetches.
  Original `CheckDMAEnd` schedules END_DMA_Event because left4<min33 and
  produces IRR80/mixer82=01; both current targets still read 00/00. The sole
  method fails both targets without setup failures. This producer event remains
  missing, including its original cancellation/remaining-DMA behavior. Do not
  hide it by demanding bytes manually or substituting a periodic IRQ delay.

- Current base `428b416`, patch 630: actual mode-1 setup 23ec now calls the
  existing translated 2630 mixer instead of its empty generated placeholder;
  the unused placeholder is removed by the patch. Original 23ee/2496 access
  mode 2293 as a BYTE. Its former DWORD accessor read the neighboring RET
  opcode as status and overwrote following code bytes when selecting a mode.
- The same reached setup exposes six more byte-width contracts at 1613..1618:
  original 2507/250e/2515/251c/2523/252a store separate BYTE normalization
  parameters. DWORD stores at 1617/1618 erased the original mixer counter's
  low bytes after synthesis. Shared BYTE accessors preserve counter 00010400,
  original queued stores 2394/23a3 and the rollover reads. Other queued-channel
  behavior, register/IF transport and variants remain unproved.
- `tools/oracle/initial_mixer_call.gdb` and `initial_mixer_call_case.json`
  bracket the actual first 23ec/AX=4e01 through entry into 138d before CLI
  changes IF, with same-process paging-aware snapshots. The complete independent
  534:536-ms trace matches the earlier unprobed source bytewise: 45185 prefix
  instructions, 1024 mixer stores, one rollover and no callback. All 39 frames,
  27518 final mixed samples and the complete 600-ms endpoint retain their bytes.
  These counts are attribution, never an injected delay.
- The reaching regression compiles the complete ordered-patched module and
  stops at its actual dispatch to 138d; every other indirect target fails.
  Parent native/WASM dispatch the wrong previous handler 2294. Correcting
  only mode width reaches 138d but omits six original mixer-state bytes;
  correcting mode/call still erases counter byte 161a. All three reaching
  stages fail both targets. Current native/WASM match the entire isolated
  one-MiB module, all 18 original changed bytes and both complete DMA halves,
  including unchanged adjacent code. No device initializer is replaced or
  accepted by that test boundary, and no final PCM or timing parity is claimed.
- `python3 scratch/sequence-capture/initial-mixer-call/check-proof.py` verifies
  the source pair, all reaching stages, 649 frozen source/archive hashes,
  unchanged 418 provisioned assets, binaries and complete production captures.
  `NATIVE=.../initial-mixer-call/native OUTJS=.../initial-mixer-call/wasm/fistrun.js
  bash tools/check_flow.sh '^(intro|mainmenu)$'`: 92 tests, exact patches,
  sequential native/WASM builds and both selected flows pass / zero failures,
  outer exit 0; `scratch/verify/run.2p0ZmT/`. This filtered gate is not a new
  full-matrix or original-output acceptance.
- Explicit SB-enabled complete 30000-ms captures retain all 2100 prior frame
  records/endpoints and all 43 register packets bytewise on both targets.
  Original 28 pixel failures, first 817, and absent final mixed port PCM remain.
  Actual startup/service dispatch, 133a/138d, CPU privilege/IF/IRQ/frame/IRET,
  setup/mixer instruction retirement and the final shared mixer remain open.
  The initial setup producer is restored; it is not yet wired into production
  sound startup. Incomplete host-timeout source scaffolding is excluded.

- Current base `95a7d1d`, patch 631: missing startup leaf 1280 is restored and
  registered in the actual sorted extender function map. Original 77e9 calls
  it to load full EBX from current TCB owner 0c93, write WORD TCB+490 to port
  WORD12cc, and zero-extend WORD+492/+494 to IRQ/DMA DWORD12c4/12c8. Intermediate
  MOV AX preserves EAX's high word; final full EAX is the DMA word. The explicit
  EAX/EBX packet supplies those translated lanes without another CPU/flag owner.
  Every original fetch uses 0026's shared clock at its actual data boundary.
  The existing shared extender operand resolver owns the TCB host binding.
- `tools/oracle/device_config_1280.gdb` and both `device_config_1280*_case.json`
  fixtures retain same-process paging-aware before/after snapshots and complete
  525:526-ms traces. The actual TCB operand is f0010000: adding DS wraps to
  guest linear10000; physical addressing still requires paging. The unmodified
  source executes eight instructions and returns EAX1/EBXf0010000; all other
  original GP/segment/lazy-flag state is preserved except ESP+4 on guest RET.
  The default image already holds220/7/1, so stores execute without changing
  module bytes. The complete trace and all 39 frames/27518PCM/end600ms match the
  preceding unprobed source bytewise. No guessed elapsed interval is inserted.
- A controlled original run changes only incoming EAX/EBX, the three consumed
  TCB words and two adjacent table bytes during this same eight-instruction
  interval. MOV AX retains high89ab; MOVZX yields0000ffa5/0000f123; six module
  bytes change and adjacent5ac3 remains untouched. Original state is restored
  before caller77ee, verified against the saved full16MiB before image. Every
  source trace record outside this controlled interval and all complete original
  600-ms frame/PCM/end bytes remain equal. These are width probes, not supported
  hardware/device-variant acceptance. Code and provisioned assets remain intact.
- The regression compiles the complete ordered-patched module and dispatches
  through its actual function map. Parent has no1280 entry and fails all16
  target phases across three methods. Both targets now match every original
  live EAX/EBX pair, instruction time/budget, the entire isolated1MiB module and
  complete guarded TCB, including upper-word preservation, zero-extension and
  neighboring byte protection at thin-budget/tick boundaries. The return clock
  is matched at the RET fetch; the original caller77ee snapshot includes one
  additional fetch, so those boundaries are kept distinct. Guest return-stack
  and CPU flag/context transport are separate open contracts, not accepted by
  the explicit C return interface.
- `python3 scratch/sequence-capture/device-config/check-proof.py` verifies both
  source profiles, parent16red/current16green,655 frozen source/archive hashes,
  actual binaries, unchanged418assets and complete production captures.
  `NATIVE=.../device-config/native OUTJS=.../device-config/wasm/fistrun.js
  bash tools/check_flow.sh '^(intro|mainmenu)$'`:95 tests, exact patches,
  sequential native/WASM builds and both selected flows pass / zero failures,
  outer exit0; `scratch/verify/run.UvWOM4/`. This is a filtered gate, not a new
  full-matrix or original-output acceptance. Compile/link scaffolding and an
  earlier unmatched caller-fetch boundary assertion are explicitly excluded.
- Explicit SB-enabled complete30000-ms captures retain all2100 prior frame/end
  bytes and43register packets on both targets. All original times/layouts/
  palettes remain equal, with the same28pixel failures, first817, and absent
  final mixed port PCM. Actual77e2 startup,133a/138d, CPU privilege/IF/vector/
  IRQ/frame/IRET, complete mixer work and final shared output remain open.
  The configuration producer is available but production sound startup is not
  wired by this step. Generated engine C remains pristine.

- Current base `21a422f`, verified short PCM8 end-event step: the shared
  DSP owner now schedules original `END_DMA_Event` when nonzero remainder is
  strictly below the transfer's latched three-ms minimum. Its callback uses
  the existing DMA/sample producer, retains auto-init remainder/IRQ coalescing,
  and does not schedule a periodic replacement. Completion, new transfer,
  pause and reset remove outstanding end events; unmask/resume schedule from
  the source's DMA transition. Command 41 leaves the active DMA rate latched.
- `scratch/sequence-capture/sb-short-event/phases/` retains 16 original phases
  and 32 native/WASM phases: every normal fetch clock/budget, DSP remainder,
  diagnostic sample count, complete sample bytes and PIC request/ACK match.
  Sampling state on each of 2000 fetches around the deadline proves the first
  emitted sample boundary. The parent has 30 reaching failures; the strict
  minimum-equality case already passes. All 13 PIC tests and seven existing
  DMA tests pass. The event probe executes the actual original command
  dispatcher; its mixer endpoint records complete unsigned-mono PCM8 input,
  not the final stereo mix or guest interrupt frame.
- The older demand fixture's abort-only clock endpoints were replaced by
  the existing shared queue/clock fixture; its untimed port recording and
  original behavioral expectations are preserved. Missing-assert, command-40
  expectation, WASM WAV-path, recording-count and link scaffolding are excluded.
  Original command 40 restarts active auto-init DMA; that separate uncovered
  command contract remains open, rather than being mistaken for command 41.
- All 100 tests, exact patch checks, sequential production builds and all
  178 existing flows pass with zero failures and terminal exit0 in
  `scratch/verify/run.bpay5o/`. The frozen 886-source archive, full phase
  outputs, binaries, gate and captures are independently rechecked by
  `scratch/sequence-capture/sb-short-event/{check-proof.py,proof.json}`.
  Complete matched SB-enabled 30000-ms native/WASM captures retain all 2100
  preceding frame/end bytes and 43 packets. All original times/layouts/palettes
  remain equal; the same 28 pixel failures start at 817. Final mixed port PCM
  is absent. This fixes a required device event, not the first-817 producer,
  original-output acceptance or production startup/CPU/IRQ integration.

- Startup producer provenance additionally captured at real engine `e2fc` and
  entry `e339`: 16 original fetches copy DS WORD port/IRQ/DMA through GS:SI to
  the TCB, store full EBX into inbox3f2 and select WORD service6c. The full
  16-MiB memory delta, partial AX/BX/SI, FS/GS and CALL stack write are exact;
  all raw/lazy flags remain unchanged. Consume
  `scratch/sequence-capture/engine-device-poster/{check-source.py,source-proof.json}`.
  Its whole525:526 trace and all39 frames/27518 mixed samples/end600ms match
  the previous original capture bytewise. An early probe matched an earlier
  e339 before the poster and is explicitly excluded.
- A native production diagnostic at actual op6c, using the frozen current
  binary and isolated game copy, confirms configuration220/7/1 is already
  posted correctly. Its inbox is 5806e816 instead of original18f. Existing
  cae6 passes CONCAT22(param2.high,e816) after 153c; original 153c leaves the
  final resource descriptor's byte-product in BX. This is reaching native
  register-transport evidence, not a synchronized video/audio comparison or
  a new two-target fix. Recover the dynamic producer result and upper register
  contract; do not inject18f as a constant. The setup and protected CPU/IRQ
  work still remain ahead of first817 acceptance.

- Fresh base `965beb8` source recovery expands that dependency before any
  port fix: `engine-resource-registers/{check-source.py,source-proof.json}`
  captures actual153c entry, returned resource-open state, descriptor-loop
  entry and callerCB13. It opens the complete4523-byte `MSPRITE0.BIN` and
  registers40 entries. The whole interval has1410 actual fetches/16043 cycles
  (14633 extra original callback cycles); the descriptor loop has854 fetches.
  The descriptor loop's complete16-MiB memory delta, GP registers and segments
  match the decoded writes, including count40, finalBX018f/SI11ab/DI0140 and two-byte stack work.
  A controlled original caller sets only incoming EBX's upper word to89ab:
  every interval register/segment/fetch matches with that upper word retained,
  giving89ab018f at return. The restored return and trace outside the interval
  match. Ten unrelated low-DOS bytes differ between processes already at input;
  all complete within-run memory deltas match, with those differences retained.
- The missing startup resource precedes that BX transport: at original tick329,
  CRT223c's far applier receivesBX0/SI0174, resolves SS:[74] to table segment446b,
  and installs16 original far-vector pairs in140 actual fetches, including
  vector388=2082:306c. `engine-resource-relocation/paired/` retains the whole
  register/segment/flag/memory pair. BX0 is not an inert return: old091/098/133
  reasoning is disproved by the executed original and f7c3/f842 retry path.
  All three normal/controlled/CRT source captures retain39 complete frames,
  27518 mixed samples and endpoint600ms bytewise against the existing original.
- Current frozen native op6c evidence has vector388=0 and resource descriptor
  at actual portDGROUP:E816 (`g_mem+2a816`) with segment0. An isolated scratch
  prototype restores only the early0174 application through the existing loader
  helper; it reaches an earlier `222f -> 26fc` SIGSEGV before op6c. The backtrace
  proves the untyped caller supplies stale CX/DX/BX/BP, and the failed-variant
  path reads DI as a DWORD spanning BP. This is a diagnostic failure, not a
  verified production fix; generated C and production binaries remain unchanged.
  Recover that original live call and register widths before moving installation
  or accepting a dynamic153c return. Never pad unknown inputs with zero.

- Base `1472e73`, patch 632 fixes the reached resource-open width defect.
  The actual original boot `222f -> 26fc` opens `BACKLAND.BIN`, not a guessed
  font resource. All three AH43 probes miss with DI 0/1/2 and inherited
  BP `1718`; the complete 115 boundary records cover 114 fetches, and the
  normal return preserves BP and restores the whole 64-byte filename.
  Original GP/segments/raw and lazy flags, all 18 changed bytes in the
  complete 16-MiB memory pair, 39 frames, 27518 mixed samples and endpoint
  600 ms are retained in `engine-font-loader/{check-source.py,source-proof.json}`.
- The parent reads DWORD DI spanning the adjacent BP bridge lane after the
  first miss, causing two reaching target failures. Patch 632 uses WORD
  DI/DX/SI accesses and preserves the existing ES bridge lane instead of an
  uninitialized C local. The real DOS attribute dispatcher now returns all
  three source-backed probe rows on Native and WASM. Both complete 16-MiB
  outputs match the specified bridge/data writes without masks; original
  CPU return flags and the legacy DOS bridge's last INT result are distinct
  scopes. Successful loader variants, live caller BP/ES, early installation,
  stack/flags and instruction integration remain open. The explicit void
  test bridge declaration repairs an excluded fixture ABI error; it is not
  a production WASM fix.
- The unfiltered canonical gate `scratch/verify/run.dkymjy/` completed
  with a durable exit0: all 101 tests, exact patch checks, both sequential
  builds and all 178 existing flows pass. `engine-font-loader/check-proof.py`
  rechecks every one of the 890 frozen source/input files and whole Native,
  JS and WASM binary identity between the earlier capture builds and this
  canonical gate. Complete matched SB-enabled 30000-ms captures retain the
  previous 2100 frame/end bytes and 43 register packets on both targets.
  First original failure remains 817, with 28 differing frames; strict full
  comparison still fails because final mixed port PCM is absent. All 418
  original assets remain unchanged. These checks accept only patch632's
  failed-variant WORD bridge/data repair, not startup or full output parity.
- `engine-font-loader/proof.json` and `proof-final.log` retain the full
  parent-red/current-green and source/target/binary proof. Earlier gates
  are excluded: `run.VPN4nz/` reached 178/0 but its outer session ended143
  without a durable terminal receipt; `run.rAmTNP/` ended2 because the new
  output directory was missing. Both failures and the directory correction
  are retained separately. `run.P3o3M4/` was deliberately canceled143 to
  remove patch-context whitespace; its emitted C was byte-identical.

- Additional original live-BP provenance is complete in
  `engine-live-bp/{check-source.py,source-proof.json}`. The 2293 fetched
  instructions to boot222f take 2363 cycles, including 70 recorded callback
  cycles. All 127 hardware BP changes from tick300 are retained. The last
  BP producer is actual1345's third-list `MOV BP,1718` at image11355/runtime
  2082:1cc5; whole GP/segments/flags match the single assignment, and the
  remaining 29 fetches to222f preserve BP. Entry GP/segments/flags and all
  39 frames/27518 samples/end600ms match the earlier source capture. Twelve
  low-DOS bytes differ between the processes' complete entry memory snapshots;
  all are retained explicitly and whole entry-memory equality is not claimed.
- A debugger stop at current frozen Native222f confirms the existing
  `g_fist_1345_bp` producer is1718, while2144's incoming parameter is0 and
  vector388 is still0. That parameter cannot represent the updated live BP.
  `engine-live-bp/port-entry-complete/diagnostic-proof.json` retains the entire
  16-MiB target snapshot, state and binary hash. Debugger exit0 means the
  intended stop, not a successful complete game run. The late TIMER519 hook
  (exit124) and first GDB array-address fixture (exit1) are excluded. Reuse
  the actual MEMMGR output when recovering the reached caller; do not seed BP
  from this measured value or claim an unproved general CPU-register owner.

- Additional controlled original `222f` loaded-bit evidence is complete in
  `engine-resource-skip/{check-source.py,source-proof.json}`: one descriptor
  flag byte is set before the interval and restored only after the recorded
  return. Actual TEST/JNZ/RET has three fetches/four boundaries, leaves the
  whole 16-MiB guest memory unchanged and changes only ESP by two among GP
  registers. Segments and raw flag cell are preserved; lazy state is TESTb
  with operands/result80, not a materialized CPU flag return. All 39 frames,
  27518 mixed samples and endpoint600ms still match the normal source output.
  All interprocess baseline-memory differences are retained. This verifies
  the caller's skip branch, not a general loaded-resource flow or port CPU
  integration. The initial verifier transcribed C388 from a symbol name;
  actual image/runtime instruction bytes both prove far operand0388. That
  excluded verifier did not change a producer, source input or trace.

- Consume0036's fresh original application fetch-state proof in
  `engine-entry-registers/`: full EBX is `00000000` at cycle629918/budget82.
  The complete original MZ image, all 1143 relocations and both DOS EXEC stack
  stores account for every loaded-image byte; all 39 frames/27518 mixed samples
  and end600ms match. Generated app_entry currently combines its WORD BX
  parameter with an uninitialized high-word local. Recover a full incoming
  DWORD owner rather than clearing unknown caller bits. The original153c
  controlled `89ab` upper-word preservation remains required; the Native
  scratch typed222f/early-install prototype is not a two-target repair.

- Patch 633 at base `cadd1fa` restores the original CRT223c
  early0174 installation through the existing relocation-table owner and
  boot222f's typed CX/DX/BX/BP arguments. BP consumes actual1345's published
  output; no constant1718 is injected. The source-proven loaded-bit skip
  remains. Delayed098 installation and133 MSPRITE0 retry are removed from
  e714; its harness presentation boundary still enables scripted input there.
  Original e714 starts with BE0E(4), without either inserted resource operation.
- `engine-boot-resource/{check-partial.py,partial-proof.json}` reproduces four
  reaching parent failures: Native misses all16 CRT vectors and passes invalid
  boot-call arguments, while WASM traps at both wrong production signatures.
  Eight current Native/WASM phase cases pass, including the existing three
  failed variants and loaded-bit skip. Every 16-MiB target output is checked;
  CRT comparison starts immediately after the last preceding DOS call and
  accounts for every vector write. Earlier legacy interrupt/ES stores are
  retained as input; they are not accepted as original CPU behavior. The actual
  emitted RET helpers and real DOS dispatcher execute, and unexpected recovery
  or successful-variant loader paths fail instead of being replaced with stubs.
  All 892 frozen code/input files match. The unfiltered canonical gate
  `scratch/verify/run.Hw6Cpn/` completed with durable exit0: all 103 tests,
  exact patches, both sequential builds and 178 existing flows pass.
  `engine-boot-resource/{check-proof.py,proof.json,proof-final.log}` rechecks
  the full source archive and captured/tested binary bindings. Accept only the
  early vector/data and boot-call arity repair. Dynamic153c BX output,
  uninitialized incoming high EBX, guest CPU/flags/stack/IRQ/time, first817
  and final mixed PCM remain open.

- Fresh patch 633 production captures in `engine-boot-resource/` reach
  end 30000 ms with 2100 full frames on each target, retaining every632 frame/end
  byte. All original palettes/layouts/times agree; the same 28 pixel failures
  remain, first817 at11704156us/byte8754. Strict full comparison still fails
  absent final port PCM. All 43 complete sound-register records match between
  Native/WASM. Their payloads retain632's values, but 15 timestamps change;
  every old/new record is retained in `capture-diagnostic-proof.json`, not
  discarded or accepted as original timing. The first gate shifts 2593 cycles;
  the actual early4523-byte read consumes 9130 cycles from budget 9135 to5.
  This is measured existing DOS/clock behavior, not a fitted total delay.
- The verified 633 Native binary reaches boot222f with the source-backed
  filename and CX/DX/BX/BP lanes, installs vector388=`0f69:306c`, and loads
  MSPRITE0 with 40 registered entries before selector6c. Config220/7/1 remains.
  The retained whole16-MiB debugger snapshot still posts inbox`5806e816`,
  proving the remaining descriptor-offset transport. Backtrace shows both
  app_entry and00d0 reconstruct uninitialized high words. Recover both full
  incoming DWORD paths and actual153c's dynamic WORD output; neither may be
  replaced by a fixed018f or cleared unknown high bits. The debugger's exit0
  is an intentional observation stop, not a complete game run. All 418 assets
  remain unchanged. These diagnostic limits also apply after the green full gate.

- Original153c return branches are now directly measured in
  `resource-return-branches/{check-source.py,source-proof.json}`. Controlled
  empty first-length input takes14 fetches and returns toCB13 with unchanged
  lowBX=`e816`; a controlled callee-return CF input takes5 fetches, restores
  AX/CX/BX and also preserves `e816`. The latter proves the carry consumer,
  not a real loader-failure producer. Both complete16-MiB branch deltas match.
- Controlled incoming DX at the source1630 table limit and one descriptor
  below it take14/35 fetches. The second case computes BX=121 from the actual
  first MSPRITE0 dimensions0b*0b; neither exit stores the resource entry count.
  Both execute15bf POP DS / STC / RET with one earlier saved DS still on the
  stack. RET therefore consumes `2d19` as IP, leaving DS at resource54c2 and
  SP two bytes below the normal CB13 return. This is an observed abnormal
  original return, not a guessed normal overflow contract or a proved crash.
  Do not describe the port's simple C overflow return as guest-stack parity.
- All four cases retain complete GP/segments/raw+lazyflags/control state,
  instruction boundaries and whole memory; every interprocess entry-memory
  difference is retained. Debugger exit0 is an intentional branch stop;
  complete frame/PCM/end validation explicitly fails for these partial runs.
  Two observer timeouts are excluded. The corrected Python arming callback
  records ticks0..524 and explicitly gates524; a breakpoint condition property
  alone did not bound the failed observers. Normal full-output and controlled
  high89ab evidence remain separate in `engine-resource-registers/`. Recover
  the actual dynamic successful WORD output and carry/CPU owners without
  seeding018f, clearing unknown high bits or hiding these branch contracts.

- At base `fc14eb4`, the unmodified original controlled-dimension pair in
  `resource-bx/source-dimensions/` changes only the last loaded MSPRITE0
  dimension WORD from1315 to0705. Actual153c returns BX=35 from5*7;
  all other complete GP/segments/raw+lazyflags/control fields match the
  original normal return. Every whole16-MiB loop write is accounted for,
  including all40 descriptors, header and retained stack words. The loop
  still takes854 fetches; the whole registrar takes1410/16043cycles.
  All39 complete frames/27518 mixed samples/end600ms retain the normal
  original bytes. `resource-bx/check-source-dimensions.py` verifies this
  source-only proof; it does not accept a production patch or CPU/device
  timing. Consume the existing normal, empty and high89ab pairs alongside it.

- At base `dd58003`, the original settings op68 poster is now paired in
  `settings-poster/{check-source.py,source-proof.json}`. The actual e2df
  entry has full EBX=5 and independent AX=2. Eight fetches store the complete
  EBX to the task inbox, preserve the raw/lazy flags and unrelated registers,
  update FS/GS, set the WORD command68 and retain the near-CALL stack word.
  Every whole16-MiB write matches; all39 complete frames/27518 mixed samples/
  end600ms and the complete525 CPU trace retain the normal source bytes.
- The frozen634 candidate's 30-second diagnostics retain633's2100 frame/end
  bytes and43 full sound-register rows. Its first6c inbox is now0000018f
  on both targets, but its47 complete extender packets contain one remaining
  cross-target inequality: op68 inbox `f6e80008` versus `00000008`.
  Original op68 inbox is5; neither port's low8 is established as original
  ambient BX. Preserve both complete rows. Recover the preceding settings/DOS
  handle/return-register producer and independent AX consumption before any
  generic op68 claim; clearing a high word or substituting5 is not its owner.
  This source/probe evidence does not accept the pending634 production patch,
  original CPU/IRQ/clock or final mixedPCM; the full existing matrix is still live.

- Base `c18f2d6`, actual original task-mode get354/put358 startup pair:
  `task-mode-registers/{check-source.py,source-proof.json}` accounts for all
  15 fetches and16 boundaries. Getter354 reads the existing CS mode byte0
  into AL, replacing incoming AX=`00ea`; setter358 consumes that returned
  byte and exchanges it with CS mode2d59. Its SS:ea2c/ea2e resolves actual
  task1000:0 and writes the byte at task+496. Every whole16-MiB stack/task/
  exchange write and complete GP/segments/raw+lazyflags/control field matches.
  Original far CALL/RETF materialize pending WORD ADD/OR flags; MOV itself
  does not clear them. All39 frames/27518 mixed samples/end600ms and the
  complete500..526 CPU trace retain normal bytes. Nonzero/null-pointer
  cases and production CPU/IRQ/clock integration remain unproved.
- The frozen634 Native first6c watchpoint diagnostic attributes all three
  other whole-memory differences. 46b6 stores an uninitialized ES. d99b
  discards getter354's byte and its argument-less vector358 calls23bf with
  observed stack-residue AL144. The old129 SS literals `3a4bc/3a4be` resolve
  another resource's packet and write physical76386, then CS mode12d59.
  Original SS is a recovered operand; neither a guessed mode0 nor the stale
  literal owns this contract. `resource-bx/native-diagnostic/proof.json`
  retains all seven full16-MiB differences, the three writer backtraces,
  actual first6c inbox018f and every4523 resource/5688 directory byte.
  This is an intentional Native debugger stop, not two-target architectural
  memory, complete output or pending634 production acceptance.

- Patch634 exposes153c's actual WORD BX owner after every reached MUL
  and preserves incoming BX on the empty chain; cae6 consumes it after
  return. Explicit full incoming DWORDs replace uninitialized high-word
  construction in app_entry,0007 and00d0. The void registrar signature stays.
  `resource-bx/check-proof.py` verifies five reaching parent transport failures
  plus three cross-target packet failures, then six complete startup cases
  on both targets under original and default start fixtures. Normal/changed
  0705/empty inputs produce018f/0023/e816; every600-ms frame/end byte agrees.
  No startup producer is replaced with a test stub. All895 frozen code/input
  files and captured/tested binary bindings agree. The unfiltered canonical
  `scratch/verify/run.9hkvAE/` completes103 tests, exact patch checks, sequential
  Native/WASM builds, the six new cases and all178 existing flows with zero
  failures; durable `gate.exit` and `proof-final.exit` are0. Proof:
  `resource-bx/proof.json`. This accepts only the BX/full-EBX data contract.
  Complete30000-ms captures retain all2100 prior frame/end bytes and43 full
  sound-register rows. All47 extender packets remain recorded; op68 still
  differs (`f6e80008` Native / `00000008` WASM), also differing from original5.
  Original palettes/layouts/times agree,28 pixel differences remain first817
  /11704156us/byte8754, and strict comparison rejects absent final mixed port
  PCM. Original loaderCF/overflow gueststack, CPU/IRQ/IF and instruction/device
  work stay open; this gate does not establish complete original parity.

- At base `a28706d`, controlled original mode-byte cases in
  `task-mode-controlled/{check-source.py,source-proof.json}` prove the
  getter preserves all upper24 EAX bits and returns mode42. The setter stores
  incoming42/a5 to the task and CS byte but returns old42, preserving89ab7b.
  Null SS task-pointer input skips the LES/task store and still exchanges the
  CS mode. All complete GP/segments/raw+lazyflags/control states and16-MiB
  interval writes match. Reached paths take15/15/13 fetches; the shorter null
  path's two-cycle difference is retained, without a time or budget adjustment.
  Architectural test inputs and, for the null case, observed normal flags are
  restored only after the controlled return. All39 frames/27518 mixed samples/
  end600ms then retain normal bytes. These are controlled byte/pointer proofs,
  not natural unchanged-state startup, a port repair or full CPU/device/PCM
  acceptance. Thread the actual getter output; neither zero nor incomingAL
  can replace the measured exchange return contract.

- The task-mode CS owner is also wrong in the emitted image: original
  `MOV AL,CS:[2d59]` and `XCHG AL,CS:[2d59]` address image123e9,
  while `DAT_1000_2d59` currently declares a WORD at code bytes12d59=ff78.
  `task-mode-prototype/check-mode-mapping.py` verifies the complete56-byte
  original code/data region against the image and derives the base from
  the actual caller/getter segments. A scratch-only repair of this shared
  BYTE owner, rebased SS pointer and typed get/put transport gives eight
  complete16-MiB Native/WASM cases after eight reaching parent failures.
  Cases include a nonzero adjacent-byte input, independent exchange return
  and null pointer. The first pointer/caller-only prototype failed because
  the stale WORD owner overwrote adjacent code; its artifacts remain in
  `task-mode-prototype-excluded-missing-width-cs-base/`. No production code,
  complete startup, handler depth/error fields, CPU/flags/time or PCM is
  accepted by this leaf/post-create-fragment proof. Source and scoped target
  proofs: `task-mode-prototype/{mode-mapping-proof.json,proof.json}`.

## Next

1. Restore actual get354/put358 byte transport, the rebased SS pointer and
   the source-proven CS BYTE owner through the original pairs and scratch
   regressions above. Verify complete startup and both target matrices before
   production acceptance. Preserve632/633 callee widths, early resource
   installation, typed `222f -> 26fc` and634's dynamic WORD BX/full incoming
   EBX transport. Consume the original
   normal/controlled/CRT/return-branch pairs above; scratch prototypes are not
   production acceptance. Keep abnormal overflow and unproved CF producers explicit.
   Wire original device initialization and consume the recovered intro registers. Recover
   the 14e0 completion IRQ, DMA/mixer demand and other channel-switch contracts; restore
   remaining mission/damage/weapon transport as reached. Supply actual instruction/event work to 0026 so
   0034 can recheck frame 817. Add original-backed buffer/state regressions on both targets;
   do not charge an aggregate measured delay or invent a sample source.
2. Recover speaker synthesis against a captured original event schedule. Prove its first sample;
   let 0026 repair event timing. Reuse 0036's start fixture and 0034's capture/comparison tools.
3. Route speaker/OPL/SB into one mixer driven by the shared clock. Test simultaneous emission,
   silence and saturation. Remove duplicate WAV/sequence/browser writers after their inputs are wired.
4. Recover original channel allocation/pitch/completion; test overlap, exhaustion and release.
5. Capture complete matched menu and AZER1 output, then cockpit loss/debrief and device variants.
   Hand reusable content fixtures to 0011; report the first unequal final sample.

## Accept

Equal complete sample counts and PCM bytes across all three targets. No resampling, time-warping,
fitted offset or correlation threshold replaces equality. Compare final mixed stereo with final
mixed stereo. Original content/trace entry points: `capture_audio.sh`, `trace_opl.sh`,
`FIST_AUDIO_WAV`, `FIST_OPL_REGLOG`, `ref/audio_menu_noteseq.txt`.
