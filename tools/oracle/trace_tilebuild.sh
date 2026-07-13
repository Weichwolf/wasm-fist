#!/usr/bin/env bash
# =============================================================================
# trace_terrain.sh -- DECISIVE oracle: identify the code that writes the cockpit
# TERRAIN pixels to VGA 0xA0000 in a live AZER1 mission of the ORIGINAL game.
#
# Boots the ORIGINAL Armored Fist under an INSTRUMENTED DOSBox (core=normal so
# reg_eip is exact), drives BATTLES -> OK -> ACCEPT via XTest, lets the terrain
# frame settle, then:
#   * SIGUSR1 -> arm: reset the per-instruction-pointer histogram of 0xA0000 writes
#   * (window) capture several terrain frames' worth of pixel writes
#   * SIGUSR2 -> dump: writers.txt (top writer IPs + code bytes), ram.bin (guest
#                physical RAM = heightmap/colormap/camera), vram.bin, pal.bin
# The writer's CS:EIP + code bytes are matched offline against our decompile
# images to resolve engine (FIST.DAT) vs MGAVIDEO driver vs Doug-Huffman extender.
#
# Write-isolated: DOSBox mounts a throwaway cp -a of armoredfist/ (repo untouched).
#
#   usage: trace_terrain.sh [GX1 GY1 GX2 GY2 GX3 GY3 WAIT GAP1 GAP2 SETTLE ARM DUMP OUTPREFIX]
# =============================================================================
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
GX1="${1:-160}"; GY1="${2:-100}"; GX2="${3:-205}"; GY2="${4:-128}"; GX3="${5:-40}"; GY3="${6:-186}"
WAIT="${7:-60}"; GAP1="${8:-12}"; GAP2="${9:-12}"; SETTLE="${10:-30}"; ARMW="${11:-5}"; DUMPW="${12:-8}"
PREFIX="${13:-$ROOT/scratch/oracle/terrain}"
DOSBOX="${DOSBOX:-/tmp/debs/dosbox-fist}"
CYCLES="${CYCLES:-max}"

[ -x "$DOSBOX" ] || { echo "missing instrumented dosbox at $DOSBOX (see tools/oracle/README_terrain.md)"; exit 1; }
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -I/tmp/debs/sysroot/usr/include \
    -L/tmp/debs/sysroot/usr/lib/x86_64-linux-gnu -o /tmp/xclick -lX11 -lXtst
mkdir -p "$(dirname "$PREFIX")"
SCRATCH="$(mktemp -d /tmp/fist_terrain.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SCRATCH"' EXIT
CONF="$SCRATCH/dosbox_terrain.conf"
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
cycles=$CYCLES
[autoexec]
mount c $DATA
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
LOG="$SCRATCH/dosbox.log"
timeout $((WAIT+GAP1+GAP2+SETTLE+ARMW+DUMPW+60)) \
  xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  export FISTLOG='$PREFIX'
  '$DOSBOX' -conf '$CONF' -exit >'$LOG' 2>&1 &
  DPID=\$!
  sleep $WAIT
  ROOTPNG='$SCRATCH/root.png'
  import -window root \$ROOTPNG 2>/dev/null
  GEOM=\$(convert \$ROOTPNG -fuzz 1% -format '%@' info:)
  echo \"[terrain] menu game rect: \$GEOM\"
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo '[terrain] click BATTLES'; /tmp/xclick \$((OX+$GX1)) \$((OY+$GY1)); sleep $GAP1
  echo '[terrain] click OK';      /tmp/xclick \$((OX+$GX2)) \$((OY+$GY2)); sleep $GAP2
  echo '[tilebuild] ARM-EARLY (SIGUSR1) before ACCEPT map-load'; kill -USR1 \$DPID
  echo '[terrain] click ACCEPT';  /tmp/xclick \$((OX+$GX3)) \$((OY+$GY3)); sleep $SETTLE
  import -window root '$PREFIX.frame.png' 2>/dev/null
  echo '[terrain] DUMP (SIGUSR2)'; kill -USR2 \$DPID; sleep $DUMPW
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
echo '=== dosbox stderr tail ==='; tail -5 "$LOG" 2>/dev/null
echo '=== oracle artifacts ==='; ls -la "$PREFIX".* 2>/dev/null
echo '=== top writers ==='; head -40 "$PREFIX.writers.txt" 2>/dev/null || echo "NO writers.txt (dump did not fire)"
