# Light-tile source builder — the port ALREADY reproduces ds:0x3911 byte-exact; it is only gated off

> ## VERDICT (2026-07-17) — **The port does NOT currently build `ds:0x3911` (it is NULL: `[0x3911]=0`),
> but the builder IS in the port (`FUN_0000_89b0` tail, 0x8d26) and, when un-gated, it reproduces the
> oracle source **100 % BYTE-IDENTICAL**. The source is the **128 KB decoded `5.SKY`** (KLC1 128×1024),
> read by the engine's own `643c`/`9ec0` FILEMGR — NOT a preload, NOT bdc4-upsampled. The gate is the
> render-time detail **SELECTOR at ext `0x7660`** (`[0x395c] = TCB[+0xcc]`), NOT the `.MEG` findfirst
> probe. `TCB[+0xcc]=1` in the port (== the oracle) → the faithful selector value is `395c=1`, which runs
> **zero** bdc4 passes → the source is the raw `5.SKY`. The banked "256 KB" oracle sample's blocks 2/3 are
> a DOSBox over-dump (`fwrite(src,1,0x40000)` hardcoded); the real `ds:0x3911` is 128 KB and 689a samples
> **exactly** within it (max offset `(1023<<7)+127 = 131071`). GOAL-2: the exact 689a resample formula is
> pinned and reproduces the tile's LIGHT rows 160..255 at ~100 % (55.55 % whole-tile = the complete 689a
> contribution); rows 0..159 are the `FUN_0000_6980` voxel overlay (mechanism fully mapped below).**
> Engine PRISTINE unchanged (`61453e42`/`0051cb56`/`75c6d726`); only 3 default-OFF diagnostic seams added
> to `tools/native_main.c` (`FIST_DS3911DUMP`, `FIST_FORCE395C`, `FIST_DS3911EXIT`). NOT landed as active
> behaviour (inert without the 689a-wiring + a 128 KB alloc would perturb the deterministic spawn) — the
> proof is via the seams; the faithful wiring belongs to the later 689a-recovery iteration.

**Date:** 2026-07-17 · **Method:** static disassembly of `re_out/fist_image.bin` + the port's own
decompiled `FUN_0000_89b0`, driven with a default-OFF read-only dump seam on a live AZER1 map-load
(BATTLES→OK→ACCEPT), plus an offline first-principles 689a reconstruction verified vs the frame-matched
`.cap`. Supersedes the "block-A-style last-writer hunt needed" step of `docs/oracle_lighttile_source.md`:
the builder was found statically and the reproduction proven directly in the port — no DOSBox hunt required.

## (1) GOAL 1 — the builder, PINNED and REPRODUCED
`ds:0x3911` (the `add esi,[0x3911]` source of `FUN_0000_689a` @ ext 0x6933) is a **32-bit pointer slot**;
`[0x3911]` points at the source buffer. It is built by the tail of **`FUN_0000_89b0`** (the op-0x18
map-load setup), image 0x8d26..0x8dea (the port has it verbatim, `re_out/fist_ext.c` ~13268):

```
if ([0x395c] != 0) {                              # gate = the detail count
    643c(0, ..., TCB+0xaa)            # open '5.SKY' -> [0x937] = decoded size
    36bf(&[0x3911], [0x937])          # alloc source buffer (128 KB)
    643c([0x3911], ..., TCB+0xaa)     # open again into the buffer
    9ec0([0x937], [0x3911])           # READ+DECODE 5.SKY into [0x3911]
    while (--[0x395c] != 0) {          # bdc4-UPSAMPLE per extra detail level
        2f95(&[0x3911]); 345c(...); bdc4([0x3911]); [0x3915]++; [0x3916]++;
    }
}
```

- `TCB+0xaa = '5.SKY'` (verified from the op-0x18 asset log: `+0x7a=D32.KLC +0x8a=C32.KLC +0x9a=532.pal
  +0xaa=5.SKY`). `5.SKY` header = `"KLC1"` + width `0x80`(128) + height `0x400`(1024) → decodes to
  **128×1024 = exactly 131072 B**. So the source is 128 KB, one decoded `5.SKY`.
- **The gate `[0x395c]` is set by the SELECTOR at ext `0x7660`**, NOT the `.MEG` probe:
  ```
  7660: ebx=[0xc93]; [0x3958]=0x6877; [0x395c]=1
  7677: cmp BYTE[ebx+0xcc],0; je +.. ; [0x3958]=0x689a; [0x395c]=[ebx+0xcc]   # 395c = TCB[+0xcc]
  ```
  So `TCB[+0xcc]!=0` ⇒ `[0x3958]=0x689a` (perspective resample) AND `[0x395c]=TCB[+0xcc]`. **The port's
  `TCB[+0xcc]=1` at op-0x18** (byte-identical to the oracle's render-time `[0x395c]=1`, `[0x3915]=7`,
  `[0x3916]=10` read from `scratch/oracle/lt.pass00.ram.bin`). `395c=1` ⇒ the `while(--395c)` runs **0**
  times ⇒ **no bdc4** ⇒ the source is the raw decoded `5.SKY`.
- The `.MEG` findfirst probe (`FUN_0000_5c98`: INT 21h AH=4E on `4/8/16/40.MEG`, image 0x89e7..0x8a4c)
  only OVERRIDES `395c`/`8490` UPWARD when a `.MEG` file exists. None do (in our data dir or the oracle's
  `cp -a` copy) → those branches never run. The initial `8490 = TCB[+0x59] = 0xb` and `395c = 0` (until
  the selector runs). This was the red herring in the first hunt.

**Port reproduction — DIRECT, byte-exact.** With `FIST_FORCE395C=1` (the selector value), the port's
**own** `89b0` build runs and its `ds:0x3911` is **byte-identical to the oracle source's first 128 KB**:

| config | port `[0x3911]` | `[0x937]` | vs oracle |
|---|---|---|---|
| default (unforced) | `0` (NULL) | — | not built (selector never runs, `395c=0`) |
| `FIST_FORCE395C=1` (raw read) | valid 128 KB | 0x20000 | **blocks 0+1 = 65536/65536 + 65536/65536 = 100.0 %** |
| `FIST_FORCE395C=2` (1 bdc4 pass) | 128 KB | 0x20000 | 4–22 % (WRONG — bdc4 must not run) |

The banked `oracle_lighttile_source_ds3911_static.bin` is **256 KB** only because the DOSBox patch dumps
`fwrite(src,1,0x40000)` unconditionally; its blocks 2/3 (dark-rich, mean 128) are the ADJACENT allocation,
NOT `ds:0x3911`. The real source = blocks 0/1 (128 KB, light) and 689a samples **exactly** within it.
Committed proof: `tools/oracle/samples/port_ds3911_128k_raw5sky.bin` (the port's byte-exact dump).

**The faithful port fix (for the later iteration, NOT landed here):** run the `0x7660` selector logic
(`[0x3958]=0x689a; [0x395c]=TCB[+0xcc]`) before the op-0x18 `89b0` map-load, so `89b0`'s tail builds
`ds:0x3911`. This is real engine behaviour (`TCB[+0xcc]=1` is a genuine mission property). It is **inert
for the framebuffer today** (the port's render is `8deb→85d0→8120→9200`; nothing samples `689a`/`ds:0x3911`
until the tile-fill pipeline is wired) and allocating the 128 KB buffer perturbs the heap → would shift the
bit-verified deterministic spawn md5 for zero colour benefit. So it lands TOGETHER with the 689a/6980
tile-fill wiring, per `docs/oracle_lighttile_source.md`'s multi-part plan.

## (2) GOAL 2 — the algorithm spec + reproduction %
`FUN_0000_3931`: `85d0(camera); if([0x395d]==0){ (*[0x3958])(); 6980(); } else { 686f(); 6c00(); }`.
So **`689a` fills the whole tile, then `6980` overlays** — the tile = `6980_overlay( 689a_resample( 5.SKY ) )`.

### 689a — PINNED EXACT (`tools/oracle/sim_lighttile_689a.py`, reproducible)
Inputs (from the `.cap` glob, ext[0x9000..0x9200]): `90c0 90b0 90b4 90dc 90e0`; constants `38ed=38f1=256`,
shift `cl=[0x3916]=10`. Pre-compute:
```
ebp0 = (90c0 * 90b0) >>u 16 ;  esi0 = (ebp0 * 90b4) >>s 16 ;  ebp = ebp0 << 3
edx0 = -90e0        - (38ed>>1)*esi0        # [0x910c]
ebx0 =  (90dc>>3)   - (38f1>>1)*ebp         # [0x9110]
```
Fill (column-major, `tile[col*256 + row]`), for `col` in 0..255:
```
srccol = ((edx >> 22) & 0x3ff) << 7         # top 10 bits of edx -> source column base (0..130944)
ebx = ebx0
for 128 texel-pairs:
    tile[out++]   = source[ srccol + (ebx>>25 & 0x7f) ] ; ebx += ebp
    tile[out++]   = source[ srccol + (ebx>>25 & 0x7f) ] ; ebx += ebp
edx += esi0                                 # [0x90bc] per-column advance
```
**Result: 55.55 % whole-tile vs `oracle_9200_framematched_pass08.cap`**, and this is the COMPLETE 689a
contribution — per row-band: rows **160-191 98.8 %, 192-223 100.0 %, 224-255 99.1 %** (689a owns the
LIGHT sky/horizon), rows 0-159 25.8–43.4 % (`6980` overwrites the terrain). The residual ~1 % in the sky
rows is 1-LSB texel-cell scatter.

### 6980 — the voxel overlay, MECHANISM MAPPED (rows 0..159; the remaining GOAL-2 work)
`FUN_0000_6980` (image 0x6980..0x6b3d) is a classic Voxel-Space column raycaster:
- `if(90c0 != 90c4){ 90c4=90c0; 395e(); }` recompute depth tables; build rotation `90fc/9100` from
  `90e0/90dc` and the `[ebx*4+0x9450]` matrix.
- `4a60[k] = [0x3918] + k*38ed` for k=0..38f1 (per-tile-column write pointers); `4e60[]` height buffer
  zeroed.
- For screen column `edx = 38fd(128) .. 3905(223)` (the near band): march the ray `ebx=[0x90d4]`,
  `ebp=[0x90d8]` by SMC-patched steps (the `0x7fffffff` placeholders at 0x6ad0/0x6ad6/0x6b50/0x6b56 are
  patched per column from `[edi*4+0x4224]·90fc/9100` and `[edi*4+0x4624]`); coordinate
  `ecx = (ebp>>? | ebx>>?)` via `shld ..,10`; sample **height** `al = heightmap[esi+ecx]`
  (`esi=[0x85bc]`); if `> [edx+0x4e60]` (current column height) → **plot**: `color =
  [esi+ecx+0x100000]` (the colormap plane 1 MB above the heightmap), span = height delta, `rep stos`
  that colour into the tile column `4a60[edx]`, update `4e60[edx]` and `4a60[edx]`.
- Inputs needed to reproduce: decoded heightmap `[0x85bc]` (D32.KLC) + colormap `[0x85bc]+0x100000`
  (C32.KLC), depth tables `0x4224`/`0x4624` (from `395e(90c0)`), camera `90d4/90d8`, rotation `90fc/9100`,
  and the per-column SMC-step derivation. These are all map-load-static or `.cap`-derivable; the offline
  6980 reproduction (frame-matched to pass08) is the concrete remaining GOAL-2 task and belongs with the
  689a-recovery iteration (the port already compiles `m_ext_FUN_0000_6980`, absent only `689a`).

## (3) What remains for the mission-colour unblock
1. **Recover `FUN_0000_689a`** into `re_out/fist_ext.c` (seed 0x689a + re-decompile — a clean ~40-insn
   `stosw` loop; the port has 686f/6877/6980 but not 689a).
2. **Wire the selector + build** — run `0x7660` (`[0x395c]=TCB[+0xcc]`, `[0x3958]=689a`) then let `89b0`'s
   tail build `ds:0x3911` (PROVEN byte-exact here; the port's FILEMGR reads 5.SKY faithfully).
3. **Run the per-frame fill** `3931 → 85d0 → (*[0x3958])=689a → 6980` before `9200` in the op-0x24 render.
4. Secondary/positional: TCB `+0x3a/+0x3c` pitch/roll (the paged-out `a20d/a192/0459` bridge).

## Repro
```bash
NATIVE=/tmp/fist_ds3911 make native   # md5s stay 61453e42 / 0051cb56 / 75c6d726
SCR=$(mktemp -d); cp -a armoredfist "$SCR/af"
MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0"
# port ds:0x3911 == oracle source (blocks 0/1) byte-exact:
setarch -R env FIST_DATADIR="$SCR/af" FIST_COOP_TICK=1 FIST_TICK_HZ=25000 FIST_MOUSE="$MOUSE" \
  FIST_FORCE395C=1 FIST_DS3911DUMP=/tmp/p.bin FIST_DS3911EXIT=1 /tmp/fist_ds3911
cmp <(head -c 131072 tools/oracle/samples/oracle_lighttile_source_ds3911_static.bin) /tmp/p.bin   # identical
# 689a offline reconstruction (55.55% whole-tile, rows160-255 ~100%):
python3 tools/oracle/sim_lighttile_689a.py .
```
