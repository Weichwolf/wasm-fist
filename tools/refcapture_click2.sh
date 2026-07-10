#!/usr/bin/env bash
# GENUINE NATIVE 320x200 DOSBox reference of a DIALOG INTERACTION reached by TWO clicks: first a menu
# item (opens a sub-screen/dialog), then an element INSIDE it (row / button).  Non-circular, same method
# as refcapture_click.sh (output=surface, scaler=none, -trim to the game rect, XTest clicks, crop same
# rect, 6->8bit VGA bit-replication).
#   usage: refcapture_click2.sh GX1 GY1 GX2 GY2 WAIT GAP SETTLE OUT.png
#          (GX1,GY1 = menu-item game pixel; GX2,GY2 = dialog-element game pixel)
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GX1="${1:-160}"; GY1="${2:-74}"; GX2="${3:-130}"; GY2="${4:-119}"
WAIT="${5:-40}"; GAP="${6:-8}"; SETTLE="${7:-8}"; OUT="${8:-$ROOT/ref/selplayer_select_native320.png}"
CONF="$ROOT/scratch/refcap/dosbox_click2.conf"; mkdir -p "$(dirname "$CONF")"
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
ROOTPNG="$(mktemp /tmp/fist_click2root.XXXX.png)"
timeout $((WAIT+GAP+SETTLE+40)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  dosbox -conf '$CONF' -exit >/tmp/fist_refcap_click2.log 2>&1 &
  DPID=\$!
  sleep $WAIT
  import -window root '$ROOTPNG' 2>/dev/null
  GEOM=\$(convert '$ROOTPNG' -fuzz 1% -format '%@' info:)
  echo \"[click2] game rect: \$GEOM\"
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[click2] click1 menu item rect+($GX1,$GY1)\"
  /tmp/xclick \$((OX+$GX1)) \$((OY+$GY1))
  sleep $GAP
  echo \"[click2] click2 dialog element rect+($GX2,$GY2)\"
  /tmp/xclick \$((OX+$GX2)) \$((OY+$GY2))
  sleep $SETTLE
  import -window root '$ROOTPNG' 2>/dev/null
  echo \"\$GEOM\" > /tmp/fist_click2_geom.txt
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
GEOM="$(cat /tmp/fist_click2_geom.txt)"
echo "[click2] cropping post-interaction at $GEOM"
convert "$ROOTPNG" -crop "$GEOM" +repage "$OUT"
rm -f "$ROOTPNG"
[ -f "$OUT" ] && echo "[click2] captured -> $OUT" || { echo FAILED; exit 1; }
