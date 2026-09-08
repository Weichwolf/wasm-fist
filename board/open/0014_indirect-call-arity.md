Type: bug
Area: decompile
Tags: wasm-divergence arity icall
Title: every indirect call passes exactly as many arguments as its target takes

Every indirect call in the port passes exactly as many arguments as its target takes, so that no
`call_indirect` signature mismatch exists on wasm and no callee reads its arguments off stack residue
on native.

## Why this is a capability and not a tidy-up

The call sites go through a `(code *)` cast, so the C compiler infers the indirect-call signature FROM
THE ARGUMENT LIST.  The two targets diverge:

  * **gcc -m32** is caller-cleans cdecl: a short argument list means the callee reads the missing
    arguments off whatever the stack happens to hold.  It usually "works" -- it paints something, it
    returns -- so nothing fails visibly.
  * **wasm** puts the signature in `call_indirect` and type-checks it.  A short list does not match the
    target's type, and the call does not happen at all.

So this defect class is INVISIBLE on native and silently drops work on wasm.  It is, by construction, a
native<->wasm divergence generator, and it is exactly what was behind the terrain-flow failure.

## Already found and fixed by this class (3 for 3)

| patch | vector | target | sites |
|---|---|---|---|
| 528 | `[0x6b4]` | `26a1` (5 params) | 11 sites at 2 args -- the whole remaining `terrain-*` framebuffer difference (a 7x5 sprite drawn on native, not drawn at all on wasm) |
| 529 | `[0x694]` | `23d8` (2 params) | 2 sites at 1 arg -- descriptor landed in param_1, param_2 was stack garbage; SYRIA1's 38-second stall |
| 530 | `[0x6c8]` | `294d` (5 params) | 2 sites at 2 args, in a function whose `[0x6b4]` sibling 528 had already fixed |

## The audit that finds them

Mechanical, and it needs no run:

  1. resolve each far vector to its target -- the vector table is DGROUP `0x600..0x6e8`, far pointers
     `off=word[vec], seg=word[vec+2]`, and the MGA module segment is `0x3e78` in a live dump;
  2. read the target's parameter count from its declaration;
  3. count the arguments at every `fist_icall_far((uint32_t)(DAT_1000_cXXX))` call site;
  4. list the sites that disagree.

## Remaining candidates (corrected audit, HEAD after 530)

**The first version of this list was WRONG and is replaced.**  My extractor matched the parameter list
with `FUN_0000_XXXX\(([^;]*?)\);`, which also matches each function's DEFINITION -- there the `)` is
not followed by `;`, so the match ran on into the body and counted the commas it found there.  Later
matches overwrote earlier ones, so every function whose definition follows its declaration got an
inflated count: 1091 read as 3 params (really 2), 2004 as 11 (really 3), 0310 as 4 (really 3).  The
three largest "clusters" I first published -- [0x60a] 13 sites, [0x68c] 10 sites, [0x560] 8 sites --
were artefacts of that bug and are NOT defects.  Extract the declaration by scanning to the matching
paren and requiring the next non-space character to be `;`.

Corrected: **17 vectors, ~35 sites**, over the MGA-segment (0x3e78) vectors whose target declaration is
resolvable:

    0x6c8 -> 294d (5)   7 sites at 1        0x594 -> 04a3 (0)   5 sites at 1-2
    0x61a -> 11db (2)   6 sites at 1        0x558 -> 026e (2)   3 sites at 1 or 3
    0x6d0 -> 2a39 (4)   3 sites at 1        0x6b4 -> 26a1 (5)   2 sites, at 1 and 4
    0x544 -> 0166 (3)   2 sites at 4        0x5b0 -> 084e (1)   2 sites at 2
    0x60a -> 1091 (2)   2 sites, at 3 and 1 (the twelve 2-arg sites are CORRECT)
    0x540 0x554 0x55c 0x564 0x584 0x5a4 0x67c 0x6d8   1 site each

Two shapes dominate and should be treated separately:

  * sites passing a literal `0xf69` or `unaff_CS` as the FIRST argument -- that is the far-call SEGMENT
    leaking into the argument list, i.e. the board:0010 segment class, not this one.  Fixing the arity
    without removing the leak would just move the bug.
  * sites passing a single `0` or `ax` to a 4- or 5-parameter target (e.g. the seven [0x6c8] 1-arg
    sites) -- these are the closest in shape to 528/529/530 and are the ones to verify first.

## The discipline this item must keep

The declared parameter counts come from Ghidra's `__allregs` model and are THEMSELVES reconstructions,
not ground truth.  A mismatch therefore means "one of the two is wrong", not "the call site is wrong".
Every one of 528/529/530 was landed only after reading the asm at the call site and confirming which
register carries which argument (`mov ax,<id>` / `mov bx,<descriptor>` / `lcall *<vec>`).  A mass
conversion on the strength of the declaration alone would be exactly the kind of guess this project
forbids, and would silently bake a wrong prototype into 30 call sites.

Note also the 0-argument sites (hundreds of them).  Those are the arg-less-dispatch class: the asm
really does `lcall *<vec>` with the registers ambient from the caller, so the fix is to identify the
carrier and thread it (as patches 310/463/513 do with `g_fist_render_si` / `g_fist_render_dx` /
`g_fist_paintax`), not to invent arguments.  They are NOT part of this item.

## Re-audit at HEAD (after 547-556): the [0x6c8] cluster is the ARG-LESS class, not the arity class

This item nominated the seven `[0x6c8]` 1-argument sites as "the ones to verify first".  Re-reading them
at HEAD, that nomination is wrong in a way worth recording: the remaining sites are not short argument
lists, they are ZERO-argument dispatches --

    (*(code *)fist_icall_far((uint32_t)(DAT_1000_c6c8)))();

-- mixed in with sites patch 530 already converted to the full five:

    ((void (*)(uint,uint,uint,uint,uint))fist_icall_far((uint32_t)(DAT_1000_c6c8)))(ax,0,0,bx,0);

The target's asm settles what they need.  `294d` (MGA segment, `re_out/fist_mga_image.bin`) opens

    294d: 53            push %bx
    294e: 8b 3e 24 07   mov  0x724,%di
    2952: e8 b7 fc      call 0x260c        ; 260c(ax, bx, [0x724])

and patch 136 already records the contract: **AX is the sprite-directory byte offset and BX the
position descriptor**.  So every remaining zero-argument site needs AX and BX read out of the asm AT
THAT SITE -- which is this item's own "identify the carrier and thread it" class, explicitly excluded
from it, not the arity class.

Consequence for whoever picks this up: the `[0x6c8]` line in the candidate table above should be read
as "sites already converted by 530, plus N arg-less sites belonging to the carrier class", and the
first-to-verify recommendation should move to a vector whose sites really do pass a short list.  The
same re-check is owed for the other clusters before any of them is trusted.

A live example of why the carrier class matters, from board:0007: `FUN_0000_c33c` dispatches its ten
phase handlers with `call *0xe6f5(%bx)` and ES:DI ambient, and `FUN_0000_c38b` -- one of the ten -- had
Ghidra's invented `param_2` deref'd as a host pointer.  Measured `param_2 = g_mem - 620`, an
out-of-bounds write that SEGV'd AZER7 as soon as patch 553 made the branch reachable.  Patch 554 fixes
it by taking the cursor from `g_fist_render_di`, exactly as this item prescribes.
