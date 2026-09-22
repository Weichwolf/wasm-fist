Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

Capture every presented indexed frame, all 256 palette entries, presentation time and continuous
mixed PCM across explicit scenario boundaries. Match initial state, timed input and devices. Report
the first unequal event, byte or sample; reject missing or incomplete output without masks.

## Evidence

- The sequence Oracle captured 3,448 presented frames and 2,182,440 stereo samples at 44,100 Hz
  with valid footers. Use `FIST_DOSBOX_CYCLES='fixed 30000'`; `cycles=max` varies. DOSBox captures
  `RENDER_EndUpdate`, including unchanged scanouts. Its mode-13 scanouts start at 475.189 ms and recur
  every 14.268 ms. Evidence: `scratch/sequence-capture/fixed30k-full/`, `scantrace-original/`.
- Native and WASM capture at VGA scanout, independently of INT-8. They match all 24 indexed events
  through tick 20. The first event is 155.397 ms; the Original-to-port offset is about 319.792 ms,
  but post-mode VGA phase differs by 2.8 ms. The port omits preceding 640x400 loader frames; no common
  scenario start marker exists yet. PCM is absent from the port sequence. Evidence:
  `scratch/sequence-capture/patch616b-{native,wasm}/` and `scantrace-original/`.
- Patches 611–614 restore timer relocation, FAR callback registers and sentinels. The scanout capture
  and removal of the `0x3da` poll fast-forward preserve the Original's 1000-read `30f8` timeout.
  Patch 615 fixes MGAVIDEO `0a50`: Original writer `0a57` stores 16-bit AX plus 8-bit DH every three
  bytes; Ghidra's `int*` widened the store and skipped the blue byte. Independent complete 178-flow
  native/WASM matrices passed for these bounded changes, with source and binary hashes under
  `scratch/verify/full-{611-614,scanout,no-skip,615}/`. Oracle trace:
  `scratch/sequence-capture/palbuf-writer2.flatwriters.txt`.
- The Original posts Extender op `0x6c` at 525.016 ms, reaches MGAVIDEO `04a3` at 535.011 ms after
  the next IRQ, then waits for the following IRQ to clear byte `0x786` at 546.916 ms. The port's gate
  had returned immediately and patch 070 had cleared the byte synchronously with a 16-bit store.
  The intro task yield plus patch 616's byte semaphore/IRQ wait makes frames 1–6
  byte-identical to the Original (all 64,000 indices and 768 palette bytes each). Frame 7 first
  differs in 16,000 pixels: Original 0x7120 writes the entire framebuffer at 553.103 ms between
  DOSBox's first and second VGA draw parts. DOSBox samples four 50-row parts per frame; the first
  part remains old. Port 0x7120 writes at 228.399 ms, 4.9 ms earlier relative to the 319.792-ms
  sequence offset, and the old sequence capture took one end-of-frame framebuffer snapshot.
  Frames 8–18 match again. Native/WASM match all 24 events, and the targeted two-flow check passes.
  The first full matrix failed ten Oracle-backed cockpit crops despite native/WASM equality;
  an intro-only gate condition did not remove those failures. INDIA2's failed PPM was entirely black
  although its indexed framebuffer has 51,239 nonzero pixels; its passing baseline has a full-color
  cockpit. At capture, the palette buffer holds entry 17 = 56,56,56, but DAC entry 17 is 0,0,0 and
  semaphore `0x786` is still 1. Forced `FIST_PALNOW` passes diagnostically; it is not a fix. The
  capture occurred before the pending IRQ upload. Waiting one VGA period for the real ISR, without
  forcing the DAC, restored all 23 op-`0x2c` cockpit flows on native/WASM. The capture now fails if
  the semaphore remains set. The exact-source complete 178-flow native/WASM matrix passed
  (45+45+44+44 unique, all exit 0; hashes under `scratch/verify/full-616irq/`).
  In INDIA2, input step 6 also sees tick 133 instead of 134. Evidence:
  `scratch/verify/full-616b/`, `scratch/verify/run.Gx6yMu/`,
  `scratch/verify/diag-palstate2/`, `scratch/verify/diag-waitirq/`, `scratch/verify/run.hKE2yL/`,
  `scratch/sequence-capture/c786-orig.watch.txt`,
  `aa10b-orig.watch.txt`, `filltrace2-original/dosbox.log`, `port-fillwatch-gdb.log`,
  `patch616b-{native,wasm}/`, `scratch/verify/run.gwtAev/`.
- Original KDV open→first DAC is 548.162→552.471 ms at fixed 30000 cycles/ms (4.309 ms);
  at 60000 it is 533.204→535.369 ms (2.165 ms). The approximately halved interval locates
  the gap in emulated CPU work before `746b`. Port open→DAC is 227.754→227.755 ms, with no
  virtual time charged for that work. Evidence: `scratch/sequence-capture/kdv-timing-{original,60k}/`,
  `patch616c-native/`. Reproducible `build_kdv_profile_oracle.sh` with
  `FIST_DOSBOX_CORE=normal FIST_KDV_PROFILE=<path> FIST_KDV_PROFILE_N=3` counts 102,357,
  61,827 and 61,417 instructions in `7135` for the first three frames, each over 4,000
  decoded cells; times are 3.5054, 2.0609 and 2.0472 ms. The branch mix changes per frame,
  so one fixed decoder delay is false. Evidence: `scratch/sequence-capture/kdv-profile-3calls.tsv`.
- In a diagnostic run, charging 5,134 PIT counts before the first KDV decode plus the trial IRQ wait
  and four-part scanout makes all 24 captured mode-13 frames byte-identical in indices and palette
  across Original/native/WASM. The Original-to-port presentation offset drifts from 319.792 to
  319.781 ms, so timing is still unequal. DOSBox retains its 14.268064-ms loader-mode vertical
  period when mode 13's computed 14.268123 ms differs by less than its 0.0001-ms rearm threshold;
  the port uses 14.268569 ms. That predicts 11.62 µs drift over 23 intervals. This is a measured
  first-frame probe, not a validated
  general decoder-cost model or a passing full matrix. Evidence:
  `scratch/sequence-capture/trial-cost5134-{native,wasm}/`.
- DOSBox uses four 50-row `VGA_DrawPart` samples per mode-13 presentation. The port now latches each
  part at its draw event and captures the palette at the final part; native/WASM produced 24 exactly
  matching events through tick 20. The first four Original 320x200 events remain byte-identical in
  content. Its exact-source 178-flow native/WASM matrix passed (45+45+44+44 unique, exit 0;
  source/binary hashes under `scratch/verify/full-partscan/`).
  Evidence: `scratch/sequence-capture/partscan20-{native,wasm}/`, `scratch/verify/run.A0MQ1c/`.
- Browser canvas may drop worker posts because it retains only `latest`. OPL/SB write separate WAVs;
  browser drains only OPL. Neither is final mixed PCM. Owners: 0026 and 0003.

## Next

1. Compare the post-IRQ cockpit capture phase to Original presentation. Model KDV decoder CPU work
   from its measured branch paths, not a first-frame constant.
   Make VGA phase fractional across status polls,
   retrace and draw parts; verify the complete event sequence and timestamps.
2. Capture the loader frames, establish one virtual scenario start/end and matched timed input.
   Compare complete frame order, dimensions, timestamps, indices and palettes without truncation.
3. Give the port one mixer owner (0003) and compare continuous PCM including SB effects. Add failing
   cases for dropped/reordered events, changed samples and incomplete captures.

## Accept

All three captures cover the same complete intro/menu scenario with independent Oracle provenance.
The runner reports the first differing event/time/byte/sample. A remaining difference keeps 0012,
0003 or 0026 open with its producer named.
