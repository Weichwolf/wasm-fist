Type: refactor
Title: Segment context is explicit without breaking the reproducible patch base

## Contract

Resolve actual CS/DS/ES values and preserve correct segmented pointer bases through decompilation,
assembly and patching. Never replace undefined segment reads with arbitrary zero initialization.

## Evidence

`tools/decompile.sh` already includes SetCSContext, INT-return ES/DS threading and SegmentFixup on
fresh analysis. Reusing a saved project exports it without rerunning that pass sequence.
The committed patch base predates some pipeline improvements; the historical rebase experiment
produced widespread signature/body conflicts. Context fixes do not subsume pointer-basing patches.
Patch 610 resolved CS reads in the existing tree; board:0012 records its bounded parity evidence.
Remaining ES and other unresolved lanes need per-use evidence, not a blanket CS-cluster rule.

## Next

1. Reproduce an actually reached unresolved segment read and its original register value. Prefer a
   bounded correction in the existing patch base when it closes a real failing flow.
2. If fresh output is needed, use a separate output/project location. Diff signatures, globals and
   patch applicability before touching committed generated sources.
3. Recommendation: migrate complete interdependent signature/body groups. Never skip nonapplying
   patches or use fuzz to make an incomplete migration compile.
4. For each group run patch checks, both target matrices, the relevant SIMHASH comparison and seeded
   oracle replay. Segment values can change output even when both compilers agree.

## Accept

All changed context values have asm/runtime evidence; no required correction is lost in regeneration.
The full patch chain reproduces and affected original/native/WASM behavior matches.
