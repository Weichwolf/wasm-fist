#!/usr/bin/env bash
# Pin the base-ray-curve (0x3a24/0x3e24) PRODUCER via dosbox-fist's CR3-aware flat-writer
# watch. Correct mechanism: FIST_WATCHFLAT sets the watched flat range at startup; SIGUSR1
# arms the recorder POST-BOOT (no decompression penalty) + resets the flat histogram; the
# game writes 3a24 during mission-load; SIGUSR2 -> fist_dump() writes $FISTLOG.flatwriters.txt.
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
export FISTLOG="$FISTLOG" FIST_WATCHFLAT=0x10003a24 FIST_WATCHFLATSPAN=0x800
XC="$ROOT/tools/oracle/xclick.py"
rm -f "$FISTLOG".* 2>/dev/null || true
timeout 260 xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c '
  "'$DBX'" -conf "'$SC'/db.conf" -exit > /tmp/3a24_dbx.log 2>&1 &
  DP=$!
  sleep 44
  DPID=$(pgrep -n -f "dosbox-fist -conf" || echo $DP)
  echo "dosbox pid=$DPID"
  import -window root "'$SC'/m.png" 2>/dev/null
  G=$(convert "'$SC'/m.png" -fuzz 1% -format "%@" info:)
  OX=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\1/"); OY=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\2/")
  echo "OX=$OX OY=$OY"
  kill -USR1 $DPID; echo "SIGUSR1 sent (flat-watch armed)"    # arm BEFORE battle-select/mission-load
  python3 "'$XC'" $((OX+160)) $((OY+100)); sleep 10           # BATTLES
  python3 "'$XC'" $((OX+118)) $((OY+93));  sleep 3            # select battle row
  python3 "'$XC'" $((OX+205)) $((OY+128)); sleep 14           # OK
  python3 "'$XC'" $((OX+40))  $((OY+186)); sleep 30           # ACCEPT -> mission renders (3a24 built)
  kill -USR2 $DPID; echo "SIGUSR2 sent (dump)"; sleep 4       # dump -> flatwriters.txt
' 2>&1 | grep -iE 'pid=|OX=|USR|fist-oracle|dumped|error' || true
echo "=== FLATWRITERS (the 3a24 producer) ==="
cat "$FISTLOG".flatwriters.txt 2>/dev/null | head -40 || echo "NO flatwriters.txt"
