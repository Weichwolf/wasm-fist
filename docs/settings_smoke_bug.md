# SETTINGS: SMOKE EFFECTS toggle -- LED-render bug (found 2026-08-10)

Attempting to add a `settings-smoke-off` verify flow (DoD "every setting") surfaced a REAL localized
front-end render bug -- SMOKE EFFECTS is the first NON-radio (checkbox) DISPLAY toggle tried; the radio
toggles (detail-low/med, sound-fx-med, music-off, sky) are all AE=0.

## Repro / evidence
- Oracle: `tools/refcapture_click2.sh 160 126 181 83 40 8 8 ref/pending/settings_smoke_off_native320.png`
  (open SETTINGS at 160,126 -> click SMOKE EFFECTS LED at 181,83). Deterministic (2 captures AE=0).
- Port flow: FIST_MOUSE "200:160:126:0;800:160:126:1;1400:160:126:0;3000:181:83:0;3600:181:83:1;4200:181:83:0;4800:181:83:0"
  FIST_RUNMS=22000 FIST_FBDUMP. native==wasm (0-diff, deterministic), but AE=35 vs oracle.
- The 35 differing px are a 7x7 cluster at x179-185 y79-85 = the SMOKE EFFECTS LED. Status line row
  (y186-198) is AE=0 -> the port DOES toggle the logic ("SMOKE EFFECTS DISABLED" renders correctly).
- Visual: port LED stays RED (on) after disable; DOSBox oracle LED goes DARK (off). So the port toggles
  the SMOKE state + status text but does NOT clear the LED indicator on disable.

## Root (to fix)
Find the SMOKE EFFECTS toggle handler (a DISPLAY checkbox, distinct from the detail/sound RADIO groups
whose LED render is already correct). The checkbox's LED-redraw on the OFF transition is missing/wrong
in the port -- likely a dropped arg / store-width / element-dirty in the checkbox paint method (the LED
element at DGROUP rect for SMOKE, ~(181,83)). Compare to the working radio LED paint (detail/sound).
grep the SETTINGS build for the SMOKE element + its 6ef5-class config->state store (patch 134 fixed
SKY/SMOKE flags as BYTE stores typed WORD -- the SMOKE flag store may be right but the LED PAINT reads
it wrong, or the LED element isn't marked dirty on toggle). Once fixed: AE=0 -> add settings-smoke-off
to verify.sh (ref/pending/settings_smoke_off_native320.png -> ref/). Same likely applies to AUTO TURRET
CONTROL / PROMPTS (other checkboxes). The radio toggles are unaffected.

## Other uncovered SETTINGS toggles (candidate flows once render is correct)
MUSIC ON (re-enable), SOUND FX OFF/HIGH, the 6 non-default joystick types (STD/FLIGHTSTICK/TM-FCS/CH/
TM-WCS/EXTERNAL DRIVER), CALIBRATE, AUTO TURRET CONTROL, PROMPTS. Each: refcapture_click2 + AE=0 both
targets. LED column x=181 (DISPLAY), joystick LEDs x~35 (CONTROL), SOUND LEDs x~246/differently.

## Deeper analysis (2026-08-10, cron iter): NOT the toggle -- it's the LED re-paint dispatch
asm-verified the full chain:
- Paint FUN_0000_6d45 (asm 0x6d45): `cmpb $0,[0x8b4f]; je +2; mov al,0xe; call 6d54` -> draws checked
  sprite 0xf0 if byte[0x8b4f]!=0 else empty 0xe8. Reads byte[0x8b4f]. CONFIRMED.
- Toggle FUN_0000_6c1d (asm 0x6c1d, PRISTINE re_out:17406): `xorb $1,[0x8b4f]; movb $3,[0x8bb5];
  si=0x179e; cmpb $0,[0x8b4f]; je +; si=0x1788; call 7018(post status)`. Flips byte[0x8b4f], sets
  redraw marker byte[0x8bb5]=3, posts ENABLED(0x1788)/DISABLED(0x179e) by NEW state.
- SKY toggle 6c02 is the IDENTICAL structure (flag 0x8b4e, marker 0x8bca) and WORKS (settings-sky AE=0).
- Port behaviour: status shows "SMOKE EFFECTS DISABLED" -> 6c1d ran, `if(word[0x8b4f]!=0)` was FALSE ->
  byte[0x8b4f]==0 AND byte[0x8b50]==0 (the WORD read didn't misfire). So the flag IS flipped to 0.
- YET the LED stays red (checked). So 6d45 was NOT re-dispatched after the toggle (it still shows the
  init/checked render). SKY's 6d36 IS re-dispatched on SKY toggle. => the bug is the ELEMENT RE-PAINT
  DISPATCH for the SMOKE LED element specifically, NOT the toggle var width (byte==word here: high bytes
  0x8b50/0x8bb6 are unreferenced, so patch-134-style byte retyping would NOT fix it -- verified by reason).
- The store-width theory (first hypothesis) is DISPROVEN. This is a paint-walk (209e) dispatch issue:
  the SMOKE LED element's dirty flag (byte[0x8bb5]=3) does not cause 6d45 to re-run, while SKY's
  (byte[0x8bca]=3) causes 6d36 to re-run. NEXT: instrument whether 6d45 is called post-toggle + how the
  209e walk maps the marker byte[0x8bXX] to the element paint method; SMOKE's element may be registered/
  dirtied differently than SKY's. Deeper than a quick front-end fix -- comparable to the mission-render
  dispatch work. Oracle stays in ref/pending/.

## Final precise root (2026-08-10): settings element re-paint dispatch, not the markers
The redraw markers byte[0x8bb5] (SMOKE) / byte[0x8bca] (SKY) written =3 by the toggles are READ NOWHERE
in the image (objdump scan) -> they are config-CHANGED markers (for the ACCEPT-time save), NOT paint
dirty flags. So the LED re-paint is driven by the element dispatch, not the markers. Confirmed the exact
symptom: after SMOKE toggle byte[0x8b4f]==0 (status "DISABLED" proves the `if(word[0x8b4f]!=0)` was false),
so 6d45 -- IF re-run -- would read 0 and draw the empty box 0xe8; but the LED stays checked (0xf0) -> 6d45
is NOT re-dispatched. SKY's 6d36 IS re-dispatched on SKY toggle (settings-sky AE=0). => the gap is the
settings-screen element re-paint dispatch for the SMOKE checkbox element specifically ({activate 6c1d,
paint 6d45} vs SKY {6c02, 6d36}). This is the settings display-list element layer (patches 132 paint /
318-320 toggle-activate). NEXT: compare how the SMOKE vs SKY DISPLAY checkbox elements are registered +
marked dirty on activate in that build (element method-vector wiring near the SETTINGS template build);
the SMOKE element likely isn't re-painted post-activate while SKY is. Deep (element-dispatch), not a quick
front-end fix. Oracle stays ref/pending/settings_smoke_off_native320.png.

## Session conclusion on tractability
Every frontier investigated this session (missions AZER2/UKRAINE1 native mga, INDIA1/SAUDI1 wasm op-0x1c
spin, plant-tree save-serializer, SMOKE settings checkbox) resolves to a genuine deep root requiring
focused multi-step RE -- there is no remaining "quick win". All are precisely mapped + banked (roots,
diagnostics, oracles, disproven hypotheses) so the eventual landings are fast. The port is a multi-session
undertaking; the hourly cron restarts at these banked, routed fix points.

## RESOLVED (patch 387, commit 49ee2b2) + PRODUCTIVE VEIN
SMOKE was the SKIPPED entry in the patch-319 store-width sweep: dirty flag 0x8bb5 (between the 319-fixed
0x8bb2 and 0x8bbb) + state flag 0x8b4f were left WORD-typed. Retyping both to undefined1 (asm-faithful
byte ops @6c1d/6c22) -> the `=3` no longer clobbers the renderer-index byte 0x8bb6 -> the dirty-walk
dispatches the correct SMOKE renderer -> LED re-renders OFF. AE=0 both targets, 38/38, no regression.

VEIN: the SETTINGS checkboxes are the tractable coverage path (front-end, low-risk, refcapture_click2
oracle + FIST_RUNMS dump verify). Remaining uncovered toggles likely the SAME store-width class (check each
flag's #define type vs the asm at its toggle handler; retype WORD->BYTE where asm is byte): AUTO TURRET
CONTROL, PROMPTS (CONTROL checkboxes, LEDs x~35), MUSIC-ON (re-enable), SOUND FX OFF/HIGH (x~246), the 6
non-default joystick radios (STD/FLIGHTSTICK/TM-FCS/CH/TM-WCS/EXTERNAL, LEDs x~35). Each = capture oracle
(refcapture_click2 160 126 <tx> <ty> 40 8 8 out.png) -> add flow -> AE=0 both -> matrix grows by 1. This
is the fastest matrix-growth path while the deep mission/editor roots await focused sessions.

## AUTO TURRET CONTROL LED fixed (patch 388) -- flow BLOCKED on checkbox cursor-calibration
Same store-width class: AUTO TURRET toggle FUN_0000_6be7 (asm `xorb [0x8b4d]`, `movb [0x8bbe],3`) had
state 0x8b4d (DAT_2000_4b4d) + dirty 0x8bbe (uRam00024bbe) left WORD-typed (the 318/319/387 sweep skipped
them). Retyped both to undefined1 (patch 388) -> the AUTO TURRET LED now re-renders on toggle (region diff
58->9, the box is now correctly empty, visually confirmed; native==wasm; verify.sh both = 38/38, zero
regression -> 388 is behaviour-neutral). Patch 388 KEPT (fixes a real observable bug per the doctrine).
BUT the settings-autoturret-off FLOW cannot be added at AE=0: an irreducible ~9px CURSOR-render residual
remains at the AUTO TURRET row (y~143) -- deterministic (oracle o1==o2, port stable 32px; best port pos
35,142->9px; button-center 90,143->34px; no click position gives 0). Other checkbox/radio flows align at
AE=0 (music-off y136, sound-fx-med y150, joystick 35,34) but y~143 at x35/90 does NOT -> a genuine
position-dependent cursor-sprite divergence between the port's integer-positioned INT-33h cursor and
DOSBox's mouse-scaled cursor. This BLOCKS the whole settings-checkbox vein's clean AE=0 flows.
NEXT (focused): solve the checkbox cursor-calibration generally -- either (a) park the cursor after the
toggle at a position proven to align (capture variant with a 3rd cursor-move; both oracle+port), or (b)
find why the port cursor at y~143/x35 renders !=DOSBox (cursor hotspot/clip at the left CONTROL panel).
Once solved: add settings-autoturret-off (oracle in ref/pending/) + PROMPTS + MUSIC-ON + SOUND-FX OFF/HIGH
+ 6 joystick types -- the vein opens. Patch 388's LED fix is the engine half; the flow is the harness half.

## Correction (2026-08-10): patch 388 LED fix is sound; residual is CURSOR/HARNESS, not the LED
Ground-truth region view (cursor parked): the AUTO TURRET LED renders IDENTICALLY in port and DOSBox
oracle (both the red-cross indicator). Patch 388 is correct (asm-faithful byte retype of 0x8b4d/0x8bbe,
class-identical to 387/319/318; it reduced the toggle-frame diff 58->32, so it DID change/fix the LED
render; no regression, 38/38). The remaining AE (~32-43) is the CURSOR: (a) DOSBox's mouse-scaled cursor
position != the port's direct FIST_MOUSE set (position-dependent 1px divergence -- the working flows align
by luck of position, AUTO TURRET's y~143 doesn't), and (b) an attempted cursor-PARK (move to 155,105 after
toggle) did NOT take effect in the port frame (FIST_MOUSE park steps 5000-6200 not reached within
FIST_RUNMS=22000, cursor stayed at the LED) -> the parked port had cursor-at-LED vs oracle-parked-away.
The AUTO TURRET flow is BLOCKED on a HARNESS issue (cursor park timing + DOSBox cursor-scaling), NOT an
engine bug. To land it + the checkbox vein: fix the port cursor-park (extend RUNMS / lower park pump
numbers so the cursor reaches the park spot) + use a REGION-cropped compare (like mission-cockpit) that
includes the LED and excludes the parked cursor. Deferred as a focused harness task. Patch 388's engine
fix stands. NB: over-invested this iteration -- the SMOKE flow (387) is the banked matrix growth; AUTO
TURRET's is harness-blocked.

## PRODUCTIVE METHOD ESTABLISHED (2026-08-10): aligned-column settings flows -> matrix 38->43
The clean, no-rabbit-hole path for settings coverage: pick a toggle/radio whose x-COLUMN aligns with
DOSBox's cursor scaling (x181 DISPLAY, x246 SOUND, x35 CONTROL/joystick at y<=88), capture oracle
(refcapture_click2 160 126 <x> <y> 40 8 8), render port (FIST_MOUSE open+toggle, FIST_RUNMS=22000 dump),
compare full-frame AE=0 -- most just work with NO engine change (the radio/checkbox render is already
correct; the only bugs were the SMOKE/AUTO-TURRET checkbox store-width flags 387/388). Batch-test native-AE
first (fast), then one full verify.sh both. LANDED this iteration: settings-sound-fx-off + settings-joy-
{flightstick,tmfcs,ch,tmwcs} = matrix 39->43, all AE=0 both targets, no patch.
REMAINING candidates: EXTERNAL DRIVER joystick (35,105 -- batch timed out, retest), NO JOYSTICK re-select
(35,23), SOUND FX HIGH (default, needs toggle-away-then-back), MUSIC ON (default). The CHECKBOXES SMOKE
(done 387) + AUTO TURRET (388 engine fix done, flow harness-blocked) + PROMPTS (likely same harness block
at its y) need the region-crop+cursor-park harness fix for a clean flow. So: aligned-column radios = free
matrix growth; misaligned-column checkboxes = need the harness fix. This is the fastest matrix-growth vein
while the deep mission/editor roots await focused sessions.
