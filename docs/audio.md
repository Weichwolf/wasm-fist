# AUDIO subsystem — recon + shim foundation (iterations 1–3)

Status date: 2026-07-18. Author task: stand up the first port audio output + the audio-verify method.
This is a multi-iteration subsystem. Honest scope: the platform layer (`fist_sb.c`) + the verify harness +
the reference are landed and validated (iter 1); the SOUND DRIVER is now WIRED and its device-registration
init runs (iter 2, §8) — but the engine still does NOT drive the play methods on the menu, so there is NO
port sound-port I/O yet. No bit-exact audio yet. Iteration 3 (§9) FULLY MAPPED the engine-side blocker to a
concrete, asm-verified multi-step chain and proved iter-2's device-registration NEVER actually ran (guard
fails). **No PCM this iteration; no code landed (the foundational fix regresses the 26 video flows — see
§9); the deliverable is the precise, actionable blocker roadmap.**

## 9. Iteration 3 — engine-side blocker chain MAPPED (no PCM; no landable patch) (2026-07-18)

**Headline honesty:** patch 344's device-registration block **never executed** in iter-2 — its guard
`sig==0xcbc3` FAILS (runtime: DGROUP:0 reads `0xffff`, not the engine signature). So iter-2's "init
device-registration runs" was FALSE; it is inert. Root-caused the whole chain, asm-verified. NO patch
landed: the foundational fix (patch 345 draft) is asm-correct but deterministically SIGSEGVs the DEFAULT
(FIST_SB-off) boot 4/4 by un-suppressing a base-lost near-reloc applier — it would regress the 26 video
flows, so it is NOT committed. All hard md5s unchanged (`fist.c 61453e42`, `fist_ext.c 0051cb56`,
`fist_mga.c 75c6d726`, `fist_snd.c abcafc9d`); default boot rc=0; mainmenu AE=0 native.

### The play dispatch (FOUND — the "how the engine programs the SB")
`FUN_1000_516f(a,b)` → **`FUN_1000_50e6(0x4fa, a, b)`** (`0x4fa` = the sound device-descriptor DGROUP
near-offset, == the `mov bx,0x4fa` in the SOUNDDVR init 0x78). 50e6: `*(byte[0x4fc]) |= 0x20; if
((char)byte[0x4fc] < 0) { DAT_1000_5951 = 6; DAT_1000_5953 = *desc; (*DAT_2000_1951)(0x1000); }`. So the
**play method (driver +0x6) fires iff bit7 of the device-descriptor status byte `DGROUP:0x4fc` is set
("device present")**, dispatched by the op-vector `DAT_2000_1951` with op=`DAT_1000_5951`=6.  (op=2 = the
init/open, set by `FUN_1000_5051`; that is the +0x2 our port already reaches.)  Bit7 is set by the device
REGISTRATION, which is the whole blocked chain below.

### The blocking chain (each asm-verified vs `re_out/fist_dat_image.bin`), in dependency order
1. **`FUN_1000_11e0` MEMMGR object-pool init is base-lost (asm 0x111e0 / node-alloc 0x11099).** It builds
   two 2-node sentinel lists for pools `DGROUP:0x16f6`/`0x1718`.  Each node comes from `FUN_1000_1099`,
   which UNLINKS the head of the free-node list at `DGROUP:0x16e2` and **RETURNS it in DS**; the node
   fields are then written via `DS:[0..0xe]` with DS = that node.  Ghidra dropped the DS-reassignment →
   every field store landed at `DGROUP:0..0xe` (`DAT_1000_c000..c00e`).  Net: (a) the pools are never built
   (nodes never populated — verified: `0x16dc` free-count UNCHANGED across the two 11e0 calls, i.e. 1099
   consumes nothing), and (b) node-B's `[0]=0xffff` write **CLOBBERS the engine signature `0xcbc3` at
   DGROUP:0** (set by `FUN_0000_f738`/patch 001) → the SOUNDDVR init's `cmpw [ds:0],0xcbc3` (asm 0x94)
   fails → registration skipped.  Free-node list IS valid at 11e0 time (head 0x39fc, count 0x2fc=764).
   **Draft patch 345** reconstructs 11e0 inline (leaving the shared 1099 untouched — it is called from ~17
   engine sites, its own base-loss must be resolved per-caller) → signature preserved, pools built.  **But
   it EXPOSES blocker 2 (crash).**
2. **The NEAR reloc applier `FUN_0000_f7ef` (DGROUP:0x0a) is base-lost.**  Ghidra MERGED 0xf7ef with the
   adjacent entry 0xf81a and dropped both the `f7c3` retry and the `DS:SI` section walk.  Real asm: bx==0 →
   `je 0xf7ec` → `call f7c3` (loads `bx = [DGROUP:0x74]` = `0x3352` = the reloc-table seg, gated on the
   `DGROUP:0x76` flags) → apply near section `ds=bx : si`.  The old (base-lost) 11e0 had ACCIDENTALLY
   zeroed `DGROUP:0x0a` (the applier vector) → all 5 near-applier sites were harmless no-op traps.  Patch
   345 stops that zeroing → the boot-path site `FUN_0000_1384` (`xor bx,bx; mov si,0x2c4; lcall [0x0a]`)
   fires into the base-lost f7ef C (which takes the wrong branch on garbage `__allregs` params) → SIGSEGV
   at `FUN_0000_f7ef:38823`.  The 5 near sites + their sections (all `base_para=0`, format = leading base
   para then `(dgroup_off, code_off)` pairs → `DGROUP[dgroup_off] = code_off + base<<4`, 0-terminated):
   `1384` si=0x2c4 → event-queue near-vecs `DGROUP:0x3e8..0x3fa`; `1c78` si=0x378 → display-list
   `0x1622..0x162c`; `216d` si=0x39c → `0x1630/0x1632`; `fa7c` si=0x8c → driver-method near-vecs
   `0x26e..`; `14b24` si=0x258 → `0x306..0x310`.  FIX: reconstruct f7ef + f7c3 + a near-section applier
   shim + thread the 5 sites.  **RISK:** several of these near-vecs (e.g. event-queue 0x3e8=FUN_0000_139e)
   are ALSO set by existing patches (125/126) — the reconstruction must be proven not to conflict/regress
   the 26 flows (they are AE=0 vs DOSBox WITH the near-sections applied, so applying them SHOULD stay AE=0,
   but this needs verification).
3. **The FAR section `si=0xec` (which DEFINES `DGROUP:0xd4`=`0x2287:0xf69`=`0x11917`=1917 and
   `DGROUP:0xf4`=`0x19ea:0xf69`=`0x1107a`) is NEVER applied on the menu path** (watchpoint: `DGROUP:0xd4`
   is never written from `app_entry` to the menu; runtime `d4=f4=0` at the sound init).  Patch 031's
   `si=0xec` install does not reach `0x78..0xf8` (documented in patch 049).  FIX: drive the `si=0xec` far
   install at the right time (before the SOUNDDVR init 0x78, after SOUND.CFG parse) so 0xd4/0xf4 populate.
4. **`FUN_1000_1917` (0x11917, device work-object alloc) and `FUN_1000_107a` (0x1107a, IRQ-register →
   `call 0x14ce7` with the driver IRQ handler `cs:0x3d6`) are UNRECOVERED** — not decompiled C functions,
   so even with 0xd4/0xf4 installed they trap.  1917 is a MEMMGR owner-tagged allocator: `push ax; call
   0x11345` (search pools `0x16d4/0x16f6/0x1718` for a block owned by `(bx, cx)`) `→ 0x10cce` (split).  It
   needs pools 0x16f6/0x1718 (blocker 1).  **Recovering these two likely needs an ENGINE RE-DECOMPILE**
   (seed 0x11917/0x1107a into a SeedEngineVecs-style script → changes `fist.c 61453e42` → **APPROVAL-GATED
   per the discipline**), OR hand-reconstruction as loader-shim helpers (107a is an IRQ-register the shim
   can model directly since we own the IRQ layer; 1917 an owner-tagged MEMMGR alloc over the built pools).
5. **The menu-music-START (op=6) trigger inside the `FUN_0000_00d0` → `FUN_0000_cae6` menu subtree is not
   yet located.**  00d0 itself is the top-level `do { 5c3a; cae6; 5c5f } while(!CF)` menu loop + teardown
   (`516f` STOP-sound after the loop, then INT21 4C00).  With the device registered present (blockers 1-4),
   the next iteration must find where the menu build dispatches op=6 (or where the op=2 activate begins
   continuous SB-DMA streaming) — gated on `DAT_1000_5951/5953` + the `DGROUP:0x4fc` bit7.

**Bottom line:** the chain is 1→2→3→4→5, each a genuine asm-verified base-loss/recovery.  Blocker 4 is the
likely approval gate (engine re-decompile).  No shortcut: the signature fix (1) inescapably exposes the
near-applier cascade (2), so both must land together (crash-free) before the device can register — that
alone is a multi-patch iteration, before any PCM.  fist_sb.c + the harness (iter 1) remain correct and
ready to stream the moment the driver programs the SB.

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
