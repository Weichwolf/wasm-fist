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
- The port captures text VRAM, BDA cursor, 8×16 glyph scanout and the source's cursor
  blink. `FIST_TEXT_STATE=.../loader-vga-probe/oracle-pre-dat` and
  `FIST_TEXT_PHASE_NS=20960467,20168067` select the original DAT-EXEC state/vertical.
  Against the matching-path `start-state-probe/sequence.frames`, native and WASM now
  match **every byte and time of frames 0–26** (640×400 indexed pixels and 256-entry
  palettes); their complete 51-frame files are identical (`text-boot-{native,wasm}-6/`).
  The strict full-frame comparison first fails at event 27: Oracle 418,117 µs,
  black 640×400; port 483,574 µs, 320×200 before the resize model. Mixed port PCM is still absent.
  `make check`, 29 tests and all 178 existing flows pass both targets with exact
  45+45+44+44 coverage (`scratch/verify/startup-phase-g{0..3}/`).
- `cursor-trace/dosbox.log`: the current text vertical begins at 20.168067 ms with
  cursor count 1, cell 1840, scanlines 13–14. After the title scroll it is cell
  1920; count 8 makes frame 7's 16 cursor pixels visible, count 16 clears them.
- `boot-ip-trace/dosbox.log`: from DAT entry to first AH=09, DOSBox executes 776,424
  guest instructions, 776,408 in relocated CS `2082` (relative `0xf69`). Four IPs
  `54ce/54cf/54d1/54d2` execute 193,993 times each: `nop; jmp; dec ax; jne` in
  `FUN_1000_4b5e`. Original call inputs are `0,59849,40799,27809`
  (`boot-call-trace/`); its first input comes from `FUN_0000_f738`, whose asm leaves
  AX=0. Patch 618 returns AX=0, wraps the loop at 16 bits and charges its four
  instructions per iteration through a fractional 30,000-cycle/ms clock. The BIOS
  PIT begins in mode 3, whose latched count falls by two per PIT tick (DOSBox
  `timer.cpp`); the port now reads it that way. Patch 619 restores the six BIOS-tick
  wait at `f6e9` that patch 004 had omitted.
- `oracle-timer-619/` versus `text-boot-native-5/timer-stages-phase-gdb.log`:
  first `30de` retrace return is 332.893233 versus 332.893054 ms after anchoring
  text VGA status to the measured vertical. `515e` begins at 404.240700 versus
  404.267748 ms, but the mode-13 call is 408.148600 versus 404.889614 ms.
  `oracle-{exec,hist,sndhist,sndcal}-619/` isolates the gap: the two `4B/03`
  overlay loads take only 22/11 µs, and there are no DOS `3F` reads. The
  SOUNDDVR cluster executes 86,583 instructions between them, including 42,240
  hits each at the `07f4` delay loop's `dec ax/jne`; its 26 calls use calibration
  values 90→114. Patch 620 restores the 16-bit return and charges the asm loop's
  CPU instructions; the port now gets 86→110. DOSBox's I/O delay guard suppresses
  PIT2 port delays near the end of a CPU slice (`oracle-pit2-620/`), which the port
  does not yet reproduce. The port mode-13 call is 407.410 ms versus 408.149 ms.
  DOSBox `VGA_StartResize` delays renderer setup by 50 ms;
  Oracle presents three black 640×400 frames after the mode-set call.
- With patch 620, native/WASM full 51-frame captures match each other exactly and both
  retain the 27-event Oracle prefix (`text-boot-{native,wasm}-7/`). `make check`,
  29 unit tests and all 178 flows pass on both targets with exact disjoint
  45+45+44+44 coverage (`scratch/verify/snd620-g{0..3}/`).
- `oracle-iocal-621/` records the SOUNDDVR I/O contract directly: DOSBox uses write/read
  delays of 21/29 CPU cycles (0.700/0.967 us at the fixed 30 MHz rate) while the current
  slice has at least three such delays left; it applies zero delay below that threshold and
  refills the slice after the decoder returns. The port has no CPU-slice state yet, so its
  calibration remains 86→110 rather than 90→114.
- Replacing the port's one-PIT-count access cost with only the 21/29-cycle base moves its first
  DAC reset from 496.943 to 497.070 ms and mode 13 from 407.410 to 407.289 ms. It cannot close
  the 503.001-ms target and is discarded: a global base-cost substitution is not the decoder
  slice contract.
- The port models `VGA_StartResize`: after the mode-13 BIOS call it keeps the active
  640×400 scanout, clears its three pending presentations, then changes drawing mode at the
  measured 50-ms setup event without resetting the vertical phase. The strict comparison now
  matches events 0–34 exactly; event 35 (546,529 µs) first differs at palette byte 5
  (`170 != 0`) and the port thereafter draws index 24 (`text-boot-{native,wasm}-8/`).
- The event-35 palette producer is the engine's MGAVIDEO DAC method (`4ec3:0bf2..0bff`),
  reached from the INT-8 service. Original resets the DAC at 503.001 (`[0x452]=1`),
  532.648 (`3`) and 546.917 ms (`4`); the port does so at 496.943, 526.281 and
  540.951 ms. The port therefore clears the DAC about 6 ms before event 35, while the
  Oracle clear follows it at 546.972 (`kdv-normal-trace/`, `text-boot-native-9/`).
- `oracle-dacreq-624/` separates the BIOS mode-set DAC reset at 408.278 ms (`f000:1304`,
  tick 1) from the first MGAVIDEO reset at 503.001 ms. The port initializes its DAC state
  directly during mode 13, so this BIOS write has no pre-event-35 observable difference;
  it is not the later engine producer.
- Port `op` tracing places the first KDV extender `0x6c` at 512.152 ms, after the first
  MGAVIDEO reset at 496.943 ms. Its one-IRQ yield cannot account for the earlier gap.
- `oracle-gap-profile-627/` counts 1,763,172 original instructions from the BIOS mode-set DAC
  reset through the first MGAVIDEO reset (408.582→503.000 ms). The 1,643,811 hottest instructions
  are the `[0x44e]` wait at running `2082:439b`, whose bytes belong to `FUN_1000_3a14` under the
  live CS mapping, not the similarly shaped `4bb7/4bcc` Ghidra identities. Patch 044 already
  implements 3a14's two `[0x44e]` tick waits. Replacing unrelated 3346/4bb7 bodies does not alter
  the capture and is discarded.
- `oracle-calib-631.profile` splits that wait: original `3a14` enters at 447.171700 ms and leaves
  its first `[0x44e]` loop at 503.568200 ms (56.396500 ms). The port trace
  `port-3a14-630/port.log` enters at 455.471169 and leaves at 511.869103 ms (56.397934 ms).
  The loop contract therefore matches; the 8.299469-ms displacement already exists at entry.
  Its subsequent `0x201` reads take 1.000 µs in DOSBox and the port reaches the next tick at
  525.893787 ms. Do not alter patch 044 to compensate for an earlier divergence.
- `FUN_1000_30de` is the preceding CRTC-status wait (`in 0x3da`). Oracle calls end at
  418.499233, 432.767300 and 447.035400 ms; the port ends them at 426.882068, 441.150638 and
  455.419207 ms. The second and third each take 14.266667 ms in both runs. The first port wait
  is 1.151967 ms too long, but its entry is already 7.231084 ms late. `oracle-timer-init-638.reg`
  places the matching second `2fd3` timer calibration at 408.582200 ms; port-timer-636 enters it
  at 415.804965 ms. The remaining primary gap is therefore between the mode-set return and `2fd3`,
  not joystick timing or the recurrent CRTC wait.
- The `2fd3` caller is `MGAVIDEO:00e8`, call site `4ec3:0132`. Its port trace gives entry
  415.784851, INT 10 completion 415.791556 and `2fd3` return 455.430940 ms. Oracle's
  `oracle-mga00e8-640.reg` gives entry 408.127900 ms. The whole driver mode-call is therefore
  already 7.656951 ms late before its INT 10 invocation; inspect the caller of `00e8` next.

## Next

1. Trace the caller that enters `MGAVIDEO:00e8`; it is 7.656951 ms late before INT 10. Then recover
   the first `30de` wait's 1.151967-ms phase error. Align the first four MGAVIDEO DAC calls without
   hiding the event-35 palette difference. Preserve events 0–34 as a strict prefix of the full
   comparison.
2. Recover the remaining PIT2/port-61 CPU-slice I/O timing in SOUNDDVR `07b7` so calibration
   starts at 90 and evolves as measured. Recheck mode-set time and mixed audio.
3. Complete continuous PCM and subsequent sequence parity with 0034/0003.

## Accept

The common boundary has independent Oracle provenance, accounts for excluded emulator-only
frames, and every application-produced frame/time/palette/PCM event through the first
mode-13 presentation matches native and WASM with complete captures.
