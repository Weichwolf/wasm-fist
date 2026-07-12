# Terrain-writer oracle — instrumented DOSBox

Purpose: identify, from the ORIGINAL Armored Fist in a live mission, exactly which code writes the
cockpit **terrain** pixels to VGA `0xA0000` and from what data. Findings:
**`docs/oracle_terrain_writer.md`** (headline: the terrain is the extender's `FUN_0000_9200`).

Why a custom DOSBox: the QEMU+FreeDOS oracle (`run_oracle.sh`) crashes in the extender **before the
menu** (documented in `README.md`), so it can't reach a mission. DOSBox reaches the mission but stock
0.74 has no debugger/memory monitor. So we build a **local, instrumented** DOSBox that logs the CS:EIP
of every `0xA0000` writer and dumps guest RAM / VRAM / palette on signal. `core=normal` makes `reg_eip`
exact.

## Build (no sudo required)
```bash
cd /tmp/debs
# 1. SDL1.2 + X11 dev headers into a local sysroot (apt-get download needs no root)
apt-get download libsdl1.2-dev libxtst-dev libx11-dev libxext-dev x11proto-dev libxau-dev libxdmcp-dev
mkdir -p sysroot; for d in *.deb; do dpkg-deb -x "$d" sysroot; done
# fix the dangling -dev symlinks to point at the installed runtime .so
cd sysroot/usr/lib/x86_64-linux-gnu
ln -sf /usr/lib/x86_64-linux-gnu/libSDL-1.2.so.1.2.68 libSDL-1.2.so.0
ln -sf libSDL-1.2.so.0 libSDL-1.2.so; ln -sf libSDL-1.2.so.0 libSDL.so
for l in libX11 libXtst libXext libXau libXdmcp; do \
  ln -sf "$(ls /usr/lib/x86_64-linux-gnu/$l.so.* | grep -E '\.so\.[0-9]+$' | head -1)" $l.so; done
cd /tmp/debs; sed -i 's|^includedir=.*|includedir=/tmp/debs/sysroot/usr/include|;s|^libdir=.*|libdir=/usr/lib/x86_64-linux-gnu|' sysroot/usr/bin/sdl-config
# 2. DOSBox 0.74-3 source
curl -sL -o dosbox.tar.gz "https://sourceforge.net/projects/dosbox/files/dosbox/0.74-3/dosbox-0.74-3.tar.gz/download"
tar xzf dosbox.tar.gz && cd dosbox-0.74-3
# 3. apply the VGA-write instrumentation
patch -p0 < /path/to/repo/tools/oracle/dosbox_vga_terrain_trace.patch   # patches src/hardware/vga_memory.cpp
# 4. configure + build
export SDL_CONFIG=/tmp/debs/sysroot/usr/bin/sdl-config
export LDFLAGS="-L/tmp/debs/sysroot/usr/lib/x86_64-linux-gnu" CXXFLAGS="-O2 -w" CFLAGS="-O2 -w"
./configure --with-sdl-prefix=/tmp/debs/sysroot/usr --disable-opengl
make -j"$(nproc)"
cp src/dosbox /tmp/debs/dosbox-fist
```

## Run the terrain trace
```bash
cc -O2 tools/oracle/xclick.c -I/tmp/debs/sysroot/usr/include \
   -L/tmp/debs/sysroot/usr/lib/x86_64-linux-gnu -o /tmp/xclick -lX11 -lXtst
DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/trace_terrain.sh
```
Drives BATTLES→OK→ACCEPT (headless xvfb, XTest), settles on the terrain frame, then:
- `SIGUSR1` arms a per-instruction-pointer histogram of `0xA0000` writes,
- `SIGUSR2` dumps `<prefix>.writers.txt` (top writer IPs + 64 code bytes each), `<prefix>.ram.bin`
  (16 MB guest physical), `<prefix>.vram.bin`, `<prefix>.pal.bin`, and `<prefix>.frame.png`.

Instrumentation env: `FISTLOG=<prefix>` (default `/tmp/fistlog`). The hook lives in the chained- and
unchained-VGA `writeb/writew/writed` handlers (mode 13h uses the chained handler).

Sample evidence from a real AZER1 run is committed at `samples/terrain_writers.txt`.
The instrumented binary + 16 MB dumps are NOT committed (rebuild via above).
