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

- Actual original3446-to46b6 keyboard-hook exchange disproves the generic
  claim in610 that a FUN1000 name implies runtime/rebased CS1000. The original
  executes CS2082 with main load1119, giving rebased CS0f69 and actual callback
  owner image14628/1462a. Four source cases prove explicit incoming ES, full
  pointer exchange and old AX/BX/ES outputs, not a blanket zero/cluster seed.
  Source owner0030 and compact fixture `tools/oracle/callback_exchange_case.json`.
  Existing610 target-parity evidence does not establish every original segment.

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
