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

## CORRECTION: the measurement stands, the inference does not

"No unit's X/Y ever changes" is measured and holds. But the claim that this is a DEFECT is not
established, and the table above invited the wrong reading. Following it up:

- **Type 0x11 DOES move**, and it is not a unit. Its X runs in the tens of millions and wraps
  (`10226323 -> 3280673 -> 21496623 -> 14550973 -> 7539798` across samples) while Y barely changes --
  a scrolling backdrop layer advanced by the global `dword[0x92f2]`, not a vehicle.
- **Type 0x10 is a static emitter**, confirmed from asm 0xb51f-0xb582: it adds table offsets to a
  SCRATCH position at 0x9c7f/0x9c83 and calls bb64. It is not supposed to translate.
- **Types 0x05/0x06 -- the real ground-vehicle mover (9e2b, which does `o+4 += 03a9(heading,speed)`) --
  are simply absent from AZER1.** The step table was dumped from the image and the port dispatches it
  correctly, so this is faithful.

That leaves type 0x02 as the only candidate for "a unit that ought to move", and whether it ought to is
NOT established. Its objects carry a rotating heading and a non-zero speed word, but `902c`'s asm does
not integrate position and neither does the sub-chain reached from it. It is entirely possible that
AZER1's type-0x02 objects are static defensive positions that engage at range, in which case nothing
here is wrong and the mission is meant to be decided by fire, not manoeuvre.

DO NOT treat "units never move" as a bug until it is shown that the ORIGINAL moves them. The cheap test
is the oracle: run AZER1 under dosbox-fist and watch whether the type-0x02 objects' dword[obj+4]
changes. Until then this item is a QUESTION, not a defect.

## ORACLE VERDICT: confirmed a defect. Types 00/01/02 move in the original.

Ran the in-repo oracle headless (Xvfb + `third_party/dosbox-fist` via `tools/oracle/census_azer1.sh`,
`FIST_MEMARM_BOOT=1`) and dumped guest RAM twice in the same deterministic AZER1 run, at 600M and 1000M
recorded writes.

Locating DGROUP in the dump: the engine runs under the extender's paging (`cr3=0xe000`), so it is NOT
at a guessable address. Found it by SIGNATURE -- the per-type step table `7c1d 87df 902c 97d5 bab4
9e2b 9e2b b5e7` is unique -- at guest **0x2d190**, which independently matches the dump header's
`ss=2d19` (SS=DS=DGROUP, exactly the engine's model). a294=64, a296=16, 80 live objects.

Object positions between the two samples:

```
 off  type      A(X,Y)                 B(X,Y)              delta         Z A->B    gnd A->B
 c05c  00  (  598662, 1125164)  (  639585, 1078998)  (+40923,-46166)  20736->16384  81->64
 c157  01  (  572664, 1155516)  (  624964, 1116122)  (+52300,-39394)   8960->22016  35->86
 c92f  02  (  463818,  760119)  (  510072,  801795)  (+46254,+41676)     768->2304   3->9
 ca2a  02  (  412946,  709202)  (  464906,  759406)  (+51960,+50204)    2816->1536  11->6
 cb25  02  (  387500,  699672)  (  444178,  744379)  (+56678,+44707)    9216->2816  36->11
 cc20  02  (  371689,  681290)  (  425157,  727576)  (+53468,+46286)    2304->2816   9->11
 ce16  01  (  567576, 1190816)  (  618024, 1143364)  (+50448,-47452)    6400->11520  25->45
 cf11  00  (  575820, 1175669)  (  601034, 1118632)  (+25214,-57037)    8704->24576  34->96

 per type:  00: moved 2 / static 0    01: moved 2 / static 0    02: moved 4 / static 4
            03,10,15,1a,1b: 0 moved / 60 static
```

So:

- **Types 00, 01 and 02 DO translate in the original**, by 40000-57000 units between samples, in
  coherent directions (00/01 heading +X/-Y, 02 heading +X/+Y). In the port all three are frozen.
- **Their Z tracks the terrain as they move** (gnd 81->64, 35->86, 3->9, 36->11). So these types
  terrain-follow too, which settles board:0018's other half.
- Types 0x10 and 0x15 are static in the ORIGINAL as well -- confirming the b51f emitter reading and
  that "not everything is supposed to move" was the right caution.

This item is therefore a DEFECT, not a question, and it is the same defect as board:0018: types
00/01/02 neither move nor terrain-follow in the port. Both are downstream of whatever drives their
per-frame physics.

NEXT: the port's spawn positions match the original exactly (e.g. the type-02 at (549136,1017675) is in
both), so the divergence begins at the per-frame update. Instrument the original's writers of
`dword[obj+4]` for offset c05c with `FIST_WATCHFLAT` at the CR3-aware engine-flat address of
DGROUP:0xc060 and read the `cs:eip` of the writer -- that names the function the port is failing to run.
