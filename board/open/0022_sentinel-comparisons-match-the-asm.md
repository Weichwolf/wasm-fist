Type: bug
Status: open
Parent: 0012
Title: every sentinel comparison uses the asm's constant at the asm's width

Ghidra renders a comparison against a sentinel as `== -1` regardless of the variable's declared type.
That is correct for an `undefined4`/`uint` -- `-1` converts to `UINT_MAX` -- and a CONSTANT FALSE for
an `undefined1` or `undefined2`, which promote to `int` under the usual arithmetic conversions and can
never equal -1. Wherever the second case occurs the guard is dead and the code it protects, or the code
it skips, never runs.

Patch 548 closed the nine sites on DGROUP:0x6da2 (the mission teardown countdown) after each was
matched to a `cmpw $0xffff,0x6da2` in the image. The class is not exhausted. What remains, with the
asm width where it is already known:

| variable            | DGROUP  | sites | asm width | sentinel | status |
|---------------------|---------|-------|-----------|----------|--------|
| `DAT_2000_2da2`     | 0x6da2  | 9     | WORD      | 0xffff   | FIXED -- patch 548 |
| `DAT_2000_5fd9`     | 0x9fd9  | 4     | WORD      | 0xffff   | FIXED -- patch 551 |
| `DAT_2000_2db6`     | 0x6db6  | 1     | WORD      | 0xffff   | FIXED -- patch 551 |
| `DAT_2000_5fd6`     | 0x9fd6  | 2     | BYTE      | 0xff     | FIXED -- patch 550 (narrowed too) |
| `DAT_2000_a918`     | 0xe918  | 3     | BYTE      | 0xff     | FIXED -- patch 552 (narrowed too) |
| `DAT_2000_a82e`     | 0xe82e  | 1     | WORD      | 0xffff   | FIXED -- patch 557 |
| `DAT_1000_d8e4`     | 0x18e4  | 1     | WORD      | 0xffff   | FIXED -- patch 557 |
| `DAT_1000_e02e`     | 0x202e  | 1     | NOT LOCATED | ?      | the only one left -- see below |

The `DAT_1000_` two were locatable once patch 553 established that `DAT_1000_XXXX` with XXXX >= 0xc000
is DGROUP XXXX-0xc000: `DAT_1000_d8e4` is DGROUP:0x18e4, the ready-queue head, with `cmpw $0xffff`
at 0x13915 inside `FUN_1000_3907`.  That one was not cosmetic -- it decides the carry `FUN_0000_e714`'s
idle loop branches on, so the port reported "event queue non-empty" when the queue was empty.

### The one that remains: `DAT_1000_e02e` (DGROUP:0x202e)

It has NO plausible absolute-displacement reference in the image, so its width cannot be read the way
the other seven were; it is reached through a base register or is a register-modelled temporary.  It
also carries the `< 0` subclass and sits in a 32-bit compare helper Ghidra otherwise treats as signed
(SBORROW2, plus an `(int)` cast on ONE of its two `< 0` tests and not the other).  Width and signedness
have to be settled together, from the containing function.

The three settled ones, with their evidence:

- **`DAT_2000_5fd9` (0x9fd9) is a WORD.** The image contains exactly seven references to the
  displacement, and every one is word-width: `movw $0xffff,0x9fd9` at 0xbddd and 0xbe6e,
  `mov 0x9fd9,%bx` at 0xbe00, `cmp 0x9fd9,%bx` at 0xbe0e, `cmpw $0xffff,0x9fd9` at 0xbe14 and 0xc096,
  `mov %bx,0x9fd9` at 0xbe25.  The port's four `== -1`/`!= -1` sites map onto the two `cmpw $0xffff`
  compares (two pairs of Ghidra functions share a tail, as `FUN_0000_bee2`/`FUN_0000_bee8` do).
  The declaration is already right; only the constant is wrong.
- **`DAT_2000_5fd6` (0x9fd6) is a BYTE, and it overlaps a live word.**

        beb7: 80 3e d6 9f ff    cmpb  $0xff,0x9fd6
        bebc: 74 1d             je    0xbedb
        bebe: a1 d7 9f          mov   0x9fd7,%ax        <- 0x9fd7 is a WORD, DAT_2000_5fd7
        bec1: 3b 06 de 6c       cmp   0x6cde,%ax
        bec7: a0 d6 9f          mov   0x9fd6,%al
        bed6: c6 06 d6 9f ff    movb  $0xff,0x9fd6
        bedc: c6 06 d6 9f ff    movb  $0xff,0x9fd6
        beef: a2 d6 9f          mov   %al,0x9fd6

  Declared `undefined2`, so `DAT_2000_5fd6 = 0xff` writes 0x00ff and zeroes 0x9fd7 -- the word the very
  next test reads.  It must be narrowed AND its two `-1` constants changed to 0xff.
- **`DAT_2000_2db6` (0x6db6) is a WORD** (`incw`/`decw`/`cmpw`, `movw $0xffff` at 0x1bef3), sentinel
  0xffff, shown at 0x1bf1d below.

`DAT_2000_2db6` is the clearest of them. It is a genuine word, and the port cycles it backwards like
this:

    DAT_2000_2db6 = DAT_2000_2db6 + -1;
    if (DAT_2000_2db6 != -1) break;        /* always true -> the break ALWAYS taken */
    DAT_2000_2db6 = DAT_2000_2dd0;         /* wrap to the end -- unreachable */

so the wraparound arm of a list-cycling loop is dead code. The asm is unambiguous:

    1bf16: ff 0e b6 6d       decw  0x6db6
    1bf1a: a1 b6 6d          mov   0x6db6,%ax
    1bf1d: 3d ff ff          cmp   $0xffff,%ax
    1bf20: 75 08             jne   0x1bf2a          ; no underflow -> use this index
    1bf22: a1 d0 6d          mov   0x6dd0,%ax       ; underflowed -> wrap to the end ...
    1bf25: a3 b6 6d          mov   %ax,0x6db6
    1bf28: eb ec             jmp   0x1bf16          ; ... and step again

There is a second, smaller subclass with the same cause: a bare `DAT_... < 0` on an unsigned macro,
which is also a constant false. Four sites: `DAT_1000_e02e` (x2), `DAT_2000_2d18`, `DAT_1000_c004`.
`DAT_1000_e02e` carries both forms and sits in a 32-bit compare helper that Ghidra otherwise treats as
signed (`SBORROW2`, and a `(int)` cast on one of the two `< 0` tests but not the other), so its width
and signedness have to be settled together.

"Not yet located" above means not found by a flat linear sweep of the images (`objdump -D -b binary
-m i8086`), which desynchronizes wherever code and data interleave and therefore cannot prove absence.
The reliable route for those is to disassemble the CONTAINING function from its known entry address --
the port's function name carries it -- rather than to grep the sweep.

Each site needs its own asm check before it is touched -- the constant may be `0xff` or `0xffff`, and
in the byte cases the declaration has to be narrowed first or the repaired comparison is still wrong.
`DAT_2000_2b74 == -1` (x2) is NOT in this class: it is declared `int **`, so it is a pointer sentinel
and a separate question.

DONE when: every one of these sites has been matched to its instruction in the image and either
corrected by an asm-verified patch or shown to be correct as written, and the tree contains no
comparison of an unsigned decompiled variable against a negative constant.
