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
