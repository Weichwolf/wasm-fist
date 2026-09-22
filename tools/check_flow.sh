#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
mkdir -p scratch/verify
RUN="$(mktemp -d "$ROOT/scratch/verify/run.XXXXXX")"
export FIST_FLOWS="${1:-}"
export NATIVE="${NATIVE:-/tmp/fist_native}"
export OUTJS="${OUTJS:-/tmp/fisttest/fistrun.js}"
printf 'evidence: %s\n' "$RUN"
{ git rev-parse HEAD; printf 'flows: %s\n' "${FIST_FLOWS:-all}"; git status --short; } > "$RUN/revision.txt"
git diff --binary HEAD > "$RUN/worktree.patch"
python3 -B -m unittest discover -s tests -v > "$RUN/tests.log" 2>&1
make check > "$RUN/patch.log" 2>&1
make native NATIVE="$NATIVE" > "$RUN/native-build.log" 2>&1
make wasm OUTJS="$OUTJS" > "$RUN/wasm-build.log" 2>&1
sha256sum "$NATIVE" "$OUTJS" "${OUTJS%.js}.wasm" tools/verify.sh tools/compare_output.py > "$RUN/sha256.txt"
# board:0033: preserve the compared buffers and the producer's exit status, including failed runs.
FIST_VERIFY_OUT="$RUN" bash tools/verify.sh both 2>&1 | tee "$RUN/verify.log"
