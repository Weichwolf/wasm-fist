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
- A new five-second fixed-30k Oracle run reaches 297 frames but differs in its first shell frame
  (`oracle-start-boundary-689/`). The historic matching capture was made with a temporary
  instrumented start-state hook that has been restored out of DOSBox. A normal launcher run is
  therefore not a continuation of `oracle-pre-dat`; do not use it as a reference for this item.
- A cursor-count override changes part of the mismatch but not all of it (`oracle-cursor{0,15b}-*`).
  The loader text surface itself is not pinned. Reject cursor/blink forcing: it masks a changing
  producer and cannot establish the attributed application boundary.

## Next

1. Restore a minimal, versioned Oracle hook at the attributed DAT transition. It must record or
   restore the complete required start state, open the sequence there, and prove that two runs
   have identical first records. Then extend the reference beyond 229 port frames and report the
   first unequal record.
2. Capture and compare the corresponding PCM prefix. Preserve timestamps; no rounding or prefix
   trim.
3. Keep title-frame timing work in 0034 and this item limited to the application boundary.

## Accept

The common start state yields identical complete records in all three runs, including timestamps
and PCM. `0033` coverage passes on both targets.
