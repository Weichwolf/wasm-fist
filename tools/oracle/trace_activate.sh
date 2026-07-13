#!/usr/bin/env bash
# =============================================================================
# trace_activate.sh -- capture the mission-entry player-view ACTIVATE on the
# ORIGINAL: WATCH the engine view-state block (DGROUP 4d0e/4d52/4e1c..4e34) and
# log every writer's flat-IP + near-call stack backtrace, from BATTLES->OK->
# ACCEPT through the settled terrain frame.  Answers: does the original set
# 4d0e=2 (cockpit) at mission entry, and from what event source?
#
# Engine load base in guest phys is deterministic under LOADGAME/DOSBox = 0x11190
# (found by locating fist_dat_image.bin content in a prior RAM dump).  So:
#   DGROUP (DAT_2000_ seg 0x2000) phys = 0x11190 + 0x20000 + off
#   4d0e -> 0x35e9e ; 4e1c -> 0x35fac.  Watch [0x35e50 .. 0x36050) (span 0x100).
# Override FIST_WATCHPHYS/FIST_WATCHSPAN to retarget.
#
#   usage: trace_activate.sh [GX1 GY1 GX2 GY2 GX3 GY3 WAIT GAP1 GAP2 SETTLE OUTPREFIX]
# =============================================================================
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
GX1="${1:-160}"; GY1="${2:-100}"; GX2="${3:-205}"; GY2="${4:-128}"; GX3="${5:-40}"; GY3="${6:-186}"
WAIT="${7:-60}"; GAP1="${8:-12}"; GAP2="${9:-12}"; SETTLE="${10:-25}"
PREFIX="${11:-$ROOT/scratch/oracle/activate}"
DOSBOX="${DOSBOX:-/tmp/debs/dosbox-fist}"
CYCLES="${CYCLES:-max}"
WATCHPHYS="${FIST_WATCHPHYS:-0x35f50}"
WATCHSPAN="${FIST_WATCHSPAN:-0x100}"

[ -x "$DOSBOX" ] || { echo "missing instrumented dosbox at $DOSBOX"; exit 1; }
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -I/tmp/debs/sysroot/usr/include \
    -L/tmp/debs/sysroot/usr/lib/x86_64-linux-gnu -o /tmp/xclick -lX11 -lXtst
mkdir -p "$(dirname "$PREFIX")"
rm -f "$PREFIX.watch.txt"
SCRATCH="$(mktemp -d /tmp/fist_activate.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SCRATCH"' EXIT
CONF="$SCRATCH/dosbox_activate.conf"
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
timeout $((WAIT+GAP1+GAP2+SETTLE+60)) \
  xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  export FISTLOG='$PREFIX' FIST_WATCHPHYS='$WATCHPHYS' FIST_WATCHSPAN='$WATCHSPAN'
  '$DOSBOX' -conf '$CONF' -exit >'$LOG' 2>&1 &
  DPID=\$!
  sleep $WAIT
  ROOTPNG='$SCRATCH/root.png'
  import -window root \$ROOTPNG 2>/dev/null
  GEOM=\$(convert \$ROOTPNG -fuzz 1% -format '%@' info:)
  echo \"[activate] menu game rect: \$GEOM\"
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo '[activate] click BATTLES'; /tmp/xclick \$((OX+$GX1)) \$((OY+$GY1)); sleep $GAP1
  echo '[activate] click OK';      /tmp/xclick \$((OX+$GX2)) \$((OY+$GY2)); sleep $GAP2
  echo '[activate] ARM (SIGUSR1) before ACCEPT'; kill -USR1 \$DPID
  echo '[activate] click ACCEPT';  /tmp/xclick \$((OX+$GX3)) \$((OY+$GY3)); sleep $SETTLE
  import -window root '$PREFIX.frame.png' 2>/dev/null
  echo '[activate] disarm (SIGUSR2)'; kill -USR2 \$DPID; sleep 3
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
cp "$LOG" /tmp/activate_dosbox.log 2>/dev/null; echo "=== dosbox log tail ==="; tail -15 "$LOG" 2>/dev/null
echo "=== watch hits: $(wc -l < "$PREFIX.watch.txt" 2>/dev/null || echo 0) ==="
sort "$PREFIX.watch.txt" 2>/dev/null | uniq -c | sort -rn | head -40
