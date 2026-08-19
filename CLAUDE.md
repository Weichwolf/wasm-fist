# Armored Fist (1994) → native + WebAssembly

## Das Ziel

A faithful, **1:1** rebuild of NovaLogic's *Armored Fist* (DOS protected-mode, `FIST.RUN`/`FIST.DAT`)
that is **bit-identical to the original** under DOSBox/QEMU: given the same input and RNG state, every
frame produces the same 320×200 palette framebuffer and the same audio stream — on **native and in the
browser (WASM)**, and the two builds are **byte-identical to each other** (hard invariant, checked after
every change).

**Done** = the DD2 completeness standard, nothing skipped: every menu / screen / dialog, every campaign
mission and map, every setting (detail, sound devices, input modes, serial link), and the built-in
level/mission **editor** with a bit-verified round-trip (create → save → reload → simulate, byte-identical).
The gate: a subagent confirms the WASM build error-free and fully functional **ten consecutive times** on
the complete exhaustive test matrix — one failure resets the count to zero.

## Die Erfolgsgeschichte: wasm-dd2

`~/Git/wasm-dd2/` (Destruction Derby 2 → WASM) is the **proof the whole chain works end-to-end** — the same
method took a DOS-era commercial game all the way to a complete, bit-verified native+WASM port through many
patient test/fix iterations. Armored Fist shares **no code** with it (DD2 is Win32/DirectDraw; AF is 32-bit
DOS protected-mode), but it inherits the **method and the discipline** verbatim. AF is in several ways the
*easier* target: no COM/Win32/DirectDraw layer, one linear 320×200×8bpp framebuffer, and it runs
deterministically under DOSBox/QEMU. **Es ist schon einmal gelungen — es gelingt wieder.**

## Das Vorgehen (der reproduzierbare Chain)

Everything starts with `make` — the target chain *is* the documentation (`make help`):

```
make image → decompile → assemble → patch → native / wasm → verify / verify-wasm
```

- **image / decompile** [done, reproducible] — Ghidra headless (`x86:LE:16 Real Mode`, base 0) →
  pristine `re_out/fist_decomp.c` (2245 fns), register dataflow threaded via the custom `__allregs`
  model. **The decompile stays pristine — never hand-edit it.**
- **assemble** — mechanical `fist_decomp.c` → `re_out/fist.c` (flat `uint8_t g_mem[]` model, forward
  decls, `ghidra_compat.h`).
- **patch** — ordered `patches/NNN-slug.diff` applied `-F0 --fuzz=0` onto `re_out/` → `build/`. **Every
  engine correction is one commented, asm-verified patch** with its rationale in the header. Drift fails
  loudly.
- **native** (gcc `-m32`) / **wasm** (emcc). **verify / verify-wasm** — `tools/verify.sh` runs the
  bit-verify flow matrix on both targets.

**Hand-written = only the platform shim** (`fist_*.c`, `tools/native_main.c`): VGA (linear `0xA0000` +
DAC → framebuffer), DOS/FILEMGR (INT 21h), mouse/keyboard/joystick (INT 33h / INT 9 / port 0x201), SB/GUS
+ OPL audio, timer (PIT/INT 8), the extender/overlay loader, indirect-call dispatch. Editing the shim is
fine; everything else is decompile + patches.

**What the target is:** `FIST.RUN` = loader + Doug-Huffman DOS-extender (→ shim). **`FIST.DAT` (216 KB) =
THE ENGINE** and the decompile target: 16-bit segmented, base 0, almost entirely hand-written asm (Kyle
Freeman) — NovaLogic **Voxel Space** terrain (Comanche lineage). Flat model: `g_mem`, **DGROUP at linear
0x1c000** (SS=DS=DGROUP=0x1c00), VGA fb at `0xA0000`. Data under `armoredfist/FISTDATA/` (`.KLC` maps,
`.MS3` missions, `.FSG` battles, `.M##` models, `.FSG/FSE/FSW` sounds/briefings, …).

## "Code ist die Wahrheit" — der Oracle

Verify **every** claim against the original: `armoredfist/FIST.DAT` (static / `objdump -m i8086`) or a live
run. Two oracles are installed: **DOSBox** (fast, hackable — instrument it to dump FB/RAM/ports; reaches
missions) and **QEMU** (`-s -S` gdb-stub, `-icount` record/replay — true protected mode). A rebuilt
instrumented `dosbox-fist` + capture scripts under `tools/oracle/` dump full guest RAM at a chosen tick;
the engine's DGROUP relocates to **guest phys 0x2d190**, so `oracle[0x2d190+X] ≡ port g_mem[0x1c000+X]` —
any engine field is directly comparable to the original. No approximations, no band-aids, no stubs: **any
observable deviation from the original is by definition a bug.**

## Die Grundhaltung: "geht nicht gibt es nicht"

The whole chain is already proven by wasm-dd2, so **nothing here is unreachable** — only bounded,
decomposable work. When something looks hard, it is *not* a reason to stub or declare a surface out of
reach; it is the next thing to decompose into a small, verifiable step. **One verified step at a time.**

- **Land small, verify immediately.** A single asm-verified patch that makes one flow bit-identical on
  both targets is a real win. Commit it, `verify.sh both`, move on.
- **Iterate and correct without ego.** A failing test means a bug in the *code*, not the test. A wrong
  hypothesis, disproven against the code, is progress — it eliminates a dead end. Correcting yourself is
  the method working, not a setback.
- **Widerstand ist Information.** If something resists, it means there's something not yet understood —
  stop, look at the asm / the oracle, then take the next small step. Never improvise a guard around it.
- **Momentum beats perfection.** Pick the most tractable next flow, make it green on native + wasm, bank
  it. Breadth grows one bit-verified surface at a time until the 10× gate falls.

## Konventionen

- `re_out/fist.c` pristine; corrections are `patches/NNN-*.diff`; `make check` must apply all patches.
- Verify **both targets after every change** (native ↔ wasm byte-identical is a hard invariant).
- Original files under `armoredfist/` present at run time. `third_party/` + extracted images gitignored.
- Git: no "Co-Authored-By: Claude" / "Generated with Claude Code" attribution in commits or PRs.

## Wo wir stehen

The **entire front-end is bit-verified on both targets** — intro, all 7 main-menu items, every SETTINGS
toggle, all list-dialog interactions (select/scroll/page/cancel), OK-save, campaign/battle drill-down,
briefing — plus 26 op-0x24 mission cockpit-chrome frames, **21 op-0x2c FSG-battle cockpits**, the **`.FSG`
editor round-trip for all 47 battles**, and THREE real editor edit-ops each over a theater spread:
**ADD-TANK** (patch 362, real b21d friendly roster-insert, base→base+1), **ADD-ENEMY** (patch 406, b21d's
enemy-pool branch 0xc05c, base→base+1) and **REMOVE-TANK** (patch 405, real b2ef object-destroy,
base→base-1), all bit-verified as idempotent fixed points, PLUS the editor **"simulate" leg** for add and
remove (the edited .FSG loads + spawns its op-0x2c cockpit AE=0 -> create→save→reload→simulate closed):
**159 `tools/verify.sh` flows, AE=0 native + wasm, native↔wasm 0-diff.** The reproducible chain is solid;
`re_out/fist.c` pristine; the shim covers VGA/DOS/mouse/audio/extender.

**Mission-render coverage for the M1 chrome is COMPLETE** (a full 47-battle op-0x2c spawn scan + a
14-ref cross-match): every M1-cockpit battle is bit-verified via op-0x24 and/or op-0x2c against a genuine
DOSBox ref; INDIA3 is the SOLE crasher (near-heap OOM); and exactly **10 battles render a DIFFERENT,
non-M1 central chrome** (azer6/cyprus4/saudi5 share one; cyprus5/saudi6/syria5/syria7/train1/ukraine3/
ukraine6 span several more — AE 64..86 vs the M1 refs, deterministic). Those 10 are faithful-but-unverified
pending their OWN vehicle DOSBox refs (multiple distinct chromes -> multiple oracle captures needed).

**The mission-paint "crash bucket" is cracked** (patches 397-399): the in-mission windshield/HUD dispatch
reached four un-ported per-vehicle sprite-animation element methods (8039/7681/87cc/90bf + their
state-setters) whose raw Ghidra host-ptr derefs SEGV'd on reach — NOT the wild-writer / spurious-viewport
/ fb_seg red herrings earlier sessions chased (all disproven with **non-perturbing** gdb-hw-watchpoint
data; mprotect single-step + `FIST_COOP_TICK` corrupt the garbage-dependent crash — always validate
rendering under the real-timer *verify* condition, never COOP_TICK).

**Oracle without the instrumented dosbox**: `tools/oracle/capture_battle_stock.sh` drives the ORIGINAL
FIST.RUN under stock DOSBox + Xvfb + ctypes-XTEST (`xclick.py`, no root/xdotool/headers) and grabs the
320×200 spawn — AZER1 M1-spawn central-chrome AE=0 vs the port, and AZER6 settles to AE=2 vs the port's M1
(so AZER6's player is **M1**, faithful). **Caveat, hard-learned:** the oracle menu nav is NON-deterministic
— the battle-list row-select and, worse, the original's **auto-cycle** (after a variable idle it swings the
view to OTHER units, e.g. a steering-wheel/M3 vehicle) make a single row-selected grab unreliable. A grab
that catches the cycle looks like a "wrong vehicle" but is not. Validate a specific battle only from a
selection-zoom-verified capture that settles to AE≈0 vs the port in the M1-spawn window (~ACCEPT+24..31s),
and cross-check the on-screen goal count. (A premature "port renders the wrong vehicle" claim here was a
cycle-frame artifact and was retracted; the 12 op-0x2c flows stand.)

**INDIA3 is the SOLE crashing battle** — a full 35-battle op-0x2c spawn scan proved every *other* FSG
battle renders its cockpit crash-free (34/35). INDIA3's mga-icall SEGV chain is fully ported through
patches 400-404 (7eb7 base-loss, 84c3/7eba icall arg-threading, 8390 viewport, 852f base-loss); the crash
now advances into the engine MEMMGR as a **genuine, deterministic near-heap OOM**: the cockpit-instrument
bbox-analyzer (2004) allocs buf1(0xc00)=192 paras OK then buf2(0x140)=20 fails on a fragmented ~437-para
free pool (l0c_max=223). Root-caused port-only as far as it goes — pool size (~480KB), patch 401 (84c3),
patch 332 (2004 alloc sizes 0xc00/0x140), and the allocator (0a31/1040/0c7d faithful `stc;ret`) are ALL
asm-verified faithful; at op-0x18 INDIA3≡AZER3 byte-identical (free=1681), so the divergence is an
INDIA3-specific over-consumption in the synchronous post-map cockpit-setup cascade (INDIA3 reaches 2004
via 84c3, children 0x8f8c/0x8fa8/0x8fc4, guarded once by ds:0x8f8a; AZER3 via a different caller and nets
+free). **The last question is oracle-only**: does the ORIGINAL have more near-heap headroom at 84c3-entry
(guest phys 0x2d190+0x16d8)? — shim under-provision vs engine over-alloc. (Diagnostics: shim env
FIST_DBG_MM dumps the d6d4 pool free/desc/l0c-max at op-0x18/0x2c; a `[2004]` build/fist_mga.c entry log
gives the per-call trajectory.)

**Other open frontiers**: the 23 crash-free-but-different-chrome battles need their own DOSBox refs
(different vehicle / dynamic instruments); the windshield **voxel terrain** (AZER1/AZER6 render it in the
oracle); audio bit-exactness; more editor edit-ops (modify-unit; the interactive 4c7a map-click add is
blocked on the post-6015 map-view frontier) + the editor "simulate" round-trip leg; save/load; controls —
one green flow at a time, all the way to the 10× gate. **Los geht's.** 🚀
