#!/usr/bin/env bash
# Self-play sweep: run every mission as a deterministic AI-vs-AI simulation with EMPTY player input
# over the cooperative tick, and report whether it reaches a resolved outcome -- on either target,
# and (with `both`) whether the two targets agree.
#
# This is the harness behind the goal's "every mission plays itself through to a resolved
# victory/defeat state, native and WASM byte-identical".  It is deliberately committed rather than
# ad-hoc: the failing set has to be reproducible by anyone, on demand.
#
#   tools/selfplay.sh [native|wasm|both] [tick-cap|""] [mission-regex]
#
# The tick cap is optional and normally left empty: a mission runs until the ENGINE resolves it.  Pass
# one only to bound an investigation, and keep it under 0xffff (see below).
#
# Classification per mission, from the shim's `[outcome]` line (tools/native_main.c).  The criterion is
# the ENGINE's own end-of-mission state, not a roster count: byte[DGROUP:0xe814] is the flag that
# FUN_0000_459a's mission loop exits on, and word[DGROUP:0x6da0] is the outcome code every one of the
# six flag-writers sets just before raising it.
#
#   RESOLVED   the engine raised byte[0xe814]; `code=` says which outcome
#                0 VICTORY (opposing side destroyed, 1a6ae)   1 DEFEAT (own side lost, 1a6bb)
#                2 TIME EXPIRED (mission clock hit 00:00:00, 1a60b)
#                3 (6115)   4 ABORTED (15e4c)   5 (15e8c timeline task)
#   UNRESOLVED the mission loaded and simulated but the engine never resolved it inside the budget
#   NOLOAD     the roster never spawned
#   TIMEOUT    the wall-clock watchdog fired (a hang, or simply slower than the budget)
#   CRASH      non-zero exit that is not the watchdog
#
# A mission runs until the ENGINE resolves it (FIST_STOP_ON_OUTCOME=1), not to a tick cap.  A cap could
# not express this anyway: [DGROUP:0x452] is 16-bit, so any FIST_DUMPTICK above 0xffff never fires, and
# one 15-minute mission is ~54000 sim ticks -- close enough to the ceiling to be a trap.  The wall-clock
# budget is the only bound.  A 15-minute mission is ~2 minutes of native CPU (~54000 sim ticks at ~161
# cooperative pumps each), so the default below is generous by a wide margin rather than tight.
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARGET="${1:-native}"; TICKCAP="${2:-}"; FILTER="${3:-.}"   # TICKCAP empty = no cap; the engine decides
BIN="${NATIVE:-/tmp/fist_native}"
OUTJS="${OUTJS:-/tmp/fisttest/fistrun.js}"
NODE="$(ls "$HOME"/Git/emsdk/node/*/bin/node 2>/dev/null | head -1)"; NODE="${NODE:-node}"
BUDGET="${FIST_SELFPLAY_TIMEOUT:-900}"
# The runs are independent, so run several at once; each gets its OWN datadir because a run writes .FPL
# pilot files back and two parallel runs must not share one.  SP_JOBS=1 restores serial order.
JOBS="${SP_JOBS:-$(nproc 2>/dev/null || echo 2)}"
# The BATTLES -> OK -> ACCEPT menu navigation that drives patch 380's FIST_FSG_BATTLE into any of
# the 47 .FSG.  Identical to verify.sh's MC_MOUSE; without it the engine never leaves the menu and
# every mission reports NOLOAD.  Player input inside the mission stays EMPTY -- the last event is a
# release at t=431 vblanks after the menu entry (~7 s of game time), well before the roster spawns, so the
# run is pure AI-vs-AI from there on.
SP_MOUSE="12:160:100:0; 48:160:100:1; 84:160:100:0; 180:205:128:0; 216:205:128:1; 251:205:128:0; 323:40:186:0; 359:40:186:1; 395:40:186:0; 431:40:186:0"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT

# Runs write back into FISTDATA (.FPL pilot files); never let a sweep mutate the read-only originals,
# and never let two parallel runs share one copy.

missions=(); for f in "$ROOT"/armoredfist/FISTDATA/*.FSG "$ROOT"/armoredfist/FISTDATA/*.fsg; do
  [ -e "$f" ] || continue; b="$(basename "$f")"; b="${b%.*}"
  printf '%s' "$b" | grep -Eq "$FILTER" || continue
  missions+=("$b")
done
IFS=$'\n' missions=($(printf '%s\n' "${missions[@]}" | sort -u)); unset IFS

run_one() {  # $1=target $2=mission -> prints "STATE|outcome-code|secs"
  local tgt="$1" m="$2" st t0 t1 rc out DD
  out="$TMP/$tgt.$m.err"
  DD="$TMP/data.$tgt.$m"; cp -r "$ROOT/armoredfist" "$DD"
  t0=$SECONDS
  if [ "$tgt" = native ]; then
    setarch -R env FIST_DATADIR="$DD" FIST_SIMRUN=1 \
      FIST_FSG_BATTLE="$m" FIST_STOP_ON_OUTCOME=1 ${TICKCAP:+FIST_DUMPTICK=$TICKCAP} FIST_MOUSE="$SP_MOUSE" \
      timeout "$BUDGET" "$BIN" >/dev/null 2>"$out"; rc=$?
  else
    env FIST_DATADIR="$DD" FIST_SIMRUN=1 \
      FIST_FSG_BATTLE="$m" FIST_STOP_ON_OUTCOME=1 ${TICKCAP:+FIST_DUMPTICK=$TICKCAP} FIST_MOUSE="$SP_MOUSE" \
      timeout "$BUDGET" "$NODE" "$OUTJS" >/dev/null 2>"$out"; rc=$?
  fi
  t1=$((SECONDS - t0)); rm -rf "$DD"
  local line code
  line="$(grep -m1 '^\[outcome\]' "$out" 2>/dev/null || true)"
  code="$(printf '%s' "$line" | sed -n 's/.*code=\(-\?[0-9]*\).*/\1/p')"; code="${code:--}"
  if   [ "$rc" = 124 ];                                    then st=TIMEOUT
  elif [ -z "$line" ] && [ "$rc" != 0 ];                   then st=CRASH
  elif printf '%s' "$line" | grep -q 'over=1';             then st=RESOLVED
  elif printf '%s' "$line" | grep -q 'never loaded';       then st=NOLOAD
  elif [ -n "$line" ];                                     then st=UNRESOLVED
  else                                                          st=CRASH
  fi
  printf '%s|%s|%s' "$st" "$code" "$t1"
}

printf '=== self-play sweep: target=%s missions=%d budget=%ss jobs=%s ===\n' \
       "$TARGET" "${#missions[@]}" "$BUDGET" "$JOBS"

# One job per mission, at most $JOBS at a time; results land in files and are reported in mission order
# so the output is identical whatever the scheduling.
work_one() {
  local m="$1"
  if [ "$TARGET" = both ]; then
    { run_one native "$m"; printf '\n'; run_one wasm "$m"; printf '\n'; } > "$TMP/res.$m"
  else
    { run_one "$TARGET" "$m"; printf '\n'; } > "$TMP/res.$m"
  fi
}
for m in "${missions[@]}"; do
  while [ "$(jobs -rp | wc -l)" -ge "$JOBS" ]; do wait -n; done
  work_one "$m" &
done
wait

pass=0; fail=0; declare -a failed=()
for m in "${missions[@]}"; do
  if [ "$TARGET" = both ]; then
    IFS='|' read -r ns nm nt < <(sed -n 1p "$TMP/res.$m")
    IFS='|' read -r ws wm wt < <(sed -n 2p "$TMP/res.$m")
    if [ "$ns" = RESOLVED ] && [ "$ws" = RESOLVED ] && [ "$nm" = "$wm" ]; then
      printf '  %-10s RESOLVED   both (code=%s)  %ss/%ss\n' "$m" "$nm" "$nt" "$wt"; pass=$((pass+1))
    else
      printf '  %-10s native=%-10s(code=%s) wasm=%-10s(code=%s)  %ss/%ss\n' "$m" "$ns" "$nm" "$ws" "$wm" "$nt" "$wt"
      fail=$((fail+1)); failed+=("$m")
    fi
  else
    IFS='|' read -r s mn tt < <(sed -n 1p "$TMP/res.$m")
    if [ "$s" = RESOLVED ]; then
      printf '  %-10s RESOLVED   (code=%s)  %ss\n' "$m" "$mn" "$tt"; pass=$((pass+1))
    else
      printf '  %-10s %-10s (code=%s)  %ss\n' "$m" "$s" "$mn" "$tt"; fail=$((fail+1)); failed+=("$m")
    fi
  fi
done
printf '== self-play: %d resolved, %d not (%d missions) ==\n' "$pass" "$fail" "${#missions[@]}"
[ "$fail" -gt 0 ] && printf 'not resolved: %s\n' "${failed[*]}"
[ "$fail" -eq 0 ]
