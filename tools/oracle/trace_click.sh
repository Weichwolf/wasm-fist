#!/usr/bin/env bash
# DOSBox open-trace of a menu sub-screen reached by CLICKING an item.
# Boots the real game under LD_PRELOAD=opentrace.so, waits for the menu, XTest-clicks
# the item's game pixel, waits, and prints the [OPEN:*] log lines (game files opened).
#   usage: trace_click.sh GX GY WAIT SETTLE
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
GX="${1:-160}"; GY="${2:-125}"; WAIT="${3:-42}"; SETTLE="${4:-10}"
CONF="$ROOT/scratch/refcap/dosbox_click.conf"
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
LOG=/tmp/fist_trace_click.log
: > "$LOG"
timeout $((WAIT+SETTLE+40)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  LD_PRELOAD=/tmp/opentrace.so dosbox -conf '$CONF' -exit >'$LOG' 2>&1 &
  DPID=\$!
  sleep $WAIT
  echo '===MENU-REACHED===' >>'$LOG'
  ROOTPNG=\$(mktemp /tmp/fist_troot.XXXX.png)
  import -window root \$ROOTPNG 2>/dev/null
  GEOM=\$(convert \$ROOTPNG -fuzz 1% -format '%@' info:)
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[click] game rect \$GEOM -> click (\$((OX+$GX)),\$((OY+$GY)))\" >>'$LOG'
  /tmp/xclick \$((OX+$GX)) \$((OY+$GY))
  echo '===CLICKED===' >>'$LOG'
  sleep $SETTLE
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
echo "=== full [OPEN] sequence ==="
grep -n 'OPEN:\|===' "$LOG" | sed -E 's/\[OPEN:[a-z0-9]+\]/[OPEN]/'
