Type: feature
Title: Original and ports share a proven application-start presentation boundary
Parent: 0034

## Contract

Establish one original-attributed DAT-entry state and frame/PCM origin on all three targets.
Retain the prefix and timestamps. This item owns the start fixture; 0026 owns device time,
0003 owns audio synthesis/mixing, and 0034 owns complete capture endpoints and comparison.

## Evidence

- Start fixture: `FIST_TEXT_STATE=scratch/sequence-capture/loader-vga-probe/oracle-pre-dat`,
  `FIST_TEXT_PHASE_NS=20960467,20168067`. The versioned `sequence_start_state.patch`
  restores recorded B800/BDA at `FIST.RUN` entry; fixture hashes are checked. It retains the
  original VGA/PIC queue, cursor, scanout and time. KDV title timing belongs to 0034.
- All 227 complete Oracle records in `start-state-probe/` match native/WASM indices, full
  palette, dimensions and time (`{port,wasm}-picfinal-688`). Both ports continue two frames;
  external Oracle termination explains length, not suffix equality.
- Fixed-30k `oracle-start-state-{700,701}` match all 297 frames and the earlier 227-record
  prefix. Capture 702 matches 296 records; F9 delivery changes the suffix length.
- Oracle PCM uses DOSBox `nosound=true`: emulated 1-ms mixer ticks replace host SDL scheduling.
  Canonical 512-frame records use sample-position timestamps and a final partial record.
  `oracle-pcm-blocks-{707,708}` match completely: 157 frames, 101,827 stereo samples.
- First original nonzero stereo sample is 18,887 at 428,276 us, source `SPKR`.
  The first port counter is 6.538382 us late. Channel synthesis and timing evidence now live
  in 0003/0026; changing start phase cannot repair either producer.
- Versioned `speaker_trace.patch` and port `FIST_SPEAKER_TRACE=1` diagnose counter/type
  events. At `65d42dc`, `port-speaker-714`/`wasm-speaker-713` match all 81 events and
  all 229 frames; `oracle-speaker-711` matches capture 707's complete PCM payload prefix.
  These are scoped diagnostics, not complete original PCM equality.
- Fresh `port-speaker-724`/`wasm-speaker-725` retain all 229 equal frames and 81 equal speaker
  events after port isolation. `oracle-speaker-726` retains the first counter at 404.277133346 ms;
  external termination gives 226 frames. Its shorter suffix does not satisfy 0034 acceptance.

## Next

1. Reproduce the fixture after timing/mixer changes; include the first frame and sample.
2. Use 0034's matched scenario endpoint for complete captures. Investigate boundary differences
   here; route device-clock and mixed-output defects to their owners.
3. Keep evidence to the current boundary result; retain superseded trials in Git.

Reproduce: `bash tools/oracle/build_sequence_oracle.sh`; use `FIST_DOSBOX_CORE=normal`,
`FIST_DOSBOX_CYCLES='fixed 30000'`, `FIST_SPEAKER_TRACE=1` with
`bash tools/oracle/capture_sequence.sh 4 <fresh-dir>`. For ports set the fixture/phase above and
`FIST_SPEAKER_TRACE=1 FIST_OPL=1 FIST_SB=1`;
`bash tools/capture_port_sequence.sh <native|wasm> 170 <fresh-dir>`.

## Accept

The fixture is original-attributed and verified on both targets, with matched first presented
frame/palette/time and mixer origin/initial fill. 0033 coverage passes. Complete frame/PCM sequence
equality remains mandatory in 0034; proving this boundary does not close that parent.
