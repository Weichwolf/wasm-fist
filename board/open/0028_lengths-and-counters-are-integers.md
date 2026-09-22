Type: bug
Title: Lengths and loop counters use original integer arithmetic
Parent: 0012

## Contract

Represent lengths/counts as integers with the original wrap width; no termination condition may
rely on invalid pointer arithmetic that optimizing WASM can fold away.

## Evidence

Patch 589 repaired 21bd's basename scan/copy: Ghidra pointer counters caused oversized scans and
an unbounded WASM copy. Patch 593 fixed b355's word counter/throttle. Casting an integer value to
a pointer is not the same defect as using a pointer as a loop bound; classify by use.

## Next

1. Search patched engine C for pointer comparisons to small constants, decrement-to-null loops and
   pointer differences reused as pointers. Historical clusters: 2891, 1bd8, ae3c, 4e5b, 4237,
   b7c4, 56f8, fd79, 2e0d, 1f43, 4268, 32a6, 2486; rebuild the current list.
2. Read each scan/cmp/loop in asm and recover integer width, initial count and zero-count behavior.
   Recommendation: keep cursor offsets and counts separate; wrap offsets where the CPU wraps them.
3. Exercise empty input, zero/full count and wrap boundaries on both compilers. A 16-bit LOOP with
   initial CX=0 executes 65536 iterations; adding a zero guard changes the original behavior.

## Accept

No reached length/count computation depends on invalid pointer arithmetic. Boundary behavior,
stored bytes and termination match asm on native and WASM; residual candidates are explicitly audited.
