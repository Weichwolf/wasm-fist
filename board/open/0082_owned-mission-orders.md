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

Recover complete record access/append/count and descriptor admission rules before choosing
typed validation. Implement owned order decoding/retention, source-release and malformed-input
atomicity, then canonical installation/observation through the shared mission caller. Verify
all eight records, full-width signed coordinates, retained unknown/header/unused bytes, every
count boundary, complete corpus and original DOS loader evidence. Preserve actual scenes.
Return to active 0081 for complete heading/RNG/nested command consumption; no partial callback
or full living-method acceptance follows merely from owned order input.

## Accept

Both production targets pass complete owned-order/canonical installation transcripts and
original corpus/reference checks without missing bytes or skipped inputs. Source bytes can be
freed/reused before observation; invalid data publishes no partial state and does not consume
RNG. Required full builds/style, meaningful memory and current scene regressions pass. Complete
command/AI/world/PCM/playable-mission and final ten-run requirements remain open under 0081/0041.
