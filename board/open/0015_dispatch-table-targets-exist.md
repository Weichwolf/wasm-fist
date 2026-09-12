Type: bug
Area: decompile
Tags: unpromoted dispatch render
Title: every function named by a dispatch table exists in the port and is reachable

Every function named by one of the engine's data-driven dispatch tables exists in the port and is
reachable through the fist_icall map, so no table entry resolves to nothing.

## What was found

FUN_0000_c4df renders each object in the display list by looking its method up in a per-VIEW table:

    d549 == 0x1e (cockpit) -> word[STRSEG : classid*2 + 0x3632]
    d549 == 0x20           -> word[STRSEG : classid*2 + 0x35fa]
    d549 == 0x22           -> word[STRSEG : classid*2 + 0x35c2]
    otherwise              -> word[STRSEG : classid*2 + 0x358a]

Reading all four tables out of a live memory dump (STRSEG = word[DGROUP:0x70], 0x2d74 in the runs
measured) gives 24 distinct targets across 4 views x 24 classes.  **Ten of the 24 did not exist in the
port at all** -- neither defined in build/fist.c nor present in the fist_icall fmap.  Ghidra never
promoted them, so the dispatch resolved to nothing:

| view | missing when found | class slots |
|---|---|---|
| 0x1e cockpit | c829, c86e, (c894), c715, c774 | 12 |
| **0x20** | c713, c74d, c77d, c7a5, c817, c82d, c874 | **18 of 24** |
| 0x22 | c783 | 6 of 24 |
| default | none | 0 |

View 0x20 was missing three quarters of its object rendering; view 0x22 was missing its only non-default
target.

## How it was found, and why the earlier sweep missed it

TRAIN4's SIGSEGV (after patch 531 moved it) landed inside FUN_0000_f8e9 -- which is not a function
entry at all but a loop head inside an interrupt-disabled list-splice routine.  A render method cannot
be that, so the vector was resolving to nothing.  Measuring at the fault (g_fist_render_si=0xb0ad,
classid=13, d549=0x1e, STRSEG=0x2d74) and reading the table gave 0xc86e, which the port did not have.

board:0001's log records the trap-trace method as EXHAUSTED for the un-seeded-function class.  That
conclusion holds for what it measured, but it cannot see this: these functions are reached through a
DATA table, so nothing traps at a recognisable un-seeded entry -- the dispatch simply goes somewhere
wrong.  The audit that does see it is static and takes seconds: read the four tables, collect the
distinct targets, check each against the port.

## Landed so far

    patch 533   c86e                      8 class slots (cockpit classes 7..14)
    patch 534   c829 + c894               2 class slots (cockpit classes 5..6)
    patch 535   c713 c874 c77d c82d c817  16 class slots (all view 0x20)

Each was reconstructed from an established template -- the c70d convention (patch 463: si =
g_fist_render_si, bx = classid*2 live from c4df, bp -> 0), c894, or the plain CF protocol.

Note that "stc ; ret" is the genuine body of several of these, not a stub: CF=1 is the object-method
protocol for "nothing drawn, keep walking" (threaded via g_fist_cf by patches 310/463).

## Remaining

    patch 556   c715 c74d c774 c7a5       4 class slots (2 view 0x1e, 2 view 0x20)

Both things this section listed as "not established yet" turned out to be readable rather than open:

- **The c8c2 argument convention.**  c8c2 forces `AL=0x10` itself (`c8cc: b0 10`) and takes only AH
  from its caller -- the port's patch-310 body already renders exactly that as
  `CONCAT11((char)(param_1 >> 8), 0x10)`.  So `c774`'s inherited AL is a DON'T CARE, and `c715`
  (`mov %ax,%bx ; xor %ah,%ah`) is fully determined.  Neither needed a new convention.
- **The STRSEG deferred-object list write.**  The asm spells it out, and the port already performs the
  same GS/STRSEG store in `FUN_0000_5fb0` (patch 526) and `FUN_1000_a5dc`:

      c75b: mov 0xe6e0,%bx ; cmp $0x40,%bx ; jae ...    ; count, bounded at 0x20 entries
      c764: mov 0x70,%gs   ; mov %si,%gs:0x366a(%bx)
      c76d: addw $0x2,0xe6e0

  `c74d` is entirely this append and always returns CF=1 -- it has no drawing arm at all -- and it is
  also `c715`'s else-branch.

### Still open: c783 (view 0x22, 6 slots)

This one is genuinely blocked, and on a different thing than this item first supposed.  It sets AL and
inherits AH:

    c79d: b0 0e     mov  $0xe,%al
    c79f: e8 10 01  call 0xc8b2

`c8b2` forces `AL=0x12`, so c783's own `mov $0xe,%al` is DEAD CODE in the original -- and it passes AX
through to `ca2f`, which stores the WHOLE word (`ca3a: ab  stos %ax,%es:(%di)`).  So AH reaches a live
record field and matters.  Neither `c4df` (which sets DX, not AX: `c4fd mov 0xe48c(%bx),%dl ; c501 xor
%dh,%dh`) nor its view-0x22 arm at `c559` sets AH, so it is threaded from above c4df.

NEXT for this item: find AH's source above c4df -- the oracle write-trace on the record field ca2f
writes it into would name the writer in one run -- then c783 is a five-line transcription like the
other four.

## Extend the audit

The same check should be run for EVERY data-driven table the engine dispatches through, not just
c4df's four: the 22dd render-script table at DGROUP:0x6c82, the 201a/209e element method vectors at
DGROUP:0x3e18, and the extender op table.  A target that resolves to nothing is silent on native
(it lands in whatever the trampoline does) and is a native<->wasm divergence generator as surely as
board:0014's arity mismatches.

## Found while opening the kill chain (board:0017, patches 564-570)

- Every target of c14f's interaction table (DGROUP:0xe518: 7c09 87cb 9018 97c1 9c97 a0a1 b382 bc32
  bd69 c31c) was unpromoted -- patch 564 adds them.  The icall trap's "returning 0" leaves g_fist_cf
  untouched, which is how a missing handler silently became a hit.
- 0000:030b (patch 563), FUN_1000_177f (scratch/parked/571) were unpromoted; FUN_1000_9b39/9b7e/9bc3/
  9c15 existed but pristine (567).
- The FUN map was unsorted after hand-inserted entries (534/535/543/556/563) and its binary search
  missed present functions -- patch 565 sorts a copy at first use, so insertion order no longer
  matters.
- Still open: 390d (render method id 0x18, the two-point 3d99 line) is pristine; c47d/c4a2 (label
  vertex producers of the c33c phase table) set no CF in C and so never deliver a record.

## The trap census on the rebuilt gate (2026-09-12)

`FIST_TRACE_TRAPS` now prints the tick and a native backtrace per unmapped target, and
`FIST_TRAP_BREAK=<linear>` stops gdb at one of them (re_out/fist_icall.c).  A 40 s AZER1 run listed
every dispatch into the void; patch 581 emitted the mission-time ones -- 4397 (the model budget),
b061/b062/b081/b09e (the fire-mission subtasks, b040's table read in the wrong segment), c783/c845/
c862/c876/c888 (render methods), 8e80 and 689a (extender mid-entries) -- and what remains is:

    00000   at boot, 018a <- 1384 <- 06bc: a null vector
    01b31   3920 <- 35a7 (the frame poll's method vector, 3637 calls in 20 s) and 01391 later
    00008   30f8's BIOS chain through [0x432] = 0000:0008 -- the saved INT-8 vector is the IVT
            slot's address, not its content (fist_dos.c AH=35), so the BIOS tick never chains
    13f7f, 1360f   35a7's vectors on the mission-entry poll
    000a1 09900 0a200 0a600 09100   97d5 / 902c sub-dispatches after tick 18500 (garbage table
            reads from the type-2/3 update templates' second tables)
    155cd..1562d   the 55c5 number-formatter family (only 55c5 is a function; its own save/restore of
            [0x684] is Ghidra's `[0x686] = CS ; [0x684] = old [0x686]` mash)

### Patch 582 -- the census closed on AZER1 and TRAIN2

Every one of the first four lines above was a reading of the CRT's near-hook trampoline, and the
AH=35 line was a misdiagnosis of the shim: fist_dos.c hands out FE00:0008 correctly; it was 2ebe's
store that lost it.

- `call [ss:0x3a]` (0x18a, 0x19e) is `ff 16` -- a NEAR call in segment 0 to the hook offset the caller
  dropped into DGROUP:0x3a.  [0x3c]:[0x3e] = 0000:018a is the vector TO that trampoline, and the word
  at [0x3c] is not a segment.  Patch 073 modelled 3920 (`push [0x310] ; ljmp [0x48]`, [0x48] =
  0000:0199) as a far call [0x3c]:[0x3a] -> 018a:0291 = 0x1b31.  [0x310] = 0x291 is the LFSR step
  FUN_0000_0291.  **Every scheduler poll (35ae, 38e5) steps the RNG once in the original; the port's
  RNG did not move there.**  The oracle's block trace has the 1119:0199 / 019e / 0291 blocks and the
  1119:0000 block (the null hook's `ret`, which 1384 reaches through hook slot [0x272] = 0).
- 018a and its seven `push [0x2xx] ; pop [0x3a] ; lcall [0x3c]` callers (1384 1394 13a4 13b4 13c4
  13d4, 3d07) used Ghidra's static SS 0x2ba9 for the slot (DAT_2000_baca = 0x2baca) -- the boot-time
  null-vector trap.  DGROUP-based now; FUN_0000_0000 (`ret`) and FUN_0000_1c66 (`clc ; ret`, 3835's
  constant hook) exist.
- 13f7f / 1360f: 3566 stored CS = 0x1000 into the task continuation where the oracle runs 3566 and
  35a7 at 2082:3ed6 / 2082:3f17 (0f69).  The continuation is 0f69:3f7f = the `lret` at 0x1360f,
  FUN_1000_360f now.
- 00008: `mov [0x432],bx ; mov [0x434],es` after INT 21h AH=35 -- the 4-byte DAT_1000_c432 store zeroed
  [0x434] and `unaff_ES` was the pre-INT ES.  Word stores of BX and the returned ES; 30f8's chain
  reaches fist_dos.c's INT 08 (1890 chains in a 20 s AZER1 run) and the BIOS tick moves.
- One more target the census had not reached: word[0x6ce4] = 0x5db8, the per-sim-tick handler
  FUN_0000_4886 installs at the verdict, near-called by the main loop 45ed once per tick; unpromoted.
  Transcribed (FUN_0000_5db8), with 72ee's idle handler 0x773e (`ret`).

AZER1 to its verdict and 60 s beyond it (the debrief), TRAIN2 to its verdict: FIST_TRACE_TRAPS prints
nothing.  Not reproduced in those runs, so still listed rather than closed: the 97d5/902c garbage
sub-dispatches (seen after tick 18500 before 582 -- the RNG change moved every battle) and the 55c5
family.  That family is decoded, for when a run reaches it: reloc section 0x2f0 installs
DGROUP:0x4a8..0x4ec = 0f69:5f35.. = the fourteen buffered formatters 55c5..562d, each `mov word
[0x3a],<0f69-offset> ; jmp 5633` where 5633 pushes [0x684]:[0x686], sets the glyph vector to the buffer
emitter 0f69:5fc5 (0x15655: `mov [di],al ; inc di ; retf`), `push cs ; call [0x3a]` (near, in 0f69:
5cf7 -> 0x15387, 5d8b -> 541b, 5de1 -> 0x15471 ..) and pops the vector back.  FUN_1000_55c5's body is
Ghidra's `[0x686] = CS ; [0x684] = old [0x686]` mash and the other thirteen do not exist; no site in
the decompile calls [0x4a8..0x4ec] by name (no `lcall [0x4xx]` in the image either), so whoever
dispatches them does it through a computed offset that has not been seen executing yet.
