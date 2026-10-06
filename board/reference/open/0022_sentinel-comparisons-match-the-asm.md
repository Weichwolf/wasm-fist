Type: bug
Title: Sentinel and signed comparisons use the original widths and constants
Parent: 0012

## Contract

Preserve the original compare operands, width and signedness, including underflow wrap behavior.

## Evidence

An unsigned byte/word promotes to int and cannot equal -1; an unsigned dword compared to -1 can
legitimately test its all-ones sentinel. Do not replace every negative constant mechanically.
Patches 548/550/551/552/557 corrected known sentinels at DGROUP 6da2/9fd6/9fd9/6db6/e918/e82e/18e4.

## Next

1. Locate the containing function for `DAT_1000_e02e` (DGROUP 0x202e), which still had a sentinel
   and signedness question. Read from the function entry; failed flat disassembly searches do not
   prove the variable has no original reference.
2. Audit bare unsigned `< 0` candidates `DAT_1000_e02e`, `DAT_2000_2d18`, `DAT_1000_c004`.
   Settle actual signedness and access width together with board:0023.
3. Distinguish `DAT_2000_2b74` pointer-sentinel uses from integer comparisons; verify their original
   representation separately. Retain legitimate unsigned-dword all-ones comparisons.
4. Drive boundary cases (empty list, decrement through zero, negative signed value) on both targets
   and compare the branch/store effects against asm or the original trace.

## Accept

Every candidate is corrected or justified by an original instruction. No dead comparison caused by
integer promotion remains; constants and declarations use the asm width without corrupting neighbors.
