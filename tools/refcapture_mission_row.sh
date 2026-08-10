#!/usr/bin/env bash
# GENUINE 320x200 DOSBox ref of the first settled in-mission frame for a NON-DEFAULT battle, reached by
# BATTLES -> click battle LIST ROW (RX,RY) -> OK -> ACCEPT. Same non-circular method as refcapture_mission.sh
# (output=surface, scaler=none, XTest clicks, crop the game rect, 6->8bit VGA bit-replication). Write-isolated.
#   usage: refcapture_mission_row.sh RX RY OUT.png
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
RX="${1:?row X}"; RY="${2:?row Y}"; OUT="${3:?out png}"
GX1=160; GY1=100; GX2=205; GY2=128; GX3=40; GY3=186
WAIT=42; GAP0=6; GAP1=9; GAP2=9; SETTLE=${4:-28}
SCRATCH="$(mktemp -d /tmp/fist_refmsnrow.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"; trap 'rm -rf "$SCRATCH"' EXIT
CONF="$ROOT/scratch/refcap/dosbox_msnrow.conf"; mkdir -p "$(dirname "$CONF")"
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
ROOTPNG="$(mktemp /tmp/fist_msnrowroot.XXXX.png)"
timeout $((WAIT+GAP0+GAP1+GAP2+SETTLE+50)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  dosbox -conf '$CONF' -exit >/tmp/fist_refcap_msnrow.log 2>&1 &
  DPID=\$!
  sleep $WAIT
  import -window root '$ROOTPNG' 2>/dev/null
  GEOM=\$(convert '$ROOTPNG' -fuzz 1% -format '%@' info:)
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/'); OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[row] geom \$GEOM  OX=\$OX OY=\$OY\"
  /tmp/xclick \$((OX+$GX1)) \$((OY+$GY1)); sleep $GAP0      # BATTLES -> open list
  /tmp/xclick \$((OX+$RX))  \$((OY+$RY)); sleep $GAP1       # select the battle row
  /tmp/xclick \$((OX+$GX2)) \$((OY+$GY2)); sleep $GAP2      # OK -> briefing
  /tmp/xclick \$((OX+$GX3)) \$((OY+$GY3)); sleep $SETTLE    # ACCEPT -> mission
  import -window root '$ROOTPNG' 2>/dev/null
  echo \"\$GEOM\" > /tmp/fist_msnrow_geom.txt
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
GEOM="$(cat /tmp/fist_msnrow_geom.txt)"
convert "$ROOTPNG" -crop "$GEOM" +repage "$OUT"; rm -f "$ROOTPNG"
[ -f "$OUT" ] && echo "[row] captured -> $OUT" || { echo FAILED; exit 1; }
