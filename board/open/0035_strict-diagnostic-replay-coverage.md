Type: bug
Title: Diagnostic memory and RNG replay reports cannot hide missing coverage
Parent: 0017

## Evidence

Source review: replay_cmp.py compares min(oracle,port steps) and can pass an equal prefix with a
missing suffix. replay_objdiff.py breaks at a missing dump without marking failure and excludes
MASK_VEH bytes. These are limited diagnostics, not frame/audio acceptance proofs.
DOSBox word/dword hooks emit per-byte callbacks before the store; portable memory-access branches
lack the hooks. Physical watches have caps; last-writer tables are not exhaustive ordered journals.

## Contract

Diagnostic reports state exact compared scope and fail missing expected records. They help locate
an output defect without imposing identical internal host representations on the port.

## Next

1. Test empty, truncated, missing, extra and reordered schedules/dumps. Require matching explicit
   step identities and completion rather than accepting an overlapping prefix.
2. Keep masks explicit in reports. Provide raw differing bytes on demand; never call a masked result
   full-state equality. Distinguish a deliberate bounded diagnostic from a complete scenario.
3. Before relying on an all-write journal, audit bulk/DMA/translated-core stores, record caps and
   instruction grouping; preserve sequence, guest address, old/new bytes and execution context.

## Accept

Incomplete input fails; bounded/masked diagnostics declare their limits. Frame/audio acceptance
remains board:0033/0034. Do not require identical optimized host-store sequences.
