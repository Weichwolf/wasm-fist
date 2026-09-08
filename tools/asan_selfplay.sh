#!/usr/bin/env bash
# Enumerate the base-loss cascade with AddressSanitizer instead of the layout lottery.  board:0007
#
# The port's out-of-bounds writes are near offsets used as host pointers.  Whether one FAULTS depends
# on what happens to sit below g_mem in .bss, so a build that "runs clean" may simply be corrupting
# mapped memory quietly -- adding two globals to the shim was enough to turn three of these from silent
# into fatal.  ASan removes the luck: every bad access is reported at its exact site with a stack.
#
#   tools/asan_selfplay.sh [mission] [tick-cap]
#
# Prints the ASan report, or "clean" if the mission reached the cap with no diagnostic.  Fix the site it
# names, rebuild, run again; each pass surfaces the next one.
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
M="${1:-AZER7}"; CAP="${2:-6000}"
BIN=/tmp/fist_asan
ERR="$(mktemp /tmp/asan.XXXXXX.err)"
DD="$(mktemp -d)/data"; cp -r "$ROOT/armoredfist" "$DD"    # runs write .FPL back; keep originals clean
trap 'rm -rf "$(dirname "$DD")"' EXIT

# Identical menu navigation to tools/selfplay.sh -- BATTLES -> OK -> ACCEPT, then EMPTY player input.
SP_MOUSE="200:160:100:0; 800:160:100:1; 1400:160:100:0; 3000:205:128:0; 3600:205:128:1; 4200:205:128:0; 5400:40:186:0; 6000:40:186:1; 6600:40:186:0; 7200:40:186:0"

echo "[asan] building $BIN"
ASAN='-fsanitize=address' bash "$ROOT/tools/build_native.sh" "$BIN" >/dev/null 2>&1 \
  || { echo "[asan] BUILD FAILED"; exit 2; }

echo "[asan] $M to [0x452]=$CAP"
ASAN_OPTIONS=detect_leaks=0 setarch -R env \
  FIST_DATADIR="$DD" FIST_SIMRUN=1 FIST_COOP_TICK=1 FIST_FSG_BATTLE="$M" \
  FIST_DUMPTICK="$CAP" FIST_MOUSE="$SP_MOUSE" \
  timeout "${ASAN_BUDGET:-1800}" "$BIN" >/dev/null 2>"$ERR"
rc=$?

if grep -q 'AddressSanitizer' "$ERR"; then
  sed -n '/AddressSanitizer/,/^SUMMARY/p' "$ERR" | grep -vE '^ *$' | head -30
  exit 1
fi
[ "$rc" = 124 ] && { echo "[asan] TIMEOUT (no diagnostic) -- raise ASAN_BUDGET or lower the cap"; exit 3; }
[ "$rc" != 0 ]  && { echo "[asan] exit $rc with no ASan diagnostic"; tail -3 "$ERR"; exit 4; }
echo "[asan] clean: $M reached [0x452]=$CAP with no diagnostic"
