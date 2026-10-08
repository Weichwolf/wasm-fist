# Ground target motion

Closed WI0114 proves the complete original target-aware turret wrappers and their
composition after normal-detail ground motion. It extends the untargeted motion
contract; it does not yet change the shared C driver or accept a full class tick.

| Class | Motion | Turret wrapper | Target preparation before slew |
| --- | --- | --- | --- |
| M1 | 7cd5 | 7d0f / 7d1d | Retain heading +9b. |
| M3 | 88e4 | 8917 / 8925 | Retain heading +9b. |
| T80 | 912d | 90cd / 90db | Call complete f69:abd5 to refresh geometry. |
| BMP | 98c3 | 9911 / 991f | Call complete f69:abd5 to refresh geometry. |

All four original class entries run motion before the turret wrapper. When +97
is nonzero, the wrapper stores requested relative turret offset +8b as target
heading +9b minus the **updated** hull heading +26, with word wrap. M1/M3 do not
load the target record at this stage: recomputing their heading here would change
behavior. T80/BMP refresh target heading, elevation +38 and shifted target range
+99 using the existing complete aim owner and the **updated** actor position.
The target height/variant and coarse-angle rules remain owned by that aim contract.

With no target, the existing requested offset survives, including hull-recenter
adjustments made during motion. Every wrapper then limits the signed wrapped
offset difference to364, writes absolute heading as updated hull plus offset,
restores animation selectors to128/0/0 and marks the established class components
and scene refresh when the turret changes. Neither stage consumes RNG or PCM.

The new pure model reuses the proved untargeted motion owner with a settled turret
to obtain motion without introducing a second speed/hull/rotation implementation.
It restores the caller's requested offset, including recenter displacement, then
applies the target-aware stage. Every complete251-byte result is checked against
unchanged actual class methods. Guards also compare the rest of all65,536 DGROUP
bytes, code, text, RNG, actor identity and complete near-return/segment behavior.

Actual release/reallocation exposes the original near-pointer defect at this
consumer too. All four classes retain a nonzero +97 after its physical target is
released. T80/BMP also silently aim at a successor when the same physical slot,
near pointer and saved registry tuple are reused. The canonical C caller must use
the existing captured live-reference owner; neither saved words nor matching
registry tuples prove that a target is still the same allocation.

## Required evidence

Four required groups, no filters or skips, pass279,880 complete original returns:

- All four classes and28 target types, present/absent targets, recenter inputs and
  both angle modes:896 turret and448 ordered motion returns.
- All65,536 retained-heading words for M1/M3 and all65,536 hull words for
  T80/BMP:262,144 complete turret returns, covering every signed slew difference.
- Four256-update independent retained drives with moving targets and interrupted
  target presence:1,024 complete ordered returns. Predicted state feeds the next call.
- All256 motion-flag bytes across six phase gates and both recenter states,
  all256 drive-profile bytes, all256 target-variant bytes in both angle modes,
  and actual target release/reuse:15,368 complete returns.

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python \
  tests/test_original_ground_target_motion.py --originals \
  --review-dir /tmp/wasm-fist-0114-review
```

The compact receipt is /tmp/wasm-fist-0114-review/target-motion.json. A separate
acceptance receipt retains exact source/reference pins, terminal results and log
hashes. Full living-class callbacks, collision, engine PCM, canonical C integration
and actual playable-mission acceptance remain open under0081/0065/0041.
