Type: bug
Status: open
Parent: 0017
Title: unit line-of-sight uses the engine's own unit altitude, not a shim stand-in

The op-0x58 line-of-sight service in `tools/native_main.c` is a hand-written shim reimplementation,
and it carries an explicit approximation in its own comment:

```c
/* board:0012 unit-Z terrain-follow (camera-Z class stand-in for the absent overlay unit-altitude:
 * the flight-model per-unit ground-clamp is paged out, exactly like the camera-Z at op-0x24).
 * Use the LOS's OWN fixed-10 terrain index so endpoints sit on the sampled terrain + eye. */
oz = ((int32_t)hm[oi & 0x3fffff] << 8) + 1792;
cz = ((int32_t)hm[ci & 0x3fffff] << 8) + 1792;
```

Both endpoints' Z are FORCED to `terrain_height + 1792` rather than read from the units. The goal
forbids approximations, and this one is now load-bearing: it decides every engagement.

## Why it matters now

With patches 538 and 539 in, the aim and range primitives are correct and the LOS test became the
dominant filter. Measured in AZER1 over 20000 ticks:

| | before 539 | after 539 |
|---|---|---|
| op-0x58 calls   | 17795 | 21377 |
| out of range    |  8591 | 12607 |
| occluded        |  2523 |  8324 |
| **VISIBLE**     |  6681 |   446 |

Of 8770 in-range queries only 446 are visible -- 95% occlusion. Units therefore almost never acquire a
target, which is why AZER1 now scores one kill (a296 16 -> 15) instead of resolving.

95% occlusion over voxel terrain at ~150k-unit separation is not obviously wrong, so this item does NOT
assert the LOS is too strict. It asserts something narrower and certain: the endpoints are a stand-in,
so the number cannot be trusted either way until the real per-unit altitude feeds it.

NEXT: recover the engine's per-unit ground-clamp (the "paged out" overlay altitude) and drive both
endpoints from it, then re-measure the occlusion ratio. Until then the mission-resolution numbers in
board:0017 rest on an approximation and must be reported as such.

## Measured: the engine DOES supply endpoints, and swapping them in is not the answer either

The shim reads both endpoints from the extender TCB (`tcb+0xd2..0xe6`) and then overwrites the two Z
words. Logging what the engine supplied *before* the overwrite (AZER1):

```
[los 0] engine oz=2048  cz=3584  | substituted oz=21760 cz=20224
[los 3] engine oz=4096  cz=1536  | substituted oz=19200 cz=13312
[los 7] engine oz=4096  cz=19200 | substituted oz=19200 cz=17152
[los 9] engine oz=4096  cz=19968 | substituted oz=19200 cz=17920
```

So the engine supplies non-zero, varying values, not the zeros the stand-in's premise assumes. The
observer `oz` is always 2048 or 4096 -- consistent with a unit height -- while `cz` ranges 1536..19968,
which spans both "small like a unit height" and "large like a terrain altitude". The two are therefore
not obviously in the same space, and that ambiguity is the whole problem.

A/B behind an env gate, 20000 ticks:

| | AZER1 VISIBLE | AZER1 min_a296 | AZER4 VISIBLE | AZER4 min_a296 |
|---|---|---|---|---|
| shim stand-in   |  446 | 15 | 1285 | 15 |
| engine's own Z  |   68 | 16 | 4705 | 14 |

Split: the engine's Z makes AZER1 strictly worse (fewer visible, one fewer kill) and AZER4 better.
Neither is right, so the default was NOT changed and the gate was not kept -- swapping one guess for
another is not progress, and a 2-mission A/B cannot decide it.

NEXT: this needs the oracle, not experimentation. Arm the patched DOSBox on an original AZER1 run
(`FIST_MEMARM_BOOT=1 FISTLOG=<prefix>` with `FIST_WATCHFLAT` on the TCB endpoint words) and read what
the ORIGINAL puts in `tcb+0xda` / `tcb+0xe6` at op-0x58, plus what its op-0x58 returns. That is the
only way to settle both the endpoint space and the occlusion ratio.
