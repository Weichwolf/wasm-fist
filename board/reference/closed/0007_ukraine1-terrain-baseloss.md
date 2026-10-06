Type: bug
Title: The observed mission base-loss cascade is repaired

This engine can run the recorded battle sweep without the observed near-offset/host-pointer crashes.

## Evidence

On the patch-609 tree, `scratch/oracle/sweep609.log` records all 47 battles reaching outcomes without
crash or timeout. `scratch/oracle/asan_rep.log` records full-resolution ASan runs for AZER1, CYPRUS1,
INDIA1, SAUDI1, SYRIA1, TRAIN1, UKRAINE1, UKRAINE6 and SAUDI6 without diagnostics.
These samples close the observed cascade; they do not prove every possible path memory-safe.

## Preserve

- Indirect calls need the target's actual register contract; a positional argument may be stack
  residue, not a polymorphic pointer. Inspect both caller and callee before rebasing.
- Restoring a producer can expose a previously unreachable broken consumer. Audit the consumer too.
- Run `make patch` before direct build scripts; otherwise a patch experiment measures old code.
- A tick cap can exit in the intro. Require evidence that the tested mission path executed.

## Follow-up

Use `tools/asan_selfplay.sh` for regressions and full-sweep extension. Broader dispatch and object
contracts remain board:0014, board:0015 and board:0019; full cross-target runs remain board:0012.
