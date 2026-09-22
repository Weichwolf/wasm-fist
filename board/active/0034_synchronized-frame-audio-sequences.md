Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

For matched initial state, timed input and devices, compare every presented indexed frame, all
256 palette entries, presentation time and continuous mixed PCM across the same scenario bounds.
Report the first unequal event, byte or sample. Missing, truncated or masked output fails.

## Evidence

- The fixed-30000-cycle Oracle has 4,151 frames and 2,626,325 stereo samples with valid footers
  (`scratch/sequence-capture/kdv-profile-full-run/`). Its first 30 frames are 640×400 DOSBox
  shell/launcher output; frame 26 includes Armored Fist's copyright text, frames 27–29 are black.
  Mode 13 starts at 475.189 ms after one skipped 640×400 presentation. DOSBox retains that
  mode's period `100×449/(28322000/9 truncated)
  = 14.268064195 ms` and schedules events with float PIC indices. The port's fractional
  scanout now spans 328,165 µs over 24 frames versus Oracle 328,166 µs; native/WASM contents
  match each other and the first six Oracle mode-13 frames (`fractional-*/`). Absolute start,
  startup capture boundary/launcher presentation, phase/long-run rounding and mixed PCM remain
  unmatched (0036).
- First content difference: mode-13 frame 7. Original KDV fills the framebuffer between the
  first and second 50-row draw parts; the port fills it earlier. Original present→decode costs
  1,042/331/380 PIT counts for frames 1–3, decoder→DAC 4,183/2,459/2,443, DAC→fill
  753–757; the port charges zero for the first two stages and 769 for the last. The first
  decode executes 102,357 instructions plus 15 first-touch faults at 187 cycles each.
  Use `FIST_DOSBOX_CORE=normal` for comparable timing. Stage/profile traces:
  `scratch/sequence-capture/kdv-stage-readcost.tsv`,
  `kdv-profile-{3calls,60k,full}.tsv`, `kdv-exceptions*.txt`.
- DOSBox `INT 21h/3Fh` charges `4×read bytes`, capped by the remaining 1-ms CPU slice
  (`src/dos/dos.cpp:modify_cycles`). First three KDV calls read 768+5,000, 768+1,381 and
  768+1,671 bytes. A reverted uncapped read-cost trial plus the measured first decoder cost
  matches all 24 Oracle frame contents/palettes, proving the producer of frame 7 but not a
  general timing model (`scratch/sequence-capture/readcost-decoder-gdb/`).
- Assembly `7135..746a` yields an exact instruction formula: `7+7×rows+2×headers` plus
  `82×two-bit + 97×one-bit + 25×solid + 64×raw + 13×skip` per cell, plus
  `2×two-bit-writes + one-bit-writes + raw-writes`. Patch 617 counts these predicates while
  decoding, without charging time. `bash tools/check_kdv_counts.sh
  scratch/sequence-capture/kdv-profile-full.tsv <fresh-out>` proves all 395 native and WASM
  counts match the pure Oracle decoder; seven calls include interleaved ISR work
  (`scratch/sequence-capture/kdv-count-check-617/`). Keep file reads, faults and ISR cost
  separate from the decoder formula. All 178 existing flows pass both targets with exact
  coverage and binary hashes (`scratch/verify/full-kdv-instruction-meter/`).
- Patch 616 restores the MGAVIDEO palette IRQ wait; all 23 cockpit flows pass both targets
  (`scratch/verify/run.hKE2yL/`). The strict frame/PCM comparator is
  `tools/oracle/compare_sequences.py`; its ordering/completion test passes. Browser canvas
  drops superseded worker posts, and OPL/SB are not yet one mixed PCM stream (0026/0003).

## Next

1. Reproduce DOSBox's read-cycle slice cap in the port clock. Apply decoder cost during cell
   execution so PIT/ISR events interleave; model first-touch faults. Compare complete frame
   contents and timestamps through the first difference, including the post-IRQ cockpit.
2. Resolve the startup capture boundary and launcher output in 0036. Align VGA phase, timed
   inputs and float PIC event rounding. Compare every frame's order, size, time, indices and
   palette without silently dropping a prefix.
3. Route OPL and SB through one continuous mixer (0003); compare every PCM sample and fail on
   missing, reordered or unfinished output.

## Accept

All three complete intro/menu captures have independent Oracle provenance and identical frame,
palette, time and PCM sequences. Remaining flight, audio or timing differences stay with
0012, 0003 or 0026 and name their first differing producer.
