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
- Current base `5b8f0fd`, 7120 REP-copy step (0026): all 698 complete 10000-ms original frames
  still match both targets. The fresh 30000-ms native/WASM streams match all 2100 frame records;
  all original time/layout/palette records match, with 27 pixel-difference events and first still
  817 (11704156 us / byte 8754 / 965 pixels). Final menu is equal and visually checked.
  `scratch/sequence-capture/pixel-817/{check-proof.py,proof.json}` retains full scope and event list.
  The reaching copy regression fails all ten parent cases and passes on both targets; the
  61-test/two-flow gate is in `run.deIWNB/`. The copy contract is fixed; first-817 parity is open.
  An IRQ-7-armed original snapshot preserves all 908 frames and 574358 PCM samples over 13000 ms
  and exposes the software mixer running before the decoder. Its state owner is 0003, shared
  time/interrupt owner 0026. Full comparison still fails missing port mixed PCM.

- Current base `bb25d36`, active no-rollover mixer fix (0003): complete fresh 10000-ms native/WASM
  captures match all 698 original frames/palettes/times. Complete 30000-ms captures retain all
  2100 previous frame records bytewise; native/WASM match, every original time/layout/palette
  matches, with the same 27 pixel-difference events, first 817. Mixed port PCM remains absent
  and strict complete comparisons fail at both endpoints. The source paired probe retains all
  908 original frames and 574358 PCM samples over 13000 ms. Exact scope/proof:
  `scratch/sequence-capture/mixer-2630/{check-proof.py,proof.json,native-10s/,wasm-10s/,
  native-30s/,wasm-30s/}`. The mixer block is corrected; production SB IRQ ordering stays open.

- Patch 623 (0003): fresh complete native/WASM 10/30-second captures retain every
  preceding port frame byte. All 698 original 10-second frames match both targets; 30 seconds
  still has the same 27 pixel differences (first 817), with equal original times/layouts/palettes
  and 2100 identical native/WASM frames. Strict comparison still fails missing mixed port PCM.
  `scratch/sequence-capture/mixer-rollover/{check-proof.py,proof.json}` distinguishes these frame
  diagnostics from the successful 63-test/full-178-flow gate `run.N3Lzlt/`. Callback/DMA/IRQ source-state
  evidence and remaining work belong to 0003; the first-817 producer is still unresolved.

- Patch 624 (0003), reached default-RET callback: new complete 10/30-second native/WASM
  captures retain all preceding port frame bytes. All 698 original 10-second frames match;
  2100 cross-target 30-second frames match, every original time/layout/palette matches,
  and the same 27 pixel-difference events remain (first 817). Port mixed PCM is absent and
  strict comparison fails at both endpoints. `scratch/sequence-capture/mixer-callback/{
  check-proof.py,proof.json}` retains exact scope; its paired original probe preserves all
  1119 frames and 706658 PCM samples over 16000 ms. The 64-test/two-flow gate belongs to 0003;
  this is callback-buffer progress, not SB IRQ/timing or complete frame/audio acceptance.


- PCM8 DMA/DSP consumption correction (0003), base `b3d4e72`: fresh complete 10/30-second
  native/WASM captures retain every preceding port frame byte. All 698 original frames
  match at 10 seconds; 30 seconds retains the same 27 pixel differences, first 817,
  with 2100 identical cross-target records and equal original times/layouts/palettes.
  Strict comparison still fails missing mixed port PCM. The original demand fixture and
  isolated device-input regression are owned by 0003; production device setup, IRQ dispatch
  and clock ordering stay open. `scratch/sequence-capture/sb-demand/production-proof.json`
  records this unchanged capture scope. The 68-test/full-178-flow gate `run.RDIm2R/`
  passes with zero failures, exit 0; its bounded device-input proof remains with 0003.
  Production IRQ/timing and complete frame/audio acceptance remain open.

- Patch 630 (0003), initial mixer setup: complete matched SB-enabled 30000-ms
  native/WASM captures retain every previous PIC-controller frame/end byte
  and all 43 register packets. All 2100 cross-target records match; original
  times/layouts/palettes remain equal, with the same 28 pixel differences,
  first 817. Strict comparison still rejects missing final mixed port PCM.
  The restored isolated 23ec-to-138d producer and its original paired state
  belong to 0003. Consume `initial-mixer-call/proof.json`; the 92-test/two-flow
  filtered gate (`run.2p0ZmT/`, exit 0) is not full-matrix or sequence acceptance.
  Actual startup/IRQ/IF/mixer instruction work still must reach the production
  clock before this first differing video event can be repaired.

- Patch631 (0003), original1280 device-configuration leaf: complete matched
  SB-enabled30000-ms native/WASM captures retain every previous initial-mixer
  frame/end byte and all43register packets. All2100 cross-target records match;
  original times/layouts/palettes remain equal, with the same28pixel failures,
  first817. Strict comparison still rejects absent final mixed port PCM.
  Original/source-controlled eight-fetch register/storage proofs belong to0003;
  consume `device-config/proof.json`. The95-test/two-flow filtered gate
  (`run.UvWOM4/`, exit0) is not full-matrix or sequence acceptance. Actual startup,
  CPU/IRQ/IF context and mixer instruction work still must reach production.

- Current base `21a422f`, short PCM8 end-event recovery (0003): fresh complete
  matched SB-enabled 30000-ms native/WASM captures retain all 2100 preceding
  frame/end bytes and 43 packets. Original times/layouts/palettes match; the
  same 28 pixel failures start at 817 / 11704156 us / byte 8754. Final mixed
  port PCM remains absent. Original device-event/sample proofs belong to
  0003; they do not establish protected delivery, startup or sequence parity.
  The 100-test/exact-patch/sequential-build gate passes all 178 existing
  flows in `run.bpay5o/`, zero failures and terminal exit0.
  `sb-short-event/proof.json` checks the frozen 886-source archive, complete
  captures and first remaining original failure. This establishes the bounded
  device change and existing matrix coverage; original sequence parity stays open.

- Patch634 (0003), actual registrar BX/full incoming EBX data transport:
  fresh complete matched SB-enabled30000-ms Native/WASM captures retain
  all2100 previous frame/end bytes and43 complete sound-register rows.
  Original palettes/layouts/times match; the same28 pixel failures start at
  817/11704156us/byte8754. Final mixed port PCM remains absent and fails
  strict comparison. Every47 extender packet is retained; only op68 differs
  cross-target (`f6e80008` / `00000008`), also differing from original5.
  The source/transport proofs belong to0003. The frozen895-source unfiltered
  `run.9hkvAE/` passes103 tests, exact patches, sequential builds, six new
  complete startup cases and all178 existing flows, zero failures and durable
  exit0. Consume `resource-bx/proof.json`; complete sequence acceptance,
  startup CPU/IRQ/IF and final mixer work remain open.

- Temporary storage cleanup at user request: removed the64-GiB ignored
  repository scratch history and obsolete raw logs/captures. The checkout
  is188MiB including the installed original and local DOSBox source/build.
  One `tools/work_dir.sh` owner now defaults disposable engine staging,
  captures and verification to `/tmp/wasm-fist-<uid>-<workspace>/`;
  `FIST_WORKDIR` overrides it. The provision ZIP cache is also in `/tmp`.
  `make clean` removes owned temporary build/capture/verification directories.
  Six build/path/cleanup tests, four provisioning tests, shell syntax and
  exact full patch staging pass; emitted635 engine bytes equal the previous
  candidate. This verifies storage behavior only. Historical raw artifacts
  are retired; source fixtures and commands remain versioned. Current635's
  earlier107-test/two-build/six-start-case gate was interrupted with runner129
  before all178 flows completed and is unaccepted. Recreate its source/output
  evidence under `/tmp` before its production acceptance.

- External staging compile correction: the first unfiltered635 attempt
  under `/tmp/wasm-fist-production635/verify/run.uKaKE6/` passed110 tests and
  exact patches, then failed Native compilation with exit2 because the VGA
  shim's relative `../tools/oracle/fist_sequence_endpoint.h` include assumed
  a repository-local build. The shared endpoint header now resolves through
  explicit oracle include directories in Native, Node-WASM and browser builds.
  Six build/path/cleanup tests and shell syntax pass. Full sequential Native,
  Node-WASM and browser compilation/linking succeeds from the external staged
  source; logs are temporary `storage-{native,wasm,web}.log` under that root.
  Generated engine sources remain unchanged. This bounded build-path repair
  does not accept635 or the interrupted full flow matrix; rebuild and run the
  complete current gate before production acceptance.

- Shared-header compatibility follow-up: the first635 gate at
  `/tmp/wasm-fist-635-check/verify/run.vt9Ag5/` ended1 during tests with nine
  compile errors. The prior renamed header worked in full builds but omitted
  the include directory from existing direct test compilations. Restore the
  original source-relative include and add the source header directory to
  all three build tools. The new behavior test preprocesses the actual VGA
  shim staged externally through Native/Node-WASM/browser tool invocations:
  all three original16699ed scripts fail, all corrected scripts pass. Seven
  build tests pass; the entire previous110-test suite passes in141.836s,
  exit0. Full sequential Native, Node-WASM and browser builds/linking also
  pass, exit0, under `/tmp/wasm-fist-include-check/`. This verifies the shared
  header search contract for both source-local tests and external builds;
  pending635 still requires a complete111-test/full178-flow gate. Raw failed
  snapshots/logs are disposable; no production output is accepted from them.

- Verification isolation correction: the111-test/exact-patch/two-build,
  six-resource-start attempt `run.66aX8a/` was explicitly stopped with143
  while the178-flow matrix was incomplete. Source audit found the inherited
  normal, terrain, audio and OPL-register capture routes could still execute
  directly against repository originals. All verification engine runs now
  consume fresh per-target temporary data copies through the existing shared
  copy owner; editor reload/simulation still receives its explicitly edited
  isolated copy. Parent33bb8d6 fails the new mutation/pristine-input behavior
  test; both corrected targets retain original input for six independent
  normal/terrain/audio runs and leave fixture originals unchanged. All12
  verifier tests pass. Real command `FIST_FLOWS='^(mainmenu|terrain-azer1|audio-opl-init)$'
  bash tools/verify.sh both` passes3/0 with the frozen635 candidate binaries
  from that attempt, using `/tmp/wasm-fist-isolation-check/`. All419 repository
  original file hashes and inventory remain unchanged. This proves isolation
  and those compared outputs only; no full-matrix or635 production acceptance.
  Recreate the complete current112-test/full178-flow gate before acceptance.

- Patch635 (0003) restores the reached task-mode BYTE/SS/typed-caller contract.
  The current unfiltered gate at base6c0d89d completes112 tests, exact patches,
  sequential builds, six startup cases and all178 existing flows, durable0.
  Fresh complete30000-ms Native/WASM captures retain all2100 parent634 frame/end
  bytes,43 register rows and47 full packets per target; all cross-target frame
  records match. Every original time/layout/palette matches,28 pixel events
  still differ, first817/11704156us/byte8754. Strict comparison fails absent
  mixed port PCM; op68 retains its full Native/WASM inequality. Receipt and
  reproduction commands: `tools/oracle/task_mode_production_case.json`.
  No original CPU/IRQ/time or complete surface/frame/audio acceptance follows.

- Consume0030's accepted patch636 WORD callback exchange, explicit actual
  caller CS and packed old pointer; source/production receipts live in
  `tools/oracle/callback_exchange{,_production}_case.json`. The unfiltered
  117-test/six-startup/full178-flow gate passes with durable exit0; actual
  Native writes only the four callback bytes and preserves stale code. This
  does not accept upper GP/flags/farstack/IRQ/time, live INT9 input consumption
  or complete original frame/audio sequences.

- Consume0014's accepted patch637 saved startup file handles/read CF.
  The unfiltered124-test/exact-patch/sequential-build/six-startup/full178-flow
  gate passes with durable0. All2100 fresh30000-ms Native/WASM frame/end
  bytes and43 sound register rows remain equal to636; all original layouts/
  palettes/times match and the same28 pixel failures begin817/11704156us.
  All47 extender packets is retained without masking: only op68's low
  handle changes8→5 per target; its full upperDWORD still differs. Strict
  comparison still rejects absent final mixed port PCM. Source/production
  receipts: `tools/oracle/file_close{,_production}_case.json`. No complete
  original video/audio, CPU/IRQ/time or full surface acceptance follows.

- Consume0009's accepted patch638 source module DTA storage/addressing.
  `tools/oracle/dta_production_case.json` records four reaching parent
  failures, eight target cases and actual corrected constructor/startup
  slots. The unfiltered126-test/exact-patch/sequential-build/six-startup/
  full178-flow gate passes with durable0 and419 pristine originals.
  All2100 fresh30000-ms frame/end bytes and43 sound register rows retain
  parent637 behavior; every original palette/layout/time matches. The same
  28 original pixel failures start817/11704156us/byte8754. All47 packets
  are retained without masks: Native op68 is now `f6680005`, WASM remains
  `00000005`; its full register producer remains open in0009/0014.
  Strict comparison still rejects absent final mixed port PCM. DTA
  storage/addressing does not accept op44 dispatch/CF/errors, device setup,
  original GP/flags/stack/IRQ/time or complete original output.


- Consume0009's accepted patch6396032 find/open carry branches. The
  frozen1046-input gate passes128 tests/exact patches/both sequential
  builds/six startup cases/all178 flows with durable0 and419 unchanged
  originals. Fresh complete30000-ms captures retain every2100 parent638
  frame/end byte,43 sound-register rows and all47 service packets. All
  original palettes/layouts/times match;28 pixel failures still begin
  817/11704156us/byte8754. Full op68 Native/WASM inboxes remain unequal
  f6680005/00000005, and final mixed port PCM is absent. Complete fresh
  missing-HIGH error captures retain the first560798us/F36/palette-byte5
  failure because manual op44 still bypasses6032. Source/production
  receipt and reproduction belong to0009's
  `tools/oracle/file_carry_production_case.json`. Loader CF acceptance does
  not prove task error/GP/stack/CPU/IRQ/device time or original output.

- Consume0009's accepted patch641 task-error WORD store. The frozen1061
  inputs pass132 tests/exact patches/sequential builds/six startup cases/
  all178 flows with durable0. Every2100 fresh30000-ms frame/end byte,
  43 sound-register rows and all47 unmasked packets retain640 behavior.
  Original palettes/layouts/times match;28 pixel differences remain,
  first817/11704156us/byte8754. Native/WASM op68 remains e6480005/00000005,
  and strict comparison still fails absent final mixed port PCM. Complete
  missing-HIGH production still first differs at560798us/F36/palettebyte5
  because manualop44 bypasses6032. The error-helper observation stops after
  its stores before unaccepted nonlocal continuation. Source/production
  receipt belongs to0009's `file_error_status_production_case.json`; no
  complete original output, GP/flags/stack/IRQ/time or full surface
  acceptance follows from this width correction.

- Consume0014's accepted642 normal return/configuration transport and its
  `tools/oracle/detail_return_production_case.json`. Complete fresh30s
  Native/WASM streams retain all2100 parent frame/end bytes and43 sound
  rows. Every original layout/palette/time and first10s video remain equal;
  the same28 pixel failures begin817/11704156us/byte8754. All47 unmasked
  cross-target service packets now agree, including op68 EBX5 with separate
  source-backed EAX2; this fixes the data input, not its missing effects
  body. Strict original comparison still fails absent final mixed PCM.
  Complete missing-HIGH error output still differs at560798us/F36/palette
  byte5 and has39 versus38 frames. The full135-test/exact-patch/sequential
  Native-WASM/six-startup/178-flow gate is green with durable0; complete
  original output and all-surface acceptance remain open.

- Consume0003's accepted643 effects BYTE storage correction and
  `tools/oracle/effects_width_production_case.json`. The frozen1089-input
  137-test/exact-patch/sequential-build/six-startup/full178-flow gate and
  additional600-ms/30s captures finish0. Every2100 fresh Native/WASM
  frame/end byte,43 sound rows and47 full unmasked packets retains642
  behavior; all cross-target records agree. Original first10s video and
  every original layout/palette/time still match. The same28 pixel failures
  begin817/11704156us/byte8754, final mixed PCM is absent, and strict original
  comparisons fail. Missing-HIGH error output retains39 versus38 frames and
  first560798us/F36/palettebyte5. The isolated actual76fd-to138d regression
  proves storage width only; production effects/device/CPU/IF/IRQ/time and
  complete original sequence/surface acceptance remain open.

- Consume0003's accepted644 actual reset/full-CPU-context correction and
  `tools/oracle/device_reset_production_case.json`. The frozen1353-input
  140-test/exact-patch/sequential-build/six-startup/full178-flow gate
  `run.9fdN3p` and all additional600-ms/30s captures finish0. Every2100
  fresh Native/WASM frame/end byte,43 sound rows and47 full unmasked
  packets retains643 behavior; all cross-target records agree. Original
  first10s video and every original layout/palette/time still match.
  The same28 pixel failures begin817/11704156us/byte8754; final mixedPCM
  is absent and strict original comparisons fail. Missing-HIGH error
  output retains39 versus38 frames and first560798us/F36/palettebyte5.
  The isolated reset's complete register/flag/core-exit proof supplies
  the actual startup dependency; production77e2/op6c/op68, protected
  IRQ/IRET/IF transport and complete original sequence/surface acceptance
  remain open.

- Consume0003's accepted645 full configuration CPU/resident-RAM receipt,
  tools/oracle/device_config_cpu_production_case.json. The immutable1154
  source inputs plus248 original references and419 unchanged originals
  bind140 tests in289.911s, exact patches, sequential builds, six startup
  cases and178 existing flows with zero failures and terminal0. Complete
  additional captures and post-cleanup replays finish0. All2100 fresh
 30s Native/WASM frame/end bytes,43 sound rows and47 unmasked packets
  retain644 output and agree across targets. Original first10s video and
  every original layout/palette/time match; the same28 pixel failures
  begin817/11704156us/byte8754. Final mixed portPCM is absent; strict
  original parity still fails. Missing-HIGH retains39 versus38 frames and
  first560798us/F36/palettebyte5. Full resident configuration/reset CPU/RAM
  regressions establish a startup dependency, not production77e2/device/
  IRQ/time integration or complete sequence/surface acceptance. Completed
  raw matrix/build/game artifacts were retired in/tmp after verification.

- Consume0003's accepted shared startup/checkpoint flag receipt,
  tools/oracle/cpu_startup_flags_production_case.json. Immutable1163 source
  inputs,248 original source references and419 unchanged originals bind
  run.A056Yn:143 tests in279.322s, exact patches, both sequential builds,
  six startup cases and178 existing flows with zero failures and terminal0.
  Complete additional startup/error/30s captures and post-cleanup replays
  finish0. Every2100 Native/WASM frame/end byte,43 sound rows and47 unmasked
  packets preserve645 output. Original layouts/palettes/times and first10s
  video still match. The same28 pixel failures begin817/11704156us/byte8754;
  final mixed portPCM is absent and strict original comparison fails.
  Missing-HIGH retains39 versus38 frames and first560798us/F36/palettebyte5.
  Scope is the shared flag dependency, not actual77e2/checkpoint/task/bank/
  device/IRQ/IF time integration or complete original sequence acceptance.


- Consume0026's shared CPU/system helper receipt,
  tools/oracle/cpu_interrupt_production_case.json. The frozen1412-input gate
  run.g2L0Tw passes147 tests, exact patches, sequential builds, six startup
  cases and all178 flows with zero failures and terminal0. Complete additional
  30s Native/WASM captures preserve all2100 preceding frame/end bytes,43 sound
  rows and47 unmasked service packets. Original layouts/palettes/times and
  first10s video match;28 pixel differences remain, first817/11704156us/
  byte8754. Strict original comparison rejects absent final mixed portPCM.
  Complete CPU/system/RAM API proofs establish a shared dependency; actual
  startup/IRQ/IF/handler integration and full original output remain open.
  Completed raw matrix artifacts and isolated games were retired under/tmp.

- Consume0026's shared physical-memory adoption receipt,
  `tools/oracle/cpu_memory_production_case.json`: full151-test/178-flow/six-resource
  gate passes with terminal0. Fresh complete30000-ms Native/WASM captures retain
  all2100 previous frame/end bytes,43 sound rows and47 unmasked service packets.
  Original layout/palette/time still match; the same28 pixel differences start
  at817/11704156us/byte8754. Final mixed portPCM remains absent and strict parity
  fails. Actual application startup/IRQ transport remains with0026; complete
  original output is not accepted by these API/producer regressions.

- Consume0026 software-DOS API adoption, parent2fdaeee: actual firstINT21
  and final outer32-bit RETF with complete58-word/RAM/provider/cache/VGA parity
  on Native/WASM. Full155-test/178-flow/six-resource regression passes with
  terminal0;1480 source inputs and419 originals unchanged at terminal. Consume
  `tools/oracle/software_dos_production_case.json` for command, hashes and scope.
  Complete kernel execution and production startup/IRQ transport remain open. Fresh candidate30-second Native/WASM
  captures retain2100 previous frames/end,43 sound rows and47 full service
  packets. All original palettes/layouts/times match;28 pixel failures begin817
  and strict comparison rejects missing final mixed portPCM.

- Consume0026's original PIC binary32 rounding receipt,
  `tools/oracle/pic_float_production_case.json`. Frozen1481-input run.RZjRij
  passes155 tests, exact patches, sequential builds, six startup cases and
  all178 flows with no failures and terminal0;419 originals remain unchanged.
  Existing source-backed SB/PIC expectations now also run optimized Native32.
  Complete fresh30000-ms Native/WASM captures retain all2100 preceding
  frame/end bytes,43 sound rows and47 unmasked service packets. Original
  layouts/palettes/times match;28 pixel errors begin817/11704156us/byte8754
  and strict comparison rejects missing final mixedPCM. The private continuous
  startup/both-DOS proof matches1034 CPU/time,52 complete memory and four host
  contexts on both release targets, with initial host inputs observed in the
  same original run. This supports later production adoption; actual startup/
  IRQ transport, first817 and complete original sequence acceptance stay open.

- Consume0026's versioned complete startup/both-DOS observation and shared
  narrow flag receipt, `tools/oracle/software_startup_dos_production_case.json`.
  The frozen1485-input run.UGEIUm passes159 tests, exact patches, sequential
  builds, six resource-start cases and all178 flows with durable0;419 original
  assets remain unchanged. The public capture retains1014 original fetches,
  complete memory/provider/cache/VGA and four host states; original39frames/
  27518PCM/end600 remain equal to the unprobed run. Private continuous
  preparation using public flags matches1034 CPU/time,52 memory and four host
  contexts on both release targets. Reaching flag substitutions retain complete
  RAM/host outputs but reject complete CPU state. The interpreter and actual
  production startup/IRQ transport remain open. Fresh complete30000-ms streams
  retain all2100 prior frames/end,43 sound rows and47 unmasked service packets.
  Every original layout/palette/time matches;28 pixel failures still start817/
  11704156us/byte8754 and strict parity rejects missing final mixedPCM.

## Next

1. Recover the first remaining pixel producer at event 817 / 11704156 us in the complete
   30000-ms capture. Preserve the now-equal 10000-ms video and 0036's attributed MZ handoff.
   Recover the reached SB IRQ/software-mixer state in 0003 and its CPU/event ordering in 0026.
   Preserve 7120's now-proved REP contract. Do not adjust capture phase, inject a fitted delay
   or mask fields.
2. Compare every 30000-ms record through the stable menu after fixing that producer.
   Then add matched timed inputs. Retain the first unequal producer; reuse the versioned 0036 fixture.
3. Implement the final mixer in 0003 on 0026's shared device time; require complete mixed stereo PCM.

## Accept

All three complete intro/menu captures have independent Oracle provenance and identical frame,
palette, time and PCM sequences. Flight/audio/timing extensions remain with 0012/0003/0026.
