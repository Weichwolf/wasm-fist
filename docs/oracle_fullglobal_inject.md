# Full-global inject — the DECISIVE input-state test for the mission-windshield terrain colour

> ## VERDICT (2026-07-17) — **AE-NEUTRAL, and the 12-iteration "same-renderer-same-globals-different-output"
> paradox is RESOLVED with a concrete mechanism: the frame-matched RAM globals are NOT frame-matched to the
> VRAM. The port's FUN_0000_9200 is faithful (offline sim reproduces the port runtime 99.4 %); injecting the
> COMPLETE value-determining oracle global set + the oracle horizon + the byte-exact oracle colormap tile,
> VERIFIED reaching 9200 byte-exact, leaves the terrain band-match at 0.009 % (baseline 0.376 %) and the
> full-frame index-diff at 33226/64000 (baseline 33145) — i.e. SLIGHTLY WORSE. It is NOT the renderer, NOT the
> tile, NOT the palette, NOT an incomplete injection. The captured globals produce a DEGENERATE walk
> (U_hi≈const 162 across every screen row) that cannot make the oracle's horizontal texture gradient, because
> the captured per-texel U-step (ebp) ≈ −0.012 tile-cols/px ≈ 0. The globals + camera in the RAM snapshot are
> internally self-consistent (the port's own 8120 fed the captured camera angles reproduces the captured
> globals BYTE-EXACT) but belong to a DIFFERENT frame than the displayed VRAM.**
>
> Engine PRISTINE unchanged (`61453e42`/`0051cb56`/`75c6d726`); only `tools/native_main.c` gains the default-OFF
> read-only `FIST_INJECT_ALLGLOBALS` diagnostic seam. `make check` clean; the 25 verify flows are preserved by
> construction (seam default-OFF).

## The test the lead asked for (done rigorously, not a partial inject)
Runtime capture (`docs/oracle_runtime_9200_capture.md`) proved the oracle's running terrain renderer at flat
`0x10009260` is BYTE-IDENTICAL to `re_out/fist_image.bin` @ `0x9200`. 9200's rendered INDEX output is therefore
a PURE FUNCTION of its inputs:

| input | oracle frame-matched value (from `scratch/oracle/mspawn.ram.bin` @ ext phys `0x131000`) |
|---|---|
| `90d4` (U base) | `0xb1c0a498` |
| `90d8` (V base) | `0x39331d90` (hi `0x39` = tile row 57) |
| `esi` (V per-texel step, `param_2`) | `0x007ff621` (= +0.4998 tilerow/px) |
| `ebp` (U per-texel step, `param_1`) | `0xfffcdbd5` (= **−0.0123 tilecol/px ≈ 0**) |
| `90b8`/`90bc` (per-scanline base step) | `0xfff9b7aa` / `0x00ffec42` (= 2·ebp / 2·esi) |
| `90f0`/`90f8` (colheight 288 / scanlines 81) | `0x120` / `0x51` |
| `90ac` (dest advance) | `0x20` |
| horizon table @ `9114`=`0x7568` | `[6,4,3,2,1, 0×65, 1,2,3,4,5,6,7,8,8,9,9]` |
| colormap tile @ `3918` (256×256) | phys `0x175200`, 212 distinct, min 37 max 249 |

`scratch/oracle/mspawn.ram.bin` is the SAME capture as the committed frame-matched samples (its `.vram.bin`+`.pal.bin`
are byte-identical to `oracle_mission_spawn_framematched.vram.bin`/`oracle_mission_spawn_dac.pal.bin`; the derived
`idx` == `oracle_mission_spawn_framematched_idx.bin`). So these globals are from the SAME SIGUSR2 tick as the
reference VRAM.

**`FIST_INJECT_ALLGLOBALS` (new seam, `tools/native_main.c`)** overwrites the ENTIRE value-determining set with
these bytes right before `m_ext_FUN_0000_9200`, BYPASSING 8120, sets `esi`/`ebp` DIRECTLY (not via the
`90c0*9104` derivation), writes the oracle horizon at the port's `9114` pointer, and (`FIST_INJECT_TILE=<64KB>`)
loads the oracle colormap into the buffer `DAT_0000_3918` points at (the exact buffer 9200 samples). `90a8`
(dest, a host fb pointer) and `90ac` are DEST geometry, not value-determining, so the pixels land where the port
already draws them (silhouette matches: port 1.0 % black vs oracle 1.2 %). Verified reaching 9200 byte-exact via
`FIST_R3D_GDUMP2` (`90d4=b1c0a498 90d8=39331d90 90b8=fff9b7aa 90bc=00ffec42 90f0=288 90f8=81 esi=007ff621
ebp=fffcdbd5`).

## Hard numbers (spawn op-0x24 post #1, `/tmp/fist_native`, DEFAULT-build render path + 5598 DAC merge)
Reference = `oracle_mission_spawn_framematched_idx.bin` (raw VRAM indices; DAC-free comparison).

| config | band(rows8-88, ora≥80, n=22055) exact | port band-mean | full-frame idx-diff /64000 | RGB AE vs fm-oracle png |
|---|---|---|---|---|
| **DEFAULT (baseline)** | 83 (**0.376 %**) | 127.3 | 33145 | 33209 |
| **FIST_INJECT_ALLGLOBALS + oracle tile** | 2 (**0.009 %**) | 123.0 | 33226 | 33215 |
| oracle band-mean = **185.3** (distinct 64) | | | | |

The inject is AE-NEUTRAL-to-WORSE. (RGB AE vs the old X11 `ref/mission_azer1_spawn_native320.png`: baseline
33250, inject 33266 — same story.)

## Why it doesn't help — the concrete mechanism (this is the deliverable)
1. **The port's 9200 is faithful.** An offline Python sim of the exact `re_out/fist_ext.c` `FUN_0000_9200` C
   (asm-verified affine: `V+=esi, U+=ebp` per pixel; `V_base+=90b8, U_base−=90bc` per scanline; sample
   `tile[(V>>24)<<8 | U>>24]`), fed the injected globals + oracle tile, reproduces the port's RUNTIME inject
   render at **99.4 %** (window rows5-85 cols16-304, 23185/23328). So there is NO compile/decompile discrepancy
   — the port draws exactly what the affine math dictates.

2. **The captured globals make a DEGENERATE walk.** Instrumenting the per-pixel sample for screen row 20:
   `V_hi` advances ~0.5 tile-row/px (56,57,57,58,…) but **`U_hi` stays constant at 162** across the whole
   288-px scanline (because captured `ebp` = U-per-pixel = −0.0123 tilecol/px). So the port samples a NEAR-
   VERTICAL, near-constant COLUMN of the tile per screen row → flat scanlines (row20 all-104, row40 all-105,
   row60 all-92, mean 123).

3. **The oracle VRAM has a horizontal gradient the captured globals CANNOT produce.** e.g. oracle screen row 20
   cols16-40 = `248,245,245,243,243,243,243,238,238,234,234,224,224,230,…` — a smooth horizontal ramp. That
   requires a substantially non-zero U-per-pixel step. With `ebp≈0` no affine parametrisation of these captured
   globals reaches it.

4. **The captured camera ↔ captured globals are self-consistent — so the VRAM is the odd one out.** The port's
   own `FUN_0000_8120`, fed the captured oracle camera angles (`FIST_ISO_3A=512 FIST_ISO_PITCH=256`, heading
   unchanged), reproduces the captured RAM globals BYTE-EXACT (`90d4/90d8/90b8/90bc/9104/9108` all identical).
   So {camera → 8120 → globals} is one coherent frame-state. Since that coherent state does NOT render the
   captured VRAM (steps 1-3), **the RAM snapshot (camera+globals) is from a DIFFERENT frame than the displayed
   VRAM.**

## Resolution of the 12-iteration paradox
The "frame-matched" capture proved VRAM↔DAC↔PNG self-consistency (all derived from the same `vga.mem`+`vga.dac`
at one SIGUSR2 tick). It did NOT — and cannot, with this capture method — prove **RAM-globals ↔ VRAM** match.
9200's projection globals are recomputed by 8120 every frame and consumed immediately; the framebuffer holds the
LAST-PRESENTED frame while the RAM camera/globals are the LIVE projection (which, on the fast-moving mission
camera, is already a frame ahead/behind). One frame of skew shifts `ebp`/`esi`/base enough that the captured
globals sample a flat tile column instead of the oracle's gradient path. **Every prior "inject the captured
globals" experiment (6+ iterations, all ~0 % band-match) failed for this one reason: there is no frame where
these globals produced that VRAM.**

This CLEANLY reconciles the two prior findings that looked contradictory:
- `oracle_runtime_9200_capture.md`: the oracle runs the byte-identical affine 9200. ✓ (so an affine parametrisation
  that reproduces the VRAM MUST exist — the oracle's own render-time globals).
- `oracle_mission_spawn.md`: no affine reproduces the VRAM. ✓ **but only because the captured/searched globals are
  frame-skewed** — not because the render is a paged-out perspective routine. The "perspective signature" (bottom/top
  h-texture-activity 0.51) is the oracle's affine on ITS (unknown) render-time globals, not evidence of a different
  renderer.

## Verdict for the acceptance metric + fix plan (input-state, honest caveat)
- **It is INPUT-STATE (the projection globals / camera), NOT the renderer, tile, or palette.** Branch-(b)
  elimination is complete: injecting the byte-exact oracle colormap tile does not help (0.009 %); the comparison
  is raw-index so the DAC/palette is irrelevant by construction; the SMC `0x44200` colormap base is exactly the
  buffer injected. All three are eliminated.
- **The genuine port defect** (already flagged, AE-neutral): the mission render-camera TCB `+0x3a`/`+0x3c`
  pitch/roll are stale (`0`/`−256`) because the flight-model camera bridge `a20d/a192/0459` threads only heading
  `+0x38`. Fixing them to the live pitch/roll would make 8120 produce the captured globals (proven) — but that is
  **AE-NEUTRAL for this reference**, because the reference's own globals are not frame-matched to its VRAM.
- **The true blocker is a capture-methodology gap, not a port bug.** To bit-verify mission terrain we need a
  globals↔VRAM frame-matched oracle: dump the projection globals AT 9200's entry (breakpoint eip `0x9260`) and
  the VRAM produced by THAT same 9200 call, before the next 8120 overwrites the globals. Only then does "inject
  the globals → the faithful port 9200 reproduces the VRAM" become a valid decisive test. Until that capture
  exists, do NOT land a camera pitch/roll fix chasing the current reference (it regresses the deterministic spawn
  md5 with no colour benefit), and do NOT re-chase renderer/tile/palette (all faithful/eliminated).

## Repro
```bash
make native
# extract frame-matched oracle globals/tile/horizon from the same-tick RAM:
python3 - <<'PY'
r=open('scratch/oracle/mspawn.ram.bin','rb').read()
open('/tmp/oracle_fm_tile.bin','wb').write(r[0x175200:0x175200+65536])   # colormap @ ext 0x44200
PY
SCR=$(mktemp -d); cp -a armoredfist "$SCR/af"
MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0"
setarch -R env FIST_DATADIR="$SCR/af" FIST_COOP_TICK=1 FIST_TICK_HZ=25000 FIST_MOUSE="$MOUSE" \
  FIST_INJECT_ALLGLOBALS=1 FIST_INJECT_TILE=/tmp/oracle_fm_tile.bin FIST_R3D_GDUMP2=1 \
  FIST_MISSFB=/tmp/inj.ppm FIST_MISSFB_N=1 FIST_MISSFB_FBIDX=1 FIST_MISSFB_FBDUMP=1 /tmp/fist_native
# /tmp/fb_idx.bin = raw 0xA0000 indices; diff band rows8-88 idx>=80 vs
# tools/oracle/samples/oracle_mission_spawn_framematched_idx.bin  -> 0.009% (baseline 0.376%)
```
