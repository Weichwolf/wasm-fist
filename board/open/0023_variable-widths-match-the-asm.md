Type: bug
Title: Every access uses the original width and preserves neighboring bytes
Parent: 0012

## Contract

Match original byte/word/dword loads, stores and arithmetic, including overlapping text-buffer writes.
A declaration change is not sufficient when Ghidra merged multiple original stores.

## Evidence

Patches 539/544/546–553 corrected aim, speed, outcome and adjacent-byte corruption. Later seeded
replay fixes (593–595) repaired heading/turret/counter/product widths. Both too-wide and too-narrow
accesses occur. The historical “harmless dead successor” set was only a heuristic classification.

## Next

1. Re-run `python3 tools/width_triage.py --byte` and `--dword` against freshly patched sources.
   Inspect instructions for each candidate; raw displacement hits and nearby 0x66 are not proof.
2. Start with DGROUP 0x1f91/0x1f92 in the ASCII buffer at 0x1f8c. Restore the actual overlapping
   byte/word store sequence, including the shift at 1a342..1a349; do not simply narrow macros.
3. Re-audit residual dword candidates at 0x6cc2/0x6cc4/0x5951/0x8d52/0xe902, plus any newly found
   ones. Preserve genuine four-byte far pointers (LES/LDS/lcall/ljmp do not need a 0x66 prefix).
4. Include both symbol ranges: DAT_2000_xxxx maps to DGROUP 0x4000+xxxx; DAT_1000_xxxx with
   xxxx>=c000 maps to xxxx-c000. Examine indirect accesses before declaring a successor unused.
5. Use the first differing seeded-replay object byte to prioritize active defects. Verify buffer
   neighbors and both targets after every asm-backed correction.

## Accept

Each candidate is corrected or justified; the merged text stores and remaining byte-only tail are
explicitly accounted for. No parser heuristic alone authorizes a width change or an “unused” exemption.
The arctangent's historical random-input match does not clear unrelated input-width defects.
