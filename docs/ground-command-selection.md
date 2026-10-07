# Ground command mode selection

The shared C owner implements the complete original ab82 return: far f69:b4ef,
raw 1ab7f..1ac09. The parent ab03 phase remains pending under WI 0081. This helper does not
advance counters, sample heading history or replace other command callbacks with returns.

The canonical world owns the installed platoon descriptor and RNG. The caller supplies the
already consumed parent-phase random word. Saved mode (+43), maneuver (+45) and target-reference
(+97) survive class initialization and are owned values. The target reference is tested for
presence only here; it is not dereferenced or interpreted as a physical slot or C pointer.
Fourteen of the 960 original saved ground records contain a nonzero reference.

The complete ordered selection is:

| Condition, first match wins | Mode |
| --- | --- |
| Controlled, control bit 1 clear | Leader 0, follower 2 |
| Automatic and behavior word +0 equals 3 | 14 |
| Motion flags +19 contain either bit in 6 | 12 |
| Control flags contain 10h | 10 |
| Maneuver byte +45 nonzero | 8 |
| Control flags contain 40h | 4 |
| Control flags contain 20h and parent random low byte <= damage chance | 4 |
| Target reference nonzero and additional random low byte < target chance | 6 |
| Otherwise | Leader 0, follower 2 |

The 20h branch clears only that bit whenever reached, including a failed chance comparison.
Earlier priority branches preserve it. The target branch calls original b26a -> 0291, consuming
one additional value from the same world RNG, even for zero target chance. Equality is accepted
for the damage comparison and rejected for the target comparison.

Original DS:9946 damage chances are 100/10/200/0, indexed by behavior word +0. DS:9956 target
chances are 0/50/255/0, indexed by waypoint-mode word +2. Original UI 620c/621c and 626f/6282
increments/decrements each word with wrap at four; executable tails independently prove the
four-choice domain. The owned wire decoder retains every full-width descriptor word. This
consumer rejects out-of-domain words only when their table lookup is reached. Earlier branches
may safely consume retained unknown values without inventing a clamp. A rejected transaction
preserves the complete world, including a provisional cleared damage bit and RNG.

`original_ground_command_oracle.py` executes unchanged ab82 and both nested original returns,
checks all 251 actor bytes and the actual RNG state, and checks all unrelated DGROUP bytes.
Only actual call-stack and random cursor/word/result writes are excluded from the latter check.
Host setup provides declared actor/descriptor/parent-random inputs; no instruction or result is
substituted. UI checks execute the complete original mutation tails, not the surrounding device UI.

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tools/rewrite/test_ground_command.py \
  --target all --originals --oracle
python3 tools/rewrite/check_style.py
CTEST_PARALLEL_LEVEL=4 bash tools/rewrite/build.sh all
```

Production probes release overwritten source bytes before observing owned runtime state.
The canonical-world gate loads real TRAIN1 orders and all 85 original objects, checks complete
world observations after each selector return and preserves all unrelated payloads and orders.
Complete heading/command scheduling, target resolution, navigation, PCM and playable battle
remain unproved. The final complete WASM streak remains zero.
