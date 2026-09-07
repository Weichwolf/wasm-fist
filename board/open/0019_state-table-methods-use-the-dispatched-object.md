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
| 9f66  | fixed, patch 542 |
| 9fe5  | fixed, patch 542 |

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

## More sites, found by driving combat harder

With the board:0018 probe forcing LOS open (so units actually engage), AZER1 reaches another instance
of the same class within 40000 ticks:

```
#0  FUN_0000_86e3 (param_1=49495)      <- 0xC157, a DGROUP near offset, host-deref'd
#1  FUN_0000_8711 (param_1=49495)
#2  FUN_0000_afa2  (the aim gate)
#3  FUN_0000_af97
#4  FUN_0000_ab03
```

`FUN_0000_86e3` is pristine and reads `[param_1 + 0xb7]`, `[param_1 + 0xb5]`, `[param_1 + 0xb8]`,
`[param_1 + 0xe0]`, `[param_1 + 0xe2]` through a near offset used as a host pointer.

Its caller `FUN_0000_8711` carries a second defect worth fixing at the same time:

```c
undefined1 in_CF = 0;
FUN_0000_86e3(param_1);
if ((bool)in_CF) { FUN_0000_86b8(param_1); }
```

`in_CF` is a fabricated local initialised to 0, so `86b8` is dead code -- the asm's carry out of 86e3
is dropped, exactly the class patch 284/497 threading (`g_fist_cf`) exists to fix.

These do not fire in the default build today because units rarely engage; they WILL fire as soon as
board:0018 is fixed. They are a hard prerequisite for it, not an independent nicety.

## CLOSED for the 9e2b table; the weapon-station cluster came with it

All seven entries are now reconstructed from their own disassembly (497, 498, 540, 542), and the two
that board:0019 held open were settled rather than guessed:

- **9fe5** needed 054c's multi-register return. Established from `054c`'s own asm: `0x554 push %ax`,
  `0x558 push %ax ; push %dx`, `0x56f pop %dx ; pop %cx ; pop %ax` -- so AX=bearing, BX=pitch,
  CX=range LOW, DX=range HIGH. CX/DX are now threaded the way patch 473 already threads BX. The
  pristine code had fabricated the range word out of its own parameters.
- **9f66** needed its two 0f69-segment call targets resolved: `lcall 0f69:0xbb4f` = linear 0x1B1DF =
  b1df, and `lcall 0f69:0xa86c` = linear 0x19EFC = 9efc. (An arithmetic slip first put the second at
  0x1A0FC, which disassembles to a bare `lret` -- the wrong answer, caught by checking it.)
  Its callee **b7a9** was fixed with it.

Patch 541 additionally closed the weapon-station cluster reached from the aim gate -- 86e3, 8711,
86b8, 8270, 842f, 844b -- which carried the same base-loss plus TWO dropped carries: 86e3's and
86b8's `stc`/`clc` results, and `8711`'s fabricated `undefined1 in_CF = 0` that made FUN_0000_86b8
unreachable dead code.

Verified under board:0018's LOS probe, which is the only way to reach these paths today:

```
before 541:  AZER1 SIGSEGV in 86e3
after  541:  AZER1 SIGSEGV in 9f66
after  542:  AZER1 exit 0, a296 16 -> 11 (five kills); AZER4 20 -> 14; SAUDI2 13 -> 3
```

No crash remains under the probe on the missions tested. Matrix 177/0 on both targets.

This item stays OPEN only until the same audit is run over the OTHER dispatch tables -- the class is
"every dispatched method reads the object it was handed", and only 9e2b and c31e have been swept.
