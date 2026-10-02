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

## Next

1. Wire original device initialization and consume the recovered intro registers. Recover
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
