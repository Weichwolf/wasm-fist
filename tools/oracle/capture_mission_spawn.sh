#!/usr/bin/env bash
# =============================================================================
# capture_mission_spawn.sh -- GENUINE frame-matched AZER1 mission SPAWN capture.
#
# Captures the ORIGINAL Armored Fist's windshield framebuffer at the EARLIEST
# mission frame (spawn / frame-1) -- the sim state the port's deterministic
# op-0x24 frame is at -- NOT the fully-settled frame. Drives BATTLES->OK->ACCEPT
# under the instrumented DOSBox (tools/oracle/dosbox_vga_terrain_trace.patch),
# then polls the SDL surface every 0.3s; the LOADING photo grabs are large PNGs
# (~30-40 KB), the first windshield frame drops to ~14-22 KB -> that transition
# marks frame-1. At that instant it saves the 1:1 320x200 game rect and fires
# SIGUSR2 to dump guest RAM (for the live camera XY/alt read).
#
# 6->8-bit DAC expansion is DOSBox's VGA bit-replication (v<<2)|(v>>4), the same
# expander fist_vga.c / FIST_MISSFB use, so the grab is directly byte-comparable
# to the port's frame dump. output=surface + scaler=none + aspect=false => the
# SDL surface IS the mode-13h 320x200 framebuffer (asserted via -trim rect).
#
# Non-circular: the reference comes from DOSBox running the ORIGINAL FIST.RUN
# (never a copy of the port output); its md5 differs from the port frame.
#
# Findings: docs/oracle_mission_spawn.md.  Env: DOSBOX (default /tmp/debs/dosbox-fist).
# Usage: capture_mission_spawn.sh [OUT_PNG]   (default ref/mission_azer1_spawn_native320.png)
# =============================================================================
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${1:-$ROOT/ref/mission_azer1_spawn_native320.png}"
DOSBOX="${DOSBOX:-/tmp/debs/dosbox-fist}"
PREFIX="${FISTLOG:-$ROOT/scratch/oracle/mspawn}"
[ -x "$DOSBOX" ] || { echo "missing instrumented dosbox at $DOSBOX (see tools/oracle/README_terrain.md)"; exit 1; }
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -I/tmp/debs/sysroot/usr/include \
    -L/tmp/debs/sysroot/usr/lib/x86_64-linux-gnu -o /tmp/xclick -lX11 -lXtst
mkdir -p "$(dirname "$PREFIX")" "$(dirname "$OUT")"
SCRATCH="$(mktemp -d /tmp/fist_mspawn.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SCRATCH"' EXIT
CONF="$SCRATCH/db.conf"
cat > "$CONF" <<CFG
[sdl]
fullscreen=false
output=surface
autolock=false
[render]
frameskip=0
aspect=false
scaler=none
[dosbox]
memsize=16
[cpu]
core=normal
cputype=auto
cycles=max
[autoexec]
mount c $DATA
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
LOG="$SCRATCH/db.log"
rm -f "$PREFIX".ram.bin "$PREFIX".pal.bin
timeout 160 xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  export FISTLOG='$PREFIX' FIST_TILEPHYS=0x141000
  '$DOSBOX' -conf '$CONF' -exit >'$LOG' 2>&1 &
  DPID=\$!
  sleep 60
  import -window root '$SCRATCH/root.png' 2>/dev/null
  GEOM=\$(convert '$SCRATCH/root.png' -fuzz 1% -format '%@' info:)
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[mspawn] menu rect \$GEOM\"
  echo '[mspawn] BATTLES'; /tmp/xclick \$((OX+160)) \$((OY+100)); sleep 12
  echo '[mspawn] OK';      /tmp/xclick \$((OX+205)) \$((OY+128)); sleep 12
  echo '[mspawn] ACCEPT';  /tmp/xclick \$((OX+40))  \$((OY+186))
  DONE=0
  for i in \$(seq 1 40); do
     import -window root '$SCRATCH/g.png' 2>/dev/null
     G=\$(convert '$SCRATCH/g.png' -fuzz 1% -format '%@' info:)
     convert '$SCRATCH/g.png' -crop \$G +repage '$SCRATCH/frame.png' 2>/dev/null || true
     SZ=\$(stat -c%s '$SCRATCH/frame.png' 2>/dev/null || echo 0)
     echo \"[mspawn] poll \$i sz=\$SZ rect=\$G\"
     # first windshield frame (~14-22 KB) after the LOADING photo (~30-40 KB)
     if [ \$DONE -eq 0 ] && [ \$SZ -gt 10000 ] && [ \$SZ -lt 26000 ]; then
        case \"\$G\" in 320x200+*+*) : ;; *) echo '[mspawn] rect not 320x200 -> scaling'; exit 1 ;; esac
        cp '$SCRATCH/frame.png' '$OUT'
        echo '[mspawn] FIRST windshield frame captured -> '$OUT'; SIGUSR2 RAM dump'
        kill -USR2 \$DPID; DONE=1; sleep 1
     fi
     sleep 0.3
  done
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
echo '=== reference ==='; identify "$OUT" 2>/dev/null
echo '=== ram dump (for the camera read) ==='; ls -la "$PREFIX".ram.bin 2>/dev/null
# live camera at spawn: X~583982 Y~1142557, alt @ +8 (=terrain height h*~292):
[ -f "$PREFIX.ram.bin" ] && python3 - "$PREFIX.ram.bin" <<'PY'
import struct,sys
d=open(sys.argv[1],'rb').read(); seen=set()
for i in range(0,len(d)-12,2):
    x=struct.unpack_from('<i',d,i)[0]
    if 570000<x<620000:
        y=struct.unpack_from('<i',d,i+4)[0]
        if 1100000<y<1180000:
            a=struct.unpack_from('<i',d,i+8)[0]
            if 300<a<45000 and (x,y,a) not in seen:
                seen.add((x,y,a)); print("live-cam off=0x%06x X=%d Y=%d alt=%d (h~%.1f)"%(i,x,y,a,a/292.0))
PY
