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

Current output:

    undefined2 macros accessed only by BYTE: 255  (of which 100 have a live successor)
    undefined4 macros with NO 32-bit access: 9

**One hundred macros are declared a word, are only ever accessed as a byte, and have a live byte
sitting immediately after them.** Each of those is a store that corrupts an unrelated variable, the
same shape as the mission clock (patch 547) and the side counters (patch 549). Several are adjacent
RUNS -- 0x8aad/0x8aae/0x8aaf/0x8ab0, 0x7b1e/0x7b1f, 0xf885/0xf886, 0x9c7d/0x9c7e, 0x87d5/0x87d6 --
where each member aliases the next, exactly like DGROUP:0x6da4..0x6db0.

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

DONE when: every `undefined4` above is either corrected by an asm-verified patch or shown to be
genuinely 32-bit, and the same triage has been run over the `undefined2` macros for the byte case that
patches 546/547/549 kept finding.
