# Controlled ground-vehicle scene

`app/driving` owns either an initialized standalone roster-zero player or the complete supported
mission world, plus immutable unit definitions, original terrain/model assets and a separately
installed numerical height field. In mission mode, control, camera, sprite composition and HUD
borrow the selected physical ground actor directly from `world->objects`; there is no retained
second player. The canonical combat owner owns selection and the physical roster. All other
objects and the final file-order initialization RNG survive controlled intervals unchanged. `sim/driver` connects
manual commands, altitude transfer, gun/recoil, movement/manual turret, phase progression,
reload/history dispatch and ground contact. `app/driving_view` draws the current actor through the
common sprite compositor and follow camera, then its C/softgl weapon display. Native SDL2
(software renderer and software window framebuffer) and browser events provide
storage, monotonic elapsed time and
presentation; neither platform implements movement or control rules.

The scene delivers driving with weapon selection and mechanical reload feedback. Firing/projectiles,
targeting, living AI, world scheduling, battle presentation, objectives, outcomes, suspension
animation and audible PCM remain work in 0041/0043/0044; it is not a complete playable mission.
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

| Binding | Shared command |
| --- | --- |
| W / S | Signed throttle +1 / -1 while below +254 / above -254. |
| A / D | Requested hull turn -182 / +182, modulo one 16-bit turn. |
| Q / E | Requested turret offset -364 / +364, independently of the hull. |
| Space | Set throttle to zero; existing speed subsequently brakes through its normal servo. |
| P | Toggle pause on the press edge. |
| 1–4 / 5 | Select on the press edge; station 5 exists only on T80. |
| Tab | Cycle stations 1→2→3→4→1 on the press edge, including on T80. |
| Native Escape / window close | Release the scene and exit. |

These are explicit new held-key bindings, not the original complete input/profile dispatcher.
The original a410/a427/a45c actions establish throttle behavior; a3e2/a3ec establish hull
increments. aae8 calls the actual class refresh handlers at 784d/7faf/8cc8/9407, clearing control
flag 1 and marking the respective component byte 3. a376/a3a8 and their complete cursor table
establish the chosen 364-unit manual turret increment with view mode 1. Opposing directions
cancel; Space wins over throttle changes in the same step. Weapon edges execute after the
elapsed interval and pause edge; paused selection is ignored without being deferred. Simultaneous
digits execute in ascending order, then Tab. Re-selection retains the original setter's no-op
state but still runs the mapped take-control refresh. The [weapon owner](weapon-control.md)
supplies all class profiles, ammunition queries and selection/reload events.

Each step transfers only ground byte +1dh to altitude byte +0dh, preserving the other three
altitude lanes. It then runs the gun/recoil prefix and complete motion/manual turret stages,
adds 2 to phase +3dh, dispatches reload on phase & 1eh == 0 and position history on phase &
1eh == 6, then publishes height and independent hull/turret slopes at the new pose. Failure
preserves the complete actor, feedback and controller clock/input state. The
[position-history owner](vehicle-history.md) retains all six saved samples and the shared
initialized counter. Other phase callbacks and class firing/damage/AI stages remain separate work.

The session owns selection/reload/voice-request counts, the last requested voice ID and the
active notice with its simulation-tick deadline. This is mechanical feedback and verification,
not an audio queue or audible delivery. Large intervals advance every reload dispatch and
notice deadline with the same cadence as finely partitioned input.

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

The shared overlay uses authored 5×7 glyphs, cyan station/border, amber status and light ammunition
text on an opaque dark panel. It displays only the selected store and class-defined reserve.
RELOADING denotes a noncontinuous station with a running countdown; EMPTY denotes zero
ammunition, and PAUSED takes display priority. SELECTED is not a recovered firing eligibility
claim. A read-only weapon query supplies both rendering and verification, including T80's
separate fifth store. Missing/invalid station data fails loading or drawing rather than showing
a fabricated default. SDL and canvas only present the complete C frame.

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
Use `--vehicle-class T80` (or M1/M3/BMP) with constructed inputs to reproduce class-specific
controls and the fifth-station display. This option cannot override an original scenario.

Verification commands:

```sh
python3 tools/rewrite/test_driving.py
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_driving.py --originals --oracle
python3 tools/rewrite/verify_browser.py --driving --output-dir /tmp/wasm-fist-driving-review
python3 tools/rewrite/verify_driving_native.py --scenario armoredfist/FISTDATA/AZER1.FSG --assets armoredfist/FISTDATA --output-dir /tmp/wasm-fist-driving-review
python3 tools/rewrite/check_style.py
```

The optional original oracle requires pinned Unicorn 2.1.4; see `oracle_requirements.txt`.
Nine test groups compare complete typed state/clock/input/feedback transcripts, all four classes,
wrapped signed positions, throttle boundaries/out-of-range values, opposing inputs, altitude
lanes, interval partitioning, event boundaries, pause/repeats, missing assets/player and invalid
field/input/station rejection, station capability/repeat/pause behavior and reload partitioning.
The explicit corpus gate verifies all 47 player starts and eight installed 2048-square fields.
It executes complete original control, class motion, turret, station setter, reload and ground
returns, plus the altitude and gun/recoil fragments and actual phase ADD/index instructions,
at the stated manual boundary. Missing output or skipped requested coverage fails.

Actual Chromium compares every canvas pixel with the current complete C RGBA framebuffer and
checks movement, independent headings, weapon selection/cycling/class capabilities, reload
expiry, pause, focus loss and teardown. The isolated Xvfb native
gate injects actual SDL keys, checks complete changing/stable frames, focus behavior and clean
exit; it requires `xvfb`, `xdotool` and ImageMagick. CI runs both with constructed assets.
Actual AZER1 and INDIA3 native/browser views include the reviewed shared weapon panel and
reserve/countdown states for 0065; these checks are not final all-mission or ten-consecutive-WASM
acceptance. `--settle-seconds 1` allows sanitizer SDL builds to publish their complete frame
before inspection without weakening any pixel or state assertion.

Memory verification uses a separate external build with the production fast-math flags:

```sh
cmake -S . -B /tmp/wasm-fist-0065-scene-sanitized -G Ninja \
  -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  '-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer'
cmake --build /tmp/wasm-fist-0065-scene-sanitized --target fist_driving_probe fist_driving_preview
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_driving.py \
  --target native --originals --native-probe /tmp/wasm-fist-0065-scene-sanitized/fist_driving_probe
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  python3 tools/rewrite/verify_driving_native.py --settle-seconds 1 \
  --native-preview /tmp/wasm-fist-0065-scene-sanitized/fist_driving_preview \
  --scenario /tmp/wasm-fist-rewrite/wasm/assets/AZER1.FSG \
  --assets /tmp/wasm-fist-rewrite/wasm/assets --output-dir /tmp/wasm-fist-driving-review
```

Prepare the isolated assets with `prepare_driving_preview.py` before the SDL command, as above.

## Canonical mission mode (0078)

Append the explicit `mission` argument to the native scene, or prepare a browser manifest with
`--mission`. Both frontends call the same `fist_driving_load_mission` API. Complete unsupported
worlds return `FIST_MISSION_UNSUPPORTED`; exhaustion returns `FIST_POOL_UNAVAILABLE`; invalid
inputs, missing player/assets and allocation/detail failures return -1 without publishing a
partial session. There is no standalone fallback. The existing default diagnostic mode still
accepts all 47 original player starts independently of whole-world class support.

Selection initially uses physical roster entry zero, performs the delivered original control
refresh, and installs ground contact. Driving intervals stage only a temporary transactional
actor, then commit directly to that physical payload. They do not run complete living methods,
consume world combat RNG, schedule combat visits or acknowledge selected-loss impacts. An open
selected-loss continuation rejects control intervals atomically. Changing vehicle selection,
loading its replacement visual family and actual takeover UI remain separate work.

```sh
/tmp/wasm-fist-rewrite/native/fist_driving_preview \
  armoredfist/FISTDATA/TRAIN1.FSG armoredfist/FISTDATA 2048 mission
python3 tools/rewrite/prepare_driving_preview.py --mission \
  --scenario armoredfist/FISTDATA/TRAIN1.FSG --output-dir /tmp/wasm-fist-0078-browser/assets
python3 tools/rewrite/test_mission_driving.py
/tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_mission_driving.py --originals --oracle
python3 tools/rewrite/verify_driving_native.py --mission \
  --scenario armoredfist/FISTDATA/TRAIN1.FSG --assets armoredfist/FISTDATA
```

The mission gate compares complete unfiltered timed session/world streams and byte lengths.
Its C probe additionally checks the actual canonical pointer, every invalid physical selection,
dual-owner rejection, selected-loss suspension, failure atomicity, unchanged options and complete
nonplayer/world preservation. Source buffers are released before the first observed state.
The required original gate executes complete saved-object installation, immediate control
refresh, initial ground contact and each declared manual stage. All ten supported contexts
(671 objects) and all 37 explicit whole-world rejections are required. The standalone original
47-player gate remains a separate regression. Actual SDL/browser device and pixel gates exercise
movement, independent turret, pause, weapon HUD/reload, focus loss, shutdown and startup failure.
The SDL gate acknowledges visible pause/resume through the reviewed authored PAUSED label
before checking complete changing/stable frames; queued presentation cannot be synchronized
by a fixed sleep alone. The five-second acknowledgement deadline fails missing publication,
and every observed frame still requires complete opaque RGBA and textured output. This
verification change follows a reaching .25-second paused-frame capture failure under concurrent
load; simulation and input rules were unchanged. These observations prove the controlled subset; complete battle/PCM/mission acceptance remains
open under 0041/0065, and the final complete WASM streak remains zero.
