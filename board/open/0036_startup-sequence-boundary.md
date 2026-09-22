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
- SDL callback scheduling changed both PCM record sizes and sample content (first repeated-run difference:
  sample 18,862). Oracle sequence capture now uses DOSBox `nosound=true`, whose emulated 1 ms tick drives
  the mixer without a host audio thread. `oracle-nosound-{703,704}` match exactly over their common 4,283
  PCM records and 297 frames; external F9 delivery only changes the suffix length.

## Next

1. Emit native and WASM mixed PCM sequence records from the same start state and sample clock. Include
   silence and use deterministic sample-position blocks; do not substitute the OPL-only or SB-only rings.
2. Compare the complete common PCM prefix against `oracle-nosound-703`. Report the first unequal sample
   or boundary, trace its producer and fix that contract.
3. Keep title-frame timing work in 0034 and this item limited to the application boundary.

## Accept

The common start state yields identical complete records in all three runs, including timestamps
and PCM. `0033` coverage passes on both targets.
