Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

Capture every presented indexed frame, all 256 palette entries, presentation time and continuous
mixed PCM across matched scenario boundaries, input and devices. Report the first unequal event,
byte or sample. Missing or incomplete output fails; no masks or truncated comparisons.

## Evidence

- The fixed-30000-cycle DOSBox Oracle captured 4,151 presented frames and 2,626,325 stereo
  samples with valid footers (`scratch/sequence-capture/kdv-profile-full-run/`). The earlier
  `fixed30k-full` artifact contains only 258 frames and 168,864 samples. Mode-13 presentation
  starts at 475.189 ms; the mode switch skips one 14.268-ms presentation. DOSBox retains the
  loader-mode period `100×449/(28322000/9 truncated) = 14.268064195 ms` because the mode-13
  period differs by less than its 0.0001-ms rearm threshold. Across 4,149 uninterrupted
  intervals the Oracle emits 3,881 × 14,268 µs and 268 × 14,269 µs. The port's fixed 17,025
  PIT counts give 14.268569 ms, explaining its roughly 11-µs drift over 23 intervals. DOSBox
  queues VGA events with `float` PIC indices; the exact long-run timestamp sequence still needs
  that rounding and the inherited phase, beyond the rational hardware period.
- The port captures four 50-row VGA draw parts at their sampling times and the palette at the
  final part. The scanout schedule now retains the loader's fractional VGA period: native/WASM
  produce 24 byte-identical events over 328,165 µs versus the Oracle's 328,166 µs; the former
  17,025-count schedule took 328,177 µs. The first six mode-13 frame contents match the Original
  (`scratch/sequence-capture/fractional-*/`, `patch616irq-*/`).
  The port starts at 155.397 ms, omits the preceding 640x400 loader frames and mixed PCM, and
  has no common scenario start marker. All 178 existing native/WASM flows pass after the
  fractional scheduler change, with hashes and exact disjoint coverage in
  `scratch/verify/full-fractional/`.
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
  work must advance the port clock. The first call also faults on 15 new framebuffer pages:
  `(102357 + 15×187)/30000 = 3.5054 ms`; the 60000-cycle run independently gives
  1.7527 ms for the same counts and faults. Later calls have no such faults. A full title run
  has 395 calls, 298 distinct instruction counts (54,357–317,270), confirming variable cost.
  The measured first-call cost converts to 4,183 PIT counts, below the diagnostic 5,134-count
  charge; that trial cannot establish the cost of work surrounding the decoder.
  A same-core stage trace places first KDV open at 548.110 ms, file access at 548.170,
  present at 548.179, decode at 549.053–552.558, DAC at 552.559 and framebuffer fill at
  553.189. A GDB trace of the port at `scratch/sequence-capture/gdb-kdv-stage/trace.log`
  puts `11dd`, `7135` and `746b` on the same PIT count. Original present→decode takes
  1,042/331/380 PIT counts for the first three frames and decode→DAC takes 4,183/2,459/2,443;
  the port omits both variable costs. DAC→fill is already close: 769 port counts versus
  753–757 Original counts. Model pre-decode, decoder and post-DAC costs
  separately; a 5,134-count lump only crosses one scanout boundary.
  Uninstrumented `core=normal` and prior `core=auto` have equal mode-13 contents/timestamps
  across 197 common events, but their subframe KDV timing differs. The stage hook itself shifts
  event 51 by 1 µs; use it to attribute stages, not as a timing reference. Keep core and hooks
  fixed for comparisons. Reproduce with `tools/oracle/build_kdv_{profile,stage}_oracle.sh`,
  `FIST_DOSBOX_CORE=normal`, `FIST_KDV_STAGE=<path>`,
  `FIST_KDV_PROFILE=<path>` and `FIST_KDV_PROFILE_N=3`; trace:
  `scratch/sequence-capture/kdv-stage-points.tsv`, `kdv-profile-{3calls,60k,full}.tsv`
  and `kdv-exceptions*.txt`. Both instrumented builders round-trip; `bash tools/check_flow.sh
  '^intro$'` passes native/WASM at this revision (`scratch/verify/run.N4pOhl/`).
- Browser canvas may drop worker posts because it retains only `latest`. OPL/SB write separate
  WAVs; browser drains only OPL. Neither is final mixed PCM (owners 0026 and 0003).
- `tools/oracle/compare_sequences.py` validates complete frame/PCM streams and reports the
  chronologically first unequal time, layout, palette byte, pixel byte or PCM byte. It confirms
  the 24 native/WASM frames match; full comparison fails because the port has no PCM stream.
  `python3 -m unittest tests/test_compare_sequences.py` covers ordering and invalid footers.

## Next

1. Model the missing present→decode and decode→DAC costs from the original paths, including
   branch-dependent decoder instructions and first-touch faults. Compare full frame contents
   and timestamps through the first difference; align inherited VGA phase and loader frames.
   Verify the post-IRQ cockpit capture against the Original presentation phase.
2. Capture loader frames. Establish identical virtual scenario start/end and timed input; compare
   complete frame order, dimensions, timestamps, indices and palettes without truncation.
3. Route OPL and SB through one continuous mixer (0003). Compare every PCM sample and fail on
   missing, reordered or unfinished frame/audio output.

## Accept

All three captures cover the same complete intro/menu scenario with independent Oracle provenance.
The runner reports the first differing event, time, byte or sample. Remaining parity, audio or
presentation differences keep 0012, 0003 or 0026 open with their producer named.
