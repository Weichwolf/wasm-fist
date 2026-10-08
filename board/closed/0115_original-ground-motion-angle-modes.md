Type: Work item
Title: Prove both angle modes and self targets in ordered ground motion
Depends: 0114

## Contract

Extend the complete original target-motion gate to both angle modes, self targets
and signed coordinate overflow. Keep all four earlier required groups and their
independent full-state expectations. The shared C implementation remains a separate
acceptance step.

## Evidence

The strengthened five-group gate finishes with terminal0 and no skips:311,800
complete original returns. The four earlier groups retain their exact output
digests from0114. The added group checks31,920 complete ordered motion/turret
returns for all four classes, present/absent and self/other targets,19 heading
edges, seven signed speeds and five motion directions. Each angle selector0/1/255
has10,640 returns;7,980 returns have a present self target. Both coordinates
start at signed32-bit extrema and cross the wrap boundary as applicable.

The pure model reuses the established rotation owner for coarse translation and
updates the actor record seen through a self-target alias before fresh geometry.
Unchanged actual class routines confirm every251-byte result, scene refresh,
return/segment contract, RNG and unrelated memory. No missing call, substitute
return or original instruction change is used.

The added group output SHA-256 is
8c52b41d4db3f40d89cd34c42fbcaa2f27ccff9adf3ba2b7ea5121d917320561.
Source pins and complete group results are in
/tmp/wasm-fist-0115-review/target-motion.json and acceptance-receipt.json.

The preceding complete production regression is terminal0:47 native CTests and
the entire WASM suite. Its compact receipt is
/tmp/wasm-fist-0081-review/owned-parent-production-all-development.json.
Subsequent local C changes split the existing motion owner into drive and turret
stages and compose class-specific aiming using captured targets. The first native
development comparison passes10,032 complete owned-state results, including both
angle modes and self targets. These C/probe changes are excluded from this
reference publication; their full canonical, lifetime, failure, style, production
and memory gates remain required. Full0081/class/battle/PCM acceptance stays open.

## Next

Finish the shared C target-motion implementation and its required both-target
gates. Preserve one movement and one aim owner, retained M1/M3 feedback, moved
T80/BMP geometry, self aliases and captured allocation identity. Continue the
remaining class callbacks and canonical command scheduling under0081.

## Accept

All five required groups finish without skips, retain the prior coverage/digests,
compare complete original results and verify source hashes before/after:

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  tests/test_original_ground_target_motion.py --originals \
  --review-dir /tmp/wasm-fist-0115-review
```

Frozen references, softgl and read-only originals are unchanged. Completed owned
logs are removed after recording compact results/hashes. Complete-game WASM
streak remains zero.
