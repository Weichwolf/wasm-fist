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
- `FIST_SEQUENCE_END_MS` selects a positive uint32 endpoint on the emulated timeline.
  DOSBox stops after the tick's mixer and before the next CPU/queued presentation phase;
  ports stop on the shared clock. Missing/mismatched `sequence.end` markers fail, including
  early host exits with valid frame footers. Default Oracle: normal core, fixed 30000 cycles/ms.
- Complete 10000-ms original runs 731/734/742 match 698 frames and 442058 samples.
  Matched native 794/WASM 795 retain all prior frame bytes and match each other.
  Every original palette/time matches; event 443 pixels differ. The endpoint is during the intro.
- At event 443 (6,367,900 us), 8,674 pixels differ exclusively in rows 100–149.
  Original framebuffer copy 7120 starts at 6365.540100 ms, after the third scanout band;
  the prior port starts at 6364.404592 ms, before it. Callback 88's pure decoder count matches.
  Original pre-decoder DOS/kernel work is absent from the port (0026). The bounded CPU trace
  accounts for all 35524 cycles before decoder 88; do not inject that measured total as a delay.
  Formula and original CPU/queue proofs belong to 0026; versioned start fixture belongs to 0036.
- Matched 30000-ms original captures 796/797 reach the stable main menu: all 2100 frames and
  `1103 + floor(29999×722534/16384) = 1324058` PCM samples match. Last original frame visually checked.
  Native 809/WASM 810 match all frame bytes and previous captures 798/799 (also baseline 800).
  Original palettes/times match; pixels differ at 23 events (`2100−23 = 2077` equal), first still 443.
  Complete comparison fails missing port PCM. Proofs: `start-epoch/capture-proof.json` and
  `cpu-retirement/complete-30s-proof.json` under `scratch/sequence-capture/`. Timing belongs to 0026.
- PCM length follows 0003's mixer: `1103 + floor((end_ms−1)×722534/16384)`.
  Ports still lack final mixed sequence PCM. `--frames-only` is diagnostic, never acceptance.

## Next

1. Implement shared PIC event/slice ownership and the recovered DOS/Extender kernel transfer
   and instruction-time contract in 0026, including the attributed DOS-load handoff (0036).
   Fix callback 88's premature copy and compare every 10000-ms record on both targets.
   Do not adjust capture phase, inject a fitted delay or mask fields.
2. After the first difference is fixed, compare every 30000-ms record through the stable menu.
   Then add matched timed inputs. Retain the first unequal producer; reuse the versioned 0036 fixture.
3. Implement the final mixer in 0003 on 0026's shared device time; require complete mixed stereo PCM.

## Accept

All three complete intro/menu captures have independent Oracle provenance and identical frame,
palette, time and PCM sequences. Flight/audio/timing extensions remain with 0012/0003/0026.
