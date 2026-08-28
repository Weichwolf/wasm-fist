#!/usr/bin/env bash
set -e
ROOT="/home/cosmo/Git/wasm-fist"
DOSBOX="$ROOT/third_party/dosbox-fist"
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -o /tmp/xclick -lX11 -l:libXtst.so.6
WORK="$(mktemp -d /tmp/oc.XXXXXX)"; DATA="$WORK/armoredfist"; cp -a "$ROOT/armoredfist" "$DATA"
CONF="$WORK/db.conf"; PREFIX="/tmp/ocensus"; rm -f "$PREFIX".*
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
export FISTLOG="$PREFIX" FIST_MEMARM_BOOT=1 FIST_TILEPHYS=0x141000
export FIST_MEMDUMP_N="${OC_N:-1500}"
"$DOSBOX" -conf "$CONF" -exit >"$PREFIX.dosbox.log" 2>&1 &
DPID=$!
sleep 55
import -window root "$WORK/root.png" 2>/dev/null || true
GEOM=$(convert "$WORK/root.png" -fuzz 1% -format '%@' info: 2>/dev/null || echo "320x200+0+0")
OX=$(echo "$GEOM" | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/'); OY=$(echo "$GEOM" | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
echo "[oc] menu rect $GEOM"
/tmp/xclick $((OX+160)) $((OY+100)); sleep 12   # BATTLES
/tmp/xclick $((OX+205)) $((OY+128)); sleep 12   # OK
/tmp/xclick $((OX+40))  $((OY+186)); sleep 10   # ACCEPT
import -window root "$PREFIX.cockpit.png" 2>/dev/null || true   # verify mission reached
echo "[oc] in mission; running to auto-dump (N=${OC_N:-1500}M writes)"
sleep 60
kill $DPID 2>/dev/null || true; wait 2>/dev/null || true
echo "[oc] ram.bin:"; ls -la "$PREFIX".ram.bin 2>/dev/null || echo "  (none)"
echo "[oc] memdump/oracle log:"; grep -iE "fist-memdump|fist-oracle|dumped" "$PREFIX.dosbox.log" | tail -3 || echo "  (none)"
rm -rf "$WORK"
