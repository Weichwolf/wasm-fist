#!/usr/bin/env bash
# =============================================================================
# capture_6980_framematched.sh -- the 6980 voxel-raycaster COLOUR-GATE capture.
#
# Runs the instrumented DOSBox with BOTH FIST_R9200CAP=1 (the affine-renderer
# globals<->VRAM frame-match, unchanged) AND FIST_R6980CAP=1 (NEW: the voxel
# tile-fill's OWN world-space camera + render-time colormap + depth ramps).
#
# Why both: the r92 .cap captures the camera at 9200-entry, but 8120 runs between
# 6980 and 9200 and OVERWRITES 90d4/90d8 with the tile-space camera -- so the .cap
# globals are useless for 6980 (world-space).  FIST_R6980CAP snapshots 6980's own
# camera DURING 6980 and dumps the colormap read through LIVE paging AT 6980
# execution (mid-fill), so the dumped colormap is the exact bytes 6980
# dereferences this frame -- not the frame-skewed 8120-boundary page a full RAM
# dump catches.  The r6980 pass number is shared with the r92 .cap, so pass NN's
# 6980 inputs and pass NN's .cap tile are the SAME frame.
#
# Outputs (FISTLOG prefix, default scratch/oracle/r69):
#   <pfx>.pass??.cap            (r92: hdr + ext[0x9000..0x9200] + horizon + tile)
#   <pfx>.r6980.pass??.bin      (r69: hdr + world cam ext[0x9000..0x9200] + 16 named
#                                globals + 3a24/3e24/4224/4624 ramps + tile + 6980's
#                                actual stores 'shadow')
#   <pfx>.r6980.map_{hm,cm,reduce}.bin  (render-time heightmap/colormap/reduce, 1MB each)
#   <pfx>.r6980.proj.bin        (render-time projection table, 64KB)
# stderr shows per-pass 6980 store value stats (LIGHT/dark = the gate-A answer).
#
# Env: DOSBOX (default /tmp/debs/dosbox-fist), FISTLOG, FIST_R9200MAX (default 16).
# =============================================================================
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DOSBOX="${DOSBOX:-/tmp/debs/dosbox-fist}"
PREFIX="${FISTLOG:-$ROOT/scratch/oracle/r69}"
MAXPASS="${FIST_R9200MAX:-16}"
[ -x "$DOSBOX" ] || { echo "missing instrumented dosbox at $DOSBOX"; exit 1; }
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -I/tmp/debs/sysroot/usr/include \
    -L/tmp/debs/sysroot/usr/lib/x86_64-linux-gnu -o /tmp/xclick -lX11 -lXtst
mkdir -p "$(dirname "$PREFIX")"
rm -f "$PREFIX".pass*.cap "$PREFIX".r6980.*
SCRATCH="$(mktemp -d /tmp/fist_r69.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SCRATCH"' EXIT
CONF="$SCRATCH/db.conf"
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
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
LOG="$SCRATCH/db.log"
timeout 220 xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  export FISTLOG='$PREFIX' FIST_R9200CAP=1 FIST_R6980CAP=1 FIST_R9200MAX='$MAXPASS'
  '$DOSBOX' -conf '$CONF' -exit >'$LOG' 2>&1 &
  DPID=\$!
  sleep 60
  import -window root '$SCRATCH/root.png' 2>/dev/null
  GEOM=\$(convert '$SCRATCH/root.png' -fuzz 1% -format '%@' info:)
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[r69] menu rect \$GEOM\"
  echo '[r69] BATTLES'; /tmp/xclick \$((OX+160)) \$((OY+100)); sleep 12
  echo '[r69] OK';      /tmp/xclick \$((OX+205)) \$((OY+128)); sleep 12
  echo '[r69] ACCEPT';  /tmp/xclick \$((OX+40))  \$((OY+186))
  for i in \$(seq 1 60); do
     N=\$(ls '$PREFIX'.r6980.pass*.bin 2>/dev/null | wc -l)
     echo \"[r69] poll \$i captured=\$N/'$MAXPASS'\"
     [ \$N -ge '$MAXPASS' ] && break
     sleep 1
  done
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
echo '=== per-pass 6980 store stats + read-side capture (gate A: LIGHT/dark) ==='
grep -aE '\[r6980' "$LOG" || echo '(none -- check db.log)'
echo '=== per-pass 92cap projection log ==='
grep -a '\[r92cap\]' "$LOG" | head -20
echo '=== captured r6980 files ==='
ls -la "$PREFIX".r6980.pass*.bin 2>/dev/null | tail -20
ls -la "$PREFIX".r6980.map_*.bin "$PREFIX".r6980.proj.bin 2>/dev/null
