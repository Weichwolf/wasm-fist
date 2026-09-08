Type: bug
Status: open
Parent: 0012
Title: every decompiled variable's declared width matches its asm access width

Ghidra infers a variable's width from how it happens to see it used, and gets it wrong often enough
that this is the single most productive defect class in the tree. A too-WIDE declaration is the
damaging direction: the macro then overlaps its neighbours, so every store corrupts an unrelated live
variable and every load mixes foreign bits into the test.

Landed instances, each an asm-verified patch:

| patch | address           | declared | asm      | what it broke |
|-------|-------------------|----------|----------|---------------|
| 539   | 0x9684 + 5 more   | word     | dword    | AI aim vectors; launch aim error -139 deg -> +0.01 deg |
| 544   | obj+0x34          | dword    | word     | the speed-limit index; units never moved |
| 546   | 0xe814            | word     | byte     | the mission-over flag also wrote 0xe815 |
| 547   | 0x6da4..0x6db0    | word x13 | byte x13 | the mission clock's borrow chain; no mission could time out |
| 549   | 0x6d38..0x6d3b    | word/dword| byte/word| the defeat outcome demanded BOTH sides be wiped |
| 550   | 0x978c, 0x9f1e, 0x9fd6 | dword/word | word/byte | 0x978c overwrote the victory counter at 0x978e |
| 552   | 48 addresses      | word x48 | byte x48 | the systematic pass -- see below |
| 553   | 0x1549..0x154f, 0x044a, 0x1e7a | word x6 | byte x6 | DGROUP's first 16 KB, missed by 552 |

The direction is not uniform -- 539 and 544 point opposite ways -- so each case has to be read off the
instruction, never assumed.

## The open sweep

`tools/width_triage.py` makes this reproducible. It scans every `DAT_2000_*` macro in `build/fist.c`,
finds each one's 2-byte absolute displacement in the engine image, and classifies every reference by
the opcode immediately preceding it (modrm /06 and moffs forms; a 0x66 prefix within the preceding
five bytes marks 32-bit). The heuristic validates against patch 539, whose `DAT_2000_5684` shows 11 of
13 references genuinely 32-bit.

    python3 tools/width_triage.py --byte     # undefined2 that is only ever accessed as a byte
    python3 tools/width_triage.py --dword    # undefined4 with no 32-bit access anywhere

Output BEFORE patch 552:

    undefined2 macros accessed only by BYTE: 254  (128 with no unclassified reference, 48 of those with a live successor)
    undefined4 macros with NO 32-bit access: 9

and after it:

    undefined2 macros accessed only by BYTE: 206  (80 with no unclassified reference, 0 of those with a live successor)
    undefined4 macros with NO 32-bit access: 7

**The harmful byte set is closed.** Patch 552 narrowed all 48 macros that were declared a word, are
only ever accessed as a byte, have NO reference the scanner cannot decode, and have a live byte
immediately after them. Several were adjacent runs -- 0x87ca..0x87e7, 0x8aae..0x8ab1, 0x8e46..0x8e55,
0x6c42..0x6c45 -- where each member aliased the next, exactly like DGROUP:0x6da4..0x6db0.

What deliberately remains:

- **206 byte-only `undefined2` macros with a DEAD successor.** Still wrong, but by the available
  evidence they corrupt nothing: the next byte is never referenced in the image. That evidence is the
  same reference heuristic used everywhere here, so it is not proof -- a neighbour reached only through
  a base register would not show up. Low priority, not zero.
- **80 of those pass the strict no-unclassified test**; the other 126 have at least one reference the
  scanner cannot decode and must not be touched on this evidence alone.
- **7 `undefined4` macros with no 32-bit access**: `_DAT_2000_aa2c` (0xea2c, 81 refs -- patch 524 calls
  this a host-pointer macro, so check that first), `_DAT_2000_2cc2`, `DAT_2000_1951`,
  `_DAT_2000_2cc4`, `DAT_2000_aa16`, `DAT_2000_4d52`, `_DAT_2000_a902`.

The `succ` column is the priority order: a too-wide byte is harmless until its successor is live.

Macros declared dword with ZERO 32-bit references:

| macro              | DGROUP  | refs | 32-bit | note |
|--------------------|---------|------|--------|------|
| `_DAT_2000_578c`   | 0x978c  | 4    | 0      | CONFIRMED -- see below |
| `_DAT_2000_aa2c`   | 0xea2c  | 85   | 0      | patch 524 calls this a host-pointer macro; check that first |
| `_DAT_2000_a902`   | 0xe902  | 9    | 1      | |
| `_DAT_2000_2cc2`   | 0x6cc2  | 7    | 0      | |
| `DAT_2000_1951`    | 0x5951  | 7    | 0      | |
| `_DAT_2000_2cc4`   | 0x6cc4  | 5    | 0      | |
| `DAT_2000_aa16`    | 0xea16  | 4    | 0      | |
| `_DAT_2000_5f1e`   | 0x9f1e  | 3    | 0      | byte AND word access, never dword -- see below |
| `DAT_2000_259a`    | 0x659a  | 3    | 0      | |
| `DAT_2000_4d52`    | 0x8d52  | 3    | 0      | |
| `DAT_2000_272a`    | 0x672a  | 1    | 0      | |

This is triage, not proof: the byte-pair search also hits data, and a prefix further than five bytes
away is missed. Each entry needs its instruction read before it is touched.

### `_DAT_2000_578c` (0x978c) -- confirmed, and it corrupts the victory counter

    ab0a: a3 8c 97          mov  %ax,0x978c        ; `a3` = mov %ax,moffs16 -- TWO bytes
    afbb: f6 06 8c 97 03    testb $0x3,0x978c      ; read by byte
    afd6: a0 8c 97          mov  0x978c,%al        ; read by byte

Four references, all byte or word. Declared `undefined4`, so the port's single store
`_DAT_2000_578c = FUN_0000_0291();` writes 0x978c..0x978f and overwrites **0x978e -- the live-object
count that FUN_1000_a5dc's victory test reads** (0x1a624 zeroes it, 0x1a636 counts it, 0x1a694 tests
it). It does not corrupt a5dc's own decision, since a5dc zeroes, counts and tests within one call with
no engine code in between, but it does corrupt the leftover value that 0x1a618 copies to `prev`
(0x9792), which gates the count-decreased notification at 0x1a66b. See board:0017.

### `_DAT_2000_5f1e` (0x9f1e) -- byte and word, never dword

    bde4: 80 0e 1e 9f 20    orb   $0x20,0x9f1e
    bdeb: f6 06 1e 9f 10    testb $0x10,0x9f1e
    be37: 81 0e 1e 9f 20 03 orw   $0x320,0x9f1e

A word flag field updated by byte in one place and by word in another. As a dword every `orw` also
touches 0x9f20/0x9f21.

## Two corrections the sweep itself forced

Both are recorded because each would have produced a WRONG patch, and both were caught by making the
scanner say what it could not decode rather than assuming silence meant absence.

**1. Half of DGROUP was invisible.** Ghidra names a DGROUP variable by the segment it guessed, so one
variable space has two spellings:

    DAT_2000_XXXX  ->  g_mem+0x2XXXX  ->  DGROUP offset 0x4000+XXXX    (DGROUP 0x4000..)
    DAT_1000_XXXX  ->  g_mem+0x1XXXX  ->  DGROUP offset XXXX-0xc000    (DGROUP 0x0000..0x3fff)

The scanner knew only the first, so patch 552's sweep skipped the whole first 16 KB.  The port's own
patch comments prove the mapping: `DAT_1000_d8e4` is the ready-queue head the asm calls `[0x18e4]`,
and `DAT_1000_fe00` is `[0x3e00]`.  Adding the second spelling found 52 more byte-only macros and six
more with a live successor -- among them **DGROUP:0x1549, the mission-state discriminator** (0x1c in
the cockpit; the shim reads it to know it is in a mission), whose fourteen references are all
byte-width and whose successor 0x154a is a live byte with twelve of its own.  Patch 553.

**2. Far pointers are four bytes without a 0x66 prefix.** `lcall *m16:16` (`ff 1e`), `ljmp *m16:16`
(`ff 2e`) and LES/LDS all read four bytes with no operand-size prefix.  Without them the scanner
reported the SERVICE-VECTOR TABLE at DGROUP:0x0e..0x36 -- 4-byte far pointers, 77 references on
DGROUP:0x0e alone -- as having "no 32-bit access", i.e. as a candidate for narrowing to two bytes.
Narrowing it would have broken every engine service call.  Adding the far forms and filtering the
`--dword` report on unclassified references collapses that list from 36 to 13, of which only 2 are
strictly clean; `_DAT_2000_aa2c`, which patch 524 identifies as a host-pointer macro, correctly drops
off it.

## DGROUP:0x1f91 / 0x1f92 -- looked at, and deliberately NOT patched

These are the only two `undefined4` macros that pass the strict filter, and they overlap each other,
which is the signal that they are not scalars at all. They are part of a small ASCII buffer at
DGROUP:0x1f8c -- `FUN_1000_a5dc` itself calls `FUN_0000_02e8(count, &DAT_1000_df8c)`, a number-to-string
conversion into it -- and the constants are format bytes: 0x25 = '%', 0x4d = 'M', 0x2e = '.'.

    16b2d: c7 06 91 1f 25 00   movw $0x0025,0x1f91
    16b3c: c7 06 92 1f 4d 00   movw $0x004d,0x1f92
    1a746: c7 06 91 1f 17 00   movw $0x0017,0x1f91
    1a7b2: c6 06 91 1f 25      movb $0x25,0x1f91
    1a7b7: c7 06 92 1f 0c 00   movw $0x000c,0x1f92
    1a342: a1 92 1f            mov  0x1f92,%ax
    1a345: a3 93 1f            mov  %ax,0x1f93      <- an overlapping shift, not a scalar access
    1a349: c6 06 92 1f 2e      movb $0x2e,0x1f92

Ghidra merged adjacent stores into single 4-byte writes and expressed the partial ones with CONCAT:

    _DAT_1000_df91 = CONCAT12((*(undefined1 *)(g_mem+0x1df93)),0x25);   /* for movw $0x25,0x1f91 */
    _DAT_1000_df91 = 0x4d25;                                            /* two movw merged into one */
    _DAT_1000_df92 = CONCAT21((int)_DAT_1000_df92,0x2e);                /* for movb $0x2e,0x1f92 */

Each writes at least one byte more than the instruction does -- 0x1f94 in the first two cases. That is
the same defect class, but the repair is not a declaration change: it is un-merging the stores back to
what the asm does, byte for byte, in a text buffer where a wrong byte changes displayed characters. It
needs the containing functions read in full, not a triage verdict, so it is left for its own patch.

DONE when: every `undefined4` above is either corrected by an asm-verified patch or shown to be
genuinely 32-bit, DGROUP:0x1f91/0x1f92 are un-merged as described, and the byte-only tail with dead
successors is either narrowed or explicitly accepted with the reason recorded.
