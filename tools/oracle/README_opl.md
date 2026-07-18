# OPL-writer oracle — instrumented DOSBox

Purpose: identify, from the ORIGINAL Armored Fist at the MENU (background music playing), exactly which
code posts the OPL/AdLib register writes (the melody), from what note source, via which call path.
Findings: **`docs/audio.md` §20** (headline: the menu music is the MIDI song embedded in
`MAINMENU.MS3`, played by the SOUNDDVR.DVR sequencer — NOT extender-resident; static RE misattributed
it twice).

Why a custom DOSBox: stock 0.74 has no debugger. We patch `src/hardware/adlib.cpp` so `Module::PortWrite`
(the 0x388 register-select / 0x389 data handler) logs, for every guest OPL write, the guest `CS:IP`
(`SegPhys(cs)+reg_eip`), `PIC_Ticks`, port, value, the resolved OPL register, and a 12-word guest-stack
backtrace (the OPL primitive is a leaf out-pair helper, so `[SS:SP+k]` holds the melody poster's return
chain). `core=normal` makes `reg_eip` exact. On SIGUSR2 it dumps a per-CS:IP histogram + the low 4 MB of
guest RAM (`.ram.bin`) for offline code decode against the driver images.

## Build (reuses the terrain-oracle sysroot; see README_terrain.md steps 1-2)
```bash
cd /tmp/debs/dosbox-0.74-3
patch -p0 < /path/to/repo/tools/oracle/dosbox_opl_trace.patch   # patches src/hardware/adlib.cpp
export SDL_CONFIG=/tmp/debs/sysroot/usr/bin/sdl-config
export LDFLAGS="-L/tmp/debs/sysroot/usr/lib/x86_64-linux-gnu" CXXFLAGS="-O2 -w" CFLAGS="-O2 -w"
make -j"$(nproc)"
cp src/dosbox /tmp/debs/dosbox-fist
```
The terrain patch (`dosbox_vga_terrain_trace.patch`, vga_memory.cpp + mem.h) and this OPL patch are
independent and coexist in the same tree.

## Run the OPL trace
```bash
DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/trace_opl.sh /tmp/fistopl 32 10
```
Boots the original headless (xvfb) to the menu, lets the music play ~10 s, then SIGUSR2 → dumps. Outputs:
- `$PREFIX` — the time-ordered write log (`seq t=<ms> cs=SEG:OFF flat=... port=... val=... DATA|SEL reg=... ss=SEG:SP stk=w0,w1,...`)
- `$PREFIX.writers.txt` — per-CS:IP histogram
- `$PREFIX.ram.bin` — low 4 MB guest physical RAM (identify load segs / decode callers offline)

Instrumentation env: `FISTOPLLOG=<prefix>`. Default OFF → behaviour-neutral. The hook is in
`Module::PortWrite` (adlib.cpp), so it fires for both OPL2 and OPL3 modes.
