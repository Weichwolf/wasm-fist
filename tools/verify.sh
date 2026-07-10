#!/usr/bin/env bash
# Stage-2/3 VERIFY HARNESS — the crash-free + bit-identical matrix that gates "done".
# Runs each covered FLOW on BOTH targets (native + wasm/node) and asserts, per flow:
#   (1) crash-free  (rc==0, no SEGV/abort)
#   (2) native <-> wasm framebuffer BIT-IDENTICAL  (cmp -l == 0 differing bytes)  [the hard invariant]
#   (3) framebuffer BIT-IDENTICAL to the genuine native DOSBox reference (if one exists)  [Stage-2 fidelity]
# Prints PASS/FAIL per flow + an overall count. This is the unit the 10x-consecutive-clean gate repeats
# (see tools/consecutive.sh — analog to DD2's passrun.sh/consecutive.sh).
#
# Usage:  bash tools/verify.sh [native|wasm|both]   (default: both)
# A FLOW = { name, tick-hz, run-ms, input-script (optional), dosbox-ref (optional) }. Flows are added
# here as screens/missions/settings/editor land; keep each flow DETERMINISTIC (fixed tick + scripted input).
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NATIVE="${NATIVE:-/tmp/fist_native}"
OUTJS="${OUTJS:-/tmp/fisttest/fistrun.js}"
NODE="$(ls "$HOME"/Git/emsdk/node/*/bin/node 2>/dev/null | head -1)"; NODE="${NODE:-node}"
WHICH="${1:-both}"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
pass=0; fail=0

# --- FLOW TABLE: name | tickhz | runms | mouse-script(FIST_MOUSE, may be empty) | dosbox-ref(may be empty) ---
# Extend as coverage grows.  The mouse-script field is the scripted-input program (FIST_MOUSE =
# "pump:x:y:btn; ..."); it may contain spaces/semicolons but MUST NOT contain '|'.
FLOWS=(
  "mainmenu|25000|22000||$ROOT/ref/main_menu_native320.png"
  "about|25000|22000|200:160:139:0; 800:160:139:1; 1400:160:139:0; 2000:160:138:0|$ROOT/ref/about_native320.png"
  "settings|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 2000:160:126:0|$ROOT/ref/settings_native320.png"
  "review|25000|22000|200:160:113:0; 800:160:113:1; 1400:160:113:0; 2000:160:113:0|$ROOT/ref/review_native320.png"
  "selplayer|25000|22000|200:160:74:0; 800:160:74:1; 1400:160:74:0; 2000:160:74:0|$ROOT/ref/selplayer_native320.png"
  "battles|25000|22000|200:160:100:0; 800:160:100:1; 1400:160:100:0; 2000:160:100:0|$ROOT/ref/battles_native320.png"
  "campaigns|25000|22000|200:160:87:0; 800:160:87:1; 1400:160:87:0; 2000:160:87:0|$ROOT/ref/campaigns_native320.png"
  # Interaction flows (patch 142): open a list dialog, then CLICK A ROW inside it -> the red selection
  # bar moves to the clicked row.  Two different dialogs (SELECT PLAYER via CAMPAIGNS row 87 selecting
  # KKR; SELECT BATTLE via BATTLES row 100 selecting AZER5).  The dialog covers the menu-item click
  # position (rows 87/100 are inside the panel), so no cursor ghost remains.
  "campaigns-select|25000|22000|200:160:87:0; 800:160:87:1; 1400:160:87:0; 3000:130:119:0; 3600:130:119:1; 4200:130:119:0; 4800:130:119:0|$ROOT/ref/campaigns_select_native320.png"
  "battles-select|25000|22000|200:160:100:0; 800:160:100:1; 1400:160:100:0; 3000:130:119:0; 3600:130:119:1; 4200:130:119:0; 4800:130:119:0|$ROOT/ref/battles_select_native320.png"
  # Close flow (patch 143): open the CAMPAIGNS list dialog, then click CANCEL -> the dialog closes and
  # the plain main menu is restored (the CANCEL button @205,140 is the final cursor rest position, so no
  # ghost).  Exercises the cb74/cb7c OK/CANCEL return-to-menu path (previously a base-lost SIGSEGV).
  "cancel|25000|22000|200:160:87:0; 800:160:87:1; 1400:160:87:0; 3000:205:140:0; 3600:205:140:1; 4200:205:140:0|$ROOT/ref/cancel_native320.png"
  # Close flow via the OTHER list-dialog front-end: BATTLES opens the SELECT BATTLE .FSG list (47 entries,
  # cb7c) then CANCEL @205,140 -> menu restored.  Independently exercises cb7c's shared-body close path
  # (distinct enumerate + dialog from campaigns/cb74); the destination frame is the plain menu.
  "battles-cancel|25000|22000|200:160:100:0; 800:160:100:1; 1400:160:100:0; 3000:205:140:0; 3600:205:140:1; 4200:205:140:0|$ROOT/ref/battles_cancel_native320.png"
  # Scroll flow (patch 144): open the BATTLES SELECT BATTLE list (47 entries) then PRESS-AND-HOLD the
  # down-scroll arrow (172,135).  The game's own auto-repeat (a85b=d103) steps the view down until it
  # CLAMPS at the bottom (offset 39 -> UKRAINE1-8, red bar on UKRAINE1).  The clamp is a deterministic
  # endpoint independent of the exact iteration/tick count, so native, wasm and DOSBox all settle on the
  # identical frame.  Ref captured mid-hold (button still down) via tools/refcapture_scroll.sh so the
  # arrow's pressed state matches; the port script holds the button through the capture (no release).
  "battles-scroll|25000|22000|200:160:100:0; 800:160:100:1; 1400:160:100:0; 3000:172:135:0; 3600:172:135:1|$ROOT/ref/battles_scroll_native320.png"
)

run_target() { # $1=target $2=hz $3=ms $4=mouse-script $5=out.ppm ; echo rc
  local t="$1" hz="$2" ms="$3" mouse="$4" out="$5"
  if [ "$t" = native ]; then
    timeout 40 env FIST_TICK_HZ="$hz" FIST_RUNMS="$ms" FIST_MOUSE="$mouse" FIST_FBDUMP="$out" "$NATIVE" >/dev/null 2>&1; echo $?
  else
    timeout 120 env FIST_TICK_HZ="$hz" FIST_RUNMS="$ms" FIST_MOUSE="$mouse" FIST_FBDUMP="$out" "$NODE" "$OUTJS" >/dev/null 2>&1; echo $?
  fi
}

echo "== verify ($WHICH) =="
for row in "${FLOWS[@]}"; do
  IFS='|' read -r name hz ms inp ref <<<"$row"
  ok=1; detail=""
  if [ "$WHICH" != wasm ]; then
    rc=$(run_target native "$hz" "$ms" "$inp" "$TMP/$name.nat.ppm"); [ "$rc" = 0 ] || { ok=0; detail+=" native-rc=$rc"; }
  fi
  if [ "$WHICH" != native ]; then
    rc=$(run_target wasm "$hz" "$ms" "$inp" "$TMP/$name.wasm.ppm"); [ "$rc" = 0 ] || { ok=0; detail+=" wasm-rc=$rc"; }
  fi
  # (2) native<->wasm bit-identical
  if [ "$WHICH" = both ] && [ -s "$TMP/$name.nat.ppm" ] && [ -s "$TMP/$name.wasm.ppm" ]; then
    d=$(cmp -l "$TMP/$name.nat.ppm" "$TMP/$name.wasm.ppm" 2>/dev/null | wc -l)
    [ "$d" = 0 ] || { ok=0; detail+=" nat!=wasm($d)"; }
  fi
  # (3) vs DOSBox native reference
  cap="$TMP/$name.nat.ppm"; [ "$WHICH" = wasm ] && cap="$TMP/$name.wasm.ppm"
  if [ -n "$ref" ] && [ -f "$ref" ] && [ -s "$cap" ]; then
    ae=$(compare -metric AE "$cap" "$ref" /dev/null 2>&1)
    [ "$ae" = 0 ] || { ok=0; detail+=" ref-AE=$ae"; }
  elif [ -n "$ref" ]; then detail+=" (no-ref)"; fi
  if [ "$ok" = 1 ]; then echo "  PASS $name$detail"; pass=$((pass+1)); else echo "  FAIL $name$detail"; fail=$((fail+1)); fi
done
echo "== verify: $pass passed, $fail failed =="
[ "$fail" = 0 ]
