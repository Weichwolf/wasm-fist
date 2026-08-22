#!/usr/bin/env bash
# =============================================================================
# wasm_gate_driver.sh -- grind the DoD gate: 10 CONSECUTIVE clean WASM full-matrix
# runs (tools/verify.sh wasm).  A single failed run resets the consecutive counter
# to zero.  Long-lived background process; loop wakeups just tail $GATELOG.
# =============================================================================
set -u
ROOT="/home/cosmo/Git/wasm-fist"
NODE="$(ls "$HOME"/Git/emsdk/node/*/bin/node 2>/dev/null | head -1)"; [ -z "$NODE" ] && NODE=node
OUTJS="/tmp/fisttest/fistrun.js"
GATELOG="/tmp/wasm_gate.log"
NEED=10
consec=0
run=0
echo "[gate] START need=$NEED consecutive clean wasm runs  node=$NODE  $(date)" > "$GATELOG"
while [ "$consec" -lt "$NEED" ]; do
  run=$((run+1))
  rl="/tmp/wasm_gate_run$run.log"
  echo "[gate] run #$run START (consecutive so far=$consec)  $(date)" >> "$GATELOG"
  bash "$ROOT/tools/verify.sh" wasm "$NODE $OUTJS" > "$rl" 2>&1
  summary="$(grep -E '== verify:' "$rl" | tail -1)"
  passed="$(grep -c '  PASS ' "$rl")"
  failed="$(grep -c '  FAIL ' "$rl")"
  if echo "$summary" | grep -qE '0 failed' && [ "$failed" -eq 0 ] && [ "$passed" -ge 175 ]; then
    consec=$((consec+1))
    echo "[gate] run #$run CLEAN ($passed pass / $failed fail) -> consecutive=$consec/$NEED  $(date)" >> "$GATELOG"
  else
    consec=0
    echo "[gate] run #$run FAILED ($passed pass / $failed fail) [$summary] -> consecutive RESET to 0  $(date)" >> "$GATELOG"
    echo "[gate]   first failures:" >> "$GATELOG"
    grep -E '  FAIL ' "$rl" | head -8 >> "$GATELOG"
  fi
done
echo "[gate] DONE -- $NEED consecutive clean wasm runs achieved.  $(date)" >> "$GATELOG"
