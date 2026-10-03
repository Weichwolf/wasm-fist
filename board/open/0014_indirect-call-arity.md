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

## Next

1. Resolve live vectors to modules/offsets. Extract real prototypes with balanced parentheses and
   a declaration terminator; the earlier regex counted commas inside definitions and invented bugs.
2. Classify sites: missing/extra arguments, correct count but wrong register values, ambient-register
   calls, and far-segment constants leaked into arguments. Cover all classes; arg-less grep misses
   short nonempty lists.
3. For one screen/cockpit group, read caller and callee asm. Thread explicit register carriers with
   correct widths and a typed call. Preserve non-AX returns and CF; never pad unknown inputs with zero.
4. Drive the screen that reaches each repaired site, then run both target matrices. Use 0027's
   vehicle coverage; an AZER1 census cannot clear unseen screens.

## Accept

Every indirect site has a verified input/output contract and a reaching test or explicit unresolved
record. No native stack-residue arguments or WASM signature mismatches remain. Keep target discovery
in board:0015 and object-specific contracts in board:0019.
