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
320×200 spawn (AZER1 M1-spawn central-chrome AE=0 vs the port). **The non-deterministic auto-cycle is
SOLVED by BURST capture** (proven for AZER6): grab a burst of ~40 frames across ACCEPT+20..40s (row at
OY+123 for AZER6, OX/OY=192/184), crop each central chrome, and SELECT the frame whose dashboard matches
the port — the cycle (the original swings the view to friendly units incl. a friendly M1 every few
seconds) is no longer a blocker, you just capture through it and pick the right frame. This makes reliable
per-battle oracle refs achievable.

**AZER6 resolved (corrects the earlier "AZER6 is M1" claim, which was a friendly-M1 CYCLE frame):** AZER6's
player is a **distinct NON-M1 vehicle**. The port's AZER6 op-0x2c central chrome matches the original's
non-M1 view across every cycle pass at **AE=2 (MIN over all 40 burst frames — persistent, not timing)** —
i.e. the dashboard is faithful except a **real 2-pixel bug at abs(114,135)+(114,136): port renders BLACK
(0,0,0) where the original renders GRAY (85,85,85)/(77,73,73)**. The full-frame delta (~29 290 px) is the
**windshield voxel terrain** (the open frontier), not the dashboard. So per-vehicle mission-render fidelity
= (a) tiny dashboard micro-bugs like this AZER6 2px + (b) the windshield voxel — both deep-RE, now
oracle-verifiable via the burst method.

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

**THE central remaining frontier — the windshield VOXEL TERRAIN.** All op-0x24/op-0x2c flows verify only
the DASHBOARD (central chrome cols80-180 rows96-188); the actual game view above it is UNVERIFIED and
mostly WRONG. Scoped on AZER1 (dashboard AE=0, i.e. a clean M1 spawn via the burst tool): the windshield
(rows 0-95) is **75.5% different from the original (23 197 / 30 720 px)** — only the top ~5 sky rows match,
the terrain (rows ~5-85) differs ~280 px every row. So the port's NovaLogic Voxel-Space terrain
(m_ext_FUN_0000_82b8 render / 8120 camera→projection / 9200 per-column texel walk, in fist_ext.c) is NOT
bit-exact. This blocks the mission-render DoD for EVERY battle and is deep RE (existing tooling:
tools/oracle/{capture_9200_framematched.sh, sim_voxel6980*.py, trace_terrain.sh, dosbox_vga_terrain_trace.patch}).

**Voxel state — HONEST, experiment-grounded (supersedes an earlier colour-gate over-claim):** the DEFAULT
windshield render is `m_ext_FUN_0000_9200` (per-column texel walk) sampling the terrain colour TILE at
`[0x3918]` (bc9c's blend matrix). The `sim_voxel6980_framematched.py` colour-gate (6980 reading
`[0x85bc]+0x100000`) is an EXPERIMENTAL render path (FIST_TILEFILL), NOT the default — a FIST_VOXEL_CONTIG
fix that rebuilt a contiguous HM+reduce for 6980 changed the default frame by 0 px (removed). What IS solid:
  - The port DOES build the correct LIGHT reduce (dist=78, max=228) at `[0x85b8]` in the default path
    (confirmed by the FIST_HMDUMP `[0x85b8]` bank, identical under default / FIST_TILEWIN / FIST_NOTILEALIAS).
  - Oracle-CONFIRMED the default terrain is **MIS-COLOURED**, not mis-shaped: on the AZER1 M1 spawn
    (dashboard AE=0 via the burst tool) the terrain band (rows 5-85) shares only **31 colours** with the
    original (port 131 vs oracle 108 distinct) — a **13% colour-multiset overlap**.
  - The colour source 9200 walks is the tile `[0x3918]` (dist=176, max=255) — and FIST_TILEWIN=0x4200 does
    NOT change it (nor `[0x85bc]+0x100000`). So the default-path defect is DEEPER than the 6980 colour-gate:
    it is in the bc9c `[0x3918]` tile BUILD and/or 9200's INDEXING into it — still OPEN.
**DECISIVE narrowing (wrong-tile vs wrong-index — settled):** the port's `[0x3918]` tile matches the
original's captured tile (sample `voxel6980_framematched_pass08`, the "settled tile" dist=66/mean=199.3) at
only **0.5%** (port dist=176/mean=110.7). So it is a **WRONG TILE, not wrong indexing** — the bug is bc9c's
tile BUILD, downstream of a CORRECT input: the block-A blend matrix (DSOUNDS.BIN @0xe00, 64KB) is 100%
byte-identical to the oracle groundtruth `tools/oracle/samples/oracle_bdc4_matrix_blockA_0x141000.bin`. So
bc9c/bdc4 turn a correct block A into a wrong tile in the DEFAULT map-load. **Input is CONFIRMED correct:**
the shim's DEFAULT op-0x18 path (native_main.c ~2204-2212, UNCONDITIONAL — not FIST_TILEFILL-gated) preloads
block A into [bc90] via fist_preload_blockA, and a FIST_BLKADUMP shows that [bc90] is 100% byte-identical to
DSOUNDS.BIN block A. So the earlier "default path doesn't supply block A" hint is WRONG — block A IS supplied.
**The defect is therefore squarely inside the bc9c/bdc4 tile-BUILD** (algorithm or its write-vs-read window):
per the FIST_BBDUMP note, bc9c writes the flat M[ch][cl] matrix at the 64KB-aligned block base while 9200
reads the tile at the +0x4200 window ([0x3918]), and patch-289's bc90->tile alias was meant to reconcile
that but the result is still a 0.5%-match tile. **NEXT:** read FUN_0000_bc9c + FUN_0000_bdc4 in
re_out/fist_ext.c against the asm; fix the tile-build (the +0x4200 window / the alias / the M[ch][cl] index
math) so `[0x3918]` -> the original dist=66/mean=199 tile; verify vs the burst-oracle windshield, guarding
the 159 dashboard flows. This is the deepest, actively-worked layer (patch-289 + the tools/oracle voxel docs).

**Other open frontiers**: per-vehicle dashboard micro-bugs (e.g. AZER6's confirmed 2px) on the ~10 non-M1
battles — now oracle-verifiable via `capture_battle_burst.sh`; audio bit-exactness; modify-unit editor op
(64ea is a modal, map-view-gated); save/load; controls — all the way to the 10× gate. **Los geht's.** 🚀
