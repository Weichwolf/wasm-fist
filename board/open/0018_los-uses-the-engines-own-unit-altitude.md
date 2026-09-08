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

## CONFIRMED: the LOS stand-in is what stops missions resolving

Traced the stall to its cause. At the stall point in SAUDI2 (t=35439, a296 stuck at 8):

```
[stall] t=35439 a296=8 | fire-req(a286)=43 shots=0 ammo-empty=0 roster-full=18 |
        [t2 tgt=0000 a=90/16/5] [t3 tgt=0000 a=11264/36865/1025] [t2 tgt=0000 a=90/13/5]
```

Every live vehicle has `tgt=0000` -- no target -- while ammo is plentiful and fire requests are frozen
at 43. So the blocker is target ACQUISITION, not ammo, weapons, or the roster.

`FUN_0000_a6e3` (target acquire, patch 425) sets `word[self+0x97] = candidate` and then validates it
with `e20a` -> the op-0x58 LOS. **When LOS says occluded it clears the target again.** With the
stand-in occluding 95% of in-range queries, units acquire almost nothing, and whatever they do acquire
they lose.

Chain: LOS stand-in -> 95% occlusion -> a6e3 clears the target -> tgt=0000 -> no fire requests ->
attrition stalls -> no mission resolves.

Proof by probe (diagnostic only, NOT committed as a default): forcing the terrain-occlusion test to
pass roughly doubles attrition in SAUDI2:

```
normal:          13 -> 7
LOS forced open: 13 -> 3
```

SAUDI2 still does not reach 0, so LOS is not the *only* factor -- but it is the dominant gate, and no
mission-resolution number is meaningful until the endpoints are real. This item is now the top blocker
for board:0017.

## What the endpoints MEAN (from the engine's own code, not a guess)

`FUN_0000_e20a` (patch 425) is what fills the TCB before posting op-0x58, and it is unambiguous:

```c
*(uint32_t *)(tcb + 0xd2) = dword[self + 4];      /* self X */
*(uint32_t *)(tcb + 0xd6) = dword[self + 8];      /* self Y */
*(uint32_t *)(tcb + 0xda) = word[DGROUP:(selftype*2 - 0x1a40)] + dword[self + 0xc];
*(uint32_t *)(tcb + 0xde) = dword[tgt + 4];       /* target X */
*(uint32_t *)(tcb + 0xe2) = dword[tgt + 8];       /* target Y */
*(uint32_t *)(tcb + 0xe6) = <target's height LUT> + dword[tgt + 0xc];
```

So each Z endpoint is **a per-type eye-height LUT plus the object's own Z**. That is exactly what an LOS
wants -- the stand-in is not filling a hole in the data, it is discarding data that is already there.

What is NOT established is the SCALE relationship. Vehicles carry `dword[obj+0xc]` around 1280..2048,
while the extender's terrain is `heightmap_byte << 8`, i.e. 12544..22016 for the same map. Either the
engine Z is a height above ground (and terrain must be added), or the two are in different units, or
the type-0x1b objects at Z=7936..13824 are a third case.

## Three endpoint models tested; all three are inconclusive

| model | AZER1 VISIBLE | AZER1 kills | AZER4 kills | SAUDI2 kills |
|---|---|---|---|---|
| stand-in `terrain + 1792`      |   446 | 1 | 5 | 6 |
| engine Z raw                   |    68 | 0 | 6 | 6 |
| `terrain + engine Z`           | 36142 | 1 | 6 | **4** |

Each helps some missions and hurts others; `terrain + engine Z` makes LOS wildly permissive (80x the
visible count) because a target Z of 19968 lands the endpoint far above the terrain. None was shipped.

## Ruled out, so nobody repeats it

- The extender is NOT missing from the tree: `tools/extract_image.py` extracts the whole 32-bit app
  image (FIST.RUN 0x583a..0x117ca = 49040 bytes = `re_out/fist_image.bin`). There is no second module
  and no other file: the game ships only FIST.RUN, FIST.DAT, MGAVIDEO.DVR and SOUNDDVR.DVR.
- `FIST_EXTGATE_LIN` (0x8799) is where the ENGINE far-jumps, not the dispatcher's address inside the
  extender image; disassembling `fist_image.bin` at 0x8799 gives unrelated terrain code.
- The only indexed jump in the extender image, `0xae56 jmp *0xae60(,%eax,4)`, is a 9-entry SPRITE-TYPE
  dispatcher (the adjacent string is "PKLOT: Illegal Sprite Type"), not the service dispatcher.
- `re_out/fist_ext.c` decompiles only part of the extender (the KDV player cluster); its single
  `switch` is a blit routine.

NEXT: locate the extender's PM service dispatcher in `re_out/fist_image.bin` and read op-0x58's real
handler. That is ground truth already in the repository and settles both the endpoint scale and the
occlusion rule without any oracle run. Until then no mission-resolution number is meaningful, because
target acquisition is decided by whichever of the three models is installed.

## The original op-0x58 handler is IN THE TREE, and it settles the scale

`re_out/fist_image.bin` contains the extender's real LOS service at **0x8030-0x811e**. Decoded:

```
8030: mov 0xc93,%edi                  ; the TCB
8036: mov 0xde(%edi),%ebx  ...        ; target X/Y/Z  (0xde/0xe2/0xe6)
8048: sub 0xd2(%edi),%ebx  ...        ; minus self X/Y/Z (0xd2/0xd6/0xda)
805a: cmp $0x40000,%edx / jge         ; the same +-0x40000 range gate the shim has
808a: shl $0xd,%edx ; shl $0xd,%ebx ; neg %edx ; shl $0x10,%eax
8095: mov $0x3000000,%ebp             ; the same step normalisation loop
80ba: mov 0x85bc,%esi                 ; the heightmap
80d1: mov 0xd2(%edi),%ebx ...         ; march from SELF
80eb: shl $0x10,%ebp                  ; self Z << 16
810a: movzbl (%eax,%esi,1),%edi       ; terrain byte
810e: shl $0x18,%edi                  ; h << 24
8111: cmp %ebp,%edi / jae 811c        ; OCCLUDED
8117: xor %eax,%eax / not %eax / ret  ; -1  = clear LOS
811c: xor %eax,%eax / ret             ;  0  = blocked
```

Two conclusions, neither of which needed an oracle run:

1. **The shim's marching is a faithful reimplementation** -- same range gate, same normalisation, same
   index packing, same return values. Only the ENDPOINTS are substituted.
2. **The scale is proven.** `shl $0x10` on the self Z against `shl $0x18` on the terrain byte means the
   comparison is `h<<8` vs `Z`. So an object's Z must be in terrain scale, i.e. `terrain_byte << 8`.
   The stand-in is therefore not a modelling choice -- it is masking wrong object Z.

## Census: which object types are actually on the ground (AZER1, t>8000)

`Z[+0xc]` is always `ground_byte[+0xd] << 8`, so the engine's altitude really is the ground byte.

| type | ground byte | terrain byte | delta |
|------|-------------|--------------|-------|
| 0x1a, 0x1b (static map objects) | 31..63 | 31..63 | **exactly 0** |
| 0x10 (AI tanks, moving)         | 35..63 | 23..65 | -26..+24 (slew lag) |
| 0x02, 0x03, 0x11                | 0..82  | 37..79 | -79..+35 |
| **0x00, 0x01**                  | **5, 6** | **36, 72** | **-31, -66** |

The mechanism WORKS -- types 0x1a/0x1b sit exactly on the terrain. Types 0x00 and 0x01 are buried by
31 and 66 height units (about 8000 and 17000 in Z), and they are exactly the two whose step methods are
`7c1d` and `87df`, neither of which calls the op-0x54 terrain probe. The only probe callers are
`9a50`, `9ae9`, `9db1`, `9e2b`, `adcd` (plus two extender-module functions).

Note also that op-0x54 fires FEWER THAN 500 times in a 20000-tick AZER1 run -- for ~150 live objects
that is nowhere near "every 4th frame per object", so the per-frame terrain follow is largely not
running even for the types that do call it.

REVISED NEXT STEP (this supersedes "capture the original's TCB endpoints"): the endpoints are already
correct by construction -- `e20a` sends `eye LUT + dword[obj+0xc]` and that IS the right quantity. The
defect is that `dword[obj+0xc]` is not maintained for every moving object. Find what drives the
terrain follow for types 0x00/0x01 in the original, and why op-0x54 fires so rarely; then delete the
shim stand-in, which at that point has nothing left to mask.

### Not the frame gate (checked)

The terrain probe in `9e2b` is gated on `(word[DGROUP:0x6cde] & 3) == 0`, and that counter IS
incremented once per object-update walk (`FUN_0000_c0e5`: `DAT_2000_2cde = DAT_2000_2cde + 1`, and
DAT_2000_2cde is the same address as DGROUP:0x6cde). So the gate opens on one walk in four as intended
-- it is not the reason op-0x54 fires so rarely. The remaining candidate is how many objects actually
reach a probing step method: `9e2b` has no DIRECT callers, it is reached only through the c0e5 step
table, and the type->target map sampled on AZER1
(`00->7c1d 01->87df 02->902c 03->97d5 04->bab4 08/0b/0d->b5e7 10->b51f 11->9b11 12->9bc6 13->c0ba
15->9c4f 17->bc0c 1a->bc46 1b->b355`) contains no entry for it at all.

## Where the altitude for types 00/01/02 actually comes from (traced, not guessed)

Types 00/01/02 do NOT terrain-probe, and that is FAITHFUL. Their step methods begin:

```
7c1d: mov 0x1d(%di),%al ; mov %al,0xd(%di)     ; type 00
87df: (identical)                               ; type 01
902c: (identical)                               ; type 02
```

so their ground byte is a straight copy of `byte[di+0x1d]` every frame. The real terrain-follow is
`FUN_1000_adcd` (asm 0x1adcd = 0f69:0xb73d -- `lea 0x4(%di),%di ; lcall e1d1 ; mov %al,0xd(%di)`),
which the port has correctly (patch 222). It has EIGHT call sites in the asm --
`554d 9af6 9c16 b306 b433 b50b bc2c bcf7` -- and **none of them is in the 00/01/02 step chain**. All
eight are present in the port. So "types 00/01/02 never probe the terrain" is the original's design,
not a port defect, and the earlier reading of this item was wrong on that point.

That moves the question to `byte[di+0x1d]`, which carries their altitude. Its only asm writers are:

- `0xf1b8` (`mov %ax,0x1d(%di)`), inside the mission/object LOADER at 0xf1a1 -- it sets [di+0x19] and
  [di+0x1d] from a parsed value;
- `0x1ae32`, inside the guidance routine at 0x1addb, which writes the velocity triple
  [di+0x1d]/[di+0x1f]/[di+0x21] from a192 -- i.e. +0x1d is a UNION field with a different meaning for
  guided objects.

Measured discrepancy against the oracle, same object slot, AZER1:

```
original  type 00 @c05c:  gnd 81 -> 64   (Z 20736 -> 16384)
port      type 00:        gnd 5, constant (Z 1280)
```

So the port's `[di+0x1d]` is 5 where the original's is 81 -- the divergence is already present in the
value the object carries, not in the per-frame copy, which the port performs correctly.

NEXT: compare `byte[di+0x1d]` at SPAWN between the port and the oracle for the same object slot. If it
is already 5 vs 81 at spawn, the defect is in the loader at 0xf1a1 (or whatever feeds it); if it starts
equal and then diverges, the writer at 0x1ae32 / the guidance path is where to look. Do not assume
either -- the two writers mean opposite things.

Also noted while reading 7c1d (a separate, small, real defect, NOT yet patched): asm
`7c26 sar $0x8,%ax` is an ARITHMETIC shift feeding `byte[di+0xa7]`, and the port has
`(char)((uint)*(undefined2 *)(param_4 + 0x38) >> 8)` -- a LOGICAL shift on an unsigned, which differs
for negative headings. The same line appears in the 87df and 902c heads.
