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
  Native 743/WASM 744 match each other and all original times. Restoring the exact queued PIC
  origin (0036) fixes six 1-us errors, first event 362 at 5,212,186 versus 5,212,187 us.
  Original content parity still fails: event 405 differs in palette, 443/648/662 in pixels.
  Attribution: `scratch/sequence-capture/pic-dispatch-362/attribution-final.json`.
  The last visual frame is still the helicopter intro; this is not a complete intro/menu proof.
- PCM count follows the original mixer: `1103 + floor((end_ms−1)×722534/16384)`, yielding
  133,358/207,005/442,058 samples at 3000/4670/10000 ms. Ports still lack final mixed sequence PCM
  (0003); the complete comparator correctly fails the missing file. `--frames-only` is diagnostic.
- PIC scanout schedules from stored float `srv_lag` and exposes a float tick fraction.
  GDB proves event 362 dispatches at cycle 5595; the old rounded origin produced 5596.
  Source-derived fixture and reproduction: `tools/oracle/start_state.{vga,pic.gdb}` (0036).
  Global rational-clock replacement, mode-switch phase scaling and a one-cycle offset broke earlier records.
- KDV gate order is `00,20,04,44,68,44,6c,70,78,64,78,…`: `0x70` opens `11cb`, `0x78`
  enters `11dd`. Patch 617 meters asm `7135..746a`; all 395 native/WASM counts match the pure
  Oracle decoder (`kdv-count-check-617/`), seven callbacks include separate ISR work.
  Formula: `7+7×rows+2×headers +82×two-bit +97×one-bit +25×solid +64×raw +13×skip`,
  plus `2×two-bit-writes +one-bit-writes +raw-writes`. First decode: 102,357 instructions;
  15 first-touch faults cost 187 cycles each. DOS `3Fh` read cost is `4×bytes`, capped by the 1-ms slice.
- Boundary/fixture tests fail at their unmodified bases on both targets and pass after each fix. All 37 tool tests
  pass; `bash tools/check_flow.sh '^(intro|mainmenu|audio-opl-init)$'` passes its three flows on
  both targets (`scratch/verify/run.ksO3t0/`). This is partial matrix evidence.
  Historical full 178-flow proof: `full-kdv-instruction-meter/`.

## Next

1. Trace the original palette upload at event 405, then decoder/framebuffer writes at 443/648/662.
   Recover intervening instruction/read/fault/ISR costs; do not adjust capture phase or mask fields.
2. Extend the matched endpoint through the first stable original menu, then timed inputs. Compare
   every record; retain the first unequal producer. Reuse the versioned 0036 fixture.
3. Implement the final mixer in 0003 on 0026's shared device time; require complete mixed stereo PCM.

## Accept

All three complete intro/menu captures have independent Oracle provenance and identical frame,
palette, time and PCM sequences. Flight/audio/timing extensions remain with 0012/0003/0026.
