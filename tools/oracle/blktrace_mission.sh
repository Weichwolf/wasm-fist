#!/usr/bin/env bash
# board:0013 / board:0002 -- every instruction the ORIGINAL executes from boot through OC_WALL seconds
# of a battle, ONCE each in first-execution order (FIST_BLKTRACE; OC_CS=<hex> scopes to one code
# segment -- the oracle's 1119 is the engine's segment 0, 2082 its 0f69 cluster, 2119 its 1000 cluster;
# the extender's 32-bit code runs under a PM selector, so leave OC_CS unset to see it), so the windshield's
# real render chain can be read off the trace instead of guessed.  Same drive as census_debrief.sh
# (OC_BATTLE narrows the .FSG list to one battle).  Output: $OC_OUT/blk.txt, $OC_OUT/marks.txt.
set -e
ROOT="/home/cosmo/Git/wasm-fist"
DOSBOX="$ROOT/third_party/dosbox-fist"
if [ -z "$OC_INNER" ]; then
  exec env OC_INNER=1 xvfb-run -a --server-args="-screen 0 1024x768x24" bash "$0" "$@"
fi
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -o /tmp/xclick -lX11 -l:libXtst.so.6
WORK="$(mktemp -d /tmp/ocb.XXXXXX)"; DATA="$WORK/armoredfist"; cp -a "$ROOT/armoredfist" "$DATA"
if [ -n "${OC_BATTLE:-}" ]; then
  find "$DATA/FISTDATA" -maxdepth 1 -iname '*.fsg' ! -iname "$OC_BATTLE.fsg" -delete
fi
CONF="$WORK/db.conf"; OUT="${OC_OUT:-$ROOT/scratch/oracle/blktrace}"; mkdir -p "$OUT"
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
export FIST_BLKTRACE="$OUT/blk.txt"
[ "${OC_CS:-all}" = all ] || export FIST_BLKTRACE_CS="$OC_CS"   # OC_CS=<hex selector> scopes to one CS
"$DOSBOX" -conf "$CONF" -exit >"$OUT/dosbox.log" 2>&1 &
DPID=$!
sleep 55
import -window root "$WORK/root.png" 2>/dev/null || true
GEOM=$(convert "$WORK/root.png" -fuzz 1% -format '%@' info: 2>/dev/null || echo "320x200+0+0")
OX=$(echo "$GEOM" | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/'); OY=$(echo "$GEOM" | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
echo "[ocb] menu rect $GEOM; lines before BATTLES: $(wc -l < "$OUT/blk.txt")" | tee "$OUT/marks.txt"
/tmp/xclick $((OX+160)) $((OY+100)); sleep 12   # BATTLES
/tmp/xclick $((OX+205)) $((OY+128)); sleep 12   # OK
echo "[ocb] lines before ACCEPT: $(wc -l < "$OUT/blk.txt")" | tee -a "$OUT/marks.txt"
/tmp/xclick $((OX+40))  $((OY+186)); sleep 10   # ACCEPT
import -window root "$OUT/cockpit.png" 2>/dev/null || true
echo "[ocb] in mission; lines: $(wc -l < "$OUT/blk.txt")" | tee -a "$OUT/marks.txt"
sleep "${OC_WALL:-120}"
kill $DPID 2>/dev/null || true; wait 2>/dev/null || true
echo "[ocb] done; lines: $(wc -l < "$OUT/blk.txt")" | tee -a "$OUT/marks.txt"
rm -rf "$WORK"
