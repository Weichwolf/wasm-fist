Type: Test
Title: Acknowledge complete native presentation under instrumentation
Depends: 0118

## Contract

Wait for a visible native window and a complete textured frame before accepting
input/pause publication. Keep the normal five-second deadline and all complete
frame, HUD, stability, input, focus and clean-exit assertions. Instrumentation
may request an explicit positive deadline of at most sixty seconds.

## Evidence

The unchanged production program under Valgrind exposes two fixture races: PID/name
discovery can precede window mapping (X_SetInputFocus BadMatch), and the mapped window
can precede the first complete rendered frame. A five-second publication deadline
also expires during a later weapon-cycle acknowledgement under instrumentation.
Discovery now requires --onlyvisible. In the bounded publication loop an initial
untextured frame remains pending; direct stability captures still reject it.
Missing/incomplete RGBA, never-published frames and incorrect visible pause state
still fail. --timeout-seconds defaults to 5; the profiling gate explicitly uses 30.
Zero, negative, over-limit, NaN and infinity inputs all fail with status 2 before
starting clients. Production game/build inputs are unchanged.

The actual normal TRAIN1 gate passes with the default deadline, session9117,
terminal0. The same complete gate with the actual production program under
Valgrind 3.24.0 passes, session87132, terminal0. Both verify held driving/turret
input, full pause/stability, weapon selection/cycle, focus/released controls and
clean Escape shutdown. Each retains thirteen accepted full frames; intermediate
publication attempts are separate evidence. The instrumented after-frame was
visually reviewed.

Memcheck reports zero errors, definitely/indirectly/possibly lost bytes and
suppressions. All 121 reachable-allocation records were inspected: 50,032 bytes
in 912 blocks originate in X11 and 3,445 bytes in 34 blocks in D-Bus, totaling
53,477 reachable bytes. Their recorded allocator stacks contain no owned-code
frame. SDK reachability remains reported; no shutdown hint or suppression is used.
The simulation lifetime gate separately exits with no remaining blocks under0117.

Source/program/log/capture hashes, all allocator stacks and reproduction commands:
/tmp/wasm-fist-0117-review/native-publication-deadline.json. The wrapper
native-valgrind-owned.sh passes --error-exitcode=97 --leak-check=full
--show-leak-kinds=all --errors-for-leak-kinds=definite,indirect,possible
--track-origins=yes. Candidate0117's 508 pinned source inputs remain unchanged.

## Next

Continue the full target-motion production regression under0117 and complete
class/world scheduling under0081. Neither this scene nor Memcheck proves full-game
behavior, PCM, hardware-counter performance or the final complete-game WASM streak.

## Accept

Delivered: normal-default and instrumented complete real-client gates pass with
bounded visible-window/full-frame acknowledgement and unchanged content/input/exit
assertions. Invalid deadlines fail early; SDK reachable memory is explicitly retained
in the evidence. Commit/push the fixture separately from unaccepted C prototypes.
