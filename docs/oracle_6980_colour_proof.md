# PROVE the mission-terrain colour fix in the port's REAL FUN_0000_6980 — HARD result

> ## VERDICT (2026-07-17) — **The port's REAL 6980 now RUNS to completion (patch 342, was a
> guaranteed SIGSEGV) and the COLOUR fix is PROVEN DIRECTIONALLY through it: feeding 6980's colour
> slot the LIGHT reduce makes the real code emit LIGHT colours (max 132, value-set 5/5 in the reduce),
> feeding the dark C32 emits DARK (max 61, 6/6 in C32).  The geometry ENTRY is BYTE-EXACT — the port
> draws the exact 190-column silhouette and all 190 near top-rows match the oracle shadow within ±1.
> BUT the tile is NOT byte-exact (2.50 % over the shadow footprint / 40.66 % whole-tile): the port
> over-draws each column 5.11× into the deep rows.  Residual PINPOINTED = deep-ray HEIGHTMAP fidelity
> — the frame-matched capture's FLAT 1 MB heightmap/reduce do NOT reproduce the render-time
> NON-CONTIGUOUS PM paging, so far-ray coord→sample is skewed (near-ray, correctly paged, is exact —
> hence 190/190 top-rows match).  Confirmed by three independent marches (port 5.11×, an independent
> Python march 4.8×, the doc's own `sim_voxel6980` 3.3×) ALL over-drawing on the same flat buffers.
> This is an INPUT-fidelity limit of the capture, NOT a 6980-code / patch-342 defect.**
>
> Engine PRISTINE (`61453e42`/`0051cb56`/`75c6d726`/`1e0cfd38`).  One engine patch (342, mission-only)
> + one default-OFF standalone diagnostic (`FIST_6980PROVE`, `tools/native_main.c`).  No regression:
> `make check` = 342 patches apply; dedicated build (`NATIVE=/tmp/fist_prove make native`) default boot
> alive (rc=124 timeout, not SEGV); mainmenu flow AE=0/64000.

## Repro
```bash
NATIVE=/tmp/fist_prove make native
FIST_6980PROVE=1    /tmp/fist_prove   # LIGHT reduce (the fix)   -> light output through the real 6980
FIST_6980PROVE=dark /tmp/fist_prove   # dark C32 (baseline)      -> dark  output
```

---

## What the diagnostic does (integration-readiness proof, standalone — no engine boot)

`FIST_6980PROVE` runs BEFORE `app_entry()` and `_exit()`s, so it cannot touch the 25 verify flows.
It feeds the port's real `m_ext_FUN_0000_6980` the FRAME-MATCHED render-time inputs banked in
`tools/oracle/samples/voxel6980_framematched_pass08.bin.gz` (the `FIST_R6980CAP` capture,
docs/oracle_colour_gate.md) and byte-compares its tile output to the banked oracle 6980 **shadow**
(6980's actual render-time stores = byte-exact ground truth).  The three input fixes exercised:

| # | fix | how, in the proof | how, in the real port |
|---|-----|-------------------|-----------------------|
| 1 | colormap → LIGHT reduce | inject the captured LIGHT reduce at `[0x85bc]+0x100000` | the part-2 `bdc4→blockA` fix ([[colormap-groundtruth]] part 2: port bdc4 fed block A → 256 distinct) + feed 6980's slot |
| 2 | ramps-bank `3a24/3e24/4224/4624` | inject the captured ramps at `ext_base+off` | boot-seed the paged-boot ramp values (all-1 in `fist_image.bin`) |
| 3 | proj-SMC-model | **patch 342** (asm-verified) | **patch 342** |

`90c0==90c4` in the capture, so 6980 skips `395e` and consumes `4224/4624` directly — the faithful
render-time path.  The proj table is placed 0x100-aligned (the SMC masks the operand low byte).

## Patch 342 — proj-SMC-model + un-based in-image data-ref rebase (FUN_0000_6980 only)

Asm-verified vs `re_out/fist_image.bin` @ ext 0x6980..0x6bf6 (x86:LE:32 flat PM).  The self-modifying
projection lookup:
```
6a04: mov ds:0x6add,eax    ; eax = ds:0x3909 (proj base ptr), al := -(90dc>>25) = L0  -> patches the
6adc: mov eax,0x7fffffff   ;   `mov eax,IMM32` operand to (projptr & 0xffffff00) | L0 (inc'd +0x100/depth)
6aeb: add al,[esi+ecx]     ; al := (L0 + heightmap[coord]) & 0xff
6aee: mov al,[eax]         ; sy = proj[ (projptr & 0xffffff00) + d*0x100 + ((L0+h)&0xff) ]
```
Ghidra could not model the SMC → emitted `CONCAT31((uint)LAB_0000_3909>>8, L0)` (a BYTE deref of
[0x3909], not the 32-bit proj-pointer VALUE) for the init and `*(byte*)CONCAT31(0x7fffff, hm[coord]-1)`
(the un-patched 0x7fffffff immediate → wild deref → SIGSEGV) for the lookup.  Patch 342:
* SMC init reads the 32-bit proj-pointer VALUE at `[0x3909]`.
* both colour-column lookups: `sy = *(byte*)((op & 0xffffff00) | (byte)((byte)op + hm[coord]))`,
  `op = _uRam00006add` (fwd) / `_uRam00006b5d` (rev); the +0x100/depth advance was already modelled.
* rebases the three bare in-image reads Ghidra left as raw host literals (0x9454 sin/cos matrix,
  0x4224/0x4624 ramps) to `g_mem + fist_ext_base + off`.

The per-depth SMC advance (`inc word[6ade]/[6b5e]`) was already present as `sRam*+1`.  Control flow 1:1.

## HARD numbers

```
recomputed 90fc=ad301958 9100=9e669af4  == capture ad301958/9e669af4   (rotation basis BYTE-EXACT)

FIST_6980PROVE=1 (LIGHT reduce):
  tile-vs-shadow byte-exact 26645/65536 (40.66%) | tile_nz=38760 shadow_nz=7585 over=31496 under=321
  over-shadow-footprint     190/7585  ( 2.50%) exact where 6980 wrote
  PORT tile value-set: distinct=5 mean=115.4 min=84 max=132
    contained in LIGHT-reduce(max228): 5/5 | dark-C32(max104): 4/5 | oracle-shadow: 1/5
  => the real 6980 emitted LIGHT colours (max 132 > dark-C32 max 104)

FIST_6980PROVE=dark (baseline):
  PORT tile value-set: distinct=6 mean=49.5 min=23 max=61
    contained in LIGHT-reduce(max228): 0/6 | dark-C32(max104): 6/6 | oracle-shadow: 0/6
  => the real 6980 emitted DARK colours

Geometry silhouette (both modes identical — colour does not affect which cells draw):
  shadow: nz=7585  cols[33..222] rows[0..161]   mean 39.9 nz/col
  port:   nz=38760 cols[33..222] rows[0..254]   mean 204.0 nz/col   (over-draw 5.11×)
  shared cols = 190/190 ; cols with matching near top-row (±1) = 190/190
```

**Reading it:** the COLOUR slot decisively controls light-vs-dark through the real 6980 (fix #1 proven).
The geometry ENTRY is byte-exact (identical 190-column silhouette + 190/190 near top-rows) — proving the
rotation, ray-step, proj lookup and SMC model (patch 342) are all faithful.  The residual is the per-column
depth over-march (5.11×) into deep rows: at deeper depths the port's `sy=proj[d][(L0+h)&0xff]` keeps rising
where the oracle's plateaus, because `h=heightmap[coord]` differs for far-ray coords.  Since proj, ramps,
camera and the near heightmap are all byte-exact (near top-rows match 190/190), the ONLY differing input is
the deep-ray heightmap sample → the flat capture heightmap does not reproduce render-time paging.

## Honest scope / what this corrects

- **CORRECTS the overstated gate claim** "the port's FLAT g_mem 6980 reads correctly natively with the
  right buffer content" (docs/oracle_colour_gate.md, [[colormap-groundtruth]] part-8 update).  The port's
  6980 CODE is faithful, but the *captured flat buffers are not "the right buffer content"* for byte-exact:
  they are frame-boundary CR3-walk dumps that do not reproduce the render-time NON-CONTIGUOUS paging, so
  far-ray coord→sample is skewed.  Byte-exact 6980 is therefore NOT achievable from this capture.
- The real byte-exact path needs the port's OWN map-load pipeline to build the flat heightmap + reduce in
  `g_mem` with the coord layout 6980 addresses (the KLC heightmap load + the part-2 `bdc4→blockA` 254/256-
  distinct reduce), then run the real 6980 over them — the NEXT iteration.

## What is landed / not landed this iteration

- **LANDED:** patch 342 (proj-SMC-model + rebase) — asm-verified, mission-only, `make check` clean, no
  regression (mainmenu AE=0, md5s pristine).  The default-OFF `FIST_6980PROVE` diagnostic.
- **NOT landed (next iteration):** the op-0x24 wiring `3931→689a→6980` before `9200`; `FUN_0000_689a`
  recovery (rows 160-255); the bdc4→blockA reduce build into 6980's colour slot; the KLC heightmap load;
  the camera bridge (TCB +0x3a/+0x3c).  Verifying byte-exact needs those (paged buffers, not the flat capture).
