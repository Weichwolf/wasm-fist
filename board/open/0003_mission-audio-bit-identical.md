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

## Next

1. Restore script register transport, then wire the original
   device initialization and 14e0 completion IRQ, DMA/mixer demand and other channel-switch contracts,
   including device setup and DMA cadence. Supply actual instruction/event work to 0026 so
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
