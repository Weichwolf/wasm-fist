#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DOSBOX="${DOSBOX:-$ROOT/third_party/dosbox-build/dosbox-0.74-3/src/dosbox}"
DURATION_SEC="${1:-50}"
OUT="${2:-$ROOT/scratch/sequence-capture/run}"
if [[ ! "$DURATION_SEC" =~ ^[0-9]{1,7}$ ]]; then echo 'invalid emulated duration' >&2; exit 2; fi
DURATION_SEC=$((10#$DURATION_SEC))
export FIST_SEQUENCE_END_MS="${FIST_SEQUENCE_END_MS:-$((DURATION_SEC*1000))}"
if [[ ! "$FIST_SEQUENCE_END_MS" =~ ^[0-9]{1,10}$ ]]; then echo 'invalid endpoint milliseconds' >&2; exit 2; fi
FIST_SEQUENCE_END_MS=$((10#$FIST_SEQUENCE_END_MS))
if [ "$FIST_SEQUENCE_END_MS" -eq 0 ] || [ "$FIST_SEQUENCE_END_MS" -gt 4294967295 ]; then
  echo 'endpoint milliseconds outside uint32 range' >&2; exit 2
fi
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
core=${FIST_DOSBOX_CORE:-normal}
cputype=auto
cycles=${FIST_DOSBOX_CYCLES:-fixed 30000}
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
if [ -z "${FIST_SEQUENCE_START_STATE:-}" ]; then
  FIST_SEQUENCE_START_STATE="$OUT/start-state"
  base64 -d "$ROOT/tools/oracle/start_state.text.gz.b64" | gzip -dc > "$FIST_SEQUENCE_START_STATE.text"
  base64 -d "$ROOT/tools/oracle/start_state.bda.gz.b64" | gzip -dc > "$FIST_SEQUENCE_START_STATE.bda"
  cp "$ROOT/tools/oracle/start_state.vga" "$FIST_SEQUENCE_START_STATE.vga"
fi
printf '%s  %s\n' \
  95497cd569be155a0efb76408a22f54fbe00dc8a267d19e7ad9375b70088bacb "$FIST_SEQUENCE_START_STATE.text" \
  5c1a8ad522ad0d6019ccb91847f7eff7fd611e9f710eed350e80afa77fe799d7 "$FIST_SEQUENCE_START_STATE.bda" \
  2b15a5e6f618bce966ae54fd0cb101009ce54041f65eb3fa2f28bcf40aed0d65 "$FIST_SEQUENCE_START_STATE.vga" | sha256sum -c -
export SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=x11 FIST_SEQUENCE="$OUT/sequence" FIST_SEQUENCE_START_STATE
timeout --signal=TERM "$(((FIST_SEQUENCE_END_MS+999)/1000+30))" \
  xvfb-run -a --server-args="-screen 0 1024x768x24" \
  "$DOSBOX" -conf "$OUT/dosbox.conf" -exit > "$OUT/dosbox.log" 2>&1
python3 "$ROOT/tools/oracle/sequence_format.py" "$OUT/sequence" --end-ms "$FIST_SEQUENCE_END_MS" | tee "$OUT/summary.txt"
sha256sum "$DOSBOX" "$OUT/sequence.frames" "$OUT/sequence.pcm" "$OUT/sequence.end" > "$OUT/sha256.txt"
printf 'capture: %s\n' "$OUT"
