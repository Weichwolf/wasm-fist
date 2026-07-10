#!/usr/bin/env bash
# GENUINE NATIVE 320x200 DOSBox reference of a WRITE-ISOLATED "OK" interaction reached by TWO clicks:
# first a menu item (opens a list dialog), then the OK button INSIDE it (which SAVES a player .FPL to
# disk and returns to the menu with "CURRENT PLAYER: <name>").  Identical method to refcapture_click2.sh
# (output=surface, scaler=none, -trim to the game rect, XTest clicks, crop the same rect, 6->8bit VGA
# bit-replication) EXCEPT it mounts a FRESH cp -a scratch copy of armoredfist/ so the engine's real
# create/write of the .FPL never mutates the repo -- the write-isolation policy, DOSBox side (mirrors
# tools/verify.sh's per-run scratch datadir for WRITE flows).  git status stays clean after a capture.
#   usage: refcapture_ok3.sh GX1 GY1 RX RY GX2 GY2 WAIT GAP SETTLE OUT.png
#          (GX1,GY1 = menu-item pixel; RX,RY = list-ROW pixel to select; GX2,GY2 = OK-button pixel)
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GX1="${1:-160}"; GY1="${2:-74}"; RX="${3:-130}"; RY="${4:-119}"; GX2="${5:-205}"; GY2="${6:-128}"
WAIT="${7:-40}"; GAP="${8:-8}"; SETTLE="${9:-8}"; OUT="${10:-$ROOT/ref/selplayer_ok_kkr_native320.png}"
# WRITE ISOLATION: a fresh throwaway copy of the game dir that DOSBox mounts (so its .FPL write lands here)
SCRATCH="$(mktemp -d /tmp/fist_refok.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SCRATCH"' EXIT
CONF="$ROOT/scratch/refcap/dosbox_ok.conf"; mkdir -p "$(dirname "$CONF")"
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
ROOTPNG="$(mktemp /tmp/fist_okroot.XXXX.png)"
timeout $((WAIT+GAP+SETTLE+40)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  dosbox -conf '$CONF' -exit >/tmp/fist_refcap_ok.log 2>&1 &
  DPID=\$!
  sleep $WAIT
  import -window root '$ROOTPNG' 2>/dev/null
  GEOM=\$(convert '$ROOTPNG' -fuzz 1% -format '%@' info:)
  echo \"[ok] game rect: \$GEOM\"
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[ok] click1 menu item rect+($GX1,$GY1)\"
  /tmp/xclick \$((OX+$GX1)) \$((OY+$GY1))
  sleep $GAP
  echo \"[ok] click2 list ROW rect+($RX,$RY)\"
  /tmp/xclick \$((OX+$RX)) \$((OY+$RY))
  sleep $GAP
  echo \"[ok] click3 OK button rect+($GX2,$GY2)\"
  /tmp/xclick \$((OX+$GX2)) \$((OY+$GY2))
  sleep $SETTLE
  import -window root '$ROOTPNG' 2>/dev/null
  echo \"\$GEOM\" > /tmp/fist_ok_geom.txt
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
GEOM="$(cat /tmp/fist_ok_geom.txt)"
echo "[ok] cropping post-interaction at $GEOM"
convert "$ROOTPNG" -crop "$GEOM" +repage "$OUT"
rm -f "$ROOTPNG"
[ -f "$OUT" ] && echo "[ok] captured -> $OUT (repo armoredfist/ untouched; scratch=$SCRATCH)" || { echo FAILED; exit 1; }
