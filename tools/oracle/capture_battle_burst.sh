#!/usr/bin/env bash
# =============================================================================
# capture_battle_burst.sh -- GENUINE oracle per-battle spawn capture that DEFEATS
# the original's post-spawn auto-cycle by BURST grabbing.
#
# The stock capture (capture_battle_stock.sh) grabs a single "settled" frame, which
# is unreliable: after spawn the ORIGINAL auto-cycles the mission view to OTHER
# (friendly) units -- including a friendly M1 every few seconds -- so a lone grab can
# land on the wrong view.  This variant instead saves EVERY polled frame across
# ACCEPT+20..40s into $OUTDIR/f01..f40.png; the caller then crops each central chrome
# (cols80-180 rows96-188) and SELECTS the frame whose dashboard matches the port's
# op-0x2c spawn -- the cycle is no longer a blocker.
#
# Proven on AZER6 (row OY+123): the player's NON-M1 vehicle dashboard recurs every
# cycle pass and matches the port to MIN AE=2 over 40 frames (a real 2px dashboard
# micro-bug at abs(114,135)+(114,136)); the friendly-M1 frames are the cycle, not the
# spawn.  Non-circular: the reference is DOSBox running the ORIGINAL FIST.RUN.
#
# Usage: capture_battle_burst.sh ROW_Y_OFFSET [OUTDIR]
#   ROW_Y_OFFSET  battle-row Y as an OFFSET from the detected surface OY
#                 (AZER1 row1 = +93, ~6px pitch; AZER6 = +123, CYPRUS1 row8 = +135;
#                  rows past CYPRUS1 need the scrollbar -- not handled here).
#   OUTDIR        default /tmp/oracle_burst  (cleared then filled with f01..f40.png)
# =============================================================================
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ROWOFF="${1:?ROW_Y_OFFSET (e.g. 123 for AZER6) required}"
OUTDIR="${2:-/tmp/oracle_burst}"
DOSBOX="${DOSBOX:-dosbox}"
command -v "$DOSBOX" >/dev/null || { echo "no dosbox"; exit 1; }
SC="$(mktemp -d /tmp/orcb.XXXXXX)"; DATA="$SC/armoredfist"; cp -a "$ROOT/armoredfist" "$DATA"
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
mkdir -p "$OUTDIR"; rm -f "$OUTDIR"/f*.png
timeout 220 xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c '
  "'$DOSBOX'" -conf "'$SC'/db.conf" -exit > "'$SC'/db.log" 2>&1 &
  sleep 42
  import -window root "'$SC'/m.png" 2>/dev/null
  G=$(convert "'$SC'/m.png" -fuzz 1% -format "%@" info:)
  OX=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\1/"); OY=$(echo $G|sed -E "s/.*\+([0-9]+)\+([0-9]+)/\2/")
  echo "OX=$OX OY=$OY"
  python3 "'$XC'" $((OX+160)) $((OY+100)); sleep 10                 # BATTLES
  python3 "'$XC'" $((OX+118)) $((OY+'$ROWOFF'));   sleep 3          # select battle row
  python3 "'$XC'" $((OX+205)) $((OY+128)); sleep 14                 # OK
  python3 "'$XC'" $((OX+40))  $((OY+186)); sleep 20                 # ACCEPT
  for i in $(seq 1 40); do                                          # burst ACCEPT+20..40s
    import -window root "'$SC'/g.png" 2>/dev/null
    GG=$(convert "'$SC'/g.png" -fuzz 1% -format "%@" info:)
    case "$GG" in 320x200+*+*)
       convert "'$SC'/g.png" -crop 320x200+$OX+$OY +repage "'$OUTDIR'"/f$(printf %02d $i).png 2>/dev/null;;
    esac
    sleep 0.5
  done
' 2>&1 | grep -iE 'OX=|error' || true
n=$(ls "$OUTDIR"/f*.png 2>/dev/null | wc -l)
[ "$n" -gt 0 ] && echo "burst: $n frames in $OUTDIR" || { echo "no frames"; exit 1; }
