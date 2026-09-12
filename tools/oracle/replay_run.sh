#!/usr/bin/env bash
# board:0017 -- replay_run.sh <port binary> <battle> <prefix> [<oracle regs.txt>]: the port plays <battle>
# seeded with the oracle's LFSR state and driven by its three schedules (replay_sched.py's <prefix>_*),
# writing FIST_RNGTRACE to <prefix>_port.txt and the DGROUP window at every sim step to
# <prefix>_step.<n>; with the oracle's regs.txt it then runs replay_cmp.py.  The mouse script is the
# self-play's (BATTLES, OK, ACCEPT).  REPLAY_DUMPEVERY=k matches a capture's OC_DUMPEVERY=k;
# REPLAY_STEPS bounds the run (default: the schedule's length); REPLAY_TIMEOUT the wall clock.
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="${1:?}"; BATTLE="${2:?}"; PFX="${3:?}"; REGS="${4:-}"
WORK="$(mktemp -d /tmp/replay.XXXXXX)"; cp -r "$ROOT/armoredfist" "$WORK/armoredfist"
SP_MOUSE="12:160:100:0; 48:160:100:1; 84:160:100:0; 180:205:128:0; 216:205:128:1; 251:205:128:0; 323:40:186:0; 359:40:186:1; 395:40:186:0; 431:40:186:0"
setarch -R env FIST_DATADIR="$WORK/armoredfist" FIST_SIMRUN=1 FIST_FSG_BATTLE="$BATTLE" FIST_MOUSE="$SP_MOUSE" FIST_STOP_ON_OUTCOME=1 \
  FIST_RNGTRACE="${PFX}_port.txt" FIST_RNG_SEED="$(cat "${PFX}_seed.txt")" FIST_RNG_SCHEDULE="${PFX}_sched.txt" \
  FIST_FRAME_SCHEDULE="${PFX}_frames.txt" FIST_PHASE_SCHEDULE="${PFX}_phases.txt" \
  FIST_STEP0_DUMP="${PFX}_step" FIST_STEPEVERY_DUMP="${REPLAY_DUMPEVERY:-1}" FIST_STEP_DUMPLEN=10000 \
  FIST_STOP_AT_STEP="${REPLAY_STEPS:-$(wc -l < "${PFX}_sched.txt")}" \
  timeout "${REPLAY_TIMEOUT:-900}" "$BIN" > "${PFX}_run.log" 2>&1 || echo "[replay] port exit $?"
rm -rf "$WORK"
[ -n "$REGS" ] && python3 "$ROOT/tools/oracle/replay_cmp.py" "$REGS" "${PFX}_port.txt" "${PFX}_sched.txt" "$BIN"
