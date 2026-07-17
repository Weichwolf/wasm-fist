# FUN_0000_6980 — the voxel-column overlay: algorithm PINNED, geometry REPRODUCED, colour BLOCKED

> ## VERDICT (2026-07-17) — **The `FUN_0000_6980` NovaLogic Voxel-Space column raycaster is fully
> disassembled and its algorithm PINNED (rotation recompute reproduces BYTE-EXACT; the fill produces a
> coherent voxel terrain whose silhouette lands exactly on the real terrain region — where 6980 writes,
> the CR3 render-time tile is LIGHT mean 165–190, 93–97 % idx≥140). BUT the whole-tile reproduction does
> **NOT** advance past the 55.55 % 689a baseline, and adding the 6980 overlay REGRESSES on colour.
> Two hard blockers, both real:**
> 1. **The colormap `6980` indexes — `[0x85bc+0x100000]` — is DARK (105 distinct, max 104) in every
>    oracle dump and CANNOT produce the LIGHT terrain (140–252).** The decoded `C32.KLC` is itself 92 %
>    dark (mean 53, only 8 % ≥140), yet the rendered terrain is 93 % light. A LIGHT reduce-colormap DOES
>    exist at `[0x85b8]` (= `[0x85bc]+0x400000`, phys 0x376e60, mean 141, max 228) — but 6980's asm does
>    **not** index it. This is the documented **85b8 reduce-colormap collapse** (see
>    [[colormap-groundtruth]]), now re-confirmed from the 6980 side.
> 2. **The full RAM dumps are FRAME-SKEWED** — a dump's ext globals (`90d4/90d8` camera) are a different
>    frame than its own CR3 render-time tile at phys 0xb78200, so no full dump gives a self-consistent
>    (camera ↔ map ↔ tile) triple for a byte-exact colour validation. The only frame-matched ground truth
>    is `oracle_9200_framematched_pass08.cap` (globals + tile, **no map/colormap/proj**).
>
> Engine PRISTINE (`61453e42` / `0051cb56` / `75c6d726`). Read-mostly offline validation: only
> `tools/oracle/sim_voxel6980.py` + one banked sample were added. Repro:
> `python3 tools/oracle/sim_voxel6980.py .`

**Method:** static disassembly of `FUN_0000_6980`/`FUN_0000_395e` @ ext 0x6980/0x395e in
`re_out/fist_image.bin`, cross-checked against the port's decompiled `re_out/fist_ext.c`; driven with a
first-principles Python re-implementation over CR3-resolved render-time buffers extracted from the banked
full-RAM oracle dumps (`scratch/oracle/{lt2.pass00,mspawn,aa10}.ram.bin`).

---

## 1. The 6980 + 395e algorithm — PINNED (asm-exact)

`FUN_0000_3931`: `85d0(camera); if([0x395d]==0){ (*[0x3958])()=689a; 6980(); }` — so **689a fills the whole
tile, then 6980 overlays terrain rows 0..159**. Tile is column-major at ext-flat `[0x3918]=0x44200`,
`tile[col*256+row]`.

### 6980 (image 0x6980..0x6b3d) — a classic front-to-back Voxel-Space raycaster
```
if 90c0 != 90c4:  90c4=90c0;  395e()                         # rebuild depth tables + projection table
# rotation basis from the heading 90e0 and the static sin/cos matrix at [0x9450]/[0x9650]:
idx = (90e0>>23)&0x1ff ;  edi=(90e0<<9)>>1 ;  esi=(~(90e0<<9))>>1
90fc = 2*( imulhi(mat[0x9450+idx*4], esi) + imulhi(mat[0x9454+idx*4], edi) )
9100 = 2*( imulhi(mat[0x9650+idx*4], esi) + imulhi(mat[0x9654+idx*4], edi) )
# SMC shade base (per-frame pitch):  L0 = -(90dc>>25)&0xff
zero 4e60[0..38f1)                                            # per-column running screen-height (y-buffer)
4a60[k]  = 3918 + k*38ed  for k in 0..38f1                    # per-column tile write pointer (col base)
for d in 0 .. [0x3a20)-1:                                     # DEPTH march, near(d=0) -> far
    ebx = 90d4 ; ebp = 90d8                                   # camera U,V
    ebp -= 2*imulhi(4624[d]>>1, 9100) ; ebx -= 2*imulhi(4624[d]>>1, 90fc)   # ray start at distance d
    stepU = imulhi(4224[d], 9100) ; stepV = imulhi(4224[d], 90fc)           # per-column lateral step
    # loop 1: tile col edx = 38fd .. up to 3905 ; loop 2: edx = 38fd .. down to 3901 (element edx-1)
    for each col (applying +/- stepU to ebx, -/+ stepV to ebp):
        coord = ((ebp>>22)&0x3ff)<<10 | ((ebx>>22)&0x3ff)    # 1024x1024 map cell (shld ecx,ebp/ebx,10)
        h  = heightmap[ [0x85bc] + coord ]
        sy = projtbl[ d*0x100 + ((L0 + h)&0xff) ]            # SMC: eax=[0x3909]+d*0x100, al=(L0+h); mov al,[eax]
        if sy > 4e60[col]:                                    # terrain rises above the running silhouette
            span  = sy - 4e60[col] ;  4e60[col] = sy
            color = colormap[ [0x85bc] + 0x100000 + coord ]  # <-- THE COLOUR SOURCE (raw C32, +1MB)
            rep stos color into 4a60[col] for `span` bytes ;  4a60[col] += span
    inc word[6ade]  (# advance the projection-table base by 0x100 per depth)
    copy 4e60[] into [0x390d] + d*0x100                       # per-depth y-buffer save (bookkeeping)
```
`imulhi(a,b) = (int32 a * int32 b) >> 32` (`imul r/m32`, take EDX). `4224[d] = (3a24[d]*90c0)>>24`,
`4624[d] = 3e24[d]` (both from 395e). **Inputs:** heightmap+colormap `[0x85bc]` (2 MB, HM low, CM +1 MB);
projection table `[0x3909]` (0x100 B/depth); depth ramps `3a24[]`/`3e24[]`; camera `90d4/90d8/90e0/90dc`;
scale `90c0`; matrix `0x9450`/`0x9650`; scalars `38ed=38f1=256, 38fd=128, 3901/3905` col bounds, `3a20` depth
count.

### 395e (image 0x395e..0x3a17) — rebuild `4224`/`4624` + the projection table `[0x3909]`
```
3924=(38f1-1)*0x100 ; 3928=(38f1-1)*0x80+0x100
for d in 0..3a20-1:
    4624[d] = 3e24[d]
    v = 4224[d] = (3a24[d]*90c0)>>24 ;  step = (0xffffffff / v) << 5     # signed idiv edx=0:eax=0xffffffff
    for j in 0..127:  i3 = 3928 + ((j*0x1000000)*step>>32) ;  proj[++] = min(i3,3924)>>8   # ascending
    for j in 0..127:  u = ((0x80000000 - j*0x1000000)*step>>32) ; proj[++] = 0 if 3928<u else (3928-u)>>8
```

---

## 2. Reproduction result — 55.55 % → **still 55.55 %**; the 6980 overlay REGRESSES

| stage | whole-tile vs pass08 | note |
|---|---|---|
| 689a only (baseline) | **55.55 %** | rows 160-255 98-100 %; rows 0-159 25-43 % (`sim_lighttile_689a.py`) |
| + 6980 overlay | **NO improvement / regresses** | 6980 GEOMETRY correct, COLOUR wrong (below) |

**GEOMETRY reproduced (validated):**
- The rotation recompute `90fc/9100` reproduces the dump's stored values **byte-exact** (`fb148b1e`/`80187f4e`
  on aa10) — the matrix read + `imulhi` + shift chain is exactly right.
- 6980 fills a coherent voxel terrain (38 % of the tile on lt2.pass00, 74 % on mspawn — varies with camera),
  190/189 columns, top rows ~129-132. **Where 6980 writes, the CR3 render-time tile is LIGHT** (lt2.pass00:
  mean 165.1, 93 % idx≥140; mspawn: mean 190.4, 97 % idx≥140) — i.e. the fill silhouette lands on the real
  terrain region.

**COLOUR wrong (blocked):** my 6980's colours there are DARK (mean 23.6 with the raw colormap; 99.8 with the
85b8 reduce) → exact-value match **0.00–1.06 %**. Two compounding causes:
1. **Colormap.** `[0x85bc+0x100000]` (CR3 phys 0x1f6e60) is DARK: 105 distinct, max **104** — physically
   cannot emit the tile's 140-252 terrain. A LIGHT reduce colormap exists at `[0x85b8]=[0x85bc]+0x400000`
   (CR3 phys 0x376e60, mean 141, max 228, 55 % light) but 6980's asm does not index it. `C32.KLC` decoded is
   itself 92 % dark. **No buffer anywhere in ext<4 MB, at ANY base, holds the tile's near value 176 at the
   sampled coord.** → the light terrain colour is produced by a reduce/indirection that is NOT the raw
   `[0x85bc+0x100000]` read — the exact **85b8 collapse** the colormap frontier documents.
2. **Frame-skew.** A full dump's ext globals (camera) are a different frame than its own CR3 render-time tile
   (proved: the dump's identity tile @0x175200 AND CR3 tile @0xb78200 both mismatch a 6980 run on the dump's
   own globals). So even with the correct (85b8) colormap the sampled coord doesn't align with the tile —
   1.06 % is coord-misaligned, not a real match.

---

## 3. Port faithfulness — the port's 6980 is **NOT faithful/functional as-is** (three defects)

1. **SMC not modeled (would SIGSEGV).** The projection lookup `mov al,[eax]` is self-modifying: at runtime
   `_uRam00006add` is patched to `([0x3909] & 0xffffff00) | L0`, incremented `+0x100` per depth. Ghidra
   emitted the **static** form: `*(byte*)CONCAT31(0x7fffff, height-1)` (the un-patched `0x7fffffff` immediate)
   AND `_uRam00006add = CONCAT31((uint)LAB_0000_3909>>8, …)` uses the **address** of label `3909` instead of
   the **value** at `[0x3909]`. Run as-is the port dereferences a wild host address `0x7fffff00|…` → crash /
   garbage. (`re_out/fist_ext.c` ~10980, 11005.)
2. **Depth ramps `3a24[]`/`3e24[]` are all-1 in the image with NO writer.** Full-image + full-C scan: the two
   tables at ext 0x3a24/0x3e24 are `0x00000001×200` in `re_out/fist_image.bin`, referenced ONLY by the two
   reads in 395e, never written anywhere in the engine, drivers, or ext image. At oracle render time they are
   a proper ramp (`0x197d..0xee0ed3` / `0xeef4d..0x8b7cafda`) → **filled by paged extender boot code absent
   from `fist_image.bin`.** The port builds ext memory from the image and runs no filler ⇒ the port's 6980
   depth march is degenerate (constant step) ⇒ wrong terrain.
3. **395e is not reproducible from the image alone.** A faithful Python re-implementation of the asm 395e
   (matching the port's decompiled 395e) reproduces the map-static live projection table `[0x3909]` at only
   **1.75 %** (best over all 38f1). So either the port's 395e Ghidra decompile is subtly wrong OR the live
   proj is built by paged code too. The live proj is map-static (byte-identical across aa10/lt2/mspawn:
   `[116,115,115,117,119,121,125,129,…]`) — it must be captured or the 395e model must be re-derived.

**Colormap faithfulness (the explicit question):** the colormap 6980 indexes is the dark raw
`[0x85bc+0x100000]`; the light terrain comes from a reduce (`85b8` family) the port has documented as
COLLAPSED (89 vs 254 distinct — [[colormap-groundtruth]]). So the port's 6980 colormap input is **NOT
faithful**; it is the same 85b8 defect, now reached from the 6980 side.

---

## 4. What remains for the integration iteration

The tile-fill = `689a` (rows 160-255, PINNED) + `6980` (rows 0-159, algorithm PINNED, geometry reproduced).
For a byte-exact 6980 the integration iteration must resolve, **in order**:
1. **A frame-matched full-state capture** (extend the `FIST_R9200CAP` DOSBox to also dump, at the pass08
   9200 moment, the CR3-resolved `[0x85bc]` 2 MB map, `[0x3909]` proj, `3a24/3e24`, and the camera) — the
   only way to validate 6980 byte-exact, since the existing full RAM dumps are frame-skewed.
2. **The depth-ramp filler** (`3a24/3e24`) — locate the paged boot writer or bank the map-static ramp.
3. **The projection-table builder** — re-derive 395e (the port's decompile mis-builds it) or capture `[0x3909]`.
4. **The SMC-modeling patches** for 6980 (thread `[0x3909]` value + the runtime-patched `_uRam00006add`).
5. **The colormap** — resolve the 85b8 reduce so the terrain colour is light (the dominant residual; the
   raw `[0x85bc+0x100000]` is dark and cannot render the terrain).

**What is faithful in the port already:** the rotation matrix `[0x9450]/[0x9650]` (== the image, byte-exact);
the 6980 control flow / y-buffer / span-plot structure; the 5.SKY source for 689a (per [[colormap-groundtruth]]
part 8). **What must be reconstructed:** items 2-5 above.

## Repro
```bash
python3 tools/oracle/sim_voxel6980.py .        # geometry-correct / colour-blocked, self-contained
# banked CR3-resolved input bundle: tools/oracle/samples/voxel6980_inputs_lt2pass00.bin.gz
# (built from scratch/oracle/lt2.pass00.ram.bin via the page-directory walk PD@0xe000: ext-flat X ->
#  linear 0x10000000+X -> guest-phys; e.g. tile 0x44200 -> 0xb78200, colormap 0x174e60 -> 0x1f6e60.)
```
