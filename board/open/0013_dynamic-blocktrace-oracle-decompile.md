Type: tooling
Area: oracle
Tags: dosbox blocktrace decompile flight-model overlay cfg smc icall determinism

The port can reconstruct the actually-executed extender flight/combat model (the overlay + the CS
clusters that static Ghidra cannot resolve) from a DYNAMIC basic-block trace of the patched DOSBox --
one AZER1 self-play run yields every executed basic block, deduplicated, as a {hash}.asm file, plus the
jump-target edges that reassemble the complete executed program.

## Why this closes the gap static decompile cannot (idea: Cosmo, 2026-08-27)

Static Ghidra fails systematically on exactly the four classes that gate board:0012/0001/0010:
  - indirect control flow (call [tbl], jmp [reg], lcall [mem]:off)  -> the open fist_icall / op-0x4c work
  - self-modifying code (the engine's SMC dispatch / cb-list)
  - the overlay @ 0x100000 + the CS clusters (0x1000/0xf69/0x2082/0xf000/e000) not in the static images
  - CS<->linear window assignment (board:0010, guessed by SegWrapFixup)
A dynamic block trace resolves ALL FOUR by observation: the real indirect target is executed; each SMC
variant appears as (same addr, different bytes); overlay code is captured because it runs; CS + cr3 are
recorded per block.

## Design (refined)

- BASIC BLOCK = branch target (or post-branch) .. next control-flow op.  Hook the CPU decode path
  (normal-core: record opcode bytes + cs:eip per decoded insn, close the block at a branch; dynamic-core:
  at block-translation entry).  The memory hooks (fist_memrec in mem.h) already exist; this is the
  instruction-fetch sibling.  Env-armed like the other FIST_* traces.
- DEDUP KEY = hash(block instruction bytes).  The bytes ARE the code -- stable across loop iterations /
  re-invocations (register/memory VALUES are not in the byte stream, so nothing needs to be "stripped").
  Loop body -> same hash every iteration -> ONE file.  SMC is the only exception and is wanted: key =
  (start_cs:eip, hash(bytes)) so SMC variants surface as same-address / different-bytes.
- EDGES (the "reconstruct the whole program from jump addresses" point): per block record
  (cs:eip_start, cs:eip_end, exit_type in {jmp, jcc->(taken,fall), call->(callee,return), ret, int}, target[s]).
  The jump targets are the CFG edges -> function bounds (call targets = prologues, ret = epilogues) ->
  the entire EXECUTED program.  Indirect edges get their concrete runtime targets.
- CONTEXT per block: CS + effective linear base (solves the CS-cluster assignment) + cr3 (overlay paging).
- COMPLETENESS: only executed paths.  Criterion: while a run emits NEW {hash}.asm, coverage is open; a
  run that emits none = the reachable code is captured.  AZER1 self-play covers flight/LOS/combat/damage;
  targeted scenarios (each weapon, each unit type, win AND lose) close the edges.

## The determinism/falsifiability bonus (the goal demands it)

The trace IS the oracle reference.  Combined with the existing memrec write-trace, each block carries not
only its bytes but its emitted WRITE SEQUENCE (who writes what where).  A ported C block is then verified
against (a) the block bytes (structure) AND (b) the write sequence tick-for-tick (behaviour) -- exactly
where the CR3-paged oracle blocked direct verification before.

## Honest scope

NOT a one-click resolver.  It replaces GUESSING (indirect edges, SMC, overlay) with OBSERVATION; porting
the captured blocks to faithful C + verifying stays real work (like the 424 patches).  But it converts the
remaining flight-model reconstruction from "undecompiled / guess" to "observed / port", and in ONE run
answers what static instrumentation could not (which dispatch + spawn method AZER1 really fires; what sets
[obj+0xc] unit-Z per frame -- the real computation, NOT the forbidden stand-in approximation; the
projectile flight/damage path; and it confirms/corrects the statically-decoded op-0x58 LOS @0x802e).

## First deliverable

Extend third_party/dosbox-fist with the block-trace hook (FIST_BLKTRACE=<dir> [FIST_BLKTRACE_CS=..] to
scope to the flight-model clusters), run AZER1 self-play once, and reassemble the {hash}.asm + edge list.
Then port the flight-model blocks (unit-Z/dynamics, fire dispatch->spawn, projectile flight, damage) as
asm-verified patches, verified against the block bytes + write sequence.  This is the concrete tool for
finishing board:0012 (and it retires the "oracle is CR3-blocked" excuse that recurs across the board).
