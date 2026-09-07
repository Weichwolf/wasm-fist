Type: bug
Area: decompile
Tags: wasm-divergence arity icall

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

## Remaining candidates (from that audit, HEAD after 530)

    0x60a -> 1091 (3)   13 sites at 2 or 1        0x68c -> 2004 (11)  10 sites at 3
    0x6c8 -> 294d (5)    7 sites at 1             0x560 -> 0310  (4)   8 sites at 3
    0x61a -> 11db (2)    6 sites at 1             0x594 -> 04a3  (0)   5 sites at 1-2
    0x6d0 -> 2a39 (4)    3 sites at 1             0x558 -> 026e  (2)   3 sites at 1 or 3
    0x6b4 -> 26a1 (5)    2 sites at 1 and 4       0x544 -> 0166  (3)   2 sites at 4
    0x554 0x55c 0x564 0x584 0x5a4 0x5fe 0x67c 0x6d8 0x540   1 site each

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
