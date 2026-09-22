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
- The phase fix retains the loader's VGA phase through the mode-13 switch. Native and WASM now
  match the Oracle through event 35. Event 36 is the first difference: 560,798 us versus
  560,797 us. Its frame data now matches; only the timestamp differs.
- The differing title-frame writer is KDV. Its timing and gate reconstruction belong to 0034;
  do not alter startup phase to hide it.
- Native and WASM agree through event 67. Event 68 differs by one microsecond. The timestamp is
  part of the contract.

## Next

1. Keep the shared start state and phase inputs fixed. Run the full comparator after each timing
   change and report the first unequal record.
2. Trace the native/WASM event-68 conversion. Preserve microseconds; no rounding or prefix trim.
3. Move title-frame timing work to 0034. Keep this item limited to the application boundary.

## Accept

The common start state yields identical complete records in all three runs, including timestamps
and PCM. `0033` coverage passes on both targets.
