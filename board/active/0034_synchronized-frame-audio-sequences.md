Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

Capture every presented indexed frame, all 256 palette entries, presentation time and continuous
mixed PCM across matched scenario boundaries, input and devices. Report the first unequal event,
byte or sample. Missing or incomplete output fails; no masks or truncated comparisons.

## Evidence

- The fixed-30000-cycle DOSBox Oracle captured 3,448 presented frames and 2,182,440 stereo
  samples with valid footers (`scratch/sequence-capture/fixed30k-full/`). Mode-13 presentation
  starts at 475.189 ms. DOSBox retains its 14.268064-ms loader-mode vertical period after the
  mode switch because the 0.000059-ms difference is below its 0.0001-ms rearm threshold.
- The port captures four 50-row VGA draw parts at their sampling times and the palette at the
  final part. Native/WASM produce 24 byte-identical indexed events through tick 20; the first
  six mode-13 frame contents also match the Original (`scratch/sequence-capture/patch616irq-*/`).
  The port starts at 155.397 ms, omits the preceding 640x400 loader frames and mixed PCM, and
  has no common scenario start marker. All 178 existing native/WASM flows pass with hashes and
  exact coverage in `scratch/verify/full-616irq/`.
- Original intro op `0x6c` crosses the next IRQ before MGAVIDEO `04a3`; `04a3` requests a palette
  upload via byte `0x786` and waits for the following ISR. Patch 616 restores that wait. The
  op-`0x2c` cockpit dump previously captured before the IRQ with DAC black, palette buffer valid
  and semaphore set. Waiting one VGA period for the real ISR, then rejecting a pending semaphore,
  passes all 23 cockpit flows on both targets without forcing the DAC. Evidence:
  `scratch/sequence-capture/c786-orig.watch.txt`, `scratch/verify/run.hKE2yL/`.
- First content difference is mode-13 frame 7: Original KDV writes all 64,000 indices between
  DOSBox's first and second draw parts, leaving 50 old rows; the port writes before the first
  draw part. A diagnostic 5,134-PIT-count first-decode charge makes all 24 frame contents and
  palettes equal across all three runs, but it is not a valid decoder model and presentation
  time still drifts by 11 µs over 23 intervals (`scratch/sequence-capture/trial-cost5134-*/`).
- DOSBox `7135` decodes 4,000 cells per KDV frame. The first three calls execute 102,357,
  61,827 and 61,417 instructions and take 3.5054, 2.0609 and 2.0472 ms. Branch-dependent
  work, not a fixed first-frame delay, must advance the port clock. Reproduce with
  `tools/oracle/build_kdv_profile_oracle.sh`, `FIST_DOSBOX_CORE=normal`,
  `FIST_KDV_PROFILE=<path>` and `FIST_KDV_PROFILE_N=3`; trace:
  `scratch/sequence-capture/kdv-profile-3calls.tsv`.
- Browser canvas may drop worker posts because it retains only `latest`. OPL/SB write separate
  WAVs; browser drains only OPL. Neither is final mixed PCM (owners 0026 and 0003).

## Next

1. Model the original KDV decoder's branch-dependent CPU work and the inherited fractional VGA
   phase; compare complete frame contents and timestamps through the first differing event.
   Verify the post-IRQ cockpit capture against the Original presentation phase.
2. Capture loader frames. Establish identical virtual scenario start/end and timed input; compare
   complete frame order, dimensions, timestamps, indices and palettes without truncation.
3. Route OPL and SB through one continuous mixer (0003). Compare every PCM sample and fail on
   missing, reordered or unfinished frame/audio output.

## Accept

All three captures cover the same complete intro/menu scenario with independent Oracle provenance.
The runner reports the first differing event, time, byte or sample. Remaining parity, audio or
presentation differences keep 0012, 0003 or 0026 open with their producer named.
