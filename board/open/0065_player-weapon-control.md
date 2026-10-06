Type: Work item
Title: Primary player weapon control and ammunition/reload feedback
Depends: 0064

## Contract

Extend the real shared driving session toward its first playable mission with recovered primary
player weapon selection, firing eligibility, ammunition consumption and reload timing. Show
verified weapon/ammunition feedback in both continuous scenes. Recover actual weapon slot/type
and input contracts before choosing behavior; do not infer them from initialization counts or
invent the meaning of the common trailing parameter 20. Share state, input and event ownership.
Projectile/hit rules and sound must follow their recovered contracts, not silent placeholders;
retain explicit scope until those stages are delivered.

## Evidence

0064 supplies a continuous real player/terrain/model scene on native SDL2 and browser, shared
rational PIT clock, manual controls and current render inputs. 0060 retains class ammunition,
cycle/stock and trailing parameters with their still-unrecovered firing/reload identities.
The driving scene has no weapon command, feedback, projectile, hit, audio or outcome behavior.

## Next

Recover primary slot/type, command eligibility and timed fire/reload from original class/input
routines and real mission state. Reproduce reaching fire/no-fire/reload boundaries. Implement
owned shared state transitions and truthful visible ammunition/weapon feedback, replay timed
inputs on both targets and verify complete output/state/error behavior, visuals and memory.
Keep 0041 active until projectiles/hits, objectives/outcomes and audible events form a complete
playable mission; continue subsequent bounded stages without substituting invented rules.

## Accept

Both running platforms handle primary player weapon commands, ammunition and reload timing
correctly for the recovered bounded contract, with visible verified feedback, meaningful timed
behavior tests, strict tooling and actual native/browser evidence. This item alone does not
claim complete combat, all weapons/classes, mission objectives/outcomes or audio delivery.
