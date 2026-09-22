Type: bug
Title: Every data-driven dispatch target exists and resolves correctly

## Contract

Every target stored by the original's dispatch tables resolves to the correct implementation.
A missing target must not silently preserve stale flags or masquerade as an original no-op.

## Evidence

c4df's four STRSEG tables at 0x3632/0x35fa/0x35c2/0x358a exposed unpromoted render methods.
Patches 533–556 and 581 added them, including the formerly blocked c783. Patch 565 sorts the map
before binary search; hand-inserted map entries had previously made present functions unreachable.
Patch 582 repaired near-hook/RNG polling, BIOS chaining and verdict handlers. AZER1/debrief and
TRAIN2 then produced no traps in recorded runs; this is not an exhaustive dispatch census.

## Next

1. Enumerate original table entries, compare against current definitions and resolver maps, and
   verify the entry address is a function or intentional mid-entry in the right CS window.
2. Extend beyond c4df: 22dd's script table (DGROUP:0x6c82), element vectors (0x3e18), c33c phase
   methods and the extender table. Check downstream consumers when enabling list producers.
3. Reproduce residual candidates: 390d's line renderer; c47d/c4a2's record/CF protocol;
   97d5/902c late sub-dispatches; buffered formatters behind DGROUP:0x4a8..0x4ec (55c5..562d).
   Their old trap sightings are leads, not proof they still fail on the current tree.
4. Use `FIST_TRACE_TRAPS` and `FIST_TRAP_BREAK=<linear>` across missions, views and editor paths.
   Recover each missing target from asm, then satisfy its argument contract under 0014/0019.

## Accept

All enumerated table targets resolve correctly and exercised scenarios contain no unexplained
traps. Verify CF=1 `stc; ret` methods as genuine original behavior rather than treating them as stubs.
