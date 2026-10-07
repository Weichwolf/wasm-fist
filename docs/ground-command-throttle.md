# Command throttle and drive-profile transitions

Closed WI 0095 supplies the complete recovered ad2f throttle bank and its unconditional ad3b
continuation. Controlled parent entry three calls ad3b alone. The shared C owners are
`fist_mission_world_throttle_command`, `fist_mission_world_update_command_profile` and the
vehicle profile setter. The complete sixteen-entry parent, target feedback production,
living battle and audible PCM remain open; no partial parent bank is installed.

## Complete eight-entry throttle bank

Original DS:97f0 uses the even command byte as a byte offset. Its mode 4/6 entry order differs
from the direction bank at DS:9810. Throttle selection has no automatic-control-bit gate.

| Command | Entry | Complete throttle behavior |
| --- | --- | --- |
| 0 | ad62 | Invalid goal stops. Valid unsigned range <=8 uses 80; larger ranges use PINF +6 to select 96/160/208/224. |
| 2 | ad8c | Invalid goal, range <=3 or 65535 stops; ranges <=8/32/48/80 use 16/32/128/240, otherwise 272. |
| 4 | ae06 | Genuine return; retain throttle before the unconditional profile continuation. |
| 6 | addb | Read independent unsigned target feedback word +99. Values <45/<60/<90 use -48/0/80; larger values use 240. |
| 8 | ae07 | Maneuver byte 2 stops, 6 uses -48, every other byte uses 64. |
| 10 | ae25 | Genuine return; retain throttle before the unconditional profile continuation. |
| 12/14 | ae26/ae2c | Stop, then execute the unconditional profile continuation. |

The throttle destination is signed word +57. Navigation range +53 and target range +99 are
independent retained words. The rewrite restores +99 without translating its saved near
reference or inventing a live target. Later aiming/acquisition must supply target feedback;
this callback consumes it and does not perform a target lookup.

Only an admitted leader branch with valid goal and range >8 reads PINF +6. Actual increment/
decrement UI tails 632f/6342 prove its four-choice domain. Reject a used value outside 0..3
atomically; an unused retained selector must not reject another original branch. Controlled
ad3b does not inspect command mode, maneuver, ranges or PINF choices.

## Complete profile and display effects

ad3b first compares byte +90 unsigned with 1. Larger retained profile bytes do nothing.
For profiles 0/1 it compares signed terrain-pitch word +34 with 3584, selecting profile 1
at or above the threshold and profile 0 below it. Original speed is word +55; it is not the
transition operand. A matching current profile does nothing. A transition calls the real
far a19e dispatcher, which indexes DS:965e by the ground type's doubled WORD.

| Ground class | Actual setter | Component destination | Component-local index |
| --- | --- | --- | --- |
| 0 | 7da5 | +d6 | 23 |
| 1 | 8679 | +c8 | 12 |
| 2 | 8f3a | +ca | 12 |
| 3 | 96cb | +d2 | 22 |

Each setter stores AL at +90, writes 3 to that component and executes complete f69:7a5b.
The latter writes 3 at DS:8e58/8e5a/8e5c/8e5e and returns far. It has no audio/device call.
The C producer reports one explicit `refresh_drive_display` notification for all four controls;
platform UI drawing/cache consumption remains a later UI flow. Direct setter calls store any
byte, including the current value, and always produce component/display refresh. Automatic
ad3b only invokes that owner on an actual 0/1 transition.

The existing shared motion owner consumes the updated profile through its slope-speed table.
Reaching tests execute complete throttle/profile followed by original class motion and manual
turret returns, comparing actor state and current C motion. Separate tests carry the actual
C target-loss goal/bearing result into throttle: absent routes/formation leaders stop, while
valid new goals use ordinary navigation. No unrelated DS coordinates, stale successor target,
extra RNG draw or suppressed continuation is used.

## Verification

    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_ground_throttle.py --target all --originals --oracle --review-dir /tmp/wasm-fist-0095-required
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/test_original_ground_range_retention.py --review-dir /tmp/wasm-fist-0095-retention-required
    PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache python3 tools/check_style.py
    CTEST_PARALLEL_LEVEL=4 bash tools/build.sh all

The independent persistent model and unchanged original observer check complete near/far
returns, exact actor/component/display effects, complete unrelated DGROUP preservation and
unchanged RNG. The required gate covers all control words in all eight modes, all unsigned
navigation/target ranges, all signed pitch words for both automatic profiles, every retained
profile/maneuver byte, all direct setter bytes, lazy invalid fields, atomic transport failures,
all 47 saved ground corpora and actual all-47 prepared worlds on all eight original decoded/
resampled maps at four details. Every prepared world declares and executes all eight bank
entries in registry order; this is a complete child-callback boundary, not an original untouched
living tick. Missing inputs, unequal lengths, incomplete calls or required skips fail.

A separate required original field-retention gate checks 96 complete c296 starts and 96
complete class-ready returns across all four ground classes, links 0/1/2 and eight low/high
WORD boundaries. Complete raw actors match the existing independent start/reset models,
and readiness preserves unrelated DGROUP and RNG. This proves original +99 retention;
the C batch separately checks every supplied word across restoration/start/preparation.

The final required current-artifact native/WASM gate passes eight groups without skips in
766.590 seconds; the complete ASan/UBSan gate passes the same coverage in 502.599 seconds.
Each target has 872120 scalar cases, 780 atomic rejections, 871340 original command/setter
returns, 188 prepared worlds and 30720 complete canonical callbacks. Reaching coverage includes
768 actual C target-loss repair continuations, 352 stops without a valid goal, 512 original
motion/manual-turret returns and 64 actual PINF UI cycles. Both gates have normalized SHA256
459f0937ba5f4344cf1db0bae10b27cedbfb1632fe44ee559a393299d46f8321.

The separate 192-return original-retention gate passes without skips. Strict LLVM 19.1.x
style passes 87 owned units/headers; the full build passes 41 native CTests, 39 WASM Python
suites and two Node pixel probes. Actual native/browser/sanitized TRAIN1 scenes pass, and
both production after frames were visually reviewed. The first terminated memory attempt
is excluded; only the complete retry counts. Compact evidence and reproduction commands
remain under /tmp/wasm-fist-0095-review; see board/closed/0095_ground-command-throttle.md.
Full parent, target feedback/acquisition, living battle/PCM/outcomes and independent complete
game acceptance remain open. The complete-game WASM streak is zero.
