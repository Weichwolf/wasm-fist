# wasm-fist — Armored Fist (FIST.RUN) → WebAssembly

A faithful port of NovaLogic's **Armored Fist** (1994) to reproducible C compiling to native and
WebAssembly. The engine C is **mechanically derived from the binary via Ghidra** (decompile → assemble/patch
→ compile); only the platform/runtime shim (VGA Mode 13h → canvas, Sound Blaster/GUS → WebAudio, DOS/DPMI
→ host) is hand-written.

Armored Fist is a 32-bit **DOS protected-mode** program (bound Doug Huffman DOS extender, launched by
NovaLogic's `LOADGAME.EXE`) running NovaLogic's **Voxel Space** terrain engine — the same lineage as
*Comanche: Maximum Overkill*. Graphics are VGA 320×200×256; sound is Sound Blaster / Gravis UltraSound.

Pipeline: `FIST.RUN → make image → make decompile → make assemble → make patch → make native / make wasm`.
The original bit-for-bit reference is captured by running the game under **DOSBox**.

This project reuses the *methodology* of its sibling `wasm-dd2` (the decompile → patch-series → dual-target
→ bit-verify discipline) but shares no code — DD2 is Win32/DirectDraw, Armored Fist is DOS/VGA.

See **CLAUDE.md** for the recon findings, planned build commands, current status, and conventions.

## Status
Phase 0 — planning and format reconnaissance. The pipeline is specified in CLAUDE.md; nothing is built yet.

## Layout
- `armoredfist/` — the original game (`FIST.RUN`, `LOADGAME.EXE`, drivers, and `FISTDATA/`), present at run time.
- `CLAUDE.md` — plan of record, recon, conventions.
- (planned) `Makefile`, `tools/`, `re_out/`, `patches/`, `web/`, `third_party/` (gitignored).
