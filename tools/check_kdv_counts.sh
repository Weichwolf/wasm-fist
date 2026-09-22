#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PROFILE="${1:?Oracle KDV profile TSV required}"
PROFILE="$(realpath "$PROFILE")"
OUT="${2:-$ROOT/scratch/sequence-capture/kdv-count-check}"
NATIVE="${NATIVE:-/tmp/fist_native}"
OUTJS="${OUTJS:-/tmp/fisttest/fistrun.js}"
NODE="${NODE:-$(ls "$HOME"/Git/emsdk/node/*/bin/node 2>/dev/null | head -1 || true)}"
NODE="${NODE:-node}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
if [ -e "$OUT/native-game" ] || [ -e "$OUT/wasm-game" ]; then
    echo "capture directory already used: $OUT" >&2
    exit 2
fi
cp -a "$ROOT/armoredfist" "$OUT/native-game"
cp -a "$ROOT/armoredfist" "$OUT/wasm-game"
cd "$ROOT"
timeout 90 env FIST_DATADIR="$OUT/native-game" FIST_KDV_DUMPFRAME=395 \
    FIST_KDV_INSTRLOG="$OUT/native-counts.txt" "$NATIVE" > "$OUT/native.log" 2>&1
timeout 240 env FIST_DATADIR="$OUT/wasm-game" FIST_KDV_DUMPFRAME=395 \
    FIST_KDV_INSTRLOG="$OUT/wasm-counts.txt" "$NODE" "$OUTJS" > "$OUT/wasm.log" 2>&1
python3 tools/oracle/check_kdv_instruction_formula.py "$PROFILE" --port-counts "$OUT/native-counts.txt"
python3 tools/oracle/check_kdv_instruction_formula.py "$PROFILE" --port-counts "$OUT/wasm-counts.txt"
cmp "$OUT/native-counts.txt" "$OUT/wasm-counts.txt"
sha256sum "$NATIVE" "$OUTJS" "${OUTJS%.js}.wasm" "$OUT/native-counts.txt" "$OUT/wasm-counts.txt" > "$OUT/sha256.txt"
printf 'evidence: %s\n' "$OUT"
