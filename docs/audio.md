# AUDIO subsystem — recon + shim foundation (iterations 1–5)

Status date: 2026-07-18. Author task: stand up the first port audio output + the audio-verify method.
This is a multi-iteration subsystem. Iter 5 (§11) advanced the SOUNDDVR driver dispatch two methods
deeper under FIST_SB (the device-config method 014e + its dispatcher 0833 are asm-exactly reconstructed;
the menu SIGSEGV moves from 0833 to the sound-source REGISTER method 0af4) — crash-free and NO-REGRESSION
to the 26 video flows on both targets. Still no PCM: the register path 0af4 needs an engine-side DROPPED
SEGMENT (ES = the sound-descriptor source seg) and the device-select + SB device methods are not in the
driver fmap. The exact remaining chain to the SB `out()` is mapped below.

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
