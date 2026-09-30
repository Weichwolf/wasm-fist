Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

Under matched start state, timed input and devices, compare every presented indexed frame,
all 256 palette entries, presentation time and continuous mixed PCM over identical scenario bounds.
Report the first unequal event/byte/sample. Missing, truncated or masked output fails.

## Evidence

- Capture/comparison owners: `capture_sequence.sh`, `capture_port_sequence.sh`,
  `sequence_format.py`, `compare_sequences.py` under `tools/` or `tools/oracle/`; format checks 0033.
- `FIST_SEQUENCE_END_MS` now selects a positive uint32 endpoint on the emulated timeline.
  DOSBox stops after that tick's mixer handlers and before the next CPU/queued presentation phase;
  ports stop on the shared clock. `sequence.end` proves the endpoint was reached. Validation rejects
  missing/mismatched markers, including an early `FIST_RUNMS=1` exit with a valid frame footer.
  Original durations are now emulated seconds; default core/cycles are normal/fixed 30000.
- Complete 3000-ms original runs `oracle-endpoint-{727,728}`: identical 207 frames and 133,358
  stereo samples. Native 729 and WASM 730 match all frame records, palettes, indices and times.
  At 4670 ms, original 735/native 736 both contain 324 equal frames; the frame at 4670 ms is excluded.
- Complete 10000-ms original runs 731/734/742 match all 698 frames and 442,058 samples.
  Native 762/WASM 763 match each other. The exact queued PIC origin (0036) fixes six 1-us errors;
  original PIT latch rounding (0026) fixes palette event 405 and pixel events 648/662.
  All 698 palettes/times now match; only event 443 pixels differ, first byte 32004: 13 versus 14.
  Attribution: `scratch/sequence-capture/pit-calibration/attribution.json`.
  Last frame is still the helicopter intro; full intro/menu acceptance remains unproved.
- At event 443 (6,367,900 us), 8,674 pixels differ exclusively in rows 100–149.
  Original framebuffer copy 7120 starts at 6365.540100 ms, after the third scanout band;
  port starts at 6364.404592 ms, before it. Callback 88's pure decoder count matches.
  Original pre-decoder DOS reads consume CPU slice time absent from the port (0026).
  GDB captures `oracle-read-cycles-766`/`oracle-pre-88-767` retain every original 742 frame/PCM byte.
- PCM count follows the original mixer: `1103 + floor((end_ms−1)×722534/16384)`, yielding
  133,358/207,005/442,058 samples at 3000/4670/10000 ms. Ports still lack final mixed sequence PCM
  (0003); the complete comparator correctly fails the missing file. `--frames-only` is diagnostic.
- PIC scanout schedules from stored float `srv_lag` and exposes a float tick fraction.
  GDB proves event 362 dispatches at cycle 5595; the old rounded origin produced 5596.
  Source-derived fixture and reproduction: `tools/oracle/start_state.{vga,pic.gdb}` (0036).
  Global rational-clock replacement, mode-switch phase scaling and a one-cycle offset broke earlier records.
- Patch 617 meters asm `7135..746a`; all 395 native/WASM counts match the pure Oracle decoder
  (`kdv-count-check-617/`), seven callbacks include separate ISR work. Formula and fault costs:
  `tools/check_kdv_instruction_formula.py`, patch 617. DOS transfer/slice timing belongs to 0026.
- Original stage probe now accepts `FIST_KDV_STAGE_N` (default three hits/IP); use 395 with
  `FIST_KDV_STAGE=<file>` for later callbacks. Build hash-checks the CPU source/patch.
  Original 748 with expanded tracing retains all 742 frame/PCM bytes; a one-hit capture retains
  all original 1000-ms bytes. Invalid limits fail; the producer never reports a reached endpoint.
- PIT regression compares actual original `timer.cpp:counter_latch` in 70 cases on each target;
  unmodified port fails 80 subtests. All 38 tool tests pass with the correction.
  `bash tools/check_flow.sh` passes all 178 existing flows on both targets on base `68bf531`
  plus the PIT patch (`scratch/verify/run.lwhih4/`); pinned binary/script hashes still match.
  This accepts the bounded PIT correction, not complete original frame/audio parity.

## Next

1. Recover callback 88's file-transfer packetization, CPU slice boundaries and surrounding
   instructions in 0026. Fix its premature copy and compare every 10000-ms record on both targets.
   Do not adjust capture phase, inject a fitted delay or mask fields.
2. Extend the matched endpoint through the first stable original menu, then timed inputs. Compare
   every record; retain the first unequal producer. Reuse the versioned 0036 fixture.
3. Implement the final mixer in 0003 on 0026's shared device time; require complete mixed stereo PCM.

## Accept

All three complete intro/menu captures have independent Oracle provenance and identical frame,
palette, time and PCM sequences. Flight/audio/timing extensions remain with 0012/0003/0026.
