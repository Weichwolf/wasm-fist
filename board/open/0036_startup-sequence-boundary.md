Type: feature
Title: Original and ports share a proven application-start presentation boundary
Parent: 0034

## Contract

Define the same observable launch instant and initial state for DOSBox, native and WASM.
Compare every presented indexed frame, all 256 palette entries, time and continuous PCM from
that point. Attribute DOSBox shell and application output; never trim a mismatching prefix.

## Evidence

- Oracle `scratch/sequence-capture/kdv-profile-full-run/sequence.frames` starts with 30
  640×400 text frames: shell plus the game's copyright output, then three black frames.
  Mode 13 first presents at 475.189 ms. Shell pixels include the mounted path, so a
  canonical path is required. `start-boundary-fixed30k/dosbox.log` puts `LOADGAME.EXE`
  at 0.998700 ms, `FIST.RUN` at 9.606767, `FIST.DAT` EXEC at 20.960467, its actual
  code entry at 20.997200 (`exec-stage-trace/`), first title byte at 46.985600, and
  the mode-13 set at 408.148600 ms.
- Oracle pre-DAT `loader-vga-probe/oracle-pre-dat.{text,bda}` holds all 4,000 text bytes,
  256 BDA bytes, mode 3, 80 columns and cursor `(23,0)`. DOSBox's 8×16 glyphs plus
  text attributes reconstruct frame 0 with zero pixel differences. Frame 1 is the
  old state in rows 0–299 and the scrolled title state in rows 300–399, also zero
  differences; DOSBox scans four 100-row parts.
- The port now stores text VRAM at `g_mem+0xb8000`, keeps cursor/mode in the BDA, routes
  DOS AH=09 to it and captures text scanout. With `FIST_TEXT_STATE=<prefix>` and
  `FIST_TEXT_PHASE_NS=20960467,20168067` (DAT EXEC, preceding vertical), native/WASM
  frame files are identical; their first five times exactly match Oracle: 32,879,
  47,147, 61,415, 75,683, 89,951 µs (`text-phase-{native,wasm}-1/`). Port frame 0
  has Oracle frame 2's full pixels/palette because the title is emitted too early.
  The strict whole-sequence check still fails on missing port mixed PCM. The existing
  178 flows pass both targets in disjoint groups 45+45+44+44 (`scratch/verify/text-phase-full/`).
- `cursor-trace/dosbox.log`: the current text vertical begins at 20.168067 ms with
  cursor count 1, cell 1840, scanlines 13–14. After the title scroll it is cell
  1920; count 8 makes frame 7's 16 cursor pixels visible, count 16 clears them.
- `boot-ip-trace/dosbox.log`: from DAT entry to first AH=09, DOSBox executes 776,424
  guest instructions, 776,408 in relocated CS `2082` (relative `0xf69`). Four IPs
  `54ce/54cf/54d1/54d2` execute 193,993 times each: `nop; jmp; dec ax; jne` in
  `FUN_1000_4b5e`. Original call inputs are `0,59849,40799,27809`
  (`boot-call-trace/`); its first input comes from `FUN_0000_f738`, whose asm leaves
  AX=0. The port's missing return instead passes 287 and its 32-bit C loop charges
  no virtual CPU time. Oracle uses fixed 30,000 CPU cycles/ms.

## Next

1. Patch `FUN_0000_f738` to return its asm-proven AX=0. Make `FUN_1000_4b5e` wrap AX at
   16 bits and charge the four executed guest instructions per iteration through a
   fractional 30,000-cycles/ms → PIT clock. Keep scanout/interrupts interleaved. Compare
   original/native/WASM from frame 0; profile any residual timing difference.
2. Capture a fixed-path initial state including DAC and cursor phase. Implement the
   measured cursor blink and reached BIOS text services; match every launch frame and
   the transition to mode 13 without excluding application output.
3. Complete the strict PCM and subsequent sequence comparison with 0034/0003.

## Accept

The common boundary has independent Oracle provenance, accounts for excluded emulator-only
frames, and every application-produced frame/time/palette/PCM event through the first
mode-13 presentation matches native and WASM with complete captures.
