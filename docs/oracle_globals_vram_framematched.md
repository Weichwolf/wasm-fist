# Globals↔VRAM frame-matched 9200 capture — the FIRST VALID input-state test (methodology VALIDATED)

> ## VERDICT (2026-07-17) — **METHODOLOGY VALIDATED. With a globals↔VRAM FRAME-MATCHED capture (the
> projection globals 9200 READ + the VRAM 9200 WROTE, from the SAME 9200 invocation), the port's faithful
> `FUN_0000_9200` REPRODUCES the oracle terrain at 93.44 % band-exact (vs 0.009 % for the old frame-skewed
> inject). The 14-iteration "same-renderer-same-globals-different-output" paradox is DEFINITIVELY closed as a
> capture frame-skew — and the dominant divergent input is NOT the camera: it is the TILE. The 3918 colormap
> tile 9200 samples at render time (ext-flat 0x44200, mean 200 LIGHT) is a BYTE-DIFFERENT buffer from the
> committed `blockB` @ phys 0x175200 (mean 128 DARK) that every prior iteration injected — 0.1 % equal. The
> `ebp≈0` the prior verdict called "degenerate" is CORRECT and produces the gradient (the gradient is along V,
> not U).** Engine PRISTINE unchanged (`61453e42`/`0051cb56`/`75c6d726`); only the DOSBox oracle instrumentation
> (`FIST_R9200CAP`) + a default-OFF read-only port seam (`FIST_INJECT_CAPFILE`) added. `make check` clean; the
> 25 verify flows preserved by construction (seam default-OFF).

## The capture — self-consistent by construction
The extender per-frame render is `82b8 = { 8120(projection); 9200(render); 82d0() }`. The instrumented DOSBox
(`FIST_R9200CAP`, `tools/oracle/dosbox_vga_terrain_trace.patch` + `tools/oracle/capture_9200_framematched.sh`):
1. On the FIRST 0xA0000 write of a 9200 pass (lin `[0x10009200,0x100092a0)`, the opaque-LOD texel writer)
   SNAPSHOTS the ext-flat globals `0x9000..0x9200` + the ESI/EBP registers (the CONSTANT per-texel u/v step,
   invariant across the whole 9200 call) → frame F's exact render inputs.
2. When 8120 NEXT executes (a RAM write with lin `[0x10008120,0x10008260)` = the projection recompute for
   frame F+1, which by 82b8's call order runs BEFORE 9200-F+1), the VRAM still holds frame F's terrain →
   DUMPS the shadow globals + the current VRAM + DAC + the colormap tile at ext-flat 0x44200.

The globals and the VRAM are thus from ONE 9200 call. Unlike the SIGUSR2 dump this fires SYNCHRONOUSLY in
graphics/pmode (`vga.mode=3` mode-13h chained, `cpu.pmode=1`) — no text-mode skew. Committed capture:
`tools/oracle/samples/oracle_9200_framematched_pass08.{cap,idx.bin,pal.bin,png}` (pass 08 = a settled AZER1
spawn frame; the `.cap` = hdr[magic,passno,esi,ebp,hzptr] + ext[0x9000..0x9200] + horizon[256] + tile[65536]).

## The captured triple (pass 08, settled; passes 02..15 byte-identical)
| | value |
|---|---|
| `esi` (V per-texel step) | `0x007ff621` = +0.4998 tilerow/px |
| `ebp` (U per-texel step) | `0xfffcdbd5` = **−0.01227 tilecol/px ≈ 0** |
| derived `esi=(90c0·9104)>>32`,`ebp=(90c0·9108)>>32` | == the ESI/EBP registers, BYTE-EXACT (self-consistent) |
| `90d4` (U base) | `0xafc0cc10` |
| `90d8` (V base) | `0x39268ce8` (hi `0x39` = tile row 57) |
| `90b8`/`90bc` (per-column base step) | `0xfff9b7aa` / `0x00ffec42` (= 2·ebp / 2·esi) |
| `90f0`/`90f8` (colheight/scanlines) | 288 / 81 |
| horizon `9114`→`0x7568` | `[6,4,3,2,1, 0×…]` |
| **tile @ ext-flat 0x44200** | **68 distinct, min 116 max 252, mean 200.2 (LIGHT)** |
| VRAM band(rows8-88, idx≥80) | 21854 px, mean 183.1, distinct 95 |

## Non-degeneracy — PROVEN, and the prior "degenerate ebp" premise is REFUTED
The task worried `ebp≈0` means a flat scanline. It does NOT. Along screen row 20, `ebp≈0` keeps the tile
COLUMN constant (`U>>24 = 155` for all 288 px) while `esi≈0.5` walks the tile ROW `56→80`. So 9200 samples a
VERTICAL SLICE of the tile — column 155, rows 56..80 — and the VRAM "horizontal gradient" (row 20:
`248,243,240,245,…,220,…`) IS that tile column's values. **The gradient is produced by the V-walk (esi), not
the U-walk (ebp)** — so `ebp≈0` is exactly correct and non-degenerate. The prior verdict's "U_hi const 162 →
flat scanlines" was wrong because it assumed tile column 162 is constant; the real (light) tile's columns are
gradients.

## THE VALIDATION (port faithful 9200 vs the SAME 9200 call's VRAM)
Port raw 0xA0000 index buffer (`FIST_INJECT_CAPFILE` overwrites the value-determining globals + esi/ebp DIRECT
+ horizon + the 3918 tile from the `.cap`, then `m_ext_FUN_0000_9200`; `FIST_MISSFB_FBIDX/FBDUMP`), spawn
op-0x24 post #1, `setarch -R` deterministic. Comparison vs the pass's own de-interleaved VRAM idx:

| config | terrain band(rows8-88, idx≥80) exact | port band-mean | oracle band-mean | terrain rows8-88 exact | full-frame |
|---|---|---|---|---|---|
| **pass08 globals + pass08 tile** | **20421/21854 = 93.44 %** | 184.9 | 183.1 | 87.06 % | 81.30 % |
| pass02 globals + pass02 tile | 93.26 % | 185.0 | 183.2 | 86.91 % | 81.24 % |
| pass08 globals, **port's OWN tile (no inject)** | **0.02 %** | 128.3 (DARK) | 183.1 | 7.31 % | 48.08 % |
| pass00 globals + tile (transitional frame 1) | 3.30 % | 185.0 | 164.7 | 10.11 % | 51.85 % |
| (prior `FIST_INJECT_ALLGLOBALS`, frame-skewed) | 0.009 % | 123.0 | 185.3 | — | — |

- **93.44 % band-exact, mean-matched (184.9 vs 183.1), best spatial shift = dx=dy=0** (not a misalignment).
  Row 20 is pixel-identical port-vs-oracle. This is a DECISIVE reproduction — the texture-lookup match
  surface is pathological (a 1-LSB address error scrambles the index; the prior optimiser recovered even
  synthetic affines only ~10-13 %), so 93 % means the port walks the SAME path with only occasional 1-LSB
  divergences (the 6.56 % residual: 809/1433 diffs are |Δ|≥10 discrete texel jumps, scattered across mid
  columns 100-166, not edges — sub-pixel/horizon-boundary texel-cell flips, NOT a systematic error).
- **DECISIVE tile result:** WITHOUT the oracle tile (port's own 3918 buffer) the band-match collapses to
  0.02 % and the mean goes DARK (128) — so the port's 3918 tile content is the DOMINANT colour defect.

## The mechanism the paradox reduces to (this is the deliverable)
1. **The port's 9200 is faithful** (byte-identical to the oracle runtime, `oracle_runtime_9200_capture.md`) —
   confirmed end-to-end here: fed the frame-matched inputs it reproduces the frame-matched VRAM at 93 %.
2. **Every prior "inject the captured globals" iteration failed at ~0 % because the captured tile AND the
   90d4/90d8 base were NOT frame-matched to the VRAM:**
   - **Tile (dominant):** the buffer 9200 SMC-samples is ext-flat `0x44200`. At RENDER time the extender's PM
     paging maps ext-flat `0x44200` to a physical page holding a LIGHT tile (mean 200, 68 distinct). The
     prior investigations dumped phys `0x175200` (assumed == ext-flat 0x44200) and got the DARK `blockB`
     (mean 128, 212 distinct) — a DIFFERENT buffer (0.1 % byte-equal). `colormap-groundtruth` explicitly
     warned the fixed-phys watch is confounded by the flat→phys remap; that confound is exactly what bit the
     6-iteration tile analysis. **The oracle's sampled tile is light; the port (and blockB) are dark.**
   - **Base (secondary):** the old hardcoded `FIST_INJECT_ALLGLOBALS` base `90d4=b1c0a498 90d8=39331d90`
     differs from the frame-matched `afc0cc10 / 39268ce8` in the low bytes (sub-tile U/V offset) — same
     esi/ebp/high-bytes, but a skewed base shifts WHICH texels are sampled and scrambles the band-exact match.
3. **The camera↔globals are self-consistent** (derived esi/ebp == register esi/ebp byte-exact), so the RAM
   snapshot is one coherent frame-state. The frame-matched capture pairs it with ITS OWN VRAM → 93 %.

## The two real port defects (honest, ranked by impact)
1. **DOMINANT — the 3918 colormap tile is the WRONG (dark) buffer.** The port populates 3918 with a
   blockB-class dark tile (mean 128); the oracle's 3918 at render time holds a LIGHT tile (mean 200, 68
   distinct). No-tile → 0.02 %, +oracle-tile → 93.44 %. This overturns `colormap-groundtruth` part 4's "tile
   byte-exact" (it compared the DARK blockB @ 0x175200, not the light render-time buffer @ ext-flat 0x44200).
   **Next step (the real frontier): instrument the oracle to capture the LIGHT tile's builder/source — who
   writes ext-flat 0x44200 with the mean-200 content, at what point (post-map-load? a LOD/shade pass?)** —
   analogous to the block-A last-writer hunt. The committed `.cap` tile IS that ground truth (banked).
2. **SECONDARY (positional) — the mission render-camera TCB `+0x3a`/`+0x3c` pitch/roll are stale.** The port's
   live 8120 emits `90d8=2dc548e0` (V-row 45) vs the oracle `39268ce8` (row 57) because the flight-model
   rotated-camera bridge `a20d`/`a192`/`0459` threads only heading `+0x38`, leaving `+0x3a=0 +0x3c=-256` (vs
   oracle 512/256). 8120 reads `+0x3a`/`+0x3c` → wrong V-base. This shifts WHICH texels (positional exactness),
   not brightness. The bridge is extender-role (paged-out PM), **NOT a clean base-loss → NOT landed** (per
   doctrine + the task's discipline; a camera-only fix regresses the deterministic spawn md5 with no colour
   benefit while the tile is still dark).

Both must be right for the port to bit-reproduce the terrain; the methodology (inject both → 93 %) proves it.

## Repro
```bash
make kernel-image && make native
DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/capture_9200_framematched.sh   # -> scratch/oracle/r92.passNN.*
# validation: inject a settled pass into the port, dump raw 0xA0000, diff vs the pass's own VRAM idx
SCR=$(mktemp -d); cp -a armoredfist "$SCR/af"
MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0"
setarch -R env FIST_DATADIR="$SCR/af" FIST_COOP_TICK=1 FIST_TICK_HZ=25000 FIST_MOUSE="$MOUSE" \
  FIST_INJECT_CAPFILE=tools/oracle/samples/oracle_9200_framematched_pass08.cap FIST_R3D_GDUMP2=1 \
  FIST_MISSFB_FBIDX=1 FIST_MISSFB_FBDUMP=1 /tmp/fist_native   # -> /tmp/fb_idx.bin
# band-diff /tmp/fb_idx.bin vs oracle_9200_framematched_pass08.idx.bin, rows8-88 idx>=80 -> 93.44%
# (add FIST_INJECT_CAP_NOTILE=1 to prove the tile is the dominant lever -> 0.02%)
```
