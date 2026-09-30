Type: feature
Title: Original and ports share a proven application-start presentation boundary
Parent: 0034

## Contract

Own one original-attributed DAT-entry fixture and frame/PCM origin. Preserve the prefix and times.
Device time belongs to 0026, mixed output to 0003, complete capture/comparison to 0034.

## Evidence

- `capture_sequence.sh` materializes and hash-checks `.text`, `.bda`, `.vga` from versioned assets
  under `tools/oracle/start_state.*`. `sequence_start_state.patch` restores B800/BDA at `FIST.RUN`.
  Set port `FIST_TEXT_STATE=<oracle-capture>/start-state`; no separate phase environment is needed.
- `.vga` distinguishes measured clock/scanout phases (20,960,467/20,168,067 ns) from queued PIC
  origin: tick 20, float lag `0x3e2c1900` = `0x1.5832p-3` = 0.168064117431640625 ms.
  Original queue: `float(0.9000000953674316 + float(14.2680641955)) − 15` gives this residual.
  GDB probe: `tools/oracle/start_state_pic.gdb`; log `oracle-pic-start-741/dosbox.log` under
  `scratch/sequence-capture/`. Dispatch cycle 5042 is later than the queued origin; conflating them
  caused six 1-us presentation errors. Missing/malformed `.vga` now fails on both targets.
- Original 742 retains complete 731/734 frame/PCM bytes. Native 743/WASM 744 match every one of
  its 698 presentation times and each other; four content differences remain in 0034.
  Proof: `scratch/sequence-capture/pic-dispatch-362/`, filtered flows `scratch/verify/run.ksO3t0/`.
- Audio origin remains unresolved: original first nonzero stereo sample 18,887 at 428,276 us
  comes from `SPKR`; the port's first counter is 6.538382 us late. Continue in 0003/0026.

## Next

1. Reproduce this fixture after device-time/mixer changes, including the first frame and sample.
2. Use 0034's `FIST_SEQUENCE_END_MS` for matched captures. Investigate start-state defects here.

Reproduce: build with `bash tools/oracle/build_sequence_oracle.sh`; run
`bash tools/oracle/capture_sequence.sh 10 <fresh-original-dir>`. For each port set
`FIST_TEXT_STATE=<fresh-original-dir>/start-state FIST_OPL=1 FIST_SB=1 FIST_SEQUENCE_END_MS=10000`;
run `bash tools/capture_port_sequence.sh <native|wasm> 170 <fresh-port-dir>`.

## Accept

Original-attributed fixture verified on both targets, including first frame/palette/time and
mixer origin/initial fill. 0033 checks pass. Full frame/PCM sequence equality remains with 0034.
