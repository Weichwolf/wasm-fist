Type: Work item
Title: Own and consume complete ground route and formation goals
Depends: 0082, 0083

## Contract

Implement the complete ac75 command-goal return using canonical routes/descriptors and the
physical platoon roster. Own retained heading history/average +28..2e and goal XY +49/+4d.
Execute both active entries and the six actual original returns without replacing unimplemented
work. Reuse shared rotation and portable position addition; preserve all unrelated world/RNG
state. Reject malformed used selectors and inconsistent roster metadata transactionally.
This nested prerequisite does not replace 0081's complete parent phase or admit partial dispatch.

## Evidence

Frozen image SHA256 d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5.
DS:9800 dispatches 0/2 to ac7e/ac9e; all six other entries ad02..ad07 are genuine RETs.
ac7e copies first route XY and sets control bit 2 only if the route is nonempty. It does not
clear a retained goal/valid bit when empty. ac9e returns immediately for controlled actors.
For automatic followers it selects one of six four-member formations through DS:9830,
uses the physical platoon leader at 6d3c+platoon*8, returns for an absent/type-23 leader,
adds the leader's retained average heading, rotates through actual 03a9, sign-extends lanes,
scales by 32, wraps position addition and sets the goal-valid bit. Original UI 62cf/62e2
cycles formation word +4 through six choices. Fine rotation is the declared caller boundary.

## Next

Continue full 0081 on master using the canonical selector and complete route/formation goal
helper. Recover the remaining bearing/range, waypoint, target-discovery, physical roster and
presentation callbacks, plus complete parent sampling/admission/scheduling. Keep target-reference
interpretation explicit; a saved word is not a physical C slot. Advance the reaching living
prefix only after complete dispatch is delivered. First playable battle, PCM and outcomes remain
required under 0041/0065, followed by the full rewrite surface and final ten-run gate.

## Accept

Both targets pass complete original 251-byte goal returns and canonical whole-world transcripts
after source release. All used malformed data and wrong metadata fail without changing state;
unused descriptor words stay full-width. Initialization retains every newly owned field. Full
build/style/memory and current scene/device gates pass. No heading-only parent dispatch or
complete battle/PCM/mission claim; active 0081/0041/0065 requirements remain unchanged.

## Verified acceptance

Required `test_ground_goal.py --target all --originals --oracle` passes seven groups with zero
skips: 184948 fixtures, 47268 explicit rejections, all 47 files / 960 ground snapshots and 381
complete world boundaries per target. Complete installation covers all ten currently supported
original worlds; the other 37 contexts explicitly reject. It executes every heading word,
all control words, all mode/count bytes, every formation/member branch and signed position edges.
Original formation UI checks cover all 96 platoon/value/direction returns. Each bulk invocation
also rejects 325 malformed loaded/component/type/binding/roster states without mutation.

The same complete required goal gate passes ASan/UBSan/LeakSanitizer with production fast-math:
seven groups, zero skips, identical fixture/corpus/world scope. Required canonical driving also
passes under sanitizers: seven groups, 123 fixtures, 924 complete timed boundaries, 907 installed
objects and 60 explicit rejections. No source/order/leader/RNG ownership leak is reported.

Required existing selector and canonical driving gates pass on both targets with zero skips:
nine selector groups / 162676 fixtures / 572 rejections / 960 ground states / 63 whole-world
boundaries, and seven driving groups with the same 123/924/907/60 scope. Native class start passes
all seven original groups; scoped WASM start checks pass the complete all-47/960 corpus group
and the expanded all-class initialization-retention fixture on both targets. The latter varies
all four retained heading words and signed goals as well as the previous byte/word fields.
Original command input/bank verification passes three groups, all 47 files, 376 complete loader
returns and 109040 bytes. Both production Ninja trees are current; full strict format/tidy passes
all 77 owned C units. `CTEST_PARALLEL_LEVEL=4 bash tools/rewrite/build.sh all` completes with exit
zero: all 33 Native CTests (142.50 seconds), all 31 WASM Python suites and both renderer gates.
The standard suites intentionally omit optional originals; the separate required gates above
have zero skips and prove their complete recorded original scope. Required combined goal run
takes 205.719 seconds; the sanitized goal run takes 222.670 seconds.

Actual isolated TRAIN1 native SDL and Chromium gates pass complete frames, held controls,
pause/focus/shutdown and browser worker/startup-failure cleanup. Four before/after images were
inspected: upright tank, terrain and HUD remain visible, with independent browser turret motion.
These are current scene regressions, not complete battle or PCM acceptance. An initial native
display start exited without a diagnostic and is rejected; a separate debugger run remained live
for 25 seconds without reaching SDL_Quit, and the subsequent complete scene gate passed. No
unproved startup-cause claim or weakened check is used. Initial probe header/identity/tidy failures
were corrected through owner includes, the actual leader registry identity and named widths.

Raw logs, captures, isolated assets and sanitizer output are owned under /tmp/wasm-fist-0084-review
and /tmp/wasm-fist-0084-sanitized. Retain a compact source/binary/log/visual receipt after acceptance
and remove obsolete owned artifacts. Branch development now uses master under closed 0085;
the full rewrite goal and final complete WASM streak remain open (zero).
