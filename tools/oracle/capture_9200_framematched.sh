#!/usr/bin/env bash
# =============================================================================
# capture_9200_framematched.sh -- the globals<->VRAM FRAME-MATCHED 9200 capture.
#
# Drives the ORIGINAL Armored Fist (BATTLES->OK->ACCEPT into the AZER1 spawn)
# under the instrumented DOSBox with FIST_R9200CAP=1, which captures -- for the
# first N terrain frames -- a SELF-CONSISTENT (globals, VRAM, DAC, tile) triple
# from ONE FUN_0000_9200 invocation:
#   * globals snapshot taken on the FIRST 0xA0000 write of a 9200 pass (the exact
#     projection state 9200 reads: 90d4/90d8 base, ESI/EBP per-texel step, 90b8/
#     90bc per-column step, 90f0/90f8 counts, the 9114 horizon table);
#   * VRAM dumped when 8120 next runs (frame F+1's projection recompute), which by
#     82b8's {8120;9200;82d0} order happens BEFORE 9200-F+1 -> the VRAM still holds
#     frame F's terrain, matched to frame F's globals.
# Unlike the SIGUSR2 dump, this fires synchronously in graphics/pmode (no text-mode
# skew) and is per-9200-call frame-matched by construction.
#
# Outputs (FISTLOG prefix, default scratch/oracle/r92):
#   <pfx>.passNN.cap      = hdr + ext[0x9000..0x9200] + horizon[256] + tile[65536]
#   <pfx>.passNN.vram.bin = 256KB raw chain-4 VGA memory
#   <pfx>.passNN.pal.bin  = 768B DAC
# stderr shows per-pass esi/ebp/90d8 so the non-degenerate gradient frame is picked.
#
# Env: DOSBOX (default /tmp/debs/dosbox-fist), FISTLOG, FIST_R9200MAX (default 16).
# =============================================================================
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DOSBOX="${DOSBOX:-/tmp/debs/dosbox-fist}"
PREFIX="${FISTLOG:-$ROOT/scratch/oracle/r92}"
MAXPASS="${FIST_R9200MAX:-16}"
[ -x "$DOSBOX" ] || { echo "missing instrumented dosbox at $DOSBOX"; exit 1; }
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -o /tmp/xclick -lX11 -l:libXtst.so.6
mkdir -p "$(dirname "$PREFIX")"
rm -f "$PREFIX".pass*.cap "$PREFIX".pass*.vram.bin "$PREFIX".pass*.pal.bin
SCRATCH="$(mktemp -d /tmp/fist_r92.XXXXXX)"; DATA="$SCRATCH/armoredfist"
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
timeout 200 xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  export FISTLOG='$PREFIX' FIST_R9200CAP=1 FIST_R9200MAX='$MAXPASS'
  '$DOSBOX' -conf '$CONF' -exit >'$LOG' 2>&1 &
  DPID=\$!
  sleep 60
  import -window root '$SCRATCH/root.png' 2>/dev/null
  GEOM=\$(convert '$SCRATCH/root.png' -fuzz 1% -format '%@' info:)
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  echo \"[r92] menu rect \$GEOM\"
  echo '[r92] BATTLES'; /tmp/xclick \$((OX+160)) \$((OY+100)); sleep 12
  echo '[r92] OK';      /tmp/xclick \$((OX+205)) \$((OY+128)); sleep 12
  echo '[r92] ACCEPT';  /tmp/xclick \$((OX+40))  \$((OY+186))
  # let the mission run long enough for MAXPASS terrain frames to render + dump
  for i in \$(seq 1 60); do
     N=\$(ls '$PREFIX'.pass*.cap 2>/dev/null | wc -l)
     echo \"[r92] poll \$i captured=\$N/'$MAXPASS'\"
     [ \$N -ge '$MAXPASS' ] && break
     sleep 1
  done
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
echo '=== per-pass projection log (esi/ebp/90d8) ==='
grep -a '\[r92cap\]' "$LOG" || echo '(no passes captured -- check db.log)'
echo '=== captured files ==='
ls -la "$PREFIX".pass*.cap 2>/dev/null | tail -20
