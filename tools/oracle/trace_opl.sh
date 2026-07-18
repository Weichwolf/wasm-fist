#!/usr/bin/env bash
# OPL-writer oracle: capture the guest CS:IP of every AdLib/OPL port write
# (0x388 register-select / 0x389 data) from the ORIGINAL Armored Fist, booted
# to the MENU where the background music plays.  Decisively identifies the
# code that posts the menu melody (A0/B0 fnum + key-on stream), which static
# RE has misattributed twice (op=6 = shutdown; be0e->OPL = garbage).
#
# Needs the instrumented dosbox-fist (adlib.cpp hook, README_opl.md build).
#   usage: tools/oracle/trace_opl.sh [OUTPREFIX] [MENU_WAIT] [PLAY_SECONDS]
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PREFIX="${1:-/tmp/fistopl}"
MENU_WAIT="${2:-32}"; PLAY="${3:-10}"
DOSBOX="${DOSBOX:-/tmp/debs/dosbox-fist}"
CAP="$ROOT/tools/oracle/_oplcap"; rm -rf "$CAP"; mkdir -p "$CAP"
CONF="$CAP/dosbox.conf"
rm -f "$PREFIX" "$PREFIX.writers.txt"
cat > "$CONF" <<CFG
[sdl]
output=surface
[dosbox]
memsize=16
captures=$CAP
[cpu]
core=normal
cputype=auto
cycles=max
[sblaster]
sbtype=sb16
sbbase=220
irq=7
dma=1
oplmode=auto
oplrate=44100
[autoexec]
mount c $ROOT/armoredfist
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11 FISTOPLLOG="$PREFIX"
timeout $((MENU_WAIT+PLAY+30)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  FISTOPLLOG='$PREFIX' '$DOSBOX' -conf '$CONF' -exit >/tmp/fist_oplcap.log 2>&1 &
  DPID=\$!
  sleep $MENU_WAIT                      # boot through intro to the menu (music playing)
  sleep $PLAY                           # capture the continuous melody stream
  kill -USR2 \$DPID 2>/dev/null         # dump the CS:IP histogram
  sleep 1
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
echo "[opl] log: $PREFIX ($(wc -l < "$PREFIX" 2>/dev/null || echo 0) writes)"
echo "[opl] histogram: $PREFIX.writers.txt"
