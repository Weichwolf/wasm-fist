#!/usr/bin/env bash
# board:0017 -- does the ORIGINAL resolve a mission to VICTORY/DEFEAT when the player never touches the
# controls?  Drives stock FIST.RUN under the instrumented DOSBox to the default battle with the same
# click sequence tools/selfplay.sh feeds the port (160,100 BATTLES / 205,128 OK / 40,186 ACCEPT), then
# sits still.  FIST_WATCHFLAT is armed on the engine-flat address of DGROUP:0x6da0 -- the outcome word
# FUN_1000_a5dc writes (0 VICTORY at 1a6ae, 1 DEFEAT at 1a6bb, 2 TIME EXPIRED at 1a60b) -- with a 0x10
# span that also covers its countdown 0x6da2, the mission clock 0x6da6/7/8 and the 0x6dab gate.  DGROUP
# is segment 0x1c00 over an image loaded at linear 0, so the flat address is 0x1c000+0x6da0 = 0x22da0.
#
#   OC_WALL=<sec>  how long to sit in the mission (default 1500 = 25 min wall)
#   the mission's own limit is 15 or 30 min of MISSION time (table DGROUP:0x7b14 = 05 0f 1e ff)
set -e
ROOT="/home/cosmo/Git/wasm-fist"
DOSBOX="$ROOT/third_party/dosbox-fist"
export DISPLAY="${DISPLAY:-:77}"
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -o /tmp/xclick -lX11 -l:libXtst.so.6
WORK="$(mktemp -d /tmp/oco.XXXXXX)"; DATA="$WORK/armoredfist"; cp -a "$ROOT/armoredfist" "$DATA"
CONF="$WORK/db.conf"; PREFIX="${OC_PREFIX:-/tmp/ooutcome}"; rm -f "$PREFIX".*
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
echo "[oco] menu rect $GEOM"
/tmp/xclick $((OX+160)) $((OY+100)); sleep 12   # BATTLES
/tmp/xclick $((OX+205)) $((OY+128)); sleep 12   # OK
/tmp/xclick $((OX+40))  $((OY+186)); sleep 10   # ACCEPT
import -window root "$PREFIX.cockpit.png" 2>/dev/null || true
echo "[oco] in mission; sitting still for ${OC_WALL:-1500}s (no input)"
sleep "${OC_WALL:-1500}"
import -window root "$PREFIX.final.png" 2>/dev/null || true
kill -USR2 $DPID 2>/dev/null || true; sleep 20     # request the RAM dump, let it flush
kill $DPID 2>/dev/null || true; wait 2>/dev/null || true
echo "[oco] watch log:"; ls -la "$PREFIX".watch.txt 2>/dev/null || echo "  (none)"
echo "[oco] last writes in the window (outcome word = phys 0x33f30..33f31):"
tail -30 "$PREFIX.watch.txt" 2>/dev/null || true
echo "[oco] gate variables from the RAM dump:"; ls -la "$PREFIX".ram.bin 2>/dev/null && python3 "$ROOT/tools/oracle/read_counters.py" "$PREFIX.ram.bin" || echo "  (no ram.bin)"
rm -rf "$WORK"
