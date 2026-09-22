Type: feature
Title: Original and ports share a proven application-start presentation boundary
Parent: 0034

## Contract

From one attributed DAT-entry state, DOSBox, native and WASM emit the same indexed frames,
256-entry palettes, presentation timestamps and PCM. The comparison includes the prefix; it
never trims an unmatched emulator frame.

## Facts

- Use `FIST_TEXT_STATE=scratch/sequence-capture/loader-vga-probe/oracle-pre-dat` and
  `FIST_TEXT_PHASE_NS=20960467,20168067`. This selects the attributed pre-DAT mode-3 state.
- The phase fix retains the loader's VGA phase through the mode-13 switch. With the fixed
  30,000-cycles/ms Oracle, all 227 complete records in `start-state-probe/` match native and
  WASM exactly in time, dimensions, palette and pixels (`{port,wasm}-picfinal-688`). Both ports
  continue for two more frames because the Oracle capture ends; that is a capture bound, not a
  matching verdict for the suffix.
- KDV owns the title-frame timing; its gate and PIC scanout reconstruction belong to 0034. Do not
  alter startup phase to hide a later producer defect.
- Native and WASM match all 229 captured frame records. The timestamp is part of the contract.
- `tools/oracle/sequence_start_state.patch` restores the recorded B800/BDA fixture when DOSBox enters
  `FIST.RUN`; `capture_sequence.sh` decodes and verifies the versioned fixture. It does not alter the
  VGA/PIC queue, cursor, scanout or timestamps. `build_sequence_oracle.sh` applies and verifies the hook.
- Fixed-30k captures `oracle-start-state-{700,701}` are identical over all 297 frames. Their first 227
  records match `start-state-probe/` exactly in timestamp, dimensions, palette and pixels; both extend
  70 records beyond that fixture. A third runner-driven capture (`702`) matches its complete 296-record
  prefix; external F9 delivery accounts for the one-record length difference.
- PCM capture is not deterministic yet. Against the historic stream, Oracle event 99 starts at the same
  sample position 5331 but contains 45 frames instead of 44. Repeated runs also end with different sample
  counts. Mixer callback batching is observable in the file and must be normalized at the producer.

## Next

1. Emit Oracle PCM in deterministic sample-position blocks independent of SDL mixer callback sizes.
   Preserve every sample and derive each timestamp from its sample position; do not trim or round a prefix.
2. Compare the complete common PCM prefix across two Oracle runs, then against native and WASM from the
   same start state. Report the first unequal sample or boundary.
3. Keep title-frame timing work in 0034 and this item limited to the application boundary.

## Accept

The common start state yields identical complete records in all three runs, including timestamps
and PCM. `0033` coverage passes on both targets.
