Type: bug
Area: decompile
Tags: unpromoted dispatch render

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

    c715  view 0x1e, 1 slot   testb [0x6dab],2 ; byte[si+0x1c] indexes word[DGROUP:bx-0x17f3] ; c8c2
                              else: append si to the STRSEG list at gs:[bx+0x366a], bound word[0xe6e0] < 0x40
    c74d  view 0x20, 1 slot   the same STRSEG-list append, gated on word[0x6d2c]&4
    c774  view 0x1e, 1 slot   mov bx,0x13b0 ; xor ah,ah ; call 0xc8c2   -- needs the c8c2 arg convention
    c7a5  view 0x20, 1 slot   own arithmetic on byte[si+0x19] (shr 2, neg, add 0xc, clamp)
    c783  view 0x22, 6 slots  three-way flag test on [si+0x16]/[si+0x17]/word[0x6cde], then al=0xe

None of these is hard, but each needs something the tree has not established yet (the c8c2 argument
convention, or the STRSEG deferred-object list write).  They must be read out of the asm, not guessed.

## Extend the audit

The same check should be run for EVERY data-driven table the engine dispatches through, not just
c4df's four: the 22dd render-script table at DGROUP:0x6c82, the 201a/209e element method vectors at
DGROUP:0x3e18, and the extender op table.  A target that resolves to nothing is silent on native
(it lands in whatever the trampoline does) and is a native<->wasm divergence generator as surely as
board:0014's arity mismatches.
