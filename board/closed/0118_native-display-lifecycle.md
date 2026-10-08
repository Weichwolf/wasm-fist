Type: Test
Title: Keep private native test displays alive during client discovery
Depends: 0064

## Contract

Fix intermittent unavailable-X11 native test starts without changing game code,
scene/input assertions or isolation. Each fixture owns and terminates its Xvfb
server; short-lived discovery clients must not reset that server during SDL startup.

## Evidence

The original actual native scene exits early with status 1 and no runtime diagnostic.
An error-only temporary frontend, linked to the exact same production libraries,
reproduces SDL initialization failure "x11 not available" at startup trial 13.
It is diagnostic evidence, not a replacement scene gate. The installed SDL 2.32.4
X11 backend opens separate display and request connections, rejecting either open
failure. Official source:
https://raw.githubusercontent.com/libsdl-org/SDL/release-2.32.4/src/video/x11/SDL_x11video.c

Installed Xvfb -help documents -noreset as preventing reset after the last client
exits. The inferred race is between short-lived xdotool search clients and SDL's
startup connections. Adding only -noreset to the private fixture prevents such
resets; no SDL hint, application retry, suppressed error or disabled check is added.
The server still terminates in the existing finally block.

All 32 fresh starts of the unchanged actual production program pass with -noreset,
empty stdout/stderr and clean Escape shutdown, session46908, terminal0. The complete
repaired actual native TRAIN1 gate passes, session75368, terminal0: full frames,
held driving/hull/turret inputs, weapon selection/cycle, pause publication, focus
loss/released controls and clean shutdown. Seventeen captures and all program,
helper and diagnostic-source hashes are retained.

Evidence/reproduction commands are in
/tmp/wasm-fist-0117-review/native-display-lifecycle.json and native-start-noreset.json;
the startup fixture is /tmp/wasm-fist-0117-native-start-diagnostic.py. The checked
program is /tmp/wasm-fist-ground-motion-integration-build/native/fist_driving_preview.
These startup trials do not count toward the independent complete-game WASM gate.
C/build inputs remain unchanged; no C acceptance follows from this fixture repair.

## Next

Continue exact target-motion integration gates under 0117 and complete class/world
work under 0081. Full graphical Valgrind instrumentation remains separate from the
already-passing production lifetime Memcheck; source/game/profiling gates are not
accepted by these startup trials.

## Accept

Delivered: the observed startup failure is reproduced, the private display reset
policy is corrected, 32 fresh real-client starts and the complete actual native
scene gate pass without weakening scene/input/exit assertions. Private server
ownership and cleanup remain intact.
