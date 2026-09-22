Type: bug
Title: Every state-table method receives the dispatched object and live register lanes
Parent: 0017

## Contract

All state-table methods use the same object and live register values the original dispatcher supplies.
A DGROUP near offset is not a host pointer; Ghidra parameter positions are not a global register ABI.

## Evidence

All seven 9e2b entries (9f1d/9f66/9fe5/a069/a07c/a02a/a03f) were repaired in patches 497/498/540/542.
The 054c return contract is AX=bearing, BX=pitch, CX=range low, DX=range high. Patch 541 repaired
86e3/8711/86b8/8270/842f/844b and their dropped carries; patch 545 repaired a19e's table.
Historical probe runs and both-target matrices passed. Other tables are not cleared by that evidence.

## Next

1. Inventory remaining object/state dispatch tables together with 0015. For each record the input
   object register, other live inputs, return registers/flags and near/host representation.
2. Start with an actually reached uncovered state. Trace the caller's register writes and callee's
   first reads; repair the complete producer/consumer contract, including multi-register returns.
3. Add a scenario that enters each state normally. Forced-LOS probes are diagnostic evidence only;
   preserve default-path verification after removing the probe.

## Accept

Every state table is audited; exercised transitions read the intended object and return correct
flags on both targets. Keep generic call-signature work in 0014 and missing target promotion in 0015.
