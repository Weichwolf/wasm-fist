Type: bug
Status: open
Parent: 0012
Title: every mission plays itself to a resolved victory/defeat state

The self-play sweep (`tools/selfplay.sh`) runs all 47 missions as AI-vs-AI simulations with empty
player input over the cooperative tick. Under the goal's actual criterion -- one side eliminated --
**no mission resolves yet**. AZER1 reports UNRESOLVED (min_a296=11) at 20000 and at 60000 ticks.

The earlier "41/47" figure counted missions that did not HANG, not missions that resolved.

## What is measured (AZER1, native, 20000 ticks)

The combat chain is alive end to end until one specific point:

| stage                                   | measurement            | verdict |
|-----------------------------------------|------------------------|---------|
| LOS between opposing units              | 6681 VISIBLE of 17795  | works   |
| cross-unit range vs the 0x40000 gate    | 158670 < 262144        | in range|
| aim gate -> fire request (`a286`)       | 138 requests           | works   |
| `byte[obj+0x92]` countdown consumer     | `91b8` entered 5704x   | works   |
| weapon-station gate `[0x91]==6 \|\| [0xa8]==0` | opens 668 of 8000 | works   |
| station handler dispatch                | 700 (88d1/888c/8916)   | works   |
| **projectile allocation (`b1df`)**      | **492 of 700 refused** | **FAILS** |
| impact/damage walk (`bb1b`)             | 0 entries              | never reached |

`b1df` fails because `b21d` reports the roster FULL: `a294` (the small-object roster, stride 0x37 at
DGROUP:0xa022, presence flags at 0xe2f7) repeatedly reaches its 0x96=150 cap -- 7429 refusals in one
run. The vehicle roster (`a296`, stride 0xfb at 0xc05c) stays at 11 and is never full.

## What is NOT the defect (each checked and eliminated)

- **The roster code.** `FUN_1000_b21d` was diffed against asm 0x1b21d-0x1b293 instruction by
  instruction: bound 0x96, strides 0x37/0xfb, flag arrays 0xe2f7/0xe38d, `mov [di],ax; mov [di+2],bx`.
  The port is faithful. The 150 cap is the original's.
- **The free path.** `b2ef` clears the presence flag and decrements. It runs: 8731 friendly frees
  against 8881 allocations. Allocation and free agree on the team table
  (`type-0x19ec` == `0xe614+type` mod 2^16).
- **A missing dispatch target.** Every object type in the `c0e5` step walk resolves:
  `00->7c1d 01->87df 02->902c 03->97d5 04->bab4 08/0b/0d->b5e7 10->b51f 11->9b11 12->9bc6 13->c0ba
  15->9c4f 17->bc0c 1a->bc46 1b->b355`. No UNRESOLVED entries.
- **A leak in the display table.** DGROUP:0xdfbc (182 entries) churns normally, 99..161 live.

## The open question

Object type 0x04 spawns in bursts of ~80 (t=7035: `04:80`; t=11139: `04:70`) and those bursts are what
push `a294` into its cap. Either the original does the same and tolerates it, or the port over-spawns
type 0x04. This is not decidable from the port alone.

NEXT: put the DOSBox oracle on it. Arm `FIST_MEMARM_BOOT=1 FISTLOG=<prefix>` with `FIST_WATCHFLAT`
pointed at the engine-flat linear of `a294` (DGROUP+0xe294) in an original AZER1 run and compare the
occupancy trajectory against the port's. If the original's `a294` never approaches 0x96, the defect is
upstream in whatever spawns type 0x04; if it does, the refusals are normal and the missing resolution
is a different stage.

## CORRECTION (measured) -- the root cause is one function, and the table above is wrong twice

Two rows of the table above came from **dead shim counters** in `tools/native_main.c` (`[SPLASH]`,
`[chain]`, `[7e29]`, `[reload]`, `[spawn]`) that print zero unconditionally. They are not wired to the
code they name. Trusting them produced a false premise here and in board:0016.

Live counters give the real chain: `bb1b` is entered **12548** times (not 0), the impact dispatch
`c14f` returns a hit **9440** times, the firer filter rejects **none**, and the damage call `bbb7`
runs **9500** times. Nothing was blocked. The impacts were simply **against the wrong objects**:

```
[hit] impacted object types: 00:3 01:1 13:2 1b:7994
```

7994 of 8000 impacts hit object type 0x1b -- six permanent objects at z=7936..13824, while the
vehicles sit at z=1280..2048.

The cause is `FUN_0000_0ea9`, the 3-lane proximity/range test under every collision walk in the
engine. Pristine Ghidra modelled each 32-bit coordinate lane as a 64-bit value (pairing X with Y as
one lane's low/high halves, testing Z as lane 2, never testing Y), and -- decisively -- **never set
the carry its callers branch on**, so every range answer was a stale carry from an unrelated call.
Fixed asm-verified in `patches/538-0ea9-range-test-lanes-and-carry.diff`.

The roster saturation described above was a SYMPTOM: each bogus hit spawned a type-0x4 explosion
template until `a294` hit its 0x96 cap. After patch 538: a294 150 -> 114, a296 11 -> 16, LOS queries
17795 -> 30190 over the same tick budget.

STILL OPEN: with the false hits gone, AZER1 now kills nothing at all (min_a296=16). Projectiles no
longer explode on the type-0x1b objects but do not yet reach their targets either. That is the next
stage to trace -- and it is a different question from the one this item started with.

TODO (hygiene): delete the dead `[SPLASH]`/`[chain]`/`[7e29]`/`[reload]`/`[spawn]` counters from
tools/native_main.c. They have now caused two false conclusions and cost more than they are worth.

## The second defect: the AI never aimed (patch 539)

With 538 in, projectiles stopped hitting the decoy objects but hit nothing else either: their closest
approach to any object over 538010 range tests was 11601 units against a collision bound of 1280.

The launch path is direct-fire: for a firer of type != 0x1a, `ace0` sends the projectile along the
firer's own heading `word[di+0x10]` (`b793` passes ax=0, which asm 0xb79c `mov $0x0,%ax` confirms).
Logging launches against a reference atan2 -- and confirming the engine's convention is
`vx = s*sin(h)`, `vy = s*cos(h)`, which the logged velocities match exactly -- showed the heading was
139 degrees off.

The turret slew is NOT at fault: `word[di+0x10]` tracks the computed aim `word[di+0x9b]` exactly. The
**computed aim itself** was wrong. And the angle solver is not at fault either: `FUN_0000_0731`
returns 54998 where a reference atan2 gives 54997, one unit of fixed-point rounding.

It was being fed garbage. `a265` builds the two aim vectors at DGROUP:0x9684 (self) and 0x9690
(target), and the asm writes all six slots with the 0x66 prefix -- `mov %eax,0x9684`, i.e. DWORD
stores. The port declared every slot `undefined2`, so each 32-bit world coordinate was truncated to
its low word:

```
[a265] di=cf11 obj=(570286,1201009,2048)  si=c34d tgt=(549136,1017675,0)
       vec5684=( 45998,  21361,4096)      vec5690=( 24848,  34635,1792)
```

Fixed by widening the six macros to `undefined4` in patches/539-aim-vectors-are-dwords.diff. The slots
are 4 bytes apart with no overlap and every existing user already treats them as wide.

Measured aiming error at launch, AZER1: before -139 deg and -115 deg; after **+0.01 deg**.
AZER1 outcome: a296 16 -> 15 -- the first real kill.

STILL OPEN: one kill is not a resolved mission. The next question is why engagements still resolve so
slowly -- note op-0x58 VISIBLE dropped 8465 -> 446 when the vectors were corrected, because 0x9684/
0x9690 are shared buffers that the LOS path also reads, so LOS results changed materially too.

## Full sweep with patches 538 + 539 + 540 (native, 20000-tick cap, 100s budget)

```
RESOLVED     0
UNRESOLVED  33   mission loads, roster spawns, simulation runs, no side eliminated
NOLOAD       9   INDIA2 INDIA3 INDIA4 SAUDI1 SAUDI2 SAUDI5 TRAIN1 TRAIN2 TRAIN4
TIMEOUT      5   AZER6 AZER7 CYPRUS4 INDIA1 TRAIN3
CRASH        0   (was 6+ before patch 540)
```

The five TIMEOUTs are genuine hangs, not slow runs: AZER6 and CYPRUS4 were re-run at a 280s budget and
still produced no `[outcome]` line at all, so they hang at or before mission load.

The crash class introduced-and-then-traced this round is closed for the five dispatch entries patched
(see board:0019 for the two that remain latent).

Three distinct blockers now, and they are NOT the same bug:

1. **33 UNRESOLVED** -- engagement never eliminates a side. Gated by board:0018 (the LOS endpoints are
   a shim stand-in), so the number cannot be interpreted until that is real.
2. **9 NOLOAD** -- the mission never spawns its roster (a296 never reaches 15). Distinct from the
   others: the simulation never starts. INDIA2/3/4, SAUDI1/2/5, TRAIN1/2/4.
3. **5 hangs** -- no progress at all at 280s. AZER6, AZER7, CYPRUS4, INDIA1, TRAIN3.

Note for anyone reading the older numbers in this session's history: "41/47" counted missions that did
not hang, under a criterion that no longer applies. Against the goal's criterion the count has been
0/47 throughout; what changed is that 33 missions now genuinely load and simulate without crashing.

## CORRECTION 2: the "NOLOAD" class was my own instrument, and combat is far more alive than reported

`g_a296_loaded` was gated on `a296 >= 15` -- a threshold sized for AZER1's roster. Roster size is PER
MISSION: TRAIN1 fields **3** vehicles. So every small-roster mission was reported as "never loaded",
and a mission that actually RESOLVED would have been reported that way too, because the resolved test
required the same flag.

Re-measured with the gate fixed (in-mission and a296>0, keeping a high-water mark):

| mission | peak a296 | final a296 | kills |
|---------|-----------|------------|-------|
| SAUDI2  | 13 |  7 | **6** |
| INDIA3  | 10 |  6 | **4** |
| SAUDI1  | 13 |  9 | 4 |
| SAUDI5  | 14 | 10 | 4 |
| TRAIN4  | 13 | 10 | 3 |
| INDIA2  | 14 | 12 | 2 |
| INDIA4  | 14 | 13 | 1 |
| TRAIN2  | 13 | 13 | 0 |
| TRAIN1  |  3 |  3 | 0 |

All nine load, spawn, and simulate. None was a NOLOAD. The corrected sweep is **42 of 47 missions
loading and simulating with real combat attrition, 5 hanging, 0 resolving**.

## Attrition STALLS -- more ticks do not help

SAUDI2 kills 6 of 13 early and then stops. Run to a 60000-tick cap it is still at exactly 7:

```
20000 ticks: a296=7  peak=13
60000 ticks: a296=7  peak=13
```

So the missions do not fail to resolve for want of time. Something stops engagement after an initial
burst -- the same shape as AZER1 (16 -> 15, then nothing). That is one question, not 42, and it is
the next thing to isolate: whether surviving units stop acquiring targets (board:0018's LOS), stop
being acquirable, or stop firing.

## Sweep after patches 538-545 + the op-0x20/0x1c services + LOS de-approximation

```
RESOLVED     0
UNRESOLVED  45
TIMEOUT      2   INDIA1, TRAIN3
NOLOAD       0
CRASH        0
```

Hangs went **5 -> 2**: AZER6, AZER7 and CYPRUS4 now run to completion (they were confirmed hangs at a
280s budget before). 45 of 47 missions load, spawn, move, terrain-follow and engage without crashing.

## CAVEAT on "resolved": the criterion is MINE, not the engine's

`tools/selfplay.sh` calls a mission resolved when `a296` reaches 0 -- one side eliminated. That is a
construction of this harness, not something read out of the engine, and it should not be inherited
uncritically. The game ships four mission-outcome scripts:

```
FISTDATA/WWIN.MS3  WLOSE.MS3  EWIN.MS3  ELOSE.MS3
```

and their names sit in a packed table at DGROUP:0x9f7a with a 4-byte-spaced index table at
DGROUP:0x9f38 (image 0x25f38: WWIN, WLOSE, EWIN, ELOSE, MSN1, ...). So the engine has its OWN
mission-end path that selects among win/lose scripts -- and it may well end a mission on an objective,
a timer, or a base-loss cascade rather than on annihilation.

I did NOT find the code that indexes that table: there is no asm reference to 0x9f38 or -0x60c8, so it
is reached through a computed or register-held base.

This matters for the goal's wording. Two possibilities, and they are NOT yet distinguished:
(a) missions genuinely fail to reach the engine's own end state -- a real defect; or
(b) missions do reach it and this harness cannot see it, because it is watching a296 instead of the
    engine's outcome flag -- in which case the measurement is wrong, exactly like the `a296 >= 15`
    roster gate was.

The oracle bears on this: between its two dumps the ORIGINAL killed only 2 vehicles (a296 16 -> 14),
so the original does not resolve AZER1 by rapid annihilation either. That is weak evidence for (b).

NEXT: find the reader of the DGROUP:0x9f38 table and the flag it switches on, then teach
tools/selfplay.sh to watch THAT rather than a296. Until then, "0 of 47 resolved" is a statement about
this harness's criterion as much as about the engine.

## CAVEAT RESOLVED: the engine's own outcome path is never reached either

The previous section raised two possibilities -- (a) missions really fail to reach the engine's end
state, or (b) they reach it and this harness cannot see it. **It is (a).**

`FUN_0000_be0e` (asm 0xbe0e) is the mission-script loader: `dx = word[DGROUP:0x9f2c + bx]` is the
filename offset and bx selects the script, so bx = 0x0c -> WWIN.MS3, 0x10 -> WLOSE, 0x14 -> EWIN,
0x18 -> ELOSE, 0x1c -> MSN1, 0x20 -> MSN2. Instrumented to log every distinct index loaded during
self-play:

```
AZER1   idx 0x00 (t=74)   0x04 (t=4382)   0x08 (t=235)   0x20 = MSN2 (t=274)
SAUDI2  idx 0x00 (t=74)   0x04 (t=4382)   0x08 (t=235)
```

**No run ever loads 0x0c, 0x10, 0x14 or 0x18.** The engine never selects a win or lose script, so the
harness's `a296 -> 0` criterion and the engine's own outcome path agree: no mission ends. The
measurement is not the problem.

(Checked and eliminated on the way: the table at DGROUP:0x9f54 that `be75` indexes holds 0x1c/0x20/0x24
-- the MSN1/MSN2/MSN3 briefing screens picked at random -- not outcomes. And the `mov $0xc,%bx` /
`$0x14` / `$0x18` sites at 0x58fa/0x5908/0x5916 are stubs into a common handler at 0x5932, i.e. .MS3
script-command entry points, not the outcome dispatcher.)

NEXT: find what SHOULD call be0e with 0x0c/0x10/0x14/0x18 -- the win/lose decision itself. The
remaining unexamined callers in the port are at fist.c:38200 (`FUN_0000_be0e(*(uint16_t *)(R + bx + 6))`,
a computed index) and fist.c:68961. The asm call sites of 0xbe0e are 0xbdfc, 0xbe05, 0xbe82, 0xe5b9,
0xe718, 0xe800, 0xe8bb and 0x1bba3 -- the 0xe5xx-0xe8xx cluster is the most likely home of the
mission-end decision and has not been read yet.

## The outcome selection is DATA-DRIVEN, in the .MS3 script -- not a hardcoded engine constant

Read `be0e`'s call cluster. The one that matters is asm 0xe59f-0xe5bd:

```
e59f: lea 0x8(%bx),%ax ; mov %ax,0xf6ec
e5a5: mov %es:0x2(%bx),%ax ; mov %ax,0xf6e0      ; ES-relative record fields
e5ac: mov %es:0x4(%bx),%ax ; mov %ax,0xf6e2
e5b4: mov %es:0x6(%bx),%bx                       ; <-- the SCRIPT INDEX comes from the RECORD
e5b9: call 0xbe0e                                ; load that script
e5bd: mov -0x15d2,%es                            ; ES = word[DGROUP:0xea2e]
```

(the port's `fist.c:38200`, `FUN_0000_be0e(*(uint16_t *)(R + bx + 6))`). The other three sites are
fixed screens: 0xe800 and 0xe8bb both pass bx=8.

So nothing in the engine hardcodes "load WWIN when the enemy is dead". The .MS3 script for the mission
carries the screen index in its records, and the interpreter -- whose command handlers are the
0x58ec/0x58f3/0x58fa/0x5901/0x5908/0x5916 stubs into the common dispatcher at 0x5932 -- evaluates the
condition and follows the record. The engine's outcome path is reached only when a script command fires.

### A distinction this session has NOT been testing

`tools/selfplay.sh` drives missions through the BATTLES menu (`FIST_FSG_BATTLE`, the 47 `.FSG` files).
The `MSN1/MSN2/MSN3.MS3` scripts are the CAMPAIGN mission scripts, and `WWIN/WLOSE/EWIN/ELOSE.MS3` are
campaign outcome screens. It is entirely possible that a BATTLE ends by a different mechanism than a
campaign mission, in which case looking for a WWIN/ELOSE load in a battle run is looking for the wrong
event -- the same kind of criterion error as the `a296 >= 15` roster gate and the `a296 -> 0` resolution
test.

NEXT, in this order:
1. Determine how a BATTLE (.FSG) signals its end, as opposed to a campaign mission. The goal's wording
   covers both ("Kampagnen- und Battle-Missionen"), so both paths must be identified before "resolved"
   can be measured honestly for either.
2. Instrument the .MS3 command dispatcher at 0x5932 (its stubs are already located) to see which
   commands run in a mission and which condition is evaluated but never satisfied.

## The mission loop and its end-predicate, traced correctly

The in-mission loop is asm 0xe753-0xe76a inside `FUN_0000_e714`:

```
e74d: movw $0x0,0x452          ; reset the frame timer
e753: call 0x206f              ; the per-frame work
e756: lcall 0f69:0x4277 ; jae e766
e75d: lcall *0x52c
e761: lcall 0f69:0x12dc
e766: lcall *0x40a             ; <-- the CONTINUE predicate, returns CF
e76a: jb 0xe753                ; loop while CF=1; CF=0 ends the screen/mission
```

**The port's loop is FAITHFUL** -- it has all three intermediate calls and evaluates `[0x40a]`, looping
on that call's carry. (I first read a truncated grep of only the `do {` / `} while` lines and wrongly
concluded the port had dropped them; it had not.)

Resolved `[DGROUP:0x40a]` at RUNTIME rather than by guessing at a name: it is `0f69:0x3f17` =
**linear 0x135A7 = FUN_1000_35a7**. (The port's inline comment says "35a7", which I first disassembled
at CS=0 -- the wrong address, a data-copy routine. The 0f69 segment has to be resolved to linear.)

asm 0x135a7 is the **cooperative TASK SCHEDULER**, not a mission-outcome test:

```
135a7: testb $0xff,0x6a ; jne back        ; re-entrancy guard
135ae: call 0x13920
135b1: testb $0xff,0x3e00 ; jne 0x13610
135b8: cmpw $0xffff,0x18e4 ; je 0x13610   ; ready-list EMPTY -> 0x13610
135bf: mov 0x18e0,%bx ... lcall *0xc(%bx) ; dispatch the head task
135d9: pushf ; cli ; ...                   ; unlink/relink the ready list at 0x18e0/0x18e4/0x18e8
```

So "the mission ends" is not an outcome test at all -- the screen runs until the scheduler says stop,
i.e. until a TASK signals completion. The mission-end decision therefore lives in whatever task the
mission installs, and the outcome script load (be0e with 0x0c/0x10/0x14/0x18) would be that task's
action.

NEXT: instrument the scheduler's ready list (DGROUP-relative 0x18e0 head / 0x18e4 next / 0x18e8 free,
in the 0f69 cluster's own data) to enumerate which tasks are queued during a battle and whether a
mission-supervisor task is installed at all. That is the concrete question behind "no mission resolves",
and it is a queue-contents question, not another asm-reading one.

## ROOT CAUSE FOUND: the mission-over flag, its six writers, and the missing timeline task

`FUN_0000_459a` IS the mission -- `e714` calls it once and everything after the call is teardown +
outcome. Its loop (asm 0x459a-0x460c) is gated on a single flag:

```
45a3: movb $0x0,-0x17ec        ; byte[DGROUP:0xe814] = 0, cleared at mission ENTRY
45c8: testb $0xff,-0x17ec
45cd: jne 0x4614               ; NONZERO -> leave the mission
45e1: ... call *0x6ce4 ; call 0xc0ca ; call 0x461b   ; the per-frame work
45fb: jne 45c8
```

**`byte[DGROUP:0xe814]` is the MISSION-OVER flag.** Six asm sites raise it, and each first writes an
OUTCOME CODE to `word[DGROUP:0x6da0]`:

| asm | outcome | in the port? |
|-----|---------|--------------|
| 0x4168  | (adds 2 to 0x4a86 first) | not located |
| 0x4457  | -- | yes, FUN_0000_4457 |
| 0x6115  | 3 | yes, FUN_0000_6104 |
| **0x15e4c** | **4** | **NO** |
| **0x15e8c** | **5** | **NO** |
| 0x1a5e9 | (decw 0x6da2 first) | yes, FUN_1000_a5dc |

The port's function list jumps straight from `FUN_1000_5dfc` to `FUN_1000_5e98`, so everything in
1000:0x5e00..0x5e97 was never promoted -- the board:0015 class.

The one that matters for self-play is the task at **1000:0x5e52-0x5e97**, a cooperative TIMED
EVENT-LIST task (note the `ljmp $0x0,$0x0` yield point at 0x15e52, patched at run time by the 0f69
CRT's task switcher):

```
15e5f: decb 0x6cc8            ; count down the current delay
15e63: jne <yield>
15e72: les 0x6cc2,%si         ; FAR pointer to the mission's event list
15e76: lods %es:(%si),%ax
15e78: cmp $0xffff,%ax ; je 15e86   ; END OF LIST
15e7d: mov %ax,0x6cc8 ; mov %si,0x6cc2 ; <yield>    ; else take the next delay
15e86: movw $0x5,0x6da0       ; OUTCOME = 5
15e8c: movb $0xff,-0x17ec     ; MISSION OVER
```

So **a battle ends when its scripted timeline is exhausted** -- not by annihilation. That also explains
why the oracle only kills 2 vehicles in its sample: the original is not racing to wipe the enemy out,
it is running a timeline.

(The other missing site, 0x15e36-0x15e51, is a KEY handler -- `ah` = 0x01/0x10/0x39 -> outcome 4 -- so
it is the player's abort path and would not fire under the empty input self-play uses.)

Also noted: the port's `DAT_2000_a814` macro is `undefined2` (16-bit) while every asm access is a BYTE
(`movb $0xff` / `testb $0xff`). Writing it as a word clobbers the neighbouring byte at 0xe815. Same
width class as patch 539, and it must be fixed with the rest.

NEXT: promote 1000:0x5e00..0x5e97 (at minimum the timeline task at 0x5e52 and the key handler at
0x5e36), work out how the task is installed into the 0f69 scheduler's ready list, fix the a814 width,
and re-measure. This is the concrete path to a mission actually ending.

### The three missing functions, and what is still unknown about installing them

Disassembled the whole unpromoted span. `FUN_1000_5dfc` in the port covers only asm 0x15dfc-0x15dfd
(`stc ; lret`); everything after it up to 0x15e98 is absent. Three real functions live there:

```
1000:5dfe  mission init/teardown.  Gated on byte[0x6cdb]==1; clears bit6 and sets 0x22 in byte[0x6cc6],
           calls [0x442] with bx=0x67c2, then [0x2a] and [0x15a0], clears 0x6cdb and 0x16ac.
           Returns CF=0 on the taken path, CF=1 otherwise.
1000:5e36  the ESC/abort KEY handler.  ah == 0x01 | 0x10 | 0x39 -> outcome 4 + mission over.
           Cannot fire under self-play's empty input.
1000:5e52  the TIMED EVENT-LIST task (the one that matters).  A coroutine: `ljmp $0x0,$0x0` at 0x15e52
           is its YIELD, patched at run time by the 0f69 CRT's task switcher; the body at 0x15e57 loops
           back to it.  Ticks down byte[0x6cc8]; when it expires, advances the far event list at
           dword[0x6cc2]; when that list terminates (0xffff) it sets outcome 5 and raises mission-over.
```

**NOT yet known: how 0x5e52 is installed into the scheduler.** It has no static reference in the image
-- an apparent hit at image 0x206cd is a FALSE MATCH, an odd offset straddling two entries of an
unrelated word table (`... 56d1 5767 57d6 ...` at DGROUP:0x46c0). So the task address is either computed
or written by the CRT's own task-creation call. That has to be found before the task can be transcribed
faithfully rather than invented.

Callers of the other two ARE locatable and are the place to start: 0x5e36 is referenced at image
0xcfa4 (CS=0 code) and 0x172ff (1000: code); 0x5dfe at 0x119d6, 0x1232b and 0x1e645.

ORDER OF WORK:
1. Read the referencing sites above -- they show how this family is registered, which very likely names
   the same mechanism 0x5e52 uses.
2. Promote the three functions and fix `DAT_2000_a814` to a BYTE (asm uses movb/testb; the port's
   undefined2 clobbers 0xe815).
3. Re-measure, and change tools/selfplay.sh's criterion: a battle ends on `byte[DGROUP:0xe814] != 0`
   with the outcome in `word[DGROUP:0x6da0]`, NOT on a296 reaching 0.

### The installer found -- and it is a whole missing CLUSTER, not two functions

Searching by raw bytes produced only false matches (0xcfa4, 0x172ff, 0x206cd all land mid-instruction
or straddle unrelated table words -- all retracted). Searching by SEGMENT OFFSET instead is what works:
linear 0x15e52 = `0f69:0x67c2`, and that constant appears in the region itself:

```
1000:5dfe (the installer)
   15e0f: push %cs ; pop %es
   15e11: mov $0x67c2,%bx      ; ES:BX = 0f69:0x67c2 = linear 0x15e52 = THE TIMELINE TASK
   15e14: lcall *0x442         ; the CRT's TASK-CREATE vector
```

`lcall *0x442` occurs exactly TWICE in the whole engine -- at 0x15e14 above and at 0x15cf3, which is
the same shape:

```
1000:5cd8 (sibling installer)
   15cd8: cmpb $0xff,0x6cdb ; jne ...          ; state gate
   15ce2: mov %cs,%bx ; mov $0x6596,%si ; mov %cs,%cs:(%si)   ; patch CS into a stored far pointer
   15cea: lcall *0x2a
   15cee: push %cs ; pop %es
   15cf0: mov $0x670b,%bx      ; ES:BX = 0f69:0x670b = linear 0x15d9b = another TASK
   15cf3: lcall *0x442         ; CREATE TASK
```

**The port is missing this entire cluster.** Its function list jumps `FUN_1000_5c8a -> FUN_1000_5cf7`
and `FUN_1000_5dfc -> FUN_1000_5e98`, so at minimum these are absent:

```
1000:5cd8  task installer A   (creates the task at 0f69:0x670b)
1000:5d9b  task A
1000:5dfe  task installer B   (creates the timeline task at 0f69:0x67c2)
1000:5e36  ESC/abort key handler -> outcome 4
1000:5e52  the TIMELINE task  -> outcome 5 + mission over
```

So the mission's whole cooperative task set is unpromoted, which is why nothing ever raises
byte[DGROUP:0xe814] and no mission ends.

REVISED ORDER OF WORK:
1. Establish what the shim does for the CRT vectors `[DGROUP:0x442]` (task-create) and `[0x2a]`. If the
   shim does not implement a task-create, that is the prerequisite -- and the scheduler at
   0f69:0x3f17 (linear 0x135a7) is the consumer it must feed.
2. Promote 1000:0x5cd8..0x5e97 as a unit (they are one cluster and share the installer pattern).
3. Fix `DAT_2000_a814` to a BYTE (asm uses movb/testb; the undefined2 macro clobbers 0xe815).
4. Change tools/selfplay.sh: a mission ends on `byte[DGROUP:0xe814] != 0`, with the outcome code in
   `word[DGROUP:0x6da0]` (3, 4, 5 seen so far) -- not on a296 reaching 0.

### PREREQUISITE IDENTIFIED: two reloc sections the port never applies

The CRT vectors the missing task cluster needs are supplied by the engine's own DGROUP relocation table
(image 0x33520, applied by `fist_apply_reloc_section(si, is_far)`). Enumerating every section in that
table and checking which the port applies:

```
si=0x038  0f69  20 entries  DGROUP:0x0a..0x64   -- includes 0x2a = 0f69:0279  and 0x58 = 0f69:0314
si=0x174  0f69  16 entries  DGROUP:0x344..0x394 -- APPLIED (fist_ensure_dlist_vecs)
si=0x1b8  0f69   7 entries  DGROUP:0x426..0x442 -- includes 0x442 = 0f69:3c98  (TASK-CREATE)
si=0x1d8  0f69   9 entries  DGROUP:0x3fe..0x41e -- APPLIED; includes 0x40a = 0f69:3f17 (SCHEDULER)
```

**The port applies only si=0x174 and si=0x1d8.** So `[DGROUP:0x40a]`, the loop's continue-predicate, is
live -- which is why the mission loop runs at all -- but `[DGROUP:0x442]`, the TASK-CREATE vector the
missing installers call, is never populated. `[0x2a]` and `[0x58]` (the scheduler entry named in the
shim's own extender-gate comment) come from si=0x038, also unapplied.

This closes the analysis. The shim's comment at the extender gate says it plainly:

> "A cooperative-task scheduler is a later-stage concern and is not on the path to first light."

That deferral is exactly what blocks mission resolution. The machinery is all present in the image --
task-create at 0f69:0x3c98 (linear 0x13328), scheduler at 0f69:0x3f17 (linear 0x135a7) -- and only the
wiring is missing.

## COMPLETE ORDER OF WORK for the next pass

1. Apply reloc sections **si=0x1b8** and **si=0x038** (same mechanism, same function, one line each) so
   `[0x442]`, `[0x2a]` and `[0x58]` go live. Verify against the oracle DGROUP sample the way si=0x174
   and si=0x1d8 already are (`tools/oracle/samples/dgroup_0x0_0x100.bin`).
2. Confirm the task-create at 0f69:0x3c98 and the scheduler at 0f69:0x3f17 actually run in the port
   (they are engine/CRT code, not shim code -- they may work once their vectors exist).
3. Promote **1000:0x5cd8..0x5e97** as a unit: 5cd8 (installer A), 5d9b (task A), 5dfe (installer B),
   5e36 (key handler), 5e52 (the timeline task).
4. Fix `DAT_2000_a814` to a BYTE (asm uses `movb`/`testb`; the `undefined2` macro clobbers 0xe815).
5. Change tools/selfplay.sh: a mission ends on `byte[DGROUP:0xe814] != 0`, outcome in
   `word[DGROUP:0x6da0]` -- not on a296 reaching 0.
6. Re-measure the 47-mission sweep against the corrected criterion.

### Attempted step 1 -- and it exposed the real blocker: the TASK-CREATE target itself is unpromoted

Applied reloc section si=0x1b8 and measured. `[DGROUP:0x442]` populated correctly
(`0f69:3c98`), and usefully, `[0x2a]` and `[0x58]` turned out to be ALREADY SET
(`0f69:0279` / `0f69:0314`) by `g_dgroup_init` -- so **si=0x038 does not need applying**.

But the run REGRESSED: `MAP-LOAD` never fires, the battle never starts (`a294=0 a296=0
loaded=0`). Checking every target in the section explains why:

| vector | target | in the port? |
|--------|--------|--------------|
| 0x426 = 0f69:3943 | FUN_1000_2fd3 | yes |
| 0x42a = 0f69:39d4 | FUN_1000_3064 | yes |
| 0x42e = 0f69:3b33 | FUN_1000_31c3 | yes |
| 0x436 = 0f69:3c63 | FUN_1000_32f3 | **NO** |
| 0x43a = 0f69:3c73 | FUN_1000_3303 | **NO** |
| 0x43e = 0f69:3c88 | FUN_1000_3318 | yes |
| **0x442 = 0f69:3c98** | **FUN_1000_3328 (TASK-CREATE)** | **NO** |

**The task-create function itself is unpromoted.** Making the vectors live points three of
seven at nothing, which is strictly worse than the null vectors the shim traps cleanly --
hence the front-end breakage. The change was REVERTED (behaviour confirmed restored:
AZER1 back to a296 16 -> 14).

So applying the section is NOT step 1. The correct order is functions first, wiring last:

1. Promote **FUN_1000_3328** (task-create), **3303**, **32f3** -- the CRT task machinery.
2. Promote **1000:0x5cd8..0x5e97** -- installer A, task A, installer B (0x5dfe), the key
   handler (0x5e36), the timeline task (0x5e52).
3. THEN apply reloc section si=0x1b8 (si=0x038 is unnecessary -- already seeded).
4. Fix `DAT_2000_a814` to a BYTE (asm `movb`/`testb`; `undefined2` clobbers 0xe815).
5. Change tools/selfplay.sh: end on `byte[DGROUP:0xe814] != 0`, outcome in
   `word[DGROUP:0x6da0]`.
6. Re-sweep all 47 against the corrected criterion.

Every address needed for steps 1-3 is now known; none of it requires further search.

### The complete missing set: 13 functions in the 0f69 CRT cluster

Followed the dependency all the way down. `FUN_1000_3328` (task-create) is a thin wrapper:

```
13328: push %ds ; pushf ; cli ; push %cs ; pop %ds   ; DS = 0f69
1332d: mov $0x3bf1,%si      ; the task list head, 0f69:0x3bf1
13330: mov %cs,%cx          ; the task entry's segment
13332: mov $0x3bf6,%di
13335: lcall *%ss:0x32e     ; the LIST-INSERT primitive (SS=DGROUP)
1333a: popf ; pop %ds ; lret
```

`[DGROUP:0x32e]` comes from reloc section **si=0x274** (the CRT list primitives, DGROUP:0x312..0x33e),
which the port also never applies. **But the good news: the two primitives task-create actually needs
are already in the port** -- `[0x32a]` = FUN_1000_4ce7 and `[0x32e]` = FUN_1000_4d05 both exist. The
core list machinery is there; only wrappers and callers are missing.

Both sections have SOME missing targets, which is why neither can be applied before promotion:

```
si=0x274 (list primitives, 12 entries)   missing: 4c96, 4ccd, 4de3, 4e98, 4f07
si=0x1b8 (task wrappers,   7 entries)    missing: 32f3, 3303, 3328
mission task cluster                     missing: 5cd8, 5d9b, 5dfe, 5e36, 5e52
```

That is **13 functions, all in the 0f69 CRT cluster** -- coherent, because nothing in the port has ever
called into the cooperative-task subsystem, so Ghidra never promoted it. This is the subsystem the
shim's extender-gate comment deferred as "a later-stage concern".

FINAL ORDER OF WORK (every address known; no search left):

1. Promote the 13: `1000:32f3 3303 3328` · `1000:4c96 4ccd 4de3 4e98 4f07` ·
   `1000:5cd8 5d9b 5dfe 5e36 5e52`.
2. Apply reloc sections **si=0x274** then **si=0x1b8** (si=0x038 is NOT needed -- `[0x2a]`/`[0x58]`
   are already seeded by g_dgroup_init; verified by measurement).
3. Fix `DAT_2000_a814` to a BYTE (asm `movb`/`testb`; the `undefined2` macro clobbers 0xe815).
4. Change tools/selfplay.sh: a mission ends on `byte[DGROUP:0xe814] != 0`, outcome in
   `word[DGROUP:0x6da0]` -- not on a296 reaching 0.
5. Re-sweep all 47 against the corrected criterion, and verify a mission actually loads WWIN/WLOSE/
   EWIN/ELOSE via be0e (indices 0x0c/0x10/0x14/0x18), which no run has ever done.

Expect each promotion to need the same asm-verified treatment as patches 540-545 (dispatch objects,
dropped carries, WORD-vs-DWORD widths) rather than a mechanical paste.

### Step 3 done (patch 546); and what the list machinery actually is

`DAT_2000_a814` is now a BYTE. All eight asm accesses to DGROUP:0xe814 are byte-width
(`movb`/`testb`); the port's `undefined2` typing made every store also write 0xe815, and 0xe816 is a
live descriptor address (`cb07 mov $0xe816,%bx`, patch 133's MSPRITE0 sheet). Behaviour is unchanged
today -- the flag's readers only test against zero -- but it becomes load-bearing the moment the task
cluster is promoted. Matrix 177/0 both targets.

Also established while reading the primitives: **the CRT task list is a self-modifying far-jump chain.**
`FUN_1000_4ce7` (patch 012, already in the port) writes opcode **0xEA (JMP FAR)** at the new node and
splices `new.fwd := prev.fwd ; prev.fwd := (BX+5):ES`. That is why the timeline task at 1000:0x5e52
begins with `ljmp $0x0,$0x0` -- the node IS a far jump, and the scheduler patches its target. So the
"yield" is not a call/return discipline at all; the tasks are a linked chain of JMP FARs the CRT rewires.

That reframes the remaining work. `FUN_1000_4d05` -- the list walk/remove at `[DGROUP:0x32e]`, the one
`FUN_1000_3328` (task-create) actually calls -- IS present in the port but is PRISTINE: it carries an
`unaff_ES` pseudo-var and host-pointer derefs (`*(int *)(param_3 + 1)`), i.e. it does not work. It
needs the same asm-verified reconstruction as 4ce7 got in patch 012, against asm 0x14d05 (`lds
0x1(%si),%si ; sub $0x5,%si` -- it walks the chain by loading each node's far pointer).

REMAINING (unchanged in substance, now with 4d05's state known):

1. Reconstruct `FUN_1000_4d05` (pristine, unaff_ES + host derefs) -- prerequisite for task-create.
2. Promote `1000:32f3 3303 3328` (the task wrappers -- trivial, thin `cli`/`DS=CS`/`lcall *%ss:0x32a|0x32e` shells).
3. Promote `1000:4c96 4ccd 4de3 4e98 4f07` (the rest of si=0x274's targets).
4. Promote `1000:5cd8 5d9b 5dfe 5e36 5e52` (the mission task cluster).
5. Apply reloc sections si=0x274 then si=0x1b8 (si=0x038 NOT needed -- measured already seeded).
6. Change tools/selfplay.sh to end on `byte[DGROUP:0xe814] != 0` with the outcome in
   `word[DGROUP:0x6da0]`, then re-sweep and confirm be0e finally loads 0x0c/0x10/0x14/0x18.

### CORRECTION: `[DGROUP:0x442]` is the WALK/UNLINK path, not "task-create"

Disassembled `FUN_1000_4d05` (asm 0x14d05-0x14d44) in full:

```
14d05: push %ds
14d06: lea 0x5(%bx),%bp        ; BP = BX+5  (this node's own jump-target offset)
14d09: mov %es,%dx             ; DX = ES    (this node's segment)
14d0d: lds 0x1(%si),%si ; sub $0x5,%si      ; step: follow the chain's far pointer, back up to the header
14d13: mov 0x1(%si),%ax ; cmp %di,%ax ; je 14d27      ; fwd offset == DI ?
14d1a: cmp %bp,%ax ; jne 14d0d                        ; else is it OUR node?
14d1e: mov 0x3(%si),%ax ; cmp %dx,%ax ; jne 14d0d     ; ...and our segment?
14d25: jmp 14d31
14d27: mov 0x3(%si),%ax ; cmp %cx,%ax ; jne 14d21
14d2e: pop %ds ; stc ; lret    ; FOUND DI:CX -> CF=1
14d31: mov %es:0x1(%bx),%ax ; mov %ax,0x1(%si)        ; UNLINK: prev.fwd := node.fwd
14d38: mov %es:0x3(%bx),%ax ; mov %ax,0x3(%si)
14d42: pop %ds ; clc ; lret    ; CF=0
```

So 4d05 walks the SMC chain from DS:SI and either reports DI:CX present (CF=1) or unlinks the
(BX+5):ES node (CF=0). It is the **walk/unlink** primitive; `FUN_1000_4ce7` (patch 012) is the insert.

Following the vectors through:

```
[DGROUP:0x43e] -> 0f69:3c88 -> 1000:3318 -> lcall *%ss:0x32a -> FUN_1000_4ce7  (INSERT)
[DGROUP:0x442] -> 0f69:3c98 -> 1000:3328 -> lcall *%ss:0x32e -> FUN_1000_4d05  (WALK/UNLINK)
```

and the two mission installers (asm 0x15cd8, 0x15dfe) both call **`lcall *0x442`** -- the walk/unlink
path, NOT the insert path. **So labelling `[0x442]` "TASK-CREATE" in the earlier entries was wrong.**

What that means is not yet settled and must not be guessed: either the installers are DEREGISTERING a
previous instance (with the actual insert happening elsewhere), or 4d05's contract is
"present? -> CF=1, else splice out and let the caller insert", or the wrapper at 1000:3328 does more
than the three instructions visible before its `lcall`. The wrapper's own body is
`push ds ; pushf ; cli ; push cs ; pop ds ; mov $0x3bf1,%si ; mov %cs,%cx ; mov $0x3bf6,%di ;
lcall *%ss:0x32e ; popf ; pop ds ; lret` -- it supplies the list head (0f69:0x3bf1) and the search key
(0f69:0x3bf6) but NOT BX/ES, which come from the installer.

NEXT: settle the create/remove question before writing any of it -- the cheapest way is the oracle
write-trace on the task-list head at 0f69:0x3bf1 during a battle, which shows what the original
actually does to the chain at mission entry. Reconstructing an SMC jump-chain primitive from a guessed
contract would corrupt the chain silently, and this is the one place in the remaining work where a
wrong sign is not caught by the matrix.

### ORACLE NEGATIVE RESULT: the original does NOT install tasks on that list during a battle

Ran the write-trace on the task-list head. Address mapping (verified): the original's DGROUP sits at
guest 0x2d190 where the port's is 0x1c000, so the load offset is +0x11190; the head at port-linear
0x13281 (= 0f69:0x3bf1) is guest **0x24411**. Watched 6 bytes, which covers +1..+3 -- exactly where
`FUN_1000_4ce7` writes (`prev.fwd := (BX+5):ES` stores at `[si+1]` and `[si+3]`, not at `[si+0]`).

Result over a full battle run: **33 writes, every one `val=00` from a single `cs=02dd eip=0000444f`** --
a memory-clearing pass at init. No node is ever spliced in.

So the original does not install a task on this list during a battle. That **rules out the theory that
a missing task installation is what stops battles ending**, and by extension weakens the whole
"promote the 13 functions" plan as the route to mission resolution:

- 1000:5dfe and 1000:5cd8 (the installers) are evidently not reached in a battle in the ORIGINAL either;
- so the battle's mission-over flag must be raised by one of the OTHER writers -- 0x4168, 0x4457 or
  0x1a5e9 -- and all three of those ARE present in the port (FUN_0000_4457, FUN_0000_6104 for 0x6115,
  FUN_1000_a5dc for 0x1a5e9).

That inverts the question. It is no longer "what code is missing" but "what CONDITION do the writers
the port already has never see". 0x4168 is the one not yet located in the port and is the first thing
to identify; 0x4457/0x6115/0x1a5e9 should be instrumented to see which of their guards fails.

NOT established, and not to be assumed: whether the timeline task at 1000:5e52 runs in the original by
some other installation path, or whether it belongs to the CAMPAIGN flow rather than battles. The
earlier sections of this item that treat the 13-function promotion as the critical path should be read
with this result in front of them.

NEXT: instrument the three flag-writers the port already has (their asm sites 0x4457, 0x6115, 0x1a5e9)
during a battle and record which guard each one fails; and locate 0x4168 in the port. That is a
counter-measurement, not a search, and it does not risk corrupting an SMC chain.

### COUNTER-MEASUREMENT RESULT: the mission clock could not reach zero -- two width/signedness defects

The counter-measurement above was run: `FUN_1000_a5dc` was instrumented guard by guard during an AZER1
battle (env-gated `FIST_DUMP_END` in the shim, read-only).

First correction to the plan: **`1000:a5ef`/`a60b`/`a6ae`/`a6bb` are NOT unpromoted.** The gap in the
port's function list between `FUN_1000_a5dc` and `FUN_1000_a6c8` is because a5dc's body EXTENDS over
the whole region -- the port has the victory/defeat evaluation, and it transcribes the asm faithfully.
The board:0015 suspicion was wrong.

`FUN_1000_a5dc` has two ways to end a mission, and both were unreachable:

1. **the teardown countdown** word[0x6da2] -- armed to 0xffff (= none pending) at 0x47b2;
2. **the mission clock** byte[0x6da6]:byte[0x6da7]:byte[0x6da8] reaching 00:00:00, which sets
   outcome 2 at 0x1a60b and arms a 2-tick teardown.

The measurement showed the clock frozen. Over 3.19M cooperative ticks:

    [end] t=3186000 out=0000 tmr=ffff | 6da6=0f 6da7=00 6da8=01 | e814=00
    [end] t=3188000 out=0000 tmr=ffff | 6da6=0f 6da7=00 6da8=f4 | e814=00

sub-seconds walked through 0x00 and wrapped to 0xf4 instead of reloading to 0x3b, and seconds stayed
0x00 for the entire run. Two independent Ghidra defects, each fixed by an asm-verified patch:

- **patch 547** -- DGROUP:0x6da4..0x6db0 is a run of THIRTEEN adjacent BYTE variables (122 byte
  accesses in the image, zero word accesses, bracketed by the genuine words 0x6da2 and 0x6db2). All
  thirteen were typed `undefined2`, so each macro aliased its successor. `DAT_2000_2da8 - 1` therefore
  stepped a 16-bit value 0x0000 -> 0xffff and the `cmpb $0xff` underflow test at 0x4732 never matched:
  the borrow chain in `FUN_0000_4712` never propagated. The aliasing also corrupted live neighbours --
  `DAT_2000_2daa = 1` was zeroing 0x6dab, the flag a5dc itself branches on at 0x1a678.
- **patch 548** -- `DAT_2000_2da2 == -1` is a CONSTANT FALSE. The macro is `uint16_t`, which promotes
  to `int`, so it can never equal -1. Ghidra emitted this for NINE of the ten `cmpw $0xffff,0x6da2`
  sites (0x52f2, 0x535e, 0x53c5, 0x54a8, 0x5577, 0x56d1, 0x5767, 0x57f8, 0xbf3c) and the correct
  `!= 0xffff` for the tenth. In each of the nine the test is the function's first statement and its
  body is the then-branch, so nine in-mission command handlers were no-ops for the whole mission.

After both patches the clock runs correctly -- seconds count 0x3b -> 0x00 and borrow into minutes:

    [end] t=0       ... | 6da6=0e 6da7=3b 6da8=14 |
    [end] t=3180000 ... | 6da6=09 6da7=20 6da8=06 |

### The tick cap, not just the code, was hiding this

The measured rate is ~4000 engine ticks per mission-minute (~159 cooperative pumps per engine tick).
AZER1's limit comes from the per-mission table at 0x7b14 (`5167: mov 0x7b14(%bx),%al ; 516e: mov
%al,0x6da6`) and is 15 minutes = **~60000 engine ticks**. `tools/selfplay.sh` was sweeping at 20000.
So even with a working clock the sweep's cap was 3x too small to observe a timeout. Both facts were
needed; either alone would have kept every mission UNRESOLVED.

### The mission time limit is a SETTING, with four values

The table the clock is armed from (`5167: mov 0x7b14(%bx),%al ; 516e: mov %al,0x6da6`, `bx = ax >> 1`)
is four bytes at DGROUP:0x7b14, and they are:

    05 0f 1e ff        =  5 minutes / 15 / 30 / NO LIMIT

So it is a global game option, not a per-mission value, and AZER1's 15 was the default rather than
anything specific to AZER1. 0xff is handled by the engine as "no clock": `1a5ef: cmpb $0xff,0x6da6`
skips the whole time branch in a5dc, and `16597: cmpb $0xff,0x6da6` skips the HUD readout.

Two consequences:

- **The sweep is bounded by this.** At 15 minutes a mission is ~54000 sim ticks and ~25 minutes of
  native CPU. At 30 it is twice that; at "no limit" a mission can only end by victory, defeat or abort,
  so a sweep run with that setting would not terminate on the clock at all.
- **It is one of the settings the goal requires to work**, and all four values need covering: 5 and 30
  are untested, and the 0xff path is the one that makes the victory/defeat evaluation the ONLY exit --
  which makes it the sharpest test of that evaluation, not merely another option.

The selection reaches the clock by two different routes, which matters for how to drive it:

    5167: 8a 87 14 7b   mov 0x7b14(%bx),%al    ; MENU path, bx = selection >> 1
    516b: a2 af 6d      mov %al,0x6daf         ; ... remembered at DGROUP:0x6daf
    516e: a2 a6 6d      mov %al,0x6da6

    d5df: a0 87 e9      mov 0xe987,%al         ; RESTORE path, from the saved block
    d5e2: 0a c0         or  %al,%al
    d5e4: 75 02         jne 0xd5e8
    d5e6: b0 ff         mov $0xff,%al          ; a zero saved value means NO LIMIT
    d5e8: a2 af 6d      mov %al,0x6daf
    d5eb: a2 a6 6d      mov %al,0x6da6

The battle flow the harness drives takes the MENU path, so it gets the default 15 regardless of what is
saved. Reaching 0xff needs a click on the time-limit control, whose coordinates are not known yet, so
it is not the cheapest next experiment.

The cheapest one is the sweep itself: any mission whose own side is WINNING resolves by victory before
the clock can expire, and any whose own side is wiped resolves by defeat. Running all 47 is therefore
also the test of whether outcomes 0 and 1 are reachable at all.

### Still open

- Confirm end to end that expiry raises byte[0xe814] and reaches the outcome screen (long run in
  flight at the time of writing).
- `tools/selfplay.sh` still classifies on `a296` (side eliminated). Its criterion must become
  byte[DGROUP:0xe814] != 0 with the outcome code read from word[DGROUP:0x6da0]: outcome 2 is TIME
  EXPIRED, 0 and 1 are the two evaluated results at 0x1a6ae/0x1a6bb, and 3/4/5 come from the other
  flag-writers. A timeout is a resolved state, but it is not the same resolved state as a victory.
- The elimination path (`live == 0` at 0x1a694, counting objects in the 0xdfbc table with
  `byte[si+0x17] & 8`) still never fires: `live` stays 11..13 while `a296` falls only 16 -> 14 over a
  full mission. Combat happens but is far too slow to wipe a side.
- **The `undefined2 == -1` class is not exhausted.** Seven further sites survive: DGROUP:0x9fd9 (x4),
  0x9fd6, 0xe918 (x2), 0xe82e, 0x6db6, and two in the 0x1000 segment (0x1e02e, 0x1d8e4). Each is a
  constant false today. They were left out of patch 548 because each needs its own asm width and
  branch-sense check first. `DAT_2000_2b74 == -1` (x2) is a different class -- an `int **` sentinel.

### FIRST RESOLVED MISSION

AZER1, native, empty player input, purely cooperative tick:

    [fist] mission resolved: dumping frame + exiting (video-mode=0x13, [0x452]=54294)
    [outcome] a294=120 a296=8  loaded=1 min_a296=8 peak_a296=16  over=1 code=2  TIME EXPIRED

byte[DGROUP:0xe814] raised, word[DGROUP:0x6da0] = 2. The tick count matches the prediction from the
clock rate exactly: a 15-minute limit at 60 sub-ticks per second is 15*60*60 = 54000 sim ticks, and the
engine resolved at 54294.

A third defect was found and fixed on the way there:

- **patch 549** -- the side unit counters at DGROUP:0x6d38..0x6d3b are FOUR BYTES (live A, live B,
  peak A, peak B), written by one word store plus two byte stores at 0x16085, and read back only by
  `cmpb`/`movzbw`. `DAT_2000_2d38`/`2d3a`/`2d3b` were `undefined2`, so a5dc's `DAT_2000_2d38 == 0`
  demanded BOTH sides be wiped and the defeat outcome at 0x1a6bb was unreachable in exactly the case
  it exists for. `_DAT_2000_2d38`/`_DAT_2000_2d3a` were `undefined4` where the asm is `movw`, so
  `_DAT_2000_2d3a = 0` at mission init also cleared 0x6d3c -- the base of the cell-index table that
  the census task FUN_1000_6049 walks.

Patch 548 also changed the simulation materially, which is the expected consequence of nine revived
handlers: before it AZER1 lost 2 of 16 vehicles over a whole mission (a296 16 -> 14); after it, 8
(16 -> 8), and the own-side live count 0x6d38 fell 4 -> 1.

### Harness changes that this required

- `FIST_STOP_ON_OUTCOME=1` (shim): stop the moment byte[0xe814] is raised, and report the outcome code.
  A mission now runs until the ENGINE resolves it rather than to a tick cap.
- `FIST_DUMPTICK` cannot express this bound at all: `[DGROUP:0x452]` is read as a `uint16_t`, so any
  value above 0xffff never fires and the run spins to its wall-clock watchdog. One 15-minute mission is
  ~54000 sim ticks, close enough to the ceiling to be a trap. The shim now warns instead of hanging.
- `tools/selfplay.sh` classifies on `over=`/`code=` from the engine, not on `a296`.

  **CORRECTION.** An earlier revision of this item said a full mission is ~20 minutes of native CPU.
  That was WRONG, and the error is instructive: it was read off the `end3`/`end4` runs, which set
  `FIST_DUMPTICK` to 75000 and 68000. `[DGROUP:0x452]` is read as a `uint16_t`, so neither value can
  ever be reached -- those runs finished their mission in the normal time and then SPUN to the
  wall-clock watchdog waiting for a tick that would never come, and the spin got attributed to the
  mission. Measured properly, with a fresh datadir and nothing else claiming the CPU:

      AZER1, no diagnostic      141 s
      AZER1, FIST_DUMP_END set  146 s     (the diagnostic costs ~3%)
      AZER1, inside the sweep   114 s

  all four runs identical in engine terms ([0x452]=54294, code=2, a294=120, a296=8). So a 15-minute
  mission is about **two minutes** of native CPU, and the whole 47-mission sweep is under an hour at
  two jobs rather than the ~10 hours the wrong figure implied. The ~161 cooperative pumps per sim tick
  is unaffected and still holds (8.7M pumps over 54294 ticks).

### Open, and NOT to be assumed settled

- **Only outcome 2 (TIME EXPIRED) has been observed.** A timeout is a resolved state and it is what the
  goal's criterion reads, but it is not a victory or a defeat. Outcomes 0 and 1 remain unobserved.
- The own-side count 0x6d38 fell 4 -> 1 but not to 0, so the defeat path at 0x1a6bb was approached and
  not reached within the mission clock.
- The victory path at 0x1a694 (`word[0x9790] != 0 && word[0x978e] == 0`) has still never fired.

  Chasing the probe's `live=0` readings found a FOURTH width defect, and also invalidated the readings.
  a5dc is the only writer of 0x978e *in the asm*, and the probe cannot sample inside its recount window
  (that loop makes no pump calls), so a zero reading had to mean another writer -- and there is one:

      #define _DAT_2000_578c (*(undefined4 *)(g_mem+0x2578c))     /* 0x978c */
      ab0a: a3 8c 97      mov  %ax,0x978c                          /* a WORD store */

  `a3` is `mov %ax,moffs16` -- two bytes. As an `undefined4` the port writes FOUR, so every call to this
  per-object update overwrites 0x978e/0x978f, the live-object counter. (0x978c itself is read by byte
  elsewhere -- `testb $0x3,0x978c` at 0xafbb, `mov 0x978c,%al` at 0xafd6 and 0x1abbb -- so it is a word
  written whole and read by halves; only the declared WIDTH is wrong.)

  This does NOT by itself explain the victory failure: a5dc zeroes, counts and tests 0x978e inside one
  call, with no engine code in between, so the clobber cannot corrupt its own decision. What it does
  corrupt is `prev` (0x1a618 copies the leftover 0x978e to 0x9792), which gates the "count decreased"
  notification at 0x1a66b. And it means every `live=` figure reported from outside a5dc in this item is
  worthless. The victory path remains unexplained and has to be measured INSIDE a5dc.
- The ~161 cooperative pumps per sim tick is unexplained. It does not affect determinism -- time passes
  only when pumped -- and it costs far less than first thought (see the CORRECTION above), but if the
  original's ratio is ~1 it still points at something in the pump/wait path.

### With the counter repaired (patch 550), the victory condition is now measurable

Re-ran AZER1 with 550/551 applied. The outcome is bit-identical to the run before them --
`[0x452]=54294`, `code=2`, `a294=120`, `a296=8`, `min_a296=8`, `peak_a296=16` -- so the patches removed
the corruption without perturbing the simulation. What changed is the counter itself:

    before 550:  live=0 / live=13 alternating,   prev=0  for the whole mission
    after  550:  live=13 stable,                 prev=13

So the earlier readings really were the `_DAT_2000_578c` clobber, and the victory test now has a
trustworthy input. The measurement it yields is the next thread:

**`live` stays at exactly 13 for the whole mission while `a296` falls 16 -> 8.**

The first reading of that was that objects are not being unlinked on death. That is WRONG, and the asm
says so. `FUN_1000_b2ef` (0x1b2ef) is the free path and it does clear the slot:

    1b2ef: 8b d8             mov  %ax,%bx
    1b2f2: c1 e3 02          shl  $0x2,%bx           ; stride 4, as a5dc's walk uses
    1b2f5: 8b bf bc df       mov  0xdfbc(%bx),%di
    1b2fd: 80 4d 16 01       orb  $0x1,0x16(%di)     ; mark freed
    1b301: c7 87 bc df 00 00 movw $0x0,0xdfbc(%bx)   ; CLEAR THE SLOT
    1b307: ff 8f be df       decw 0xdfbe(%bx)        ; the entry's second word is a refcount
    1b31c: ff 0e 94 e2       decw 0xe294             ; ... and the per-side count
    1b32a: ff 0e 96 e2       decw 0xe296

a5dc's walk skips a zero slot (`or %si,%si ; je`), so a freed object leaves `live` correctly.

The likelier reading is that the two numbers simply count different populations, and that nothing here
is broken:

  - `0x6d38` is the OWN side's live count and gates the DEFEAT outcome at 0x1a6bb. It fell 4 -> 1 over
    the mission: the player's side is being destroyed.
  - `0x978e` counts every 0xdfbc entry with `byte[si+0x17] & 8`, with no side filter, and gates the
    VICTORY outcome at 0x1a6ae. It is 13 from the first sample and never moves.
  - `a296` is the vehicle roster (stride 0xfb at 0xc05c) -- a third structure again.

So AZER1 as the port simulates it is a mission the player's side is LOSING, and the clock expired
before `0x6d38` reached zero. That is a coherent outcome, not obviously a defect.

What is NOT established: whether the original does the same. A set of 13 that never loses a member
while eight vehicles die is the kind of number that is either correct or badly wrong, and the port
cannot answer it. NEXT: put the DOSBox oracle on `0x978e`, `0x6d38` and `a296` through an AZER1 battle
and compare the three trajectories. If the original's 0x978e also holds constant, the victory path is
simply not AZER1's outcome and the thing to do is find a mission whose own side wins; if it falls, the
defect is in whatever should be clearing `byte[obj+0x17] & 8`.

## What the victory condition actually counts: `byte[obj+0x17] & 8` marks OBJECTIVES

With patch 550 removing the `_DAT_2000_578c` clobber, `live` (DGROUP:0x978e) is trustworthy for the
first time, and it is a flat 13 for the whole of AZER1. Breaking that 13 down by object type at one
instant (AZER1, `FIST_DUMP_REG`, 83 live objects):

    type   total   with [0x17]&8
    0000     2          0
    0001     2          0
    0002     9          2
    0003     3          2
    0010    25          0
    0011     3          0
    0015    27          0
    001a     6          6
    001b     6          3
                       --
                       13

**Nine of the thirteen are types 0x1a/0x1b -- the STATIC MAP OBJECTS** this board's own census
(board:0018) identified as sitting exactly on the terrain and never moving. The other four are two
type-0x02 and two type-0x03 vehicles. Meanwhile the 25 type-0x10 AI tanks and the 27 type-0x15 objects
carry the bit not at all.

So bit 3 of `byte[obj+0x17]` is not "alive" or "enemy" -- it marks the mission's OBJECTIVES, and
FUN_1000_a5dc's outcome 0 at 0x1a6ae is "every objective destroyed" = VICTORY. That is a coherent
design and it explains the constant 13: with the player's side losing (0x6d38 falls 4 -> 1) nothing on
the port's side ever gets close to destroying the objective set.

### What this changes about the two unobserved outcomes

- **Outcome 0 (VICTORY) requires destroying 13 objectives, nine of them static structures.** Under
  EMPTY player input with the player's own tank stationary, that is not obviously something AZER1's AI
  side would ever achieve, so its absence may be correct rather than a defect.
- **Outcome 1 (DEFEAT) is much closer.** It fires when `byte[0x6d38]` -- the own-side live count --
  reaches zero, and AZER1 gets to 1 before the clock expires. A longer clock would very likely reach 0.

That makes the mission TIME LIMIT the decisive variable, and it is a four-value setting (5 / 15 / 30 /
none, from the table at DGROUP:0x7b14). The harness drives the menu path, which takes the 15-minute
default. Two ways to settle it, in order of cost:

1. **Run the sweep.** 47 missions differ in roster balance; if outcomes 0 or 1 are reachable at all at
   15 minutes, some mission will show one. This needs no new capability.
2. **Reach the "no limit" setting**, where a5dc's clock branch is skipped entirely (`1a5ef: cmpb
   $0xff,0x6da6`) and victory/defeat become the ONLY exits. That needs the menu coordinates of the
   time-limit control, or the saved-settings path at 0xd5df where a zero `byte[0xe987]` defaults to
   0xff.

Until one of those runs, "no mission reaches outcome 0 or 1" is an observation about a 15-minute clock
and five missions, not a proven defect.

## The self-play cost changed again: rendering the deferred list is ~6x

Patches 556 and 558 connected c33c's deferred-object producers (`c74d`, `c715`) to their consumer
(`c9af`).  Before them the list was never populated and c9af returned on its first test; now it draws.
Measured on AZER1:

    before 556/558   resolves in 141 s
    after            exceeds the sweep's 900 s budget

and instrumenting the drain shows why -- and shows it is NOT a runaway:

    [c9af] drawn#8200000 cursor=4 count=34 sub=0 di=0x6bf0     (12000 ticks)

`count=34` is 17 list entries, each with up to 8 sub-parts (`sub` 0..7, masked by
`byte[0x9646+sub]` against `byte[si+0x1b]`), so a full drain is ~136 records.  `di` is reset to 0x6bbe
by c33c on every call and advanced to 0x6bf0 (+0x32) by c9af, i.e. ONE record at a time, consumed by
378e's render dispatch before the next -- the same pipeline `caab` and `ca2f` use.  The cursor advances
and wraps, and `FUN_0000_c99c` (called from c33c's re-init) resets count/cursor/sub each walk.

So the port is now performing a per-frame display-list render it previously skipped in full, and the
cost is inherent rather than a defect.  Consequences that must be carried:

- **The sweep budget must be raised.** Missions that resolve now take 90-285 s; AZER1, AZER4 and AZER7
  exceed 900 s.  A full 47-mission sweep needs ~2400 s per mission and several hours.
- **The earlier "~2 minutes per mission" figure is obsolete**, as the "~20 minutes" figure before it
  was wrong for a different reason.  Any cost estimate in this item should be read with the patch level
  it was measured at.

Partial sweep at 554-558 (8 of 47 dispatched, 900 s budget, 2 jobs, sharing the machine with a matrix
run):

    AZER2 RESOLVED code2 277s   AZER3 RESOLVED code2  90s   AZER5 RESOLVED code2 285s
    AZER6 RESOLVED code2 271s   CYPRUS1 RESOLVED code2 263s
    AZER1 TIMEOUT   AZER4 TIMEOUT   AZER7 TIMEOUT      (0 crashes)

Zero crashes is the result that matters here -- before 554/555/558 this set produced SEGVs.

## The gate variables measured directly -- and the instrument that was watching the wrong pair

Every "no outcome" observation in this item up to here was made through `FIST_SIMTRACE`, which prints
`a294`/`a296` (DGROUP:0xe294/0xe296, the two ROSTER occupancy counts). **The outcome gate reads neither
of them.** Decoding 1a678..1a6c7 instruction by instruction:

    1a678  cmpb $0x0,0x6dab   jne -> lret      no verdict at all while 0x6dab is set
    1a67f  cmpb $0x0,0x6d3a   je  -> lret      own-side HIGH-WATER zero = pre-spawn, no verdict
    1a686  cmpb $0x0,0x6d38   je  -> 1a6bb     own side wiped out            -> DEFEAT  (code 1)
    1a68d  cmpw $0x0,0x9790   je  -> lret      no objective ever existed, no verdict
    1a694  cmpw $0x0,0x978e   jne -> lret      objectives remain
                              else 1a6ae       every objective destroyed     -> VICTORY (code 0)

and their producer, the side-strength counter at 1a6049, is a two-pass walk over the slot table at
DGROUP:0x6d3c (2-byte near pointers, 16 per side; a slot counts as ALIVE when `word[di] != 0x17`):

    byte[0x6d38] = cl   own side alive        byte[0x6d3a] = max(byte[0x6d3a], cl)   own high-water
    byte[0x6d39] = ch   other side alive      byte[0x6d3b] = max(byte[0x6d3b], ch)   its high-water

`FIST_SIMTRACE` now prints these five plus `0x978e`/`0x9790`/`0x6da0`/`0x6da2` as a `gate{...}` group,
so the verdict is read from the words the engine actually branches on. AZER3, native, one full run to
the engine's own TIME EXPIRED:

    t=314    live=97  goals=11  gate{alive=6/16 hw=6/16 obj=11/11 edit=0 verdict=0/65535}
    t=54283  live=136 goals=11  gate{alive=5/13 hw=6/16 obj=11/11 edit=0 verdict=2/2}

Three things this settles:

- **The gate machinery is correct and demonstrably fires.** `verdict` flips 0/65535 -> 2/2 at exactly
  the tick the clock reaches zero. Nothing about the outcome path is broken.
- **Combat kills units.** Own side 6 -> 5, other side 16 -> 13 over the mission. The chain this item
  spent so long opening does work end to end.
- **`obj` NEVER MOVES.** Eleven objectives at t=314, eleven at t=54283. VICTORY is gated on `0x978e`
  reaching zero, so in this run it is unreachable by construction -- not "nearly reached".

## The objectives are never damaged AT ALL, and it is not the accept filter

Censusing the objective set itself (the objects the gate counts, `byte[+0x17]&8`) with each one's
damage accumulator `byte[+0x1a]`, its destruction threshold `byte[+0x1b]` (bd09 destroys when the 8-bit
add carries or acc >= thr) and its team bit (`byte[word[obj]-0x19ec]&1`, b1df's own keying) -- AZER3,
first tick with a loaded roster:

    4x type 0003  team 1  acc=17 thr=6    <- the PLAYER's own team
    3x type 001b  team 0  acc=0  thr=223/31/185
    4x type 001a  team 0  acc=0  thr=80/80/4/4
       PLAYER     team 1

and at the end of the run every team-0 objective still reads `acc=0`. Not one point of damage is ever
applied to an objective in 54283 ticks. Two of them have `thr=4` -- a single hit would destroy them.

The obvious suspect, target-acquisition filtering structures out, is **eliminated**. The accept test at
0000:a6e3 rejects a candidate on exactly two bits:

    a6e9  testb $0x40,0x17(%si)  jne reject
    a6ef  testb $0x1,0x16(%si)   jne reject      (bit 0 = bd36's destroyed marker)

and the enemy objectives carry `f17=1c` / `f16=4e`, so they pass both. Measuring it directly rather
than arguing it: counting, per tick, how many acquired targets (`word[+0x97]`) are objective-flagged,
AZER3 reaches **`tgtobj=4`, `tgtstruct=3`** and holds nonzero values across the run. AI units DO
select objective structures as targets.

So the break is neither in the verdict, nor in the objective bookkeeping, nor in target selection. It
is between **target acquired** and **damage applied** -- the impact/damage leg, whose entry bd09 is
itself correct (its carry test `((unsigned)old + al) <= 0xff` faithfully reproduces `bd2b jb`, patch
447) and is dispatched per-type from the table at DGROUP:0xe584. Note also that `firereq` -- the
`[+0x17]&0x80` fire-request bit -- reads 0 at every sampled tick of the whole run while units still
die, so either the bit is consumed within a tick or the sampled bit is not the one the fire path sets.
That is the next thing to measure, and it is a narrow target.

### Whether this is a DEFECT at all is now an oracle question, and the oracle run exists

Four of AZER3's eleven objectives sit on the PLAYER's own team, which makes "destroy every objective"
an unlikely reading of the mission and reinforces this item's earlier finding that outcome selection is
data-driven from the .MS3 script. Whether a faithful engine resolves a battle at all when the player
never touches the controls cannot be settled from the port; it needs the original.

`tools/oracle/census_outcome.sh` (new) settles it. It drives stock FIST.RUN under the instrumented
DOSBox to the default battle with the same click sequence `tools/selfplay.sh` feeds the port, then
sits still, with `FIST_WATCHFLAT=0x22da0 FIST_WATCHFLATSPAN=0x10` armed -- the engine-flat address of
DGROUP:0x6da0 (DGROUP is segment 0x1c00 over an image loaded at linear 0), so the span covers the
outcome word, its countdown 0x6da2, the mission clock 0x6da6/7/8 and the 0x6dab gate, and every write
is logged with the live `cs:eip` that made it. Low traffic, and decisive:

- original writes 0 or 1 -> the port has a real defect on the damage leg, and this item stays a bug;
- original writes 2 (TIME EXPIRED) with its objectives untouched -> the port is FAITHFUL, and the
  goal's "every mission plays through to a resolved victory/defeat state" is not something this engine
  does with an idle player. That would be a finding about the requirement, not a licence to force an
  outcome -- forcing one would be exactly the Umgehung the project forbids.

Until that log is read, "outcomes 0 and 1 never fire" remains an observation, as this item already
warned two sections above -- now with the gate variables actually in view.

## CORRECTION and the second mission-end path: the player's tank, the PL:1 prompt, and what "AUTO CONTROL" is

The previous section's "the player tank never moved once across the entire run" was measured on AZER3
and does not generalise.  On the DEFAULT battle (AZER1 -- the one the oracle's click sequence also
selects) the port's player object DRIVES: X 584582 -> 711214, Y 1141637 -> 993366 over 7500 ticks,
hull speed word[+0x57] = 224 from the first in-mission tick, velocity [+0x59]/[+0x5b] nonzero.  On AZER3
word[+0x57] is 0 throughout.  The two missions differ in the player's initial/commanded speed, not in
the port's code path.

**What drives it is NOT an autopilot.**  Traced every candidate:

- `a57a` (called each tick by all four type-A templates, acting only on the player) is the JOYSTICK
  DEVICE controller: `table_976c[word[0x8b43]]` where word[0x8b43] is the SETTINGS screen's CONTROL
  field (NO JOYSTICK / STD JOYSTICK / FLIGHTSTICK W-THROTTLE / THRUSTMASTER FCS / CH FLIGHTSTICK PRO /
  THRUSTMASTER WCS / EXTERNAL DRIVER = 0..6 -> a5ea a5eb a5f1 a62a a624 a630 a59b), then
  `table_9778[byte[+0xa0]]` = the key sub-state a487 sets.  With device 0 and no keys both are `ret`.
  In the port the whole controller was a silent no-op (the callers passed the host pointer, so its
  `cmp 0x6d34,%di` never matched) -- patch 562 restores it and its unpatched drive-state cluster
  (a376/a3a8/a3e2/a5b0/a5cd/a5f1).  Real, but not the self-play question.
- The HUD's "AUTO CONTROL" is the SETTINGS screen's separate AUTO TURRET CONTROL checkbox
  (byte[+0x3f]&8, toggled by the cockpit click handlers at 0x74b9..0x8471) -- turret auto-aim.
- The 12 `cmp 0x6d34,%di` "is the player" sites in the engine are aim/HUD/message/damage specials;
  none drives the hull.  `a631` even EXCLUDES the player from the unit-AI targeting call.
- `[+0x38]` is the GUN ELEVATION, not the speed (a202 nudges it on the elevation keys with clamps
  0x238c / -0x1554, a2a8 zeroes it on target loss, a265 writes a18e's pitch into it).  The hull speed
  is word[+0x57] (a410..a431, clamp +-0xfe).

So under empty input nothing in the type-0 method touches the hull; the AZER1 motion comes from the
mission's initial state (word[+0x57]=224 at the first tick), and it is the ORACLE, not the code, that
says whether the original's tank moves the same way -- `tools/oracle/census_outcome.sh` is armed on
the default battle for exactly that (its RAM dump also yields the original's word[0x8b43] at
0x2d190+0x8b43 and the player object at 0x2d190+0xc05c).

**The second mission-end path.**  At t=7550 the port's AZER1 player object is destroyed (own side 4 ->
2, the 0xc05c slot freed) and the engine leaves the sim: the backtrace sits in `e4bb <- e714 <- cae6`
pumping the PIT wait in 30f8 with [0x452] racing, and the framebuffer shows the cockpit with the
**"PL:1" platoon-vehicle selector** in the windshield -- the prompt the original raises when your
vehicle is gone (`e714` is the mission loop itself, `cb32` its single call site in the mission-entry
sequence).  The a5dc verdict never fired (`verdict=0/65535`, own alive 2 not 0), so:

- `over=`/`code=` (the harness's classifier) sees only the a5dc verdict; a player-death end reads as
  TIMEOUT.  The sweep's "TIMEOUT" class conflates "still fighting" with "sitting at PL:1".
- Under EMPTY input this prompt is never dismissed, so on every mission where the player's vehicle
  dies before the clock expires the run cannot reach any verdict.  That is the engine's own behaviour,
  not a port defect.  Whether the sweep may send one dismissing click (the same class of input it
  already uses to enter the mission) is a policy decision for the requirement's owner, not something to
  paper over in the harness.

**A visible HUD defect found on that screen: "GOALS REMAINING:+U".**  The original shows the count.
The chain: a5dc `1a655 mov $0x1f8c,%si ; lcall 0:2e8` formats word[0x978e] into DGROUP:0x1f8c and copies
the two bytes at 0x1f8f into the HUD text at es:0x2d8e.  `02e8` installs the per-character emitter
`030b` (`mov %al,(%si); inc %si; lret`) as the CRT printer's callback ([0x684]=0x30b,[0x686]=cs), calls
0f69:5d8b = FUN_1000_541b, and NUL-terminates at the advanced SI.  541b BCD-packs the value (DX =
ten-thousands, CX = the four remaining digits) and forces byte[0x2672]=1 so all five digits print
zero-padded -- "00013", which is why the copy takes buffer+3.  54ac/54b4/54c2 are a fall-through
nibble-emitter chain passing each digit in AL through `lcall *[0x684]`.  In the port: `FUN_0000_030b`
does not exist (unpromoted dispatch target, board:0015 class -- only 0x31c is in the icall table), the
emitters call the callback with NO arguments (AL and SI dropped), 541b passes `unaff_CS` where the
packed CX goes, and 02e8 writes its NUL at the buffer START.  Nothing is ever written, the buffer keeps
stale bytes, and the HUD prints them.  Patch 563.

## ORACLE RESULT: the original reaches DEFEAT on AZER1 with no input -- and two corrections

`tools/oracle/census_outcome.sh` (stock FIST.RUN under the instrumented DOSBox, default battle = AZER1,
no input after ACCEPT, `FIST_WATCHPHYS` on the original's DGROUP:0x6da0 at guest 0x33f30), twice:

    ph=33f30 <- 01   cs:eip 2082:b02b = flat 0x2b84b = port 0x1a6bb   movw $0x1,0x6da0   DEFEAT
    ph=33f32 <- 05   cs:eip 2082:b031 = flat 0x2b851 = port 0x1a6c1   movw $0x5,0x6da2
    ph=33f32 <- 04 03 02 01 00   from 2082:af53 = port 0x1a5e3         decw 0x6da2 (x5)

at mission clock 09:11 remaining = 5:49 elapsed, i.e. **the original's own side is wiped out** (the
1a686 `cmpb $0,0x6d38` edge) with the sim running the whole way.  The first run's evidence died with
/tmp in a reboot; the second reproduced it.  DOSBox runs this mission at ~0.3x real time (clock 11:19
after 12 min wall), so the 15-minute limit is unreachable inside the 25-minute window and TIME EXPIRED
was never a candidate here.  The verdict is real, reproducible, and it is DEFEAT.

**Correction 1 -- "AUTO CONTROL" IS hull auto-drive, not the turret checkbox.**  `ab03` sits in the
type-0 (player) update template's table1 at entries 7 and 15 (`0x7c91`: 7d69 a202 a631 aa37 a202 7cbf
7cb2 **ab03** 7cb1 a202 7cb1 a904 a9a0 a202 a46e **ab03**) with no player exclusion, and it dispatches the
movement state machine (`table_98dc`/`table_98fc` -> `ad2f`/`ad08` -> `ac9e` the formation follower
that sets bit 1 of [+0x40] and the waypoint [+0x49]/[+0x4d]; `ad62` the class-speed setter, table
DGROUP:0x992c whose entry 3 is exactly the 0xe0 = 224 the port's player carries).  The player's own
tank drives itself in formation until manual takeover (aae8's bit-0 path).  `a631` excludes the player
only from `0f69:aa1b` = FUN_1000_a0ab, the *targeting* pass.  So the port's player driving at 224 is
FAITHFUL, and the previous section's "no code path drives the player's hull without input" was wrong.

**Correction 2 -- there is no "PL: prompt" mission-end path.**  `FIST_OBJTRAP=0xe814` on the port's
AZER1: **nobody writes the mission-over flag** in the first 9000 ticks, and at t=9000 the sim is still
in-mission with `alive=2/10` and registry index 0 = 0xa84c, a small object.  What happened at t=7550 is
that the player's vehicle was destroyed, its registry entry freed, and **index 0 reused by the next
spawn** -- `fbc[0]` was never a stable handle for the player; the player is `word[DGROUP:0x6d34]`
(every `cmp 0x6d34,%di` in the engine).  The "player=0000 -> at the PL: prompt" stop and the
`PLAYERDEAD` sweep class were built on that misread and are removed; `[outcome]` and `FIST_SIMTRACE`
now read the player through 0x6d34 and report its destroyed bit.  The e714/e4bb state gdb'd three days
ago was reached at some LATER point that run never characterised (e714 builds MAINMENU.MRL -- it is the
main menu's screen loop, not a mission dialog; the "PL: 1" box over an unrepainted cockpit is a menu
modal whose identity is still open).

What is now being measured: the port's AZER1 run to its own verdict (`FIST_STOP_ON_OUTCOME`, no tick
cap, `FIST_OBJTRAP=0xe814` armed) -- if it writes DEFEAT the two agree in kind and the comparison moves
to the tick; if it does not, the port has a divergence between the third own-vehicle death (t=7550,
4:10) and the original's fourth (5:49).  `tools/oracle/census_player.sh` (per-write trace of the
original's player object, `FIST_WATCHPHYS` over 0x2d190+0xc05c span 0x80) is the next oracle run
either way: it gives the original's player X/Y/speed/damage per write, against the port's per tick.

## The debrief numbers: the port's combat is an order of magnitude less lethal than the original's

The original's post-verdict screen (`ref/oracle_azer1_debrief_defeat_320.png`, captured by census_outcome.sh
at the end of the DEFEAT run) reads:

    MISSION LOST            OBJECTIVES REMAINING: 08
    ENEMY GROUND KILLS: 10  ENEMY AIR KILLS: 01
    GROUND UNITS LOST: 04   UNITS REMAINING: 00     AIR UNITS LOST: 00   FRATRICIDE: 00

against the port's AZER1 run to its own verdict (`FIST_SIMTRACE` with the gate group, 54283 ticks):

                              original (5:49)     port (15:00)
    objectives destroyed      5 of 13             0 of 13
    enemy units killed        11                  2   (a296 12 -> 10, both before t=7133)
    own units lost            4 of 4              2 of 4 (t=789 and t=7550; none after)
    verdict                   DEFEAT              TIME EXPIRED

Two things this settles:

- **The original's AI destroys objectives.**  Five of the thirteen fell to it with the player idle, so
  `obj=13/13` in the port is a DEFECT, not "the player's job" -- the earlier hedge in this item is
  withdrawn.
- **The port's kill chain works but almost never completes.**  Both sides fire throughout (a294, the
  small-object roster, oscillates 112..123 every tick to the end), targets are acquired (tgt/tgtobj
  nonzero), yet after t=7550 (4:10) not one unit on either side dies for eleven minutes.  The original
  kills 15 units in under six.  Whatever is wrong sits between the shot and the kill -- flight, the
  bb1b proximity test (`cx = word[si+0x14]+0x100 ; call 0xea9 ; call 0xc14f`), or bd09's damage
  lookup -- and it is wrong by a factor of ten, not by a rounding.

Instrument corrections that the measurement needed (all shim-side): `FIST_STOP_ON_OUTCOME` now latches
on the verdict countdown (0x6da2 != 0xffff, or 0xe814) and is gated on the roster high-water, not on
`byte[0x1549]==0x1c` -- that byte cycles 00/1c/20/22 within a tick (it is a phase byte, not a
mission-state flag), which is why AZER1 "timed out" in the sweep while AZER2/3/5/6 "resolved" by phase
luck; `[outcome]` prints the verdict words and the player via 0x6d34.  Oracle-side: the instrumented
DOSBox's watch log capped at 40000 entries (FIST_WATCHMAX now overrides it, file kept open), and
adlib.cpp's OPL hook was stealing SIGUSR2 from the RAM-dump handler on its first port write -- every
mid-mission `.ram.bin` request had silently gone to the OPL logger.  Both fixed and rebuilt.

The mission's clock runs at 60 ticks/s ([0x452]); a 15-minute mission is 54000 ticks.  Earlier
"30 ticks/s" figures in this item were derived from a wrong 30-minute assumption for AZER3.

## The kill chain, found and repaired in seven patches -- and the first DEFEAT the port ever wrote

Working from the original's debrief numbers (10 ground + 1 air kills, 4 own lost, 5 objectives at
5:49) against the port's (2 / 2 / 0 at 15:00), the chain was opened link by link on AZER1 self-play:

    564  c14f's ten per-type INTERACTION handlers (DGROUP:0xe518) were all unpromoted; the icall trap
         left 0ea9's carry in place, so every object inside a shell's 2-D range circle -- trees, the
         type-0x10 markers, wrecks -- was "hit" and the shell detonated there.  With them: kills 2->6,
         objectives 0->3 by 5:52 (the original's order of magnitude).
    565  the icall FUN map is bsearched but had been left unsorted by 534/535/543/556/563; patch 535's
         four render methods existed and were "unmapped".  Sort a private copy once.
    566  97d5 (the T-80 template) dispatched its table1 DS-relative (patch 252) into DGROUP garbage --
         FIST_TRACE_TRAPS listed that garbage verbatim -- so the three T-80s never targeted, moved or
         fired for 15 minutes.  The whole battle had been the nine AH-64s.
    567  the T-80 cluster 566 switched on: 997d/986e/987b/98fb, the fire step 99a2 and its four spawns
         9b39/9b7e/9bc3/9c15, the fifth a57a call site patch 562 missed -- all pristine host derefs.
    568  60e5, the side-table clear on a failed wreck spawn, walked the 32 WORD slots as ints: a dead
         vehicle stayed counted alive and the DEFEAT edge could not fire.  After it the port's own
         side reaches 0 and `0x6da0 = 1` is written -- the FIRST time the port has ever produced a
         verdict other than TIME EXPIRED.
    569  378e passed nothing to its render methods (BX = the deferred record 0x6bbe in the asm); 3823/
         37cd read stack residue and hung the MGA blitter on a record pointer inside vehicle 11.
    570  the MGA line clipper (m_mga 1677) was 32-bit and never wrote the clipped endpoint back; the
         frame after the last own vehicle dies projects a polyline point to x=28759 and the unclipped
         draw left g_mem.  Rebuilt 1:1 in 16-bit arithmetic.

Each was the producer/consumer shape this board keeps recording: the correct patch reaches code that
had never executed and finds it pristine.  After 570 the run writes DEFEAT, starts the countdown, and
hangs in the sprite blitter on records whose sprite HEADERS are garbage -- the sheets were never
loaded -- which is board:0024 (the memory manager and the model budget), the gate to the mission-over
exit and to every drawn object.

Spread, not defect: two oracle runs of the same mission gave DEFEAT at 5:49 with 11 kills and DEFEAT
at 1:38 with 1 kill -- the original's battles vary widely run to run (the RNG is not seeded from the
tick), so a port run has to be compared on distribution and mechanics, not on a single tally.  The
one-shot player death (109 vs 122 damage, 2:07 vs 1:38) and the per-phase kill rates now sit inside
that spread.

Corrections to earlier sections: FIST_OBJTRAP never fires in the self-play flow (zero hits on a byte
written every tick), so the "nobody writes 0xe814" claim was an unsupported reading of a dead
instrument (the conclusion held on the clock trace); FIST_WATCHBYTE replaces it.  390d (render method
id 0x18) is still pristine and will fault when first emitted; c47d/c4a2 never deliver their label
records because their C sets no CF (board:0015).
