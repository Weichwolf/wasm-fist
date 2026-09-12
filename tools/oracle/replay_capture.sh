#!/usr/bin/env bash
# board:0017 -- the oracle's side of the replay instrument (patch 592): the original plays OC_BATTLE for
# OC_WALL seconds under the patched DOSBox with FIST_REGTRACE on the six seams -- 0x11447/0x11454
# (0291's returns, EAX = the draw), 0x24288 (the INT-8 ISR), 0x1d275 (c0e5, the sim step), 0x24ab0
# (the 3920 poll trampoline), 0x158e4 (4754, the battle load = the sync point), 0x1346d (22dd, the
# render), 0x13487 (the 22f7 phase dispatch), 0x1d7e9 (c659, a shell drawn: ESI = the shell) -- and
# dumps the DGROUP window (the oracle's DGROUP is at
# guest phys 0x2d190) at every sim step.  Output: $OC_OUT/regs.txt, $OC_OUT/dg.bin.<step>; feed them to
# replay_sched.py / replay_objdiff.py.  Usage: OC_BATTLE=AZER1 OC_OUT=scratch/oracle/regtrace_X [OC_WALL=45]
# [OC_DUMPEVERY=1]  (a window per sim step is 64 KB: a full mission at every 10th step is ~150 MB)
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
: "${OC_BATTLE:?}" "${OC_OUT:?}"
mkdir -p "$OC_OUT"
exec env OC_WALL="${OC_WALL:-45}" \
  FIST_REGTRACE="$OC_OUT/regs.txt" \
  FIST_REGTRACE_LIN=11447,11454,24288,1d275,24ab0,158e4,1346d,13487,1d7e9 \
  FIST_REGTRACE_DUMPLIN=1d275 FIST_REGTRACE_DUMPEVERY="${OC_DUMPEVERY:-1}" \
  FIST_REGTRACE_DUMPBASE=2d190 FIST_REGTRACE_DUMPLEN=10000 FIST_REGTRACE_DUMPFILE="$OC_OUT/dg.bin" \
  bash "$ROOT/tools/oracle/blktrace_mission.sh"
