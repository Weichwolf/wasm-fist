#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DOSBOX="${DOSBOX:-$ROOT/third_party/dosbox-build/dosbox-0.74-3/src/dosbox}"
DURATION_SEC="${1:-50}"
OUT="${2:-$ROOT/scratch/sequence-capture/run}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
if [ -e "$OUT/game" ]; then
  echo "capture directory already used: $OUT" >&2
  exit 2
fi
cp -a "$ROOT/armoredfist" "$OUT/game"
cat > "$OUT/dosbox.conf" <<CFG
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
core=auto
cputype=auto
cycles=max
[sblaster]
sbtype=sb16
sbbase=220
irq=7
dma=1
[autoexec]
mount c $OUT/game
c:
LOADGAME -K400,0,1000 -X5000 FIST.RUN
CFG
cc -O2 "$ROOT/tools/oracle/xkey.c" -o "$OUT/xkey" -lX11 -l:libXtst.so.6
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11 FIST_SEQUENCE="$OUT/sequence"
timeout --signal=TERM "$((DURATION_SEC+30))" xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c '
  "$1" -conf "$2" -exit > "$3" 2>&1 &
  process=$!
  sleep "$4"
  "$5" F9 ctrl
  wait "$process"
' bash "$DOSBOX" "$OUT/dosbox.conf" "$OUT/dosbox.log" "$DURATION_SEC" "$OUT/xkey"
python3 "$ROOT/tools/oracle/sequence_format.py" "$OUT/sequence" | tee "$OUT/summary.txt"
sha256sum "$DOSBOX" "$OUT/sequence.frames" "$OUT/sequence.pcm" > "$OUT/sha256.txt"
printf 'capture: %s\n' "$OUT"
