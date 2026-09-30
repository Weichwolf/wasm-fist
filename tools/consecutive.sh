#!/usr/bin/env bash
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WHICH="${GATE_WHICH:-both}"
TARGET="${1:-10}"
MAXITER="${2:-30}"
NATIVE="${NATIVE:-/tmp/fist_native}"
OUTJS="${OUTJS:-/tmp/fisttest/fistrun.js}"
NODE="${NODE:-$(ls "$HOME"/Git/emsdk/node/*/bin/node 2>/dev/null | head -1)}"
NODE="${NODE:-node}"

case "$WHICH" in native|wasm|both) ;; *) echo "invalid target: $WHICH" >&2; exit 2;; esac
for value in "$TARGET" "$MAXITER"; do
  case "$value" in ''|*[!0-9]*|0) echo "invalid positive integer: $value" >&2; exit 2;; esac
done
[ -z "$(git -C "$ROOT" status --porcelain --untracked-files=no)" ] || {
  echo "tracked worktree is dirty; gate requires one fixed revision" >&2
  exit 2
}

if [ "${NO_BUILD:-0}" != 1 ]; then
  [ -s "$ROOT/re_out/fist_image.bin" ] || make -C "$ROOT" kernel-image || exit 2
  case "$WHICH" in
    native) make -C "$ROOT" native NATIVE="$NATIVE" || exit 2;;
    wasm) make -C "$ROOT" wasm OUTJS="$OUTJS" || exit 2;;
    both) make -C "$ROOT" native NATIVE="$NATIVE" wasm OUTJS="$OUTJS" || exit 2;;
  esac
fi

inputs=("$ROOT/tools/verify.sh" "$ROOT/tools/compare_output.py")
[ "$WHICH" = wasm ] || inputs+=("$NATIVE")
[ "$WHICH" = native ] || inputs+=("$OUTJS" "${OUTJS%.js}.wasm")
for input in "${inputs[@]}"; do
  [ -s "$input" ] || { echo "missing gate input: $input" >&2; exit 2; }
done

REV="$(git -C "$ROOT" rev-parse HEAD)"
if [ -n "${GATE_OUT:-}" ]; then
  EVIDENCE="$GATE_OUT"
  mkdir -p "$EVIDENCE"
else
  mkdir -p "$ROOT/scratch/verify"
  EVIDENCE="$(mktemp -d "$ROOT/scratch/verify/gate.${WHICH}.${REV:0:12}.XXXXXX")"
fi
MANIFEST="$EVIDENCE/build.sha256"
sha256sum "${inputs[@]}" > "$MANIFEST"
printf '%s\n' "$REV" > "$EVIDENCE/revision.txt"
printf 'target=%s streak=%s max=%s\n' "$WHICH" "$TARGET" "$MAXITER" >> "$EVIDENCE/revision.txt"

streak=0
best=0
for run in $(seq 1 "$MAXITER"); do
  dir="$EVIDENCE/run.$(printf '%03d' "$run")"
  mkdir -p "$dir"
  if [ "$(git -C "$ROOT" rev-parse HEAD)" != "$REV" ] ||
     [ -n "$(git -C "$ROOT" status --porcelain --untracked-files=no)" ] ||
     ! sha256sum -c "$MANIFEST" >/dev/null; then
    echo "gate inputs changed during run $run" | tee "$dir/result.txt"
    exit 2
  fi

  if FIST_VERIFY_OUT="$dir" NATIVE="$NATIVE" OUTJS="$OUTJS" NODE="$NODE" \
      bash "$ROOT/tools/verify.sh" "$WHICH" > "$dir/verify.log" 2>&1; then
    rc=0
  else
    rc=$?
  fi
  summary="$(grep -E '^== verify: [0-9]+ passed, [0-9]+ failed ==$' "$dir/verify.log" | tail -1)"
  passed="$(printf '%s\n' "$summary" | sed -nE 's/^== verify: ([0-9]+) passed, 0 failed ==$/\1/p')"
  observed="$(grep -c '^  PASS ' "$dir/verify.log" || true)"

  if [ "$rc" -eq 0 ] && [ -n "$passed" ] && [ "$passed" -gt 0 ] && [ "$observed" -eq "$passed" ]; then
    streak=$((streak+1))
    [ "$streak" -gt "$best" ] && best="$streak"
    printf 'CLEAN run=%d pass=%d streak=%d/%d\n' "$run" "$passed" "$streak" "$TARGET" | tee "$dir/result.txt"
  else
    printf 'FAIL run=%d rc=%d summary=%s observed=%d streak=0/%d\n' \
      "$run" "$rc" "${summary:-missing}" "$observed" "$TARGET" | tee "$dir/result.txt"
    streak=0
  fi
  if [ "$streak" -ge "$TARGET" ]; then
    printf 'PASSED revision=%s runs=%d streak=%d evidence=%s\n' "$REV" "$run" "$streak" "$EVIDENCE"
    exit 0
  fi
done

printf 'NOT_REACHED revision=%s best=%d/%d evidence=%s\n' "$REV" "$best" "$TARGET" "$EVIDENCE"
exit 1
