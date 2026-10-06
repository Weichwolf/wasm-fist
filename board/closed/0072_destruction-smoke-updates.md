Type: Work item
Title: Consuming wreck/target/artillery smoke and complete drifting-smoke lifecycle
Depends: 0070, 0071

## Contract

Consume retained type-23 wreck, destroyed type-26 modes 4..7 and complete type-27 update states
through their actual smoke emission/counter/flag rules. Restore the newly reached typed fields
without retaining snapshots. Supply exact low-priority type-17 smoke creation and complete
wind/rise/animation/release behavior, sharing word-counter animation with existing type 18.
Reuse pool/random/pose owners. Preserve disabled/exhausted emission's RNG and parameter rules.
Keep persistent original wrecks/targets; do not invent retirement. Aircraft 5/6 updates, live
type-26 firing, selected-player/world/input/render/audio/AI/outcome consumers remain required.

## Evidence

Untouched b355 increments word +1b, tests its low six bits, emits 19caa when unsigned +1d>128,
then decrements +1d even if smoke is disabled/exhausted; mode 1 clears flag masks 0x06/0x18 and sets 1.
bc0c sets flags 0x40/secondary 0x44, increments word +1f and emits when its low six bits clear and
unsigned +21>128, without decrementing that parameter. bc46 destroyed modes 4/6 increment byte
+1e and emit on low-six-bit zero and unsigned +1c>128, then decrement +1c by one and by four
more if the updated value exceeds 768. Modes 5/7 execute the actual side-bit OR only.
Physical 19caa (f69:a61a) requires byte 8b4f exactly 1 before low-priority type-17 admission;
only successful allocation consumes RNG. Extent=(parameter+(random&63)) modulo word, scale
is extent shifted left two modulo word, XYZ copy the source with wrapped Z+768; heading/flags/
frame/counter and other constructor bytes remain zero. 9b11 uses dword wind 92f2/92f6, adds
eight to the altitude LOW WORD without propagating carry, and animates every 12 updates through
frame 30 before actual find/release. Disabled smoke releases before drift/counters. Verify all
claims against complete original execution; patches only locate evidence.

## Verified delivery

Verified on 2026-10-07. Shared `sim/destruction_updates` owns complete bc0c/b355 and destroyed
bc46 modes 4..7. `sim/smoke` owns exact low-priority type-17 creation and complete 9b11 wind,
low-word-only rise, animation and natural release. Existing type-18 word-counter/frame logic
uses the same `sim/smoke_animation` owner, retaining its distinct constants. Short snapshot
restoration copies newly reached counters and all owned fields before input release. Original
wrecks/targets intentionally remain allocated; smoke naturally releases. Type 27's emission
word +1b survives critical damage, which resets the separate word +1f. Type-26/type-27 parameter
decay remains correct with disabled/exhausted smoke. No presentation/device flow changes.

`docs/destruction-smoke.md` records the actual instructions, scope, caller boundaries, counts,
reproduction commands and explicit live-world scheduling dependency. The original gate executes
complete creation/class/smoke methods, real critical damage and post-damage impact/cleanup;
all unrelated imported payloads, including orphans, remain unchanged. Direct creation preserves
its complete emitter. The source shell remains entirely unchanged through damage, then releases
and may be reused. Newly created smoke is checked as a complete 55-byte payload. Its scripted
parent→physical-smoke→impact-effect order is an integration fixture, not the original world pass.
Rechecked c105..c120 instead traverses live registry entries in increasing registry order,
allowing later-entry allocations to update in the same pass; live integration remains required.

- `bash tools/rewrite/build.sh all`: all 22 native CTest gates and the complete WASM build
  gates pass. Native CTest elapsed 90.76 seconds. Default gates deliberately omit separately
  requested original corpora; the following required corpus gates have zero skips.
- `python3 tools/rewrite/check_style.py`: strict LLVM 19.1.7 format/tidy passes all 61 C units.
  Initial probe diagnostics were fixed with named offsets/explicit arithmetic and snapshots
  of bytes from the same object before invalid calls. No rule or warning was disabled; copied
  C-structure padding does not determine equality.
- Required `test_destruction.py --originals --oracle`, native: all six groups pass in 199.296
  seconds. WASM: all six groups pass in 299.415 seconds. Each target covers 9,135 fixtures,
  9,166 parent updates and 14,701 complete smoke states. Required corpus: 317 unchanged saved
  targets (17:28, 23:47, 26:6, 27:236) plus 796 constructed critical type-26 successors; all
  1,113 cases retain all 47 complete occupancy contexts. Constructed successors change the
  initial damage byte to 99; they are explicitly distinguished from unchanged save states.
- New production-flags ASan/UBSan/LSan destruction probe: all six groups, same complete corpus
  and totals pass in 12.547 seconds without skips. Shared muzzle/flight sanitizer regression:
  all seven groups, 37,947 fixtures/41,925 updates and complete 179-M1/47-world corpus pass in
  14.911 seconds without skips.
- Full both-target original flight/effect regression: all seven groups and the same 37,947/
  41,925 totals pass in 469.255 seconds without skips. Full both-target remaining-damage
  regression: all nine groups pass in 475.232 seconds, 14,510 fixtures/17,218 damage/14,510
  impact/24 flight/1,920 animation observations per target, with all 1,085 current targets
  and all 47 complete occupancy/roster contexts; zero skips.
- Full both-target original ground-damage regression: all nine groups pass in 546.614 seconds,
  22,969 fixtures/25,499 damage/22,969 impact/2,736 retirement updates and one reaching
  pipeline per target, all 960 current ground targets and all 47 worlds; zero skips. Its
  original full wreck constructor continues to prove constructor-zero emission state. This
  ground pipeline does not yet schedule the separate wreck/smoke update owner.

Complete new counter/frame/quality/flag domains and declared word/parameter/wind/wrap/capacity
boundaries, natural lifetimes and critical-hit→emission→cleanup sequences pass. Atomic invalid
owner/stale/null/output checks and source-release independence pass. New simulator code remains
C11, shares pool/random/pose owners and does not link the frozen engine. Original scenarios and
reference remain read-only. Temporary inputs/logs/sanitizer binaries stay under /tmp; clean them
after commit/push and retain compact hashes/results in /tmp/wasm-fist-0072-destruction-review.

## Next

Continue active 0065/0041 with aircraft 5/6 death, live type-26 firing and selected-player loss
flows as reached; install delivered typed actors/effects into the original mutable registry
update order. Recover fire eligibility/input and deliver actual visible battle and audible PCM,
AI, objectives/outcomes and a complete first playable mission. Preserve all later all-function
and final independent ten-full-WASM requirements. This bounded numerical delivery supplies
neither a complete world pass nor a complete playable mission.

## Accept

Both targets pass complete declared state/counter/byte/wind/quality/capacity/sequence coverage
and required original corpus with no missing output, unequal lengths, skips or incomplete runs.
Actual original methods support full modeled state, unrelated payload preservation, exact
effect construction, pool identity/random ordering and natural lifetime. Invalid/stale/wrong-
owner inputs fail atomically. Required builds/style and production-flags memory/leak gates pass.
Frozen reference and originals remain pristine; bounded success is committed/pushed and cleaned.
