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

## Next

Recover controller cadence, altitude transfer and live model/camera inputs. Load the roster-zero
vehicle, initialize it and install its selected field. Drive/turn/slew from explicit shared
commands and present current state continuously on native/browser. Reuse common rendering,
assets and simulation; retain only device/clock/presentation in platform glue. Exercise timed
control playback, sustained movement and seams, independent hull/turret headings, missing assets/
players and complete shutdown. Review actual native and Chromium output during motion and check
meaningful frame/control/state behavior, strict tooling and memory. Commit/push each verified
bounded integration step, then continue parent 0041 with HUD, weapons, objectives and sound.

## Accept

A real provisioned mission scene is continuously controllable on both platforms with correct
shared installed movement/contact state and live vehicle/camera/model presentation. Actual
native/browser evidence, timed shared input tests and required tooling/memory gates pass.
No complete playable mission, full original class tick, collision/combat/objective/outcome,
all-device/menu/editor or audio completion claim is made from a driving scene alone.
