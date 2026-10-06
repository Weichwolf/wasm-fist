Type: bug
Title: Indirect calls preserve the original argument and return-register contract

## Contract

Each indirect call supplies the target's actual live inputs and exposes its outputs/flags on both
targets. Matching C arity alone does not establish the correct register values.

## Evidence

Patches 528–530 repaired MGA sprite/descriptor mismatches; 573/587/588 fixed further reached
sites. Historical remaining examples: cdc4 → vector 0x6b4 with a short list; wrong-valued lists
in 6350/6453; arg-less fill/outline sites in 3de2/3de5/4937/57a9/6612/664e/66b1/6730/684e/
688a/6909/d27e/d2bd/d2f5/d443/d480/6448/6beb/8fb6/9774/9a68. Re-audit the current tree.

- Base `965beb8`, actual sound-start dependency in0003: original CRT installs
  resource vector388 before boot222f and153c. The frozen port delays installation;
  a scratch early-install prototype reaches an arg-less222f ->26fc call with
  stale CX/DX/BX/BP and a failed-variant DI read spanning BP. Original222f assembly
  sets BX6b78, CXDS and DX6b7e before the far call; BP is an inherited lane, not
  permission to invent zero. Preserve the source's register widths and all live
  return/flag lanes. The source153c pair additionally proves dynamicBX018f and
  preservation of a controlled89ab upperEBX word. Source proof and complete
  frame/PCM capture scope belong to0003; no two-target repair is accepted yet.

- Patch 632 at base `1472e73` verifies the narrower failed-variant callee
  width contract in 0003: actual original `BACKLAND.BIN`, three AH43 misses
  with nonzero BP `1718`, two reaching parent failures, and whole 16-MiB
  current Native/WASM comparisons. DI no longer includes adjacent BP;
  DX/SI use WORD bridge lanes and the existing ES lane is preserved.
  The original caller still sets CX=DS, DX=`6b7e`, BX=`6b78` and inherits BP;
  the arg-less port caller and live ambient CPU/return state remain unresolved.
  Recover those owners before restoring early installation. Consume 0003's
  `engine-font-loader/` evidence: canonical `run.dkymjy/` completed 101 tests,
  exact patches, both builds and 178 existing flows, with durable exit0.
  The narrower repair does not establish full startup/output parity.

- Patch 633 restores boot222f's source-proven four-argument far call:
  CX=rebased DS, DX=`6b7e`, BX=`6b78`, BP from actual1345's existing output.
  It also consumes0003's original early CRT0174 vector installation and
  removes the delayed menu installation/retry. Four reaching parent failures
  and eight complete-memory Native/WASM phase cases are retained in
  `engine-boot-resource/`; unfiltered `run.Hw6Cpn/` completes 103 tests,
  exact patches, both builds and 178 existing flows, durable exit0.
  No dynamic153c return or full incoming-EBX transport repair is included.
  Actual Native backtrace also exposes00d0's separate uninitialized high-word
  reconstruction; app_entry alone cannot own the complete caller transport.

- Consume0003's four original153c return-branch pairs in
  `resource-return-branches/`: empty and controlled carry exits preserve BX;
  one emitted descriptor leaves the dynamic dimensions product121. Both
  source overflow exits consume a savedDS word as IP rather than return to
  CB13. Complete-memory/register proofs bound those inputs; partial observer
  streams fail full capture acceptance. Do not infer a generic normal return
  from the generated void signature or publish a fixed018f result.

- Consume0003's new original controlled-dimension pair at base `fc14eb4`:
  actual153c returns35 from0705, with the complete40-record loop memory
  and every other return-state field verified. Original600-ms frame/PCM/end
  bytes remain unchanged. This strengthens the dynamic output contract;
  the pending production transport repair still needs its reaching two-target
  regression and complete existing matrix. Proof: `resource-bx/source-dimensions/`.

- Consume0003's original settings e2df/op68 pair in `settings-poster/`:
  eight fetches publish full EBX=5 independently of AX=2, with complete
  16-MiB/register/segment/raw+lazyflags proof and unchanged600-ms frame/PCM/end.
  The pending634 candidate exposes a separate6e0e high-word constructor:
  op68 inbox `f6e80008` Native versus `00000008` WASM. Low8 also differs
  from the original5; retain full packets and recover upstream DOS/ambient
  registers. Source/probe evidence is not a production or generic-call repair.

- Consume0003's original task-mode getter354/setter358 pair: actual returned
  AL, inherited SS pointer and exchange outputs are proved over15 fetches
  and whole16-MiB pairs. The current d99b discards the getter then calls358
  without AL; Native receives stack-residue144. Old129 also uses stale SS
  literals. Thread the actual getter output through the setter's byte type
  and recover its pointer owner before restoring startup mode behavior.
  Source: `task-mode-registers/`; Native writer proof: `resource-bx/native-diagnostic/`.

- Original controlled task-mode cases in0003's `task-mode-controlled/`
  prove byte-width aliases explicitly: upper24 EAX bits89ab7b survive both
  calls, getter returns42 and setter returns old42 after storing incominga5.
  A null SS pointer skips only the packet path, retaining the CS exchange.
  Complete source states/memory are checked before explicitly restoring the
  architectural test inputs; no CPU time is adjusted. These source-only cases
  constrain the next typed get/put transport and pointer fix.

- Consume0003's original CS/image mapping: task mode is a BYTE at123e9,
  not the emitted WORD at12d59. The scratch typed-caller/SS/BYTE-owner
  repair passes eight whole-memory target cases after eight parent failures,
  including exchange-return, null pointer and nonzero adjacent byte inputs.
  `task-mode-prototype/` remains a leaf/post-create-fragment proof;
  production startup, broader handler and CPU/time acceptance stay open.

- Patch634 consumes0003's original153c normal/dynamic/empty WORD output
  and full incoming EBX through app_entry/0007/00d0 into the first6c poster.
  The void registrar ABI remains unchanged. Five parent transport failures
  plus three cross-target failures become six complete startup cases on both
  targets under both start fixtures. The frozen895-source unfiltered gate
  `run.9hkvAE/` passes103 tests, exact patches, both sequential builds,
  six new cases and all178 existing flows, durable exit0. Bounded proof:
  `resource-bx/proof.json`; op68, task-mode caller/owner, loaderCF/overflow
  stack, CPU/device/instruction timing and full mixed output remain open.

- Patch635 consumes0003's original CS BYTE mode/SS task-pointer contract.
  Actual d99b threads getter354's returned byte into typed setter358 instead
  of an argument-less call. Eight whole16-MiB parent failures become eight
  exact target cases; the actual Native caller/store is independently observed.
  Base6c0d89d's unfiltered112-test/sequential-build/six-startup/full178-flow
  gate passes with durable exit0. Consume the portable command and compact
  receipt in `tools/oracle/task_mode_production_case.json`. This accepts that
  typed transport only; CPU upperEAX/flags/farstack, handler depth/error,
  op68 ambient registers and46b6 callback ES/exchange remain open.

- The reached46b6 exchange is the keyboard hook, owned by0030. Actual
  original3446 supplies ES=CS and BX=3e0b; helper XCHG returns old AX/ES
  and BX, with all upper register bits/flags preserved. Its callback WORDs
  are the inline FAR-CALL operands at image14628/1462a, not14f98/14f9a.
  Consume `tools/oracle/callback_exchange_case.json` and0030's four fresh
  source cases. The parent one-argument C callee used uninitialized ES and
  lost the old offset; patch636 accepts its bounded WORD pointer/typed-caller
  repair. Full CPU/register transport and actual INT9 dispatch remain open. A function name or passing target matrix does not prove CS.

- Consume0030's accepted patch636 WORD callback exchange, explicit actual
  caller CS and packed old pointer; source/production receipts live in
  `tools/oracle/callback_exchange{,_production}_case.json`. The unfiltered
  117-test/six-startup/full178-flow gate passes with durable exit0; actual
  Native writes only the four callback bytes and preserves stale code. This
  does not accept upper GP/flags/farstack/IRQ/time, live INT9 input consumption
  or complete original frame/audio sequences.

- File lifetime provenance reached through0003's settings/op68 startup:
  original fefb actually executes0f69:086b (runtime2082), and50c8 executes
  0f69:5a38. Names do not establish CS. The config helper saves open AX,
  reads at most64 bytes, restores that handle for BX/AH3e and preserves the
  read flags with PUSHF/POPF. Natural SOUND.CFG returns ten read bytes and
  close AX3e05; the missing second config returns AX2 and skips read/close.
  Both actual overlay size paths close saved handle5 and return DX:AX sizes
  0000:433c /0000:7c9c. Fresh bounded CPU traces have complete footers and
  retain all39 original frames/27518 mixed samples/end600ms. Source fixture
  and reproduction: `tools/oracle/file_close_case.json`,
  `python3 -B tools/oracle/capture_file_close.py --output
  /tmp/wasm-fist-file-close-source`. Production still supplies unrelated
  fefb param9 /50c8 param7 to close, leaking SOUND.CFG/SOUNDDVR/MGAVIDEO;
  repair and full target regressions are pending. This source proof accepts
  no port CPU/flags/stack/IRQ/time or complete original output.

- Consume0009's fresh source-only detail-loader contract in
  `tools/oracle/detail_loader_case.json`: actual initialization stores the
  full DTA offset080b, and6032 loads that DWORD before replacing BX with
  the saved WORD handle. Thus natural full EBX becomes5 through its actual
  producer; the generic mode gate does not merely clear upper words.
  Handler and actual de89 return EAX804. The later AX2 is a separate
  configuration WORD load/shift in6de2, corrected by0009 below. The
  current6de2 still posts its pre-de89 parameter unchanged; source provenance
  constrains the next typed result transport but does not accept that repair.

- Patch637 accepts the reached saved-handle WORD/read-CF contract above.
  Actual fefb saves open AX across the read and restores its read CF after
  close;50c8 closes its saved handle while retaining both seek-size words.
  Seven real-DOS tests reach16 failures on the parent and20 successful
  target/case runs after the repair, including unrelated peer handles,
  short/full/empty/error/missing reads and sparse sizes above64KiB. Actual
  Native startup releases SOUND.CFG/SOUNDDVR/MGAVIDEO before FIST.SET;
  its handle allocation now agrees with the original. The frozen1023-input
  basec4cf756 unfiltered gate `run.8EVxkQ` passes124 tests, exact patches,
  sequential Native/WASM builds, six startup cases and all178 existing
  flows, durable gate/runner0; all419 original files are unchanged.
  Compact receipt and portable actual-startup observation:
  `tools/oracle/file_close_production_case.json`,
  `python3 -B tools/capture_file_close_native.py --native
  /tmp/wasm-fist-637-complete/production/native --output
  /tmp/wasm-fist-file-close-native`. Fresh30s frames/end/sound bytes remain
  unchanged; only op68's saved handle changes8→5 in each full47-packet
  stream. Its upperDWORD still differs Native/WASM. Full GP/ES/raw+lazyflags,
  farstack/IRQ/time, loader errors and complete original output remain open.

- Consume0009's corrected op44 return/configuration provenance in
  `tools/oracle/detail_return_case.json`: actual de89 returns EAX804,
  while6de2 later loads2→1 for c008 and4→2 for e2df from independent
  configuration WORDs. The old6e00 row was mislabelled as a leaf return.
  Keep both the returned full EBX and the actual shifted AX input; neither
  a fitted handle5 nor a constant AL2 restores their producers. The
  generated6de2 currently omits the8b4b load/shift before op68. Port repair
  and complete original output/CPU/flags/time acceptance remain open.

- Consume0009's `detail_operands_case.json`: five original detail/sky
  cases, two reached6032 calls per case, complete selected file bytes and
  observed EAX804/EBX5/ECX0/EDX0. The natural upper EBX result comes from
  the real FILEMGR DTA DWORD/handle WORD path. The sky-zero branch also
  supplies its omitted default6877 store. Do not clear an upper word or
  substitute fixed5; wire the actual producer and the independently
  loaded/shifted configuration AX from `detail_return_case.json`.
  Port CF/errors, all returned GP/flags and instruction/device time remain
  separate unaccepted contracts.

- Consume0009's accepted patch638 module DTA storage/address contract in
  `tools/oracle/dta_production_case.json`: four parent failures become
  eight real-DOS target cases; actual constructor/both startup slots hold
  source080b. The frozen1030-input full126-test/six-startup/178-flow gate
  passes0. All2100 parent frame/end bytes and43 sound rows are retained;
  full47 packets expose Native op68 `f6680005` versus WASM `00000005`.
  The upper-word difference, stale6de2 transport and omitted configuration
  AX are unresolved. No CF/errors, archive parsing, GP/flags/stack/IRQ/time
  or complete original frame/audio acceptance follows.

- Consume0009's original missing-file `file_error_case.json`: CF1 jumps
  through5e3a/38e8 to0f64; its reason DWORD andffff task WORD precede
  restoration of the saved service ESP and far return. A normal6032 or
  error-helper return would preserve the wrong caller continuation.
  Both638 targets still fail this reached case, recorded without masks in
  `file_error_port_case.json`. Recover that service boundary alongside
  real FILEMGR results; do not infer a universal returned handle or upper
  GP policy from the successful HIGH.DTL case.


- Consume0009's original full normal-return proof in
  `tools/oracle/detail_return_state_case.json`:89 fetched instructions,
  25 complete register/segment/raw+lazyflag/control/time/16MiB boundaries
  and23 exact instruction/RAM transitions retain EBX5 through e339/de89.
  The successful task-status branch restores saved EAX/ESI with WORD
  stack operations;6dfd receives EAX804. Actual WORD config loads2/4 then
  SHR supply AX1/2, with the first value stored by c008. All39 original
  frames/27518PCM/end600 remain unchanged; four negative verifier cases
  fail. This strengthens the source contract for typed transport; the
  source contract feeds642 below; remaining full GP/flags/stack stay open.


- Accepted patch642 normal return/configuration transport at tested runtime
  parent3935ac3: actual op44 publishes its real FILEMGR EAX/full EBX once;
 6de2 consumes it afterde89 and preserves EBX across actualc008. Original
  WORD[8b4b]/SHR supplies EAX separately to e2df, preserving its high WORD.
  Existing void de89/generic gateway signatures remain; the shim ISR saves
  include both explicit lanes. Two meaningful regression methods reach
  eight parent failures with no errors, then eight candidate cases pass on
  Native/WASM. Actual6de2/de89/e339/c008/e2df, op44/6032/DOS, guarded complete
  tables, six DOS commands, real peer handles6..8 and independent WORD
  configurations are exercised. The existing eight op44 cases also pass.
  Observation stops at the actual op68 input, without a missing service
  body or fabricated return. Portable scoped receipt:
  `tools/oracle/detail_return_production_case.json`.
  All1078 frozen runtime inputs/archive and419 originals retain their bytes.
  The full `run.1Tq3ru` gate passes135 tests in219.304s, exact patches,
  sequential Native/WASM builds, six startup cases and all178 existing
  flows with zero failures. Gate/runner and every additional600-ms/30s
  capture terminate0. Concurrent5ae2938/db7e81b source-only commits do not
  change these tested runtime inputs. Staged Extender bytes retain parent.
  Fresh complete2100-frame/end sequences and43 sound-register rows match
  parent; every original palette/layout/time and the entire first10s video
  remain equal. All47 full packets now agree across targets without masks;
  actual op68 is EBX00000005, with separate EAX00000002 and actual loader
  result EAX00000804/EBX00000005, matching the source. Native's previous
  e6480005 inbox is eliminated by the actual producer, not by truncation.
  The same28 pixel failures begin817/11704156us/byte8754; strict original
  parity still fails absent final mixed port PCM. Complete missing-HIGH
  output retains39 versus38 source frames and the first560798us/F36/
  palettebyte5 failure, although reason1/WORDffff diagnostics agree. This
  accepts normal EAX/fullEBX/config data transport only: complete incoming
  and returned GP/flags/segments/stack/nonlocal error/CPU/IRQ/device time,
  actual76fd/77e2/23ec execution and complete output remain open.

- Consume0003's complete original startup-prefix fixture
  `device_start_prefix_case.json`: actual77e2 arrives with a DWORD0f57
  service-return frame, calls1280 with a77ee frame, then tail-dispatches
  23c4/133a with a77ff frame. All242 fetched full CPU states and nine whole
  RAM boundaries retain original widths, raw/lazy flags and untouched
  registers. Actual1280 loads logical EBXf0010000, whose DS addition wraps
  to linear10000 and follows guest paging. Its current port carrier instead
  transports a host TCB pointer and omits guest RET/full CPU state. Preserve
  this unresolved coordinate/ABI distinction when threading the shared
  CPU context; the fixture does not authorize zero-filled ambient registers
  or fitted pointers. Six source-verifier negatives and6/3/3 existing clock/
  reset/config methods pass both targets; all1353 accepted runtime inputs
  remain unchanged. No production77e2/full CPU/guest-address acceptance.

- Consume0003's accepted645 actual1280 full CPU signature and resident
  RAM owner: tools/oracle/device_config_cpu_production_case.json. The
  obsolete EAX/EBX-only carrier is removed. The raw logical TCBf0010000
  and actual DWORD caller77ee RET are retained, with all37 CPU words and
  all16MiB source RAM compared on both targets. Sixteen reaching parent
  failures become16 passing configuration cases; the shared reset's four
  target cases and1029 original flag cases per target remain green. Full
 140-test/exact-patch/sequential-build/six-startup/178-flow gate and all
  additional captures/post-cleanup replays finish0. This proves the mapped
  leaf ABI and resident accesses; production task/gateway context, original
 77e2/op6c/op68, GP/flags/nonlocal returns/IRQ/time and complete output remain
  unresolved. A host pointer is still not a guest logical register value.

- Consume0003's complete original checkpoint source contract in
  tools/oracle/device_checkpoint_case.json. Actual CALL7809 targets3322
  and pushes DWORD780e; RET restores the actual ESP after two3661 calls
  and one3376 call. Every126 added CPU/code/time transition and all eleven
  full RAM boundaries are verified by the original ALU/condition owner.
 3661 supplies its index in ESI; the old DS:2f60 shim publication is not an
  original store. Eight coherent/missing negatives and6/3/3 clock/reset/
  configuration methods pass; full original output and419 files remain
  unchanged. This accepts source contracts, not production77e2 checkpoint/
  bank/task/CPU/IRQ/return transport or a new full matrix.

## Next

1. Preserve642's actual normal EAX/fullEBX and independent configuration
   input. Recover0009's remaining GP/flags/segment/stack/error contract and
   the next reached effects/device input using0003's `effects_mode_case.json`
   and0026's `sound_vector_init_case.json`. Keep all full op68 packets;
   do not mask upper words, post a fitted handle or pad unknown inputs.
   Consume accepted645 full1280 CPU/resident-RAM/guestRET and the
   complete startup-prefix inputs above. Recover the production task owner
   and its actual TCB binding when threading full CPU state through77e2;
   retain guest logical addresses rather than narrowing a host pointer.
2. Resolve live vectors to modules/offsets. Extract real prototypes with balanced parentheses and
   a declaration terminator; the earlier regex counted commas inside definitions and invented bugs.
3. Classify sites: missing/extra arguments, correct count but wrong register values, ambient-register
   calls, and far-segment constants leaked into arguments. Cover all classes; arg-less grep misses
   short nonempty lists.
4. For one screen/cockpit group, read caller and callee asm. Thread explicit register carriers with
   correct widths and a typed call. Preserve non-AX returns and CF; never pad unknown inputs with zero.
5. Drive the screen that reaches each repaired site, then run both target matrices. Use 0027's
   vehicle coverage; an AZER1 census cannot clear unseen screens.

## Accept

Every indirect site has a verified input/output contract and a reaching test or explicit unresolved
record. No native stack-residue arguments or WASM signature mismatches remain. Keep target discovery
in board:0015 and object-specific contracts in board:0019.
