# Armored Fist — format reconnaissance (Phase 0 ground truth)

## ENGINE IS HAND-WRITTEN ASSEMBLY (decisive finding 2026-07-07) — shapes all of Stage 1
- Credits in the binary: **"David Seeholzer, Kyle Freeman and John Butrovich"**, and a standalone
  **"KYLE FREEMAN"**. Kyle Freeman = NovaLogic's Voxel Space engine author, known for hand-optimised asm.
- **2242 of 2245 functions are frameless** (no `55 8b ec` C prologue) — only 3 have a compiled-C stack
  frame. The engine is **almost entirely hand-written assembly**, not compiled C.
- Consequence: there is **NO single calling convention** to configure (the Watcom hypothesis is FALSE —
  it is not uniform C). The pervasive Ghidra pseudo-vars (`in_AX` 1716, `in_BX` 17208, `unaff_SI` 17564,
  `unaff_DI`, `in_CX`, `in_DX`, `in_CF` …) are **real per-function register inputs/outputs**: register
  dataflow threaded implicitly through AX/BX/CX/DX/SI/DI/flags across calls.
- **Stage-1 lever (recommended):** give Ghidra a custom "**all-registers-in, all-registers-out**" calling
  convention / prototype model (.cspec) so every function takes the GP registers + flags as explicit typed
  params and returns them — turning implicit register dataflow into threaded C params. This is the
  faithful, compilable, "fix-at-decompile-stage ⇒ fewer patches" path (the asm analog of DD2's
  recover_regargs, but systematic). Recognisable compiler-emitted idioms (rep movs/stos → memcpy/memset,
  e.g. FUN_0000_3e03) become a small patch class. Alternative (messier, avoid unless needed): a global
  register-file struct read/written by all functions (CPU-emulator style). OPEN: prototype the all-regs
  convention on the current project, measure pseudo-var reduction, confirm compilability.

## ARCHITECTURE (decisive finding 2026-07-07) — the engine is FIST.DAT, not FIST.RUN
- **FIST.RUN** = Doug-Huffman DOS extender + a tiny (~4 KB, 33 fns) 32-bit protected-mode **runtime
  kernel**: CRT startup + `FILEMGR` (file/resource loader) + `MEMMGR`. Only 7.7% of its 49 KB image is
  reachable code. Its job is to enter flat 32-bit mode and **load + exec the game from FIST.DAT**. Its
  strings: `FIST.DAT`, `FILEMGR: Unable to find or open`, `RESOURCE.RES`, `PAL.RES`, `dsounds.bin`,
  `Unknown EXEC error in`. Decompiled to `re_out/fist_decomp.c` (kernel only — keep for the loader logic).
- **FIST.DAT** (216332 B) = **the actual Armored Fist game program.** Single MZ (load-module = whole file,
  1143 relocs, cparhdr=320/5120 B, entropy 5.59 = normal code+data, NOT encrypted, NOT a nested extender).
  Strings are the whole game incl. the **editor**: `(C) Copyright 1994 by NovaLogic, Inc.`, `INTRO.MS3`/
  `MAINMENU.MS3`/`MSN1..3.MS3`, `USER CREATED MISSION`, `EDITING`, `TANK ADDED`, `ERROR SAVING BATTLE!`,
  `ABRAMS`, `TIME REMAINING`. **This is the decompilation target.**
- Consequence: `make image` must extract the loaded flat image of **FIST.DAT** (apply the extender/FILEMGR
  load semantics: load base + the 1143 fixups + 16-vs-32-bit mode), using FIST.RUN's kernel decompile as
  the spec for how the load works. FIST.RUN's own image extract stays as `extract_image.py` (kernel), but
  the engine image needs its own extractor path. OPEN until FILEMGR is read: FIST.DAT load base, fixup
  format, and **processor mode (16 vs 32 bit)** — must be determined from the loader, never guessed.

---
# FIST.RUN kernel — format reconnaissance

Verified facts from `armoredfist/FIST.RUN` (71626 bytes). All offsets are file offsets unless noted.

## Outer structure — two nested MZ images
```
0x00000 .. 0x00539   Stub MZ #1  (1338 B load module)
                      - Real-mode DPMI probe. Prints "Fatal error, DPMI host does not
                        support 32 bit applications" / "80386 processor is required" if
                        launched without a DPMI/VCPI host. Header: cp=3, relocs=2,
                        cparhdr=3(48B), ss:sp=0x50:0x100, ip:cs=0x152:0x0, lfarlc=0x1c.
0x0053a .. 0x117ca   Extender MZ #2  (70288 B load module — reaches EXACTLY to EOF)
                      - The Doug Huffman DOS extender ("DOS extender Copyright 1991-1994
                        by Doug Huffman", string @0x77d). 16-bit protected-mode loader +
                        the BOUND 32-bit application image, in one MZ module.
                      - Header @0x53a: cp=138, cblp=144, relocs=33, cparhdr=10(160B),
                        ss:sp=0x1119:0x4, ip:cs=0x0:0x6f, lfarlc=0x1c.
                      - Code (past 160B header) starts @0x5da; ENTRY @0x0cca (cs*16+ip).
                      - Entry does `e9 33 10` = jmp 0x1036 -> loader body @ file 0x1d00.
                      - reloc table @0x556 (0x53a+0x1c), 33 entries (off,seg).
```
No trailing bound image outside MZ #2. The 32-bit app is *inside* MZ #2; the loader copies
it into extended memory and builds selectors at run time.

## Region hints
- **0x1800..0x5800** — high 0x66-operand-prefix density (~200+/2KB). This is the extender's
  16-bit loader doing 32-bit register work (mode-switch / copy / reloc thunks), NOT the app.
- **~0x5800..EOF (~48KB)** — candidate flat 32-bit app image (USE32; low 0x66 density
  because 32-bit is the default operand size there). BOUNDARY UNCONFIRMED — must be read
  out of the loader's copy logic, not guessed.
- Extender identity: "Doug Huffman" 1991-1994. Same extender family as Comanche / Wolfpack
  (strings `COMANCHE`, `WOLFPACK.EXE` in `LOADGAME.EXE`). Error strings list VCPI/DPMI,
  A20/XMS, page-fault + full register-dump handlers, INT 0Ch/0Dh handlers — a complete
  32-bit protected-mode runtime.

## Launch
`LOADGAME -K400,0,1000 -X5000 FIST.RUN` (`F.BAT`/`FIST.BAT`). `LOADGAME.EXE` = NovaLogic
loader v1.14 by John Butrovich, LZEXE (`LZ91`) packed. It provides/negotiates the DPMI host;
FIST.RUN's stub then hands off to the Huffman extender in MZ #2.

## Extraction strategy (decided)
- `make image` = a **static** parser of the Huffman bound format that emits `fist_image.bin`
  (flat 32-bit app at its real load base) — reproducible, no runtime, analog to DD2's
  `extract_image.py` on the PE. This is the Phase-0 deliverable that unblocks Ghidra.
- DOSBox + QEMU (`qemu-system-i386` + gdb stub / `-icount` record-replay) are the Stage-2
  bit-verify **reference oracle** (analog to DD2's Wine refcapture) and a live-dump
  bootstrap/cross-check for the static extractor — NOT the primary image source.

## Extractor — SOLVED (see tools/extract_image.py; verified 2026-07-07)
Big correction to the earlier "flat plaintext USE32" assumption: the 32-bit image is
**XOR/ROR self-decrypting**. A plaintext stub embedded mid-image checksums then decrypts the
rest in place, then jumps to the real entry. `extract_image.py` replicates the decryption
statically (no runtime) and emits `re_out/fist_image.bin`. Independently verified: decrypted
strings appear (`EXTENDER:`, `PROTMAIN:`, `No CheckSum`), entry disassembles as coherent
32-bit DOS CRT code, embedded checksum matches.

1. **Image on disk:** file `0x0583a .. 0x117ca` (app-off 0 .. 0xbf90, **49040 bytes**).
2. **Load base (linear): 0** — app linked flat at 0; app-off == linear address. Load in Ghidra
   as raw `x86:LE:32:default`, base 0.
3. **Relocations: NONE** — no fixup table; base-0 position-dependent. (The 33 MZ#2 relocs are the
   16-bit *extender's* own self-fixups, not the app's.)
4. **Entry point (decrypted): app-off 0x0d9d** (file 0x065d7) — DOS CRT startup (reads PSP
   cmdline, DOS EXEC/alloc, calls game).
5. **Decryption:** init EAX=0x9324abe1 EBX=0x439a9efa; region1 @0xab4 (4×8B static keys);
   region2 @0xc20..0xbf00 (5724×8B, `ror` keys each step, EAX^=ECX countdown). Checksum
   dword @0xaa8 = 0x088c5c9d over ciphertext.
Extender = **Doug Huffman DOS extender**, DPMI/VCPI, flat 32-bit model (GDT template @ file
0x5ea: base-0 4GB selectors 0x20/0x23 data, 0x28/0x2b code32).

## Still OPEN (low impact)
- **BSS size beyond app-off 0xbf90** — DGROUP zero-extended at runtime; size computed in the CRT
  (routine @ file 0x5175), not in the MZ header (minalloc=0). Extractor emits only the on-disk
  0xbf90. Give Ghidra extra zero-filled space for BSS/heap as needed; exact size TBD from the CRT
  or a runtime trace.
- **Exact extender→stub handoff EIP** (~app-off 0xad8) would need a runtime trace; the
  decrypted-image entry (0x0d9d) that Ghidra needs is statically confirmed.
