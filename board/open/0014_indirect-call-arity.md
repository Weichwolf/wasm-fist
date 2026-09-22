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
