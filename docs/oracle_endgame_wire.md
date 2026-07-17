# Endgame: the native mission-terrain TILE-FILL pipeline is WIRED — 689a + 6980 build the windshield

> ## HARD RESULT (2026-07-17) — **The per-frame terrain TILE-FILL (`FUN_0000_689a` sky-resample +
> `FUN_0000_6980` voxel raycaster) is now WIRED into the op-0x24 windshield render (behind the gated
> shim seam `FIST_TILEFILL`), and it produces the STRUCTURALLY + CHROMATICALLY CORRECT windshield: a
> cloudy sky, a tan-desert terrain, and a horizon at the right height — replacing the baseline's
> scrambled brown noise. With the camera-orientation aligned to the oracle the sky band is 42% EXACT and
> the whole-frame AE drops 33253→27325/64000. On the port's own (stale) live camera the colour/structure
> are already correct but the horizon is mis-placed so exact-pixel AE only moves 33253→32973. The
> residual to bit-exact is (1) the paged-out camera pitch/roll bridge and (2) exact-pixel terrain texture
> — NOT the colour, NOT the reduce, NOT the wiring.**
>
> Engine PRISTINE (`61453e42`/`0051cb56`/`75c6d726`/`1e0cfd38`). NO engine patch this iteration — all
> changes are the shim (`tools/native_main.c`) + one banked constant file. No regression: `make check`
> clean; dedicated build (`NATIVE=/tmp/fist_eg make native`) default boot alive (rc=124); mainmenu flow
> AE=0/64000; the tile-fill is env-gated (op-0x24 mission-only) so the 25 menu/intro/settings flows are
> byte-untouched.

## What is WIRED (shim `FIST_TILEFILL`, `tools/native_main.c`)

The port's op-0x24 render was `8deb → 85d0 → 8120 → 9200` — it NEVER ran the per-frame tile-fill, so 9200
affine-sampled a stale map-load tile (scrambled brown). Now, between `85d0` and `8120`:

| piece | how | faithful? |
|---|---|---|
| **`FUN_0000_689a`** (sky rows) | reconstructed `fist_ext_689a()` from the exact pinned asm formula (`tools/oracle/sim_lighttile_689a.py`); fills the whole 256×256 tile from the 128KB decoded `5.SKY` source `ds:0x3911` | pinned formula, ~100% on sky rows |
| **`FUN_0000_6980`** (terrain rows 0-159) | the decompiled `m_ext_FUN_0000_6980` (patches 342+343) — overlays the voxel terrain | asm-faithful (byte-exact to an independent march) |
| **`ds:0x3911` source build** | force `[0x395c]=1` at op-0x18 so `89b0`'s tail decodes `5.SKY` (byte-exact, `docs/oracle_tilesource_builder.md`) | real engine path |
| **ramps `3a24`/`3e24`** | banked (`tools/oracle/samples/voxel6980_ramps.bin`) + seeded; `6980→395e` recomputes `4224/4624` from native `90c0` | paged-out engine constants (no writer in `fist_image.bin`) |
| **`90b0`/`90b4`** | seeded (`0x3d00`/`0x20000`) | paged-out projection constants (no writer) |
| **contiguous HM+CM** | build `HM[0..1MB] + colormap[0..1MB]@+0x100000` so `6980`'s `[0x85bc]+coord+0x100000` colour read lands in the colormap, not the wrong Route-1 separate alloc | required by the flat memory model |
| **DAC palette** | `ext+0x5260` (the asm-9f65 SORTED-DISPLAY palette) instead of `5598` — the faithful terrain DAC; only helps once the tile is correct | asm-verified faithful |

## Measured spawn-frame AE vs `ref/mission_azer1_spawn_native320.png`

Drive = `BATTLES→OK→ACCEPT` (`FIST_MOUSE`), dump op-0x24 post #1 (`FIST_MISSFB`), `compare` per-region:

| config | WHOLE AE | terrain rows 8-88 | cockpit 88-200 |
|---|---|---|---|
| baseline (no tile-fill, scrambled) | 33253/64000 | 23706/25600 | 8372/35840 |
| **`FIST_TILEFILL` (faithful, port's own live camera)** | **32973** | **23466** | 8368 |
| `FIST_TILEFILL` + camera-orientation aligned (paged-out bridge target pitch≈256 roll≈384) | **27325** | **18620** | 8368 |

Per-region (camera-aligned frame, `close` = |ΔR|+|ΔG|+|ΔB| < 48):

```
sky/upper   rows  8-45 x32-288 : exact=42%  close=42%   (689a sky largely reproduced)
terrain     rows 45-88 x32-288 : exact= 8%  close=21%   (6980 structure right, texture pixels differ)
left panel  rows  8-88 x 0-32  : exact=12%             (cockpit device-subsystem noise, NOT terrain)
cockpit     rows 88-200        : exact=77%
```

The colour/means match (e.g. lower terrain port mean vs ref within a few LSB); the exact-pixel gap is
the terrain micro-texture + the camera. The windshield now visually matches the reference (cloudy sky,
tan desert, horizon) — see the port frame vs `ref/mission_azer1_spawn_native320.png`.

## Corrected premise — the "light-reduce collapse" is NOT the windshield colour blocker

The team lead's priority #1 (fix the 85b8 collapse so 6980's colour slot holds the 254-distinct reduce)
is **already satisfied for the windshield**: `[0x85b8]`'s **first 1 MB** (the exact region `6980` samples
via `[0x85bc]+coord+0x100000`) is **99.8% byte-identical to the banked LIGHT reduce** (78 distinct, max
228) — feeding `6980` the bundle's RED reduce vs the port's own `[0x85b8]` gives a **byte-identical
frame**. The "89 distinct" figure is over the full **4 MB** (`85b8` LOD levels); the windshield never
samples past the first 1 MB. So the `bc90→blockA` mid-`89b0` hook is **unnecessary for the windshield**
(it governs only the overhead map-view `93c0`, per `[[colormap-groundtruth]]` part 2). Confirmed by
extreme-colormap tests (all-0xff vs all-0x5a → 12753-px frame change = the colour read works; RED vs the
native reduce → 0-px change = they are the same buffer).

## What remains for full bit-exact (honest)

1. **The camera pitch/roll bridge (dominant positional).** The port's spawn camera has stale
   `TCB+0x3a`=0 / `+0x3c`=-256 (the `a20d/a192/0459` flight bridge threads only heading `+0x38`). The
   oracle is `+0x3a≈384` / `+0x3c≈256` (empirical AE minimum). Overriding gives the horizon alignment
   (terrain 23466→18620, sky 42% exact). The faithful writer is paged-out extender PM — the
   camera-orientation frontier, not landable here.
2. **Exact-pixel terrain micro-texture.** `6980` is asm-faithful but its tile cannot be bit-verified
   offline (the frame-skew / colormap re-paging capture limit, `docs/oracle_proj395e.md`); natively it
   needs the exact frame-matched camera + ramps to reproduce the oracle tile pixel-for-pixel.
3. **The left cockpit-panel noise block** (`x0-32`, the AUTO-CONTROL/speed panel) — a separate cockpit
   DEVICE-subsystem debt (patches 301/302), present in the baseline too, not a terrain issue.
4. **Promotion to default-ON.** Kept gated (`FIST_TILEFILL`) deliberately: without the faithful camera
   bridge the live-camera frame is colour-correct but horizon-misplaced, so flipping the default would
   change the deterministic spawn md5 to a not-yet-bit-exact frame with no clean verify gate. Promote it
   together with the camera bridge.

## Repro
```bash
NATIVE=/tmp/fist_eg make native
SCR=$(mktemp -d); cp -a armoredfist "$SCR/af"
MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0"
# faithful (live camera):
setarch -R env FIST_DATADIR="$SCR/af" FIST_COOP_TICK=1 FIST_TICK_HZ=25000 FIST_MOUSE="$MOUSE" \
  FIST_TILEFILL=1 FIST_MISSFB=/tmp/spawn.ppm FIST_MISSFB_N=1 /tmp/fist_eg
# with the camera-orientation target (shows the alignment the paged-out bridge would give):
#   + FIST_ISO=1 FIST_ISO_PITCH=256 FIST_ISO_3A=384
```
