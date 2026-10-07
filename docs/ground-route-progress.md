# Ground route progress and full-capacity repair

The shared `fist_mission_world_advance_command_route` implements complete original ad08 and
its nested f69:b57a service against the canonical mission orders. The parent heading/command
phase remains open under WI 0081; no callback bank or living tick is partially installed.
The actor retains original unsigned word +53 as `command.range`. Restoration and actual class
initialization preserve it; its eventual bearing producer is a separate required callback.

## Complete original behavior

Frozen DOS image SHA256 `d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
Near entry ad08 dispatches via DS:9820 and saved mode byte +43. Mode zero selects ad11;
modes 2/4/6/8/10/12/14 are seven actual single-RET entries ad28/ad2a/ad29/ad2b/ad2c/ad2d/ad2e.
Only mode zero advances a route, when goal-valid control bit 2 is set and unsigned range +53
is at most 48, inclusive. The automatic bit is not an additional gate. After the nested complete
service returns, ad11 clears only bit 2, including for an empty route.

The far method resolves the actor's actual platoon through DS:7d2a, loads PINF word +2 through
the parent-supplied descriptor pointer DS:9796 and dispatches through its original CS table.
Values 0/1/2 have identical pop behavior. Value 3 cycles the first goal to the active tail,
preserving count. Every full-width value >=4 explicitly falls back to zero in the original;
this consumer preserves that rule. Empty routes retain all stored coordinates. Headers,
descriptors, other routes, actors and random state are untouched.

Original pop decrements the byte count, then copies **old-count** coordinate pairs from
points 1 through old-count into points 0 through old-count-minus-one. This includes one
inactive stored coordinate. All such in-record values survive exactly for counts 1..31;
discarding every inactive value would change later save/load state. Original cycle also reads
one extra coordinate after the active tail, then overwrites that temporary tail with the
saved first coordinate before returning. Only count zero avoids its copy loop.

## Proved defect and intentional repair

The editor and owned format allow 32 points. At that valid count, the original pop copies
two DWORDs from eight bytes beyond the 268-byte route. Platoons 0..6 consume the next route's
count/header; platoon 7 consumes the descriptor-pointer table at DS:85a0. Complete unchanged
returns with two different neighbor inputs prove that the newly unused point 31 becomes
those neighbor bytes. This is a storage dependency, not an additional authored waypoint.
Original cycle performs the same out-of-record read, although its final returned tail is
independent of the neighbor.

The required original gate executes 192 paired boundaries (384 complete unchanged returns):
all eight platoons, counts 31/32, modes 0/1/2/3/4/ffff and two neighbor patterns.
Observed and unobserved executions agree across the complete DGROUP after every return.
Count 31 produces no out-of-record loads;
all 96 count-32 boundaries reach two actual DWORD loads beyond the current route. A code
hook observes SI only at those unchanged load instructions. It never supplies a memory read,
changes a register/instruction or manufactures a return. Complete output dependency separately
proves the pop defect. All unrelated DGROUP bytes, RNG and input/output actor identity are checked.

The rewrite preserves capacity 32 and all active goals. Full-capacity pop shifts the remaining
31 active points and retains the old last value in the newly unused last slot. Cycle shifts
only the remaining active points and stores the first at their tail, so it never reads a
point outside its active extent. Count 31 and below preserve complete original stored results.
The full-capacity inactive-slot change is an explicit functional repair; original bit identity
is not an acceptance target. No guessed coordinate, capacity clamp or hidden missing callback
is involved. Invalid used route counts above 32 fail before any mutation; unused malformed
routes remain untouched by a real return or failed goal/range admission.

## Ownership, consuming evidence and reproduction

The existing mission world owns the route, descriptor, physical actor and RNG. No source view,
guest pointer or parallel route cache survives. The helper validates actor/pool/order metadata,
the actual mode domain and any reached route count before writing. It mutates only the current
route and the current actor's goal-valid flag; invalid requests preserve the entire world.
Saved target words remain retained values. This callback does not dereference them or guess
a DOS-reference-to-slot translation. Complete target discovery/resolution remains required.

The shared goal probe's `--routes` mode owns and releases the entire input batch before
execution. It observes the complete actor and every stored value of the selected route;
byte comparisons prove all unrelated world data is unchanged. The world probe's
`--navigation` driver sequences complete delivered goal and route callbacks. This is a
declared consuming boundary, not full heading-phase scheduling. Constructed full-route worlds
exercise all platoons and physical members, including overwritten registry orphans. They
refresh goals, clear validity, exhaust finite routes and repeat cyclic routes past one rotation.
Complete original comparisons cover safe-domain returns. In the constructed capacity-repair
sequence, the original's newly unused slot is explicitly reset to the declared repaired value
between complete calls; such a sequence is not claimed as an unchanged original timeline.
The required corpus verifies all 47 pinned files, all 960 saved ground ranges/routes plus
explicitly constructed reached variants, and complete supported worlds. Unsupported worlds
remain whole-transaction rejections and are not counted as playable missions.

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_ground_route.py \
  --target all --originals --oracle
python3 tools/rewrite/check_style.py
CTEST_PARALLEL_LEVEL=4 bash tools/rewrite/build.sh all
```

Full phase admission/counters/heading sampling, target references, remaining throttle/maneuver callbacks,
roster mutation, diagnostic presentation, battle, PCM and mission outcomes remain required.
The complete independent ten-run WASM streak remains zero.

Closed [WI 0087](../board/closed/0087_ground-route-progress.md) records exact scope and timings.
Required native/WASM and production-flags address/undefined/leak gates each pass eight groups
without skips: 281524 fixtures, 75076 rejections, 656 repaired full-capacity pop inputs,
205792 complete original safe-domain returns, 960 saved ground records and 944 whole-world
boundaries. Full production verification passes 35 native CTest gates and all 33 WASM Python
suites plus both renderer gates; strict LLVM format/tidy passes all 79 owned C units.
Required selector/goal/start and timed canonical driving regressions pass without skips.
Actual current native SDL and Chromium scenes pass and four before/after images were inspected.
Standard production suites keep their explicitly optional original groups; those optional
skips do not substitute for required coverage. Temporary evidence is summarized under
`/tmp/wasm-fist-0087-review`; raw owned artifacts are removed after verified commit/push.
