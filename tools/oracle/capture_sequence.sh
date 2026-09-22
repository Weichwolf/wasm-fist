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
core=${FIST_DOSBOX_CORE:-auto}
cputype=auto
cycles=${FIST_DOSBOX_CYCLES:-max}
[mixer]
nosound=true
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
if [ -z "${FIST_SEQUENCE_START_STATE:-}" ]; then
  FIST_SEQUENCE_START_STATE="$OUT/start-state"
  base64 -d "$ROOT/tools/oracle/start_state.text.gz.b64" | gzip -dc > "$FIST_SEQUENCE_START_STATE.text"
  base64 -d "$ROOT/tools/oracle/start_state.bda.gz.b64" | gzip -dc > "$FIST_SEQUENCE_START_STATE.bda"
fi
printf '%s  %s\n' \
  95497cd569be155a0efb76408a22f54fbe00dc8a267d19e7ad9375b70088bacb "$FIST_SEQUENCE_START_STATE.text" \
  5c1a8ad522ad0d6019ccb91847f7eff7fd611e9f710eed350e80afa77fe799d7 "$FIST_SEQUENCE_START_STATE.bda" | sha256sum -c -
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11 FIST_SEQUENCE="$OUT/sequence" FIST_SEQUENCE_START_STATE
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
