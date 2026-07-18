# AUDIO subsystem — recon + shim foundation (iterations 1–2)

Status date: 2026-07-18. Author task: stand up the first port audio output + the audio-verify method.
This is a multi-iteration subsystem. Honest scope: the platform layer (`fist_sb.c`) + the verify harness +
the reference are landed and validated (iter 1); the SOUND DRIVER is now WIRED and its device-registration
init runs (iter 2, §8) — but the engine still does NOT drive the play methods on the menu, so there is NO
port sound-port I/O yet. No bit-exact audio yet. The remaining blocker is now ENGINE-SIDE (§8).

## 8. Iteration 2 — SOUNDDVR wired; blocker moved ENGINE-side (2026-07-18)

**Landed (all gated FIST_SB; the 26 video flows stay byte-identical, native↔wasm md5 `3a6ff1c5`):**
- **SOUNDDVR re-decompiled via SeedDriverVecs** (the authorized baseline change): seeded +0x2 init@0x78,
  +0x6 method@0xc1, and the 14 method-vector code offsets from the driver's 4 reloc sections (0xfd/0x14e/
  0x19a/0x1e7/0x1ec/0x1fd/0x245/0x24b/0x252/0x2e9/0x333/0x414/0x419/0x56b). `make decompile-drivers`:
  43→**89 fns**, REPRODUCIBLE (byte-identical re-run). New pristine md5s: `fist_snd_decomp.c`
  `ba2b4bfe`, `fist_snd.c` `abcafc9d` (was `1e0cfd38`). **MGAVIDEO/engine/ext UNCHANGED**
  (`75c6d726`/`61453e42`/`0051cb56`). The fmap now wires (`fmap=wired(89 fns)`); +0x2 → the init.
- **Four assemble-tool gaps** the seeded driver exposed, fixed in `tools/assemble_fist.py` (engine/mga/ext
  byte-identical): (a) `switchD_SEG:OFF::caseD_N` seg:offset jump-table names; (b) bare-`LAB_` SMC
  lvalues (`LAB_0000_12d9 = 0x411`) enabled for 16-bit driver modules (was FLAT32-only); (c) Ghidra names
  every `jmp far 0:0` thunk `app_entry` → duplicate-marker dedup (`app_entry_0000_01cd`); (d) a Ghidra
  FID phantom `TaskRegister()` in a misdisassembled-SMC gap function → no-op stub.
- **Init device-registration reconstructed (patch 344, asm-verified 0x78):** SS-context `0x2ba9`→`0x1c00`,
  signature rebased to DGROUP:0, and the inert f842 method-vector section-A install done via
  `fist_apply_reloc_at(driver_seg, 0xa, 1)` (installs DGROUP:0x508/0x51c/0x524/0x538/0x53c). Crash-free.

**Decisive characterization:** the menu music **IS SB-DMA digital** — a control DOSBox capture with
`sbtype=none` records `peak=0` (silent); the driver programs the SB DSP (`out(0x22c,0xd1)` = base+0xC
speaker-on) and 8237 DMA in its play methods. So `fist_sb.c` is the correct model.

**THE BLOCKER (moved engine-side, proven):** the engine calls **only +0x2** on the sound driver on the
menu — it never dispatches ANY driver play method. PROVEN not to be a missing driver method-vector: a
throwaway diagnostic installing method-vector sections A+B+C all together still yields only the +0x2 call
and zero SB port I/O. So the menu-music-START is an **engine-side trigger our port does not reach** — the
engine's boot (`515e` loads the sound driver at CRT-init line ~4029) reaches `FUN_0000_00d0` (main/menu)
without issuing the digital-music play, and the sound-ACTIVATE `FUN_1000_50e6`/`516f` sits AFTER the
`return 00d0` (teardown). The music-start inside 00d0 is gated on engine-side sound state
(`DAT_1000_5951/5953`, the descriptor-present bit) that the port does not satisfy. `DGROUP:0xd4`
(=`FUN_1000_1917` device work-object alloc) and `0xf4` (IRQ-callback register) are UNRECOVERED engine
targets (patch 049 class) → they trap harmlessly. **Next iteration:** locate the engine-side menu-music
trigger inside `00d0` and its sound-enable gate (which `DAT_1000_595x` flag it checks); recover
`FUN_1000_1917`/the `0xf4` service so the device fully registers present; then the engine dispatches +0x6
(section-B install + DMA buffer alloc) → the play methods program the SB → `fist_sb.c` streams PCM.

---
## (iteration 1, below)

---

## 1. Trigger-path recon — WHERE does sound fire?

| path | audio? | evidence |
|---|---|---|
| **Intro FMV (TITLE.KDV)** | **SILENT** | `TITLE.KDV` chunk scan: only `iVDK`×395 (frames), `pVDK`×75 (palettes), `xVDK`×2 (headers). **No audio chunk** (`aVDK`/`sVDK` absent). The KDV player (`fist_ext.c`) has no audio handling. The NovaLogic-logo intro plays with no sound. |
| **Main menu** | **CONTINUOUS MUSIC** | DOSBox WAV capture (`ref/audio_menu_oracle.wav`): 12.7 s, peak 6714/32767, 86% non-silent, a rich time-varying envelope from t=0 — this is streamed digital background music, not a discrete blip. |
| **In-mission** | SFX/voice (WVSOUNDS/EVSOUNDS/DSOUNDS.BIN banks) | ~30+ engine sites build sound-event records into `WVSOUNDS.BIN`-family structures (engine `fist.c`); blocked behind the wasm-mission divergence — out of scope this iteration. |

**Verdict:** the earliest deterministic sound on a dual-target-clean path is the **menu background music**
(the intro is silent). It is streamed via SB DMA. There is no discrete "menu click blip" isolable over the
continuous music in the capture — the first sound event IS the continuous music stream.

The reference was captured with `tools/oracle/capture_audio.sh` (boots the ORIGINAL under headless DOSBox,
toggles DOSBox wave-record via XTest Ctrl+F6 — `tools/oracle/xkey.c` — over the menu, copies the WAV out).
`[sblaster] sbtype=sb16 sbbase=220 irq=7 dma=1` (DOSBox defaults, matching SOUND.CFG — §3).

---

## 2. The SB DSP + 8237 DMA + IRQ model (from SOUNDDVR.DVR)

`SOUNDDVR.DVR` (`re_out/fist_snd.c`, 44 fns wired, heavily base-lost) contains several backends:
- **SB DSP** — `FUN_0000_0cd1`/`0cf3`: `out(*base, cmd)` (base port from config, passed by pointer),
  status via `in(base+status)`; the DSP write/read handshake.
- **PC speaker / PIT** — `out(0x61)`, `out(0x43,0xb6)`, `out(0x40,…)` (fallback tone).
- **MPU-401 MIDI** — `in/out(0x330/0x331)` (music via external MIDI, unused with the digital config).
- **PIC EOI** — `out(0x20,0x20)` in the IRQ path.

The driver's **init** (entry vector `+0x2` → body at module offset **0x78**) is a method-vector REGISTRAR:
it installs the driver's play/stop/service methods into the engine DGROUP via the reloc-section service
calls (`lcall *0x12` / `*0xf4` / `*0xd4`) — the same mechanism MGAVIDEO uses. The actual DSP/DMA
programming lives in those registered methods, reached on the play path.

**The SB16 protocol `fist_sb.c` models** (standard, DOSBox-verified): DSP reset (base+6: 1 then 0 → read
0xAA), sample rate (0x40 time-constant / 0x41 SB16 rate), 8-bit single-cycle (0x14) / auto-init (0x1C/0x90)
DMA output, SB16 16-bit (0xB0-0xBF) / 8-bit (0xC0-0xCF) output, speaker on/off (0xD1/0xD3), exit auto-init
(0xDA/0xD9); 8237 channel 1 (8-bit: mask 0x0A, clear-ff 0x0C, mode 0x0B, addr 0x02, count 0x03, page 0x83)
and channel 5 (16-bit: 0xD4/0xD8/0xD6/0xC4/0xC6/0x8B). On a "start DMA output" command the shim reads the
PCM block the engine placed at `(page<<16)|addr` from `g_mem`, up-converts (u8→s16), and appends it to a
PCM ring / WAV. Auto-init closes the loop via the completion IRQ (`fist_sb_pump` → engine SB ISR).

---

## 3. SOUND.CFG decode

`armoredfist/SOUND.CFG` = 10 ASCII digits **`0132710000`** (written by `SOUNDSET.EXE`). The IRQ (7) and DMA
(1) digits at positions 4–5 are the classic SB default and match DOSBox's default `irq=7 dma=1`, with which
the oracle capture produced correct music. **The exact field layout is NOT load-bearing for the shim:**
`fist_sb.c` traps whatever base port the driver programs (the 0x210–0x260 DSP window + the 8237/page ports),
so it follows the driver's runtime config rather than re-deriving it from SOUND.CFG.

---

## 4. `fist_sb.c` — the shim (LANDED, validated)

- Traps the SB DSP window + 8237 DMA (ch1/ch5) + DMA page ports, routed from `fist_vga.c` `in()/out()`
  (only when `fist_sb_owns(port)` — which is 0 for every port unless **`FIST_SB`** is set → **zero effect on
  the 26 video flows by default**).
- Decodes DMA'd PCM → signed-16 mono ring + a WAV sink (`FIST_AUDIO_WAV`, default `/tmp/fist_audio.wav`),
  finalized at `fist_dump_and_exit` (`fist_sb_flush`).
- `fist_sb_pump()` (from the cooperative timer pump) + `fist_sb_set_irq_cb()` = the auto-init streaming
  seam (raise the SB completion IRQ so the engine refills). Present, unused until the driver is wired.
- **Unit-verified standalone** (`tools/oracle/sb_selftest.c`): a synthetic DSP-reset + rate + 8237-program +
  0x14 single-cycle sequence over a 1 kHz sine in `g_mem` → the shim decodes 4410 samples @ ~22050 Hz into
  a valid WAV (peak 25600). `PASS`. This proves the platform layer end-to-end without the engine.
  ```
  cc -O2 tools/oracle/sb_selftest.c re_out/fist_sb.c -I re_out -o /tmp/sb_selftest -lm && /tmp/sb_selftest
  ```

Native + wasm both build with `fist_sb.c` linked; mainmenu + battles AE=0 on native, mainmenu AE=0 on wasm,
native↔wasm md5 identical (`3a6ff1c5`). The 4 pristine engine md5s unchanged.

---

## 5. WHY there is no engine PCM yet (the honest blocker)

Runtime trace of the port on the menu path: **zero sound-port I/O.** The engine issues
`call far SOUNDDVR.DVR:0x2` (the driver init), but the fmap has **no entry for +0x2** → `fist_icall` traps
it (`[icall] TRAP overlay call -> SOUNDDVR.DVR+0x2`) and returns 0. So the driver init never runs, never
registers its play methods, never programs the SB → **the driver is inert and silent.**

Root cause: `SOUNDDVR.DVR+0x2` = `jmp 0x78`, and the init body at **0x78 was never decompiled** as a
function (RecoverAll's static discovery can't see the runtime far-call target). MGAVIDEO had the identical
problem, fixed by `tools/ghidra/SeedDriverVecs.java` (seed the entry thunks + method vectors → re-decompile
→ the init + methods become functions in the fmap). SOUNDDVR was explicitly deferred ("sound is secondary",
`docs/stage1.md`). That deferral is the blocker.

---

## 6. Exact path to bit-exact menu music (next iterations)

1. **Wire SOUNDDVR** (baseline change — needs team-lead buy-in on the pristine md5): run
   `SeedDriverVecs.java` on SOUNDDVR (seed +0x2 init @0x78, +0x6 method @0xc1, and every registered method
   vector) and re-decompile → `fist_snd.c` gains the init + play/service functions. This changes the
   pristine `fist_snd.c` md5 (`1e0cfd38`) — the ONE discipline exception this subsystem requires, exactly
   as MGAVIDEO's re-decompiles changed `fist_mga.c`.
2. **Reconstruct the init/DSP/DMA base-losses** (patch series, asm-verified vs `re_out/fist_snd_image.bin`):
   the driver is DGROUP-base-lost like MGAVIDEO was (patches 031–072). Get the init to register its methods
   + program the SB (reset, rate, DMA, speaker) crash-free. Gate behind `FIST_SB` until proven.
3. **Close the auto-init IRQ loop**: point `fist_sb_set_irq_cb` at the engine's installed SB ISR (via
   `resolve_entry`) so `fist_sb_pump` refills each block — the port then streams the SAME PCM bytes the
   engine DMAs, at the SAME rate, deterministically.
4. **Bit-verify**: `tools/oracle/capture_audio.sh` (DOSBox oracle WAV) vs the port's `FIST_AUDIO_WAV` via
   `tools/wavcompare.py`. For a deterministic compare, PHASE-PIN both: pin the DOSBox recording start and
   the port dump to the same engine event (the menu-music stream's first DMA block) — the current oracle
   capture is phase-unpinned (a characterization reference, not yet a deterministic gate). Then drive
   `wavcompare` AE→0 and add an `audio-menu` flow to `tools/verify.sh` (the framebuffer-flow analog).

---

## 7. Files (this iteration)

- `re_out/fist_sb.c` — the SB DSP + 8237 DMA + IRQ shim (new).
- `re_out/fist_vga.c`, `tools/native_main.c` — route sound ports to `fist_sb`; flush/pump seams (shim edits).
- `tools/oracle/capture_audio.sh` — DOSBox oracle audio capture (XTest wave-record).
- `tools/oracle/xkey.c` — XTest key-chord sender (Ctrl+F6 wave-record toggle).
- `tools/oracle/sb_selftest.c` — standalone DMA→PCM→WAV unit test (`PASS`).
- `tools/wavcompare.py` — resample-tolerant audio compare (the `compare -metric AE` analog).
- `ref/audio_menu_oracle.wav` — genuine DOSBox menu-music reference (12.7 s; phase-unpinned characterization).
