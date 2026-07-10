#!/usr/bin/env bash
# DOSBox open-trace of a menu sub-screen reached by THREE CLICKS (e.g. BATTLES->OK->ACCEPT).
# Boots under LD_PRELOAD=opentrace.so, waits for the menu, XTest-clicks three game pixels in
# sequence, and prints the [OPEN:*] log lines (game files opened) with click markers.
# WRITE-ISOLATED: mounts a fresh cp -a scratch copy so any .FPL write never touches the repo.
#   usage: trace_click3.sh GX1 GY1 GX2 GY2 GX3 GY3 WAIT GAP SETTLE
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
GX1="${1:-160}"; GY1="${2:-100}"; GX2="${3:-205}"; GY2="${4:-128}"; GX3="${5:-40}"; GY3="${6:-186}"
WAIT="${7:-42}"; GAP="${8:-8}"; SETTLE="${9:-10}"
SCRATCH="$(mktemp -d /tmp/fist_trace3.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SCRATCH"' EXIT
CONF="$SCRATCH/dosbox_click3.conf"
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
core=auto
cputype=auto
cycles=max
[autoexec]
mount c $DATA
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
LOG=/tmp/fist_trace_click3.log
: > "$LOG"
timeout $((WAIT+GAP+GAP+SETTLE+40)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  LD_PRELOAD=/tmp/opentrace.so dosbox -conf '$CONF' -exit >'$LOG' 2>&1 &
  DPID=\$!
  sleep $WAIT
  echo '===MENU-REACHED===' >>'$LOG'
  ROOTPNG=\$(mktemp /tmp/fist_t3root.XXXX.png)
  import -window root \$ROOTPNG 2>/dev/null
  GEOM=\$(convert \$ROOTPNG -fuzz 1% -format '%@' info:)
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[click] rect \$GEOM\" >>'$LOG'
  echo '===CLICK1 (item)===' >>'$LOG'
  /tmp/xclick \$((OX+$GX1)) \$((OY+$GY1)); sleep $GAP
  echo '===CLICK2 (OK)===' >>'$LOG'
  /tmp/xclick \$((OX+$GX2)) \$((OY+$GY2)); sleep $GAP
  echo '===CLICK3 (ACCEPT)===' >>'$LOG'
  /tmp/xclick \$((OX+$GX3)) \$((OY+$GY3)); sleep $SETTLE
  import -window root \$ROOTPNG 2>/dev/null
  convert \$ROOTPNG -crop \$GEOM +repage /tmp/fist_trace3_after.png 2>/dev/null
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
echo "=== full [OPEN] sequence with click markers ==="
grep -n 'OPEN:\|===' "$LOG" | sed -E 's/\[OPEN:[a-z0-9]+\]/[OPEN]/'
echo "=== after-frame -> /tmp/fist_trace3_after.png ==="
