Type: bug
Title: Mission simulation reproduces original visible and audible behavior
Parent: 0012

## Contract

With matched initial state, input, RNG and scheduling, reproduce the original's presented frames,
PCM stream, timing and resolved outcome. Internal differences matter when they change those outputs.

## Evidence

- Patch-609 native sweep (`scratch/oracle/sweep609.log`): 43 DEFEAT + 2 VICTORY (SAUDI4/TRAIN2)
  + 2 TIME EXPIRED (SAUDI6/SYRIA7) = 47 resolved battles; not a current cross-target proof.
- Seeded replays compared RNG, objects, shells, LOS and rotations across several theatres. On 610,
  AZER1 fixture `scratch/oracle/regtrace_rng13/regs.txt` matched the recorded draw stream for 2726
  port steps. Check sample coverage: the fixture has 2727 oracle steps.
- Two recorded residuals read code-vector bytes as data: targetless idle bearing reads DGROUP
  4/8/c; player-hit handling consumes BX clobbered by the sound callout and reads [channel+4].
  Relocated oracle segment words differ from flat port words, changing headings/hit thresholds.
- Earlier “outcome-neutral, inherently acceptable” conclusions are withdrawn. Equal verdicts do
  not satisfy the framebuffer/audio target, and the port's representation is an implementation choice.

## Next

1. Reproduce one residual with matched original inputs. Prefer UKRAINE2's early player hit;
   preserve the BX trace, affected vector bytes and first changed object field.
2. Recommendation: separate engine-visible guest addresses from host dispatch addresses. Preserve
   original guest-visible segment words; translate them at the call/memory boundary. Prototype the
   smallest affected path and prove pointer reads, calls and data reads all retain their meaning.
3. Do not whitelist the resulting heading/hit differences or add a no-target guard absent from the
   original. Match even accidental original reads when they affect observable behavior.
4. Re-run seeded replay with strict missing/extra-step accounting; extend to all missions and
   victory/defeat/timeout/abort transitions, objectives, cockpit switches and debrief statistics.

## Accept

Complete frame/audio sequences and timing match the original, including scenarios reaching both
relocation cases. Use state/RNG traces to diagnose output differences, not to require identical host
representations. Native↔WASM
full-run coverage is 0012; preserve shared regression fixtures rather than duplicating its harness.

## Tools and traps

`tools/oracle/replay_mission.sh <BATTLE>` collects a fresh capture; `replay_sched.py`,
`replay_run.sh`, `replay_objdiff.py`, `replay_drawcmp.py`, `replay_rotcmp.py` inspect it. Read reports:
wrapper success alone is insufficient because some internal failures are tolerated by the scripts.
`FIST_DUMP_REG` is one-shot, not a timed second snapshot; use step dumps or `FIST_SIMTRACE`.
