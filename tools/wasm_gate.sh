#!/usr/bin/env bash
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NODE="${NODE:-$(ls "$HOME"/Git/emsdk/node/*/bin/node 2>/dev/null | head -1)}"
NODE="${NODE:-node}"
OUTJS="${OUTJS:-/tmp/fisttest/fistrun.js}"
NEED="${1:-10}"
REV="$(git -C "$ROOT" rev-parse HEAD)"
if [ -n "${GATE_OUT:-}" ]; then
  EVIDENCE="$GATE_OUT"
  mkdir -p "$EVIDENCE"
else
  mkdir -p "$ROOT/scratch/verify"
  EVIDENCE="$(mktemp -d "$ROOT/scratch/verify/wasm-gate.${REV:0:12}.XXXXXX")"
fi
MANIFEST="$EVIDENCE/build.sha256"

case "$NEED" in ''|*[!0-9]*|0) echo "invalid streak: $NEED" >&2; exit 2;; esac
[ -s "$OUTJS" ] && [ -s "${OUTJS%.js}.wasm" ] || {
  echo "missing WASM build; run 'make wasm'" >&2
  exit 2
}
[ -z "$(git -C "$ROOT" status --porcelain --untracked-files=no)" ] || {
  echo "tracked worktree is dirty; gate requires one fixed revision" >&2
  exit 2
}

sha256sum "$OUTJS" "${OUTJS%.js}.wasm" "$ROOT/tools/verify.sh" \
  "$ROOT/tools/compare_output.py" > "$MANIFEST"
printf '%s\n' "$REV" > "$EVIDENCE/revision.txt"

streak=0
run=0
while [ "$streak" -lt "$NEED" ]; do
  run=$((run+1))
  dir="$EVIDENCE/run.$(printf '%03d' "$run")"
  mkdir -p "$dir"
  if [ "$(git -C "$ROOT" rev-parse HEAD)" != "$REV" ] ||
     [ -n "$(git -C "$ROOT" status --porcelain --untracked-files=no)" ] ||
     ! sha256sum -c "$MANIFEST" >/dev/null; then
    echo "gate inputs changed during run $run" | tee "$dir/result.txt"
    exit 2
  fi

  if FIST_VERIFY_OUT="$dir" NODE="$NODE" OUTJS="$OUTJS" \
      bash "$ROOT/tools/verify.sh" wasm > "$dir/verify.log" 2>&1; then
    rc=0
  else
    rc=$?
  fi
  summary="$(grep -E '^== verify: [0-9]+ passed, [0-9]+ failed ==$' "$dir/verify.log" | tail -1)"
  passed="$(printf '%s\n' "$summary" | sed -nE 's/^== verify: ([0-9]+) passed, 0 failed ==$/\1/p')"
  observed="$(grep -c '^  PASS ' "$dir/verify.log" || true)"

  if [ "$rc" -eq 0 ] && [ -n "$passed" ] && [ "$passed" -gt 0 ] && [ "$observed" -eq "$passed" ]; then
    streak=$((streak+1))
    printf 'CLEAN run=%d pass=%d streak=%d/%d\n' "$run" "$passed" "$streak" "$NEED" | tee "$dir/result.txt"
  else
    printf 'FAIL run=%d rc=%d summary=%s observed=%d streak=0/%d\n' \
      "$run" "$rc" "${summary:-missing}" "$observed" "$NEED" | tee "$dir/result.txt"
    streak=0
  fi
done

printf 'PASSED revision=%s runs=%d streak=%d evidence=%s\n' "$REV" "$run" "$streak" "$EVIDENCE"
