Type: feature
Title: Original guest RAM can be captured and compared with port state

This engine can be investigated against mission-time RAM captured from the original under DOSBox.

## Evidence

`third_party/dosbox-fist` and `tools/oracle/dosbox_vga_terrain_trace.patch` provide instrumented
captures. `tools/oracle/capture_tcb_camera.sh` drives mission entry with FIST_R9200CAP and emits
camera metadata, full guest RAM, VRAM, palette and terrain inputs. The first AZER1 capture resolved
the previously unknown spawn camera.

## Preserve

Resolve engine-linear addresses through current segment bases and CR3; port DGROUP 0x1c000 is not
a fixed guest physical address. Use FIST_WATCHFLAT with FIST_MEMARM_BOOT/FISTLOG for writes.
For stage proofs, snapshot inputs and outputs at the same call boundary (board:0002).
This item was formerly numbered 0007; memory-manager work formerly numbered 0024 is now 0025.
