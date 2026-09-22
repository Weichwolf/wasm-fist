Type: feature
Title: Original and ports share a proven application-start presentation boundary
Parent: 0034

## Contract

From one attributed DAT-entry state, DOSBox, native and WASM emit the same indexed frames,
256-entry palettes, presentation timestamps and PCM. The comparison includes the prefix; it
never trims an unmatched emulator frame.

## Proven state

- `loader-vga-probe/oracle-pre-dat.{text,bda}` is the Oracle pre-DAT text VRAM/BDA state:
  mode 3, cursor `(23,0)`. `FIST_TEXT_STATE` plus
  `FIST_TEXT_PHASE_NS=20960467,20168067` selects it in a port run.
- Oracle `start-state-probe/sequence.frames` begins at 32,879 us. Native and WASM matched
  every frame byte, palette and timestamp through event 34 before this change.
- The engine calls `MGAVIDEO:00e8` at `4ec3:0132`, which invokes the second `2fd3` calibration
  after mode 13. DOSBox retains the loader's 9-dot VGA phase through that mode switch.
- `re_out/fist_vga.c` now retains `g_text_vertical_num` after the mode switch. Anchored trace
  `port-initcal-phase-trace-649/port.log` measures its three `30de` high edges at
  418.498603, 432.766334 and 447.034903 ms; Oracle measures 418.499233, 432.767300 and
  447.035400 ms. The preceding model returned 412.613499 ms for the first edge.
- The correction closes the former event-35 palette failure. `start-state-probe` and
  `port-initcal-phase-trace-649` are byte/time-identical through event 35. The first Oracle
  difference is event 36: 560,798 us versus 560,797 us; pixel 0 differs.
- Clean captures `port-phase-clean-650` and `wasm-phase-clean-650` match native/WASM exactly
  through event 67. Event 68 differs only in timestamp: native 1,017,375 us, WASM 1,017,376 us.
  This violates the contract; do not call the targets identical.
- `make check` passes at this revision. The clean native and WASM builds both complete.

## Next

1. Capture event 36 in DOSBox and both ports with the state and phase variables above. Identify
   its producer and recover the register/segment/width/flag contract. Fix the first byte before
   examining later frames.
2. Explain the native/WASM one-microsecond event-68 discrepancy from the sequence timestamp
   conversion. Add a regression that compares the full frame record, including time; do not round
   the comparison or the capture format.
3. Recover the remaining SOUNDDVR CPU-slice I/O timing: calibration must start at 90 and evolve
   as DOSBox records. Then capture continuous mixed PCM.

## Accept

One reproducible command captures complete frame and PCM streams from the common state; DOSBox,
native and WASM files compare exactly. The evidence identifies every excluded frame and proves its
producer. `0033` coverage passes on both targets.
