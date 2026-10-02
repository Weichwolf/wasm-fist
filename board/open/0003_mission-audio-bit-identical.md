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

## Next

1. Recover the reached 2630 channel/normalization/rollover contract and 14e0 completion IRQ,
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
