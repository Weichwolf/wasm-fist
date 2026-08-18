#!/usr/bin/env bash
# GENUINE 320x200 DOSBox ref of the first settled in-mission cockpit frame for a battle at SELECT-BATTLE
# list INDEX (0-based, DOSBox-ascending). Scrolls the list by paging the scrollbar track (172,115) down
# 8 rows per click, then clicks the in-view row. Non-circular (output=surface, scaler=none, XTest clicks,
# crop the game rect, 6->8bit VGA bit-replication). Write-isolated (throwaway cp -a of armoredfist).
#   usage: refcapture_mission_scroll.sh INDEX OUT.png [SETTLE]
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
IDX="${1:?list index}"; OUT="${2:?out png}"; SETTLE=${3:-2}
PAGES=$(( IDX / 8 )); OFF=$(( IDX % 8 ))
RX=130; RY=$(( 94 + (OFF*64+5)/10 ))          # y = 94 + round(off*6.4)
TRACKX=172; TRACKY=115
GX1=160; GY1=100; GX2=205; GY2=128; GX3=40; GY3=186
WAIT=42; GAPB=6; GAPP=3; GAPR=9; GAPOK=9; GAPACC=9
SCRATCH="$(mktemp -d /tmp/fist_refmsnscroll.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"; trap 'rm -rf "$SCRATCH"' EXIT
CONF="$ROOT/scratch/refcap/dosbox_msnscroll.conf"; mkdir -p "$(dirname "$CONF")"
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
ROOTPNG="$(mktemp /tmp/fist_msnscrollroot.XXXX.png)"
BUDGET=$((WAIT+GAPB+PAGES*GAPP+GAPR+GAPOK+GAPACC+SETTLE+60))
timeout $BUDGET xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  dosbox -conf '$CONF' -exit >/tmp/fist_refcap_msnscroll.log 2>&1 &
  DPID=\$!
  sleep $WAIT
  import -window root '$ROOTPNG' 2>/dev/null
  GEOM=\$(convert '$ROOTPNG' -fuzz 1% -format '%@' info:)
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/'); OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[scroll] idx=$IDX pages=$PAGES off=$OFF row=($RX,$RY) geom \$GEOM  OX=\$OX OY=\$OY\"
  /tmp/xclick \$((OX+$GX1)) \$((OY+$GY1)); sleep $GAPB        # BATTLES -> open list
  for p in \$(seq 1 $PAGES); do
    /tmp/xclick \$((OX+$TRACKX)) \$((OY+$TRACKY)); sleep $GAPP  # page down 8 rows
  done
  /tmp/xclick \$((OX+$RX))  \$((OY+$RY)); sleep $GAPR         # select the battle row
  /tmp/xclick \$((OX+$GX2)) \$((OY+$GY2)); sleep $GAPOK       # OK -> briefing
  /tmp/xclick \$((OX+$GX3)) \$((OY+$GY3)); sleep $GAPACC      # ACCEPT -> mission
  sleep $SETTLE
  import -window root '$ROOTPNG' 2>/dev/null
  echo \"\$GEOM\" > /tmp/fist_msnscroll_geom.txt
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
GEOM="$(cat /tmp/fist_msnscroll_geom.txt)"
convert "$ROOTPNG" -crop "$GEOM" +repage "$OUT"; rm -f "$ROOTPNG"
[ -f "$OUT" ] && echo "[scroll] captured -> $OUT" || { echo FAILED; exit 1; }
