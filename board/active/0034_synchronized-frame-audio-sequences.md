Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

Capture every indexed frame, all 256 palette entries, presentation time and continuous PCM over
explicit scenario boundaries. Same initial state, timed input and devices; no host-stack equality.

## Evidence

- `build_sequence_oracle.sh` plus `capture_sequence.sh` captured 3448 Oracle frame events and
  2,182,440 stereo PCM frames at 44,100 Hz with complete `FISTSEQ1` footers. The final 320×200
  frame has AE=0 against `ref/main_menu_native320.png`. A paired probe counted every one of
  146,372 generated samples, including the first 1103-sample mixer step that legacy
  `CAPTURE_AddWave` would truncate to 1024. Evidence: `scratch/sequence-capture/intro-menu/`.
- `RENDER_EndUpdate` is the capture boundary; most repeated scanouts do not change the host
  surface, and `CAPTURE_AddImage` only runs when recording is armed. `sequence_format.py` rejects
  incomplete streams. The callback's abort/frameskip semantics still need proof as presentation.
- Native and Node-WASM now emit complete frame-only `FISTSEQ1` captures at INT-8 and op 0x24.
  `bash tools/capture_port_sequence.sh {native,wasm} 400 <fresh-dir>` yielded 465 events each;
  timestamps and RGB frames match byte-for-byte. At tick 120 both emitted 138 events.
  Evidence: `scratch/sequence-capture/port-{native,wasm}-400/`. PCM is still absent.
- `cycles=max` changes startup event times between Oracle runs; its 69 loader frames and
  three-frame intro offset are provenance, not a stable timing baseline. Use
  `FIST_DOSBOX_CYCLES='fixed 30000'` with `build_timing_oracle.sh` and the sequence runners.
  Two independent fixed-cycle probes gave identical first 320×200 scanout (475.189 ms), `e584`
  entry (546.979 ms), `TITLE.KDV` open (548.162 ms), and first two writes to pixel (144,58)
  (553.103/621.253 ms). Evidence: `scratch/sequence-capture/fixed30k-{a,b}/`.
- A five-second fixed-cycle Oracle capture has 30 preceding loader frames. Its first nine distinct
  320×200 RGB images have the same hashes/order as native and WASM, but change at ordinals
  `0,11,16,20,25,30,34,39,44` versus `0,7,11,16,21,25,30,35,39`. The first visible mismatch
  is frame 7, pixel (144,58). Strict comparison finds an earlier palette mismatch at the first
  320×200 frame, entry 17 red=20 versus 0, even though both index buffers match. Loader frames
  are earlier still. Ordinal alignment is diagnostic only; neither difference can be masked.
  `compare_frames.py` now fails on every palette/index byte, including unused entries.
  Evidence: `scratch/sequence-capture/fixed30k-full/` and
  `port-vga-{native,wasm}/`.
- The original's MGA `04a3` palette-clear request starts 535.011 ms, returns 546.468 ms:
  11.457 ms awaiting its vblank service. In the port, the corresponding DAC reset is 113.524 ms
  and KDV opens 114.222 ms: patch 070 services it synchronously. Relative to the first 320×200
  scanout, the request occurs at 59.822 ms in the Oracle and 14.151 ms in the port. The
  45.671 ms preceding gap and missing vblank wait are separate causes to trace. Original KDV
  writes come from extender `0x10007120`; its descriptor has `[b6e0]=4` timer ticks. Evidence:
  `scratch/sequence-capture/fixed30k-full/dosbox.log`,
  `scratch/sequence-capture/port-vga-{native,wasm}/port.log`.
- `bash tools/check_flow.sh '^(intro|settings-sky)$'` passed tests, patch checks, both builds and
  both selected flows after adding gated port tracing; the earlier full 178-flow matrix was
  green before that trace-only change. Evidence: `scratch/verify/run.gKbZLj/` and
  `scratch/verify/full-0034-frames/`. Strict palette/index comparison passed 28 unit tests
  and all 138 native/WASM frame events; evidence: `scratch/sequence-capture/strict-palette-tests.log`.
- Browser worker posts at INT-8 and op 0x24; `fist_web_post_frame` forces the palette. The
  canvas keeps only `latest` until `requestAnimationFrame`, so worker posts may be dropped from
  actual display. This needs browser measurement under 0026; Node runs cannot prove it.
- OPL and SB each open `FIST_AUDIO_WAV` independently with `wb`; browser audio drains only OPL.
  This file is not final mixed output. Owner for that defect: 0003.

## Next

1. Recover the original mode-switch→MGA-04a3 call chain and account for its 45.671 ms work in
   the port clock. Restore 04a3's vblank-serviced palette upload/wait (patch 070) from original
   IRQ evidence. Re-capture fixed-cycle Oracle/native/WASM streams; diagnose the first mismatch.
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
