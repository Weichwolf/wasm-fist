# AUDIO subsystem — recon + shim foundation (iterations 1–12)

Status date: 2026-07-18. Author task: stand up the first port audio output + the audio-verify method.

## 18. Iteration 12 — the FNUM PATH IS FIXED + DRIVEN (patch 358): the per-tick voice-envelope advance FUN_0000_0a28 now dispatches the frequency method 10a6 → real OPL A0/B0 fnum writes fire (A0-A8: 0 → 29). HONEST: STILL NO MELODY — the note-ON FEED never runs (the engine's op=6 play trigger 516f→50e6 is NEVER reached; 0966/0997 note-post = ~0). §17's "42 note posts" is DISPROVEN at runtime (2026-07-18)

**HEADLINE: patch 358 reconstructs + drives the missing FNUM feed — `FUN_0000_0a28` (the device-3
per-tick voice/envelope advance, the 0xa3a loop) was base-lost AND its two indirect method dispatches
were mis-based to the ENGINE DGROUP (`g_mem+0x1c1b3`/`0x1c189`) instead of the driver-DS live method
slots.  Fixed → the advance now dispatches slot4 `[ds:0x1b3]` = `FUN_0000_10a6` (writes OPL A0 fnum-lo /
B0 block+fnum-hi from `cs:0x9dd`), and the shim drives 0a28 per PIT tick (fist_sb.c, alongside the
0x3dd ISR).  RUNTIME-PROVEN: real A0/B0 fnum writes now fire — `A0-A8=29  B0-B8=87  key-on=1` (was
A0=0).  BUT the port WAV peak is STILL 330 (oracle 6714), because only ONE note ever keys on: the
engine's note-ON feed never runs.**

**WHAT LANDED (patch 358 + shim; the 4 hard-pristine md5s UNCHANGED
`61453e42`/`0051cb56`/`75c6d726`/`e6d610c5`):**
- **Patch 358 — `FUN_0000_0a28` reconstructed** (asm-verified vs `re_out/fist_snd_image.bin`, driver-DS =
  `DAT_0000_0831<<4`, the patch-353/354 class).  Entry `mov cs:0x831,ds` sets ds = the driver data seg;
  Ghidra base-lost every ds: access to a bare offset.  The loop walks the 10 device-3 voices (9..0),
  accumulates each voice's note = `voice[0xc2]` (base, set on key-on by 0aa7) + `voice[0xb8]` (bend) +
  the note-stream delta at `voice[voice*2+0x90]`, and when it CHANGES from `voice[0xcc]` dispatches slot4
  `[ds:0x1b3]` = 10a6 (fnum).  The two mis-based dispatches fixed to read the driver-DS slots
  (`fist_icall(fist_snd_base + *(uint16_t*)(D+0x1b3))` freq; `+0x189` tail = slot1 = 10a5 ret).  In the
  real driver 0a28 is dispatched from the timer ISR at `cs:0x1d2` via `call [cs:0x5c2]` (the device-3
  config at 0x95b sets `[cs:0x5c2]=0x0a28`); the port drives it per PIT tick from the shim (we own INT-8).
  **INERT on the default boot** (device 3 selected + advance driven only under FIST_SB).
- **Shim (`re_out/fist_sb.c` `fist_snd_isr_tick`, FIST_SB/OPL-gated):** drive `fist_snd_base+0xa28` each
  tick after the 0x3dd ISR.  `re_out/fist_opl.c` gained fnum-write diagnostics (A0-A8 / B0-B8 / key-on
  counts in the WAV-finalize summary).

**RUNTIME-PROVEN (native, setarch -R, FIST_SB=1, 22 s @ FIST_TICK_HZ=25000):**
`[opl] fnum writes: A0-A8=29  B0-B8=87  key-on=1`.  The fnum path FIRES (10a6 reached, was 0 hits).
But `key-on=1` and the WAV peak stays 330 (near-silent); wavcompare vs `ref/audio_menu_oracle.wav` =
MISMATCH (peak 328 vs 6714, xcorr 0).

**THE EXACT REMAINING BLOCKER — the note-ON FEED never runs (measured, not guessed; bounded gdb counts
over the full 22 s FIST_SB menu run):**
- `FUN_1000_50e6` (the op=6 continuous-music PLAY handler, `516f→50e6(0x4fa,…)`, gated on
  `DGROUP:0x4fc` bit7) = **0 hits**.  `516f` unconditionally calls 50e6, so the play trigger is NEVER
  reached for the menu.  The gate itself PASSES (`DGROUP:0x4fc=0xe0`, bit7=1 — the device is present) —
  it is the CALLER (one of the 3 `516f` sites) that the menu path never reaches.  So **the engine issues
  no menu-music play command in our port's menu.**
- `FUN_0000_0966` (driver note-dispatch) = **1**, `FUN_0000_0997` (note-ON allocator) = **0**.  The "42
  device-3 note posts" of §17 are **DISPROVEN at runtime** — they do not happen (§17 overstated; the peak
  was 330 then and 330 now, i.e. the note posts never actually keyed on).  `be0e` (sound-object register)
  = 2 (boot registration only, not a note feed).
- The single key-on (=1) + the 29 A0 writes come from ONE voice: a lone note-on sets `voice[0xc2]` +
  `voice[0xcc]=0xff`, then the advance detects the change and reprograms the fnum (and the note-stream
  modulates it) — one sustained note with vibrato, NOT a melody.
- **System 1 (the arm-gated ISR sequencer, cs:0x60/note-table DGROUP:0x4fe) is ALSO dead:**
  `note-table[0x4fe]=0x0000`, `voice-active-hits=0` — the device-registration REFORMAT that populates it
  (the `FUN_1000_1917` tail `ds=blk; call 0x10cce` the shim documented-SKIPS) still never runs.

**Both note SOURCES are dead → the menu music genuinely never STARTS.**  The fnum path (this iteration)
was a real, necessary, asm-verified fix (10a6 was genuinely never called), but it is downstream of the
missing START.  Forcing `516f`/`50e6` or hand-feeding notes would be fabricated audio (forbidden) — the
feed must come from the engine's own trigger.

**NEXT ITERATION (make music START):** locate + reach the menu-music PLAY trigger — the caller path to
`FUN_1000_516f`/`50e6` (op=6) inside the menu build (`00d0`/`cae6`/`e714`), or the driver-sequencer
START that populates the note-table `DGROUP:0x4fe` (the `1917`/`0x10cce` reformat).  It is gated on
`DGROUP:0x4fc` bit7 (already set), so the caller — not the gate — is the target.  Once a real note stream
posts (`0966→0997→0aa7→10e3` key-on), THIS iteration's fnum path (0a28→10a6) supplies the pitches → the
OPL synthesizes the real melody.

**No-regression (VERIFIED both targets):** `make check` = all 358 patches apply; native mainmenu md5
`3a6ff1c5`, **wasm mainmenu md5 `3a6ff1c5` (native↔wasm 0-diff)**; default boot rc=0.  4 hard-pristine
md5s unchanged; patch 358 + the shim advance-drive are FIST_SB/OPL-gated (default OFF → the 26 video
flows byte-identical by construction) + the opl.c counters are diagnostic-only.  **HONEST: no menu melody
on either target yet — the fnum path is fixed + driven, but the note-ON feed (the engine's play trigger)
is the next, upstream blocker.  WAV peak 330 vs oracle 6714 = MISMATCH.**

## 17. Iteration 11 — the ARM-GATE PRODUCER (FUN_1000_1042) is reconstructed + driven; the arm RUNTIME-REACHES 2 and the ISR runs every tick (iter-9's "extender-resident arm" DEFINITIVELY refuted at runtime). HONEST: still no music — the sequencer is armed+driven but NOT FED (2026-07-18)

**HEADLINE: patch 357 recovers the sound-sequencer ARM-GATE PRODUCER `FUN_1000_1042` (asm 0x11042,
`push ss;pop ds; incb [ss:0x23e]`) — Ghidra had base-lost it to `DAT_2000_bcce++` (the bogus SS=0x2ba9
context).  The shim drives it per sound-tick, so the arm word DGROUP:0x23e now RUNTIME-REACHES 2 and the
SOUNDDVR timer-ISR (0x3dd, patch 356) runs its 7-voice scan every tick (538550 runs vs 0 before).  This
RUNTIME-PROVES §16's recon and closes iter-9's "the arm is extender-resident" fear: the arm IS
engine-resident and reachable via 1042.  BUT the menu music still does NOT play — the sequencer is now
ARMED + DRIVEN but never FED with notes (proven below).  Port WAV peak 330 (oracle 6714); wavcompare
MISMATCH, xcorr 0.  NO fabricated audio.**

**WHAT LANDED (patch 357 + shim; the 4 hard-pristine md5s UNCHANGED
`61453e42`/`0051cb56`/`75c6d726`/`e6d610c5`):**
- **Patch 357 — `FUN_1000_1042` reconstructed** (asm-verified vs `re_out/fist_dat_image.bin`): rebase the
  `incb` target from `DAT_2000_bcce` (linear 0x2bcce) to the real arm word low byte DGROUP:0x23e = linear
  0x1c23e; CF=1 on byte overflow.  Producer/consumer semaphore: the ISR `cmpw ss:0x23e,2` gates its scan,
  `decw` (0x40d) drops 2→1 only when a voice is active; 1042 bumps 1→2.  The fn IS in the engine fmap and
  dispatched indirectly by a sound-service vector, but the port never reaches that dispatch → the shim
  drives it (we own the PIT).  **INERT on the default boot** (never called → mainmenu byte-identical).
- **Shim (`re_out/fist_sb.c` `fist_snd_isr_tick`, FIST_SB/OPL-gated):** (a) one-time faithful voice-slot
  init `cs:0x60..0x66 = 0xff` (exactly the driver device-config 0x252 does at 0x272; that config path is
  unreached, so the ISR's scan would otherwise read uninit 0 as "note 0 active") — SAFE: with slots free
  the scan is a no-op; (b) drive `FUN_1000_1042` (via `fist_icall(0x11042)`) to pin the arm at 2 each tick.
  `FIST_SND_DIAG` diagnostic added (sequencer-fed snapshot).

**RUNTIME-PROVEN (native, setarch -R, FIST_SB=1, `FIST_SND_DIAG`):**
`[snd-diag] arm-bumps=2 isr-runs=538550 max-arm=2 voice-active-hits=0 note-table[0x4fe]=0x0000`.
The arm reaches 2 (was 0 forever); the ISR runs every tick; but **NO voice is ever activated and the
note-table DGROUP:0x4fe stays 0** → the sequencer is not fed.

**THE EXACT REMAINING BLOCKER (asm-mapped this iteration — the menu music is a device-3 OPL note stream
that key-ons WITHOUT fnums):**
- The engine posts **42 device-3 note events over time** (`0966→0997`/`+0x10e3`, calltrace: spread from
  the first tick to ~63% of the run — a MELODY, not init).  They dispatch the key-on method `0x10e3`
  (writes B0|0x20 key-on + the level 0x40 reg) but produce **ZERO A0 (fnum-lo) writes and no fnum** →
  every note keys on at fnum 0 → near-silent (peak 330).
- **Root: the FREQUENCY method `0x10a6` (device-3 slot4, writes A0 from the fnum table `cs:0x9dd[note*2]`
  + B0 fnum-hi/block) is NEVER CALLED on the note path** (calltrace: 0 hits).  10a6 is dispatched by the
  per-tick VOICE-ENVELOPE ADVANCE loop at driver `0xa3a` (`call *ds:0x1b3`), which walks each voice's
  note-stream pointer `[voice+0x90]` (set by `0cfb` from table `cs:0x15b4[note*2]`), computes the current
  note, and reprograms the fnum when it changes.  That advance loop is not reached for the posted voices:
  the note-on registers a device-3 voice (`driver_ds:0x107/0x111`) but the per-voice envelope-stream
  advance (`0xa3a` → 10a6) never runs for it → no fnum.
- **Two interleaved voice systems (now disambiguated):** (1) the ISR `0x3dd`/cs:0x60 sequencer (arm-gated,
  note-table DGROUP:0x4fe, note-on `0419(real-note)`) — armed+driven this iteration but its cs:0x60 voices
  need the SONG DRIVER that calls `0419(real-note)` (the ONLY caller of 0419 is the ISR itself with
  al=0xff/release → no note-on source reached); (2) the device-3 direct voices
  (`driver_ds:0x107/0x90/0xcc`, per-tick advance `0xa3a`, fnum `10a6`) fed by the engine's 42 note-posts.
  The 42 posts are system (2); their silence is the uncalled `10a6`/`0xa3a` fnum advance.

**NEXT ITERATION (make music play — the fnum path is the tractable lead):** drive the device-3 per-tick
voice-envelope advance `0xa3a` (→ `10a6` fnum → A0/B0) for the posted voices — determine its real per-tick
caller (is it reached via the ISR `0419→0579` device far-vector once a cs:0x60 voice exists, or a separate
device service?) and rebase/thread it so the 42 posted notes program real fnums; then the melody keys on
with pitch → OPL → PCM.  In parallel, locate the cs:0x60 SONG driver (the `0419(real-note)` caller /
note-table 0x4fe population) for the ISR-sequencer path.  Both are device-3 driver base-losses of the
same class as patches 353/354.

**No-regression (VERIFIED both targets):** `make check` = all 356 patches apply; native mainmenu/about/
settings/battles/selplayer AE=0, wasm mainmenu AE=0, **native↔wasm 0-diff (md5 `3a6ff1c5`)**; default boot
rc=0.  4 hard-pristine md5s unchanged; all additions FIST_SB/OPL-gated (default OFF → the 26 video flows
byte-identical by construction; patch 357 is inert unless the shim drives 1042).  **HONEST: no menu music
on either target yet — the arm is recovered + the ISR is driven, but the note-fnum feed is the next
blocker.  WAV peak 330 vs oracle 6714 = MISMATCH.**

## 16. Iteration 10 (READ-ONLY RECON, decisive tractability gate) — VERDICT: menu-music sequencer is **RECOVERABLE**, NOT extender-paged. Iter-9's "arm is extender-resident" is REFUTED (2026-07-18)

**HEADLINE: the whole menu-music path — arm, note-table reformat, sequencer, note-dispatch, and the song
data — lives in images we HAVE (FIST.DAT + SOUNDDVR.DVR). NOTHING is in the extender kernel image
`fist_image.bin` (proven ZERO sound code), and there is NO music data file. The blocker is unfinished
engine-side reconstruction (base-loss recovery + reaching the sound-service tick), the SAME class as the
26 video flows and iters 1–9 — BOUNDED, multi-iteration, tractable. It is NOT the genuinely-absent-from-
all-images class of the mission-terrain paging.** No engine/shim/patch changes; the 4 md5s unchanged
(`61453e42`/`0051cb56`/`75c6d726`/`e6d610c5`).

**(a) The arm-gate `DGROUP:0x23e` writer — FOUND in FIST.DAT (iter-9's "extender-resident" was an
INCOMPLETE-SCAN artifact).** Iter-9 scanned only the `c7 06 3e 02` form (mov WORD imm) → saw only
`0xe025`=disable → wrongly concluded the enable is extender-resident. A full scan (inc-byte `fe 06`,
mov-byte-imm `c6 06`, register forms) across all four images finds the real manipulators, **all in
FIST.DAT**:
  - **`0x11042` = `FUN_1000_1042` = `push ss;pop ds; incb [ss:0x23e]`** — the sequencer PRODUCER (bumps the
    gate). Ghidra base-lost it to `DAT_2000_bcce = DAT_2000_bcce + 1` (the bogus SS=0x2ba9 context, the
    §15/patch-356 class). **It IS live**: in the engine fmap (`{0x11042u,&FUN_1000_1042}`, fist.c:69736),
    dispatched indirectly via a sound-service vector (not dead-stripped).
  - **`0x11066` = `movb $0xff,[0x23e]`** (hold-overflow clamp), `0x1105e` reads it as a word.
  - **`0xe025` = `movw [0x23e],0`** (disable, intro/title — iter-9's only find).
  The mechanism is a producer/consumer SEMAPHORE, not a boolean: the SOUNDDVR ISR (`snd 0x3dd`)
  `cmpw ss:0x23e,2` runs the 7-voice scan only when ==2, and `decw ss:0x23e` (snd 0x40d) drops it to 1
  each pass; `FUN_1000_1042` must be called per sound-tick to bump it back to 2. **So the arm is
  ENGINE-resident and merely UNREACHED** (the port never calls the sequencer-tick `FUN_1000_1042`), exactly
  the same character as "the shim's 1917 records the work-obj seg but SKIPS the reformat" — recoverable.

**(b) The extender kernel `fist_image.bin` contains ZERO sound code — the decisive DISPROOF of
"extender-resident like the renderer".** `re_out/fist_kernel_decomp.c` + a byte scan of the image: no
`out 0x388/0x389` (the 0x389a decomp hits are address args, not the OPL port), no OPL/AdLib/music/sequence
strings, no arm/note-table writer (no `[0x23e]`/`[0x1c23e]`/`[0x4fe]`/`[0x1c4fe]` store in any addressing
form). Unlike the KDV renderer — which WAS found in this kernel image and integrated as the `fist_ext`
module — the menu-music sequencer is **not there.**

**(c) Note-table `DGROUP:0x4fe` + voice slots — engine/driver-resident, recoverable.** The pointer is read
all over the recovered SOUNDDVR ISR (snd 0x454/0x4d1/0x500/0x52f/0x55e) and the note-dispatch `0x1d62`
(note-on) / `0x1e13` (note-off) queue voice commands into the driver's per-voice command buffers
(driver_ds:0x12bf+) → the device method (OPL for device 3). It is written register-indirect by the engine
device-registration REFORMAT (MEMMGR path — 1917 tail `ds=blk; call 0x10cce`) that the shim documented-
SKIPS. All in FIST.DAT + SOUNDDVR.DVR.

**(d) The menu SONG is BUILT-IN (no music file) — engine-resident.** `armoredfist/FISTDATA/` has NO
music/sequence file (`.MUS/.XMI/.CMF/.MID/.IMF/.SNG` = 0; only `SOUND.CFG`, `SOUNDDVR.DVR`,
`SOUNDSET.EXE`); the sound `.BIN` banks (WVSOUNDS/EVSOUNDS/DSOUNDS) are in-mission SFX. Confirms iter-9's
"no music file opens." The menu registers sound OBJECTS on boot via engine `FUN_0000_be0e`
(`ax=word[DGROUP:0x9f2e+bx]`, `es=word[DGROUP:0x9f1c]`, register method `lcall *0x510` → driver 01ec/0af4)
from ENGINE DGROUP tables; the note feed (0966/0997) posts from those engine objects. Song source =
engine data, not a file, not the extender.

**VERDICT — RECOVERABLE.** Every piece is in an image we have. Integration is straight engine/driver patch
reconstruction (CHEAPER than the KDV `fist_ext` route — no new module, no re-decompile of a separate image):
  1. Reach/reconstruct the sequencer PRODUCER `FUN_1000_1042` (SS=DGROUP rebase of `incb [0x23e]`) + wire
     the sound-service tick that dispatches it so `[0x23e]` reaches 2.
  2. Reconstruct the device-registration REFORMAT (populate note-table `DGROUP:0x4fe` + init voice slots
     `cs:0x60+`=0xff) — the `FUN_1000_1917` tail (`ds=blk; call 0x10cce`) the shim skipped.
  3. Drive the engine note-post feed (be0e/0966/0997) from the built-in sound objects (DGROUP:0x9f2e/0x9f1c)
     so real key-ons + fnums post.
  4. THEN the already-driven ISR (`snd 0x3dd`, patch 356) advances real notes → OPL key-ons →
     `fist_opl.c` PCM → wavcompare vs `ref/audio_menu_oracle.wav`.

**RECOMMENDATION: CONTINUE audio.** It is tractable and bounded — the same base-loss-recovery method
already proven 9 iterations deep. It is NOT deep-blocked. (Contrast: the mission-terrain renderer IS the
genuinely-paged-out class; the menu-music sequencer is not.)

## 15. Iteration 9 — the driver TIMER-ISR sequencer body (0x3dd) is recovered + rebased + driven per-tick; HONEST: still no music — the sequencer is never FED (arm-gate/note-table/voice-slots never written) (2026-07-18)

**HEADLINE: FUN_0000_03dd (the SOUNDDVR driver timer-ISR body Ghidra had left as the empty 0x3d6
far-jmp placeholder) is recovered via a re-decompile seed + the note-timing family (0419/0415/0579) is
faithfully two-segment-rebased + the shim now drives the ISR once per engine INT-8 tick.  BUT the menu
music still does NOT play, and this iteration proves WHY conclusively (watchpoints): the ISR's input
state is NEVER initialized on the reached menu path — arm-gate DGROUP:0x23e stays 0, note-table pointer
DGROUP:0x4fe stays 0, the per-voice active slots cs:0x60..0x66 stay 0.  The note stream never reaches a
playable state; the port WAV is still near-silent (peak 330, 147 OPL writes = instrument init + ONE
stray key-on + no fnum).**

**WHAT LANDED (patch 356 + shim; the 3 hard-pristine engine md5s UNCHANGED
`61453e42`/`0051cb56`/`75c6d726`; fist_snd.c re-baselined `9b642483` → `e6d610c5`, reproducible):**
- **Re-decompile seed 0x3dd** (added to the snd `FIST_DRIVER_SEED_OFFS` in `Makefile decompile-drivers`;
  103→103 fns, 0x3dd now a real function; all 354 patches re-apply clean).
- **Patch 356** — recover FUN_0000_03dd + faithfully segment-rebase the note-timing family
  (0419 note-advance / 0415 release tail / 0579 device far-vector).  Asm-verified vs
  `re_out/fist_snd_image.bin`.  The family runs with **TWO segments Ghidra scrambled** (some derefs based
  at 0, some at engine-DGROUP 0x1c000, some at the bogus SS=0x2ba9 context `_DAT_2000_bcce`/`_DAT_2000_bf8e`):
  - **DS = CS = the driver LOAD seg (`fist_snd_base`)** — the per-voice static state: cs:0x5e (current-note
    word), cs:0x5f (current channel), **cs:0x60+bx (active-note-per-voice, 0xff=free)**, cs:0x67+bx (alloc),
    cs:0x68+bx (duration), cs:0x6f+bx (tag), cs:0x70+bx (note idx).
  - **SS = DGROUP (0x1c00)** — the **arm gate word [ss:0x23e]** (the 7-voice scan runs only while ==2) and
    the **device object at DGROUP:0x4fa (dev seg) / 0x4fc (status) / 0x4fe (note-table OFFSET)**.  The note
    table `VT = DGROUP:[word 0x4fe]` is indexed by note*4 (flags byte at +2).
  - Fixes: dropped `iVar4 = unaff_CS` (Ghidra mis-modeled the `push bx;…;pop bx` around the 0419 call → it
    restores the loop counter); the `[0x5e]` store-width (`mov [0x5e],ax` is 16-bit, not the 32-bit Ghidra
    emitted which would clobber cs:0x60/0x61).
- **`fist_snd_isr_tick` (re_out/fist_sb.c)** — drives `fist_snd_base+0x3dd` (the body; **0x3d6 is the
  ljmp-to-old-handler chain slot, not the entry**) once per engine INT-8 tick from the shim's ISR pump
  (native_main.c), gated on `g_snd_isr_seg` + `fist_opl_enabled()` (FIST_SB/OPL).  We ARE the extender/PIT
  layer (107a chained the driver ISR into INT-8), so per-tick invocation is the faithful hardware-timer
  equivalent.  Behaviour-neutral until the sound system is armed → the 26 video flows stay byte-identical.

**RUNTIME-PROVEN (native, setarch -R, watchpoints — the decisive residual):**
- Device present: `DGROUP:0x4fa=0x3a07`, `DGROUP:0x4fc=0xe0` (bit7=1). ✓
- **Arm gate `DGROUP:0x23e = 0` ALWAYS** — full-image scan (both .DVRs + FIST.DAT): the ISR only READS/
  DECREMENTS it; the **ONLY writer anywhere is engine `0xe025` = `movw [0x23e],0` (DISABLE)** on the
  intro/title setup.  **NO writer sets it to the enable value 2** → the arm is set by the Doug-Huffman
  EXTENDER's sound/timer init (not in FIST.DAT/either .DVR), same class as the extender-resident renderer.
- **Note-table pointer `DGROUP:0x4fe = 0` ALWAYS** (watchpoint: never written after registration).
- **Per-voice slots cs:0x60..0x66 = 0 ALWAYS** — `0419` is the ONLY thing that would set cs:0x60+bx to a
  note, and its ONLY caller is the ISR itself with al=0xff (release).  **`0419(real-note)` is never called**
  → the ISR's active-voice table is never populated.  (Init should also set cs:0x60+ = 0xff/free; it isn't,
  so uninitialised 0 reads as "note 0 active" — another symptom of the missing sound-init.)
- OPL output = 147 writes: instrument init (regs 0x20–0xe0 ×6 ch) + B0 mostly 0x1f (block/fnum-hi, **no
  key-on bit**) + **exactly ONE stray key-on** (`reg[0xb0]=0x3f`, bit0x20 set) + **zero A0 (fnum-low)
  writes** → no melody.  peak 330, near-silent.

**ROOT CAUSE (conclusive):** the driver timer-ISR (0x3dd) is a per-tick note-DURATION/envelope manager,
NOT a self-contained song player — it needs its input state (arm 0x23e, note-table 0x4fe, active slots
cs:0x60+) populated by the sound-START, which never runs on the menu.  The engine's device-3 note posts
(`0966/0997` → **driver_ds:0x107+**, a DIFFERENT voice struct) produce only init/one-off OPL writes, not a
melody.  The continuous menu music requires: (a) the extender's sound-init arm (`DGROUP:0x23e=2`), and (b)
the note-table setup (`DGROUP:0x4fe`) — the shim's `1917` records the work-object seg but SKIPS the
work-object **reformat** that would populate it.  Both are the next iteration's blockers.

**+0xfab** (device-3 instrument-load mid-entry inside `0f99`) — still a fmap MISS (1 hit); needs a seed/
fmap-register.  NB even fixed it yields only the init blips (device-3 is the SFX path here), NOT the
continuous sequencer music — do NOT expect music from +0xfab alone.

**NEXT ITERATION:** (1) reconstruct the device-registration **reformat** (populate `DGROUP:0x4fe` note-
table + init cs:0x60+ = 0xff) — trace the real `FUN_1000_1917` tail (`ds=blk; call 0x10cce`) the shim
replaced; (2) locate/replicate the extender's sequencer ARM (`DGROUP:0x23e=2`) OR the engine song-stream
feed (no music file opens → built-in/engine-fed); THEN the driven 0x3dd advances real notes → key-ons.

**No-regression (VERIFIED both targets):** `make check` = all 354 patches apply; mainmenu native AE=0 +
wasm AE=0, **native↔wasm 0-diff**; default boot rc=0.  Hard-pristine `61453e42`/`0051cb56`/`75c6d726`
unchanged; fist_snd.c `e6d610c5`.  All additions FIST_SB/OPL-gated (default OFF).  **HONEST: no menu music
plays on either target yet — the sequencer is recovered + driven but never fed.**  The wasm FIST_SB
sound-dispatch parity remains UNCONFIRMED (iter-8 §14 follow-up: no OPL trace under node).

## 14. Iteration 8 — the sound device now REGISTERS PRESENT (1917/107a recovered + si=0xec) — honest: still no continuous music (2026-07-18)

**HEADLINE: the two UNRECOVERED device-registration service fns FUN_1000_1917 (DGROUP:0xd4) and
FUN_1000_107a (DGROUP:0xf4) are recovered as loader-shim helpers + the si=0xec far section is installed,
so the SOUNDDVR device now REGISTERS PRESENT on the menu: DGROUP:0x4fc bit7=1, the device descriptor is
populated, the timer-ISR is registered.  BUT the continuous menu MUSIC still does NOT play — the note
stream needs the driver timer-ISR sequencer (registered, not yet driven per-tick) + the song source.
NOT bit-exact; port WAV near-silent (peak ~331, 147 OPL reg writes = instrument init + voice key-offs).**

**WHAT LANDED (patch 355 + hand-written shim; the 3 hard-pristine engine md5s UNCHANGED
`61453e42`/`0051cb56`/`75c6d726`, fist_snd.c UNCHANGED `9b642483` — NO re-decompile this iteration):**
- **`fist_snd_1917` / `fist_snd_107a` (re_out/fist_sb.c)** — the two engine service fns Ghidra never
  recovered, reconstructed asm-verified vs `re_out/fist_dat_image.bin` as loader-shim helpers:
  - **1917 (asm 0x11917)** = the owner-tagged MEMMGR SEARCH (NOT an allocator, as §9 assumed): `call
    0x11345` searches pools 0x16d4/0x16f6/0x1718 (built by patch 345) via 0x1135f for the block owned by
    `(bx=0x4fa, cx=DGROUP=0x1c00)` — head `word[DGROUP:pool+0xa]`, next `word[blk:0xc]`, match
    `word[blk:6]==bx && word[blk:8]==cx`.  If found → `ds=blk; call 0x10cce` (reformat); else CF=1
    (fail).  **The (0x4fa,DGROUP) block EXISTS in the pools → 1917 finds seg 0x39ea** (the caller
    discards the return, so no fake alloc / no shared-CF touch — faithful).
  - **107a (asm 0x1107a)** = the IRQ/timer-ISR register: `call 0x14ce7` splices the driver ISR
    `SOUNDDVR cs:0x3d6` into the INT-8 handler chain (a standard far-jmp chain-insert at es:bx=
    snd_seg:0x3d6, si=engine chain node cs:0x19bb).  The shim OWNS the PIT/INT-8 layer → "register" =
    record the driver ISR entry (`g_snd_isr_seg:g_snd_isr_off` = 3a07:03d6) for the future per-tick
    drive.  Both wired into `fist_icall` (linear 0x11917/0x1107a → the helpers).
- **Patch 355 (FUN_0000_0078, FIST_SB-gated)** — install the FAR reloc section `si=0xec` right before
  the driver init's `lcall [ds:0xf4]`/`[ds:0xd4]` (which patch 344 already runs but which trapped because
  DGROUP:0xd4/0xf4 were 0 — the documented blocker 3).  si=0xec DEFINES **DGROUP:0xd4 = 0xf69:0x2287 =
  linear 0x11917** and **DGROUP:0xf4 = 0xf69:0x19ea = linear 0x1107a** (verified at runtime:
  `DGROUP:0xd4=0x0f692287`, `DGROUP:0xf4=0x0f6919ea`) → the init's calls now land in the shim helpers.

**RUNTIME-VERIFIED (native, deterministic, setarch -R, break at the 1917 call):**
- `desc[DGROUP:0x4fa] = 0x3a07` (device descriptor → SOUNDDVR seg — populated).
- `status[DGROUP:0x4fc] = 0xe0`, **bit7 = 1 → the device registers PRESENT** (the 50e6 op=6 play gate).
- `[snd] 1917: device work object found @ seg 0x39ea (pools)`; `[snd] 107a: timer-ISR registered @
  3a07:03d6` (both fire; before, they trapped → registration incomplete).
- The engine's op=6 play dispatch `FUN_1000_50e6` IS reached and dispatches the note-play chain
  (SOUNDDVR +0966/+10e3/+1ec run repeatedly).  **NB: 50e6 was already reachable BEFORE this patch** (so
  do NOT attribute its reach to the registration — the honest delta is that 1917/107a now COMPLETE
  instead of trapping, and the device is present).

**WHY NO MUSIC YET (the exact residual, asm-mapped):** the menu music is a CONTINUOUS note stream driven
by the DRIVER TIMER-ISR `SOUNDDVR cs:0x3d6` (body at 0x3dd: reentrancy guard `cmpw ss:0x23e,2`, then a
7-voice scan calling the note-advance `FUN_0000_0419` → `0579`/`0415`, all driver-DS base-lost;
`_DAT_2000_bf8e` = the voice-descriptor table).  Ghidra decompiled the 0x3d6 far-jmp placeholder as the
EMPTY `app_entry_0000_03d6` — the real 0x3dd body is unrecovered.  So even though 107a now registers the
ISR, it is **not driven per PIT tick**, and the current OPL output is only the instrument-init + the
voice key-offs (0xB0..0xB8 = 0x1f/key-off, the operator patch regs 0x20-0xe0 for 6 channels — NO
`0xB0 |= 0x20` key-ON, NO time-varying fnum) → 147 writes total, peak ~331.  Also on the note-play chain
the instrument-load mid-entry **SOUNDDVR +0xfab** (asm 0xfab: `shl bx,4; add 0x1dd; mov es,cs:0x831` =
driver-DS instrument-record load) is a fmap MISS (patch-353/354 driver-DS class).

**NEXT ITERATION (make music play):** (1) reconstruct the driver timer-ISR body 0x3dd + rebase
0419/0579/0415 to the driver DS (`DAT_0000_0831<<4`) + the voice table `_DAT_2000_bf8e`; (2) drive it
cooperatively per PIT tick from the recorded 107a registration (the shim owns INT-8, like the SB IRQ
pump); (3) confirm/locate the menu SONG source the ISR streams (no music file opens on the boot — the
sequence is engine-fed or built-in; trace be0e/0966 note-posts vs. a real song); (4) fix the +0xfab
instrument-load; (5) THEN calibrate the OPL cadence + phase-pin vs `ref/audio_menu_oracle.wav` and
wavcompare.

**No-regression (VERIFIED both targets):** `make check` = all 355 patches apply; mainmenu native AE=0 +
wasm AE=0, **native↔wasm 0-diff (md5 `3a6ff1c5`)**; default boot rc=0.  All shim additions are FIST_SB/
FIST_OPL-gated (default OFF → the 26 video flows byte-identical by construction) + the fist_icall
0x11917/0x1107a injection only fires when si=0xec installs DGROUP:0xd4/0xf4 (FIST_SB path).  **The wasm
FIST_SB SOUND path was not observed to dispatch** (the menu builds + the watchdog dumps, but no OPL trace
under node) — the sound-path dual-target parity is UNCONFIRMED (a documented follow-up, likely the
call_indirect signature normalization of the sound driver's method vectors); the VIDEO flows stay
byte-identical.  **HONEST: no menu music plays on either target yet.**

## 13. Iteration 7 — FIRST REAL OPL out(0x388) ON THE MENU + fist_opl.c (DOSBox-matched DBOPL) stood up (2026-07-18)

**HEADLINE: the engine now programs the OPL chip on the menu — real `out(0x388)` writes fire (device-3
AdLib init), crash-free, both targets buildable; and `fist_opl.c` + the DOSBox 0.74-3 DBOPL core
synthesize the register stream to PCM/WAV.**  The continuous MUSIC (a note stream) does NOT yet play —
the op=6 sequencer-start is a separate, still-unreached trigger (below), so the OPL output is the
(silent) init state.  Honest: NOT bit-exact music yet, because no notes play.

**THE CORRECTED DEVICE (asm+runtime proven — supersedes §12's "device 4 = OPL"):** the SOUND.CFG music
device is **device 3 = AdLib/OPL** (letter 'C'), NOT device 4 (which is MPU-401 MIDI, writer 0d49 -> port
0x330).  Device 3's 7 methods (104f/10a5/1082/10e3/10a6/0f99/0f48) all call the OPL writer **FUN_0000_0f21
= `out 0x388,AH; out 0x389,AL`**.  Runtime-verified: `byte[DGROUP:0x248]='C'` (the engine SOUND.CFG parse
result) -> 014e code 3.

**WHY IT WAS SILENT (root cause, fixed): the device LETTER was dropped.**  bdcc does `mov al,[DGROUP:0x248];
lcall *0x508` (-> 014e); the `__allregs` indirect-vector dispatch drops AL, so 014e ran with letter 0 ->
device 0 (null) -> no OPL.  Patch 352 threads the letter; 014e selects device 3.

**LANDED (patches 352/353/354; fist_snd.c RE-DECOMPILED with the device-3 seeds -> new reproducible md5
`9b642483` [decomp `fb27726e`], fist.c/ext/mga md5s UNCHANGED `61453e42`/`0051cb56`/`75c6d726`):**
- **352 (engine bdcc)** — thread the dropped SOUND.CFG letter AL=byte[DGROUP:0x248] to shim global
  `g_snd_cfg_letter` (fist_sb.c).  No-op on default boot (c508==0).
- **353 (driver 014e/0872/104f/1082/0f99)** — 014e reads the letter (device 3); the device-SELECT 0872
  (driver-DS rebase) copies device 3's 7 method vectors from the per-device table driver_ds:0x17d.. into
  the live slots + calls slot0 = 104f (OPL init); 104f/1082/0f99 driver-DS rebased -> **the first real
  out(0x388)** (init: reg 0x01=0x20 waveform-enable, 0x08, 0xBD=0xC0, per-channel 0x40/0xB0 regs).
- **354 (driver 10e3/10a6/0aa7/0966/0997/0cfb)** — the device-3 per-voice OPL methods (10e3 key-on/off,
  10a6 frequency) + 0aa7 velocity-scale completion + the note-play chain (0966 dispatch, 0997 note-on asm
  reconstruction, 0cfb freq/env), all driver-DS rebased.  The device-select voice-reset (0c94->0ca9->0aa7
  -> tail `jmp *[ds:0x1a5]` = device-3 slot3 = 10e3) is now crash-free.

**RE-DECOMPILE (approved):** added the device-3 OPL seeds to Makefile `decompile-drivers` snd
`FIST_DRIVER_SEED_OFFS` (0x872,0xa14,0xf21,0xf48,0xf54,0xf99,0x104f,0x1082,0x10a5,0x10a6,0x10e3) -> 89->103
fns, REPRODUCIBLE.  All 3 hard-pristine engine md5s UNCHANGED.

**fist_opl.c + the DBOPL core (new platform shim):**
- **OPL core = DOSBox 0.74-3 DBOPL** (`re_out/opl/dbopl.{cpp,h}`, verbatim; only the DOSBox mixer glue
  `DBOPL::Handler` removed -- the DSP core Chip/Channel/Operator/InitTables/GenerateBlock is byte-for-byte
  the oracle's, since DOSBox 0.74-3 default `oplemu` = DBOPL).  `re_out/fist_opl_dbopl.cpp` = extern "C"
  bridge (Setup/WriteReg/GenerateBlock2).  `re_out/fist_opl.c` = the port shim: trap 0x388/0x389 (routed
  from fist_vga.c `in`/`out`), feed WriteReg, generate mono s16 PCM -> ring + WAV (FIST_AUDIO_WAV), one PIT
  period per engine INT-8 tick (fist_opl_tick).  Gate = FIST_OPL (or FIST_SB); default OFF -> 0x388/0x389
  never trapped -> zero video-flow effect.
- **Builds BOTH targets:** native (g++ -m32 -nostdinc++ -fno-rtti -fno-exceptions -> NO 32-bit libstdc++
  dep, links into the C program with gcc) and wasm (em++).  patch.sh copies `*.cpp` + `opl/` into build/;
  build_native.sh + build.sh compile the C++ units.  Unit-verified: a synthetic AdLib note -> DBOPL peak
  5091, 99.9% non-silent.
- **Rate/mode:** oracle WAV = stereo 44100 16-bit; the game (device 3) never enables OPL3 (opl3Active=0)
  -> mono GenerateBlock2 at 44100 (mono -> stereo-dup at compare time).

**THE REMAINING GAP (the continuous menu MUSIC does not play — op=6 sequencer-start not reached):**
- The engine PLAY dispatch is `FUN_1000_516f -> FUN_1000_50e6(0x4fa,..)` which dispatches op=6 to the sound
  device ONLY if **bit7 of DGROUP:0x4fc ("device present")** is set.  **Runtime probe: `FUN_1000_50e6` is
  NEVER reached on the menu** (traced, 0 hits under FIST_SB) -> the menu never issues the music-START.
- The device-present bit7 is set by the device REGISTRATION (`FUN_1000_1917` DGROUP:0xd4 work-obj alloc +
  `FUN_1000_107a` DGROUP:0xf4 IRQ-register), both UNRECOVERED (§9 blocker 4).  So even locating a 516f
  caller on the menu would gate false.
- **=> the port programs+opens the OPL device (init writes fire) but never starts a note sequence.**  The
  port WAV is silent (peak 0) -- faithful to "device open, no music playing".  Making the menu music
  actually play (and thus a meaningful bit-exact wavcompare vs `ref/audio_menu_oracle.wav`) requires:
  (1) recover 1917/107a so the device registers present (bit7 set); (2) locate the menu music-file load +
  the 516f/op=6 sequencer-start inside 00d0/cae6; (3) then calibrate the OPL sample cadence / phase-pin.
  This is the next iteration's workpackage (a multi-blocker chain, as §9 mapped).

**No-regression (VERIFIED both targets):** `make check` = all 354 patches apply; default boot mainmenu
AE=0 native+wasm, native<->wasm 0-diff (md5 `3a6ff1c5`); default boot rc=0; the OPL shim is FIST_OPL/
FIST_SB-gated (default OFF) + the driver device-3 path is FIST_SB-reachable-only -> the 26 video flows are
byte-identical by construction.

---
## (iterations 1–6, below)

This is a multi-iteration subsystem. **Iter 6 (§12) is a DECISIVE BACKEND CORRECTION: the menu background
music is OPL2/OPL3 FM SYNTHESIS (port 0x388/0x389), NOT SB-DMA digital** — A/B oracle-proven (oplmode=none
-> silent; oplmode=auto -> music, only variable changed).  `fist_sb.c` (SB-DMA -> PCM decoder) is the
WRONG shim for the menu music; bit-exact menu music needs a NEW OPL emulator (`fist_opl.c`).  Iter 6 also
landed the faithful sound-source REGISTER + voice-reset (patches 349/350/351, gated FIST_SB, no-regression)
and re-mapped the driver: device 4 = MPU-401(0x330)+OPL(0x388) MUSIC; device 5 = null; the real SB DSP
(base 0x220) at driver 0x2820 is a SEPARATE DIGITAL device (SFX/voice).  §11's "device 4 = SB / 0d49 =
first SB out" is DEBUNKED (0d49 = the MPU-401 writer 0x330).  See §12 (top).

## 12. Iteration 6 — DECISIVE: the menu music is OPL FM (not SB-DMA); driver re-mapped; register+voice-reset landed (2026-07-18)

**THE DECISIVE FINDING (oracle A/B, airtight — only `oplmode` changed between the two runs):**

| DOSBox config (sbtype=sb16, base 220, irq 7, dma 1) | menu WAV |
|---|---|
| `oplmode=auto` (OPL on) | peak 6710, 96.5% non-silent — **MUSIC** |
| `oplmode=none` (OPL off, SB-DMA fully intact) | peak **0** — **SILENT** |

Removing the OPL synth removes the music; the SB DSP+DMA path (base 0x220) left fully enabled produces
nothing.  **=> the menu background music is OPL2/OPL3 FM SYNTHESIS via port 0x388/0x389, NOT SB-DMA
digital.**  Iteration 2's "menu music IS SB-DMA" was WRONG: it used `sbtype=none`, which in DOSBox
disables the OPL too (oplmode=auto follows sbtype), so it could not distinguish FM from digital.

**Consequence: `fist_sb.c` (SB-DMA -> PCM decode) is the WRONG shim for the menu music.**  Bit-exact menu
music requires a NEW **`fist_opl.c`** — an OPL2/OPL3 register-level emulator fed by the driver's
`out(0x388, reg); out(0x389, val)` writes (the classic AdLib idiom, driver code at 0xf21).  `fist_sb.c`
stays valid for the in-mission DIGITAL SFX/voice path (the real SB DSP at driver 0x2820, base 0x220),
which is a SEPARATE device blocked behind the mission-wasm divergence.

**CORRECTED DRIVER MAP (asm-verified vs re_out/fist_snd_image.bin — supersedes §11's device table):**
- **Device selection is a per-device method-vector table** at driver_ds:0x17d..0x1d1 (7 method slots,
  indexed by `device*2`), copied into the live slots ds:0x17b/0x189/0x197/0x1a5/0x1b3/0x1c1/0x1cf by the
  device-select 088f (asm 0x8c3).  The 7 devices:
  | dev | slot0 | slot1 | slot2 | slot3 | slot4 | slot5 | slot6 | backend |
  |---|---|---|---|---|---|---|---|---|
  | 0 | 0871×7 | | | | | | | **null / default (all `ret`)** |
  | 1 | 1235 | 123f | 1240 | 1247 | 1261 | 127f | 1280 | (0xc0 DMA-port writes) |
  | 2 | 1206 | 11b9 | 1224 | 1163 | 113d | 113c | 11a4 | (0x388 OPL, 0x2000 region) |
  | 3 | 104f | 10a5 | 1082 | 10e3 | 10a6 | 0f99 | 0f48 | (0x388 OPL detect 0xf54) |
  | **4** | **0ea8** | **0dc9** | **0e7b** | **0dca** | **0e13** | **0e53** | **0ef5** | **MPU-401 0x330 (0d49) + OPL 0x388 (0xf21)** |
  | 5 | 128x | | | | | | | **null (all `ret`)** |
- **The real SB DSP (digital) backend is at driver 0x2820** (`out 0x226,1;in;out 0x226,0` = DSP reset at
  base+6 => base 0x220; `out 0x22c,0xd1` speaker-on; DMA via `[0x128d/0x128f/0x1291]` far vectors at the
  0x1d44 segment).  It is reached via the 0x80-command device-select `0a14` (from 00fd), NOT the 0x30
  music path — a DIFFERENT device layer for SFX/voice.  `0d49` = the MPU-401 MIDI writer (`out 0x330`,
  poll `0x331`), NOT the SB DSP writer (§11 mislabelled it).

**LANDED (patches 349/350/351, FIST_SB-gated; 4 pristine md5s UNCHANGED 61453e42/0051cb56/75c6d726/
abcafc9d; mainmenu AE=0 native+wasm, native<->wasm md5 `3a6ff1c5`; about AE=0):**
- **349 (engine be0e/be67)** — thread AX/BX/ES for the sound-source REGISTER indirect call (asm 0xbe3e:
  `ax=word[DGROUP:0x9f2e+id]; xor bx,bx; es=word[DGROUP:0x9f1c]; lcall *0x510`).  The __allregs indirect
  method-vector dispatch dropped them (ES is the unaff_CS/ES class); publish to shim globals
  `g_snd_reg_ax/bx/es` (fist_sb.c).  No-op on the default boot (c510==0).
- **350 (driver 01ec/0af4)** — the register method reads the published AX/BX/ES; 0af4 (asm 0xaf4) copies
  the 0x20-byte sound descriptor from ES:BX into the driver struct with the correct driver-DS rebase
  (DAT_0000_0831<<4 = load_seg+0x2a5, the 0833/347 base).
- **351 (driver 0c94/0ca9/0aa7)** — the all-voices-off tail 0af4->0c94 driver-DS rebase; the 0ca9 loop
  (Ghidra folded bx and ch<<8) reconstructed from asm; 0aa7's per-voice tail `jmp *0x1a5` is a NEAR
  indirect jump through the device per-voice method vector (fist_icall(fist_snd_base+vec)).

**RUNTIME ADVANCE (setarch -R, FIST_SB=1, FIST_SND_CALLTRACE):** the driver now runs the register+voice-
reset crash-free (0af4->0c94->0ca9->0aa7 tail-jumps to slot 0x1a5 = **0x871 = device-0 null method**),
then be0e proceeds to be58 -> DGROUP:0x530=01e7 -> 0833 -> **0966** (the note-PLAY dispatch), which is the
NEXT driver-DS base-loss (asm 0x966: `movw [ds:0x13e2],0x13e8; cmpw [ds:al+0x13e8],0` etc. -> host-
absolute deref).  **Device 0 (null) is currently selected because the device-select 0872 is a fmap MISS**
(never runs), so no OPL device is picked and no 0x388 write can fire yet.

**THE CORRECTED REMAINING CHAIN to the first OPL `out(0x388)` (in dependency order):**
1. **Note-play dispatch 0966/0997/0cfb** (asm-mapped): driver-DS rebase of the note-on allocator (0997
   scans driver_ds:(dev*8+i+0x13b) for a free voice via [0x107], writes [voice+0x111]=record[0], calls
   0cfb(record[1], voice<<8) + 0aa7).  0cfb sets the voice freq/env from tables 0x15b4/0xfc9/0x10d4/
   0x152e/0x14ac and tail-jumps `[ds:0x1c1]` (device method).  All device-independent; for device 0 the
   tails no-op.  (Reaches crash-free FIST_SB once rebased.)
2. **Device-select 0872/088f** — add 0x872 to the driver fmap + reconstruct (asm 0x872: on ds:0x3e==0 ->
   0x8c3 = 088f, which copies device `ds:0x12`'s 7 method vectors from the per-device table into the live
   slots).  088f is in the fmap but driver-DS base-lost (0x17b/0x17d derefs + DAT_1000_c012/c1a5).  This
   makes the live slots point at the selected device's methods.  **Confirm the config selects device 4**
   (the trace shows device 0 default; 014e maps a letter I/T/A/C/R->1/2/3/3/4; verify SOUND.CFG
   "0132710000" -> code 4 = OPL/MPU music device, else document the actual code).
3. **Device-4 OPL methods 0ea8/0e7b/0e13/0e53/0dca** (NOT in the fmap; 0dc9/0ef5 ARE) — re-decompile
   SEED (add these + 0x872/0xa14 to `FIST_DRIVER_SEED_OFFS` in Makefile `decompile-drivers`, re-decompile
   -> fist_snd.c rebaselines from abcafc9d to a new reproducible md5) + driver-DS rebase.  These call the
   in-fmap OPL writer **`FUN_0000_0f21`** (asm 0xf21 = `out 0x388,reg; out 0x389,val`) -> the FIRST REAL
   engine-driven OPL register write.
4. **`fist_opl.c`** — a NEW OPL2/OPL3 emulator (trap 0x388/0x389 in fist_vga.c `out`, synthesize the FM
   voices -> s16 PCM ring/WAV, the OPL analog of fist_sb.c) for bit-exact menu music vs
   `ref/audio_menu_oracle.wav`.  (A vetted reference: the Nuked-OPL3 / DBOPL core; must be deterministic
   + dual-target for the native<->wasm invariant.)
5. **The continuous-music op=6 trigger** (516f->50e6(0x4fa)->driver +0x6, gated on DGROUP:0x4fc bit7) is a
   SEPARATE start path from the be0e per-object register — still unlocated inside 00d0/cae6.

**No-regression (VERIFIED both targets):** `make check` = all 351 patches apply; default boot mainmenu
AE=0 native+wasm, native<->wasm 0-diff (md5 `3a6ff1c5`); about AE=0 native; 4 pristine md5s unchanged.
349/350/351 are reached only under FIST_SB (c510/c530 are populated only by the FIST_SB-gated 014e
section install) -> the 26 video flows are byte-identical by construction.  The FIST_SB path advancing
to the 0966 note-play frontier is the documented WIP frontier (as iters 3-5 left their frontier), not a
video-flow regression.

## 11. Iteration 5 — SOUNDDVR device-config dispatch reconstructed; frontier = the register-path ES drop + un-decompiled device methods (2026-07-18)

**Landed (patches 347/348, FIST_SB-gated → the 26 video flows stay byte-identical; 4 pristine md5s
unchanged 61453e42/0051cb56/75c6d726/abcafc9d; mainmenu AE=0 native+wasm, native↔wasm md5 `3a6ff1c5`):**

- **Patch 347 — `FUN_0000_0833` (the driver's device-METHOD DISPATCHER, asm 0x833) rebased to the driver's
  OWN data segment.**  This is the exact SIGSEGV iter-4 named (`cae6→bdcc→DGROUP:0x508→014e→0833`,
  fault-addr 0x121).  Asm 0x833: `push …; mov ds,cs:[0x831]` (DS = the DRIVER data seg — the word at code
  offset **0x831 is an MZ reloc site** = load_seg+**0x2a5**, so `DAT_0000_0831<<4` is the driver-data
  linear base into g_mem); `mov ds:[0x12],ax` (WORD store of the command word); `bl=(ah>>3)&0xfe`;
  `cx=ds:[bx+0x11b]` (the STATIC method-vector table at driver_ds:0x11b — idx 6 = 0x0872 device-select for
  the 0x30 command from 014e; idx 0x10 = 0x0a14 for the 0x80 command from 00fd); `call cx` (NEAR, driver CS
  → `fist_snd_base+cx`).  Ghidra rendered the vector read as a HOST-absolute deref of (idx+0x11b)=0x121 and
  the store as a 32-bit write to the ENGINE DGROUP 0x1c012.  Fixed both (driver-DS base + WORD width).

- **Patch 348 — `FUN_0000_014e` (the device-CONFIG method, asm 0x14e) section installs rebased.**  The two
  post-dispatch calls `mov bx,cs; mov si,0x22; lcall [ds:0x12]` and `mov si,0x4e; lcall [ds:0x26]` install
  the driver's own method-vector reloc sections snd_seg:0x22 / snd_seg:0x4e into the engine DGROUP (engine
  service appliers 0xf842 / 0xf8d2, both seg 0xf69).  Ghidra rendered them as near host-absolute derefs of
  0x12/0x26 → SIGSEGV fault-addr 0x12.  Modelled faithfully via `fist_apply_reloc_at(driver_seg, si, 1)`
  (identical INSTALL effect; the f8d2 save-back is documented-omitted — its only consumer is the 019a
  stop-sound restore, downstream of the play).  **Section 0x22 installs DGROUP:0x50c=019a / 0x510=01ec /
  0x518=01fd / 0x530=01e7** (leading seg word = the relocated driver load seg); section 0x4e installs
  DGROUP:0x426.

**Runtime-proven advance (setarch -R + `-ftrivial-auto-var-init=pattern` deterministic repro, FIST_SB=1,
FIST_SND_CALLTRACE):** the driver dispatch now runs `+0x2`(init) → `+0x245`/`+0x24b` → `+0x14e`(config) →
`+0x872`(device-select, **fmap MISS → clean no-op**) → `+0x1ec`(sound-source register) before the NEXT
base-loss.  Previously it SIGSEGV'd at the first (0833).

**FRONTIER — the immediate next blocker `FUN_0000_0af4` (the sound-source REGISTER, asm 0xaf4) is an
ENGINE→DRIVER DROPPED-SEGMENT base-loss, not a plain driver base-loss.**  Call path: engine
`FUN_0000_be0e` (asm 0xbe0e) does `xor bx,bx; mov es,ds:[0x9f1c]; call [DGROUP:0x510]` → 01ec → 0af4.  So
0af4 copies a 0x20-byte sound descriptor from **ES:BX with BX=0 and ES = [engine DGROUP:0x9f1c]** into the
driver struct (asm 0xb10: `mov si,bx; add si,0x10; mov al,es:[si]; mov [driver_ds:bx+0x20],al` ×0x10, then
+0x2a/+0x34).  TWO drops: (a) the driver-DS base (rebase-able, like 0833/088f) AND (b) **ES — a segment reg
the `__allregs` model does not thread**; the C `FUN_0000_be0e` dropped both the `xor bx,bx` (→ 0af4 gets a
garbage BX, hence the pattern-init fault-addr 0x8004) and the `es=[0x9f1c]` source segment.  The faithful
fix is an ENGINE patch on `be0e` to thread BX=0 + the source segment [DGROUP:0x9f1c] as an explicit arg
into 01ec/0af4 (the documented `unaff_CS/ES` segment-reg patch class), then a driver-DS rebase of 0af4 —
deferred as a coherent next-iteration workpackage (it opens engine-side sound-descriptor threading).

**THE FULL REMAINING CHAIN TO THE SB `out()` (asm-mapped this iteration, in order):**
1. **`be0e`+`0af4`** (above): engine ES/BX thread + 0af4 driver-DS rebase.
2. **The device-SELECT `FUN_0000_0872` / `088f` (asm 0x872, method vector 0x121) is NOT in the driver
   fmap** (nor are 0x0a14, 0x0ca9, and the per-device method sets).  0872 reads `ds:0x3e` (current device),
   on first call (ds:0x3e==0) falls into 0x8c3 (=088f) which selects device `ds:0x12`'s 7 method vectors
   from the STATIC per-device tables at driver_ds:0x17d..0x1d1 into slots ds:0x17b..0x1cf.  088f (0x8c3) IS
   partly in the fmap but base-lost (its `*(undefined2*)0x17b` derefs + `DAT_1000_c012`/`c03e`/`c1a5` are
   driver-DS, rendered host-absolute / engine-DGROUP).  Reconstruct 0872+088f inline (add 0x872 to the
   fmap array).  **Device index 4 = Sound Blaster** (its method set 0ea8/0dc9/0e7b/0dca/0e13/0e53/0ef5 lives
   in the DSP region 0xd00-0xf90).
3. **The SB device-4 methods `0ea8/0e7b/0e13/0e53/0dca` are NOT in the fmap** (0dc9/0ef5 ARE).  They build
   the DSP command sequences via the in-fmap DSP writer **`FUN_0000_0d49`** (asm 0xd49 = `out dx,al` to the
   configured SB DSP data port — `fist_sb.c` traps this) + `0d66`/`0d90`.  Reconstruct inline → the first
   REAL `out(SBbase,…)` fires here → `fist_sb.c` produces PCM.
4. **The device-register shim helpers `FUN_1000_1917` (DGROUP:0xd4, device work-obj alloc) + `FUN_1000_107a`
   (DGROUP:0xf4, IRQ register) — UNRECOVERED, approved to reconstruct as loader-shim helpers.**  The init
   0078 calls both (asm 0xaa/0xb9); they set the device-present state.  The play GATE (doc §9) is bit7 of
   `DGROUP:0x4fc` checked by `FUN_1000_50e6`; the device register must set it.
5. **The engine op=6 PLAY trigger inside `FUN_0000_00d0→cae6`** (doc §9: `516f→50e6(0x4fa)→driver +0x6`) —
   still unlocated; needed to START continuous SB-DMA streaming (bdcc/014e only device-CONFIG, not play).

**No-regression (VERIFIED):** `make check` = all 348 patches apply; default boot mainmenu AE=0 native+wasm,
native↔wasm 0-diff (md5 `3a6ff1c5`); 4 pristine md5s unchanged.  347/348 are reached only under FIST_SB
(the DGROUP:0x508 config vector is FIST_SB-installed) → the 26 video flows are byte-identical by
construction.  `fist_sb.c` + the harness remain ready to stream the moment step 3 fires the DSP `out()`.

## 10. Iteration 4 — MEMMGR pool-init + near reloc appliers LANDED (crash-free, no-regression) (2026-07-18)

**Landed (patches 345/346 + shim, UNGATED core boot infra; the 26 video flows stay byte-identical):**
- **Patch 345 — `FUN_1000_11e0` (MEMMGR object-pool init, asm 0x111e0) reconstructed.** It builds two
  2-node sentinel lists per pool struct; each node is unlinked from the free-node list (head DGROUP:0x16e2,
  count 0x16dc) by `FUN_1000_1099` (0x11099) which RETURNS the node seg IN DS.  Ghidra dropped that DS
  reassignment → every field store landed at DGROUP:0..0xe (DAT_1000_c000..c00e), which (a) never built the
  pools and (b) node-B's `[0]=0xffff` CLOBBERED the 0xcbc3 signature at DGROUP:0 AND (c) overwrote the near
  reloc-applier service vectors DGROUP:0x0a/0x0e (f7ef/f81d).  Reconstructed with an inline node-alloc
  (mirrors 1099 exactly; leaves the shared base-lost 1099 untouched — it is resolved per-caller) writing
  every field to the REAL node seg.  **Runtime-proven:** `sig[0]=cbc3` preserved (was 0xffff), pools
  0x16f6/0x1718 built (`[+a]/[+c]` valid node segs, `[+0x10]`=2), free-count consumed, service vectors
  `0x0a=015f:0f69`/`0x0e=018d:0f69` intact.
- **Patch 346 — thread the 5 near reloc-applier install sites + 5 clear sites.**  Keeping DGROUP:0x0a live
  (patch 345) would fire the base-lost `FUN_0000_f7ef` on garbage SI → the documented `FUN_0000_1384`
  SIGSEGV.  Each install site `xor bx,bx; mov si,<off>; lcall [ds:0x0a]` (0x1384 si=0x2c4, 0x1c78 si=0x378,
  0x216d si=0x39c, 0xfa7c si=0x8c, 0x14b24 si=0x258) → `fist_apply_reloc_section(si,0)`; each matched clear
  `lcall [ds:0x0e]` (f81d) → `fist_clear_reloc_section(si)`.  The 4b16 (patch 022) stray live `lcall [0x0a]`
  is dropped (its correct 0x258 install already present).  **Runtime-proven:** near vectors install
  (`DGROUP:0x3e8=139e`, `0x306=026a`).
- **Shim (`tools/native_main.c`):** fixed the NEAR path of `fist_apply_reloc_section` — it never skipped
  the near section's leading base word, so `fist_apply_reloc_section(si,0)` read the leading 0x0000 as the
  first `off` and terminated at 0 entries (silently inert — this is why patch 022's `(0x258,0)` had been a
  no-op the whole time).  Now skips it (addend=base<<4=0 in base-0).  Added `fist_clear_reloc_section` (the
  f81d near-clear) + a `FIST_MEMPROBE` diagnostic.  Asm-verified vs f7ef 0xf7fa..0xf815 / f81d
  0xf828..0xf83a.

**No-regression (VERIFIED both targets):** native 26/26 flows AE=0 crash-free (campaign-missions base==fix
0-diff); wasm 14 flows AE=0, native↔wasm 0-diff (mainmenu/about/settings/selplayer/battles/campaigns/intro/
review/settings-sky/battles-select/campaigns-select/battles-cancel/battles-cancel-briefing/selplayer-ok);
default boot 5/5 rc=0, mainmenu md5 `3a6ff1c5`; 4 pristine md5s unchanged
(`61453e42`/`0051cb56`/`75c6d726`/`abcafc9d`); `make check` = all patches apply; tree clean.

**Device-registration now RUNS (gated FIST_SB) — new frontier, NO PCM yet.**  With the signature preserved,
patch 344's device-register guard (`sig==0xcbc3`) now PASSES (was failing → 344 was inert), so it installs
the sound method-vector section A (DGROUP:0x508=0x14e:snd_seg, …).  On the DEFAULT boot (FIST_SB off) 344
does not run → DGROUP:0x508=0 → `bdcc`'s dispatch traps → no-op → clean.  Under **FIST_SB=1** the engine
menu build `FUN_0000_cae6 → FUN_0000_bdcc` dispatches DGROUP:0x508 → **`m_snd_FUN_0000_014e → 0833`, which
SIGSEGVs on a SOUNDDVR driver base-loss** (asm 0x833: `mov ds,cs:0x831` = the driver's own data seg, then
`call [driver_ds:bx+0x11b]` near-dispatch; Ghidra rendered `ds` as the ENGINE DGROUP → `[0x121]` host
deref).  This is the SOUNDDVR driver base-loss cascade (MGAVIDEO-031-072 class), the next (gated) work — it
is UPSTREAM of / independent from the doc's blocker 4 (si=0xec install + 1917/107a), which were not driven
this iteration because the boot faults at 014e first (untestable end-to-end until the driver play path is
reconstructed).  `fist_sb.c` + harness remain ready to stream once the driver programs the SB.

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
