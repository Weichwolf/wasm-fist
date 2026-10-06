Type: feature
Title: Original and ports share a proven application-start presentation boundary
Parent: 0034

## Contract

Own one original-attributed DOS-load fixture, application handoff and frame/PCM origin.
Preserve the prefix and timing.
Device time belongs to 0026, mixed output to 0003, complete capture/comparison to 0034.

## Evidence

- `capture_sequence.sh` materializes and hash-checks `.text`, `.bda`, `.vga` from versioned assets
  under `tools/oracle/start_state.*`. `sequence_start_state.patch` restores B800/BDA at `FIST.RUN`.
  Set port `FIST_TEXT_STATE=<oracle-capture>/start-state`; no separate phase environment is needed.
- `.vga` distinguishes measured DOS-load/scanout phases (20,960,467/20,168,067 ns) from queued PIC
  origin: tick 20, float lag `0x3e2c1900` = `0x1.5832p-3` = 0.168064117431640625 ms.
  Original queue: `float(0.9000000953674316 + float(14.2680641955)) − 15` gives this residual.
  GDB probe: `tools/oracle/start_state_pic.gdb`; log `oracle-pic-start-741/dosbox.log` under
  `scratch/sequence-capture/`. Dispatch cycle 5042 is later than the queued origin; conflating them
  caused six 1-us presentation errors. Missing/malformed `.vga` now fails on both targets.
- Original CPU trace 783 attributes the fixture clock to `f000:14a1`, DOS EXEC callback,
  at cycle 628814. Application `1119:0004` appears at cycle 629918 after loop decrement.
  The difference is `629918−628814 = 1104` cycles, or `1104/30000 = 0.0368 ms`.
  GDB probe 784 confirms 38 costed `FIST.DAT` mask reads: `38×29 = 1102` cycles;
  remaining slice `1186−1102 = 84 < 3×29 = 87` suppresses subsequent I/O delays.
  RETF and the first application fetch account for the remaining two decrements. The port skips
  this DOS loader work; do not call the fixture an exact application-entry CPU phase or fit an offset.
  Both probes retain all 742 frame/PCM bytes. Proof: `pixel-443/cpu-start-source-proof.json`
  under `scratch/sequence-capture/`; full clock/handoff correction remains with 0026.
- Exact fixture CPU phase is `628814×1193182 = 25009×30000000 + 19546148` PIT numerator units.
  The previous port rounded to 25010, adding `10453852/1193182 = 8.761322…` CPU cycles.
  `test_initial_cpu_phase_matches_original_queue` restores cycle 628814/budget 1186 on both targets;
  the actual-source PIC probe rejects the old 628822/1178. Proof: `start-epoch/phase-proof.json`.
  Accepted: 46 tests, both builds, 178 flows, exit 0; `run.6eoNh3/` (0026).
  Original 802 confirms all 38 costed loader reads see mask `0xf8` (IRQ2 clear); no extra mask write.
  Its complete 30000-ms frame/PCM streams match 797. Proof: `cpu-retirement/read-mask-30s-proof.json`.
- Fresh base `7426275` proof in 0026 independently confirms all 1151 FIST.DAT mask reads and the
  cycle-629918 application fetch, preserving every original 10000-ms frame/PCM byte. PIC data-port
  budget/register behavior now matches actual-source probes on native/WASM; the production harness
  still reads the extracted image directly and omits the original loader reads. The handoff remains open.
- Current base `cbbda64`: the production harness now loads FIST.DAT through the same MZ path as
  overlays. All 1151 reads match the fresh original's budget trajectory on both targets: 28 header
  bytes, six 32768-byte reads, one 14848-byte request/14604-byte result, and 1143 relocation reads.
  Reads finish at cycle 629916/budget 84. The original's next two normal-core fetches are
  f000:14a6 RETF and 1119:0004 MOV AX; production reaches app_entry at cycle 629918/budget 82.
  No measured total is injected. `fist_text_clock_init` owns the initial fixture clock/calendar;
  later BIOS/text initialization preserves elapsed loader time and the existing rebased memory layout.
  Source, memory/phase regressions and 55-test scoped validation belong to 0026. Proof:
  `scratch/sequence-capture/mz-load/{proof.json,final-native-10s/,final-wasm-10s/}`.
  The full mixer origin and remaining startup instruction/device effects are still unproved.
- Fresh unmodified original full-register capture at base `1d2e673` observes
  stable application fetch `1119:0004` at cycle629918/budget82, with full
  EBX=`00000000`. All eight complete GPRs, six segment values/bases, raw and
  lazy flags, mode/control registers and whole 16-MiB memory are retained in
  `engine-entry-registers/{check-source.py,source-proof.json}`. This is an
  observed incoming DWORD, not permission to discard a caller's upper word.
  DOS EXEC assigns WORD BX=0; the measurement also establishes the actual
  inherited upper word for this normal start. Controlled `89ab` transport
  evidence for153c remains with0003.
- The complete 211212-byte loaded image matches FIST.DAT after all 1143
  original MZ relocations and DOS EXEC's two CS/IP stack stores. Those stores
  account for every three changed image bytes; both whole 16-MiB observer
  snapshots are byte-identical. The earlier CS hardware watch fires inside
  CPU_RET before its physical CS base/IP and final SP updates; it is explicitly
  a transient implementation checkpoint, not an architectural return state.
  Complete 39 frames/27518 mixed samples/end600ms are identical to the normal
  baseline. No port full-register, final mixer or full-run parity follows.
- Original fixture/capture provenance: `pic-dispatch-362/`; current content/timing comparisons are in 0034.
- Audio origin remains unresolved: original first nonzero stereo sample 18,887 at 428,276 us
  comes from `SPKR`; the port's first counter is 6.538382 us late. Continue in 0003/0026.


- Bounded initial palette adoption, parent `e18a05b`: fresh read-only
  `DOS_Execute(FIST.RUN)` and `DOS_Execute(FIST.DAT)` entry/return snapshots retain
  every named DAC field,full RGB/xlat/combine arrays,the complete renderer
  palette,attribute mapping and actual32-bit surface/scaler fields. FIST.DAT
  loader entry has mode9/machine5/S3Trio,state WRITE,shared cursor0,write_index64
  and the literal BIOS64-color hardware table. Attribute mapping is
  `00..05,14,07,38..3f`. The old `fist_vga_text_palette.h` stores16 remapped
  renderer colors plus zeros;its hardware-DAC claim was disproved at byte19.
  Hardware and renderer now have separate shared owners. Original text mode
  writes64 hardware entries and retains the other192 entries and unmapped
  renderer colors;complete controlled dirty-state coverage belongs to0026.
- `tools/oracle/dac_startup.gdb` and `capture_dac_startup.py` share the complete
  DAC/renderer snapshot owner with0026's reaching probe. Reproduce with
  `python3 -B tools/oracle/capture_dac_startup.py --repo . --output /tmp/wasm-fist-dac-startup`
  and `python3 -B -m unittest discover -s tests -p test_dac_startup.py -v`.
  The actual production `fist_text_clock_init` constructor matches all3635
  hardware/renderer palette bytes at FIST.DAT loader entry on both targets.
  The fixture calls that constructor and contains no copied palette recipe.
  Fresh original39frames/27518mixedPCM/end600 equal unobserved output.
  The complete217-test/611-patch/both-build/six-startup/all178-flow gate passes
  with419 unchanged originals;bounded provenance is in
  `tools/oracle/cpu_outsb_production_case.json`. Actual BIOS CPU costs,full mode/
  drawing/scaler/frame-consumer transport and mixer origin remain open.

## Next

1. Preserve the now-proved MZ reads and application-fetch phase while completing 0026's production
   DOS/kernel reads, pre-speaker instructions and interrupt effects. Recheck the first sample in 0003.
2. Reproduce this fixture after device-time/mixer changes, including the first frame and sample.
3. Use 0034's `FIST_SEQUENCE_END_MS` for matched captures. Investigate start-state defects here.

Reproduce: build with `bash tools/oracle/build_sequence_oracle.sh`; run
`bash tools/oracle/capture_sequence.sh 10 <fresh-original-dir>`. For each port set
`FIST_TEXT_STATE=<fresh-original-dir>/start-state FIST_OPL=1 FIST_SB=1 FIST_SEQUENCE_END_MS=10000`;
run `bash tools/capture_port_sequence.sh <native|wasm> 170 <fresh-port-dir>`.

## Accept

Original-attributed fixture verified on both targets, including first frame/palette/time and
mixer origin/initial fill. 0033 checks pass. Full frame/PCM sequence equality remains with 0034.
