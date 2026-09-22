Type: feature
Title: Original and ports share a proven application-start presentation boundary
Parent: 0034

## Contract

Define the same observable launch instant for DOSBox, native and WASM. Capture every frame and
PCM sample from that point, including any output the game launcher or extender actually produces.
Do not drop a prefix merely to obtain a match; distinguish emulator shell from application output.

## Evidence

The 4,151-frame Oracle capture at `scratch/sequence-capture/kdv-profile-full-run/sequence.frames`
starts at DOSBox startup: frames 0–26 are nonblack 640×400 shell screens; frame 26 includes
`Armored Fist (C) Copyright 1994 by NovaLogic, Inc.` below the command line. Frames 27–29 are
black 640×400, and the first 320×200 frame is number 30 at 475,189 µs. The 640×400 period is
14,268 µs. DOSBox hooks (`scratch/sequence-capture/start-boundary-fixed30k/dosbox.log`) show
`LOADGAME.EXE` exec at 0.9987 ms, `FIST.RUN` at 9.606767 ms, `FIST.DAT` at 20.960467 ms,
the game's stdout text beginning at 46.9856 ms, and mode 13 set at 408.1486 ms. The later
first mode-13 presentation is a different event. The captures' 640×400 pixels differ where
their mount paths appear in the DOSBox shell; compare them only with a matched shell state.
Port `tools/native_main.c:main` instead loads `FIST.DAT`, synthesizes the PSP/hardware
handoff and calls `app_entry()` directly. With `FIST_VGA_TRACE=1 FIST_DUMPTICK=20`, native and
WASM each report only mode 13 at 84.987 ms (`scratch/sequence-capture/loader-vga-probe/`).
`re_out/fist_vga.c` captures sequences only in mode 13. These runs prove a start-path/capture
asymmetry; they do not yet prove that the game itself draws 640×400 graphics.
The game's title text is already byte-identical in native and WASM:
`Armored Fist\r\n(C) Copyright 1994 by NovaLogic, Inc.\r\n` occurs once in each captured
stderr (`loader-vga-probe/native-traps.log`, `wasm-console.log`). `re_out/fist_dos.c` DOS
AH=09 sends those bytes only to stderr; its BIOS text services do not render, and
`fist_vga.c` has no text-mode presentation. The first missing application-visible output
is therefore the text screen, not the string producer.
An Oracle hook at `DOS_Execute(FIST.DAT)` captured 4,000 text VRAM bytes at `0xb8000` and
the 256-byte BIOS data area (`loader-vga-probe/oracle-pre-dat.{text,bda}`): mode 3, 80
columns, page 0, cursor `(row 23, col 0)` at 20.960467 ms. Expanding each character with
DOSBox's 8×16 ROM font (`src/ints/int10_memory.cpp:int10_font_16`), foreground `attr&15`
and background `(attr>>4)&7` reproduces **all 256,000 indexed pixels** of the first
640×400 frame at 32.879 ms. Apply the observed AH=09 CR/LF text to that state with normal
80×25 scroll: Oracle frame 1 at 47.147 ms equals the *old* state for rows 0–299 and the
*new* state for rows 300–399, each with zero pixel differences. The scanout is four
100-row parts; a whole-frame text snapshot would be wrong at the transition.
The port now keeps text VRAM at `g_mem+0xb8000` and cursor/mode in the BIOS data area,
loads an optional exact initial state via `FIST_TEXT_STATE=<prefix>.{text,bda}`, routes
DOS AH=09 into it and captures 8×16 text scanout in four 100-row parts. With the captured
initial state, both port targets produce identical complete frame files; their first
text frame's 256 palette entries and all 256,000 pixels equal Oracle frame 2 (stable
post-title state), but at 26.980 ms versus Oracle's 61.415 ms
(`scratch/sequence-capture/text-gmem-{native,wasm}-1/`). GDB places the port's first
title byte at PIT count 12; the Oracle emits it 26.025133 ms after `FIST.DAT` exec.
The missing interval includes DOS loading and executed work, not a justified fixed delay.
The strict full-sequence comparator still fails on the absent port PCM stream.

## Next

1. Capture with a fixed mounted path and retain the pre-`FIST.DAT` 4,000-byte text state,
   BDA cursor and all 256 DAC entries as explicit scenario inputs. Attribute later changes
   to shell, launcher and engine; the first two pixel frames are already characterized above.
2. Measure and model the launch CPU/file/overlay costs before the first AH=09 byte, then
   align the text scanout phase. The port must present the Oracle's pre-title frame and
   mixed old/new transition frame at their real times, not merely its stable post-title
   frame. Add the VGA cursor phase and reached BIOS text services; frame 7 adds 16 cursor
   pixels at `(x=0..7,y=397..398)`, toggling every eight frames.
   Preserve pre-boundary Oracle records; never trim a failed comparison.
3. Run the same timed launch scenario on all three targets; compare full post-boundary frame
   dimensions, pixels, palettes, timestamps and PCM with strict completion checks.

## Accept

The boundary has independent Oracle provenance, accounts for every excluded DOSBox-only frame,
and all application-produced launch output through the first mode-13 frame matches both ports.
