Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

Capture every presented frame with its presentation time and the continuous PCM stream over explicit
scenario start/end boundaries. Same initial state, timed input and devices; no host-stack equality.

## Evidence

- Rebuilt instrumented DOSBox with `bash tools/oracle/build_sequence_oracle.sh` (the wrapper checks
  base/patched source hashes and applies `tools/oracle/sequence_probe.patch`); run
  `bash tools/oracle/probe_sequence.sh 50 scratch/sequence-probe/intro-menu` on an isolated asset copy.
  The run observed 3429 `RENDER_EndUpdate` calls (70 at 640×400, 3358 at 320×200, one zero mode),
  48928 `MIXER_MixData` calls, zero capture-state changes and two aborted render updates. The first
  320×200 frame completed at 1030.744 ms. Of those 3358 frames, 2963 caused no host surface update;
  repeated scanouts must still be represented. Evidence: `scratch/sequence-probe/intro-menu/dosbox.log`
  and `scratch/sequence-probe/summary.txt`. This is a timed probe, not a complete output capture.
- Full unmasked Oracle capture: `bash tools/oracle/capture_sequence.sh 50
  scratch/sequence-capture/intro-menu` yielded 3448 frame records and 2,182,440 stereo sample
  frames at 44,100 Hz; both streams have validated completion records. The last captured 320×200
  frame has AE=0 against `ref/main_menu_native320.png` after palette resolution. Evidence:
  `scratch/sequence-capture/intro-menu/` (including SHA-256, stream validator output and PPM).
  A separate four-second run with both capture and probe enabled counted 146,372 mixer samples
  and exactly 146,372 captured samples, including the first 1103-sample mixer step.
  `FISTSEQ1` stores ordered pixel-index frames plus RGB8 palette and microsecond timestamps in `.frames`,
  and continuous stereo PCM16 LE with sample positions in `.pcm`. Both end with checked record counts;
  `sequence_format.py` rejects truncated, reordered and malformed streams. This proves the Oracle
  writer, not port parity or exact scenario synchronization.
- The first mixer step generated 1103 samples; all later steps in this run generated at most 1024.
  `MIXER_MixData` limits the old `CAPTURE_AddWave` call to 1024 samples. A capture armed at boot
  would omit 79 samples from that step; normal menu recording starts later. Instrument the final
  mixer without this cap and assert total emitted sample count.
- `RENDER_EndUpdate` reaches `CAPTURE_AddImage` only under capture state; ordinary presentation
  cannot be inferred from existing image captures. Inspect abort and frameskip before treating
  this callback as a completed scanout. `PIC_FullIndex` gives DOSBox time in milliseconds.
- Native and Node-WASM now emit complete frame-only `FISTSEQ1` captures at INT-8 and op 0x24.
  `bash tools/capture_port_sequence.sh {native,wasm} 400 <fresh-dir>` yielded 465 events each;
  timestamps and RGB frames match byte-for-byte. At tick 120 both emitted 138 events.
  Evidence: `scratch/sequence-capture/port-{native,wasm}-400/`. PCM is still absent.
- The Oracle has 69 preceding non-320×200 loader frames. After the first 320×200 frame, the
  first *visible* RGB mismatch is relative frame 7, pixel (144,58), green 0 vs 4: the port
  displays the next KDV image three scanouts before the original. Original first image change is
  at guest 1173.424 ms; port at 198.608 ms (different boot origins). The first following frame
  interval differs by 644 µs. Unused palette entries differ even earlier; they do not alter RGB.
  Diagnose e584's `[b6e0]` wait, `[0x452]` ISR phase and op-0x78 posts before patching.
- Browser worker posts at INT-8 and op 0x24; `fist_web_post_frame` forces the palette. The
  canvas keeps only `latest` until `requestAnimationFrame`, so worker posts may be dropped from
  actual display. This needs browser measurement under 0026; Node runs cannot prove it.
- OPL and SB each open `FIST_AUDIO_WAV` independently with `wb`; browser audio drains only OPL.
  This file is not final mixed output. Owner for that defect: 0003.

## Next

1. Measure the first KDV timing difference against the original e584 contract. Capture timed
   `[b6e0]`, `[b6e6]`, `[0x452]` and op-0x78 events across the first visible transition; correct
   their producer, then repeat complete frame-sequence comparison on both targets.
2. Implement a single final port mixer owner (0003) and emit continuous PCM, including SB effects;
   the two existing WAV writers and OPL-only browser feed do not satisfy this capture.
3. Drive original and both ports to the same explicit *virtual* scenario end and timed inputs. The
   current 50-second Oracle run ends with Ctrl+F9 at 49.228 seconds of guest time. Add a common start
   marker and completion condition; retain the full boot capture as provenance.
4. Compare sequence order, frame time/dimensions/RGB and continuous PCM format/samples.
   Audio chunk sizes may differ. Report the first difference, retain all artifacts and seed failures
   for dropped/reordered frames, changed samples and incomplete captures.

## Accept

All three captures cover the same complete intro/menu scenario with independent oracle provenance.
The runner reports the first differing frame/time/sample; dropped, added or reordered frames,
changed samples and unfinished captures fail. Seed these faults to prove detection. A reproducible
original/port difference stays red and gets its producer WI; capture completion does not close
parity (0012), audio fidelity (0003) or presentation timing (0026).
