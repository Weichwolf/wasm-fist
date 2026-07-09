# Armored Fist (FIST.RUN) → WASM

## Goal
A faithful 1:1 rebuild of NovaLogic's **Armored Fist** (1994, `FIST.RUN`) with **bit-identical audio and
video output**: given the same input and RNG state, the rebuild produces the same 320×200 palette
framebuffer every frame and the same audio stream as the original under DOS, native and in the browser
(WASM). The route there is a reproducible chain that always starts with `make` (`make image → decompile →
assemble → patch → native / wasm`), so the whole pipeline binary → decompile → patches → build is implicitly
documented by the Make targets. Every correction to the decompile is a commented, traceable patch.
**No guards, no approximations, no band-aids** — any observable deviation from the original is by definition
a bug. **Code is the truth** — verify every claim against `armoredfist/FIST.RUN` or a DOSBox run.

**Completeness bar = the DD2 standard: a 100%-exhaustively-tested port, nothing stubbed or skipped.** The
target is not "boots and looks right" — it is *every* reachable surface exercised end-to-end and bit-verified
on **both** targets (native + browser), the way `~/Git/wasm-dd2/` was driven through many test/fix iterations
across every menu, every level, and every setting. In scope, all of it:
- **Every menu / screen / dialog** — intro, main menu, mission select, briefing, options, sound/graphics
  detail (`*.DTL`: LOW/MEDIUM/HIGH), controls config, save/load — each renders, navigates, and acts correctly.
- **Every mission / level** — the full campaign/map set (`C##.KLC`/`D##.KLC` colormap+heightmap pairs,
  `*.MS3` missions, `*.FPL` paths, `*.CAM` cameras) plays through, not just one demo map.
- **Every setting / option** — detail levels, sound devices (SB/GUS), input modes (keyboard/joystick),
  serial link — each toggles and takes effect.
- **The in-engine LEVEL / MISSION EDITOR** — Armored Fist ships a built-in editor (part of `FIST.RUN`; there
  is no separate editor `.EXE` in this copy — the `.KLC`/`.MS3`/`.FPL` files are its data). It is **in scope
  as a first-class deliverable**: the editor UI must open, render, and edit on both targets, and the
  **round-trip must be bit-verified** — create/edit a map, save it, reload it, and sim it, byte-identical to
  the original editor's output. Locating the editor's entry point (command-line switch / hidden key / menu
  item) is a Phase-0 recon task.

This project reuses the *methodology* of the sibling project `~/Git/wasm-dd2/` (Destruction Derby 2 → WASM)
but **shares no code** — DD2 is a clean Win32 PE (DirectDraw/DirectSound/COM); Armored Fist is a 32-bit DOS
protected-mode program. The doctrine transfers; the platform layer and Ghidra front-end are new. Read DD2's
`CLAUDE.md` for the patch-series discipline and the Ghidra-artifact taxonomy — both apply here verbatim.

**Doctrine — "geht nicht gibt es nicht" / there is no can't.** The full chain (binary → decompile →
patch-series → native + wasm → bit-identical + exhaustively tested) is **already proven end-to-end by
wasm-dd2**. This is not research with an open feasibility question — it is execution of a known-good method
on an *easier* target: a DOS game has **no COM/Win32/DirectDraw/DirectSound layer**, graphics are one linear
320×200×8bpp framebuffer, and the whole thing runs deterministically under DOSBox/QEMU. So anything that
looks "harder than DD2" (bound-format reversing, port/IRQ-level shim, real-/protected-mode mix) is **bounded,
tractable work to be decomposed and finished — never a reason to stub, approximate, or declare a surface
out of reach.** "Can't be done" is not an acceptable conclusion; the only question is the next step.

## What the target actually is (recon findings — do NOT re-derive)
**Two binaries — the engine is FIST.DAT, not FIST.RUN (verified; full detail in `docs/recon.md`):**
- **`armoredfist/FIST.RUN`** = the **loader/runtime kernel**, NOT the engine: a stub MZ (DPMI probe) + the
  **Doug-Huffman DOS extender** (`DOS extender Copyright 1991-1994 by Doug Huffman`) wrapping a ~4 KB
  self-decrypting 32-bit protected-mode kernel (CRT + `FILEMGR` + `MEMMGR`). It enters flat 32-bit mode and
  loads/execs the game from FIST.DAT. Launched by `LOADGAME.EXE` (NovaLogic loader v1.14, John Butrovich,
  LZEXE-packed) via `LOADGAME -K400,0,1000 -X5000 FIST.RUN`. Its role → hand-written shim (`fist_dos.c`),
  not engine source. Decompile kept as `re_out/fist_kernel_decomp.c`. Family strings: `COMANCHE`,
  `WOLFPACK.EXE`, `ULTRA` (GUS), `BLAST` (Sound Blaster), `SERIAL`.
- **`armoredfist/FIST.DAT`** (216 KB) = **THE ENGINE** and the decompilation target. A single MZ,
  **16-bit segmented** code (Ghidra `x86:LE:16 Real Mode`, flat seg<<4, base 0, entry linear 0x4), 1143
  relocs. **Almost entirely hand-written assembly** by **Kyle Freeman** (2242/2245 functions frameless) —
  NovaLogic **Voxel Space** terrain engine (Comanche: Maximum Overkill lineage): voxel-column raycaster
  over heightmap+colormap, hand-optimised fixed-point asm. The algorithm is publicly documented — use it as
  a semantic oracle for the hotspots. Because it is hand-asm, register dataflow is the inter-function
  interface (resolved at decompile time via the `__allregs` threading — see Status).
- **Graphics** = VGA Mode 13h / Mode-X, **320×200×8bpp palette**, DAC via ports 0x3C8/0x3C9. This is a
  single linear framebuffer → far simpler to mirror to a canvas than DD2's 640×480 DirectDraw surfaces.
- **Audio** = Sound Blaster / Gravis UltraSound via DMA + IRQ (drivers `SOUNDDVR.DVR`, config `SOUND.CFG`,
  `SOUNDSET.EXE`). **Input, three devices (all verified in FIST.DAT):** (a) **keyboard** (INT 9 / port
  0x60); (b) **joystick** — full flight-sim rig: `JOYMGR`, `JOYSTICK.CFG`, calibration for throttle / hat
  switch / pedals-rudder — for vehicle/combat control; (c) **mouse via INT 33h** — the game *requires* a
  mouse (`"ARMORED FIST REQUIRES A MOUSE"`); 6 INT-33h sites clustered at FIST.DAT 0x14d37–0x14dc8 drive
  the **map view / mission editor** (tools `ADD WAYPOINT`, `CUT UNIT OR PATH POINT`, `TANK ADDED`). Plus a
  `SERIAL` link. **Timing** = PIT (INT 8). All port/IRQ/INT-level, emulated in the shim — not a clean API.
- **Data** (`armoredfist/FISTDATA/`, 403 files): `*.M00/M08/M16/M32` = LOD 3D models, `*.MAL` = model
  palettes/aux, `*.SKY` skies, `*.CAM` camera paths, `*.FSG/FSE/FSW` sounds, `*.MRL`, `*.KLC` (colormap/
  heightmap map pairs `C##`/`D##`), `*.BIN`, `*.RES`, `*.MS3` missions, `*.FPL` paths, `*.DTL` detail.
  The engine (FIST.DAT) reads these at runtime via `FILEMGR` → a DOS file-I/O shim (INT 21h open/read/seek),
  analog to DD2's `dd2_filio.c`. Original data files live under `armoredfist/` and must be present at run
  time. (`FIST.DAT` itself is the engine binary, not a data container — see above.)

## Pipeline — everything starts with `make` (the target chain IS the documentation; `make help`)
`make image → decompile → assemble → patch → native/wasm → verify/verify-wasm`. Front/back ends are
DOS-specific. State: image + decompile DONE and reproducible; assemble onward is the road ahead.
- **`make image`** [DONE] — `tools/extract_dat_image.py` statically loads the FIST.DAT MZ (drop 5120 B
  header, apply 1143 relocs, base 0) → `re_out/fist_dat_image.bin` (211212 B, 16-bit, entry 0x4).
  `make kernel-image` = the FIST.RUN kernel image (`tools/extract_image.py`, shim reference).
  **`make image-drivers`** [DONE] — same extractor (parameterized `[SRC [OUT [--relocs]]]`) on the two
  runtime driver OVERLAYS → `re_out/fist_mga_image.bin` (MGAVIDEO.DVR, 31388 B/9 relocs) +
  `fist_snd_image.bin` (SOUNDDVR.DVR, 16700 B/21 relocs) + `.relocs` sidecars. The engine loads these at
  runtime via INT 21h AH=4B AL=03; the .DVR reloc factor is applied at LOAD time by the shim, not baked.
- **`make decompile`** [DONE] — Ghidra headless (`x86:LE:16 Real Mode` @ base 0, `tools/decompile.sh`) →
  pristine `re_out/fist_decomp.c` (2245 fns). preScripts `PrepAnalysis` (aggressive analyzers +
  `DS=0x1c00`/`SS` segment context) + `MarkEntry`; postScripts `RecoverAll` (jump-table/prologue/call
  discovery to a fixpoint — 71 tables) + `ApplyConv` (custom **`__allregs`** prototype model threading GP
  registers as explicit C params/returns — killed 96% of register pseudo-vars) + `ExportDecomp`. Ghidra +
  JDK are reused from wasm-dd2 via `third_party/` symlinks (gitignored).
  **`make decompile-drivers`** [DONE] — the SAME pipeline per driver overlay (`FIST_PROJ_NAME`,
  `FIST_ENTRY=0x0`, `FIST_ENGINE_SEEDERS=0` gates the engine-only SeedServiceVecs/SeedRuntimeVecs,
  `GHIDRA_FIST_OUT` redirects export) → `re_out/fist_mga_decomp.c` (233 fns) + `fist_snd_decomp.c` (44).
- **`make assemble`** [next] — mechanical `fist_decomp.c` → `re_out/fist.c` (game-code selection, fwd
  decls, flat `g_mem[]` model, `ghidra_compat.h` for `swi()`/`in`/`out`/16-bit types). `tools/assemble_fist.py`.
- **`make patch`** — ordered `patches/NNN-slug.diff` onto `re_out/` → `build/` with `-F0 --fuzz=0` (drift
  fails loudly). **Never hand-edit the decompiled engine source.** Known residual patch classes from the
  decompile: segment regs (`unaff_CS/ES`), flags/CF (no Ghidra flag-convention), `extraout_` multi-return
  (e.g. INT 33h CX/DX), FPU `ST0-7`, 16-bit wraparound where bit-identity needs `uint16_t`.
- **`make native`** — gcc `-m32`. **`make wasm`** — emcc (emsdk). **`make web`** — browser. **`make verify`
  / `verify-wasm`** — crash-free matrix on both targets.

Only hand-written code = the platform/runtime shim (`fist_` prefix): `fist_vga.c` (Mode 13h/Mode-X + DAC →
framebuffer, i.e. writes to linear 0xA0000), `fist_dos.c` (extender/kernel role: INT 21h/DPMI/DOS memory +
`FILEMGR`/`MEMMGR`; incl. **INT 21h AH=4B AL=03 load-overlay** = a faithful 16-bit MZ overlay loader),
`fist_icall.c` (indirect-call dispatcher + **multi-module overlay registry/dispatch**), `fist_modules.c`
(driver-overlay→fmap wiring, weak so engine-only builds link), `fist_sb.c` (SB/GUS DMA+IRQ → WebAudio),
`fist_input.c` (INT 33h mouse + INT 9 keyboard + joystick), `fist_filio.c`, `fist_runtime.c`,
`tools/native_main.c`. Editing those is fine; everything else is decompile + patches.

**MULTI-MODULE STRUCTURE (driver overlays).** The engine (`fist.c`) is one translation unit; each runtime
driver overlay (MGAVIDEO.DVR video, SOUNDDVR.DVR sound) is a SEPARATE unit (`fist_mga.c`/`fist_snd.c`)
assembled by the same `tools/assemble_fist.py` in `FIST_MODULE` mode — base-relative accessors that add a
per-module `fist_<mod>_base` (set at load), a base-0-module-offset-keyed `fist_<mod>_fmap`, and
`#define`-namespaced (`m_<mod>_…`) symbols so they don't collide with the engine unit. `fist_dos.c` AH=4B
loads the real .DVR at the engine-chosen `load_seg<<4`, applies its MZ relocs, and registers it;
`fist_icall` maps a runtime call target inside an overlay range to `linear-load_base = module offset →
module fmap → &FUN`. Validated end-to-end via `FIST_OVL_SELFTEST=1`. See `docs/stage1.md`.

**DGROUP INDIRECT-CALL VECTOR TABLE (the runtime method-vtable install).** The engine's DGROUP holds a
large table of far/near indirect-call vectors (service dispatch 0x0a..0x36 → seg 0xf69; device/stream
METHOD vectors 0x78..0x33e incl. the driver-filename builder at DGROUP:0x31a → FUN_1000_4c9a). These are
wired at runtime from an **in-image relocation table at seg 0x3352:0 (linear 0x33520)** — sections of
`(dgroup_off, code_off)` pairs applied by **FUN_0000_f7ef** (near) / **FUN_0000_f842** (far off:seg; ==
the DGROUP:0x12 service). f738 installs the boot sections at CRT init; each subsystem installs its own
section on init via `mov si,<off>; call far [DGROUP:0x0a|0x12]`. Ghidra dropped the string-op segment
bases in the C f7ef/f842 (DS=table-seg, ES=DGROUP) AND the SI arg of the indirect service calls, so the
sections don't land → the method slots stay 0 → their `call [ds:0xNN]` trap to linear 0. Loader-role shim
`fist_apply_reloc_section(si, is_far)` applies a section faithfully from the game's own data (base-0 ⇒
identity), called from patches at the exact engine install sites (timing is load-bearing). Decoder
validated slot-for-slot vs `tools/oracle/samples/dgroup_0x0_0x100.bin`. See `docs/stage1.md`.
**Indirect-call args are threaded PER-PATCH, not by Ghidra pointer-typing.** Typing a `(*DAT_...)()`
vector slot as a pointer-to-`__allregs`-FunctionDefinition does NOT work: Ghidra assigns FunctionDefinition
param storage from the model's fixed pentry order (AX,BX,CX,…), but `ApplyConv` commits each real target's
params in per-function varnode order (non-canonical register→slot map), so a generic pointer type emits
args in the wrong positional order → flat-C cdecl corruption. DIRECT calls thread args correctly (the
decompiler matches the known callee sig); INDIRECT method-vector sites are threaded faithfully per-patch as
they reach the executed path. **Stage-1 gate CLEARED (patches 025–030, 023-rev): the MEMMGR best-fit
allocator FUN_1000_0a31 + its interlocking callees (161d refill, 0bd1/0bef/0c21/0c7f search helpers, 0d70
unlink, 1110/1114 coalesce, 17e5, 184b) are faithfully reconstructed from asm and verified (valid segments
≥ heap base, base-sorted free list, no cycles, pool spans the full DOS block).** Execution now runs past
MEMMGR + the 14da/197b object-pool init into the 0x5200 driver-init subsystem. **TASK 2 COMPLETE (patches
031/032/033 + the 030 is_far fix): BOTH .DVRs now LOAD on the real boot path** — the driver-object method
vector section si=0xec is installed at FUN_1000_06e3 (DGROUP:0xd0 = 0xf69:0x21bb = 184b), 50c8's dropped
file-size return is restored, and FUN_1000_5051's DGROUP:0xd0->184b allocator call is threaded so it writes
the real load_seg into the driver struct (SOUNDDVR seg 0x3a06 / MGAVIDEO seg 0x3e3a, both fmap-wired). The
"0x5200 cascade" (5228/524e/…) was ROOT-CAUSED as the video-DRIVER dispatch surface (DGROUP:0x736/0x71a/
0x558/0x680 are set by the driver's OWN init, not the engine), so it null-derefed only because the driver
never loaded. The engine now dispatches INTO the video driver (new **SeedDriverVecs.java** seeds the driver
overlay entry vector 0x2/init 0x9/methods incl. the VGA mode-set FUN_0000_00e8 = INT 10h AX=13h; video
driver 233->236 fns). **FIRST LIGHT (mode-set half) ACHIEVED — INT 10h AX=13h FIRES.** The driver-init
template-copy cascade (FUN_0000_f98b + the 5 template copies + workspace allocs + the driver-local
method-vector reloc apply) is fully threaded and executing, the VGA mode-set FUN_0000_00e8 runs to its
`INT 10h AX=0x13` (`[vid] 10h set mode 0x13`), and execution then runs through the engine's 0x5200
driver-processing (FUN_1000_5228/524e/52d1/530b/531b) + the post-driver free-list (3436/3446) into INT 33h
mouse init and joystick calibration. New shim helpers in `tools/native_main.c`:
`fist_apply_reloc_at(seg,si,is_far)` (the driver-LOCAL reloc apply — `mov bx,[DGROUP:0x70e]; lcall
[DGROUP:0x12]` — installs the driver method vectors incl. the mode-set DGROUP:0x540=0xe8:load_seg) and a
`FIST_RUNMS` FB-dump watchdog.
**LATEST (57 patches, 046-059 new): ALL 94 MGAVIDEO method-vector offsets recovered + the full
mode-set→palette→driver-dispatch→cursor-init path reconstructed.** Runtime-dumped the DGROUP for every
driver far vector (`FIST_DUMP_VECS=1` in `fist_icall.c`), seeded all 94 into `FIST_DRIVER_SEED_OFFS`,
re-decompiled MGAVIDEO (216 fns) → the 3 documented driver-method traps (+0x3e0/0x26e/0x1963) are GONE.
Then reconstructed, asm-verified: the palette upload (046-049: incl. resolving the **uninstalled** device
alloc DGROUP:0xcc=182a directly to allocate the palette buffer word[0x782]), the post-mode-set driver
object dispatch (050-053: 5228/026e/1963 + the 3446 f842-no-op elision), and the INT-33h mouse-cursor
init walkers (054-059: 1cdb/1d59/201a/209e/2108/2d6d — DS=word[0x3e02] sprite-list segment, DGROUP
near-offset marks, the 0xc-byte stride, node-list dispatch vectors). 2249 engine fns pristine, no
regression (both .DVR load + mode-set + mouse init intact).
**LATEST (63 patches, 060-065 new): CURSOR CASCADE CLEARED — FUN_0000_1c6a RETURNS; the driver rect-fill
pixel-writer FUN_0000_1091 reconstructed; execution runs through filename-init + MEMMGR-search into the
MAIN function FUN_0000_00d0.** Reconstructed & asm-verified: **1091** the literal VGA pixel writer (patch
060/061 — Ghidra dropped its whole fill body "does not return"; rebuilt `di=rowtab[y0]+x0; ES=0xA000; per
row rep stosw/stosb`, threaded an explicit colour param; erase 2108 threads colour 0); the cursor-DRAW
handlers **2112/211d/2123/212a** (062 — dispatched by REGISTER, recover bx=word[DGROUP:0x3e08]/bp=word[
0x3afe] + rebase node reads to DGROUP; 212a also restores the dropped INPUT ax to the DGROUP:0x584 driver
method); the **filename-init** 21bd (064 — thread ax=0x148 + rebase param_2) + driver string-table copy
0197 (063 — DS=word[0x70a]/ES=DGROUP bases); the **MEMMGR object-search** 135f/1345/18d1 (065 — 135f's
folded segment-walk was an infinite loop; restored the chain walk + CF/found propagation). 2249 engine fns
pristine, driver 216, no regression. **NONZERO PIXELS not yet reached** (gdb-verified FB at 0xA0000 =
0/64000): the cursor erase fills colour 0 and the first real render is deeper in 00d0. **ENV-CONFIG
HANDSHAKE RESOLVED (patches 066/067 + env/memory seeds): d99b's blob is an EXTENDER callback vector, not a
LOADGAME env var.** The `IBM=`/`ATCODE=` premise is DEBUNKED — those are LZEXE-0.91 decompressor artifacts
in LOADGAME.EXE (unpacked + verified); LOADGAME sets NO env var. The nibble blob d99b decodes is
synthesized by the **Doug-Huffman EXTENDER (FIST.RUN)**, which rewrites `PSP:0x2c`. Captured live from the
original under QEMU (`pmemsave` low RAM + `[\x30-\x3f]{12,}` grep, three copies): **`06=90762117900000?300526`**
→ decode+swap → far ptr [ea16]=**0x0762:0x1179 (linear 0x8799)**, args ea1a=0x0f30 ea1c=0 ea1e=0x0526.
Disassembling that target in the oracle's memory = the extender's **real-mode→32-bit-PM callback gate**
(push segs; `rep movsl` args; stack-switch; far-jmp to PM) — so `e339`'s far-call is an EXTENDER SERVICE
call (in our port the shim/extender role → `fist_icall_far(0x8799)` honest trap→0). Seeded the real blob at
`PSP:0x2c` + the cb45 memory-gate fields (conv/XMS/disk, 32MB ref machine → cb45 passes, 0xf9fa/0x314 traps
GONE); reconstructed d99b faithfully (builds the intro/title task: D02.PCX/C02.PCX/502.PAL/5.SKY) + fixed
e339 base-loss. Execution now runs past cb45 + d99b into **FUN_0000_db3f**.
**LATEST (70 patches, 068-072 new): EXTENDER CREATE-TASK GATE IMPLEMENTED → the intro task is a REAL
segment; the banner prints; execution reaches the INTRO-ANIMATION PLAYER.** The 0x8799 gate now routes
(via `fist_icall`) to **`fist_extender_gate()`** (`tools/native_main.c`, extender role): op = DGROUP:0xea10
(aa10) selects the service; op 0 = create-task = allocate a zeroed, paragraph-aligned, real-mode task-
control block of word[ea1a]=0x0f30 BYTES from the extender pool (free gap seg 0x9000..0x9800, above the
MEMMGR heap-top, below the PSP) and return its segment in AX (e339 → aa2e). The PM service body lives in
the extender's paged image (0x10000000+, unreachable by the oracle which faults earlier), so it is modelled
faithfully from the asm-verified contract (0xe339: gate return AX = task seg; the `ljmp [DGROUP:0x58]`
scheduler tail is guarded by task[0]!=0 which stays 0). Then: **068** rebases the whole task-struct base-
loss family ONCE at the `DAT_2000_aa2c` macro (byte-ptr `g_mem+(word[aa2e]<<4)+word[aa2c]` — fixes ~88
db3f/dab0/db11/dde2/… sites' base-loss + int*×4 scaling; zero regression, no pre-db3f user); **069**
FUN_0000_153c resource-directory registrar (object struct at param_2:param_4 seg:offset; descriptor-chain
parse into word[DGROUP:0x4f0]:out_off); **070** MGAVIDEO driver 0x4a3 palette-clear + vblank DAC-upload
service (ES=word[DGROUP:0x782] base + service the 0x786 retrace semaphore); **071** FUN_0000_e584 intro-
animation-player header (resource descriptor at STRSEG:bx, STRSEG=word[DGROUP:0x70]); **072** cooperative
INT-8 pump in e584's frame-timer waits + FUN_1000_38cc (same class as 017).
**LATEST (74 patches, 073-076 new; engine re-decompiled 2249→2251): VSYNC HANG RESOLVED — the frame/event
SCHEDULER is installed + reconstructed, FUN_1000_38cc no longer spins, and the INTRO ANIMATION NOW PLAYS
end-to-end (frames advance 0→573, real-time throttled).** The prior "driver vsync-poll c40a" premise was
WRONG: **c40a is the ENGINE's frame/event SCHEDULER poll, not a driver method** — `DGROUP:0x40a = 0xf69:0x3f17
= FUN_1000_35a7` (a seg-0xf69 service fn).  Reloc **section si=0x1d8** (leading seg 0xf69) installs the
scheduler method vectors 0x3fe..0x41e (incl. 0x40a poll + 0x416=38cc); same KIND as the 0x0a..0x36 service
vectors → installed at BOOT (loader role: `fist_install_dgroup → fist_apply_reloc_section(0x1d8,1)`).  The
engine's own 0x1d8 sites don't install it: **FUN_1000_3446 (sched INIT) calls f842/bx==0 which BAILS;
FUN_1000_3485 (sched SHUTDOWN) calls f860 which CLEARS.** Seeded the nine vector targets into
`SeedServiceVecs.java` + re-decompiled (RecoverAll pulled in 0x35a7/0x349b/0x38f5/0x3920). **073** reconstructs
FUN_1000_35a7 (event-queue pump; DGROUP near-offset base-loss; returns x86 CF via the shim global `g_fist_cf`
— CF=1 when the queue is empty [0x18e4]==0xffff) + FUN_1000_3920 (cooperative-yield trampoline).  **074/076**
thread that CF into FUN_1000_38cc/38bc (Ghidra folded both CF spins to hardcoded-false → infinite). **075**
reconstructs the FUN_0000_e584 intro SCRIPT interpreter (6-byte records at ES:[b6ec], ES=STRSEG=word[DGROUP:
0x70]; STRSEG base-loss + uint*×3 cursor scaling + 38bc-CF gate).
**LATEST (75 patches, 077 new): EXTENDER TASK MODEL FULLY REVERSED + a HARD ARCHITECTURAL FINDING — the
per-frame RENDERER is Doug-Huffman-extender 32-bit-PM code that is NOT in FIST.DAT.** Reversed the whole
model slot-for-slot (asm 0xd99b/0xe2c2/0xe339 + reloc-table decode): the TCB command inbox is **task+0x3f2**
(EBX, written by ~40 posters 0xd94e..0xe37b), params +0x490/+0x492/+0x494, asset names +0x7a/+0x8a/+0x9a/
+0xaa, current name +0xba; ops = 0x00 create / 0x04-0x70 display-list setup / **0x64 post frame DATA** /
**0x78 PRESENT frame N** (FUN_0000_d97e; its return gates e584's loop). **task[0] is an ERROR/ABORT flag,
not "started"**: e339's `ljmp [DGROUP:0x58]` (only if task[0]!=0 && op!=0) → **0xf69:0x314 = the extender
ABORT/RESTART handler** (`ljmp 0:0xe0`), so the shim's no-op-return-0 (task[0] stays 0) is the faithful
non-error path.  **THE FINDING (proven by full-image scan, every encoding): task+0x3f2 is WRITTEN at 34
sites and READ by NOTHING in FIST.DAT or either .DVR.** The consumer that reads the inbox, decodes the
display list, streams TITLE.KDV (1.8 MB FMV), and blits to 0xA0000 is the extender's PM service — **absent
from FIST.DAT but PRESENT in the FIST.RUN extender kernel image `re_out/fist_image.bin` (Route 1, the
faithful path; NO DOSBox trace needed).**  **LOCATED + DECOMPILED this iteration** (`make decompile-kernel`,
new target; 33 → 183 fns): the **KDV player** cluster (decode `0x6f3e` dispatches chunk MAGIC `'xVDK'`header/
`'pVDK'`palette/`'iVDK'`frame; present `0x7120` = `rep movsd` 320×200 → framebuffer var `[0x917]`=0xA0000;
palette `0x746b` → DAC 0x3c8/0x3c9; RLE `0x7135`; open/read/close = INT 21h) and the **op-service dispatch**
(handlers `0x11cb` KDV-open reads TITLE.KDV from **TCB+0xBA** via `[0xc93]`=current-TCB, `0x11dd` decode;
op/method-vector table image 0xcef..0xd2b).  Integration = a THIRD, **32-bit-flat** assemble-module unit
(`fist_ext.c`): its own image rebased to `fist_ext_base`; register-indirect flat pointers → `g_mem[reg]`
(so framebuffer 0xA0000 + TCB 0x90000 resolve absolute; `[0x807]`=0 identity); wire `fist_extender_gate` op→
`m_ext_FUN_000011cb/11dd`.  DESIGNED + decompiled; module plumbing (a new 32-bit assemble mode) is the
next build step (full detail in `docs/stage1.md`).
**LATEST (this iteration): ROUTE 1 PLUMBING BUILT — the KDV player is a compiled+linked+wired 3rd MODULE;
execution runs from the engine's intro loop INTO the real decompiled player.** New **32-bit-flat MODULE mode**
in `assemble_fist.py` (`FIST_MODULE=ext FIST_FLAT32=1`, gated → engine byte-identical): normalizes the single
flat-address symbols to `0000:OFF`, defaults scalars to 4 B, handles `sRam`/`PTR_*`/`code`-return/`switchD::`/
switch-on-pointer/`BADSPACEBASE`/bare-LAB-SMC → `re_out/fist_ext.c` (183 fns) compiles+links. **Memory model =
host-pointers-in-slots** (the extender is a flat bump allocator: seed `[0x90b]/[0x90f]` heap, `[0x917]`
framebuffer, `[0xc93]` TCB with HOST pointers → the REAL allocator/RLE/present/DAC run with native derefs, no
per-deref rewrite). Wired: `build_native.sh` `ext` unit, `fist_modules.c` `"EXT"` row, `native_main
ext_module_init` (load `fist_image.bin`@`FIST_EXT_BASE=0x100000`, `fist_ovl_register`, seeds; g_mem grown to
0x180000), gate op 0x64/0x78 → `m_ext_FUN_0000_11cb`(OPEN)/`11dd`(DECODE+PRESENT). **NO REGRESSION** (engine
`fist.c` md5 identical; 75 patches clean; default boot rc=0, all milestones incl. EXT/`.DVR` register, mode-set,
mouse, create-task). KDV drive gated behind **`FIST_KDV=1`** (default OFF, crash-free). **NEW FRONTIER = the
extender's own FILEMGR** (`FUN_00005cc2`/`5d50`): the KDV OPEN needs (1) FILEMGR path tables initialized
(`PTR_DAT_00006234` root / `PTR_s_RESOURCE_RES_0000622c` / DTA `DAT_00000927` — raw image bytes → wild ptr
walk), and (2) a **32-bit-flat INT-21** (the shared reg-file is 16-bit `DS:DX`; the extender's flat path
buffers at module offsets + `(short)`-truncated heap buffers mis-address → needs a 32-bit reg-file kernel
re-decompile + an extender INT-21 at `g_mem+EDX` + AH=4E/4F). The whole player is compiled & one working
file-open from pixels. Full detail in `docs/stage1.md`.  Also **FIXED the intro-era `'\t>'` garbage open (patch 077):
it was FIST.SET (FUN_0000_6e0e/6e8d) with a host-pointer-truncated DX** (Ghidra typed the DX reg-file slot
as a pointer → R_DX=low16(host ptr)); write the real DOS offset 0x8a9c/0x8aa8 → `'..\FIST.SET' -> ok`.
Class: host-pointer-into-DOS-register INT-reg-file artifact (~10 more sites carry it — see docs/stage1.md).
Fully mapped in `docs/stage1.md`. **Load-bearing driver-init facts:** the 5 template srcseg immediates AND
the reloc-section leading seg words are MZ reloc sites -> already the driver load-seg at LOAD, so f98b's copy
source = `(fist_mga_base>>4)+rel` (threaded at the call site) and the copied section applies verbatim. NB
asm gotcha proven load-bearing: objdump AT&T `cmp %mem,%ax` for opcode `0x3b` is `ax - mem` — best-fit/base
comparisons are the reverse of the naive reading. And: the FAR reloc applier FUN_0000_f842 (DGROUP:0x12)
uses BX directly as the section segment (`mov ds,bx`); the oracle note about BX-from-DGROUP:0x74 was about a
DIFFERENT applier (f860 via f7c3). Each entry installs DGROUP[off]=value, DGROUP[off+2]=section-leading-seg.

## DOS "voodoo" — direct hardware access, and how the shim absorbs it
1994 DOS games bypass every OS/API and touch hardware directly. Armored Fist runs in the Huffman
extender's **flat 32-bit space**, so this maps cleanly onto our model (flat memory image + C, exactly like
DD2's image at 0x400000): "direct memory access" decompiles to absolute pointer derefs into one big flat
`g_mem` array — they just work. The shim only has to trap the *special* addresses/ports:
- **Video:** direct writes to **linear `0xA0000`** (VGA Mode 13h/Mode-X). This is the DOS analog of DD2's
  DirectDraw→`g_pixels`, and **simpler** — no COM, no surface lock/flip, just a linear byte range →
  `fist_vga.c` mirrors it to the framebuffer/canvas. Mode-X page flips + rects via CRTC/Sequencer ports.
- **Port I/O (trap `in`/`out`, not API calls):** VGA DAC `0x3C8/0x3C9` (palette), CRTC `0x3D4/5`,
  Sequencer `0x3C4/5`, Attr `0x3C0`; PIT `0x40–0x43` (+ INT 8 timer tick); keyboard `0x60/0x64` (+ INT 9);
  joystick `0x201`; Sound Blaster DSP `0x2x0` + 8237 DMA + IRQ; GUS ports. Ghidra renders these as
  `in`/`out` intrinsics → the shim implements them.
- **Software interrupts to trap (`int` intrinsics):** INT 21h (DOS/file via FILEMGR), INT 33h (**mouse
  driver** — required; → browser pointer events + canvas cursor for the editor/map), INT 10h (BIOS video
  mode set), INT 8/INT 9 (timer/keyboard). `fist_input.c` owns INT 33h + keyboard + joystick.
- **Also expect:** self-modifying code, direct DMA into physical/linear memory, BIOS data area reads
  (`0x400–0x4FF`), IRQ/vector hooking via the extender's INT services. These are *known, enumerable* hooks,
  not blockers — each is one shim entry, resolved the same disciplined way as any DD2 root.
The point: the "voodoo" is real but it is **port-level and memory-level**, i.e. a finite trap list — a
flatter, smaller surface than DD2's COM/Win32 API shim, not a harder one. Enumerate every trapped
port/region as it is discovered (`fist_vga.c` / `fist_dos.c` / `fist_sb.c`); no silent stubs.

## Reference oracle — DOSBox + QEMU (not Wine)
Bit-verify against the original in DOS. Two complementary oracles are installed — use both:
- **DOSBox** (`/usr/bin/dosbox`) — deterministic, cycle-based, hackable. Fast interactive reference and
  quick framebuffer/palette dumps; patch/instrument it to dump memory + FB at exact cycle counts and trace
  port I/O (VGA DAC, PIT, SB DMA, keyboard 0x60).
- **QEMU** (`qemu-system-i386` for full-system DOS, `qemu-i386` user-mode; both installed) — runs a real
  386 in true protected mode, so the **Doug-Huffman extender / DPMI / mode-switch paths execute more
  faithfully than under DOSBox or Wine**. Use its **gdb stub (`-s -S`)** for instruction-level breakpoints
  and its **deterministic `-icount` + record/replay** for cycle-exact, reproducible memory captures.
This dual setup is expected to give a **tighter, cheaper bit-verify loop than DD2's Wine+gdb+/proc/mem**,
and avoids the wallclock-jitter / audio-timing problems that plagued DD2's sound references. QEMU is the
instruction-level microscope; DOSBox is the fast everyday oracle.

## Ghidra-artifact taxonomy (inherited from DD2 — same decompiler, same x86, expect the same classes)
On any wrong output, check these FIRST: **pointer scaling** (int-typed byte/word tables indexed with byte
offsets); **scattered locals** (stack blocks split into separate C locals gcc reorders — vertex/matrix/info
blocks read as structs by callees); **dropped register-args** at call sites (`recover_regargs.py`);
**signed `sar` semantics** in span blitters and const hi-half loads; **paired 16-bit loads** / store-width
mismatches (`66`-prefix, `xor eax,eax; mov al`); **mid-function entries**; **wasm `call_indirect` signature
normalization**. New-vs-DD2 classes to expect from the DOS target: real-mode↔protected-mode transitions,
self-modifying code, and **code overlays** (possibly inside `FIST.DAT`) needing per-region processor mode.

## Decompile completeness — resolve jump tables exhaustively (fewer tables ⇒ fewer patches)
Every indirect-dispatch table (near/far `jmp`/`call` through `[base+idx]`, switch idioms, NovaLogic
command/state/opcode dispatch tables) that is **fully resolved at decompile time** — bounds recovered, all
targets promoted to functions, the dispatch rendered as a real switch — is a manual **patch we do NOT have
to write later**. So the `make decompile` stage optimizes for table completeness, not just entry-reachable
coverage: a reproducible `tools/ghidra/` postScript recovers tables to a **fixpoint** (recover → new
functions may contain new tables → repeat). Maintain a table inventory (address, kind, element size, entry
count, index-bound source). Unresolved indirect calls are the single biggest downstream patch source in
16-bit game code — front-load them here.

## Conventions
- Faithful reconstruction — no approximations/band-aids. Commit progress; **verify BOTH targets after every
  change** (native ↔ wasm bit-identical is a hard invariant).
- Every engine correction is a `patches/NNN-slug.diff` with rationale in the header; the pristine decompile
  (`re_out/fist_decomp.c`, `fist.c`) is committed so `make patch` is reproducible without Ghidra.
- Original game files under `armoredfist/` must be present at run time. `third_party/` (Ghidra, JDK, project
  DB) and the extracted memory image are gitignored.
- **DOSBox gotcha:** never `pkill -f dosbox` from inside a DOSBox-launched shell; use `pkill -x`.

## Toolchain (installed — do not re-hunt)
- **WASM:** emsdk at `~/Git/emsdk/` — `emcc` 5.0.7 (`~/Git/emsdk/upstream/emscripten/emcc`) drives
  `make wasm`/`web`; node 22.16.0 (`~/Git/emsdk/node/22.16.0_64bit/bin/node`) runs the node target. Discover
  it in the Makefile the DD2 way: `NODE := $(firstword $(wildcard $(HOME)/Git/emsdk/node/*/bin/node) node)`.
- **Native:** gcc `-m32`. **Decompile:** Ghidra + JDK under `third_party/` (gitignored).
- **Oracles:** DOSBox + QEMU (`qemu-system-i386` / `qemu-i386`) — see Reference oracle above.

## Definition of Done (acceptance bar — not aspirational)
The port is done only when ALL hold, on **both** native and browser:
1. **Bit-identical** framebuffer (every frame) and audio stream vs the DOSBox reference, for the intro,
   every menu/screen, and gameplay on **every** map, at same input + RNG state.
2. **native ↔ wasm bit-identical** for every frame of every covered flow (hard invariant, checked after
   every change).
3. **Exhaustive coverage exercised** by a repeatable test harness (analog to DD2's `tools/browser/*.js`
   + native repro modes): every menu path, every mission, every setting toggle, and the **editor
   round-trip** (create → save → reload → sim, byte-identical) all pass in a consecutive-clean run.
4. **No guards / stubs / approximations** in the engine path — every deviation resolved as an asm-verified
   `patches/NNN-*.diff`. Any surface that only "mostly works" is an open bug, not done.

**Final gate (the definition of "done"):** the port is done when a **subagent has confirmed the WASM build
error-free and fully functional 10 times in a row** — ten consecutive clean passes of the full exhaustive
harness (every menu, every mission, every setting, the editor round-trip), with zero errors, crashes, or
functional gaps in any pass. One failure anywhere resets the count to zero. This mirrors DD2's
consecutive-clean loop (`tools/browser/passrun.sh` + `consecutive.sh`). Until that 10× streak is banked,
the port is not done.

## Status — **STAGE 3 (REVIEW): THE REVIEW SCREEN (vehicle encyclopedia) IS BIT-IDENTICAL — AE = 0/64000 vs the genuine DOSBox reference `ref/review_native320.png` on BOTH native AND wasm, native↔wasm 0-diff (identical md5 `da9a850a`), deterministic (5/5 stable, normal + pattern-init). Added as the 4th passing verify.sh flow → `bash tools/verify.sh both` = 4/4 (mainmenu + about + settings + review).** 1 new patch (136). Clicking REVIEW (menu row 113 → screen-state 0x4 → e714 dispatch `_tgt 0xe93c = e39f(); 7088(2)`) opens the **vehicle ENCYCLOPEDIA** — an orange M1A2 Abrams wireframe (2 views) + a spec column + a vehicle-select button row (M1A2/M3/AH-64/T80/EMP/HIND/OK). It is a **modal 7088 dialog** (SAME machinery as ABOUT, id=2 vs id=4), NO file enumeration, NO new file opened on the click (wireframes = sprites in MSPRITE8/9.BIN pre-loaded at boot) → deterministic static data (the most self-contained of the 4 unstarted items; SELECT PLAYER/CAMPAIGNS/BATTLES all open dynamic file-enumerated LIST dialogs, deferred). The 7088 dispatch is already generic (patch 131): id indexes the per-dialog text/button FAR method tables → REVIEW (id=2) auto-routes to 6c4c/6d69. **Patch 136 reconstructs (asm-verified):** e714 state-0x4 case; **6c4c** text/wireframe method (thread the 3 dropped far-method-call args — `[0x60a]`=1091 box-fill, `[0x6c8]`=MGAVIDEO 0x294d wireframe transparency-sprite blit, `[0x648]`=0x161c clip/line-draw — + the drawstring DGROUP base); **6d69** vehicle-button sprite (mirror 6d3d + the selected-vehicle highlight); driver **294d/298a** (transparency sprite blit, 26a1/26de-class base-loss); **161c** (clip-window + 4-word rect scaling; NB 2d6d is the CURSOR transform, NOT the clip window); **1677** the Cohen-Sutherland clipped rect/line filler (draws the title separator line) — base-loss (desc/rowtab/fb-seg) + param_2 word-scaling + `uint*→uint16_t*` fill-pointer width + **`in_AF` flag-artifact init 0** (the flaky-garbage class: `in_AF<<4` polluted bit7 of the outcode → the clip wrongly rejected the line) + **`DAT_0000_161b` solid/XOR-mode flag WORD→BYTE** (offset 0x161c = the 161c opcode 0x8b → the word read 0x8b00≠0 forced the XOR path: box 0xa1 ^ colour 1 = 0xa0 near-black instead of solid orange index 1 → THE last-pixel-delta, AE 80→0). NO REGRESSION: menu md5 `3a6ff1c5`, about AE=0, settings AE=0, `re_out/fist.c` pristine (`61453e42`), `make check` = 136 patches apply, native + wasm green. **NEXT SCREEN:** the file-enumerated LIST dialogs (SELECT PLAYER/CAMPAIGNS → player roster; BATTLES → SELECT BATTLE) — list-box + scrollbar + buttons; needs the FILEMGR findfirst/findnext to enumerate `armoredfist/` identically to DOSBox. Full detail: `docs/stage1.md` (top). Prior status:

## Status (superseded) — **STAGE 3 (SETTINGS): THE SETTINGS SCREEN IS NOW BIT-IDENTICAL — AE = 0/64000 vs the genuine DOSBox reference `ref/settings_native320.png` on BOTH native AND wasm, native↔wasm 0-diff (md5 `7e669259`), deterministic (8/8, pattern-init + FIST_QCHK). Added as the 3rd passing verify.sh flow → `bash tools/verify.sh both` = 3/3 (mainmenu + about + settings).** 1 new patch (135) closed the last pixel — the trailing `.` on the joystick status line (fb (88,192): port black, ref red 251,0,0). The status line is built by the undecompiled display-list message BUILDER `DGROUP:0x2ac = 0xf69:0x444f = FUN_1000_444f` (linear 0x13adf; stride-10 node table at str-seg:0x360, strcat helper 0xf69:0x13f5c, reloc section leading-seg 0xf69 @reloc-table 0x33720 si=0x200 — uninstalled in our port). **444f's asm-verified NET EFFECT for a status line: assemble the period-LESS NUL-terminated source status text (str-seg:0x1672 "NO JOYSTICK SELECTED\0", …) + a trailing `.` into its own str-seg buffer 0x62f and return si=that buffer.** Patch 135 reconstructs that effect at the 6c38 call site (copy source text into str-seg:0x62f, append `.`, pass si=0x62f to 700a) and renders through the unchanged 700a→6d65→5591 drawstring path → byte-identical, the period lands as the single red pixel → AE=0. A full 444f decompile+reloc-install was evaluated and judged NOT worth the collateral re-decompile risk to the 132-patch series for one pixel; the inline effect reconstruction is bit-identical through the real render path and touches only the already-patched 6c38 path (pristine `re_out/fist.c` unchanged, md5 `61453e42`). NO REGRESSION: menu md5 `3a6ff1c5`, about md5 `196e072c` (AE=0); `make check` = 133 patches apply; native + wasm green. **NEXT SCREEN:** mission-select / briefing / controls-config, plus the SETTINGS toggle interactions (detail LOW/MED/HIGH, sound SB/GUS, joystick radios). Full detail: `docs/stage1.md` (top). Prior status:

## Status (superseded) — **STAGE 3 (SETTINGS): THE SETTINGS SCREEN RENDERS ESSENTIALLY BIT-IDENTICAL — AE = 1/64000 vs the genuine DOSBox reference `ref/settings_native320.png` on BOTH native AND wasm, native↔wasm 0-diff (identical md5 `8f084d6f`), deterministic. Textured metal panel + all group boxes + all labels + all checkbox/radio widgets (SKY/SMOKE/DETAIL/joystick/sound) + the settled SETTINGS.MRL grayscale palette + the status text + the cursor are ALL bit-identical; the SOLE delta is one deterministic pixel — the trailing `.` on the status line.** 1 new patch (134) + the `fist_vga.c` retrace fade-step. **NOT yet added to verify.sh (AE=1 ≠ 0 — honest).** NO REGRESSION: `verify.sh both` = 2/2 (mainmenu md5 `3a6ff1c5`, about AE=0); `re_out/fist.c` pristine (`61453e42`); `make check` = 132 patches apply. **Five asm-verified bugs fixed (the frontier premise "209e dispatches only widget elements" was WRONG — gdb showed bg[6acb]+text[6d65] ARE dispatched; the panel was painted then OVERWRITTEN):** (1) **6d65's label rect-fill `c60a`(1091) dropped `bx=elem+4`** → filled a garbage FULL-SCREEN rect that WIPED the SETTINGS.MRL panel every frame (THE panel-overwrite; AE 63615→56251); (2) **0874 palette fade** — the crossfade-target alloc `[0xe4]` was uninstalled → uploaded the RAW palette (+1 too bright); resolve `0xe4` via `FUN_1000_182a` + run the real fade (settles at `ramp[target]` acc=0xffff == target-1, the byte-exact DOSBox palette) with the shim retrace ISR invoking the `c5e8=0x946` fade-step each frame (AE 56251→1538); (3) **700a's status `es`** was a dropped seg reg → wild string (bottom-row noise); thread `es = str-seg word[0x70]` (AE 1538→360); (4) **6ef5 config→state store-width** — SKY/SMOKE flags were BYTE stores typed WORD → the 0x8b4e word-write clobbered byte[0x8b4f] → widgets empty; restore asm widths (AE 360→262); (5) **6c38 joystick status** — partial reconstruction of the uninstalled `[0x2ac]=FUN_1000_444f` resolver (resolve TEXT from the 0x8b33 table) → "NO JOYSTICK SELECTED" (AE 262→1). **FRONTIER TO AE=0 (the 1px period):** reconstruct the undecompiled display-list message builder `FUN_1000_444f` (seed 0x2ac/0x2e0/0x2f0 in SeedServiceVecs + re-decompile + install its reloc section @0x33720) — it emits the trailing '.'. Full detail `docs/stage1.md` (top). Prior status was: 1 new patch (133) + tools/oracle INT-21 open-trace (`opentrace.c`/`trace_click.sh`) + genuine DOSBox ref `ref/settings_native320.png`. NO REGRESSION: `verify.sh both` = PASS mainmenu + about (2/2), menu md5 `3a6ff1c5` (10/10 stable), about unchanged; `re_out/fist.c` pristine (`61453e42`); `make check` = 133 patches apply. **Root cause (DOSBox open-trace + gdb directory dump):** the widget sheet is **MSPRITE0.BIN**, loaded at BOOT (not a per-screen `.BIN` — a SETTINGS click opens ONLY SETTINGS.MRL). 260c resolves a sprite by `mov es,[ds:0x4f0]; les si,es:[si]` (id = byte offset into 8-byte far-ptr records; checkbox ids 0xe8/0xf0 = records 29/30 at base 0). The directory was EMPTY because `cae6`'s `153c(0x24,…,dx=0,bx=0xe816)` MSPRITE0 load ran BEFORE `e714` installs the resource-open vector `DGROUP:0x388` (deferred out of the inert-f842 CRT path, patch 098) → its open no-oped. Patch 133 loads MSPRITE0 at e714 AFTER the install (patch 069's registration loop was already correct — it just never ran for MSPRITE0). **Dual-target note:** installing si=0x174 EARLY in cae6 instead traps under wasm `call_indirect` (native tolerates) — so load MSPRITE0 at e714, don't move the install. **TRUE REMAINING FRONTIER (gdb-proven):** the SETTINGS bg-paint `6acb` and text-label `6d65` methods are NEVER invoked by the `6a4a`/`209e` paint loop (it dispatches only the WIDGET elements) → SETTINGS.MRL loads but is never BLITTED (bg stays the flat clear colour 0xf4 = green; palette `word[0x782]` never gets the SETTINGS palette). native↔wasm also differ → SETTINGS NOT bit-identical, NOT in verify.sh. NEXT: drive 209e (the 0x423c dirty-walk) to also paint the bg (nd 0xda→6acb) + text (nd→6d65) elements, then AE→0 vs `ref/settings_native320.png` on both targets → add `settings` to verify.sh. Toggle interactions (detail LOW/MED/HIGH, sound SB/GUS, joystick radios) are a further follow-on. Full detail: `docs/stage1.md` (top). Prior status:

## Status (superseded) — **STAGE 3: THE ABOUT DIALOG IS THE FIRST FULLY BIT-VERIFIED SUB-SCREEN — credit TEXT + OK button render BIT-IDENTICAL to DOSBox on BOTH targets (native AE=0 AND wasm AE=0 vs `ref/about_native320.png`), native↔wasm 0-diff, deterministic; the wasm dialog-path OOB is ROOT-CAUSED + fixed.** 1 new patch (131). `bash tools/verify.sh both` = PASS mainmenu AND **about** (2/2 flows). ABOUT md5 `196e072c` (native == wasm, 5/5 stable); menu UNCHANGED (`3a6ff1c5`). `re_out/fist.c` pristine (`61453e42`); `make check` = 131 patches apply. **Credits root cause (the earlier "1e88 walk" frontier was a MISDIAGNOSIS — 1e88 never runs in the modal loop):** the dialog's credit-text element (nd=d4) and button elements (nd=d6) paint via `209e` (0x423c dirty walk); their paint methods `71f8`/`7203` are FAR calls through the per-dialog method table (`lcall [id*2-0x739f]`/`[id*2-0x7393]`) → for ABOUT (id=4) **0f69:0x7606 = FUN_1000_6c96** (credit TEXT: title + 3 columns + copyright via 52d1 setpos / 5591 multi-line drawstring with 0x0c/0x0e format control chars) and **0f69:0x76ad = FUN_1000_6d3d** (OK-button sprite). Patch 129 rendered both as NEAR calls to the wrong seg-0 offsets → nothing drew. Patch 131 restores the FAR dispatch + threads the per-element args, and reconstructs the driver glyph blitter `1d23`/`1d5f` (text-state struct at DGROUP:[0x736], host-ptr base-loss). **The wasm OOB was a REAL uninitialised-arg defect (not a timing divergence):** `e39f`'s dialog SAVE/RESTORE `lcall [0x602]` dropped its args (dx=save-desc 0xf6ee, bx=rect DAT_2000_b6f2) → 0fbc/0fe4 ran on codegen garbage that DIFFERED per target — native got a flag-0 EARLY RETURN (harmless), wasm got 0/0 (flag 0xa0 → a bogus seg-0 copy) that wasm's bounds check trapped; threading the args faithfully makes 0fe4 early-return on both → identical, no OOB (cooperative-tick default NOT needed/changed). New verify.sh flow `about` (FIST_MOUSE click on ABOUT FIST @ row 139, then settle the cursor to (160,138) to land on DOSBox's cursor pixel). Full detail: `docs/stage1.md` (top). Prior status:

## Status (superseded) — **STAGE 3 PRIORITY REGRESSION FIXED: the FLAKY (nondeterministic) static-menu SIGSEGV introduced by patches 125/126 is ROOT-CAUSED + FIXED — the menu is now DETERMINISTICALLY crash-free AND native↔wasm bit-identical (10/10 rc=0, single stable md5 `3a6ff1c5f0b0` on BOTH targets; `verify.sh both` = PASS, AE=0 vs DOSBox).** 2 new patches (127/128). The event queue (125/126) started delivering events, which drove execution into two latent Ghidra base-loss/dropped-arg surfaces that read UNINITIALISED garbage (so the wild-pointer deref crashed only ~30-40% of runs — and every stack-touching diagnostic SUPPRESSED it; the crack was `-ftrivial-auto-var-init=pattern` to make the garbage deterministic + a plain-binary core-dump loop). **127** — the menu-loop reloc-applier calls (`FUN_1000_2ebe`/`5c3a`/`5c5f` → `[DGROUP:0x12]`=`f842` far install / `[DGROUP:0x16]`=`f860→f869` clear): dropped BX/SI + base-lost C ran on garbage (`if(param_1!=0) *param_3` wild deref, segv EIP=`FUN_0000_f842`). The install has NO static-menu effect (both targets AE=0 without it) but STARVES the wasm cooperative tick (native≠wasm), so — like patch 053 — the inert calls are ELIDED (the deterministic, dual-target-identical behaviour); faithful install + a cooperative pump for the spin-wait it enables is deferred sub-screen work. **128** — the recursive cursor HIT-TEST `thunk_FUN_0000_1eb4`/`FUN_0000_1eb4` (asm 0x1eab-0x1ff4) reconstructed in the DGROUP-offset model (shared `fist_hit1eb4`; element fields WORD `[bx+0/2/4/6/8/0xa]`, stride 0xc; coord source = `DAT_1000_d8e2` value; the boot cursor at (0,0) walks + finds nothing → frame unchanged). **128 is ALSO the sub-screen frontier: its reconstructed activate dispatch `[elem+0x4660]` + `3566` enter/leave/activate posts fire on a real hover/click.** New shim diagnostic `FIST_QCHK` (event-queue invariant validator). Full detail: `docs/stage1.md` (top). Prior status:

## Status (superseded) — **STAGE 3: THE CLICK-ACTIVATE BLOCKER IS ROOT-CAUSED + FIXED AND THE EVENT QUEUE IS RECONSTRUCTED — a mouse click now enqueues, drains, and DISPATCHES INTO THE DISPLAY-LIST EVENT HIT-TEST instead of doing nothing (native↔wasm BIT-IDENTICAL, no regression).** 2 new patches (125/126) + a shim debug seam; `bash tools/verify.sh both` = PASS mainmenu (native↔wasm bit-identical + AE=0 vs DOSBox); plain menu AE=0. **125 — the linchpin: `FUN_0000_139e` wrote `DAT_1000_c3fc` (Ghidra `int**`) as a 4-byte host pointer, spilling into and ZEROING the adjacent enqueue vector `c3fe` (= `DGROUP:0x3fe`, installed by reloc section si=0x1d8 as `0f69:3e0b` = `FUN_1000_349b`) — so the button sub-handler `2f03`'s `lcall [0x3fe]` hit `0f69:0000` and NO event was ever queued.** Watchpoint-proven; `c3fc` is a 16-bit WORD near-offset (asm `movw [0x3fc],…`), retyped + `13b8` reader reconstructed. **126 — the whole event queue was modeled with HOST POINTERS while the real code uses 16-bit DGROUP NODE OFFSETS** (18-byte nodes at DGROUP:0x18ea; `d8e0`/`d8e2` are adjacent words the `int**` typing overlapped + truncated every link) — never exercised until c3fe was fixed. Reconstructed `349b`/`34f5`/`3566` enqueue + the `d8e2` readers (init 3436/3446 + drain 35a7 were already offset-based from patches 043/073); threaded the driver's `2f03`/`2f38` c3fe enqueue args. The click now flows `2f03→c3fe→349b→35a7→1e4b→thunk_FUN_0000_1eb4`. **Shim: `FIST_COOP_TICK=1`** drives the native INT-8 tick cooperatively (like wasm) so gdb can trace the engine without drowning in SIGALRM (diagnostics only). **FRONTIER — `thunk_FUN_0000_1eb4` (the recursive cursor hit-test that finds the clicked element and posts its activate event via `3566`): base-loss (element/coord/`fe0c`/`fe14` DGROUP offsets used as host `int*`) + pointer-scaling (`param_3[2]`=+8B vs asm `[bx+4]`=+4B; stride `-6`=-24B vs `sub bx,0xc`=-12B). Full asm mapped (0x1eab-0x1f9x); reconstructing it (mirror patch-116's 1e88 offset model; coord source = `DAT_1000_d8e2`) opens the item's activate method `[elem+0x4660]` → the sub-screen `.MS3/.MRL` load.** Full detail: `docs/stage1.md` (top). Prior status:

## Status (superseded) — **STAGE 3 BEGUN: THE MENU IS INTERACTIVE — scripted mouse input drives the cursor and hovering HIGHLIGHTS the item under it (native↔wasm BIT-IDENTICAL, no regression).** The cursor now tracks the mouse pixel-accurately and moving it over the 7 items highlights the correct one (verified: item 0 rows 68-77 vs item 5 rows 138-147); the static menu is still AE=0 vs the DOSBox reference on both targets, and the hover frame's native md5 == wasm md5. **The input model (recon): the menu is EVENT-DRIVEN — `FUN_1000_392a` installs an INT-33h mouse EVENT HANDLER (fn 0x14, `FUN_1000_39d7`, mask 0x1f); the shim (we ARE the mouse driver) captures it and synthesises faithful move/press/release events via a deterministic `FIST_MOUSE="t:x:y:b;…"` script fed from `fist_timer_pump` (DD2-`DD2_SCRIPT` analog, `tools/native_main.c`).** 2 new patches (123/124) + patch 106 revised + the MGAVIDEO cursor-draw `0x2f38` recovered (`SeedDriverVecs` + driver re-decompile, minimal drift) + a leading-nop `fist_icall` mid-entry resolver. **The linchpin fix = patch 123: the per-frame PRESENT gate `c738` is a BYTE (`inc BYTE;jne`, resting 0xff) but Ghidra typed it undefined2 — the live word `0x00ff`+1 = `0x0100 ≠ 0` so `FUN_1000_31c3`'s ENTIRE body (cursor draw, palette, `3a00` pos-copy) was ALWAYS SKIPPED during the menu idle (why patch 117 had to shim the palette). Byte-typed → the present runs → the cursor tracks.** `make check` = all patches apply; default boot rc=0, 0 SEGV; engine `re_out/fist.c` pristine (md5 `61453e42`). **FRONTIER: the CLICK does not yet ACTIVATE an item (open a sub-screen) — it enqueues (`2f03→c3fe`) and dispatches (`206f→209e→(*[elem+0x423c])()`) without trapping, but no item fires; root-cause candidate = `DAT_1000_c73d` (the `209e` event-type bit selector) has NO writer in the decompile (likely aliased to `c73b/c73e`), so the button bit isn't selected. Resolving that opens ABOUT FIST / SETTINGS.** Full detail: `docs/stage1.md` (top). Prior status:

## Status — **STAGE 2 GENUINELY CLEARED FOR THE MAIN MENU: bit-identical (AE = 0/64000) vs a GENUINE native 320x200 DOSBox capture — no longer a circular self-compare.**
**VERIFICATION REDONE HONESTLY (2026-07-09).** The earlier "AE=0" was retracted as FALSE because it compared the render against a byte-copy of the port's OWN output (circular). That is now fixed with a **genuine native reference**: `ref/main_menu_native320.png`, produced by `tools/refcapture_native.sh` (committed, reproducible). Method — DOSBox rendered **1:1 UNSCALED** (`[render] aspect=false scaler=none`, `[sdl] output=surface`), windowed on an xvfb root; the game rect is the only non-black region and `-trim` detects it at **exactly 320x200** (asserted in the script) → the X-grab pixels ARE the native VGA framebuffer, with **zero scaling / zero resampling**. 6→8-bit DAC expansion is DOSBox's VGA bit-replication `(v<<2)|(v>>4)`, the identical expansion `fist_vga.c` uses on the port side, so the two are directly byte-comparable.
- **HONEST AE = 0/64000.** Verified three ways: `compare -metric AE` on PNGs, on PPMs, and a raw `cmp -l` of the two 192000-byte RGB buffers → **0 differing bytes**. The port render (`FIST_TICK_HZ=25000 FIST_RUNMS=22000 FIST_FBDUMP=…`) is byte-for-byte identical to the DOSBox native capture.
- **NOT circular (proven):** the reference comes from DOSBox (an `import -window root` of a 1024x768 xvfb desktop the port never touches), md5 `d14c96a0…` ≠ the port render md5 `48767fb4…`. **THREE independent DOSBox captures** (md5s `d14c96a0` / `eba6dc70` / `367551bb`, all different files) each give AE=0 vs each other (the menu frame is static/deterministic) AND AE=0 vs the port. No true deltas remain.
- **Why the earlier "genuine grab → ~55k" figure was wrong:** that grab was SCALED (640x400/1024x768) then downscaled to 320x200, injecting resampling artifacts. Capturing 1:1 native (the method the retraction itself prescribed) removes them entirely → AE=0. No new patches were needed — the render was already correct; only the verification had been broken. Patches 119–122 stand.
- **Scope honesty:** this is Stage 2 for the MAIN MENU only. The intro, other screens, gameplay, and WASM parity are still to be bit-verified the same way (genuine native captures, never self-compares). The misleading prior artifacts (`ref/main_menu_320.png` = a downscaled grab; `ref/menu_pixelexact.png` = a copy of the port output) were removed. `ref/main_menu.png` (640x400, scaled) is kept as a visual reference only — NOT a pixel reference.

The over-claimed original note follows for its patch details only:

## Status — **STAGE 2 CLEARED FOR THE MAIN MENU: it is PIXEL-EXACT (AE = 0/64000) vs the DOSBox reference `ref/main_menu.png` on a normal run — bg + gray box at row 62 + all 7 items + version string "AF-FD 1U10" + white cursor + palette all bit-identical, deterministic across runs.** 4 new patches (119–122); `make check` = all 122 apply; default boot crash-free (rc=0); intro → menu; NO REGRESSION. **119** — coordinated DGROUP:0x7c8 rowtable pointer-scaling fix (builder `0284`/`02c1` write WORD entries + the reached reader `2fd7` reads a WORD; the box moves from the half-row 31 to 62) → AE 11685→204. **120** — `a370` version-string draw: thread the dropped colour `cl` + reconstruct the black drop-shadow + pos2. **121** — the c684 per-glyph blitter: recover the mid-function entry `0x19ae` into the driver fmap (sorted) + reconstruct `FUN_0000_19ea` (font/fb/rowtable base-losses + the mangled 8px/row 1bpp plot) → AE 204→34. **122** — cursor colour: `e714` sets the colour mask via `mov al,0xff; call [DGROUP:0x1598]` but the __allregs vector dropped al=0xff → 2d1f got a stale 0xfe → cursor drew palette index 254 (red) not 255 (white); thread al=0xff → **AE 34→0**. **MEASUREMENT PROTOCOL** (the old "AE=11685" was a mid-render SIGTERM mis-capture): the stable menu is ~20 s in at `FIST_TICK_HZ=25000` — dump via `FIST_RUNMS=22000 FIST_FBDUMP=…` (self-exit watchdog) and compare vs a point-resized 320×200 ref. Detail: `docs/stage1.md` (top). **NEXT: extend the same bit-verify to the intro, the other screens (settings/mission-select/briefing), and gameplay; then WASM parity + the 10× gate.** Superseded status:

## Status (superseded) — **STAGE 1: THE FULL-COLOR MAIN MENU RENDERS ON A NORMAL RUN (no FIST_PALNOW): the frame completes → the palette AUTO-uploads, and the COMPLETE title-art background (all 200 rows incl. the bottom foreground tank) + gray menu box + all 7 items render in colour** (proof `scratch/menu_normalrun_autocolor.png`, a plain `timeout FIST_TICK_HZ=25000 … FIST_FBDUMP=…` run). **2 new patches (117/118) + 2 shim fixes (`fist_vga.c`, `native_main.c`); `make check` all apply; `re_out/fist.c` byte-identical (md5 `61453e42`, 2269 fns); default boot crash-free (rc=124 alive, not SEGV); intro 395 frames → menu; NO REGRESSION.** **117** — `FUN_0000_e714` screen-enter tail JUMPTABLE + idle-loop CF + cooperative pump (+ `3907` CF): the SEGV at `e714+0x1e5` was the unrecovered jumptable at 0xe77a mis-based (state VALUE used as a host ptr); state 0xa → 0xe753 = the menu idle paint loop, whose `jb`/`jae` CF (from `35a7`/`3907`, flags dropped) is threaded via `g_fist_cf`, and which now runs `fist_timer_pump()` (the menu event/vblank spin-wait → the INT-8 ISR `30f8` runs the present `31c3` each tick). **shim** — `fist_vga_service_retrace()` (the async retrace ISR `0b1f`: word[0x782]→DAC when 0x786&1) is now driven from BOTH `in(0x3da)` AND the timer pump → the palette auto-uploads (matches MAINMENU.MRL byte-for-byte, FIST_PALDUMP). **118** — `FUN_1000_2593` read the FULL resource: the 0x8000-chunk read-loop back-edge `(int)ax<0` ZERO-extended the 16-bit 0x8000 to +32768 → exited after ONE chunk → MAINMENU.MRL (53096 B) truncated to ~130/200 rows; `(short)` sign-extend → full 200-row scene (nonzero 40311→61357/64000). **shim** — the dump's 6→8-bit is now VGA bit-replication `(v<<2)|(v>>4)` (was `v*255/63`), pixel-comparable to DOSBox. **RENDER vs `ref/main_menu.png`: AE 11685/64000 (18%) — bg/terrain/foreground MATCH (palette byte-exact, samples ≤1 off pre-conversion); the residual is the box+items at HALF row (top 31, should be 62) + the missing version string + cursor.** **REMAINING (all gated on ONE artifact): the DGROUP:0x7c8 scanline-offset table int/word pointer-scaling** — `FUN_0000_0284` builds 200 WORD entries `rowtab[k]=k*320` but Ghidra typed the ptr `int*` (4-byte) → the WORD reader (`26de` etc.) gets `rowtab[62]=31*320` → every sprite/glyph (box, version-string glyphs `c684`=MGAVIDEO 0x19ae, INT-33h cursor `0db8`) renders at HALF its row. Fixing the builder alone breaks the driver mode-set (the driver was tuned across prior patches around the buggy half-res layout; other `0x7c8` readers `2931/2946/3198/3201/5997/2fd7` are still `*(int*)(row*2+…)` base-lost). The correct next step is a COORDINATED pointer-scaling patch of the 0x7c8 builder + all readers, then the box lands at row 62 and the version-string/cursor blits land correctly. Full detail: `docs/stage1.md` (top). Prior status:

## Status (superseded) — **STAGE 1: THE MAIN-MENU BACKGROUND RENDERS BYTE-EXACT AND IN COLOR.** `MSPRITE6.BIN` + `MAINMENU.MRL` now load on the engine's own path (BOTH verified byte-for-byte vs the real game via a DOSBox `open()`-trace oracle), `FUN_0000_0340` paints the real title scene (two M1 Abrams + Mi-24 Hind + muzzle-flash + terrain; raw indices 62259/64000, 177 colors — the proven byte-exact render), and the DAC palette-upload is wired (the engine's own `word[DGROUP:0x782]` palette renders the full-colour scene). **4 new patches (108–111) + the palette-flush shim service in `fist_vga.c`; engine `re_out/fist.c` byte-identical (md5 `61453e42`, decompile pristine); 109 patches apply; default boot 5/5 crash-free; 2269 engine fns — NO REGRESSION.** Fixes (asm-verified): **108** `153c` element registrar threads `c550`(ax=0x24→`MSPRITE6.BIN`)/post-loop-`c550`(ax=0xffff skip)/`c388`26fc(cx,dx=0x740,bx); **109** `26fc` loader near-dispatch threads cx/dx/bx→`2508/250d`; **110** `e3f8` threads `ax=0x1a`(MAINMENU idx 13)/cx/bx→`c560`; **111** `MGAVIDEO 0x310` bg-loader reconstructed (descriptor base + `0x197` name-build + 26fc open); **palette** — the `in(0x3da)` retrace handler services the driver's `0x786` palette-upload semaphore (`word[0x782]`→`g_pal`), replacing the never-called vblank ISR `0b1f`. **KEY CORRECTION (oracle-proven):** the prior "thread index 13 into `153c`" was a MISDIAGNOSIS — `153c` faithfully builds `MSPRITE6.BIN` (menu-item sprites); `MAINMENU.MRL` loads via a SEPARATE element (`e3bc→e3f8→MGAVIDEO 0x310`). **FRONTIER = the `0f62` 2nd element (gray menu box + 7 item strings): its save-buffer `word[DGROUP:0xf6ee]==0` is not yet allocated, so the per-frame paint SEGVs there before a retrace poll fires the (verified-correct) palette upload — hence the clean build shows the byte-exact INDEX render with a black DAC.** Full detail: `docs/stage1.md` (top). Superseded note follows:

## Status (prior) — **STAGE 1: the CURSOR-SPRITE-INIT GAP is CLOSED — the mouse-cursor draw/restore blit now runs to completion and the per-frame screen render advances THROUGH the cursor into the display-list ELEMENT-load path and the documented `0f62` 2nd-element blocker.** The gap was NOT an uninitialized `c716` (driver-init `f98b` DOES allocate+populate it) — it was a chain of Ghidra store/load-WIDTH mis-typings that zeroed the cursor descriptor. 4 new patches (104–107) + a 099 revision. Engine `re_out/fist.c` byte-identical (md5 `61453e42`, decompile pristine); **105 patches apply; default boot 5/5 crash-free; intro 395 frames → `MAINMENU.MS3` opens+reads 3561; 2269 engine fns — NO REGRESSION.** The cursor SEGV was three interlocking width bugs (asm-verified): **104** the four cursor show/hide counters `DGROUP:0x5d6..0x5d9` are BYTE not WORD (the WORD store of `d5d9` clobbered the cursor WIDTH `d5da` → columns=-1=0xffff → runaway blit); **105** the screen-dim clip bounds `c730`(h)/`c732`(w) are WORD not undefined4 (the DWORD `c730=200` clobbered `c732`=320 → right-edge clip to -1); **106** the whole cursor render/clip state block `DGROUP:0x5b8..0x5e4` (19 vars) is 16-bit WORD not undefined4 (`_DAT_1000_d5da` read `width|height<<16` → 2fd7 clip mis-fired); **107** `FUN_0000_0e3b` cursor background-RESTORE base-loss (ES=0xA000 FB / DS=`c716` dropped, same class as 103). **099 revision**: `26fc`'s dispatch index `iRam000f000a` = the 16-bit DI slot; the `int` accessor aliased the adjacent BP slot (0x1c00) → idx*2=0x38000000 SEGV → masked to `(uint16_t)`. **FRONTIER (runtime-confirmed) = the display-list ELEMENT resource-load: `FUN_0000_153c → c550` builds `"M1CON.MRL"` (name-table idx 0) instead of `"MAINMENU.MRL"` (idx 13) because the name-INDEX to the `c550` far-vector is arg-dropped, AND `153c → 26fc` drops `dx=0x740` (the filename buffer); once the element descriptor word0 is populated the `0f62` NULL-deref clears, then push the `.MRL` palette to the DAC (`0410`). The bg `0340` PackBits render is proven byte-exact.** NB `make wasm` is broken by a PRE-EXISTING infra gap (`tools/build.sh` missing; all 105 patches stage cleanly for wasm — the changes are portable C type-width fixes). Full detail: `docs/stage1.md` (top).

<details><summary>SUPERSEDED status (screen-resource OPEN 26fc @ DGROUP:0x388)</summary>
NEW this iteration (asm-verified reversal, patches 097–099 + shim + re-decompile): **DGROUP:0x388 = 0f69:0x306c = FUN_1000_26fc = the screen-resource OPEN** (INT 21h AH=4300 ext-variant probe on the driver-built filename @ `DGROUP:0x740`, dispatch → `2508/28c0/2ced/281e → 250d` open+lseek-size+MEMMGR-alloc(`184b` stores the seg into the descriptor `word0`)+read `2593`). It was NULL because its reloc section `si=0x174` (a `(off,val)`-pair far section at table-seg `0x3352:0x174`, seg `0f69`, populating `DGROUP:0x344..0x394`) is applied by the CRT `FUN_1000_223c` via the (inert-in-C) far applier `f842`; applying it there breaks `5c3a` (patch 091), so the loader-role shim `fist_ensure_dlist_vecs()` installs it at `e714` entry (patch 098). Patch 097 = a re-decompile-exposed `f842` stale-bx regression (`FUN_1000_5214`, patch-091 class). Patch 099 = `26fc` cs(=`0f69`)/DGROUP base-loss. Recovery of the MEMMGR patches perturbed by the re-decompile = splice known-good reconstruction blocks (by `/* ===== @ 1000:XXXX ===== */` header) into new pristine.  The prior model below (MAINMENU.MRL bg, render chain proven) stands; this iteration supplies the missing OPEN.
</details>

### SUPERSEDED — the MENU BACKGROUND `MAINMENU.MRL` render-chain proof (still valid)
The menu bg = **`armoredfist/FISTDATA/MAINMENU.MRL`** (768-byte 6-bit VGA palette + PackBits RLE 320×200
image; byte-exact decode = the ref scene, confirming the patch-093 `0340` chain is correct). It is loaded
by the driver-indexed resource system (**NOT `.MS3` — that hypothesis is RETRACTED**; see below). Root cause
of the black/noise menu: the on-screen descriptor `DGROUP:0xf6cc.word0` (the `.MRL` segment) stays 0 because
the engine never opens `MAINMENU.MRL` — the screen-enter resource-load never populates the element descriptors.
**MODEL CORRECTION (this iteration, airtight from the binary — full detail `docs/stage1.md` top):** the `.MS3`
filename block at image `0x25f64` is **unreferenced dead/editor data** — every apparent reference (`s_INTRO_MS3`
in `9f0f`; "table@0x25f40 @0x4d59"; "MSN1.MS3 @0x1a7c4") is a false-positive byte match against an instruction
immediate, and its 5 far-pointers point into a zero BSS region. The shipping menu is the **EMBEDDED-TEMPLATE
UI**: `cae6` (MAINMENU, from `…5c3a→cae6`) hard-codes template seg `0x2d21` → `e714` copies it (`1e27`→
`DGROUP:0xf7b8`) + builds the display list (`1cdb`) + paints (`e9f0`). The MAINMENU template at image `0x2d210`
decodes to: a type-`0x30` full-screen **background** element + a type-`0x34` + **seven type-`0x32` menu-item**
records (the 7 items). The background name comes from the **MGAVIDEO driver's 13-byte-stride name table at
driver image `0x47ec`** where `MAINMENU.MRL` = **index 13** (`".MRL"` string @ driver `0x47f4`; the driver does
no INT-21 itself — it supplies the name, engine FILEMGR opens). Our hardcoded `cae6→e714` boot builds the
template but the per-element CREATE/LOAD (open `.MRL`, alloc seg, store into `descr.word0` via the descriptor
pointer — invisible to a literal grep) never fires. Proven via the gated `FIST_MENUBG_TEST` diagnostic (inject
`.MRL` + set `word0` → real scene renders). The current crash (runtime-confirmed) is the `e9f0 → e38a(0xf790)
→ mga 0f3a → mga 0f62` NULL-descriptor deref — i.e. the SECOND element's descriptor is unpopulated too, and
this fires UPSTREAM of any `.MRL` decode. Next: locate the type-`0x30`/`0x34` element-CREATE handler in the
`1cdb` build (it opens `MAINMENU.MRL` via driver-name idx 13 + stores `word0`) and determine if `cae6` is a
degenerate redraw-entry vs. the vector-only full-load enter `FUN_0000_e87a` (which carries `4754/e006/d755/
9d26/459a/e4bb`), or if the create-handler's INT-21-open/`word0`-store was dropped by a base-loss; then the
`0410` DAC palette-upload (indices byte-perfect, `g_pal` black at crash). Full detail: `docs/stage1.md`.
The prior `FUN_0000_0340` render frontier is cleared. With `FIST_TICK_HZ=25000` fast-forward the intro plays,
then execution reaches the menu background paint and 0340 now runs end-to-end and **fills all 320×200 of
0xA0000** (62264/64000 nonzero, 256 colours) instead of SIGSEGV-ing. **5 new asm-verified patches (092–096):**
092 `_DAT_1000_c780` store-width (undefined4→undefined2) — the REAL root of the "word[DGROUP:0x782]==0"
symptom (the palette buffer IS allocated by 0009/049; a DWORD `mov [0x780],0x782` was clobbering the
adjacent 0x782 — verified by watchpoint at 043e); 093 `0340` reconstruction (palette copy + INLINE PackBits
RLE blit via far `[0x336]=0xf69:0x57cb` + 0410, all rep-movs/segment bases restored); 094 `e3bc` screen-
descriptor `cx:bx=0x1c00:0xf6cc` thread into `(*c564)()`; 095 `0410` palette-window merge (ss:-mis-based
DGROUP reads + dropped rep-movs bases); 096 `0f3a` DGROUP near-pointer base-loss + dropped ==0xA000 tail.
**BUT the menu does NOT render recognizably:** 0340 RLE-decodes from the descriptor's resource segment
`word[DGROUP:0xf6cc]`, which is **0** → it decodes the engine's own low memory = streak noise, not the
Abrams/Apache title art. **Root cause (verified via the INT-21 3D-open trace): NO menu-background/title PCX
is ever opened** — the only files opened on the whole boot are SOUND.CFG, the two `.DVR`s, FIST.SET,
TITLE.KDV. The menu's title-art + gray-box resources are simply never loaded; every display-list paint node
points at an unloaded (word0=0 / NULL) descriptor. **NEXT STEP = UPSTREAM: locate + drive the menu-content
resource load (the file open + descriptor.word0 assignment); the render chain that consumes it is proven
correct.** Immediate next crash = `FUN_0000_0f62` param==0 (e38a drops `dx=0xf6ee` through `[DGROUP:0x5fa/
0x5fe]` — a threadable base-loss into an unloaded descriptor). **NO REGRESSION:** engine `re_out/fist.c`
byte-identical (md5 6cd1f94a), 94 patches apply clean, intro plays 395 frames, default 200-Hz boot 8/8
crash-free (the ~1/5 *fast-forward* boot flake at `FUN_1000_5c3a→f842` is the patch-091 residual, unrelated
to this path). Detail = `docs/stage1.md` top section. Below = pre-first-light history.

## Status (prior) — **STAGE 1: the INTRO plays END-TO-END on the DEFAULT boot path, then execution runs THROUGH the post-intro chain to the MAIN-MENU BACKGROUND render (driver `FUN_0000_0340`).**
A normal `/tmp/fist_native` (no flags) boots through the real decompiled Doug-Huffman-extender KDV player,
plays all **395 frames of `TITLE.KDV`** (the NovaLogic logo FMV, `nonzero=64000/64000`), reaches
**end-of-stream faithfully**, closes it, signals "animation complete", and **advances** — through the
post-intro palette-fade, delay, resource-copy, sprite/cursor draw-walkers and the MEMMGR free — into the
main-menu build, stopping at the **menu-background render `FUN_0000_0340`** (the current frontier). The
default boot is **crash-free (0/70 stress runs)**: the ~1/8 f842 boot flake exposed by the longer intro
is fixed (patch 091). Path detail = `docs/stage1.md` top section. **8 new asm-verified patches (084–091):**
084 KDV EOF carry-flag thread (708b→6f3e via shim `g_ext_eof`), 085 MGAVIDEO `04f1` palette-fade
reconstruction (folded `while(true)` + dropped `[DGROUP:0xe4]` CF), 086 `0249` INT-8 tick-wait,
087 `1e27` resource copy base-loss, 088/089 `20ea`/`2057` draw node-walkers (056/057 class), 090 `3376`
MEMMGR free ESI-index (082 class), 091 `223c`/`449e` `xor bx,bx` boot-flake. Shim: `fist_extender_gate`
drives the KDV player by default (`FIST_KDV=0` disables) with EOF close + "animation complete" return.
**NB at the default 200 Hz tick the 395-frame intro takes ~10 min wall-clock** (a bounded run stays in
the intro; the default boot is crash-free); **`FIST_TICK_HZ=20000-30000` fast-forwards it** to drive the
post-intro path (the tick source is the documented deterministic-replaceable seam). **NO REGRESSION:**
engine `re_out/fist.c` byte-identical (md5 6cd1f94a), 89 patches apply clean, first-light intro intact,
default boot crash-free. **FRONTIER = `FUN_0000_0340` (menu background):** palette-copy + framebuffer
clear (`es=0xA000, cx=64000, call [DGROUP:0x336]=0x57cb`) + palette upload; needs (a) the screen
descriptor `cx:bx` threaded through the `(*c564)()` method-vector call (e3bc sets `bx=0xf6cc, cx=ds`,
dropped), (b) its NULL `rep movs` src/dst rebased, (c) the `word[DGROUP:0x782]==0` palette-buffer
allocation gap resolved. That renders the first menu pixels. Below = pre-first-light history.

## Status (pre-first-light) — PHASE 0 COMPLETE: native build compiles + links + runs (hollow). Now Stage 1 = the shim.
Repo skeleton + git initialized. Recon durable in `docs/recon.md` (READ IT). Ghidra **reused** from
wasm-dd2 via symlink (`third_party/ghidra_12.1.2_PUBLIC`, `jdk-21.0.11+10`; no re-download). Makefile =
the target chain; `make help` works.
- **DONE — architecture solved:** FIST.RUN = Doug-Huffman extender + a ~4 KB 32-bit runtime kernel
  (FILEMGR/MEMMGR → becomes hand-written shim, NOT engine source; decompile kept as
  `re_out/fist_kernel_decomp.c`). **FIST.DAT = the engine** (216 KB, **16-bit segmented**, flat seg<<4).
- **DONE — `make image`:** `tools/extract_dat_image.py` emits `re_out/fist_dat_image.bin` (211212 B,
  base 0, entry linear 0x4, 1143 relocs applied). Verified (entry disassembles as coherent 16-bit DOS
  CRT; 32-bit decode is garbage; far refs + NovaLogic string land at seg<<4). Kernel extractor =
  `tools/extract_image.py` (`make kernel-image`).
- **DONE — `make decompile`** (Ghidra `x86:LE:16 Real Mode` @ base 0): reproducible high-coverage decompile
  → `re_out/fist_decomp.c`. **2245 functions, 83.5% of the code region (0..0x1c000), `unaff_DS/SS` = 0**,
  **71 jump/call tables recovered (2348 targets promoted)**, verified reproducible (`FIST_FRESH=1`, ~40 s).
  Pipeline scripts: `PrepAnalysis.java` (aggressive analyzers + `DS=0x1c00`/`SS=0x2ba9` segment context)
  → `MarkEntry.java` (seed entry) → `RecoverAll.java` (jump-table/prologue/call discovery to a fixpoint)
  → `ExportDecomp.java`. Code/data boundary: **code 0..0x1c000; pure resource data ≥0x20000** (dispatch
  pointer tables 0x1fe00–0x2b800). INT calls render as `swi(0x21)`/`swi(0x33)` intrinsics (= shim traps).
  OPEN: ~16.5% of the code region is register-only-indirect (`call [bx]`/`call dx`) reachable — needs
  value-set analysis, deferred; large tables promote targets but render as opaque calls not C `switch`
  (rendering only, no coverage loss).
- **DONE — Ghidra register-threading (user-directed "tune until the output is clean enough"):** the engine
  is hand-written asm (2242/2245 frameless; Kyle Freeman), so register dataflow (AX/BX/CX/DX/SI/DI/BP) is
  the inter-function interface → was ~43k `in_*/unaff_*` pseudo-vars. Fixed at the DECOMPILE stage via
  `tools/ghidra/ApplyConv.java` (new postScript, wired into `decompile.sh` between RecoverAll and
  ExportDecomp): installs a custom **`__allregs`** prototype model via **SpecExtension** (stored in the
  program DB — NO edit to the shared Ghidra install) where all GP regs (16+32-bit) are candidate inputs,
  AX/EAX the output, and only SP/DS/CS/ES/SS/DF are unaffected (variant A: SI/DI/BP threaded, not
  preserved). Then per function it reads the decompiler's INPUT VARNODES directly (`hf.locRange`,
  `isInput`) and installs them as CUSTOM_STORAGE params + detected return — iterated to a fixpoint (5
  passes) so callee sigs propagate to callers. (Direct varnode read is required: `commitParamsToDatabase`
  only promotes registers that fill the model's input slots contiguously from slot 1, dropping funcs that
  use e.g. DX/SI/DI but not AX/BX/CX.) **Result: 42854 → 1569 register pseudo-vars (96.3%); in_BX
  17208→0, unaff_SI 17564→0, unaff_DI 3195→0, unaff_BP 519→0, in_AX 1716→0, in_stack_ 358→0.** Residual
  1569 is documented patch classes: segment regs unaff_CS/ES (723), flags in_CF/ZF/… (455, no Ghidra
  x86-16 flag-convention slot), multi-register call returns extraout_* (312, incl. INT 33h mouse CX/DX),
  32-bit upper-half sub-regs in_register_* (73), x87 FPU in_ST0-7 (67), stray byte-high/segment (10).
  Reproducible: both `make decompile FIST_FRESH=1` (full regen) and `make decompile` (reuse re-export)
  emit byte-identical threaded C. Tunables: `FIST_CONV_VARIANT` (A default/B), `FIST_CONV_ITERS` (5).
  See `docs/recon.md` "ENGINE IS HAND-WRITTEN ASSEMBLY".
- **DONE — `make assemble` + `make native` (compiles + links + runs hollow):** `tools/assemble_fist.py`
  (8 mechanical rules, never touches `fist_decomp.c`) → `re_out/fist.c` (2245 fns). **Flat memory model:**
  `uint8_t g_mem[0x100000]` = full real-mode 1 MB; image loaded low, DGROUP/stack/heap grow above, VGA fb
  at linear 0xA0000 inside it (max symbol seen 0xaf9fe). Every `DAT_SEG_OFF` → accessor at `(SEG<<4)+OFF`;
  `FUN_SEG_OFF` = C functions. `re_out/ghidra_compat.h` = types + p-code macros + `__allregs` + honest
  `swi/in/out` decls (16-bit width doctrine documented). `tools/build_native.sh` + `tools/native_main.c`
  (owns g_mem, loads image, honest trap stubs). **`make assemble && make native` green, gcc 14.2 `-m32`,
  0 engine errors, reproducible (md5-stable).** Runs: loads image → `app_entry()` → returns without crash
  (hollow — `swi/in/out` are no-op stubs). 35 greppable `OPEN:` markers in `fist.c` = Stage-1/2 scope.
  Honest stubs enumerated: INT 0x21(DOS)/0x33(mouse)/0x2f/0x16/0x10; ports 0x20 PIC, 0x40-43 PIT,
  0x60/61 kbd, DMA, 0x3c0/3c8/3c9/3d4/3da VGA.
- **DONE — Stage-2 reference oracle validated:** the ORIGINAL boots under DOSBox (headless xvfb, DPMI) THROUGH
  the Huffman extender into VGA graphics mode → `tools/refcapture.sh` + `tools/dosbox.conf`, sample
  `ref/boot_frame.png`. TODO(refine): drive to a stable known frame + dump the real 320×200×8 fb+palette.

**DUAL-TARGET INVARIANT LIVE (2026-07-09):** `make wasm` is GREEN (`tools/build.sh` written — emcc, same
units as `build_native.sh`, `native_main.c` reused via `#ifdef __EMSCRIPTEN__`) and the **MAIN MENU is
NATIVE↔WASM BIT-IDENTICAL — 0 differing bytes / identical md5**, hence AE = 0 vs the DOSBox reference on
BOTH targets. Portability seam (all `#ifdef __EMSCRIPTEN__` in `native_main.c`): the SIGALRM host timer →
a cooperative one-tick-per-`fist_timer_pump` time base; SIGSEGV/backtrace + `mprotect` FIST_FBTRAP compiled
out. The sole real WASM delta is `call_indirect` signature-checking of the untyped `code` dispatch surface,
handled at the build layer by `-sEMULATE_FUNCTION_POINTER_CASTS=1` (+ `-sBINARYEN_EXTRA_PASSES=pass-arg=
max-func-params@64` for the 19-param `__allregs` fns) — no engine/call-site edit, output byte-identical;
per-family arity normalization (DD2's doctrine-pure route) remains the long game. Env → node via
`tools/wasm_pre.js` (mirrors `process.env`→`ENV`) + `-sNODERAWFS=1`. See `docs/stage1.md`. (NB `make
verify`/`verify-wasm` still reference a not-yet-written `tools/verify.sh` — separate pre-existing gap.)

**NOW — Stage 1 (crash-free real execution): implement the shim.** The build runs hollow because
`swi()`/`in()`/`out()` are stubs. Stage 1 = make `app_entry` actually execute the game by implementing the
DOS/hardware traps as REAL handlers (INT 21h file I/O via FILEMGR semantics → `fist_filio.c`/`fist_dos.c`;
INT 10h video mode; INT 33h mouse; PIT/keyboard; then VGA 0xA0000 + palette → `fist_vga.c`), plus binding
the residual register/flags pseudo-vars where execution needs them — each an asm-verified `patches/NNN-*.diff`.
Goal: `make verify` runs the boot/attract crash-free on native, then `make wasm`/`verify-wasm` on browser.
Then **Stage 2** bit-identical vs the DOSBox/QEMU reference → **Stage 3** exhaustively playable + editor
(keyboard/joystick/**mouse**), native + browser → the 10× consecutive-clean gate. DD2 precedent:
`~/Git/wasm-dd2/CLAUDE.md`.
