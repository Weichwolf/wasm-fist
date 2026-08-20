#!/usr/bin/env bash
# Capture the ORIGINAL's render-time TCB camera + guest RAM at a battle spawn,
# using the instrumented dosbox-fist (dosbox_vga_terrain_trace.patch, FIST_R9200CAP).
# Writes $FISTLOG.cam.txt (X/Y/alt/head/pitch/roll/foc/det at 9200-entry) and
# $FISTLOG.passNN.ram.bin.  Usage: capture_tcb_camera.sh ROW_Y_OFFSET [FISTLOG]
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ROWOFF="${1:?ROW_Y_OFFSET (AZER1=93) required}"
FISTLOG="${2:-/tmp/oracle_tcb}"
DBX="$ROOT/third_party/dosbox-fist"
[ -x "$DBX" ] || { echo "no dosbox-fist at $DBX"; exit 1; }
SC="$(mktemp -d /tmp/orctcb.XXXXXX)"; DATA="$SC/armoredfist"; cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SC"' EXIT
cat > "$SC/db.conf" <<CFG
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
core=normal
cputype=auto
cycles=max
[autoexec]
mount c $DATA
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
export FISTLOG="$FISTLOG" FIST_R9200CAP=1 FIST_R9200MAX=3
XC="$ROOT/tools/oracle/xclick.py"
rm -f "$FISTLOG".* 2>/dev/null || true
timeout 200 xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c '
  "'$DBX'" -conf "'$SC'/db.conf" -exit > "'$SC'/db.log" 2>&1 &
  sleep 42
  import -window root "'$SC'/m.png" 2>/dev/null
  G=$(convert "'$SC'/m.png" -fuzz 1% -format "%@" info:)
  OX=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\1/"); OY=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\2/")
  echo "OX=$OX OY=$OY"
  python3 "'$XC'" $((OX+160)) $((OY+100)); sleep 10          # BATTLES
  python3 "'$XC'" $((OX+118)) $((OY+'$ROWOFF'));   sleep 3   # select battle row
  python3 "'$XC'" $((OX+205)) $((OY+128)); sleep 14          # OK
  python3 "'$XC'" $((OX+40))  $((OY+186)); sleep 30          # ACCEPT -> mission renders (9200 fires)
' 2>&1 | grep -iE 'OX=|fist-oracle|r92|error' || true
echo "=== capture outputs ==="
ls -la "$FISTLOG".* 2>/dev/null || echo "NO capture files -- 9200 may not have fired"
