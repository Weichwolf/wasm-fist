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
