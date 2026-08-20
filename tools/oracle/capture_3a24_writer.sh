#!/usr/bin/env bash
# Find the base-ray-curve (0x3a24/0x3e24) PRODUCER via dosbox-fist's CR3-aware flat-writer
# watch: arm the RAM recorder from boot, watch ext-flat 0x10003a24..+span, reach a battle
# mission so the curve is built, dump on 9200 render. Output: $FISTLOG.flatwriters.txt.
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
FISTLOG="${1:-/tmp/oracle_3a24}"
DBX="$ROOT/third_party/dosbox-fist"
[ -x "$DBX" ] || { echo "no dosbox-fist"; exit 1; }
SC="$(mktemp -d /tmp/orc3a24.XXXXXX)"; DATA="$SC/armoredfist"; cp -a "$ROOT/armoredfist" "$DATA"
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
export FISTLOG="$FISTLOG" FIST_R9200CAP=1 FIST_R9200MAX=4
export FIST_MEMARM_BOOT=1 FIST_WATCHFLAT=0x10003a24 FIST_WATCHFLATSPAN=0x800
XC="$ROOT/tools/oracle/xclick.py"
rm -f "$FISTLOG".* 2>/dev/null || true
timeout 220 xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c '
  "'$DBX'" -conf "'$SC'/db.conf" -exit > /tmp/3a24_dbx.log 2>&1 &
  sleep 42
  import -window root "'$SC'/m.png" 2>/dev/null
  G=$(convert "'$SC'/m.png" -fuzz 1% -format "%@" info:)
  OX=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\1/"); OY=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\2/")
  python3 "'$XC'" $((OX+160)) $((OY+100)); sleep 10
  python3 "'$XC'" $((OX+118)) $((OY+93));  sleep 3
  python3 "'$XC'" $((OX+205)) $((OY+128)); sleep 14
  python3 "'$XC'" $((OX+40))  $((OY+186)); sleep 32
' 2>&1 | grep -iE 'OX=|fist-oracle|error' || true
echo "=== flatwriters (the 3a24 producer) ==="
cat "$FISTLOG".flatwriters.txt 2>/dev/null || echo "NO flatwriters.txt -- (3a24 not written via ext-flat, or dump did not fire)"
