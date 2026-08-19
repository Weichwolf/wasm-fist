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
briefing — plus 26 mission cockpit-chrome frames, the AZER3 op-0x2c cockpit (oracle-anchored), and the
**`.FSG` editor round-trip for all 47 battles**: **105 `tools/verify.sh` flows, AE=0 native + wasm,
native↔wasm 0-diff.** The reproducible chain is solid; `re_out/fist.c` pristine; the shim covers
VGA/DOS/mouse/audio/extender.

**The mission-paint "crash bucket" is cracked** (patches 397-399): the in-mission windshield/HUD dispatch
reached four un-ported per-vehicle sprite-animation element methods (8039/7681/87cc/90bf + their
state-setters) whose raw Ghidra host-ptr derefs SEGV'd on reach — NOT the wild-writer / spurious-viewport
/ fb_seg red herrings earlier sessions chased (all disproven with **non-perturbing** gdb-hw-watchpoint
data; mprotect single-step + `FIST_COOP_TICK` corrupt the garbage-dependent crash — always validate
rendering under the real-timer *verify* condition, never COOP_TICK).

**Oracle without the instrumented dosbox**: `tools/oracle/capture_battle_stock.sh` drives the ORIGINAL
FIST.RUN under stock DOSBox + Xvfb + ctypes-XTEST (`xclick.py`, no root/xdotool/headers) and grabs the
320×200 spawn — AZER1 M1-spawn central-chrome AE=0 vs the port. M1-spawn window ≈ ACCEPT+24..30s, then the
original auto-cycles the view to other units.

**KNOWN PORT BUG (next-priority)**: the port selects the WRONG player vehicle for op-0x2c battles whose
player is not M1. Oracle-confirmed: AZER6's player is a steering-wheel/M3 vehicle, but the port renders an
M1 cockpit (both via FIST_FSG_BATTLE and real menu nav). Its roster HAS M1+M3 units; the port picks an M1
as the player instead of the designated M3. This is why the ~10 "distinct-chrome" battles don't match M1
refs — and why 11 op-0x2c flows that reused M1 refs were reverted (integrity: an M1-ref match does NOT
prove faithfulness when the port may render the wrong vehicle). Fix the player-vehicle selection → the
op-0x2c cockpit harvest re-opens with per-battle oracle refs.

**Other open frontiers**: INDIA3 SEGVs deeper in the mga sprite-sheet builder (`m_mga_FUN_0000_2004` via
the `84c3` icall — a cross-driver register-threading issue; patch 400 fixed its first stage `7eb7`); the
windshield **voxel terrain** (AZER1/AZER6 render it in the oracle); audio bit-exactness; more editor
edit-ops; save/load; controls — one green flow at a time, all the way to the 10× gate. **Los geht's.** 🚀
