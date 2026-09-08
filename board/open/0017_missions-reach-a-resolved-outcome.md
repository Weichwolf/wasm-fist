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
