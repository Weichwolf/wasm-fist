Type: bug
Status: open
Parent: 0017
Title: every mobile unit integrates its position each tick

**No unit in AZER1 ever moves.** Traced over a 20000-tick self-play run, sampling live objects from the
c0e5 display table:

```
[move] t=2472   [t02 @(616720,927563) hdg=2353  spd=260] [t02 @(781174,940809) hdg=58954 spd=261]
                [t02 @(798070,798985) hdg=53455 spd=773] [t10 @(709530,913324) hdg=0 spd=256]
[move] t=17857  [t02 @(616720,927563) hdg=4085  spd=260] [t02 @(781174,940809) hdg=52929 spd=261]
                [t02 @(798070,798985) hdg=18443 spd=773] [t10 @(709530,913324) hdg=0 spd=256]
```

Every X/Y is byte-identical across 15000 ticks. Headings DO slew (2353 -> 4644 -> 1648 -> 4085) and the
speed word [obj+0x1b] is non-zero (260, 261, 773), so the units are alive and steering -- they simply
never translate.

This very likely outranks board:0018 as the reason no mission resolves: units that start ~158670 apart
and never close will not resolve a mission whatever the line-of-sight says.

## What is established

Position integration (`dword[obj+4] += ...`) exists in exactly these step methods in the port:

| method | types | integrates |
|--------|-------|-----------|
| 9e2b | 0x05, 0x06 | yes (`9efe`: `o+4 += 03a9(heading, speed)`) |
| 9b11 | 0x11 | yes (`o+4 += dword[0x92f2]`) |
| b5e7 / b808 / b945 / b998 | projectiles | yes |
| **902c** | **0x02** | **no** |
| **b51f** | **0x10** | **no** |
| 7c1d | 0x00 | no |
| 87df | 0x01 | no |

The per-type step table was dumped from the image (DGROUP:(type*2 - 0x1bac), image offset 0x2a454) and
the port dispatches it correctly -- `9e2b` really is the type-5/6 method, and AZER1 simply fields no
type-5/6 units. So "9e2b is never called" is FAITHFUL, not a defect.

`b51f`'s asm (0xb51f-0xb582) confirms type 0x10 is a static emitter, not a vehicle -- it adds table
offsets to a scratch position at 0x9c7f/0x9c83 and calls bb64. Also faithful.

That leaves type 0x02 (`902c`), which is what AZER1's mobile units are. Its asm delegates:
`9045 call 912d`, `9048 call 90cd`, `904b call 9176`, `904e lcall 0f69:0x90cf` (= linear 0x1875F =
FUN_1000_875f, a control-surface stepper, NOT movement), `9053 call a358`, then two CS-table dispatches
`9060 call *%cs:-0x6f60(%bx)` and `906a call *%cs:-0x6e59(%bx)` -- the tables at 0x90a0 and 0x91a7
indexed by `byte[di+0x3d]`, which 902c advances by 2 every frame (patch 365).

NEXT: the movement step for type 0x02 is one of those CS-table sub-methods. Audit every entry of the
0x90a0 and 0x91a7 tables the way patch 542 audited the 9e2b table -- dump both tables from the image,
check each target exists in the port and takes the object it is handed, and find the one that should do
`dword[obj+4] += f(heading, speed)`. The same audit is then owed to 7c1d (type 0x00) and 87df (0x01).
