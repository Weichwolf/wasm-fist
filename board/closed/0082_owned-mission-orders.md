Type: Work item
Title: Own mission platoon descriptors and complete waypoint input
Depends: 0080

## Contract

Decode and own complete original PATH/PINF mission-order input in readable shared C11 before
consuming 0081's nested command callbacks. Use actual eight-platoon pointer/record evidence;
retain every header byte, descriptor word and coordinate slot without source aliases or
invented unused semantics. Connect the canonical mission's order owner at its real load
boundary, with explicit failure and source-release behavior. Keep saved actor/roster/RNG
installation owners and existing control/render paths. Do not fabricate empty orders when
required original chunks are absent or malformed.

## Evidence

Actual original FSG dispatch maps PATH to d87f and PINF to d8a9. Read mode one loads complete
blocks to DS:7d40 and DS:85b6; the actual platoon pointer tables identify eight 268-byte routes
and eight 22-byte descriptors. PATH byte zero counts entries; goal XY begins at +12, with
32 eight-byte coordinate slots in each allocated record. Eleven other header bytes and every
unused slot remain saved data. PINF contains eleven words per platoon; actual commands consume
behavior, waypoint mode, formation and throttle selectors, with other semantics still pending.

`test_original_command_boundary.py --originals` passes complete original read/seek handler
returns for all 47 inputs, whole-DGROUP preservation and unchanged RNG, plus actual callback
banks and a real TRAIN1/constructed stale-goal reaching consumer at updates 15/23. The old
isolated DCBS setup retains zero PATH/PINF and is insufficient for later command acceptance.
See docs/ground-command-phase.md. Existing envelope views are borrowed and opaque, not owned
orders, so simply exposing those pointers does not satisfy the runtime contract.

## Next

Owned decoding and canonical installation are accepted below. Return to active 0081 for
complete heading/RNG/nested command consumption using the canonical order owner. Preserve
saved header/unused data until actual consumer evidence identifies its semantics. No partial
callback or full living-method acceptance follows merely from owned order input.

## Accept

Both production targets pass complete owned-order/canonical installation transcripts and
original corpus/reference checks without missing bytes or skipped inputs. Source bytes can be
freed/reused before observation; invalid data publishes no partial state and does not consume
RNG. Required full builds/style, meaningful memory and current scene regressions pass. Complete
command/AI/world/PCM/playable-mission and final ten-run requirements remain open under 0081/0041.

## Recovered capacity and implemented owner

The actual editor append prefix 4de6..4e08 compares route count unsigned with 20h and refuses
an append at count >=32. For count <32 it selects route +12 +count*8. The unchanged admission/
address instructions pass all 2048 platoon/count boundaries, stopping before XY copy/UI work
or the refused return. Therefore the decoder admits saved counts 0..32, not 33..255. This is
bounded count/address evidence, not a complete editor-action claim. PINF selector words remain
full-width; the decoder does not derive admission from the shipped corpus's narrower values.

`src/assets/orders.c` owns every header byte, all 32 coordinate pairs and all eleven descriptor
words for all eight platoons. Publication is transactional and retains no source pointers or
private RNG. Canonical `fist_driving_load_mission` installs that data in `world->orders` after
saved-object installation and before selected control/contact/models. Payload-only world load
explicitly leaves `orders_loaded` zero. Reset clears both the flag and full data. Common probes
observe complete orders after source overwrite/free and include them at every canonical timed
boundary. Existing combat/saved-object tests explicitly observe their unloaded subset boundary;
prior payload/roster/RNG expectations are unchanged. See docs/mission-orders.md.

## Verified acceptance — 2026-10-07

- Current production `CTEST_PARALLEL_LEVEL=4 bash tools/rewrite/build.sh all` exits zero:
  31 native CTests, 29 WASM Python groups and both WASM renderer/pixel gates pass. Builds remain
  sequential; only independent native tests run concurrently. Both production Ninja trees are
  current. Required LLVM 19.1.7 formatting/tidy passes all 74 owned C translation units; warning,
  analyzer and formatting policy is unchanged.
- Required `test_orders.py --originals --oracle` passes all five groups on both targets with
  zero skips: 101 fixtures / 197200 complete valid bytes per target, including all 47 originals /
  109040 actual order bytes. Every valid fixture checks all 2048 count/platoon inputs, all 2320
  truncated block lengths, oversized/null views and complete failed-output preservation. Full
  signed coordinates, every retained header byte value, unknown full-width words and unused
  slots remain unchanged after source overwrite/free. Actual complete DOS loaders agree, preserve
  whole DGROUP outside declared destinations and leave RNG unchanged. The original editor's
  actual bounded admission/address instructions pass all 2048 count/platoon boundaries.
- Required canonical driving original gate passes seven groups, zero skips on both targets:
  123 fixtures / 924 complete timed boundaries / 907 installed objects / 60 explicit rejections.
  Four nonzero order inputs cover counts 0/1/31/32, every retained field, source release, repeated
  updates and pause. Fourteen malformed orders fail without publishing a session or consuming
  the caller's initial RNG. All ten supported original contexts and 37 whole-world unsupported
  rejections retain their previous requirements; neither actors nor records are filtered out.
- Required saved-object world original gate passes six groups, zero skips on both targets:
  900 fixtures / 4979 installed objects / 306 explicit rejections / 65569 tree requests. Complete
  physical payloads, registry orphans, roster, file-order initialization RNG, reload and reset
  remain required. Unloaded orders are observed explicitly at this subset boundary.
- ASan/UBSan/LSan with production fast-math passes all required order, canonical and world
  original gates with identical coverage and zero skips: five/seven/six groups respectively.
  No memory diagnostics, incomplete runs or hidden missing inputs are accepted.
- Actual isolated original TRAIN1 native SDL and Chromium mission scenes pass complete frames,
  held control/turret/weapon input, reload/cycle, pause, focus and shutdown. Chromium additionally
  compares every canvas pixel with the complete C framebuffer, worker isolation and failed-start
  cleanup. Both select canonical physical player 151. Reviewed before/after PNGs retain upright
  sprites, terrain and shared HUD; the browser after-frame shows independent turret rotation.
- Original command-boundary regression passes three groups, zero skips, all 47 inputs / 376
  complete read/seek returns / 109040 bytes. The existing first-difference and explicitly
  constructed stale-goal prefix evidence is retained; no C command callback is claimed here.

Initial style findings were fixed with explicit constant parentheses, a direct owner include and
small separate probe helpers, without weakening checks. The new orders observation was added to
an initially stale combat transcript; all previous combat expectations remain. An isolated preview
server lacked index.html, then a concurrent browser run timed out; the complete final isolated run
passes unchanged criteria. SIGTERM-interrupted long runs, including a world run with partial green
output, were rejected and repeated to terminal zero. Full final build/source evidence is current.

Reproduction lives in docs/mission-orders.md. Compact `/tmp/wasm-fist-0082-review/summary.json`
retains exact gate results, source/binary/log/image hashes and limitations. After verified
commit/push, remove owned raw logs, captures, copied assets and sanitizer build. Original files,
frozen generated/reference work and dependency pins are unchanged. Active 0081/0041/0065 retain
complete commands, living battle, PCM and outcomes; the final complete WASM streak stays zero.
