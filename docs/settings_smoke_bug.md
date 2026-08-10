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
