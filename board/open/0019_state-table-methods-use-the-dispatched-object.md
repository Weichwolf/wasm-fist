Type: bug
Status: open
Parent: 0017
Title: every 9e2b state-table method reads the object the dispatch actually handed it

`FUN_0000_9e2b` dispatches a per-state method with `9ecd call cs:[0x9f0f+bx]`, DI live. The table has
SEVEN entries (patch 497's comment lists six):

```
9f1d  9f66  9fe5  a069  a07c  a02a  a03f
```

The dispatch passes the object's DGROUP near offset as the call's single argument, so it arrives in
`param_1`. Ghidra bound several of these functions' DI varnode to a LATER parameter -- stack residue at
run time -- and dereferenced it as a host pointer. Each is a SIGSEGV the moment its state is entered.

| entry | state |
|-------|-------|
| 9f1d  | fixed, patch 497 |
| a07c  | fixed, patch 498 |
| a069  | fixed, patch 540 |
| a02a  | fixed, patch 540 |
| a03f  | fixed, patch 540 (this is the one AZER4 reaches) |
| **9f66** | **OPEN** |
| **9fe5** | **OPEN** |

The same defect class also hit `a0c8`, reached through the *other* dispatch (`c31e`, asm 0xc31e), fixed
in patch 540.

## Why 9f66 and 9fe5 are not fixed yet

Both need a convention established first, and guessing it would be worse than leaving them:

- **9fe5** (asm 0x9fe5-0xa029) calls `9ffd call 0x54c` with `si = di+4` and `di` = one of two DGROUP
  table addresses (0x943c, or 0x9444 when `byte[di+0x16]&8`), then consumes **three** returns:
  `a001 mov %ax,0x2e(%di)` and `a004 mov %ch,%cl ; a006 mov %dl,%ch ; a008 mov %cx,0x21(%di)`. So it
  needs 054c's AX **and** its CX/DX threaded back. The pristine code fabricates that word from its own
  parameters instead. Until 054c's register threading is established (the way `g_fist_0578_bx` /
  `g_fist_0927_dx` were for 0578), the reconstruction cannot be honest.
- **9f66** (asm 0x9f66-0x9fe4) carries a live `unaff_CS` throughout, spawns via `b1df(0xe)`, and calls
  `b7a9` -- itself pristine and host-pointer based, with `FUN_0000_b7a9`'s only call site being this
  one. It needs b7a9 resolved at the same time.

NEXT: establish 054c's multi-register return threading, then reconstruct 9fe5; then resolve b7a9 and
9f66 together. Both are latent SIGSEGVs, so they are a hard blocker for any long self-play run that
reaches those states.
