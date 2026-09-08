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
