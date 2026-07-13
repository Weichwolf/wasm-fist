#!/usr/bin/env bash
# =============================================================================
# trace_flat3918.sh -- DECISIVE, PAGING-ROBUST oracle: identify the code that
# fills tile-3918 (extender FLAT linear 0x44200) at AZER1 map-load on the
# ORIGINAL game, watching the FLAT ADDRESS regardless of which physical frame
# backs it.
#
# WHY this exists + WHAT IT PROVED (2026-07-13, docs/oracle_tile3918_producer.md):
# The concern was that the extender's PM paging moves its flat buffers, confounding
# a FIXED-PHYSICAL watch.  This CR3-aware watch (env FIST_WATCHFLAT/FIST_WATCHFLATSPAN
# in the instrumented DOSBox) walks the guest CR3 page tables on every CR3/paging
# change to keep a target LINEAR address' phys mapping current.  MEASURED RESULT:
# the extender's tile 3918 is NOT paged -- it lives at a FIXED physical offset via a
# DATA-SEGMENT BASE of 0x131000 (ext pointer 0x44200 -> CPU linear/phys 0x175200),
# STABLE across map-load and dump.  So the fixed-phys watch at 0x175200 (the
# .tilewriters.txt output, now with 20-word backtraces) is definitive here, and the
# CR3 walk merely CONFIRMS stability (map: flatpg 0x175xxx -> physpg 0x175xxx identity).
# Default FIST_WATCHFLAT is therefore the CPU LINEAR addr 0x175200 (= flat 0x44200 +
# 0x131000), NOT the raw ext pointer 0x44200 (that resolves to a DIFFERENT buffer).
#
# Arms SIGUSR1 BEFORE the ACCEPT click (map-load), captures the writers into
# <prefix>.flatwriters.txt (CS:EIP histogram + flat-offset range + 20-word near-
# call backtrace each + the final flat->phys page map), then dumps RAM/pal.
#
#   usage: trace_flat3918.sh [GX1 GY1 GX2 GY2 GX3 GY3 WAIT GAP1 GAP2 SETTLE ARMW DUMPW OUTPREFIX]
#   env:   FIST_WATCHFLAT (default 0x44200)  FIST_WATCHFLATSPAN (default 0x10000)
# =============================================================================
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
GX1="${1:-160}"; GY1="${2:-100}"; GX2="${3:-205}"; GY2="${4:-128}"; GX3="${5:-40}"; GY3="${6:-186}"
WAIT="${7:-60}"; GAP1="${8:-12}"; GAP2="${9:-12}"; SETTLE="${10:-30}"; ARMW="${11:-5}"; DUMPW="${12:-8}"
PREFIX="${13:-$ROOT/scratch/oracle/flat3918}"
DOSBOX="${DOSBOX:-/tmp/debs/dosbox-fist}"
CYCLES="${CYCLES:-max}"
WATCHFLAT="${FIST_WATCHFLAT:-0x175200}"
WATCHFLATSPAN="${FIST_WATCHFLATSPAN:-0x10000}"

[ -x "$DOSBOX" ] || { echo "missing instrumented dosbox at $DOSBOX (see tools/oracle/README_terrain.md)"; exit 1; }
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -I/tmp/debs/sysroot/usr/include \
    -L/tmp/debs/sysroot/usr/lib/x86_64-linux-gnu -o /tmp/xclick -lX11 -lXtst
mkdir -p "$(dirname "$PREFIX")"
rm -f "$PREFIX.flatwriters.txt"
SCRATCH="$(mktemp -d /tmp/fist_flat3918.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SCRATCH"' EXIT
CONF="$SCRATCH/dosbox_flat3918.conf"
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
  export FISTLOG='$PREFIX' FIST_WATCHFLAT='$WATCHFLAT' FIST_WATCHFLATSPAN='$WATCHFLATSPAN'
  '$DOSBOX' -conf '$CONF' -exit >'$LOG' 2>&1 &
  DPID=\$!
  sleep $WAIT
  ROOTPNG='$SCRATCH/root.png'
  import -window root \$ROOTPNG 2>/dev/null
  GEOM=\$(convert \$ROOTPNG -fuzz 1% -format '%@' info:)
  echo \"[flat3918] menu game rect: \$GEOM\"
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo '[flat3918] click BATTLES'; /tmp/xclick \$((OX+$GX1)) \$((OY+$GY1)); sleep $GAP1
  echo '[flat3918] click OK';      /tmp/xclick \$((OX+$GX2)) \$((OY+$GY2)); sleep $GAP2
  echo '[flat3918] ARM-EARLY (SIGUSR1) before ACCEPT map-load'; kill -USR1 \$DPID; sleep $ARMW
  echo '[flat3918] click ACCEPT';  /tmp/xclick \$((OX+$GX3)) \$((OY+$GY3)); sleep $SETTLE
  import -window root '$PREFIX.frame.png' 2>/dev/null
  echo '[flat3918] DUMP (SIGUSR2)'; kill -USR2 \$DPID; sleep $DUMPW
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
cp "$LOG" /tmp/flat3918_dosbox.log 2>/dev/null
echo '=== dosbox stderr tail ==='; tail -5 "$LOG" 2>/dev/null
echo '=== oracle artifacts ==='; ls -la "$PREFIX".* 2>/dev/null
echo "=== flat-3918 writers (flat=$WATCHFLAT span=$WATCHFLATSPAN) ==="
head -60 "$PREFIX.flatwriters.txt" 2>/dev/null || echo "NO flatwriters.txt (dump did not fire)"
