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
- Next callback evidence is isolated, not a production fix: at tick 13257 channel two rolls
  to the original silence sample and invokes default callback 2294 (actual RET). Its seven
  non-stack GP registers are preserved; ESP advances four bytes. Paired source snapshots and
  a complete independent CPU trace retain all 1119 frames / 706658 PCM samples over 16000 ms.
  `scratch/sequence-capture/mixer-callback/protected/` includes the state/asset provenance and
  isolated prototype. The parent emits unequal mixed bytes on native and traps on WASM due
  to the C function signature mismatch; the prototype matches complete state and all 1024
  original mixed bytes on both targets. Production repair, other callback variants and timing
  remain open. The earlier probe using a non-protected page root failed and is excluded.
- Read-only original device provenance: `scratch/sequence-capture/sb-lifecycle/{probe-final.gdb,
  device-events-final.jsonl,proof.json,final-original-30s/}` preserves every complete baseline
  30000-ms frame/PCM/endpoint byte. Commands are timeconstant A6, speaker enable, block size
  03ff and auto-init 1c: 11111 Hz, a 1024-byte DSP block and a separate 2048-byte DMA ring.
  The 319 IRQs alternate DMA halves; Q14 mixer increment is 4127. Tick intervals are 92/93 ms,
  not a fixed 92-ms period. The port incorrectly conflates DSP and DMA lengths. No post-transfer
  finish snapshot is claimed (the finish breakpoint was not reached). Restore actual mixer
  demand/DMA/PIC lifecycle; these observed intervals must not become a fitted timer.

## Next

1. Recover 2630 channel rollover byte writes/register callbacks and the 14e0 completion IRQ,
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
