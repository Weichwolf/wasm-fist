#!/usr/bin/env bash
# Self-play sweep: run every mission as a deterministic AI-vs-AI simulation with EMPTY player input
# over the cooperative tick, and report whether it reaches a resolved outcome -- on either target,
# and (with `both`) whether the two targets agree.
#
# This is the harness behind the goal's "every mission plays itself through to a resolved
# victory/defeat state, native and WASM byte-identical".  It is deliberately committed rather than
# ad-hoc: the failing set has to be reproducible by anyone, on demand.
#
#   tools/selfplay.sh [native|wasm|both] [tick-cap] [mission-regex]
#
# Classification per mission, from the shim's `[outcome]` line (tools/native_main.c):
#   RESOLVED   a296 reached 0 after the roster loaded -- one side eliminated
#   UNRESOLVED the mission loaded and simulated but no side was eliminated inside the tick cap
#   NOLOAD     a296 never reached >=15, i.e. the roster never spawned
#   TIMEOUT    the wall-clock watchdog fired (a hang, or simply slower than the budget)
#   CRASH      non-zero exit that is not the watchdog
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARGET="${1:-native}"; TICKCAP="${2:-20000}"; FILTER="${3:-.}"
BIN="${NATIVE:-/tmp/fist_native}"
OUTJS="${OUTJS:-/tmp/fisttest/fistrun.js}"
NODE="$(ls "$HOME"/Git/emsdk/node/*/bin/node 2>/dev/null | head -1)"; NODE="${NODE:-node}"
BUDGET="${FIST_SELFPLAY_TIMEOUT:-240}"
# The BATTLES -> OK -> ACCEPT menu navigation that drives patch 380's FIST_FSG_BATTLE into any of
# the 47 .FSG.  Identical to verify.sh's MC_MOUSE; without it the engine never leaves the menu and
# every mission reports NOLOAD.  Player input inside the mission stays EMPTY -- the last event is a
# release at t=7200ms, well before the roster spawns, so the run is pure AI-vs-AI from there on.
SP_MOUSE="200:160:100:0; 800:160:100:1; 1400:160:100:0; 3000:205:128:0; 3600:205:128:1; 4200:205:128:0; 5400:40:186:0; 6000:40:186:1; 6600:40:186:0; 7200:40:186:0"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT

# Runs write back into FISTDATA (.FPL pilot files); never let a sweep mutate the read-only originals.
DD="$TMP/data"; cp -r "$ROOT/armoredfist" "$DD"

missions=(); for f in "$ROOT"/armoredfist/FISTDATA/*.FSG "$ROOT"/armoredfist/FISTDATA/*.fsg; do
  [ -e "$f" ] || continue; b="$(basename "$f")"; b="${b%.*}"
  printf '%s' "$b" | grep -Eq "$FILTER" || continue
  missions+=("$b")
done
IFS=$'\n' missions=($(printf '%s\n' "${missions[@]}" | sort -u)); unset IFS

run_one() {  # $1=target $2=mission -> prints "STATE|min_a296|secs"
  local tgt="$1" m="$2" st t0 t1 rc out
  out="$TMP/$tgt.$m.err"
  t0=$SECONDS
  if [ "$tgt" = native ]; then
    setarch -R env FIST_DATADIR="$DD" FIST_SIMRUN=1 FIST_COOP_TICK=1 \
      FIST_FSG_BATTLE="$m" FIST_DUMPTICK="$TICKCAP" FIST_MOUSE="$SP_MOUSE" \
      timeout "$BUDGET" "$BIN" >/dev/null 2>"$out"; rc=$?
  else
    env FIST_DATADIR="$DD" FIST_SIMRUN=1 FIST_COOP_TICK=1 \
      FIST_FSG_BATTLE="$m" FIST_DUMPTICK="$TICKCAP" FIST_MOUSE="$SP_MOUSE" \
      timeout "$BUDGET" "$NODE" "$OUTJS" >/dev/null 2>"$out"; rc=$?
  fi
  t1=$((SECONDS - t0))
  local line min
  line="$(grep -m1 '^\[outcome\]' "$out" 2>/dev/null || true)"
  min="$(printf '%s' "$line" | sed -n 's/.*min_a296=\([0-9-]*\).*/\1/p')"; min="${min:--}"
  if   [ "$rc" = 124 ];                                    then st=TIMEOUT
  elif [ -z "$line" ] && [ "$rc" != 0 ];                   then st=CRASH
  elif printf '%s' "$line" | grep -q 'RESOLVED';           then st=RESOLVED
  elif printf '%s' "$line" | grep -q 'never loaded';       then st=NOLOAD
  elif [ -n "$line" ];                                     then st=UNRESOLVED
  else                                                          st=CRASH
  fi
  printf '%s|%s|%s' "$st" "$min" "$t1"
}

printf '=== self-play sweep: target=%s tick-cap=%s missions=%d budget=%ss ===\n' \
       "$TARGET" "$TICKCAP" "${#missions[@]}" "$BUDGET"
pass=0; fail=0; declare -a failed=()
for m in "${missions[@]}"; do
  if [ "$TARGET" = both ]; then
    IFS='|' read -r ns nm nt <<<"$(run_one native "$m")"
    IFS='|' read -r ws wm wt <<<"$(run_one wasm   "$m")"
    if [ "$ns" = RESOLVED ] && [ "$ws" = RESOLVED ] && [ "$nm" = "$wm" ]; then
      printf '  %-10s RESOLVED   both (min_a296=%s)  %ss/%ss\n' "$m" "$nm" "$nt" "$wt"; pass=$((pass+1))
    else
      printf '  %-10s native=%-10s(%s) wasm=%-10s(%s)  %ss/%ss\n' "$m" "$ns" "$nm" "$ws" "$wm" "$nt" "$wt"
      fail=$((fail+1)); failed+=("$m")
    fi
  else
    IFS='|' read -r s mn tt <<<"$(run_one "$TARGET" "$m")"
    if [ "$s" = RESOLVED ]; then
      printf '  %-10s RESOLVED   (min_a296=%s)  %ss\n' "$m" "$mn" "$tt"; pass=$((pass+1))
    else
      printf '  %-10s %-10s (min_a296=%s)  %ss\n' "$m" "$s" "$mn" "$tt"; fail=$((fail+1)); failed+=("$m")
    fi
  fi
done
printf '== self-play: %d resolved, %d not (%d missions) ==\n' "$pass" "$fail" "${#missions[@]}"
[ "$fail" -gt 0 ] && printf 'not resolved: %s\n' "${failed[*]}"
[ "$fail" -eq 0 ]
