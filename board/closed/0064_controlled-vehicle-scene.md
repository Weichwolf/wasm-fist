Type: Work item
Title: First controlled moving ground vehicle scene
Depends: 0063

## Contract

Connect owned scenario/assets, initialized player state, installed height field, shared movement
and ground contact to a continuous controlled native/browser scene. Use one shared frame/control
owner; platform code supplies input/time/presentation only. Recover class altitude transfer,
controller cadence and current model/camera rules before choosing behavior. Preserve immutable
asset definitions. This is the driving/control stage toward the first complete playable mission;
combat, objectives/HUD, outcomes, full animation/damage and audio remain subsequent work.

## Evidence

Closed 0059 supplies reviewed shared terrain/vehicle drawing and the explicit follow inspection
camera. Closed 0060/0061 supply owned initialization, complete movement/manual turret stages
and class component refresh. Closed 0062/0063 supply installed numerical-height resizing,
height/slope queries and independent hull/turret contact fields. The static preview still reads
an immutable definition and triangle surface; it has no live actor, class altitude transfer,
mission/controller clock or native/browser input loop. Original class entries transfer only
height byte +1dh into altitude byte +0dh before motion; contact publication itself preserves
altitude. The caller owns phase progression and participating roster selection.

## Verification

Delivered `sim/driver`, owned `app/driving` input/time/session and `app/driving_view`, live model/
camera adapters, continuous SDL2/browser previews and reproducible actual-display gates.
Recovered nominal cadence from 13064..13087 / 13263..1327d: 19886 PIT counts per step;
shared integer remainder clock consumes all elapsed steps. Held commands use recovered throttle,
hull and maximum manual turret increments, explicit new keyboard bindings, pause edges and
focus-loss release. Full rule/scope and reproduction commands: `docs/driving-scene.md`.

- `bash tools/rewrite/build.sh all`: 14 native CTests and all WASM gates pass, including new
  seven-group timed driving contract; optional corpus gates are recorded separately below.
- `python3 tools/rewrite/check_style.py`: formatting and strict LLVM 19.1.7 tidy pass for all
  40 owned translation units; requested production flags and owned warnings-as-errors retained.
- Pinned Unicorn `test_driving.py --originals --oracle`: seven groups, no skips, 19.513 s;
  complete clock/control/typed state traces for all four classes, all 47 player starts and eight
  installed 2048-square fields on native and WASM. Actual complete original manual actions,
  component prefix, motion/turret and contact returns, plus the actual class-entry altitude
  MOV instructions, are checked at their declared boundary.
  CYPRUS4/INDIA5 contain target references: the manual-stage oracle explicitly clears the
  reference at its input boundary, matching this stage's declared untargeted state. It does
  not prove targeting or full original class ticks.
- Address/undefined/leak sanitizer build of the shared loading/driver/controller and probes:
  same seven groups with all originals/oracle on native, no skips, 8.274 s, clean. Inputs/storage
  are freed before replay, destruction is repeated, deep missing-model/player/field failures run.
- Full live native preview built with address/undefined/leak sanitizers also passes the actual
  INDIA3 display/input/focus/shutdown gate. This first reached a 108124-byte Mesa/GLX leak:
  SDL's software renderer still created an accelerated window framebuffer. Explicit
  `SDL_HINT_FRAMEBUFFER_ACCELERATION=0` selects the actual software window surface, appropriate
  to softgl-owned rendering; the complete rerun is clean with leak detection still enabled.
- Actual native SDL and Chromium AZER1/INDIA3 before/after controls are reviewed. Browser checks
  every RGBA canvas pixel against the live C frame; both display gates prove held controls,
  pause, focus loss and teardown. A malformed scenario after verified browser downloads reaches
  C startup failure and releases its prestarted worker pool without publishing a scene.
  Constructed equivalents run without originals in CI.
  Static browser/terrain presentation regressions also pass after sharing manifest I/O.
- Review reached an INDIA3 camera under its own steep terrain. Follow altitude now clears the
  higher actor/camera mesh surface and pitch aims at the actor; the independently constructed
  hill regression fails the former view and passes both adapters on both targets. Display
  placement stays on the exact mesh; this change does not alter installed simulation contact.
- Xvfb verification explicitly selects SDL's X11 device, avoiding the host Wayland session.
  Shutdown injects Escape keydown only: sending its keyup after window destruction was a
  reaching harness error, now corrected. A successful game exit is still required.

## Next

Continue parent 0041 with player weapons/ammunition/reload, projectile/hit rules, feedback,
objectives and shared sound events. Preserve this working continuous driving baseline; full
mission behavior and graphics improvements remain active work. No first playable mission,
full class/targeting/AI, collision/combat/objective/outcome, all-device/menu/editor or audio
completion is claimed from this driving scene.

## Accept

A real provisioned mission scene is continuously controllable on both platforms with correct
shared installed movement/contact state and live vehicle/camera/model presentation. Actual
native/browser evidence, timed shared input tests and required tooling/memory gates pass.
No complete playable mission, full original class tick, collision/combat/objective/outcome,
all-device/menu/editor or audio completion claim is made from a driving scene alone.
