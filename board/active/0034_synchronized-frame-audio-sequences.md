Type: feature
Title: Original, native and WASM expose comparable complete frame and PCM sequences
Parent: 0012
Depends: 0033

## Contract

Under matched start state, timed input and devices, compare every presented indexed frame,
all 256 palette entries, presentation time and continuous mixed PCM over identical scenario bounds.
Report the first unequal event/byte/sample. Missing, truncated or masked output fails.

## Evidence

- Capture/comparison owners: `tools/oracle/capture_sequence.sh`,
  `tools/capture_port_sequence.sh`, `tools/oracle/compare_sequences.py`; strict format checks in 0033.
- Original fixed-30k full capture: 4,151 frames, 2,626,325 stereo samples, valid footers
  (`scratch/sequence-capture/kdv-profile-full-run/`). Shell/launcher prefix and attributed
  application-start fixture are distinguished in 0036; never drop unmatched records.
- PIC scanout schedules from stored float `srv_lag`, dispatches on the first eligible CPU cycle,
  and exposes a float tick fraction. The shared shim reproduces all 171 reference timestamps
  (`{port,wasm}-picfinal-686`). Global rational-clock replacement, mode-switch phase scaling
  and a one-cycle offset each broke earlier records. Do not revive those approximations.
- KDV open/decode/present order is `00,20,04,44,68,44,6c,70,78,64,78,…`; `0x70` opens
  `11cb`, `0x78` enters `11dd`. The old `0x64` open condition was false routing.
- Patch 617 meters asm `7135..746a`: `7+7×rows+2×headers`, plus cell costs
  `82×two-bit + 97×one-bit + 25×solid + 64×raw + 13×skip`, plus
  `2×two-bit-writes + one-bit-writes + raw-writes`. All 395 native/WASM counts match
  the pure Oracle decoder; seven callbacks include separate ISR work
  (`kdv-count-check-617/`). First decode: 102,357 instructions and 15 faults at 187 cycles each.
- DOSBox `INT 21h/3Fh` charges `4×read bytes`, capped by the remaining 1-ms slice.
  First three KDV reads are 768+5,000, 768+1,381, 768+1,671 bytes. Separate read/fault/ISR
  costs from cell execution. Traces: `kdv-stage-readcost.tsv`, `kdv-profile-*.tsv`,
  `kdv-exceptions*.txt` under `scratch/sequence-capture/`.
- Historical full 178-flow cross-target pass: `scratch/verify/full-kdv-instruction-meter/`.
  Frame boundary evidence is in 0036; missing PC-speaker/final mixed PCM is owned by 0003.
  Browser presentation drops belong to 0026. None of these subset proofs closes this contract.

## Next

1. Replace wall-clock/F9 capture termination with a matched emulated scenario endpoint. Current
   Oracle captures vary in suffix length; a common prefix is diagnostic, not full-run acceptance.
2. Reproduce the 0036 start fixture on all three targets; compare complete intro/menu records.
   Hand the first event-time discrepancy to 0026 and first mixed-PCM discrepancy to 0003.
3. Refine read-slice/fault/decoder timing only where a trace proves an intervening observable event.
   Extend synchronized coverage through the post-IRQ cockpit; retain the first unequal producer.

## Accept

All three complete intro/menu captures have independent Oracle provenance and identical frame,
palette, time and PCM sequences. Flight/audio/timing extensions remain with 0012/0003/0026.
