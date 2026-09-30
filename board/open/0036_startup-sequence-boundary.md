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
- The shared sequence writer now emits canonical 512-frame PCM records, timestamps each from exact sample
  position/rate and flushes one final partial record. `oracle-pcm-blocks-{707,708}` match completely:
  157 frames plus 101,827 stereo samples in 199 PCM records. Callback batching is no longer observable.
- A temporary per-channel DOSBox mixer probe on the same boundary identified the first nonzero sample:
  stereo sample 18,887 (428,276 us), channel `SPKR`, delta -7,045,120 on both sides. The port has no
  PC-speaker PCM producer. Its first audio mismatch therefore precedes OPL and SB mixing.
- `FIST_SPEAKER_TRACE=1` now records PIT-2 reloads, speaker mode transitions and the first original
  mono sample. The versioned `speaker_trace.patch` is applied/hash-checked by the sequence Oracle builder;
  port I/O emits reload/type diagnostics under the same switch. `oracle-speaker-711` records 27 reloads
  and 54 transitions from SOUNDDVR `07b7/07f4`: the first reload is count 512, mode 3 at 404.277133346 ms,
  with speaker state/volume zero. The first callback at 405 ms emits -860 at offset 12 of 44 samples.
- DOSBox `SetType(0/1)` enqueues -5000 even with the speaker output disabled. Its integrated ramp produces
  the first nonzero PCM; the calibration is not silent. Default mixer prebuffer is 25 ms: initial 1103
  samples plus `floor(403 * 722534 / 16384)` samples through tick 404 plus callback offset 12 = 18,887.
  The Q14 increment is `floor((44100 << 14)/1000)`. Preserve this initial fill and remainder contract.
- `port-speaker-714` and `wasm-speaker-713` match all 81 diagnostic events and all 229 complete frame
  records. Their first reload is 404.283671728 ms, 6.538382 us later than the Oracle. These diagnostics
  establish port parity and identify timing work, not original PCM equality. Oracle 711's first 101,827
  samples equal capture 707's complete PCM payload; the remaining Oracle suffix was not compared.

## Next

1. Use the speaker trace to recover instruction/I/O costs at SOUNDDVR `07b7/07f4`; fix the first event
  mismatch before matching later samples. Recover DOSBox `ForwardPIT`, queued transitions and callback
   ramp integration from `src/hardware/pcspeaker.cpp`; do not fit a waveform or a start offset.
2. Add one final stereo mixer and sequence owner for PC speaker, OPL and SB. Include silence and emit
   deterministic sample-position blocks on native and WASM from the shared PIT clock.
3. Compare the complete common PCM prefix against the Oracle. Report the first unequal sample or
   boundary and fix its producer contract.
4. Keep title-frame timing work in 0034 and this item limited to the application boundary.

Reproduce: build with `bash tools/oracle/build_sequence_oracle.sh`; export `FIST_SPEAKER_TRACE=1`,
`FIST_DOSBOX_CORE=normal`, `FIST_DOSBOX_CYCLES='fixed 30000'`; run
`bash tools/oracle/capture_sequence.sh 4 <fresh-dir>`. For ports set the start fixture/phase above,
`FIST_SPEAKER_TRACE=1 FIST_OPL=1 FIST_SB=1`; run `bash tools/capture_port_sequence.sh <target> 170 <fresh-dir>`.
Diagnostic change verification: `bash tools/check_flow.sh '^(intro|audio-opl-init)$'` passes both flows
on both targets, all 29 tool tests and the patch check (`scratch/verify/run.I04VHg/`). This is partial
matrix evidence; the full original PCM contract remains open.

## Accept

The common start state yields identical complete records in all three runs, including timestamps
and PCM. `0033` coverage passes on both targets.
