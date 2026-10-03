#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARGET="${1:?target native or wasm}"
TICK="${2:-120}"
source "$ROOT/tools/work_dir.sh"
OUT="${3:-$FIST_WORKDIR/sequence-capture/port-$TARGET-$TICK}"
case "$TARGET" in native|wasm) ;; *) echo "invalid target: $TARGET" >&2; exit 2;; esac
dumpenv=(FIST_DUMPTICK="$TICK")
if [ -n "${FIST_SEQUENCE_END_MS:-}" ]; then dumpenv=(); fi
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
if [ -e "$OUT/game" ]; then echo "capture directory already used: $OUT" >&2; exit 2; fi
cp -a "$ROOT/armoredfist" "$OUT/game"
cd "$ROOT"
if [ "$TARGET" = native ]; then
  timeout 90 env -u FIST_DUMPTICK FIST_DATADIR="$OUT/game" "${dumpenv[@]}" \
    FIST_SEQUENCE="$OUT/sequence" FIST_AUDIO_WAV="$OUT/audio.wav" \
    "${NATIVE:-/tmp/fist_native}" > "$OUT/port.log" 2>&1
else
  NODE="${NODE:-$(ls "$HOME"/Git/emsdk/node/*/bin/node 2>/dev/null | head -1)}"
  NODE="${NODE:-node}"
  timeout 240 env -u FIST_DUMPTICK FIST_DATADIR="$OUT/game" "${dumpenv[@]}" \
    FIST_SEQUENCE="$OUT/sequence" FIST_AUDIO_WAV="$OUT/audio.wav" \
    "$NODE" "${OUTJS:-/tmp/fisttest/fistrun.js}" > "$OUT/port.log" 2>&1
fi
validation=(--frames-only)
if [ -n "${FIST_SEQUENCE_END_MS:-}" ]; then validation+=(--end-ms "$FIST_SEQUENCE_END_MS"); fi
python3 "$ROOT/tools/oracle/sequence_format.py" "$OUT/sequence" "${validation[@]}" | tee "$OUT/summary.txt"
sha256sum "$OUT/sequence.frames" > "$OUT/sha256.txt"
if [ -n "${FIST_SEQUENCE_END_MS:-}" ]; then sha256sum "$OUT/sequence.end" >> "$OUT/sha256.txt"; fi
printf 'capture: %s\n' "$OUT"
