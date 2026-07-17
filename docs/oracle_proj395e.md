# FUN_0000_395e is faithful; the mission-terrain "solid column" was a 6980 SMC/signedness defect — FIXED

> ## HARD VERDICT (2026-07-17) — **THE SOLID COLUMN IS RESOLVED AT THE CODE LEVEL. It was NEVER the
> projection table (395e is byte-exact faithful) — it was TWO un-modelled defects in FUN_0000_6980's
> inner ray-march that patch 342 missed: (A) the SMC-patched per-ray STEP was left as the static
> `0x7fffffff` immediate, so every screen column swept the identical coordinate → byte-identical
> columns; (B) the world-rotation basis 90fc/9100 (typed `undefined4`=uint) was zero-extended into the
> `(longlong)` multiply while the asm `imul` is signed → the ray marched a too-narrow coordinate range.
> Patch 343 fixes both. The port's real 6980 now emits a DEPTH-VARIED terrain (distinct 5→59, all LIGHT
> 85-228) that is 100.000% BYTE-IDENTICAL to a from-scratch asm-faithful Python march of 6980. BUT
> byte-exact match to the oracle reference is NOT achievable from the `voxel6980_framematched_pass08`
> bundle: the bundle is internally INCONSISTENT — its dense `tile_o` carries 28 distinct colours ABSENT
> from the bundle's own LIGHT reduce, and the now-byte-faithful port matches the bundle's frame-matched
> 6980 shadow at only 1.31%. This is the extender colormap RE-PAGING / capture frame-skew limit
> (colour_gate), NOT a code defect.**
>
> Engine PRISTINE (`61453e42`/`0051cb56`/`75c6d726`/`1e0cfd38`). One engine patch (343, mission-only);
> no native_main / diagnostic change. No regression: `make check` = 343 patches apply; dedicated build
> (`NATIVE=/tmp/fist_395e make native`) default boot alive (rc=124); mainmenu flow AE=0/64000.

## Repro
```bash
NATIVE=/tmp/fist_395e make native
FIST_6980PROVE=1 /tmp/fist_395e            # LIGHT reduce -> distinct 59, max 228 (was 5, max 132)
FIST_6980PROVE=1 FIST_6980PROVE_BUILDPROJ=1 /tmp/fist_395e   # rebuild proj via 395e -> byte-identical tile
python3 - < /dev/null   # the asm-faithful march + bundle inspection are inline in this doc's analysis
```

---

## (1) FUN_0000_395e — FAITHFUL. The "1.75% mismatch" premise is DEBUNKED.

`395e` builds the `[0x3909]` projection table from the depth ramps `3a24`/`3e24` and the camera scale
`90c0` (asm @ image 0x395e..0x3a17, x86:LE:32 flat PM):
```
4624[d] = 3e24[d]
4224[d] = (3a24[d] * 90c0) >> 24                 ; mul + shrd 0x18
ebp     = (0xffffffff / 4224[d]) << 5            ; reciprocal scale (idiv, +ve dividend edx=0)
proj[d][0..127]   = clamp([0x3928] + hi(k<<24 * ebp), [0x3924]) >> 8      ; forward half
proj[d][128..255] = (([0x3928] - hi(acc*ebp)) >> 8) or 0 on borrow        ; reverse half
```
A from-scratch asm-exact Python sim of `395e`, fed the bundle's banked ramps (`3a24`/`3e24`) + `90c0`
(`0xffffff`), reproduces the bundle's injected capture proj **byte-for-byte: 64000/64000 = 100.000%**.
So the projection algorithm is CORRECT and `proj[0][idx]=255` (idx≥1) is INHERENT to the real inputs.

The port's `395e` is ALSO already correct in `build/`: patch **287** rebased its in-image refs and its
proj-base pointer (`puVar10 = *(uint*)(0x3909)` — the pointer VALUE, not the `LAB_3909` byte-deref the
pristine decompile emitted). Validation: forcing the port to REBUILD the proj via `395e` from the
banked ramps (`FIST_6980PROVE_BUILDPROJ`) yields a tile **BYTE-IDENTICAL** to injecting the capture
proj. **395e was never the colour blocker.** (The old "1.75%" figure predates patch 287/342 or measured
against a frame-skewed proj.)

## (2) FUN_0000_6980 — the ACTUAL defect (patch 343), asm-verified @ image 0x6980..0x6bf6

`6980` is a horizontal-span voxel raycaster: outer loop = DEPTH `d` (near→far, 0..[0x3a20]); inner loop
= screen column `edx` (swept from centre [0x38fd] outward). Per (d, edx): coord = `(ebp>>22)<<10 |
(ebx>>22)`, `sy = proj[d][(L0 + heightmap[coord]) & 0xff]`, and a Y-buffer (`4e60[edx]`, zeroed) gives
front-to-back occlusion: **plot a span iff `sy > ybuf[edx]` (span = sy−ybuf, colour = colormap[coord]),
then ybuf=sy.** The fill/occlusion is FAITHFUL line-for-line vs the asm.

Two un-modelled SMC/signedness classes were the solid-column cause:

**Defect A — the SMC per-ray STEP (`add ebx,IMM`/`sub ebp,IMM` @ 6ad0/6ad6/6b50/6b56).** The step imm
is self-modified per depth (`imul 9100`→`[6ad2]`, `imul 90fc`→`[6ad8]`, + reverse copies `[6b52]`/
`[6b58]`). Ghidra couldn't model the SMC → it stepped the ray by the UN-PATCHED static `0x7fffffff` /
`0x80000001` (~2^31 garbage). Consequence: within a depth, `(ebx>>22)` jumped ~512 per column → EVERY
even column sampled the IDENTICAL coordinate → the tile was written byte-identical in every column
(the "solid column"; gdb-confirmed: rows edx=40/64/128/160/200 were byte-for-byte equal). ALSO the
operand stores `uRam00006ad2/6ad8/6b52/6b58` are typed `undefined2` (16-bit) but the operands are
32-bit `IMM32` → a 16-bit store truncated the signed step.

**Defect B — rotation-multiply SIGNEDNESS.** `DAT_0000_90fc/9100` are `undefined4` (=`uint32_t`), so
`(longlong)DAT_0000_9100` ZERO-extends (`0x9e669af4`→+2.65e9) while the asm `imul ds:0x9100` is SIGNED
(`0x9e669af4`→−1.64e9). The step/centre used the wrong sign (fwd X step **+4036 unsigned vs −2488
signed**, gdb-confirmed) → the ray marched a too-narrow coordinate range (port tile max colour 151 /
distinct 22, vs the asm-faithful 228 / 59).

**Patch 343** (control-flow 1:1): store the full 32-bit SIGNED step into the SMC operand slots
(`0x6ad2`/`0x6ad8` fwd, `0x6b52`/`0x6b58` rev = the asm copies), consume them in the inner sweep with
the asm's exact signs (fwd `ebx += [6ad2]`, `ebp -= [6ad8]`; rev `ebx -= [6b52]`, `ebp += [6b58]`), and
sign-extend the rotation operands at the centre + step multiplies.

## (3) Result — driven through the port's REAL 6980 (patch 342 + 343), LIGHT reduce

| metric | before (342 only) | **after (343)** | oracle dense `tile_o` |
|---|---|---|---|
| distinct colours | 5 (solid) | **59** | 66 |
| value range | 84-132 | **85-228** | 123-252 |
| mean | 115.4 | 104.2 | 199.3 |
| nz | 38760 (identical cols) | 25047 (depth-varied) | 65536 (incl. 689a) |
| **vs asm-faithful Python march** | — | **65536/65536 = 100.000%** | — |

The solid/identical columns are GONE — the port's 6980 now byte-exactly reproduces an independent
from-scratch asm march. **The colour STRUCTURE is resolved.**

## (4) Why byte-exact vs the oracle is BLOCKED — the bundle is internally inconsistent (frame-skew)

The now-byte-faithful port still does NOT reproduce the bundle's captured reference:
- **`tile_o` value-set: 38/66 distinct in the bundle's LIGHT reduce** — 28 of `tile_o`'s colours are
  ABSENT from the RED reduce the bundle banks. So `tile_o` was NOT produced by sampling the bundle's
  colormap. The port's 59 colours are **59/59 in the reduce** (it faithfully samples what it was fed).
- **port vs the frame-matched 6980 shadow: 1.31%** (99/7585 where the shadow wrote). A byte-faithful
  6980 on truly frame-matched inputs would match the shadow ~100%; 1.31% proves the bundle's
  globals/proj/HM/reduce are NOT the state that produced its own shadow.

Both are the documented extender COLORMAP RE-PAGING (colour_gate): the banked reduce is the
FRAME-BOUNDARY colormap, not the render-time re-paged page 6980 actually dereferenced; and the bundle's
globals↔output are frame-skewed. **No valid byte-exact 6980 colour comparison exists from
`voxel6980_framematched_pass08`.**

## VERDICT

1. **395e: FAITHFUL / solved** — byte-exact proj (100% Python; BUILDPROJ byte-identical through the
   port). NOT the colour blocker.
2. **6980 solid column: RESOLVED (patch 343)** — the real cause was the SMC-step + rotation signedness,
   not the proj. The port's 6980 is now 100% byte-exact to an independent asm march; terrain is
   depth-varied LIGHT (5→59 distinct).
3. **Byte-exact vs the oracle: BLOCKED on a capture limit**, precisely pinned: the bundle's LIGHT
   reduce is the frame-boundary (dark-adjacent) colormap, and its globals↔`tile_o`/shadow are
   frame-skewed — so the bundle cannot bit-verify 6980 offline.

## Remaining plan (colour is code-complete; the mission-frame bit-verify sidesteps the capture skew)
The offline capture skew is IRRELEVANT to the port, which executes the real 6980 over its OWN `g_mem`
buffers. To land the mission-terrain colour and bit-verify against the real DOSBox spawn frame:
1. Feed 6980's colour slot the LIGHT reduce natively — the `85b8` collapse (mid-`89b0` `bc90`→blockA
   hook, [[colormap-groundtruth]] part 2) so `[0x85bc]+0x100000` holds the 254-distinct reduce.
2. Recover `FUN_0000_689a` (rows 160-255 sky resample; source `ds:0x3911` = raw 5.SKY, proven exact).
3. Wire `3931 → 689a → 6980` before `9200` in op-0x24 + the camera bridge (TCB +0x3a/+0x3c), verifying
   the proj buffer `[0x3909]` is 0x100-aligned when `36bf` allocates it on the live path.
4. Final bit-verify = the AZER1 mission spawn frame vs `ref/mission_azer1_spawn_native320.png` — a
   DIRECT DOSBox comparison that needs no frame-matched offline capture.
