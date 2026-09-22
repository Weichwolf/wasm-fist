Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

Capture every presented indexed frame, all 256 palette entries, presentation time and continuous
mixed PCM over explicit scenario boundaries. Match initial state, timed input and devices. Report
the first unequal byte/event; no masks, truncation or host-stack equality.

## Evidence

- The sequence Oracle (`build_sequence_oracle.sh`, `capture_sequence.sh`) captured 3,448 frame
  events and 2,182,440 stereo samples at 44,100 Hz with valid footers. DOSBox frame capture is
  `RENDER_EndUpdate`, including unchanged scanouts. Use `FIST_DOSBOX_CYCLES='fixed 30000'` for
  reproducible timing; `cycles=max` varies. Evidence: `scratch/sequence-capture/intro-menu/`,
  `fixed30k-{a,b}/`, `fixed30k-full/`.
- Native and Node-WASM now capture at a VGA scanout event after the 50-ms mode resize, next
  vertical timer and 400 visible lines, independently of INT-8. Both emit 24 identical
  indexed events through tick 20; PCM is absent. The first two 320x200 frames match the
  fixed-cycle Oracle byte-for-byte (all 64,000 indices and 768 palette bytes). The third
  differs at palette entry 17: Oracle DAC upload at 503.000 ms precedes its 503.725-ms
  scanout by 0.725 ms; port upload at 184.327 ms follows its 183.934-ms scanout by
  0.393 ms. Both the last zero PIT reload and first scanout occur 2.8 ms later relative
  to mode set in the port; the remaining gap is reload→DAC service: 57.073 ms versus
  55.959 ms in the Oracle. First scanout times differ by a constant 319.792 ms
  (475.189/155.397 ms);
  a common scenario start marker is still needed. Earlier 640x400 loader frames are not
  captured by the port yet. Evidence: `scratch/sequence-capture/fixed30k-full/`,
  `scanout2-{native,wasm}/`; `compare_frames.py` checks every byte (28 unit tests).
- Original `2ebe` applies FAR relocation section `0x1b8` at 404.239 ms, installing
  `DGROUP:0x426=2082:3943` (base-zero `0f69:3943` = timer `2fd3`). MGAVIDEO `00e8`
  calls it after BIOS mode 13h. Patch 127 had elided this section; port vector `0x426`
  was zero. Patches 611/612 restore the section and fix the newly reached `3318` FAR
  vector through `SS:0x32a`. Patches 613/614 thread the sound driver's `ES:BX` and
  initialize the two callback sentinels omitted by Ghidra. At the reached splice,
  SOUNDDVR:0x1cd is `ea f6 3b 69 0f` in the port, the base-zero equivalent of the
  Oracle's `ea f6 3b 82 20`. The first driver `0be2` service is now 99.340 ms after
  port mode set versus 95.423 ms after Oracle mode set; formerly it was 13.729 ms.
  Evidence: `scratch/sequence-capture/vector-426/mem.watch.txt`, `calib-flag/mem.watch.txt`,
  `patch614-native/port.log`, `patch614-gdb-node.log`, `snd-node/mem.watch.txt`,
  `pittrace-fixed/dosbox.log`.
- Oracle presentation timing: BIOS mode 13h enters at 407.577 ms; delayed
  `VGA_SetupDrawing` at 457.582 ms; next vertical timer at 462.478 ms;
  `RENDER_EndUpdate(false)` first captures 320x200 at 475.189 ms, then every 14.268 ms.
  Driver DAC upload starts 503.000 ms. `FIST_SCANTRACE` reproduces this chain via
  `build_timing_oracle.sh`. Evidence: `scratch/sequence-capture/scantrace-original/dosbox.log`.
- The third-frame DAC miss is caused by the port's `in(0x3da)` fast-forward.
  At the first post-calibration IRQ the Oracle polls at 501.968–502.994 ms,
  with bit 3 clear, then exits `30f8` after its 1000-read timeout. The port
  enters `30f8` at 182.186 ms with the same limit, but makes only five reads:
  its unchanged-status shortcut jumps from 182.187 to 183.934 and 184.316 ms.
  It reaches the DAC at 184.327 ms, after the 183.934-ms third scanout.
  Evidence: `scratch/sequence-capture/3da-original/dosbox.log`,
  `port-3da-gdb.log`, `port-int8-gdb.log`. Remove the shortcut and verify the
  1000-read timeout against both targets before adjusting any clock cost.
- Oracle MGA `04a3` request starts 535.011 ms and returns 546.468 ms after vblank service.
  Port patch 070 still services it synchronously; port KDV opens after its immediate reset.
  Browser canvas may drop worker posts because it retains only `latest`. OPL/SB write separate
  WAVs and browser drains only OPL; neither is final mixed PCM. Owners: 0026 and 0003.
- Patches 611–614 passed all 178 existing native/WASM flows (45+45+44+44 unique, all exit 0;
  tested hashes retained in `scratch/verify/full-611-614/`). The scanout change passed
  `bash tools/check_flow.sh '^(intro|settings-sky)$'` on both targets and 141/141 strict
  native/WASM frame events through tick 120. Evidence: `scratch/verify/run.FHZPVC/`,
  `scratch/sequence-capture/scanout120-{native,wasm}/`. Its exact-source full matrix
  passed all 178 flows (45+45+44+44 unique, all exit 0; hashes retained under
  `scratch/verify/full-scanout/`). This validates existing coverage, not full
  Original/native/WASM sequence parity.

## Next

1. Remove the `0x3da` unchanged-status fast-forward; prove the first `30f8`
   timeout, DAC upload and third frame against the Oracle. Then recover the
   separate 2.8-ms post-mode VGA phase difference, compare complete 320x200
   sequences at a common scenario start marker, restore `04a3`'s vblank wait
   from IRQ evidence and account for preceding loader frames.
2. Give the port one final mixer owner (0003), emitting continuous PCM including SB effects.
3. Drive all three runs to the same explicit virtual scenario end with timed input; compare
   event order, frame timing/dimensions/indices/palette and full PCM samples. Seed failures
   for dropped/reordered events, changed samples and incomplete captures.

## Accept

All three captures cover the same complete intro/menu scenario with independent Oracle
provenance. The runner reports the first differing event/time/byte/sample; missing, changed
or unfinished output fails. A remaining reproducible difference keeps parity (0012), audio
(0003) or presentation timing (0026) open with its producer named.
