#!/usr/bin/env bash
# =============================================================================
# trace_bc90.sh -- capture the ORIGINAL's bc9c BLEND MATRIX at AZER1 map-load,
# overwrite-robustly, at the PHYSICAL address the operative matrix lives at.
#
# Ground truth derivation (see docs/oracle_bc90_capture.md):
#   The extender's data pointer [bc90] holds an ext-flat offset; the extender
#   data segment base = 0x131000 (paging OFF at map-load, linear==phys), so the
#   matrix at ext-flat 0x10000 lives at guest-phys 0x141000.  bdc4 consumes THIS
#   block (proven: the 254-distinct settled colormap value-set is fully contained
#   in the 256-distinct block at 0x141000, and NOT in the 212-distinct block at
#   0x175200).  We watch 0x141000 with the per-byte LAST-WRITER map (FIST_TILEPHYS)
#   so the captured matrix == bc9c's output regardless of write ordering, and the
#   <prefix>.lastwriters.txt histogram proves WHICH code owns each final byte.
#
#   usage: trace_bc90.sh [GX1 GY1 GX2 GY2 GX3 GY3 WAIT GAP1 GAP2 SETTLE ARMW DUMPW OUTPREFIX]
#   env:   FIST_TILEPHYS (default 0x141000)  DOSBOX (default /tmp/debs/dosbox-fist)
# =============================================================================
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
GX1="${1:-160}"; GY1="${2:-100}"; GX2="${3:-205}"; GY2="${4:-128}"; GX3="${5:-40}"; GY3="${6:-186}"
WAIT="${7:-60}"; GAP1="${8:-12}"; GAP2="${9:-12}"; SETTLE="${10:-30}"; ARMW="${11:-5}"; DUMPW="${12:-8}"
PREFIX="${13:-$ROOT/scratch/oracle/bc90}"
DOSBOX="${DOSBOX:-/tmp/debs/dosbox-fist}"
CYCLES="${CYCLES:-max}"
TILEPHYS="${FIST_TILEPHYS:-0x141000}"
AE="${FIST_ARMEARLY:-}"; REMAIN=$(( WAIT - ${AE:-0} )); [ "$REMAIN" -lt 1 ] && REMAIN=1

[ -x "$DOSBOX" ] || { echo "missing instrumented dosbox at $DOSBOX (see tools/oracle/README_terrain.md)"; exit 1; }
[ -x /tmp/xclick ] || cc -O2 "$ROOT/tools/oracle/xclick.c" -I/tmp/debs/sysroot/usr/include \
    -L/tmp/debs/sysroot/usr/lib/x86_64-linux-gnu -o /tmp/xclick -lX11 -lXtst
mkdir -p "$(dirname "$PREFIX")"
rm -f "$PREFIX".lastwriters.txt "$PREFIX".lastval.bin
SCRATCH="$(mktemp -d /tmp/fist_bc90.XXXXXX)"; DATA="$SCRATCH/armoredfist"
cp -a "$ROOT/armoredfist" "$DATA"
trap 'rm -rf "$SCRATCH"' EXIT
CONF="$SCRATCH/dosbox_bc90.conf"
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
cycles=$CYCLES
[autoexec]
mount c $DATA
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11
LOG="$SCRATCH/dosbox.log"
timeout $((WAIT+GAP1+GAP2+SETTLE+ARMW+DUMPW+60)) \
  xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  export FISTLOG='$PREFIX' FIST_TILEPHYS='$TILEPHYS'
  ${FIST_WATCHFLAT:+export FIST_WATCHFLAT='$FIST_WATCHFLAT' FIST_WATCHFLATSPAN='${FIST_WATCHFLATSPAN:-0x10000}'}
  ${FIST_WATCHPHYS:+export FIST_WATCHPHYS='$FIST_WATCHPHYS' FIST_WATCHSPAN='${FIST_WATCHSPAN:-0x20}'}
  '$DOSBOX' -conf '$CONF' -exit >'$LOG' 2>&1 &
  DPID=\$!
  if [ -n \"$AE\" ]; then
    sleep $AE
    echo \"[bc90] ARM-VERYEARLY (SIGUSR1) at t=$AE (during boot/intro, before menu)\"; kill -USR1 \$DPID
    sleep $REMAIN
  else
    sleep $WAIT
  fi
  ROOTPNG='$SCRATCH/root.png'
  import -window root \$ROOTPNG 2>/dev/null
  GEOM=\$(convert \$ROOTPNG -fuzz 1% -format '%@' info:)
  echo \"[bc90] menu game rect: \$GEOM\"
  OX=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\1/')
  OY=\$(echo \$GEOM | sed -E 's/.*\+([0-9]+)\+([0-9]+)/\2/')
  if [ -z \"$AE\" ] && [ -z \"$FIST_MEMARM_BOOT\" ]; then echo '[bc90] ARM (SIGUSR1) at MENU, before BATTLES'; kill -USR1 \$DPID; sleep 1; fi
  echo '[bc90] click BATTLES'; /tmp/xclick \$((OX+$GX1)) \$((OY+$GY1)); sleep $GAP1
  echo '[bc90] click OK';      /tmp/xclick \$((OX+$GX2)) \$((OY+$GY2)); sleep $GAP2
  echo '[bc90] click ACCEPT';  /tmp/xclick \$((OX+$GX3)) \$((OY+$GY3)); sleep $SETTLE
  import -window root '$PREFIX.frame.png' 2>/dev/null
  echo '[bc90] DUMP (SIGUSR2)'; kill -USR2 \$DPID; sleep $DUMPW
  kill \$DPID 2>/dev/null; wait 2>/dev/null
"
cp "$LOG" /tmp/bc90_dosbox.log 2>/dev/null || true
echo '=== dosbox stderr tail ==='; tail -5 "$LOG" 2>/dev/null
echo '=== oracle artifacts ==='; ls -la "$PREFIX".* 2>/dev/null
echo "=== bc90 per-byte last-writers (phys=$TILEPHYS) ==="
head -40 "$PREFIX.lastwriters.txt" 2>/dev/null || echo "NO lastwriters.txt (dump did not fire)"
