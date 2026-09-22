Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

Capture every presented frame with its presentation time and the continuous PCM stream over explicit
scenario start/end boundaries. Same initial state, timed input and devices; no host-stack equality.

## Evidence

- Rebuilt instrumented DOSBox with `bash tools/oracle/build_sequence_probe.sh` (the wrapper checks
  base/patched source hashes and applies `tools/oracle/sequence_probe.patch`); run
  `bash tools/oracle/probe_sequence.sh 50 scratch/sequence-probe/intro-menu` on an isolated asset copy.
  The run observed 3429 `RENDER_EndUpdate` calls (70 at 640×400, 3358 at 320×200, one zero mode),
  48928 `MIXER_MixData` calls, zero capture-state changes and two aborted render updates. The first
  320×200 frame completed at 1030.744 ms. Of those 3358 frames, 2963 caused no host surface update;
  repeated scanouts must still be represented. Evidence: `scratch/sequence-probe/intro-menu/dosbox.log`
  and `scratch/sequence-probe/summary.txt`. This is a timed probe, not a complete output capture.
- The first mixer step generated 1103 samples; all later steps in this run generated at most 1024.
  `MIXER_MixData` limits the old `CAPTURE_AddWave` call to 1024 samples. A capture armed at boot
  would omit 79 samples from that step; normal menu recording starts later. Instrument the final
  mixer without this cap and assert total emitted sample count.
- `RENDER_EndUpdate` reaches `CAPTURE_AddImage` only under capture state; ordinary presentation
  cannot be inferred from existing image captures. Inspect abort and frameskip before treating
  this callback as a completed scanout. `PIC_FullIndex` gives DOSBox time in milliseconds.
- Port browser posts at `fist_web_vblank` (INT-8) and after op 0x24. `fist_web_post_frame`
  forces the palette from an engine buffer; native snapshots use DAC state. The CLI native and
  Node-WASM runs do not currently expose a common presentation stream.
- OPL and SB each open `FIST_AUDIO_WAV` independently with `wb`; browser audio drains only OPL.
  This file is not final mixed output. Owner for that defect: 0003.

## Next

1. Capture full original completed scanouts and final mixed PCM at the measured DOSBox boundaries.
   Preserve duplicates, timestamps, palette state and every mixer sample; reject overflow and abort.
   Use a fresh isolated output directory. Do not rely on DOSBox capture-state toggles.
2. Add passive native/WASM capture at shared presentation points and expose the actual browser path.
   Measure the palette-forcing and extra op-0x24 posts rather than assuming equivalence.
3. Specify a streaming format with event order, time units, dimensions/palette or RGB pixels,
   PCM rate/channel/width/sample position and an explicit completion record.
4. Build the three-way runner. Match initial state, input timing and devices; report first differing
   event/pixel/sample, fail missing/reordered output, and retain oracle provenance. Start intro→menu.

## Accept

All three captures cover the same complete intro/menu scenario with independent oracle provenance.
The runner reports the first differing frame/time/sample; dropped, added or reordered frames,
changed samples and unfinished captures fail. Seed these faults to prove detection. A reproducible
original/port difference stays red and gets its producer WI; capture completion does not close
parity (0012), audio fidelity (0003) or presentation timing (0026).
