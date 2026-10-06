# Controlled ground-vehicle scene

`app/driving` owns an initialized roster-zero player, immutable unit definitions, original
terrain/model assets and a separately installed numerical height field. `sim/driver` connects
manual commands, altitude transfer, movement/manual turret, phase progression and ground
contact. `app/driving_view` draws the current actor through the common sprite compositor and
follow camera. Native SDL2 (software renderer and software window framebuffer) and browser events provide
storage, monotonic elapsed time and
presentation; neither platform implements movement or control rules.

This delivers driving, not a complete playable mission. Weapons, targeting, AI, collision,
HUD/objectives, outcomes, suspension animation and audio remain work in 0041/0043/0044.
Only the player is currently displayed. Original targeting references are outside the typed
manual actor: CYPRUS4/INDIA5 contain nonzero references, so the original stage oracle explicitly
uses an untargeted boundary. That test does not claim complete original class execution.

## Recovered update and intentional bindings

Original mission 459a/45f7 consumes the DGROUP counter at +452h. Timer calibration
13064..13087 divides measured retrace PIT counts by **4daeh = 19886** and stores the fractional
increment at DGROUP +18b8h. ISR 13263..1327d accumulates it and increments +452h on carry.
The shared clock uses an integer phase: `elapsed_microseconds * 1193182`, with one step per
`19886 * 1000000` phase units. It retains the remainder, advances all elapsed steps and excludes
paused intervals. Original late-frame discard is deliberately replaced with complete catchup.
Drawing and event frequency do not change the simulation cadence.

`fist_driving_advance` first applies the *previous* held keys throughout the elapsed interval,
then installs new keys at that boundary. A rising P edge toggles pause; repeats do not. Focus
loss releases every held key and pauses. Resuming adds no paused-time steps.

| Binding | Shared command each simulation step |
| --- | --- |
| W / S | Signed throttle +1 / -1 while below +254 / above -254. |
| A / D | Requested hull turn -182 / +182, modulo one 16-bit turn. |
| Q / E | Requested turret offset -364 / +364, independently of the hull. |
| Space | Set throttle to zero; existing speed subsequently brakes through its normal servo. |
| P | Toggle pause on the press edge. |
| Native Escape / window close | Release the scene and exit. |

These are explicit new held-key bindings, not the original complete input/profile dispatcher.
The original a410/a427/a45c actions establish throttle behavior; a3e2/a3ec establish hull
increments. aae8 calls the actual class refresh handlers at 784d/7faf/8cc8/9407, clearing control
flag 1 and marking the respective component byte 3. a376/a3a8 and their complete cursor table
establish the chosen 364-unit manual turret increment with view mode 1. Opposing directions
cancel; Space wins over throttle changes in the same step.

Each step transfers only ground byte +1dh to altitude byte +0dh, preserving the other three
altitude lanes. It then runs the delivered complete motion/manual turret stages, adds 2 to
phase +3dh and publishes height and independent hull/turret slopes at the new pose. Failure
preserves the complete actor and controller clock/input state. Class weapon/damage/AI methods
are not replaced with invented behavior.

## Live rendering and ownership

Live composition uses current X/Y, hull heading, absolute turret heading and original +a9/+aa
selectors XOR 80h. It reuses the existing facing/part/atlas owner and original class texel
scale. Definitions and their raw snapshots remain immutable. The external camera remains the
explicit 0059 inspection view: 64 render units behind, 32 above the higher triangle surface at the actor or camera. The pitch
aims at the actor. INDIA3 exposed the old view below a steep slope; a reached synthetic hill
regression now verifies camera clearance and aim for both immutable/live adapters. Display
placement also follows that exact mesh; numerical installed heights/slopes/altitude are retained
in simulation, not silently substituted with graphics samples. This is a visual design choice,
not original cockpit or suspension behavior.

Load-time scenario/source buffers can be freed immediately. The session owns its independent
terrain, units, model and installed field. Temporary composed bitmaps are released after drawing;
shutdown frees the session and render context. Browser shutdown also ends its worker pool.

## Run and reproduce

Native requires SDL2 development files and pkg-config. Start directly from read-only originals:

```sh
bash tools/rewrite/build.sh all
/tmp/wasm-fist-rewrite/native/fist_driving_preview armoredfist/FISTDATA/AZER1.FSG armoredfist/FISTDATA 2048
```

The native third argument is explicit installed field size. The browser preview chooses 2048,
matching the recovered original default 11-bit detail; this does not replace configurable
settings. Prepare browser inputs in an empty dedicated directory, then serve its build directory:

```sh
python3 tools/rewrite/prepare_driving_preview.py --scenario armoredfist/FISTDATA/AZER1.FSG
python3 tools/rewrite/serve.py
```

Open `http://localhost:8000/driving.html`. `prepare_driving_preview.py` without `--scenario`
creates a constructed controllable fixture with no original content. All scratch remains in
`/tmp`; do not mix previous preview inputs into a new manifest.

Verification commands:

```sh
python3 tools/rewrite/test_driving.py
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_driving.py --originals --oracle
python3 tools/rewrite/verify_browser.py --driving --output-dir /tmp/wasm-fist-driving-review
python3 tools/rewrite/verify_driving_native.py --scenario armoredfist/FISTDATA/AZER1.FSG --assets armoredfist/FISTDATA --output-dir /tmp/wasm-fist-driving-review
python3 tools/rewrite/check_style.py
```

The optional original oracle requires pinned Unicorn 2.1.4; see `oracle_requirements.txt`.
Seven test groups compare complete typed state/clock/input transcripts, all four classes,
wrapped signed positions, throttle boundaries/out-of-range values, opposing inputs, altitude
lanes, interval partitioning, event boundaries, pause/repeats, missing assets/player and invalid
field/input rejection. The explicit corpus gate verifies all 47 player starts and eight installed
2048-square fields. It executes complete original control, class motion, turret and ground
returns at the stated manual boundary. Missing output or skipped requested coverage fails.

Actual Chromium compares every canvas pixel with the current complete C RGBA framebuffer and
checks movement, independent headings, pause, focus loss and teardown. The isolated Xvfb native
gate injects actual SDL keys, checks complete changing/stable frames, focus behavior and clean
exit; it requires `xvfb`, `xdotool` and ImageMagick. CI runs both with constructed assets. Actual
AZER1 and INDIA3 native/browser views were reviewed for 0064; these checks are not final
all-mission or ten-consecutive-WASM acceptance.
