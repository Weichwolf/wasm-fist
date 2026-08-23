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
  # INTRO FMV (TITLE.KDV) -- the settled "ARMORED FIST" title card.  The Doug-Huffman extender's KDV player
  # (Route-1 module fist_ext.c) decodes+blits TITLE.KDV; the stream ends on a ~25-frame HOLD of the fully
  # formed title (frames ~370..394 identical) before the engine fades to the main menu.  Pinned by FRAME
  # INDEX 385 (well inside the hold) via kdvframe=385 -> FIST_KDV=1 + FIST_KDV_DUMPFRAME.  The frame-pin
  # (native_main.c) BYPASSES e584 ENTIRELY: the display-list SETUP ops (0x20/0x04/0x44/0x68/0x6c/0x70)
  # flow through the extender gate ONCE and establish the decoder state (asset name "TITLE.KDV" at the intro
  # task +0xBA); on op 0x70 -- the LAST setup op, fired BEFORE e584 enters its throttled per-frame present
  # loop -- the gate OPENs the stream and streams frames 1..385 DIRECTLY via a tight 11dd loop (one KDV chunk
  # per call), dumps 0xA0000, and exits.  It NEVER waits for an e584-posted op-0x78 present, so frame delivery
  # is fully TIMING-INDEPENDENT: the wasm cooperative tick's e584 throttle crawl / skip-keypress abort is
  # sidestepped, and frame 385 (a pure function of consuming 385 chunks) is byte-IDENTICAL native<->wasm in
  # <0.3 s on both.  The genuine 1:1 320x200 DOSBox reference is captured by tools/refcapture_intro.sh, which
  # bursts across the hold and selects the longest non-terminal stable hold (the title) by DOSBox-internal
  # pixel signature (never compared to the port -> non-circular; two independent DOSBox runs agree AE=0).
  # READ-only.  AE=0 native AND wasm; native md5 == wasm md5 (0-diff).  Needs re_out/fist_image.bin (the
  # extender/KDV player image, a `make kernel-image` build artifact -- gitignored, like the other images).
  "intro|25000|kdvframe=385||$ROOT/ref/intro_title_native320.png"
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
  # SETTINGS RADIO toggle (patch 319), DETAIL group's THIRD radio (LOW): open SETTINGS (160,126), click
  # the LOW DETAIL radio (181,44 -- the DETAIL radios stack LOW/MED/HIGH at y 44/52/60, default HIGH lit).
  # Same 6bcc handler as MED but bx=0x12 -> 6eb8(value 0): sets DETAIL word 0x8b47=0, status "DETAIL SET
  # TO LOW" via 7018 reading the per-value status id from DGROUP:0x8b3d (the LOW slot; MED used 0x8b3f),
  # marks the 3 DETAIL widgets dirty (BYTE) -> 209e/6cf4 re-paints the lit indicator HIGH -> LOW.
  # Exercises the LOW value/status slot the MED flow does not (value 0 vs 2, 0x8b3d vs 0x8b3f).  No new
  # engine patch (319 covers all three DETAIL radios).  Static frame -> plain FIST_RUNMS dump.  READ-only.
  # ref via tools/refcapture_click2.sh 160 126 181 44 40 8 8 (2 independent DOSBox captures AE=0 ->
  # deterministic, non-circular).  AE=0 native AND wasm; native md5 == wasm md5.
  "settings-detail-low|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:181:44:0; 3600:181:44:1; 4200:181:44:0; 4800:181:44:0|$ROOT/ref/settings_detail_low_native320.png"
  # SETTINGS RADIO toggle (patch 319), SOUND-FX group (3 radios): open SETTINGS, click the MEDIUM SOUND-FX
  # radio (246,150).  Default SOUND FX = HIGH (lit); the click -> 6af0 -> 6b27 -> 6bde(bx=0x1e..) ->
  # 6ee2: SOUND-FX state word 0x8b4b, marks the 3 SOUND-FX widgets dirty (4bc1/4bc4/4bc7, BYTE) -> the
  # lit indicator MOVES HIGH->MEDIUM (6d16 renderer).  No status change (SOUND has no 7018 status line).
  # ref via tools/refcapture_click2.sh 160 126 246 150 40 8 8 (2x AE=0, non-circular).  AE=0 both targets.
  "settings-sound-fx-med|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:246:150:0; 3600:246:150:1; 4200:246:150:0; 4800:246:150:0|$ROOT/ref/settings_sound_fx_med_native320.png"
  # settings-sound-fx-off: SOUND FX radio OFF (x246, the aligned column).  Default is HIGH; clicking OFF
  # (246,137) selects it (LED moves HIGH->OFF, 6ee2/6d16 renderer).  No engine change needed (the radio
  # dirty/render already correct at this column); pure coverage of the OFF radio state.  AE=0 both targets.
  "settings-sound-fx-off|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:246:137:0; 3600:246:137:1; 4200:246:137:0; 4800:246:137:0|$ROOT/ref/settings_sound_fx_off_native320.png"
  # SETTINGS RADIO toggle (patch 319), MUSIC group (2 radios): open SETTINGS, click the OFF MUSIC radio
  # (181,136).  Default MUSIC = ON (lit); the click -> 6b27 -> 6bd5 -> 6ed4: MUSIC state word 0x8b49,
  # marks the 2 MUSIC widgets dirty (uRam00024bb8/4bbb, BYTE) -> the lit indicator MOVES ON->OFF (6d05
  # renderer).  ref via tools/refcapture_click2.sh 160 126 181 136 40 8 8 (2x AE=0, non-circular).  AE=0
  # both targets.  (Together detail-med/sound-fx-med/music-off exercise every patch-319 change: the 6b27
  # arg thread, the 6eb8 status base-loss, and all 8 radio dirty-flag BYTE retypings.)
  "settings-music-off|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:181:136:0; 3600:181:136:1; 4200:181:136:0; 4800:181:136:0|$ROOT/ref/settings_music_off_native320.png"
  # settings-smoke-off: DISPLAY SMOKE EFFECTS checkbox toggle OFF (patch 387 fixed the LED re-render --
  # store-width dirty-flag 0x8bb5, patch-318/319 class).  Click SETTINGS(160,126) -> SMOKE LED(181,83).
  "settings-smoke-off|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:181:83:0; 3600:181:83:1; 4200:181:83:0; 4800:181:83:0|$ROOT/ref/settings_smoke_off_native320.png"
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
  # SETTINGS joystick radios (x35 aligned column): each selects a joystick type (LED moves), AE=0 both
  # targets, no engine change (the joystick radio render is already correct -- settings-joystick=STD).
  "settings-joy-flightstick|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:35:46:0; 3600:35:46:1; 4200:35:46:0; 4800:35:46:0|$ROOT/ref/settings_joy_flightstick_native320.png"
  "settings-joy-tmfcs|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:35:58:0; 3600:35:58:1; 4200:35:58:0; 4800:35:58:0|$ROOT/ref/settings_joy_tmfcs_native320.png"
  "settings-joy-ch|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:35:69:0; 3600:35:69:1; 4200:35:69:0; 4800:35:69:0|$ROOT/ref/settings_joy_ch_native320.png"
  "settings-joy-tmwcs|25000|22000|200:160:126:0; 800:160:126:1; 1400:160:126:0; 3000:35:88:0; 3600:35:88:1; 4200:35:88:0; 4800:35:88:0|$ROOT/ref/settings_joy_tmwcs_native320.png"
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
  "battles-cancel-briefing|25000|tick=6000|200:160:100:0; 800:160:100:1; 1400:160:100:0; 3000:205:128:0; 3600:205:128:1; 4200:205:128:0; 5400:78:186:0; 6000:78:186:1; 6600:78:186:0; 7200:78:186:0|$ROOT/ref/battles_cancel_briefing_native320.png"
  # EDITOR .FSG battle-SAVE round-trip (patches 360/361) -- the first LEVEL/MISSION-EDITOR DoD
  # deliverable, DATA-level (no terrain/render dependency).  Special-cased below (file compare, not
  # framebuffer): BATTLES->OK->ACCEPT loads AZER1.FSG via FUN_0000_d501; the patch-361 harness hook
  # (FIST_FSG_ROUNDTRIP) re-saves it via the reconstructed serializer FUN_0000_d5f9/d6e4 (patch 360)
  # right after the load (pre-sim, deterministic) and exits.  The round-trip is a FIXED POINT: the
  # LOAD (d81e/43c1/c296/bb12) canonicalizes each unit (state flags, cell-index, derived fields;
  # asm-verified patches 200/214), so the editor-authored AZER1.FSG is NOT itself a fixed point (144
  # faithful-canonicalization diffs), but a RE-saved file IS.  Verified: load(orig)->save=file1;
  # load(file1)->save=file2; file1==file2, native AND wasm, native file1==wasm file1 (0-diff).  A
  # WRITE flow (own fresh cp -a scratch datadirs; repo armoredfist/ untouched).  d5f9/d6e4/d84e/
  # d740/b40b/d638 asm-verified vs re_out/fist_dat_image.bin.
  "editor-fsg-roundtrip|25000|roundtrip||roundtrip"
  # EDITOR .FSG round-trip generalized to MORE battles (patch 380: FIST_FSG_BATTLE overrides the loaded
  # battle filename so the single RT_MOUSE navigation loads any of the 47 .FSG).  A representative set:
  # one battle per THEATRE (CYPRUS/INDIA/SAUDI/SYRIA/UKRAINE/TRAIN -- different terrain/unit/path data
  # than AZER1) + the LARGEST .FSG (UKRAINE8, 19022 B / most chunks+units -- stresses the serializer).
  # Each proves the serializer d6e4 + parser d501 generalize past AZER1: load->save is an idempotent
  # FIXED POINT (load(orig)->save=f1; load(f1)->save=f2; f1==f2), native AND wasm, native f1==wasm f1.
  # The battle name is the 4th field.  WRITE flows (own fresh cp -a scratch datadir; repo untouched).
  "editor-fsg-cyprus1|25000|roundtrip|CYPRUS1|roundtrip"
  "editor-fsg-india1|25000|roundtrip|INDIA1|roundtrip"
  "editor-fsg-saudi1|25000|roundtrip|SAUDI1|roundtrip"
  "editor-fsg-syria1|25000|roundtrip|SYRIA1|roundtrip"
  "editor-fsg-ukraine1|25000|roundtrip|UKRAINE1|roundtrip"
  "editor-fsg-train1|25000|roundtrip|TRAIN1|roundtrip"
  "editor-fsg-ukraine8|25000|roundtrip|UKRAINE8|roundtrip"
  # EDITOR ROUND-TRIP generalized to ALL 47 battles (DoD completeness: every campaign mission's
  # .FSG load->save->reload is a byte-identical FIXED POINT, native AND wasm, native==wasm 0-diff).
  "editor-fsg-azer2|25000|roundtrip|AZER2|roundtrip"
  "editor-fsg-azer3|25000|roundtrip|AZER3|roundtrip"
  "editor-fsg-azer4|25000|roundtrip|AZER4|roundtrip"
  "editor-fsg-azer5|25000|roundtrip|AZER5|roundtrip"
  "editor-fsg-azer6|25000|roundtrip|AZER6|roundtrip"
  "editor-fsg-azer7|25000|roundtrip|AZER7|roundtrip"
  "editor-fsg-cyprus2|25000|roundtrip|CYPRUS2|roundtrip"
  "editor-fsg-cyprus3|25000|roundtrip|CYPRUS3|roundtrip"
  "editor-fsg-cyprus4|25000|roundtrip|CYPRUS4|roundtrip"
  "editor-fsg-cyprus5|25000|roundtrip|CYPRUS5|roundtrip"
  "editor-fsg-cyprus6|25000|roundtrip|CYPRUS6|roundtrip"
  "editor-fsg-cyprus7|25000|roundtrip|CYPRUS7|roundtrip"
  "editor-fsg-india2|25000|roundtrip|INDIA2|roundtrip"
  "editor-fsg-india3|25000|roundtrip|INDIA3|roundtrip"
  "editor-fsg-india4|25000|roundtrip|INDIA4|roundtrip"
  "editor-fsg-india5|25000|roundtrip|INDIA5|roundtrip"
  "editor-fsg-india6|25000|roundtrip|INDIA6|roundtrip"
  "editor-fsg-india7|25000|roundtrip|INDIA7|roundtrip"
  "editor-fsg-saudi2|25000|roundtrip|SAUDI2|roundtrip"
  "editor-fsg-saudi3|25000|roundtrip|SAUDI3|roundtrip"
  "editor-fsg-saudi4|25000|roundtrip|SAUDI4|roundtrip"
  "editor-fsg-saudi5|25000|roundtrip|SAUDI5|roundtrip"
  "editor-fsg-saudi6|25000|roundtrip|SAUDI6|roundtrip"
  "editor-fsg-saudi7|25000|roundtrip|SAUDI7|roundtrip"
  "editor-fsg-syria2|25000|roundtrip|SYRIA2|roundtrip"
  "editor-fsg-syria3|25000|roundtrip|SYRIA3|roundtrip"
  "editor-fsg-syria4|25000|roundtrip|SYRIA4|roundtrip"
  "editor-fsg-syria5|25000|roundtrip|SYRIA5|roundtrip"
  "editor-fsg-syria6|25000|roundtrip|SYRIA6|roundtrip"
  "editor-fsg-syria7|25000|roundtrip|SYRIA7|roundtrip"
  "editor-fsg-train2|25000|roundtrip|TRAIN2|roundtrip"
  "editor-fsg-train3|25000|roundtrip|TRAIN3|roundtrip"
  "editor-fsg-train4|25000|roundtrip|TRAIN4|roundtrip"
  "editor-fsg-ukraine2|25000|roundtrip|UKRAINE2|roundtrip"
  "editor-fsg-ukraine3|25000|roundtrip|UKRAINE3|roundtrip"
  "editor-fsg-ukraine4|25000|roundtrip|UKRAINE4|roundtrip"
  "editor-fsg-ukraine5|25000|roundtrip|UKRAINE5|roundtrip"
  "editor-fsg-ukraine6|25000|roundtrip|UKRAINE6|roundtrip"
  "editor-fsg-ukraine7|25000|roundtrip|UKRAINE7|roundtrip"
  # EDITOR ADD-TANK edit-op (patch 362): enter EDITING mode, add ONE tank via the engine's real
  # roster allocator + unit registry, save.  Asserts the edited .FSG has base+1 (81) DCBS units, the
  # added tank RELOADS, and load->save on the edited file is an idempotent FIXED POINT -- native AND
  # wasm, native==wasm 0-diff.  A WRITE flow (own fresh cp -a scratch datadirs; repo untouched).
  "editor-add-tank|25000|addtank||addtank"
  # ADD-TANK generalized across theaters (patch 362 hook is battle-agnostic -- clones the first loaded
  # friendly unit): each asserts the loaded battle's ORIGINAL DCBS baseline -> base+1, the added tank
  # RELOADS, and the edited file is an idempotent fixed point.  native AND wasm, native==wasm 0-diff.
  "editor-add-tank-azer2|25000|addtank||addtank"
  "editor-add-tank-azer5|25000|addtank||addtank"
  "editor-add-tank-cyprus3|25000|addtank||addtank"
  "editor-add-tank-cyprus6|25000|addtank||addtank"
  "editor-add-tank-india2|25000|addtank||addtank"
  "editor-add-tank-india5|25000|addtank||addtank"
  "editor-add-tank-saudi2|25000|addtank||addtank"
  "editor-add-tank-syria2|25000|addtank||addtank"
  "editor-add-tank-ukraine5|25000|addtank||addtank"
  "editor-add-tank-train3|25000|addtank||addtank"
  # REMOVE-TANK edit-op (patch 405, inverse of add-tank via the REAL b2ef destroy): same theater spread,
  # each asserts base->base-1 + reload-stable idempotent fixed point, native AND wasm, native==wasm 0-diff.
  "editor-rem-tank|25000|remtank||remtank"
  "editor-rem-tank-azer2|25000|remtank||remtank"
  "editor-rem-tank-azer5|25000|remtank||remtank"
  "editor-rem-tank-cyprus3|25000|remtank||remtank"
  "editor-rem-tank-cyprus6|25000|remtank||remtank"
  "editor-rem-tank-india2|25000|remtank||remtank"
  "editor-rem-tank-india5|25000|remtank||remtank"
  "editor-rem-tank-saudi2|25000|remtank||remtank"
  "editor-rem-tank-syria2|25000|remtank||remtank"
  "editor-rem-tank-ukraine5|25000|remtank||remtank"
  "editor-rem-tank-train3|25000|remtank||remtank"
  # ADD-ENEMY edit-op (patch 406): exercises b21d's ENEMY roster pool (0xc05c), untested by the friendly
  # add-tank.  base->base+1 + idempotent fixed point, native AND wasm, native==wasm 0-diff.
  "editor-add-enemy|25000|addenemy||addenemy"
  "editor-add-enemy-azer2|25000|addenemy||addenemy"
  "editor-add-enemy-cyprus3|25000|addenemy||addenemy"
  "editor-add-enemy-india2|25000|addenemy||addenemy"
  "editor-add-enemy-saudi2|25000|addenemy||addenemy"
  # EDITOR "simulate" leg (create->save->reload->SIMULATE): add-tank edit a battle, then LOAD+SPAWN the
  # edited .FSG and confirm its op-0x2c cockpit renders bit-identically (AE=0 vs saudi1 ref, native+wasm).
  "editor-sim-saudi2|25000|editsim||$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "editor-sim-cyprus6|25000|editsim||$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "editor-sim-azer4|25000|editsim||$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "editor-sim-syria1|25000|editsim||$ROOT/ref/mission_saudi1_cockpit_native320.png"
  # REMOVE-TANK "simulate" leg: a battle with one friendly REMOVED still loads + spawns bit-identically
  # (removing a friendly is central-chrome-neutral -> AE=0 vs the shared ref, native+wasm, native==wasm).
  "editor-remsim-saudi2|25000|editsim||$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "editor-remsim-cyprus6|25000|editsim||$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "editor-remsim-azer4|25000|editsim||$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "editor-remsim-syria1|25000|editsim||$ROOT/ref/mission_saudi1_cockpit_native320.png"
  # FIRST DUAL-TARGET IN-MISSION SURFACE (patches 364/365/366/367 objtype-0x02 + b059 crash-free, 381/382
  # display-walk arg-thread wasm render, +0x3e camera seed): the AZER1-spawn cockpit CENTRAL-CHROME
  # (engine-2322 dashboard, cols80-180 rows96-188) is bit-exact AE=0 vs a genuine DOSBox ref on BOTH
  # native AND wasm, native<->wasm 0-diff.  BATTLES(160,100)->OK(205,128)->ACCEPT(40,186) drives the
  # mission; FIST_MISSFB captures the settled op-0x24 render post #1 (deterministic -- raw-region md5
  # 355e5bc63d run-stable).  REGION-LIMITED to cols80-180 rows96-188: the full frame differs in rows0-19
  # (the terrain windshield -- mission-terrain-tile-pipeline frontier, a documented open surface).  WRITE-
  # isolated (fresh cp -a datadir).  Needs re_out/fist_image.bin (extender render, a `make kernel-image`
  # build artifact -- gitignored) or the render is skipped.  Special-cased below (FIST_MISSFB + region crop).
  "mission-cockpit|25000|missfb||$ROOT/ref/mission_azer1_cockpit_native320.png"
  # mission-cockpit-cyprus1: second bit-verified in-mission surface -- CYPRUS1 (map D06/C06, M1 idx0)
  # reached via FIST_FSG_BATTLE (patch 380), central-chrome AE=0 both targets vs the DOSBox spawn ref
  # (patch 386 fixed the wasm OOB in FUN_0000_bd09 that blocked all non-AZER1 maps).  Battle in `inp`.
  "mission-cockpit-cyprus1|25000|missfb|CYPRUS1|$ROOT/ref/mission_cyprus1_cockpit_native320.png"
  # mission-cockpit-{saudi1,syria1,india1}: unblocked by the wasm-mission tick-hold fix (49f8cfb) -- they
  # rendered on native but HUNG on wasm before it.  Central-chrome (MC_REGION) AE=0 both targets vs a GENUINE
  # DOSBox spawn ref.  ref/mission_saudi1_cockpit_native320.png is a genuine scroll-captured DOSBox SAUDI1
  # frame (its full frame differs from AZER1/CYPRUS1 = real distinct mission/terrain; its MC_REGION == azer1ref
  # AE=0 -- proving the M1 central chrome is map-group-invariant).  SAUDI1/SYRIA1 share map D30 (identical
  # central chrome -> both use the genuine SAUDI1 ref); INDIA1 shares map D06 with CYPRUS1 (-> cyprus1ref).
  "mission-cockpit-saudi1|25000|missfb|SAUDI1|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-syria1|25000|missfb|SYRIA1|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-india1|25000|missfb|INDIA1|$ROOT/ref/mission_cyprus1_cockpit_native320.png"
  # mission-cockpit-{saudi7,syria3,india4,cyprus7,azer7}: same-map ref-reuse expansion (NO new capture).
  # Swept all *2-7 battles for a crash-free cockpit spawn with central-chrome (MC_REGION) AE=0 on BOTH
  # targets vs an EXISTING genuine DOSBox ref; these 5 pass (rc=0, AE=0 native+wasm).  The M1/idx0 central
  # chrome is map-group-invariant modulo a spawn-frame dynamic-instrument residual, so each pins the ref its
  # crop matches AE=0: SAUDI7->saudi1ref (==azer1 crop); SYRIA3/INDIA4/CYPRUS7/AZER7->cyprus1ref.  NB AZER7
  # renders the cockpit AE=0 -> twin-#3 (map-view instead of cockpit) is mission-specific, NOT blanket-D31.
  "mission-cockpit-saudi7|25000|missfb|SAUDI7|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-syria3|25000|missfb|SYRIA3|$ROOT/ref/mission_cyprus1_cockpit_native320.png"
  "mission-cockpit-india4|25000|missfb|INDIA4|$ROOT/ref/mission_cyprus1_cockpit_native320.png"
  "mission-cockpit-cyprus7|25000|missfb|CYPRUS7|$ROOT/ref/mission_cyprus1_cockpit_native320.png"
  "mission-cockpit-azer7|25000|missfb|AZER7|$ROOT/ref/mission_cyprus1_cockpit_native320.png"
  # mission-cockpit-azer2: TWIN #3 FIXED (patch 392).  AZER2 (D31) previously HUNG rendering the MAP view
  # (chain-B phase-table overrun) because 5d43's player-vehicle gate `3 < *puVeh` read 4 bytes (Ghidra uint*)
  # instead of the asm's WORD compare -- AZER2's vehicle word[+2]=0x0008 poisoned it -> the cockpit setup
  # (44be) was skipped -> map view.  The store-width fix routes AZER2 to the cockpit.  Central-chrome AE=0
  # both targets vs a GENUINE DOSBox AZER2 spawn ref; native<->wasm 0-diff.  Unblocks the ~18-mission HANG
  # bucket root (other HANG missions now reach the cockpit path; they hit their own downstream twins next).
  "mission-cockpit-azer2|25000|missfb|AZER2|$ROOT/ref/mission_azer2_cockpit_native320.png"
  # ex-HANG-bucket missions UNBLOCKED by the twin-#3 fix (patch 392): now render the cockpit crash-free,
  # native<->wasm 0-diff, central-chrome AE=0 vs an existing ref (no new capture).  Swept post-392.
  "mission-cockpit-saudi2|25000|missfb|SAUDI2|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-saudi3|25000|missfb|SAUDI3|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-syria6|25000|missfb|SYRIA6|$ROOT/ref/mission_cyprus1_cockpit_native320.png"
  "mission-cockpit-cyprus2|25000|missfb|CYPRUS2|$ROOT/ref/mission_cyprus1_cockpit_native320.png"
  "mission-cockpit-cyprus6|25000|missfb|CYPRUS6|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-train2|25000|missfb|TRAIN2|$ROOT/ref/mission_cyprus1_cockpit_native320.png"
  "mission-cockpit-train4|25000|missfb|TRAIN4|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-azer4|25000|missfb|AZER4|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-azer5|25000|missfb|AZER5|$ROOT/ref/mission_cyprus1_cockpit_native320.png"
  # INDIA6: genuine DOSBox ref (own capture, tools/refcapture_mission_scroll.sh idx19); central chrome
  # AE=0 both targets + native<->wasm 0-diff.  The other 8 NEEDS-REF captures (AZER6/CYPRUS5/INDIA7/
  # SAUDI5/6/SYRIA5/7/TRAIN1) are held back: their refs are genuine but expose a port-side central
  # heading/azimuth-dial divergence (grey dial + red needle ~(108-118,116-131)) -> a real bug to fix.
  "mission-cockpit-india6|25000|missfb|INDIA6|$ROOT/ref/mission_india6_cockpit_native320.png"
  # UKRAINE4 (map D-group, M1 idx0) + UKRAINE7 (D32/C32, M1 idx0): crash-free spawn, central-chrome
  # (MC_REGION) bit-identical to the existing M1 refs -- resting spawn turret (no dial divergence).
  # Same-map ref reuse: UKRAINE4 matches the cyprus1 M1 crop AE=0; UKRAINE7 matches the azer1 M1 crop AE=0.
  # Deterministic (native 2x AE=0), native<->wasm 0-diff.  Reached via FIST_FSG_BATTLE (patch 380).
  "mission-cockpit-ukraine4|25000|missfb|UKRAINE4|$ROOT/ref/mission_cyprus1_cockpit_native320.png"
  "mission-cockpit-ukraine7|25000|missfb|UKRAINE7|$ROOT/ref/mission_azer1_cockpit_native320.png"
  # UKRAINE8 (D31/C31-group, M1 idx0): crash-free spawn, central chrome == azer1 M1 crop AE=0; resting
  # turret.  Deterministic (native 2x AE=0), wasm AE=0, native<->wasm 0-diff.
  "mission-cockpit-ukraine8|25000|missfb|UKRAINE8|$ROOT/ref/mission_azer1_cockpit_native320.png"
  # AZER3: the FIRST un-blocked op-0x2c crash-bucket battle (patches 397-399 ported the 4 per-vehicle
  # sprite-animation element methods that SEGV'd the mission paint).  AZER3 renders the windshield only
  # via the op-0x2c secondary-viewport path (never posts op-0x24), so it uses the `mission-cockpit-2c-`
  # capture mode (FIST_MISSFB2C, first op-0x2c post = spawn).  Central-chrome (MC_REGION) AE=0 on native
  # AND wasm, native<->wasm 0-diff; the ref crop is bit-identical to the DOSBox oracle spawn frame
  # (scratch/oracle/azer3_spawn.frame.png).  Windshield terrain band = the separate voxel-render frontier.
  "mission-cockpit-2c-azer3|25000|missfb|AZER3|$ROOT/ref/mission_azer3_cockpit_native320.png"
  # 11 more op-0x2c battles, central-chrome AE=0 vs an existing M1 cockpit ref on both targets +
  # native<->wasm 0-diff, deterministic (native 2x AE=0).  These share the M1-ref-reuse standard of the
  # op-0x24 mission-cockpit block above.  [Provenance note: these were briefly reverted after a stock-
  # DOSBox oracle capture appeared to show AZER6's player as a non-M1 steering-wheel vehicle -- but that
  # was RETRACTED: a definitive AZER6 capture (selection zoom-verified) settles to central-chrome AE=2 vs
  # the port's M1 at ACCEPT+31s, i.e. AZER6 IS M1 and the port is faithful; the steering-wheel frame was
  # the original's non-deterministic AUTO-CYCLE view (it cycles to other units after a variable idle),
  # NOT the spawn.  So there is no vehicle-selection bug.  Per-battle oracle refs still want capturing --
  # the oracle menu nav is currently non-deterministic (row-select + cycle timing) -- but the M1-ref
  # match is consistent with all reliable evidence (AZER1/AZER3/AZER6 all M1).]
  "mission-cockpit-2c-cyprus3|25000|missfb|CYPRUS3|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-india2|25000|missfb|INDIA2|$ROOT/ref/mission_azer3_cockpit_native320.png"
  "mission-cockpit-2c-india5|25000|missfb|INDIA5|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-india7|25000|missfb|INDIA7|$ROOT/ref/mission_azer2_cockpit_native320.png"
  "mission-cockpit-2c-saudi4|25000|missfb|SAUDI4|$ROOT/ref/mission_azer3_cockpit_native320.png"
  "mission-cockpit-2c-syria2|25000|missfb|SYRIA2|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-syria4|25000|missfb|SYRIA4|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-train3|25000|missfb|TRAIN3|$ROOT/ref/mission_azer3_cockpit_native320.png"
  "mission-cockpit-2c-ukraine1|25000|missfb|UKRAINE1|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-ukraine2|25000|missfb|UKRAINE2|$ROOT/ref/mission_azer3_cockpit_native320.png"
  "mission-cockpit-2c-ukraine5|25000|missfb|UKRAINE5|$ROOT/ref/mission_azer3_cockpit_native320.png"
  # op-0x2c cockpit coverage extended to every remaining CRASH-FREE FSG battle whose M1 central chrome
  # is bit-identical to the shared DOSBox spawn ref (saudi1) -- AE=0 native, AE=0 wasm, native==wasm.
  # Found by a full 35-battle op-0x2c spawn scan: INDIA3 is the ONLY crashing battle (near-heap OOM,
  # patch-404 frontier); every other battle renders, and these 9 match the saudi1 central chrome exactly.
  "mission-cockpit-2c-azer4|25000|missfb|AZER4|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-cyprus6|25000|missfb|CYPRUS6|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-saudi2|25000|missfb|SAUDI2|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-saudi3|25000|missfb|SAUDI3|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-saudi7|25000|missfb|SAUDI7|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-syria1|25000|missfb|SYRIA1|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-train4|25000|missfb|TRAIN4|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-ukraine7|25000|missfb|UKRAINE7|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  "mission-cockpit-2c-ukraine8|25000|missfb|UKRAINE8|$ROOT/ref/mission_saudi1_cockpit_native320.png"
  # ---- remaining battles: cockpit spawn native<->wasm bit-identity (no DOSBox ref yet -> native==wasm
  # only, the hard invariant).  Completes the 47-mission cockpit set except INDIA3 (board:0006 spawn OOM).
  "mission-cockpit-azer6|25000|missfb|AZER6|"
  "mission-cockpit-cyprus5|25000|missfb|CYPRUS5|"
  "mission-cockpit-saudi5|25000|missfb|SAUDI5|"
  "mission-cockpit-saudi6|25000|missfb|SAUDI6|"
  "mission-cockpit-syria5|25000|missfb|SYRIA5|"
  "mission-cockpit-syria7|25000|missfb|SYRIA7|"
  "mission-cockpit-train1|25000|missfb|TRAIN1|"
  "mission-cockpit-ukraine3|25000|missfb|UKRAINE3|"
  "mission-cockpit-2c-cyprus4|25000|missfb|CYPRUS4|"
  "mission-cockpit-2c-ukraine6|25000|missfb|UKRAINE6|"
  # ---- IN-MISSION VOXEL TERRAIN render (FIST_TERRAIN): full-framebuffer native<->wasm bit-identity ----
  # board:0002 -- extends the matrix with terrain coverage (689a+6980 voxel raycaster).  native==wasm
  # confirmed 0-diff on the full 320x200 frame; the oracle-fidelity (Stage-2 vs the live-paging capture)
  # is tracked separately (the ~0.36% CM input-provenance residual, bc9c/ac70 asm-faithful).
  "terrain-azer1|25000|terrain|AZER1|"
  "terrain-saudi1|25000|terrain|SAUDI1|"
  "terrain-cyprus1|25000|terrain|CYPRUS1|"
  "terrain-india1|25000|terrain|INDIA1|"
  "terrain-syria1|25000|terrain|SYRIA1|"
  # ---- OPL FM AUDIO stream native<->wasm bit-identity (board:0003) ----
  # The OPL FM audio is native==wasm bit-identical over the deterministic OPL-init window ([0x452]=120,
  # pure-coop); full-duration music audio (across the intro->menu [0x452]-reset transition) awaits the
  # transition-onset determinism fix.  This lands the audio native==wasm invariant for that window.
  "audio-opl-init|1000|audio||"
  # board:0003 (patch 411): OPL FM audio ACROSS the intro->menu transition -- both targets now PLAY the
  # title intro identically, so the stream stays native==wasm bit-identical past the [0x452]=188 transition.
  # Pinned at [0x452]=4000 (~64s: full intro + menu music); proven diff=0 (5696650 B both), the max window before a 2nd transition-onset divergence at [0x452]~8000.
  "audio-intro|1000|audio:4000||"
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
  # The 3rd field is FIST_RUNMS (wall-clock dump) unless it is tick=N (FIST_DUMPTICK, engine-frame-timer
  # phase pin) or kdvframe=N (the intro FMV: drive the KDV player and dump the Nth decoded frame -- see the
  # intro flow).  kdvframe pins by FRAME INDEX (native<->wasm identical, timing-independent; the decode-ahead
  # seam in native_main.c reaches frame N with no throttle wait).
  local dumpenv=(); case "$ms" in
    kdvframe=*) dumpenv=(FIST_KDV=1 FIST_KDV_DUMPFRAME="${ms#kdvframe=}");;
    tick=*) dumpenv=(FIST_DUMPTICK="${ms#tick=}");;
    *) dumpenv=(FIST_RUNMS="$ms");;
  esac
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

# ---- EDITOR .FSG load->save ROUND-TRIP (file-level fixed-point, not framebuffer) ----
# BATTLES(160,100) -> OK(205,128) -> ACCEPT(40,186) runs the cascade; the patch-361 hook (env
# FIST_FSG_ROUNDTRIP) re-saves the loaded .FSG via the reconstructed serializer FUN_0000_d5f9 (patch
# 360) right after d501 (pre-sim, deterministic) and exits.  The DoD round-trip is a FIXED POINT
# (load canonicalizes each unit, so the editor-authored file is not itself a fixed point, but a
# re-saved file IS): load(orig)->save = file1; load(file1)->save = file2; assert file1==file2.
RT_MOUSE="200:160:100:0; 800:160:100:1; 1400:160:100:0; 3000:205:128:0; 3600:205:128:1; 4200:205:128:0; 5400:40:186:0; 6000:40:186:1; 6600:40:186:0; 7200:40:186:0"
run_fsg() { # $1=target $2=datadir $3=battle(optional,default AZER1) ; echo rc (one load->save->exit)
  local t="$1" dd="$2" b="${3:-AZER1}"
  # FIST_FSG_BATTLE (patch 380) overrides the loaded battle filename so ANY .FSG can be exercised with
  # the single RT_MOUSE navigation (BATTLES->OK->ACCEPT); it selects AZER1 by default, so passing
  # b=AZER1 is behaviour-identical to the un-generalized flow.  Env mirrored to wasm by wasm_pre.js.
  if [ "$t" = native ]; then
    timeout 120 env FIST_DATADIR="$dd" FIST_TICK_HZ=25000 FIST_RUNMS=30000 FIST_FSG_ROUNDTRIP=1 FIST_FSG_BATTLE="$b" FIST_MOUSE="$RT_MOUSE" "$NATIVE" >/dev/null 2>&1; echo $?
  else
    timeout 150 env FIST_DATADIR="$dd" FIST_TICK_HZ=25000 FIST_RUNMS=30000 FIST_FSG_ROUNDTRIP=1 FIST_FSG_BATTLE="$b" FIST_MOUSE="$RT_MOUSE" "$NODE" "$OUTJS" >/dev/null 2>&1; echo $?
  fi
}
run_roundtrip() { # $1=target $2=battle(default AZER1) ; echo path-to-file1 on success, empty on failure
  local t="$1" b="${2:-AZER1}" a bd f1 f2
  a="$(fresh_datadir "rt.$b.$t.A")"; [ "$(run_fsg "$t" "$a" "$b")" = 0 ] || { echo ""; return 1; }
  f1="$TMP/file1.$b.$t.FSG"; [ -s "$a/FISTDATA/$b.FSG" ] && cp "$a/FISTDATA/$b.FSG" "$f1" || { echo ""; return 1; }
  bd="$(fresh_datadir "rt.$b.$t.B")"; cp "$f1" "$bd/FISTDATA/$b.FSG"
  [ "$(run_fsg "$t" "$bd" "$b")" = 0 ] || { echo ""; return 1; }
  f2="$TMP/file2.$b.$t.FSG"; cp "$bd/FISTDATA/$b.FSG" "$f2" 2>/dev/null || { echo ""; return 1; }
  cmp -s "$f1" "$f2" || { echo ""; return 2; }   # not a fixed point
  echo "$f1"
}

# ---- EDITOR ADD-TANK edit-op (file-level: well-formed unit delta + idempotent fixed point) ----
# Same cascade + save-point as the round-trip, but the patch-362 hook (env FIST_EDIT_ADDTANK) enters
# EDITING mode and ADDS ONE tank through the engine's REAL roster allocator (b21d) + unit registry
# (0x9fbc) -- cloning an existing loaded friendly (tank) record -- posts "TANK ADDED" (66cd 0xd27),
# then saves via d5f9 and exits.  The edited .FSG has ONE MORE DCBS unit than the round-trip save
# (80 -> 81), it RELOADS (the added tank is valid), and load->save on the edited file is IDEMPOTENT
# (a FIXED POINT, exactly like the round-trip: the raw edit output canonicalizes, then re-saves
# byte-identically).  Verified native AND wasm, native==wasm 0-diff.  DoD option (b) (see docs/editor.md).
dcbs_units() { # $1=file ; echo unit-count (first word of the DCBS chunk body) or empty
  python3 - "$1" <<'PY' 2>/dev/null
import sys,struct
d=open(sys.argv[1],'rb').read(); i=0
while i+6<=len(d):
    t=d[i:i+4]; ln=struct.unpack('<H',d[i+4:i+6])[0]
    if t==b'DCBS': print(struct.unpack('<H',d[i+6:i+8])[0]); break
    i+=6+ln
PY
}
run_edit() { # $1=target $2=datadir $3=battle(default AZER1) $4=edit-env(default FIST_EDIT_ADDTANK) ; echo rc
  # FIST_FSG_BATTLE (patch 380) selects the loaded+saved battle; the patch-362 add-tank / patch-405
  # remove-tank hooks are battle-agnostic (they act on the first loaded FRIENDLY unit), so ANY .FSG with
  # a friendly unit gains/loses exactly one tank.  Default AZER1 + add-tank -> the original flow.
  local t="$1" dd="$2" b="${3:-AZER1}" ev="${4:-FIST_EDIT_ADDTANK}"
  if [ "$t" = native ]; then
    timeout 120 env FIST_DATADIR="$dd" FIST_TICK_HZ=25000 FIST_RUNMS=30000 "$ev"=1 FIST_FSG_BATTLE="$b" FIST_MOUSE="$RT_MOUSE" "$NATIVE" >/dev/null 2>&1; echo $?
  else
    timeout 150 env FIST_DATADIR="$dd" FIST_TICK_HZ=25000 FIST_RUNMS=30000 "$ev"=1 FIST_FSG_BATTLE="$b" FIST_MOUSE="$RT_MOUSE" "$NODE" "$OUTJS" >/dev/null 2>&1; echo $?
  fi
}
run_addtank() { # $1=target $2=battle(default AZER1) $3=edit-env(default FIST_EDIT_ADDTANK) ; echo edited-file or ""
  # Battle-agnostic: the baseline unit-count is read from the freshly-copied original $b.FSG, and the
  # add op (add-tank friendly, or add-enemy via $3=FIST_EDIT_ADDENEMY) must yield exactly baseline+1.
  # Then the edited file is a reload-stable idempotent fixed point, keeping the added unit.
  local t="$1" b="${2:-AZER1}" ev="${3:-FIST_EDIT_ADDTANK}" a bd c fe fe2 fe3 base want
  a="$(fresh_datadir "at.$b.$t.A")"
  base="$(dcbs_units "$a/FISTDATA/$b.FSG")"; [ -n "$base" ] || { echo ""; return 3; }
  want=$((base+1))
  [ "$(run_edit "$t" "$a" "$b" "$ev")" = 0 ] || { echo ""; return 1; }
  fe="$TMP/edit.$b.$t.FSG"; [ -s "$a/FISTDATA/$b.FSG" ] && cp "$a/FISTDATA/$b.FSG" "$fe" || { echo ""; return 1; }
  [ "$(dcbs_units "$fe")" = "$want" ] || { echo ""; return 3; }                   # exactly base+1 units
  # idempotent fixed point on the edited file: load(fe)->save = fe2; load(fe2)->save = fe3; fe2==fe3
  bd="$(fresh_datadir "at.$b.$t.B")"; cp "$fe" "$bd/FISTDATA/$b.FSG"
  [ "$(run_fsg "$t" "$bd" "$b")" = 0 ] || { echo ""; return 1; }
  fe2="$TMP/editfp1.$b.$t.FSG"; cp "$bd/FISTDATA/$b.FSG" "$fe2" 2>/dev/null || { echo ""; return 1; }
  [ "$(dcbs_units "$fe2")" = "$want" ] || { echo ""; return 3; }                  # reload keeps the tank
  c="$(fresh_datadir "at.$b.$t.C")"; cp "$fe2" "$c/FISTDATA/$b.FSG"
  [ "$(run_fsg "$t" "$c" "$b")" = 0 ] || { echo ""; return 1; }
  fe3="$TMP/editfp2.$b.$t.FSG"; cp "$c/FISTDATA/$b.FSG" "$fe3" 2>/dev/null || { echo ""; return 1; }
  cmp -s "$fe2" "$fe3" || { echo ""; return 2; }   # edited file is not an idempotent fixed point
  echo "$fe"
}
run_remtank() { # $1=target $2=battle(default AZER1) ; echo path-to-edited-file on success, empty on failure
  # Inverse of run_addtank (patch 405 FIST_EDIT_REMTANK, driving the REAL object-destroy b2ef): remove one
  # friendly unit; the baseline is read from the freshly-copied original $b.FSG and REMOVE-TANK must yield
  # exactly baseline-1, keep base-1 across a reload, and be an idempotent fixed point.
  local t="$1" b="${2:-AZER1}" a bd c fe fe2 fe3 base want
  a="$(fresh_datadir "rt.$b.$t.A")"
  base="$(dcbs_units "$a/FISTDATA/$b.FSG")"; [ -n "$base" ] || { echo ""; return 3; }
  want=$((base-1))
  [ "$(run_edit "$t" "$a" "$b" FIST_EDIT_REMTANK)" = 0 ] || { echo ""; return 1; }
  fe="$TMP/rem.$b.$t.FSG"; [ -s "$a/FISTDATA/$b.FSG" ] && cp "$a/FISTDATA/$b.FSG" "$fe" || { echo ""; return 1; }
  [ "$(dcbs_units "$fe")" = "$want" ] || { echo ""; return 3; }                   # exactly base-1 units
  bd="$(fresh_datadir "rt.$b.$t.B")"; cp "$fe" "$bd/FISTDATA/$b.FSG"
  [ "$(run_fsg "$t" "$bd" "$b")" = 0 ] || { echo ""; return 1; }
  fe2="$TMP/remfp1.$b.$t.FSG"; cp "$bd/FISTDATA/$b.FSG" "$fe2" 2>/dev/null || { echo ""; return 1; }
  [ "$(dcbs_units "$fe2")" = "$want" ] || { echo ""; return 3; }                  # reload keeps the removal
  c="$(fresh_datadir "rt.$b.$t.C")"; cp "$fe2" "$c/FISTDATA/$b.FSG"
  [ "$(run_fsg "$t" "$c" "$b")" = 0 ] || { echo ""; return 1; }
  fe3="$TMP/remfp2.$b.$t.FSG"; cp "$c/FISTDATA/$b.FSG" "$fe3" 2>/dev/null || { echo ""; return 1; }
  cmp -s "$fe2" "$fe3" || { echo ""; return 2; }   # edited file is not an idempotent fixed point
  echo "$fe"
}

# ---- MISSION cockpit render (FIST_MISSFB op-0x24 post #1) -> region-cropped PPM ----
MC_MOUSE="200:160:100:0; 800:160:100:1; 1400:160:100:0; 3000:205:128:0; 3600:205:128:1; 4200:205:128:0; 5400:40:186:0; 6000:40:186:1; 6600:40:186:0; 7200:40:186:0"
MC_REGION="100x92+80+96"   # cols80-180 rows96-188 central chrome
run_mission() { # $1=target $2=datadir [$3=battle] [$4=mode: ""=op-0x24 spawn, "2c"=op-0x2c spawn] ; echo crop-ppm or ""
  local t="$1" dd="$2" bt="${3:-}" mode="${4:-}"
  local out="$TMP/mc.$t.ppm" reg="$TMP/mc.$t.reg.ppm"
  # $3 selects a non-default battle via patch 380's FIST_FSG_BATTLE (drives the same MC_MOUSE
  # BATTLES->OK->ACCEPT navigation into any of the 47 .FSG).  Empty -> default AZER1 (behaviour-neutral).
  local bexp=(); [ -n "$bt" ] && bexp=(FIST_FSG_BATTLE="$bt")
  # mode "2c": the FSG-battle SECONDARY-viewport render path (op-0x2c, engine-painted) for missions that
  # never post op-0x24 (e.g. AZER3, un-blocked by patches 397-399).  FIST_MISSFB2C captures 0xA0000 at the
  # first op-0x2c post (=spawn, cross-target deterministic like op-0x24 post #1).
  local m2c=(); [ "$mode" = 2c ] && m2c=(FIST_MISSFB2C=1 FIST_MISSFB_N=1)
  if [ "$t" = native ]; then
    timeout 90  env FIST_DATADIR="$dd" FIST_TICK_HZ=25000 "${bexp[@]}" "${m2c[@]}" FIST_MOUSE="$MC_MOUSE" FIST_MISSFB="$out" "$NATIVE" >/dev/null 2>&1
  else
    timeout 220 env FIST_DATADIR="$dd" FIST_TICK_HZ=25000 "${bexp[@]}" "${m2c[@]}" FIST_MOUSE="$MC_MOUSE" FIST_MISSFB="$out" "$NODE" "$OUTJS" >/dev/null 2>&1
  fi
  [ -s "$out" ] || { echo ""; return 1; }
  convert "$out" -crop "$MC_REGION" +repage "$reg" 2>/dev/null || { echo ""; return 1; }
  echo "$reg"
}
run_terrain() { # $1=target $2=battle ; echo full-framebuffer ppm or ""  -- FIST_TERRAIN voxel render
  # The in-mission VOXEL TERRAIN render (689a sky/tile + 6980 raycaster, env-gated via FIST_TERRAIN so it
  # runs on the map-load spawn state).  Asserts the FULL 320x200 framebuffer is native<->wasm bit-identical
  # -- terrain coverage for the hard invariant (board:0002).  READ-ONLY (no edit) -> use the repo datadir.
  local t="$1" bt="${2:-}"
  local out="$TMP/tr.$t.ppm"
  local bexp=(); [ -n "$bt" ] && bexp=(FIST_FSG_BATTLE="$bt")
  if [ "$t" = native ]; then
    timeout 90  env FIST_DATADIR="$ROOT/armoredfist" FIST_TICK_HZ=25000 FIST_TERRAIN=1 "${bexp[@]}" FIST_MOUSE="$MC_MOUSE" FIST_MISSFB="$out" "$NATIVE" >/dev/null 2>&1
  else
    timeout 220 env FIST_DATADIR="$ROOT/armoredfist" FIST_TICK_HZ=25000 FIST_TERRAIN=1 "${bexp[@]}" FIST_MOUSE="$MC_MOUSE" FIST_MISSFB="$out" "$NODE" "$OUTJS" >/dev/null 2>&1
  fi
  [ -s "$out" ] || { echo ""; return 1; }
  echo "$out"
}
run_audio() { # $1=target $2=dumptick(default 120) ; echo wav or ""  -- OPL FM audio, tick-pinned, native<->wasm
  # board:0003: the OPL FM audio stream is native==wasm bit-identical under pure-cooperative ticking
  # (FIST_COOP_TICK, FIST_TICK_HZ=1000 so div=250 is stable).  Since patch 411 (d97e returns e339's op-0x78
  # status) BOTH targets play the title intro identically, so the stream stays bit-identical ACROSS the
  # intro->menu transition -- audio-intro pins [0x452]=300 (past the transition); audio-opl-init pins the
  # [0x452]=120 OPL-init window.
  local t="$1" dt="${2:-120}"
  local out="$TMP/au.$t.$dt.wav"
  if [ "$t" = native ]; then
    timeout 90  env FIST_DATADIR="$ROOT/armoredfist" FIST_TICK_HZ=1000 FIST_DUMPTICK="$dt" FIST_COOP_TICK=1 FIST_OPL=1 FIST_SB=1 FIST_AUDIO_WAV="$out" FIST_FBDUMP="$TMP/au.$t.$dt.ppm" "$NATIVE" >/dev/null 2>&1
  else
    timeout 240 env FIST_DATADIR="$ROOT/armoredfist" FIST_TICK_HZ=1000 FIST_DUMPTICK="$dt" FIST_COOP_TICK=1 FIST_OPL=1 FIST_SB=1 FIST_AUDIO_WAV="$out" FIST_FBDUMP="$TMP/au.$t.$dt.ppm" "$NODE" "$OUTJS" >/dev/null 2>&1
  fi
  [ -s "$out" ] || { echo ""; return 1; }
  echo "$out"
}
run_editsim() { # $1=target $2=battle $3=edit-env(default FIST_EDIT_ADDTANK) ; echo cropped-ppm or ""
  # The editor DoD "simulate" leg: the edit op ($3: add-tank or remove-tank) edits $b.FSG in a datadir,
  # then that SAME edited battle is loaded into a mission and its op-0x2c spawn cockpit is captured.
  # Proves the editor's output is not just file-consistent but SIMULATABLE; the friendly add/remove does
  # not perturb the central chrome, so it stays AE=0 vs the shared ref, on native AND wasm.
  local t="$1" b="$2" ev="${3:-FIST_EDIT_ADDTANK}" dd
  dd="$(fresh_datadir "es.$b.$t")"
  [ "$(run_edit "$t" "$dd" "$b" "$ev")" = 0 ] || { echo ""; return 1; }   # edit into dd (saves $b.FSG)
  run_mission "$t" "$dd" "$b" "2c"                                        # op-0x2c spawn on the EDITED dd
}

echo "== verify ($WHICH) =="
for row in "${FLOWS[@]}"; do
  IFS='|' read -r name hz ms inp ref <<<"$row"
  ok=1; detail=""
  if [ "$name" = editor-fsg-roundtrip ] || [ "${name#editor-fsg-}" != "$name" ]; then
    # Editor .FSG load->save fixed-point round-trip for a named battle.  The battle name is in the
    # row's `inp` field (empty -> AZER1, the default list selection).  Patch 380's FIST_FSG_BATTLE
    # override lets the single RT_MOUSE navigation load any of the 47 battles.  Verified as a FIXED
    # POINT (load canonicalizes, so a re-saved file reloads+re-saves byte-identically) on native AND
    # wasm, native file1 == wasm file1 (0-diff).
    bt="${inp:-AZER1}"
    detail=" [$bt roundtrip fixed-point]"; f1n=""; f1w=""
    if [ "$WHICH" != wasm ];   then f1n="$(run_roundtrip native "$bt")"; [ -n "$f1n" ] || { ok=0; detail+=" native-fail"; }; fi
    if [ "$WHICH" != native ]; then f1w="$(run_roundtrip wasm "$bt")";   [ -n "$f1w" ] || { ok=0; detail+=" wasm-fail"; }; fi
    if [ "$WHICH" = both ] && [ -n "$f1n" ] && [ -n "$f1w" ]; then
      cmp -s "$f1n" "$f1w" || { ok=0; detail+=" nat!=wasm"; }
    fi
    if [ "$ok" = 1 ]; then echo "  PASS $name$detail"; pass=$((pass+1)); else echo "  FAIL $name$detail"; fail=$((fail+1)); fi
    continue
  fi
  if [ "$name" = editor-add-tank ] || [ "${name#editor-add-tank-}" != "$name" ]; then
    # ADD-TANK edit-op generalized over battles (patch 362 hook is battle-agnostic): the battle is the
    # `editor-add-tank-<BATTLE>` suffix (bare name -> AZER1).  run_addtank reads the ORIGINAL $b.FSG
    # baseline, asserts edit->save = base+1 units, the added tank RELOADS, and load->save on the edited
    # file is an idempotent FIXED POINT.  native==wasm 0-diff on the raw edit.
    bt="AZER1"; [ "${name#editor-add-tank-}" != "$name" ] && bt="$(echo "${name#editor-add-tank-}" | tr a-z A-Z)"
    detail=" [$bt add-tank base->base+1 + idempotent fixed-point]"; fen=""; few=""
    if [ "$WHICH" != wasm ];   then fen="$(run_addtank native "$bt")"; [ -n "$fen" ] || { ok=0; detail+=" native-fail"; }; fi
    if [ "$WHICH" != native ]; then few="$(run_addtank wasm "$bt")";   [ -n "$few" ] || { ok=0; detail+=" wasm-fail"; }; fi
    if [ "$WHICH" = both ] && [ -n "$fen" ] && [ -n "$few" ]; then
      cmp -s "$fen" "$few" || { ok=0; detail+=" nat!=wasm"; }
    fi
    if [ "$ok" = 1 ]; then echo "  PASS $name$detail"; pass=$((pass+1)); else echo "  FAIL $name$detail"; fail=$((fail+1)); fi
    continue
  fi
  if [ "$name" = editor-rem-tank ] || [ "${name#editor-rem-tank-}" != "$name" ]; then
    # REMOVE-TANK edit-op (patch 405, inverse of add-tank; drives the REAL b2ef object-destroy).  Battle
    # from the `editor-rem-tank-<BATTLE>` suffix (bare -> AZER1).  Asserts edit->save = base-1, the
    # removal RELOADS, and the edited file is an idempotent FIXED POINT.  native==wasm 0-diff.
    bt="AZER1"; [ "${name#editor-rem-tank-}" != "$name" ] && bt="$(echo "${name#editor-rem-tank-}" | tr a-z A-Z)"
    detail=" [$bt rem-tank base->base-1 + idempotent fixed-point]"; fen=""; few=""
    if [ "$WHICH" != wasm ];   then fen="$(run_remtank native "$bt")"; [ -n "$fen" ] || { ok=0; detail+=" native-fail"; }; fi
    if [ "$WHICH" != native ]; then few="$(run_remtank wasm "$bt")";   [ -n "$few" ] || { ok=0; detail+=" wasm-fail"; }; fi
    if [ "$WHICH" = both ] && [ -n "$fen" ] && [ -n "$few" ]; then
      cmp -s "$fen" "$few" || { ok=0; detail+=" nat!=wasm"; }
    fi
    if [ "$ok" = 1 ]; then echo "  PASS $name$detail"; pass=$((pass+1)); else echo "  FAIL $name$detail"; fail=$((fail+1)); fi
    continue
  fi
  if [ "$name" = editor-add-enemy ] || [ "${name#editor-add-enemy-}" != "$name" ]; then
    # ADD-ENEMY edit-op (patch 406): exercises b21d's ENEMY roster branch (0xc05c) via FIST_EDIT_ADDENEMY.
    # Same base->base+1 + idempotent-fixed-point contract as add-tank, cloning the first ENEMY unit.
    bt="AZER1"; [ "${name#editor-add-enemy-}" != "$name" ] && bt="$(echo "${name#editor-add-enemy-}" | tr a-z A-Z)"
    detail=" [$bt add-enemy base->base+1 + idempotent fixed-point]"; fen=""; few=""
    if [ "$WHICH" != wasm ];   then fen="$(run_addtank native "$bt" FIST_EDIT_ADDENEMY)"; [ -n "$fen" ] || { ok=0; detail+=" native-fail"; }; fi
    if [ "$WHICH" != native ]; then few="$(run_addtank wasm "$bt" FIST_EDIT_ADDENEMY)";   [ -n "$few" ] || { ok=0; detail+=" wasm-fail"; }; fi
    if [ "$WHICH" = both ] && [ -n "$fen" ] && [ -n "$few" ]; then
      cmp -s "$fen" "$few" || { ok=0; detail+=" nat!=wasm"; }
    fi
    if [ "$ok" = 1 ]; then echo "  PASS $name$detail"; pass=$((pass+1)); else echo "  FAIL $name$detail"; fail=$((fail+1)); fi
    continue
  fi
  if [ "${name#editor-sim-}" != "$name" ] || [ "${name#editor-remsim-}" != "$name" ]; then
    # EDITOR "simulate" leg: edit $b.FSG (add-tank, or remove-tank for editor-remsim-*), then load+spawn
    # that edited battle and compare its op-0x2c cockpit central-chrome AE=0 vs the shared DOSBox ref,
    # native AND wasm, native==wasm 0-diff.  Add/remove of a friendly is central-chrome-neutral.
    esev="FIST_EDIT_ADDTANK"; esb="${name#editor-sim-}"
    if [ "${name#editor-remsim-}" != "$name" ]; then esev="FIST_EDIT_REMTANK"; esb="${name#editor-remsim-}"; fi
    esb="$(echo "$esb" | tr a-z A-Z)"
    detail=" [$esb edit->simulate op-0x2c central-chrome]"
    if [ ! -s "$ROOT/re_out/fist_image.bin" ]; then echo "  FAIL $name (re_out/fist_image.bin missing)"; fail=$((fail+1)); continue; fi
    convert "$ref" -crop "$MC_REGION" +repage "$TMP/mc.ref.ppm" 2>/dev/null
    rn=""; rw=""
    if [ "$WHICH" != wasm ]; then
      rn="$(run_editsim native "$esb" "$esev")"
      if [ -n "$rn" ]; then a=$(compare -metric AE "$rn" "$TMP/mc.ref.ppm" /dev/null 2>&1); [ "$a" = 0 ] || { ok=0; detail+=" native-AE=$a"; }
      else ok=0; detail+=" native-no-frame"; fi
    fi
    if [ "$WHICH" != native ]; then
      rw="$(run_editsim wasm "$esb" "$esev")"
      if [ -n "$rw" ]; then a=$(compare -metric AE "$rw" "$TMP/mc.ref.ppm" /dev/null 2>&1); [ "$a" = 0 ] || { ok=0; detail+=" wasm-AE=$a"; }
      else ok=0; detail+=" wasm-no-frame"; fi
    fi
    if [ "$WHICH" = both ] && [ -n "$rn" ] && [ -n "$rw" ]; then
      d=$(compare -metric AE "$rn" "$rw" /dev/null 2>&1); [ "$d" = 0 ] || { ok=0; detail+=" nat!=wasm($d)"; }
    fi
    if [ "$ok" = 1 ]; then echo "  PASS $name$detail"; pass=$((pass+1)); else echo "  FAIL $name$detail"; fail=$((fail+1)); fi
    continue
  fi
  if [ "${name#audio-}" != "$name" ]; then
    # OPL FM audio stream native<->wasm bit-identity (tick-pinned).  board:0003.  The dumptick is carried
    # in the flow's 3rd field as "audio" (=120) or "audio:N" (=N).
    audt=120; case "$ms" in audio:*) audt="${ms#audio:}";; esac
    detail=" [OPL FM audio, coop tick-pinned [0x452]=$audt, full-WAV native==wasm]"
    rn=""; rw=""
    if [ "$WHICH" != wasm ];   then rn="$(run_audio native "$audt")"; [ -n "$rn" ] || { ok=0; detail+=" native-no-wav"; }; fi
    if [ "$WHICH" != native ]; then rw="$(run_audio wasm "$audt")";   [ -n "$rw" ] || { ok=0; detail+=" wasm-no-wav"; }; fi
    if [ "$WHICH" = both ] && [ -n "$rn" ] && [ -n "$rw" ]; then
      d=$(cmp -l "$rn" "$rw" 2>/dev/null | wc -l); [ "$d" = 0 ] || { ok=0; detail+=" nat!=wasm($d)"; }
    fi
    if [ "$ok" = 1 ]; then echo "  PASS $name$detail"; pass=$((pass+1)); else echo "  FAIL $name$detail"; fail=$((fail+1)); fi
    continue
  fi
  if [ "${name#terrain-}" != "$name" ]; then
    # IN-MISSION VOXEL TERRAIN render (FIST_TERRAIN): full-framebuffer native<->wasm bit-identity for a
    # named battle (row `inp`, empty -> AZER1).  board:0002 -- terrain coverage for the hard invariant.
    tbt="$inp"; detail=" [${tbt:-AZER1} FIST_TERRAIN voxel render, full-fb native==wasm]"
    rn=""; rw=""
    if [ "$WHICH" != wasm ];   then rn="$(run_terrain native "$tbt")"; [ -n "$rn" ] || { ok=0; detail+=" native-no-frame"; }; fi
    if [ "$WHICH" != native ]; then rw="$(run_terrain wasm "$tbt")";   [ -n "$rw" ] || { ok=0; detail+=" wasm-no-frame"; }; fi
    if [ "$WHICH" = both ] && [ -n "$rn" ] && [ -n "$rw" ]; then
      d=$(cmp -l "$rn" "$rw" 2>/dev/null | wc -l); [ "$d" = 0 ] || { ok=0; detail+=" nat!=wasm($d)"; }
    fi
    if [ "$ok" = 1 ]; then echo "  PASS $name$detail"; pass=$((pass+1)); else echo "  FAIL $name$detail"; fail=$((fail+1)); fi
    continue
  fi
  if [ "$name" = mission-cockpit ] || [ "${name#mission-cockpit-}" != "$name" ]; then
    # Dual-target IN-MISSION surface: <battle>-spawn cockpit central-chrome cols80-180 rows96-188.
    # Region-limited AE=0 vs the DOSBox ref on native AND wasm + native<->wasm 0-diff.  WRITE-isolated.
    # The battle is in the row's `inp` field (empty -> AZER1 default; e.g. CYPRUS1 via patch 380's
    # FIST_FSG_BATTLE).  The central chrome is map-invariant modulo dynamic instruments; the ref is a
    # genuine DOSBox spawn-frame capture whose MC_REGION crop is the compared surface.
    mbt="$inp"
    # op-0x2c capture mode for FSG battles that render only via the secondary viewport (name prefix
    # `mission-cockpit-2c-`, e.g. AZER3 un-blocked by patches 397-399).  Default "" = op-0x24 spawn.
    mmode=""; [ "${name#mission-cockpit-2c-}" != "$name" ] && mmode="2c"
    detail=" [${mbt:-AZER1} cockpit central-chrome cols80-180 rows96-188${mmode:+ op-0x2c}]"
    if [ ! -s "$ROOT/re_out/fist_image.bin" ]; then echo "  FAIL $name (re_out/fist_image.bin missing; run 'make kernel-image')"; fail=$((fail+1)); continue; fi
    # ref optional: with a DOSBox ref, assert MC_REGION AE=0 per target; without one, native==wasm only
    # (the hard invariant) -- lets the full 47-mission cockpit set land before every ref is captured.
    [ -n "$ref" ] && convert "$ref" -crop "$MC_REGION" +repage "$TMP/mc.ref.ppm" 2>/dev/null
    rn=""; rw=""
    if [ "$WHICH" != wasm ]; then
      rn="$(run_mission native "$(fresh_datadir "$name.nat")" "$mbt" "$mmode")"
      if [ -n "$rn" ]; then [ -n "$ref" ] && { a=$(compare -metric AE "$rn" "$TMP/mc.ref.ppm" /dev/null 2>&1); [ "$a" = 0 ] || { ok=0; detail+=" native-AE=$a"; }; }
      else ok=0; detail+=" native-no-frame"; fi
    fi
    if [ "$WHICH" != native ]; then
      rw="$(run_mission wasm "$(fresh_datadir "$name.wasm")" "$mbt" "$mmode")"
      if [ -n "$rw" ]; then [ -n "$ref" ] && { a=$(compare -metric AE "$rw" "$TMP/mc.ref.ppm" /dev/null 2>&1); [ "$a" = 0 ] || { ok=0; detail+=" wasm-AE=$a"; }; }
      else ok=0; detail+=" wasm-no-frame"; fi
    fi
    [ -z "$ref" ] && detail+=" [native==wasm only, no ref]"
    if [ "$WHICH" = both ] && [ -n "$rn" ] && [ -n "$rw" ]; then
      d=$(compare -metric AE "$rn" "$rw" /dev/null 2>&1); [ "$d" = 0 ] || { ok=0; detail+=" nat!=wasm($d)"; }
    fi
    if [ "$ok" = 1 ]; then echo "  PASS $name$detail"; pass=$((pass+1)); else echo "  FAIL $name$detail"; fail=$((fail+1)); fi
    continue
  fi
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
