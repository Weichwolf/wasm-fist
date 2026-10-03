#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
source "$ROOT/tools/work_dir.sh"
mkdir -p "$FIST_WORKDIR/verify"
RUN="$(mktemp -d "$FIST_WORKDIR/verify/run.XXXXXX")"
export FIST_FLOWS="${1:-}"
export NATIVE="${NATIVE:-$FIST_WORKDIR/native}"
export OUTJS="${OUTJS:-$FIST_WORKDIR/wasm/fistrun.js}"
printf 'evidence: %s\n' "$RUN"
{ git rev-parse HEAD; printf 'flows: %s\n' "${FIST_FLOWS:-all}"; git status --short; } > "$RUN/revision.txt"
git diff --binary HEAD > "$RUN/worktree.patch"
python3 -B -m unittest discover -s tests -v > "$RUN/tests.log" 2>&1
make check > "$RUN/patch.log" 2>&1
make native NATIVE="$NATIVE" > "$RUN/native-build.log" 2>&1
make wasm OUTJS="$OUTJS" > "$RUN/wasm-build.log" 2>&1
sha256sum "$NATIVE" "$OUTJS" "${OUTJS%.js}.wasm" tools/verify.sh tools/compare_output.py > "$RUN/sha256.txt"
python3 -B tools/check_resource_start.py --output "$RUN/resource-start" > "$RUN/resource-start.log" 2>&1
# board:0033: preserve the compared buffers and the producer's exit status, including failed runs.
FIST_VERIFY_OUT="$RUN" bash tools/verify.sh both 2>&1 | tee "$RUN/verify.log"
