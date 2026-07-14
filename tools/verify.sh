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
  # SETTINGS toggle interaction (patch 318): open SETTINGS (160,126), then CLICK the SKY checkbox
  # (181,23) -> the display-list element ACTIVATE handler 6af0 -> 6b27 -> 6bb1 toggles the SKY state
  # byte (0x8b46 1->0), sets the status "SKY DISABLED", marks the widget dirty (0x8ba9), and the
  # dirty-walk 209e re-paints the indicator OFF (dark).  The state/status/indicator all match DOSBox
  # bit-identically.  Root fixes: 6af0/6b27 DGROUP base-loss + the [0x626] box-outline XOR flash
  # (mga 1290), the checkbox-flag WORD->byte store-width bug (the XOR/compare pulled in an adjacent
  # byte -> wrong toggle direction + kept the indicator lit), and the widget dirty-flag WORD->byte
  # bug (the `=3` word write clobbered the adjacent renderer-index byte -> the re-paint dispatched the
  # wrong renderer).  Static frame (no blink) -> plain FIST_RUNMS dump.  READ-only (settings persist
  # only on ACCEPT).  ref via tools/refcapture_click2.sh 160 126 181 23 (2 independent DOSBox captures
  # AE=0 -> deterministic, non-circular).  AE=0 native AND wasm; native md5 == wasm md5.
  "settings-sky|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:181:23:0; 3600:181:23:1; 4200:181:23:0; 4800:181:23:0|$ROOT/ref/settings_sky_native320.png"
  # SETTINGS RADIO toggle (patch 319): open SETTINGS (160,126), then CLICK the MEDIUM DETAIL radio
  # (181,52).  The default DETAIL is HIGH (lit); clicking MEDIUM runs the ACTIVATE handler 6af0 -> 6b27
  # (now threads the handler id BX -> the radio handlers, patch 319) -> 6bcc(bx=0x14) -> 6eb8(2): sets
  # the DETAIL state word 0x8b47=2, updates the bottom status line to "DETAIL SET TO MEDIUM" via 7018
  # (reading the per-value status id from DGROUP:0x8b3f -- 6eb8's DGROUP base-loss fixed by 319), and
  # marks the 3 DETAIL radio widgets dirty (BYTE, retyped by 319).  The dirty-walk 209e re-paints the 3
  # radios via 6cf4 -> the lit indicator MOVES from HIGH (dark) to MEDIUM (lit) bit-identically.  Static
  # frame (no blink) -> plain FIST_RUNMS dump.  READ-only (a radio persists only on ACCEPT).  ref via
  # tools/refcapture_click2.sh 160 126 181 52 40 8 8 (2 independent DOSBox captures AE=0 -> deterministic,
  # non-circular).  AE=0 native AND wasm; native md5 == wasm md5, deterministic (5x single md5).
  "settings-detail-med|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:181:52:0; 3600:181:52:1; 4200:181:52:0; 4800:181:52:0|$ROOT/ref/settings_detail_med_native320.png"
  # SETTINGS RADIO toggle (patch 319), SOUND-FX group (3 radios): open SETTINGS, click the MEDIUM SOUND-FX
  # radio (246,150).  Default SOUND FX = HIGH (lit); the click -> 6af0 -> 6b27 -> 6bde(bx=0x1e..) ->
  # 6ee2: SOUND-FX state word 0x8b4b, marks the 3 SOUND-FX widgets dirty (4bc1/4bc4/4bc7, BYTE) -> the
  # lit indicator MOVES HIGH->MEDIUM (6d16 renderer).  No status change (SOUND has no 7018 status line).
  # ref via tools/refcapture_click2.sh 160 126 246 150 40 8 8 (2x AE=0, non-circular).  AE=0 both targets.
  "settings-sound-fx-med|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:246:150:0; 3600:246:150:1; 4200:246:150:0; 4800:246:150:0|$ROOT/ref/settings_sound_fx_med_native320.png"
  # SETTINGS RADIO toggle (patch 319), MUSIC group (2 radios): open SETTINGS, click the OFF MUSIC radio
  # (181,136).  Default MUSIC = ON (lit); the click -> 6b27 -> 6bd5 -> 6ed4: MUSIC state word 0x8b49,
  # marks the 2 MUSIC widgets dirty (uRam00024bb8/4bbb, BYTE) -> the lit indicator MOVES ON->OFF (6d05
  # renderer).  ref via tools/refcapture_click2.sh 160 126 181 136 40 8 8 (2x AE=0, non-circular).  AE=0
  # both targets.  (Together detail-med/sound-fx-med/music-off exercise every patch-319 change: the 6b27
  # arg thread, the 6eb8 status base-loss, and all 8 radio dirty-flag BYTE retypings.)
  "settings-music-off|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:181:136:0; 3600:181:136:1; 4200:181:136:0; 4800:181:136:0|$ROOT/ref/settings_music_off_native320.png"
  # SETTINGS joystick-TYPE radio toggle (patch 320), CONTROL column (5 radios): open SETTINGS (160,126),
  # click the STD JOYSTICK radio (35,34).  Default = NO JOYSTICK (lit).  The click -> 6af0 -> 6b27 ->
  # 6b55 (base-loss fixed by 320): reads the type index (1) via byte[DGROUP:0x8b5f+1], calls 6c38 ->
  # stores word[0x8b43]=1, marks the 5 joystick radios dirty (0x8b91/94/97/9a/9d, BYTE-retyped by 320),
  # and builds the status line via the 444f message net-effect.  The dirty-walk 209e re-paints via the
  # joystick-radio renderer 6cb6 (`si>>1 == [0x8b43]`) -> the lit indicator MOVES NO->STD.  With no
  # joystick present (shim + DOSBox-under-xvfb both) the device does not respond -> status = "STANDARD
  # JOYSTICK NOT RESPONDING!" (typename node1 "STANDARD" + str-seg:0x41f " JOYSTICK NOT RESPONDING!").
  # ref via tools/refcapture_click2.sh 160 126 35 34 40 10 10 (2 independent DOSBox captures AE=0,
  # non-circular; port md5 139294fd != any circular self-compare).  AE=0 native AND wasm; native md5 ==
  # wasm md5, deterministic (3x single md5).
  "settings-joystick|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:35:34:0; 3600:35:34:1; 4200:35:34:0; 4800:35:34:0|$ROOT/ref/settings_joystick_native320.png"
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
  # Page-scroll flow (patch 145): open the BATTLES list, then SINGLE-CLICK the scrollbar TRACK below
  # the thumb (172,115) -> ce37 press path -> d146 PAGE DOWN (+8 rows; cef4 clamps the selection into
  # view) -> CYPRUS2..INDIA2, red bar on CYPRUS2.  A single click is a deterministic endpoint (no hold
  # timing); the final cursor rests on the track inside the panel, so no ghost.
  "battles-page|25000|22000|200:160:100:0; 800:160:100:1; 1400:160:100:0; 3000:172:115:0; 3600:172:115:1; 4200:172:115:0|$ROOT/ref/battles_page_native320.png"
  # OK-path WRITE flow (patch 146): open SELECT PLAYER (row 74), then click OK (205,128) on the default
  # player D.  The engine SAVES the profile -- 6a9c reads+checksums D.FPL, 6ade rewrites it (byte-identical)
  # -- copies the name into CURRENT PLAYER (STRSEG:0xbf1, [0x6dad]=1), and returns to the menu, which now
  # draws "CURRENT PLAYER:D" at the bottom.  This is a WRITE flow -> isolated in a fresh scratch datadir
  # (see WRITEFLOWS above); the ref is captured the same way (tools/refcapture_ok.sh 160 74 205 128).
  "selplayer-ok|25000|22000|200:160:74:0; 800:160:74:1; 1400:160:74:0; 3000:205:128:0; 3600:205:128:1; 4200:205:128:0; 4800:205:128:0|$ROOT/ref/selplayer_ok_native320.png"
  # OK-path WRITE flow (patch 146), the ROW-SELECT variant: open SELECT PLAYER (row 74), SELECT the KKR
  # row (130,119 -> a838=4), then OK (205,128).  Exercises the non-default name copy (e9bd from
  # STRSEG:(a838*0x10+0x3776)) + the .FPL I/O for a different profile (KKR.FPL) -> the menu draws
  # "CURRENT PLAYER:KKR".  Isolated (write flow); ref via tools/refcapture_ok3.sh 160 74 130 119 205 128.
  "selplayer-ok-kkr|25000|22000|200:160:74:0; 800:160:74:1; 1400:160:74:0; 2400:130:119:0; 2800:130:119:1; 3200:130:119:0; 3800:205:128:0; 4200:205:128:1; 4600:205:128:0; 5000:205:128:0|$ROOT/ref/selplayer_ok_kkr_native320.png"
  # CAMPAIGNS -> OK -> the SELECT A CAMPAIGN screen (patch 148).  Click CAMPAIGNS (row 87) to open the
  # SELECT PLAYER roster, then OK (205,128) to pick the default player D -> e78c copies the name + reads
  # the .FPL, then opens FUN_0000_ec7e = the campaign modal screen: a left LIST of the 7 decrypted campaign
  # names (TRAINING [red-selected]..BURNING FROST, from patch 147's ef5e chain), a right HARDWARE group
  # (WESTERN [red]/EASTERN radios), OK/CANCEL.  ec7e's first settled frame is the deliverable (READ-only:
  # 6ade only fires on ec7e's own OK), isolated anyway (WRITE flow); ref via tools/refcapture_ok.sh 160 87 205 128.
  "campaigns-ok|25000|22000|200:160:87:0; 800:160:87:1; 1400:160:87:0; 3000:205:128:0; 3600:205:128:1; 4200:205:128:0; 4800:205:128:0|$ROOT/ref/campaigns_ok_native320.png"
  # CAMPAIGNS -> OK -> campaign-OK -> the INTRODUCTORY CAMPAIGN mission-select screen (patches 149/150).
  # THREE clicks: CAMPAIGNS (row 87) opens the SELECT PLAYER roster; OK (205,128) picks player D and opens
  # the SELECT A CAMPAIGN modal (ec7e); campaign-OK (203,159) runs ec7e's tail -- f0f1 parses the selected
  # campaign's .CAM missions, e78c writes the player .FPL (6ade -> WRITE flow, isolated), and opens
  # FUN_0000_f338 = the mission-select screen: 4 mission marker boxes (state-coloured bevel: outer black,
  # middle state-colour, INNER gray idx 2 -- patch 150's 11ba al=2), the selected mission's name + briefing,
  # ACCEPT/CANCEL.  The marker layer is TICK-PHASE-dependent (be47 draws on [0x452]&0xf==0; the selected
  # marker blinks on [0x452]&0x10), so the frame is captured PHASE-PINNED via "tick=8008": dump when the
  # engine frame-timer [0x452] first reaches 8008 (FIST_DUMPTICK) -- 8008 is 8 ticks past the mult-of-32
  # boundary 8000, so BOTH native and wasm have caught the [0x452]==8000 be47 draw (bit4=0 -> selected
  # marker BLACK, matching the DOSBox capture) -> the marker layer is native<->wasm bit-identical (a plain
  # wall-clock FIST_RUNMS dump lands at a target-dependent phase and diverges).  ref via
  # tools/refcapture_ok3.sh 160 87 205 128 203 159 (3x AE=0, non-circular; DOSBox blink phase = selected
  # marker black, reproducible at SETTLE=8).  AE=0 native AND wasm; native md5 == wasm md5, deterministic.
  "campaign-missions|25000|tick=8008|200:160:87:0; 800:160:87:1; 1400:160:87:0; 3000:205:128:0; 3600:205:128:1; 4200:205:128:0; 5400:203:159:0; 6000:203:159:1; 6600:203:159:0; 7200:203:159:0|$ROOT/ref/campaign_missions_native320.png"
  # BATTLES -> OK -> the mission BRIEFING screen (patch 151).  TWO clicks: BATTLES (row 100) opens the
  # SELECT BATTLE .FSG list dialog (cb7c); OK (205,128) picks the default battle AZER1 -> e87a proceeds
  # (asm `jae 0xe891`) into `7088(0)` = the mission-briefing modal dialog (id=0).  Its SETUP FUN_0000_7162
  # opens AZER1.FSW (INT 21h AH=3D) and reads the briefing text into STRSEG:0x452c; the text drawer 6c2d
  # renders it ("BLOODFEUD! / AS ARMENIAN FORCES PUSH INTO AZERBAIJAN...") on the FLDCOMP panel with
  # ACCEPT/CANCEL.  The frame is static (no blink) -> plain FIST_RUNMS dump; READ-only (7162 only reads
  # .FSW; the editor build + its .FPL write are deferred behind ACCEPT).  ref via
  # tools/refcapture_ok.sh 160 100 205 128 (2 independent DOSBox captures AE=0 -> deterministic,
  # non-circular).  AE=0 native AND wasm; native md5 == wasm md5.
  "battles-ok|25000|22000|200:160:100:0; 800:160:100:1; 1400:160:100:0; 3000:205:128:0; 3600:205:128:1; 4200:205:128:0; 4800:205:128:0|$ROOT/ref/battles_ok_native320.png"
  # BATTLES -> OK -> briefing -> CANCEL -> menu (patches 152/153).  THREE clicks: BATTLES (row 100) opens
  # the SELECT BATTLE .FSG list (cb7c); OK (205,128) picks AZER1 -> the mission-briefing modal (7088 id=0);
  # CANCEL (78,186 -- the 2nd bottom button, right of ACCEPT@40,186) dismisses it.  The briefing button
  # activate FUN_0000_7217 (patch 152: elem/base DGROUP-offset rebase + the dropped confirm-dispatch AX ->
  # 7249 sets DAT_2000_4be2=0xff=CANCEL) fires, then 7088's exit tears down the dialog display list and
  # RESTORES the menu's (patch 153: the 1cdb restore call `1cdb(iVar2,0x8cee,0x94,0x10,..)` -- 151 emitted a
  # scrambled `1cdb(0,0x10,0x8cee,iVar2,..)` that base-loss-crashed FUN_0000_9a27) -> the plain main menu is
  # restored bit-identically (only the cursor now rests at 78,186).  TICK-PINNED via "tick=2600" (FIST_DUMPTICK
  # -- the menu-idle repaint after the dialog teardown settles by [0x452]~1280 and stays AE=0 through ~5458;
  # a fixed tick makes it native<->wasm identical AND run-deterministic, avoiding a rare wall-clock mid-repaint
  # catch).  READ-only (briefing 7162 only reads AZER1.FSW).  ref via
  # tools/refcapture_ok3.sh 160 100 205 128 78 186 40 8 8 (DOSBox also returns to menu on CANCEL, cursor@78,186;
  # 2 independent captures AE=0, non-circular).  AE=0 native AND wasm; native md5 == wasm md5.
  "battles-cancel-briefing|25000|tick=2600|200:160:100:0; 800:160:100:1; 1400:160:100:0; 3000:205:128:0; 3600:205:128:1; 4200:205:128:0; 5400:78:186:0; 6000:78:186:1; 6600:78:186:0; 7200:78:186:0|$ROOT/ref/battles_cancel_briefing_native320.png"
)

# ============================ WRITE-ISOLATION POLICY ============================
# A few flows exercise the engine's REAL filesystem WRITES (faithful behaviour, no engine-side
# suppression): the SELECT PLAYER / CAMPAIGNS "OK" path saves the selected player profile via the
# engine's 6a9c(read+checksum) / 6ade(create+write) INT-21h AH=3C/40 (a .FPL under FISTDATA/).  The
# port resolves file paths through $FIST_DATADIR (re_out/fist_dos.c: datadir(), default "armoredfist"),
# and DOSBox resolves them through the mounted directory -- so BOTH sides can be isolated purely at the
# HARNESS level, without touching the faithful engine path.  For every WRITE flow we run the port
# against a FRESH per-run cp -a copy of armoredfist/ (a scratch datadir) so:
#   * the run starts from the identical pristine initial state every time (determinism), and
#   * the repo's armoredfist/ is never mutated (`git status` stays clean after a full verify).
# The DOSBox reference for these flows is captured the SAME way (tools/refcapture_ok.sh mounts a fresh
# cp -a copy), so the reference and the port exercise the same initial state.  Names of write flows:
WRITEFLOWS=" selplayer-ok selplayer-ok-kkr campaigns-ok campaign-missions "

is_write_flow() { case "$WRITEFLOWS" in *" $1 "*) return 0;; *) return 1;; esac; }

run_target() { # $1=target $2=hz $3=ms/dumptick $4=mouse-script $5=out.ppm $6=datadir(optional) ; echo rc
  local t="$1" hz="$2" ms="$3" mouse="$4" out="$5" dd="${6:-}"
  local ddenv=(); [ -n "$dd" ] && ddenv=(FIST_DATADIR="$dd")
  # The 3rd flow field is normally FIST_RUNMS (wall-clock dump deadline).  For screens whose frame has a
  # TICK-PHASE-dependent layer (e.g. the campaign mission-marker blink FUN_1000_be47, gated on the engine
  # frame-timer [0x452]&0xf / &0x10), a wall-clock dump lands at a target-dependent [0x452] -> native and
  # wasm would differ.  Such flows use "tick=N": dump when the engine's own [0x452] counter first reaches
  # N (FIST_DUMPTICK) -- identical on both targets -> the phase-dependent layer is bit-identical.  (See
  # the campaign-missions flow.)
  local dumpenv=(); case "$ms" in tick=*) dumpenv=(FIST_DUMPTICK="${ms#tick=}");; *) dumpenv=(FIST_RUNMS="$ms");; esac
  if [ "$t" = native ]; then
    timeout 40 env "${ddenv[@]}" FIST_TICK_HZ="$hz" "${dumpenv[@]}" FIST_MOUSE="$mouse" FIST_FBDUMP="$out" "$NATIVE" >/dev/null 2>&1; echo $?
  else
    timeout 120 env "${ddenv[@]}" FIST_TICK_HZ="$hz" "${dumpenv[@]}" FIST_MOUSE="$mouse" FIST_FBDUMP="$out" "$NODE" "$OUTJS" >/dev/null 2>&1; echo $?
  fi
}

# Fresh per-run scratch copy of the data dir for a WRITE flow (isolates engine writes from the repo).
fresh_datadir() { # $1=tag ; echo path
  local dd="$TMP/data.$1"; rm -rf "$dd"; cp -a "$ROOT/armoredfist" "$dd"; echo "$dd"
}

echo "== verify ($WHICH) =="
for row in "${FLOWS[@]}"; do
  IFS='|' read -r name hz ms inp ref <<<"$row"
  ok=1; detail=""
  # WRITE-ISOLATION: write flows run against a fresh cp -a scratch datadir (repo stays clean, run is
  # deterministic from pristine initial state).  Read-only flows use the repo armoredfist/ directly.
  ddN=""; ddW=""
  if is_write_flow "$name"; then detail+=" [isolated]"
    [ "$WHICH" != wasm ] && ddN="$(fresh_datadir "$name.nat")"
    [ "$WHICH" != native ] && ddW="$(fresh_datadir "$name.wasm")"
  fi
  if [ "$WHICH" != wasm ]; then
    rc=$(run_target native "$hz" "$ms" "$inp" "$TMP/$name.nat.ppm" "$ddN"); [ "$rc" = 0 ] || { ok=0; detail+=" native-rc=$rc"; }
  fi
  if [ "$WHICH" != native ]; then
    rc=$(run_target wasm "$hz" "$ms" "$inp" "$TMP/$name.wasm.ppm" "$ddW"); [ "$rc" = 0 ] || { ok=0; detail+=" wasm-rc=$rc"; }
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
