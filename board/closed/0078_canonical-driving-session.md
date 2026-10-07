Type: Work item
Title: Canonical world ownership in the controlled mission scene
Depends: 0041, 0064, 0065, 0075, 0077

## Contract

Load the complete supported saved mission into the existing world owner and drive/render its
selected physical ground actor directly. Retain final file-order initialization RNG and every
other physical object/roster/binding. Own either the standalone diagnostic actor or the complete
world, never a retained second copy of the canonical player. Reuse the existing input/time,
weapon feedback, contact and rendering owners. Keep all-47 standalone diagnostics available;
mission mode rejects complete unsupported inputs without an automatic fallback. Bind both
native and browser presentation to the same canonical player accessor. Do not claim complete
living class ticks, AI, fire eligibility, battle rendering, PCM or playable mission completion.

## Evidence

0077 is committed/pushed at 585dd0d. Implementation began from that reference; /tmp/wasm-fist-0077-combat-review/summary.json retains the accepted gates.
`app/driving` previously initialized only roster-zero using a separate random copy and retained
its own player value. `sim/mission_world` already installs all supported objects in file order
and owns physical roster/RNG. Steering a separate player cannot feed that canonical combat
world. Constructor/contact/take-control/manual-stage owners and frame presentation are delivered;
reuse their recovered contracts rather than inventing a complete living method.

## Delivered

Explicit mission loading and one player accessor now bind controller, HUD, camera and both
frontends to the canonical selected actor. The scene owns either one diagnostic heap actor or
one complete heap world. Source buffers are released before observation; failure preserves a
nonzero output marker and unchanged random/detail options. Invalid physical selections,
dual ownership and pending selected-loss control are rejected atomically. Every successful
interval additionally compares complete world bytes with only the selected actor temporarily
restored, proving unrelated payloads, pool, roster, RNG and combat metadata preservation.

The existing probe accepts an explicit `mission` argument and a mission-only seed/cursor
request header. Complete session/world streams reuse the existing serializers and golden
constructor, ground, manual-controller and lifetime owners; SHA-256 and byte lengths compare
unfiltered output, with bounded first-difference diagnostics. Both frontends expose mission
loading explicitly; manifest creation still has one common writer. The existing all-47
standalone mode remains the default. CI requires both diagnostic and canonical scene devices.

A reaching native refresh failed complete-frame stability after a fixed .25-second pause wait.
The verifier now acknowledges the reviewed visible PAUSED label and visible resume before
comparing complete frames. Each production and sanitizer run observed one older frame at each
resume boundary. The five-second deadline fails missing publication; complete RGBA, palette,
whole-frame stability and all device assertions remain required. Simulation/input code was not
changed to address this presentation-observation race. The first isolated browser server launch
also failed because its required index.html was absent; the test directory now contains an
owned copy of the driving entry. Neither issue weakened expectations.

## Next

Continue 0041/0065 with complete reached living ground methods, actual command eligibility and
canonical world scheduling. Then consume battle rendering, selected-loss UI, audible PCM and
mission outcomes. Other supported payloads remain frozen in this controlled scene; retained
world installation is not complete living execution or a playable mission.

## Accept

Actual native/browser controlled scenes consume one canonical selected actor in the complete
installed world. Both-target complete state/ownership/error traces, original boundaries, real
TRAIN1 and all supported contexts, standalone regression, actual visuals/devices and required
build/style/memory gates pass. Scope and remaining playable-mission requirements stay explicit.

Verified on 2026-10-07:

- `bash tools/build.sh all`: terminal zero, 28 native CTest gates and all 26 WASM Python
  gates plus both renderer probes. After include/probe/observable changes, the final native
  refresh `bash tools/build.sh native` passes all 28 gates in 275.63 seconds. Both
  production trees report no pending Ninja work; no binaries were rebuilt under their tests.
- `python3 tools/check_style.py`: terminal zero, strict LLVM 19.1.x formatting/tidy for
  all 73 owned C units. Initial direct-include, adjacent-parameter and narrow-loop diagnostics
  were fixed without rule changes; final direct preview and complete gates pass.
- `/tmp/wasm-fist-decoder-oracle/bin/python tests/test_mission_driving.py --target all --originals --oracle`: all five groups, zero skips, 16.110 seconds. Per target: 89 fixtures,
  808 complete timed boundaries, 883 installed objects, 46 explicit rejections. Includes every
  ground class, all RNG cursors, pause/partitions/weapon edges, selected physical orphan under an
  overwritten registry, source/detail failures, all ten supported pinned contexts (671 objects)
  and all 37 unsupported inputs. TRAIN1 has 85 objects and selected physical slot 151.
- Required standalone `test_driving.py --originals --oracle`: all nine groups, zero skips,
  native 26.939 / WASM 55.581 seconds, all 47 starts and eight installed maps. Expectations
  remain unchanged; optional observer composition is the only golden trace API extension.
- ASan/UBSan/LSan with production fast-math: required mission and standalone original-corpus
  gates pass all five / nine groups without skips in 10.849 / 12.463 seconds. The actual
  sanitized TRAIN1 SDL scene also passes complete acknowledged frames/devices and clean exit.
- Actual Chromium TRAIN1, constructed canonical and standalone scenes: complete opaque canvas
  equals every current C pixel, movement/independent turret, pause, ammunition/reload/Tab/fifth
  station, focus, shutdown and failed-start worker cleanup pass. TRAIN1 reports mission=1,
  physical selection=151; standalone reports mission=0 and no physical selection. Reviewed
  actual native/browser before/after frames show the authored tank, terrain and shared HUD.
- Actual SDL TRAIN1 and constructed canonical/standalone scenes pass acknowledged complete
  frames, movement, weapon display, stable pause, focus and clean shutdown. Source originals
  were copied into isolated /tmp fixtures and remain unchanged.

Compact review `/tmp/wasm-fist-0078-driving-review/summary.json` retains source/binary/log/image
hashes and exact gate outcomes. Remove owned logs, scene copies and sanitizer build after
verified commit/push. Complete mission/PCM/persistence acceptance remains open; the final
complete WASM streak is zero.
