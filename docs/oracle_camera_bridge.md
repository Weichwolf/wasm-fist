# Oracle — the mission render-camera PITCH/ROLL bridge (TCB +0x3a / +0x3c)

> ## HARD RESULT (2026-07-18)
> The AZER1 windshield render-camera **pitch (`TCB+0x3a`) and roll (`TCB+0x3c`)** were captured DIRECTLY
> from the instrumented DOSBox at the exact `FUN_0000_9200` render moment (frame-matched, deterministic).
> **The ref/AE-min frame holds `+0x3a = 384` (pitch), `+0x3c = 256` (roll), `+0x3e = 256` (foc), `+0xcc = 1`
> (detail).** The port's LIVE camera is stale `+0x3a = 0 / +0x3c = -256`. Seeding the captured attitude
> (oracle-anchored, FIST_TILEFILL-gated, mirroring the existing `alt=(h<<8)+1792` terrain-follow seed)
> lands the horizon: spawn **WHOLE AE 32973 → 27325**, sky **10993 → 6524** vs
> `ref/mission_azer1_spawn_native320.png`. Engine PRISTINE (`61453e42`/`0051cb56`/`75c6d726`/`1e0cfd38`),
> `make check` clean, no regression (default boot rc=0, mainmenu AE=0, battles-ok AE=0). The shim change
> is `tools/native_main.c` only.
>
> **The writer is PROVEN paged-out.** There is **ZERO** store to `TCB+0x3a`/`+0x3c` in EITHER static
> image — 0 in `re_out/fist.c` (engine), 0 in `re_out/fist_image.bin` (extender kernel) — the ONLY
> accessors are the two reads inside `FUN_0000_8120` (`movzwl 0x3c(%ebx)` @0x8126, `movzwl 0x3a(%ebx)`
> @0x8132). The engine feeds only heading `+0x38` (via `a20d`). The attitude is written each frame by the
> Doug-Huffman extender's 32-bit-PM flight model (absent from FIST.DAT *and* the kernel image), consistent
> with patch 302's documented "PM flight model — absent from FIST.DAT". This is the SAME class as the
> already-committed camera-Z seed (alt is paged-out too), reconstructed the SAME oracle-anchored way.

## 1. Exact oracle values (deterministic, reproduced across runs)

The render camera is the current-TCB the extender's `0x8120` projection reads through `ds:[0xc93]`.

### Address resolution (the key to reading it)
`ds:[0xc93]` holds `0xf0010000`. The CPU forms the effective address as `(DS_base + ebx + disp) mod 2^32`;
with the extender's flat `DS_base = 0x10000000`, `0x10000000 + 0xf0010000` **wraps to `0x00010000`** — the
real-mode TCB at flat/phys **0x10000** (identity-mapped, `cr3=0xe000`). Reading `+0x3a/+0x3c` at that
wrapped linear yields the sane, deterministic camera below.

### Spawn → settling trajectory (per-9200-frame; `tools/oracle/samples/oracle_camera_bridge_trajectory.txt`)
The tank drives/climbs AZER1, so the camera EVOLVES every frame (position, altitude, and attitude):

| frame | X (+0x2c) | Y (+0x30) | alt (+0x34) | head (+0x38) | pitch (+0x3a) | roll (+0x3c) | foc (+0x3e) | det (+0xcc) |
|---|---|---|---|---|---|---|---|---|
| 1 (first render) | 583982 | 1142557 | 3328 | 26729 | **384** | **384** | 256 | 1 |
| ~4–22 (**ref frame**) | 584042 | 1142465 | 12800 | 26729 | **384** | **256** | 256 | 1 |
| ~1600 (late) | 608166 | 1113545 | 29184 | 38522 | 256 | 256 | 256 | 1 |

The FIRST render is a race (some runs show the pre-flight-model init `+0x3a=0 / +0x3c=-256` — exactly the
port's "stale" value). The **ref** (`ref/mission_azer1_spawn_native320.png`, DAC byte-identical to the
captured frame-1 DAC) frame-matches the **`+0x3a=384 / +0x3c=256`** settling state (alt=12800), NOT frame-1
and NOT the doc's old "oracle camera" (X=609696/alt=29184, a LATE frame ~1600 — that earlier value was a
mis-frame). Confirmed by a port AE sweep (whole/sky):
`384/384 = 29266/8315 · 384/256 = 27325/6524(min) · 384/128 = 29472/8405 · 512/256 = 30559/9464 · 384/0 = 30288/9096`.

## 2. Writer verdict — genuinely paged-out (proven, not assumed)

- **Engine (`re_out/fist.c`):** 0 word/int stores to any `+0x3a`/`+0x3c` (only unrelated byte-field UI
  writes). `a20d` reads heading `+0x38` and rotates the view-offset deltas; it does not touch pitch/roll.
- **Extender kernel (`re_out/fist_image.bin`, 0xbf90 B):** byte-scan for `mov [reg+0x3a],r16` / `+0x3c`
  (both `66 89 4X 3a/3c` and `89 4X 3a/3c`) = **0 hits**; the only `+0x3a/+0x3c` opcodes are the two
  `movzwl` READS in `0x8120` (@0x8126/@0x8132). So neither static image writes the attitude.
- ⇒ the writer is the extender's per-frame PM flight model, loaded/decrypted into the paged PM region at
  runtime (not present in either extracted image). This matches the committed camera-Z finding
  (`tools/native_main.c` ~1176: "No `[c93+0x34]` write exists in fist_image.bin … it is paged out").
  The armed DOSBox write-watch (`FIST_WATCHPHYS`) cannot pin the IP: the mission-load pagefault under
  `core=normal` + the armed RAM recorder is deterministic, and the write-watch requires the recorder.

## 3. The fix (shim, extender-role — `tools/native_main.c`)

Oracle-anchored spawn-attitude seed in the op-0x24 block, **gated on `FIST_TILEFILL`** (default frame
unchanged), directly analogous to the committed `alt=(h<<8)+1792` terrain-follow seed a few lines above:

```c
if (getenv("FIST_TILEFILL")) {
    *(uint16_t*)(tcb+0x3a) = 384;   /* pitch — paged flight-model output, oracle-anchored spawn */
    *(uint16_t*)(tcb+0x3c) = 256;   /* roll  */
}
```

HONEST scope: this is a spawn-anchored seed (the paged model's general slope/dynamics→angle mapping is
not asm-available — no curve-fit). It reproduces the ref-frame horizon; it is NOT a general per-frame
flight model. Kept gated because the frame is not yet bit-exact (terrain micro-texture residual).

## 4. Measurement (port vs `ref/mission_azer1_spawn_native320.png`, FIST_TILEFILL)

| config | WHOLE AE | sky 8–45 | terrain 45–88 | cockpit 88–200 |
|---|---|---|---|---|
| FIST_TILEFILL, port live camera (stale 0/-256) | 32973 | 10993 | 12473 | 8368 |
| **FIST_TILEFILL + attitude seed (384/256)** | **27325** | **6524** | 12096 | 8368 |

The pitch/roll fix is a **horizon-placement** lever: it nearly HALVES the sky AE (10993→6524) and drops
the whole-frame AE by 5648. Terrain (−377) and cockpit (0) are unaffected — the remaining residual is the
terrain micro-texture + the left cockpit-panel device noise (separate frontiers), NOT the camera. `FIST_TILEFILL`
with the attitude seed is byte-identical to the prior `FIST_ISO_3A=384 FIST_ISO_PITCH=256` measurement (0-diff).

## 5. No regression
Engine md5s pristine (`61453e42`/`0051cb56`/`75c6d726`/`1e0cfd38`); `make check` = all patches apply;
default boot rc=0; mainmenu AE=0; battles-ok AE=0. The seed is `FIST_TILEFILL`-gated and op-0x24-only →
the 25 menu/intro/settings verify flows are byte-untouched. NOT promoted to default (not bit-exact).

## 6. Reproduce
```bash
# instrument (patch already carries the [r92cam] capture): build /tmp/debs/dosbox-fist
# then per-9200-frame camera dump to <FISTLOG>.cam.txt (survives the mission-load pagefault):
FIST_R9200MAX=4 FISTLOG=$PWD/scratch/oracle/cam bash tools/oracle/capture_9200_framematched.sh
#   env inside: FIST_R9200CAP=1 FIST_R92_NOMEMARM=1  (NOMEMARM avoids the armed-recorder pagefault)
# port measure:
NATIVE=/tmp/fist_cam make native
SCR=$(mktemp -d); cp -a armoredfist "$SCR/af"
MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0"
setarch -R env FIST_DATADIR="$SCR/af" FIST_COOP_TICK=1 FIST_TICK_HZ=25000 FIST_MOUSE="$MOUSE" \
  FIST_TILEFILL=1 FIST_MISSFB=/tmp/spawn.ppm FIST_MISSFB_N=1 /tmp/fist_cam
compare -metric AE /tmp/spawn.ppm ref/mission_azer1_spawn_native320.png /dev/null   # 27325
```
Banked: `tools/oracle/samples/oracle_camera_bridge_trajectory.txt` (the pinned per-frame camera),
`oracle_camera_frame1.{vram,pal}.bin` (the first-render frame, DAC == ref DAC).
