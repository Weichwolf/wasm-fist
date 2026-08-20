#!/usr/bin/env bash
# Pin the base-ray-curve (3a24/3e24) PRODUCER via dosbox-fist's CR3-aware flat-writer watch.
# Synthesis of the mechanism: FIST_MEMARM_BOOT's __attribute__((constructor)) sets fist_prefix
# from FISTLOG + arms the RAM recorder + parses FIST_WATCHFLAT ALL from boot -- so the early
# mission-load 3a24 write is recorded AND the output prefix is valid. Patient timing absorbs the
# decompression slowdown. SIGUSR2 (pkill -f, guaranteed PID) -> fist_dump() -> flatwriters.txt.
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
FISTLOG="${1:-/tmp/oracle_3a24}"
DBX="$ROOT/third_party/dosbox-fist"; [ -x "$DBX" ] || { echo "no dosbox-fist"; exit 1; }
SC="$(mktemp -d /tmp/orc3a24.XXXXXX)"; DATA="$SC/armoredfist"; cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SC"' EXIT
cat > "$SC/db.conf" <<CFG
[sdl]
fullscreen=false
output=surface
autolock=false
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
export FISTLOG="$FISTLOG" FIST_MEMARM_BOOT=1 FIST_WATCHFLAT=0x10003a24 FIST_WATCHFLATSPAN=0x800
XC="$ROOT/tools/oracle/xclick.py"
rm -f "$FISTLOG".* 2>/dev/null || true
timeout 700 xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c '
  "'"$DBX"'" -conf "'"$SC"'/db.conf" -exit > /tmp/3a24_dbx.log 2>&1 &
  sleep 230
  import -window root "'"$SC"'/m.png" 2>/dev/null
  G=$(convert "'"$SC"'/m.png" -fuzz 1% -format "%@" info:)
  OX=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\1/"); OY=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\2/")
  echo "OX=$OX OY=$OY"
  python3 "'"$XC"'" $((OX+160)) $((OY+100)); sleep 24
  python3 "'"$XC"'" $((OX+118)) $((OY+93));  sleep 8
  python3 "'"$XC"'" $((OX+205)) $((OY+128)); sleep 30
  python3 "'"$XC"'" $((OX+40))  $((OY+186)); sleep 70
  pkill -USR2 -f "dosbox-fist -conf"; echo "SIGUSR2 sent"; sleep 6
' 2>&1 | grep -iE "OX=|USR|fist-oracle|dumped|armed" || true
echo "=== FLATWRITERS (the 3a24 producer) ==="
cat "$FISTLOG".flatwriters.txt 2>/dev/null | head -40 || echo "NO flatwriters.txt"
echo "=== all outputs ==="; ls -la "$FISTLOG".* 2>/dev/null | head
