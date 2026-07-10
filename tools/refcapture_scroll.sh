#!/usr/bin/env bash
# GENUINE NATIVE 320x200 DOSBox reference of a list dialog scrolled to its BOTTOM via a HELD scroll
# arrow.  Non-circular, same 1:1 method as refcapture_click.sh: DOSBox renders the real game (surface,
# scaler=none), we locate the game rect on the xvfb root with -trim, inject a real left-click (XTest) at
# the menu item that opens the list dialog, then a real PRESS-and-HOLD on the down-scroll arrow long
# enough that the game's own auto-repeat clamps the list at the bottom, then crop the SAME rect.
#   usage: refcapture_scroll.sh GX1 GY1 AX AY WAIT GAP HOLDMS SETTLE OUT.png
#          (GX1,GY1 = menu-item game pixel; AX,AY = down-arrow game pixel)
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GX1="${1:-160}"; GY1="${2:-100}"; AX="${3:-172}"; AY="${4:-135}"
WAIT="${5:-40}"; GAP="${6:-3}"; HOLDMS="${7:-6000}"; SETTLE="${8:-3}"; OUT="${9:-$ROOT/ref/battles_scroll_native320.png}"
CONF="$ROOT/scratch/refcap/dosbox_scroll.conf"; mkdir -p "$(dirname "$CONF")"
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
ROOTPNG="$(mktemp /tmp/fist_scrollroot.XXXX.png)"
timeout $((WAIT+GAP+HOLDMS/1000+SETTLE+40)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  dosbox -conf '$CONF' -exit >/tmp/fist_refcap_scroll.log 2>&1 &
  DPID=\$!
  sleep $WAIT
  import -window root '$ROOTPNG' 2>/dev/null
  GEOM=\$(convert '$ROOTPNG' -fuzz 1% -format '%@' info:)
  echo \"[scroll] game rect: \$GEOM\"
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[scroll] open dialog: click rect+($GX1,$GY1)\"
  /tmp/xclick \$((OX+$GX1)) \$((OY+$GY1))
  sleep $GAP
  echo \"[scroll] hold down-arrow rect+($AX,$AY) for ${HOLDMS}ms (screenshot MID-hold)\"
  /tmp/xhold \$((OX+$AX)) \$((OY+$AY)) $HOLDMS &
  HPID=\$!
  sleep $SETTLE
  import -window root '$ROOTPNG' 2>/dev/null
  wait \$HPID 2>/dev/null
  echo \"\$GEOM\" > /tmp/fist_scroll_geom.txt
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
GEOM="$(cat /tmp/fist_scroll_geom.txt)"
echo "[scroll] cropping post-scroll at $GEOM"
convert "$ROOTPNG" -crop "$GEOM" +repage "$OUT"
rm -f "$ROOTPNG"
[ -f "$OUT" ] && echo "[scroll] captured -> $OUT" || { echo FAILED; exit 1; }
