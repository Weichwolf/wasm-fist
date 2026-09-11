#!/usr/bin/env bash
# board:0017 -- per-write trace of the ORIGINAL's PLAYER OBJECT (registry slot 0 = DGROUP:0xc05c) on the
# default battle with no input: every write to +0x00..+0xff (type, X/Y/Z, speed word[+0x57], damage
# byte[+0x1a], the destroyed bit [+0x19]&4 ...) with the live cs:eip, so the port's per-tick trace of the same
# object can be set against it tick for tick.  Same driver as census_outcome.sh; the watch window moves.
#
#   OC_WALL=<sec>  how long to sit in the mission (default 1500 = 25 min wall)
#   the mission's own limit is 15 or 30 min of MISSION time (table DGROUP:0x7b14 = 05 0f 1e ff)
set -e
ROOT="/home/cosmo/Git/wasm-fist"
DOSBOX="$ROOT/third_party/dosbox-fist"
# Runs under its own Xvfb (like the capture_*.sh scripts) and writes under scratch/oracle/ (gitignored,
# survives a reboot -- the first run's evidence died with /tmp).
if [ -z "$OC_INNER" ]; then
  exec env OC_INNER=1 xvfb-run -a --server-args="-screen 0 1024x768x24" bash "$0" "$@"
fi
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -o /tmp/xclick -lX11 -l:libXtst.so.6
WORK="$(mktemp -d /tmp/oco.XXXXXX)"; DATA="$WORK/armoredfist"; cp -a "$ROOT/armoredfist" "$DATA"
CONF="$WORK/db.conf"; PREFIX="${OC_PREFIX:-$ROOT/scratch/oracle/player}"; mkdir -p "$(dirname "$PREFIX")"; rm -f "$PREFIX".*
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
export FIST_WATCHPHYS=$((0x2d190+0xc05c+0x80)) FIST_WATCHSPAN=0x80   # [0x391ec,0x392ec) = player object +0x00..+0xff
export FIST_WATCHMAX=20000000   # the stock cap (40000 entries) fills in 20 s of mission
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
kill -USR2 $DPID 2>/dev/null || true; sleep 30     # request the RAM dump, let it flush
[ -s "$PREFIX.ram.bin" ] || { echo "[oco] no ram.bin after first SIGUSR2 -- retrying"; kill -USR2 $DPID 2>/dev/null || true; sleep 30; }
kill $DPID 2>/dev/null || true; wait 2>/dev/null || true
echo "[oco] watch log:"; ls -la "$PREFIX".watch.txt 2>/dev/null || echo "  (none)"
echo "[oco] writes per object field (phys - 0x391ec), top 40:"
awk '/^WATCH ph=/{split($2,a,"=");printf "%s\n", strtonum("0x"a[2])-0x391ec}' "$PREFIX.watch.txt" 2>/dev/null | sort -n | uniq -c | sort -rn | head -40 || true
echo "[oco] last 12 lines of the window:"; tail -12 "$PREFIX.watch.txt" 2>/dev/null | cut -c1-96 || true
echo "[oco] gate variables from the RAM dump:"; ls -la "$PREFIX".ram.bin 2>/dev/null && python3 "$ROOT/tools/oracle/read_counters.py" "$PREFIX.ram.bin" || echo "  (no ram.bin)"
rm -rf "$WORK"
