#!/usr/bin/env bash
# =============================================================================
# capture_battle_stock.sh -- GENUINE oracle cockpit-spawn capture using the STOCK
# system DOSBox 0.74 (NO instrumented dosbox-fist build required).
#
# Runs the ORIGINAL FIST.RUN under headless Xvfb, drives the menu via XTEST (the
# ctypes tools/oracle/xclick.py -- no xdotool / no XTest dev headers needed),
# navigates BATTLES -> [select row] -> OK -> ACCEPT, and grabs the settled 320x200
# windshield frame with ImageMagick `import`.  output=surface + scaler=none +
# aspect=false => the SDL surface IS the mode-13h 320x200 framebuffer, and DOSBox's
# 6->8 DAC bit-replication matches fist_vga.c / FIST_MISSFB -> the crop is directly
# byte-comparable to the port's frame dump.
#
# VALIDATED (pipeline): a hand-timed grab of AZER1's SETTLED M1 spawn frame (~ACCEPT
# +28s) has central-chrome (cols80-180 rows96-188) AE=0 vs the port's
# ref/mission_azer1_cockpit_native320.png -- proving both the port's faithfulness and
# that stock DOSBox + ctypes-XTEST captures the correct 320x200 frame (no instrumented
# dosbox-fist build needed).  CAVEAT / WIP: the original cycles the mission view a few
# seconds after spawn (M1 windshield -> a secondary vehicle/cinematic), so the M1-spawn
# window is NARROW; reliable automated per-battle capture still needs the settle timing
# tuned to land inside it (grab a burst around ACCEPT+26..30s and keep the M1 frame).
# The ctypes click tool (xclick.py) itself is solid -- menu/list navigation works.
#
# Non-circular: the reference is DOSBox running the ORIGINAL FIST.RUN, never a copy
# of the port output.
#
# Usage: capture_battle_stock.sh [OUT_PNG] [ROW_Y]
#   OUT_PNG  default /tmp/oracle_battle_spawn.png
#   ROW_Y    optional root-Y of the battle row to click after opening BATTLES
#            (omit -> default selection = AZER1).  Rows start ~318 at ~8px pitch.
# =============================================================================
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${1:-/tmp/oracle_battle_spawn.png}"
ROW_Y="${2:-}"
DOSBOX="${DOSBOX:-dosbox}"
command -v "$DOSBOX" >/dev/null || { echo "no dosbox"; exit 1; }
SC="$(mktemp -d /tmp/orc.XXXXXX)"; DATA="$SC/armoredfist"; cp -a "$ROOT/armoredfist" "$DATA"
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
XC="$ROOT/tools/oracle/xclick.py"
timeout 180 xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c '
  "'$DOSBOX'" -conf "'$SC'/db.conf" -exit > "'$SC'/db.log" 2>&1 &
  sleep 42
  # menu rect offset (surface is the only non-background 320x200 region)
  import -window root "'$SC'/m.png" 2>/dev/null
  G=$(convert "'$SC'/m.png" -fuzz 1% -format "%@" info:)
  OX=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\1/"); OY=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\2/")
  python3 "'$XC'" $((OX+160)) $((OY+100)); sleep 10          # BATTLES
  [ -n "'$ROW_Y'" ] && { python3 "'$XC'" $((OX+118)) "'$ROW_Y'"; sleep 3; }   # select row
  python3 "'$XC'" $((OX+205)) $((OY+128)); sleep 14          # OK
  python3 "'$XC'" $((OX+40))  $((OY+186)); sleep 20          # ACCEPT
  # grab the SETTLED windshield frame (poll; the last in-range grab is the settled one)
  for i in $(seq 1 20); do
    import -window root "'$SC'/g.png" 2>/dev/null
    GG=$(convert "'$SC'/g.png" -fuzz 1% -format "%@" info:)
    case "$GG" in 320x200+*+*)
       convert "'$SC'/g.png" -crop 320x200+$OX+$OY +repage "'$SC'/frame.png" 2>/dev/null
       SZ=$(stat -c%s "'$SC'/frame.png" 2>/dev/null||echo 0)
       [ $SZ -gt 12000 ] && [ $SZ -lt 26000 ] && cp "'$SC'/frame.png" "'$OUT'";;
    esac
    sleep 0.5
  done
' 2>&1 | grep -iE 'captured|error' || true
[ -s "$OUT" ] && echo "captured $OUT ($(identify -format '%wx%h' "$OUT" 2>/dev/null))" || { echo "no frame"; exit 1; }
