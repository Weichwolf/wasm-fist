#!/usr/bin/env bash
# GENUINE NATIVE 320x200 DOSBox reference of a SUB-SCREEN reached by CLICKING a menu item.
# Non-circular: DOSBox renders the real game 1:1 (output=surface, scaler=none), we locate the game
# rect on the xvfb root with -trim, inject a real left-click (XTest) at the item's game pixel, wait for
# the sub-screen, then crop the SAME rect. 6->8bit = VGA bit-replication, matching fist_vga.c.
#   usage: refcapture_click.sh GX GY WAIT SETTLE OUT.png    (GX,GY = game pixel of the item)
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GX="${1:-160}"; GY="${2:-139}"; WAIT="${3:-40}"; SETTLE="${4:-8}"; OUT="${5:-$ROOT/ref/about_native320.png}"
CONF="$ROOT/scratch/refcap/dosbox_click.conf"; mkdir -p "$(dirname "$CONF")"
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
mount c $ROOT/armoredfist
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
ROOTPNG="$(mktemp /tmp/fist_clickroot.XXXX.png)"
timeout $((WAIT+SETTLE+40)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  dosbox -conf '$CONF' -exit >/tmp/fist_refcap_click.log 2>&1 &
  DPID=\$!
  sleep $WAIT
  import -window root '$ROOTPNG' 2>/dev/null
  GEOM=\$(convert '$ROOTPNG' -fuzz 1% -format '%@' info:)
  echo \"[click] pre-click game rect: \$GEOM\"
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  CX=\$((OX+$GX)); CY=\$((OY+$GY))
  echo \"[click] injecting click at root (\$CX,\$CY) = rect+($GX,$GY)\"
  /tmp/xclick \$CX \$CY
  sleep $SETTLE
  import -window root '$ROOTPNG' 2>/dev/null
  echo \"\$GEOM\" > /tmp/fist_click_geom.txt
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
GEOM="$(cat /tmp/fist_click_geom.txt)"
echo "[click] cropping post-click at $GEOM"
convert "$ROOTPNG" -crop "$GEOM" +repage "$OUT"
rm -f "$ROOTPNG"
[ -f "$OUT" ] && echo "[click] captured sub-screen -> $OUT" || { echo FAILED; exit 1; }
