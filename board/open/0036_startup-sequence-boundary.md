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

## Next

1. Capture at a fixed mounted path with a trace of text VRAM, cursor and DAC at the common
   launch instant (before the game's AH=09 output). Attribute later text-screen changes to
   shell, launcher and engine; retain the exec/stdout/mode/presentation events above.
2. Initialize the port's text adapter from that matched initial state. Route DOS AH=09 and
   reached BIOS text services through the adapter; present its 640×400 indexed frames at the
   original retrace times before mode 13. Compare the first changed pixel and palette entry.
   Preserve pre-boundary Oracle records; never trim a failed comparison.
3. Run the same timed launch scenario on all three targets; compare full post-boundary frame
   dimensions, pixels, palettes, timestamps and PCM with strict completion checks.

## Accept

The boundary has independent Oracle provenance, accounts for every excluded DOSBox-only frame,
and all application-produced launch output through the first mode-13 frame matches both ports.
