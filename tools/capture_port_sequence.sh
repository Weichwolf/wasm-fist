#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARGET="${1:?target native or wasm}"
TICK="${2:-120}"
OUT="${3:-$ROOT/scratch/sequence-capture/port-$TARGET-$TICK}"
case "$TARGET" in native|wasm) ;; *) echo "invalid target: $TARGET" >&2; exit 2;; esac
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
if [ -e "$OUT/game" ]; then echo "capture directory already used: $OUT" >&2; exit 2; fi
cp -a "$ROOT/armoredfist" "$OUT/game"
cd "$ROOT"
if [ "$TARGET" = native ]; then
  timeout 90 env FIST_DATADIR="$OUT/game" FIST_DUMPTICK="$TICK" FIST_SEQUENCE="$OUT/sequence" \
    "${NATIVE:-/tmp/fist_native}" > "$OUT/port.log" 2>&1
else
  NODE="${NODE:-$(ls "$HOME"/Git/emsdk/node/*/bin/node 2>/dev/null | head -1)}"
  NODE="${NODE:-node}"
  timeout 240 env FIST_DATADIR="$OUT/game" FIST_DUMPTICK="$TICK" FIST_SEQUENCE="$OUT/sequence" \
    "$NODE" "${OUTJS:-/tmp/fisttest/fistrun.js}" > "$OUT/port.log" 2>&1
fi
python3 - "$ROOT/tools/oracle/sequence_format.py" "$OUT/sequence.frames" <<'PY' | tee "$OUT/summary.txt"
import importlib.util, sys
spec = importlib.util.spec_from_file_location('sequence_format', sys.argv[1])
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
print(module.validate(sys.argv[2], 'F'))
PY
sha256sum "$OUT/sequence.frames" > "$OUT/sha256.txt"
printf 'capture: %s\n' "$OUT"
