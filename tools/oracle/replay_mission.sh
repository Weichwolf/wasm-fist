#!/usr/bin/env bash
# board:0017 -- replay_mission.sh <BATTLE> [<port binary>] [OC_WALL]: capture the original playing BATTLE
# (replay_capture.sh, a fresh seed every time), extract its schedules, replay them on the port and report
# the first step whose draws, shell draws, LOS calls or rotations differ.  Output under
# scratch/oracle/replay/<BATTLE>.<stamp>/ (regs.txt, the dumps, the port trace, report.txt).
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
B="${1:?battle}"; BIN="${2:-/tmp/fist_native}"; WALL="${3:-300}"
STAMP="$(date +%Y%m%d-%H%M%S)"; OUT="$ROOT/scratch/oracle/replay/$B.$STAMP"; mkdir -p "$OUT"
OC_BATTLE="$B" OC_OUT="$OUT" OC_WALL="$WALL" OC_DUMPEVERY=10 OC_EXTRA_LIN=115e9,11657 bash "$ROOT/tools/oracle/replay_capture.sh" > "$OUT/capture.log" 2>&1
python3 "$ROOT/tools/oracle/replay_sched.py" "$OUT/regs.txt" "$OUT/r" > "$OUT/report.txt"
REPLAY_DUMPEVERY=10 REPLAY_TIMEOUT=2400 bash "$ROOT/tools/oracle/replay_run.sh" "$BIN" "$B" "$OUT/r" "$OUT/regs.txt" >> "$OUT/report.txt" 2>&1 || true
python3 "$ROOT/tools/oracle/replay_objdiff.py" "$OUT" "$OUT/r_step" 3 >> "$OUT/report.txt" 2>&1 || true
python3 "$ROOT/tools/oracle/replay_drawcmp.py" "$OUT/regs.txt" "$OUT/r_port.txt" >> "$OUT/report.txt" 2>&1 || true
python3 "$ROOT/tools/oracle/replay_rotcmp.py" "$OUT/regs.txt" "$OUT/r_port.txt" "$BIN" >> "$OUT/report.txt" 2>&1 || true
grep -a 'outcome' "$OUT/r_run.log" >> "$OUT/report.txt" || true
rm -f "$OUT"/r_step.*
cat "$OUT/report.txt"
