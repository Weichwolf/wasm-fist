Type: Work item
Title: Prove target-aware turret order for all four ground classes
Depends: 0061, 0097, 0113

## Contract

Recover the complete class-specific target stage between original motion and
turret slew. Check full actor results and unrelated world preservation with actual
unchanged near/far routines, retained predictions, heading/branch domains and
real physical target release/reuse. This original checkpoint does not accept a
shared C target-motion caller or complete living class.

## Evidence

Required four groups finish with terminal0 and no skips:279,880 complete original
returns. All four class entries execute motion before the turret wrapper. M1/M3
use retained +9b; T80/BMP call f69:abd5 and refresh heading/elevation/range using
the moved actor position. All classes subtract the updated hull heading before
slew. Refreshing every class's aim before motion would violate this behavior.

Coverage includes all four classes/28 target types, null/present targets, both
angle modes, hull recenter, all65,536 relevant heading words per class, four
independent256-update retained drives with moving targets, every motion-flag,
drive-profile and target-variant byte and actual allocation release/reuse.
Full251-byte actor results, all unrelated DGROUP bytes, immutable code/text,
scene refresh, actor identity, RNG and complete returns/segments are verified.

The same saved near pointer and registry tuple can identify a different physical
allocation. Actual T80/BMP silently aim at that successor; M1/M3 continue using
retained heading. Future shared consumption must apply the existing captured
live-reference repair before the used-target branch, without refreshing a lost
target's retained feedback. See docs/ground-target-motion.md.

Output SHA-256 per required group:

- Types/presence:bf2fe50f1a39c32a9aea4d3a595935d8649fdfb6a472c618ae8321af0faee12d.
- Heading domains:788635d314abbde4918358cf92e327ed48acf6735248f304c83a4f626127d03e.
- Retained motion:7e0ad0578ee7d49a7c178c06e982d3cbc09aa17261c3a7c7bf6473af39c1dd29.
- Branches/lifetimes:a2173715511fbaccb6ffb48a933d36df0d882cd87b871ff0db04a69d301d7f63.

Only Python original/model code and documentation are published. The current
local parent C/probe/configuration changes remain excluded and unaccepted. Their
full production regression remains live under session57204; heading domains and
full instrumented corpus have completed as recorded in active0081. Frozen refs,
softgl and original files remain unchanged. Complete-game WASM streak stays zero.

## Next

Consume the complete target-aware stage with one motion and one aim owner and
captured physical lifetimes. Preserve the untargeted API and its existing tests.
Then finish the remaining actual class callbacks and caller globals before
accepting canonical command/class integration under0081.

## Accept

Run all four required groups without filters or skips, compare every complete
return and retain independently predicted next-step state. Missing inputs/output,
partial returns, unrelated writes and changed source pins fail. Reproduce with:

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  tests/test_original_ground_target_motion.py --originals \
  --review-dir /tmp/wasm-fist-0114-review
```

Terminal results, source/reference pins and compact log history are under
/tmp/wasm-fist-0114-review/target-motion.json and acceptance-receipt.json.
