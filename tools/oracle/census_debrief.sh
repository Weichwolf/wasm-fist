#!/usr/bin/env bash
# board:0017 -- what does the ORIGINAL show after the verdict?  Same drive as census_outcome.sh (stock
# FIST.RUN, default battle, the selfplay.sh click sequence, then no input), but the screen is captured
# every OC_STEP seconds for OC_WALL seconds so the mission -> verdict -> debrief -> whatever-follows
# sequence is on record frame by frame (scratch/oracle/debrief/f_NNNN.png), and the outcome word's
# watch log (DGROUP:0x6da0.. at guest phys 0x33f30) gives the verdict's position in that sequence.
set -e
ROOT="/home/cosmo/Git/wasm-fist"
DOSBOX="$ROOT/third_party/dosbox-fist"
if [ -z "$OC_INNER" ]; then
  exec env OC_INNER=1 xvfb-run -a --server-args="-screen 0 1024x768x24" bash "$0" "$@"
fi
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -o /tmp/xclick -lX11 -l:libXtst.so.6
WORK="$(mktemp -d /tmp/ocd.XXXXXX)"; DATA="$WORK/armoredfist"; cp -a "$ROOT/armoredfist" "$DATA"
CONF="$WORK/db.conf"; OUT="${OC_OUT:-$ROOT/scratch/oracle/debrief}"; mkdir -p "$OUT"; rm -f "$OUT"/f_*.png "$OUT"/outcome.*
PREFIX="$OUT/outcome"
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
core=normal
cputype=auto
cycles=max
[autoexec]
mount c $DATA
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
export FISTLOG="$PREFIX" FIST_MEMARM_BOOT=1
export FIST_WATCHPHYS=0x33f38 FIST_WATCHSPAN=0x8
"$DOSBOX" -conf "$CONF" -exit >"$PREFIX.dosbox.log" 2>&1 &
DPID=$!
sleep 55
import -window root "$WORK/root.png" 2>/dev/null || true
GEOM=$(convert "$WORK/root.png" -fuzz 1% -format '%@' info: 2>/dev/null || echo "320x200+0+0")
OX=$(echo "$GEOM" | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/'); OY=$(echo "$GEOM" | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
echo "[ocd] menu rect $GEOM"
/tmp/xclick $((OX+160)) $((OY+100)); sleep 12   # BATTLES
/tmp/xclick $((OX+205)) $((OY+128)); sleep 12   # OK
/tmp/xclick $((OX+40))  $((OY+186)); sleep 10   # ACCEPT
T0=$(date +%s); STEP="${OC_STEP:-20}"; WALL="${OC_WALL:-1500}"; n=0
echo "[ocd] in mission at $T0; capturing every ${STEP}s for ${WALL}s"
while [ $(( $(date +%s) - T0 )) -lt "$WALL" ]; do
  import -window root "$OUT/f_$(printf '%04d' $n).png" 2>/dev/null || true
  echo "$n $(( $(date +%s) - T0 )) $(wc -l < "$PREFIX.watch.txt" 2>/dev/null || echo 0)" >> "$OUT/frames.txt"
  n=$((n+1)); sleep "$STEP"
done
kill -USR2 $DPID 2>/dev/null || true; sleep 30
kill $DPID 2>/dev/null || true; wait 2>/dev/null || true
echo "[ocd] outcome writes:"; grep -E 'ph=00033f3[0123] ' "$PREFIX.watch.txt" 2>/dev/null | cut -c1-96 | uniq -c | head -40 || true
rm -rf "$WORK"
