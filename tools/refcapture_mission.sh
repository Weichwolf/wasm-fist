#!/usr/bin/env bash
# GENUINE NATIVE 320x200 DOSBox reference of the FIRST SETTLED IN-MISSION FRAME (AZER1), reached by the
# three-click BATTLES -> OK -> ACCEPT route (the same game pixels the port's FIST_MOUSE harness uses:
# BATTLES(160,100), OK(205,128), ACCEPT(40,186)).  Identical non-circular method to refcapture_ok.sh
# (output=surface, scaler=none, -trim to the game rect, XTest clicks via /tmp/xclick, crop that rect,
# 6->8bit VGA bit-replication) so the grab pixels ARE the native mode-13h framebuffer with no scaling.
# Write-isolated: DOSBox mounts a throwaway cp -a of armoredfist/ so any mission/player write never
# touches the repo (git stays clean).
#   usage: refcapture_mission.sh GX1 GY1 GX2 GY2 GX3 GY3 WAIT GAP1 GAP2 SETTLE OUT.png
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GX1="${1:-160}"; GY1="${2:-100}"; GX2="${3:-205}"; GY2="${4:-128}"; GX3="${5:-40}"; GY3="${6:-186}"
WAIT="${7:-42}"; GAP1="${8:-9}"; GAP2="${9:-9}"; SETTLE="${10:-28}"
OUT="${11:-$ROOT/ref/mission_azer1_native320.png}"
SCRATCH="$(mktemp -d /tmp/fist_refmsn.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SCRATCH"' EXIT
CONF="$ROOT/scratch/refcap/dosbox_mission.conf"; mkdir -p "$(dirname "$CONF")"
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
ROOTPNG="$(mktemp /tmp/fist_msnroot.XXXX.png)"
timeout $((WAIT+GAP1+GAP2+SETTLE+50)) xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  dosbox -conf '$CONF' -exit >/tmp/fist_refcap_mission.log 2>&1 &
  DPID=\$!
  sleep $WAIT
  import -window root '$ROOTPNG' 2>/dev/null
  GEOM=\$(convert '$ROOTPNG' -fuzz 1% -format '%@' info:)
  echo \"[msn] menu game rect: \$GEOM\"
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[msn] click1 BATTLES rect+($GX1,$GY1)\";  /tmp/xclick \$((OX+$GX1)) \$((OY+$GY1)); sleep $GAP1
  echo \"[msn] click2 OK rect+($GX2,$GY2)\";       /tmp/xclick \$((OX+$GX2)) \$((OY+$GY2)); sleep $GAP2
  echo \"[msn] click3 ACCEPT rect+($GX3,$GY3)\";   /tmp/xclick \$((OX+$GX3)) \$((OY+$GY3)); sleep $SETTLE
  import -window root '$ROOTPNG' 2>/dev/null
  echo \"\$GEOM\" > /tmp/fist_msn_geom.txt
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
GEOM="$(cat /tmp/fist_msn_geom.txt)"
echo "[msn] cropping post-ACCEPT frame at $GEOM"
convert "$ROOTPNG" -crop "$GEOM" +repage "$OUT"
rm -f "$ROOTPNG"
[ -f "$OUT" ] && echo "[msn] captured -> $OUT (repo armoredfist/ untouched; scratch=$SCRATCH)" || { echo FAILED; exit 1; }
