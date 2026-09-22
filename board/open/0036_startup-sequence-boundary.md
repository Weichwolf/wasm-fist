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
14,268 µs. Port `tools/native_main.c:main` instead loads `FIST.DAT`, synthesizes the PSP/hardware
handoff and calls `app_entry()` directly. With `FIST_VGA_TRACE=1 FIST_DUMPTICK=20`, native and
WASM each report only mode 13 at 84.987 ms (`scratch/sequence-capture/loader-vga-probe/`).
`re_out/fist_vga.c` captures sequences only in mode 13. These runs prove a start-path/capture
asymmetry; they do not yet prove that the game itself draws 640×400 graphics.

## Next

1. Instrument DOSBox at the `LOADGAME` exec, FIST.RUN entry, first application text output,
   VGA mode switch and first mode-13 presentation. Attribute each 640×400 change to shell,
   launcher or extender and record the chosen common application-start boundary.
2. Make the port's capture begin at that boundary and implement any application-visible launch
   output or timing omitted by its direct `FIST.DAT` entry. Preserve the Oracle pre-boundary
   record separately; never hide it by trimming a failed comparison.
3. Run the same timed launch scenario on all three targets; compare full post-boundary frame
   dimensions, pixels, palettes, timestamps and PCM with strict completion checks.

## Accept

The boundary has independent Oracle provenance, accounts for every excluded DOSBox-only frame,
and all application-produced launch output through the first mode-13 frame matches both ports.
