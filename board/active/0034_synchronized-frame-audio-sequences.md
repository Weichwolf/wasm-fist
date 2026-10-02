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
- Current PIC data-port change at base `7426275` (0026): fresh complete 10000-ms native/WASM
  frames match each other and the previous native capture; first original pixel failure is still
  event 443 / byte 32004 (13 vs 14). A fresh original GDB startup probe preserves every frame and
  PCM record against `resume-a469760-original/`. Evidence: `scratch/sequence-capture/pic-mask/`.
  These frame diagnostics do not prove full PCM equality or a corrected DOS-load handoff.
- Current MZ load/start-phase change at base `cbbda64` (0026/0036): production native/WASM entries
  match the original's cycle 629918/budget 82 and all 1151 loader-read budgets. Complete 10000-ms
  native/WASM captures retain 698 frames and match each other and the preceding port byte streams.
  Every original palette/time matches; only pixel event 443 differs. Evidence:
  `scratch/sequence-capture/mz-load/{proof.json,final-native-10s/,final-wasm-10s/}`.
  Mixed port PCM and pre-decoder DOS/kernel work remain unresolved; complete comparison still fails.


- Current DOS disk-read step at base `ce89956` (0026): new complete 10000-ms native/WASM streams
  still match one another and the preceding port frames bytewise; every original time/layout/palette
  matches, with pixel event 443 the sole differing frame. Evidence: `dos-transfer/proof.json` under
  `scratch/sequence-capture/`. The MZ/app-entry phase is retained. Extender packet/gateway/REP work
  and mixed port PCM remain open; complete comparison continues to fail.
- The same step's fresh 30000-ms captures reach the stable menu: 2100 native/WASM frames match
  bytewise; all original times/layouts/palettes match, 23 pixel events differ, first still 443.
  Original mixed PCM contains 1324058 samples; port PCM is absent. The last original/port menu
  frame matches bytewise and was visually checked (`dos-transfer/original-last.png`). Complete
  scope: `scratch/sequence-capture/dos-transfer/{check-30s.py,30s-proof.json}`.
- Current base `159ea34`, reached VCPI packet/gateway/SS/REP step (0026): fresh complete
  10000-ms native/WASM streams now match the original over all 698 frames, including every
  indexed pixel, palette entry and presentation time. Event 443 is fixed without a phase offset.
  Fresh 30000-ms native/WASM frame streams match one another over all 2100 records; every
  original time/layout/palette matches. Pixel differences occur at 27 events, first 817 at
  11704156 us, byte 8754 (16 vs 212), 965 unequal pixels. Scope and exact event list:
  `scratch/sequence-capture/kernel-read/{check-proof.py,proof.json}`.
  Corrected frame 443, original/port frame 817 and final menu were visually inspected.
  Original mixed PCM has 1324058 samples at 30000 ms; the port sequence PCM file is absent.
  Strict comparison fails at both endpoints. The 60-test/two-flow gate and source provenance
  belong to 0026. This is bounded video progress; complete frame/audio acceptance remains open.

## Next

1. Recover the first remaining pixel producer at event 817 / 11704156 us in the complete
   30000-ms capture. Preserve the now-equal 10000-ms video and 0036's attributed MZ handoff.
   Timing fixes belong to 0026. Do not adjust capture phase, inject a fitted delay or mask fields.
2. Compare every 30000-ms record through the stable menu after fixing that producer.
   Then add matched timed inputs. Retain the first unequal producer; reuse the versioned 0036 fixture.
3. Implement the final mixer in 0003 on 0026's shared device time; require complete mixed stereo PCM.

## Accept

All three complete intro/menu captures have independent Oracle provenance and identical frame,
palette, time and PCM sequences. Flight/audio/timing extensions remain with 0012/0003/0026.
