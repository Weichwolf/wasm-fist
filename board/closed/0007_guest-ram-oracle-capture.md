Type: feature
Area: oracle
Tags: oracle instrument

A guest-RAM oracle capture reaches a chosen mission tick in the ORIGINAL and dumps
guest RAM, so any engine or extender field is byte-comparable to the port's g_mem:
the engine DGROUP relocates to a known guest-physical base, and the extender's TCB
and near-heap pool are readable there. This is the single capability that lets a
port field be checked against the original at a mission spawn rather than only at
the framebuffer.

Two routes exist and either satisfies this: an instrumented DOSBox (dosbox-fist)
that reaches missions and dumps RAM at a tick, or QEMU driven into a mission with
pmemsave. QEMU currently boots the engine but its render loop stays frozen at the
solid clear (a VGA-retrace / PIT-timing difference), and dosbox-fist needs a dosbox
source tree to patch — building one of these is the work.

It unblocks two waiting items: the spawn TCB camera state (0002) and the INDIA3
near-heap headroom question (0006).

## Comments

Delivered via the instrumented dosbox route. Built dosbox-0.74-3 from source with
tools/oracle/dosbox_vga_terrain_trace.patch (-p0; touches include/mem.h +
src/hardware/vga_memory.cpp) against system SDL1.2 -> third_party/dosbox-fist.
tools/oracle/capture_tcb_camera.sh drives it (xvfb + XTEST nav like the burst
tool) with FIST_R9200CAP=1; when the mission renders (guest reaches ext 0x9200)
the patch writes $FISTLOG.cam.txt (the render-time TCB camera) plus .passNN.ram.bin
(full 16 MB guest RAM) and VRAM/palette/src3911 dumps. First run (AZER1) captured
the original's camera cleanly, immediately resolving 0002's camera question.
