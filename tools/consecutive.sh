#!/usr/bin/env bash
# consecutive.sh -- the 10x-consecutive-clean DoD gate (DD2 passrun/consecutive analog).
#
# The Definition-of-Done requires TARGET consecutive clean full-suite passes, a single
# failure resetting the streak to zero. This harness builds the current commit ONCE (the
# binary is fixed; the gate tests RUN-stability -- it catches per-boot flakes like the
# f842 boot SIGSEGV that patch 383 fixed, since each verify.sh run boots every flow), then
# runs `verify.sh both` up to MAXITER times, tracking the consecutive-clean streak.
#
# Usage:  bash tools/consecutive.sh [TARGET=10] [MAXITER=30]
# Env:    CONSEC_LOG=<path>   NO_BUILD=1 (skip the build, use existing /tmp binaries)
# Exit:   0 = gate reached (streak==TARGET) ; 1 = not reached within MAXITER ; 2 = build failed
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARGET="${1:-10}"
MAXITER="${2:-30}"
SHA="$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo nogit)"
LOG="${CONSEC_LOG:-/tmp/consecutive_${SHA}.log}"
: > "$LOG"

say(){ echo "$*" | tee -a "$LOG"; }

say "== consecutive-clean gate :: target=$TARGET maxiter=$MAXITER commit=$SHA :: $(date -u '+%Y-%m-%dT%H:%MZ') =="

if [ "${NO_BUILD:-0}" != 1 ]; then
  say "[build] make kernel-image (if needed) + native + wasm (once; the gate tests run-stability) ..."
  if [ ! -s "$ROOT/re_out/fist_image.bin" ]; then ( cd "$ROOT" && make kernel-image ) >>"$LOG" 2>&1; fi
  if ! ( cd "$ROOT" && make native && make wasm ) >>"$LOG" 2>&1; then
    say "BUILD FAILED -- cannot run the gate (see $LOG). NB verify.sh is RUN-ONLY; it needs pre-built binaries."
    exit 2
  fi
  say "[build] OK"
fi

streak=0; best=0
for i in $(seq 1 "$MAXITER"); do
  res="$(cd "$ROOT" && bash tools/verify.sh both 2>&1)"
  line="$(printf '%s\n' "$res" | grep -E '^== verify: [0-9]+ passed, [0-9]+ failed ==' | tail -1)"
  fails="$(printf '%s' "$line" | sed -nE 's/.*, ([0-9]+) failed ==/\1/p')"
  if [ -z "$line" ]; then
    say "[iter $i] NO RESULT LINE (verify.sh crashed/hung?) -> streak RESET"
    printf '%s\n' "$res" | tail -5 | tee -a "$LOG"
    streak=0
  elif [ "$fails" = 0 ]; then
    streak=$((streak+1)); [ "$streak" -gt "$best" ] && best=$streak
    say "[iter $i] CLEAN :: $line :: streak $streak/$TARGET"
  else
    say "[iter $i] FAIL :: $line :: streak RESET to 0 (was $streak)"
    printf '%s\n' "$res" | grep -E '^  FAIL' | tee -a "$LOG"
    streak=0
  fi
  if [ "$streak" -ge "$TARGET" ]; then
    say "== GATE PASSED :: $TARGET consecutive clean full-suite runs (commit $SHA) =="
    exit 0
  fi
done
say "== GATE NOT REACHED :: best streak $best/$TARGET after $MAXITER iters (commit $SHA) =="
exit 1
