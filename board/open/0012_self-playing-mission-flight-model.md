Type: feature
Title: Every mission remains native/WASM identical through its resolved outcome

## Contract

Run every battle to the engine's own outcome with matched initial state, timed input and devices.
Compare complete presented-frame and PCM sequences with their timing. Internal state hashes localize
defects; their equality is not an additional acceptance requirement.

## Evidence

- Patch 609: all 47 battles resolved natively (board:0017).
- Patch 610: historical per-tick SIMHASH matched all 47 through tick 2500; AZER1 additionally
  matched through resolution (6978 logged ticks, same DEFEAT). This does not prove the other tails.
- Reopened during consolidation: the former closure extrapolated a prefix to untested suffixes.
- `FIST_SIMHASH` hashes DGROUP 0x9000..0xefff plus selected clip bytes, not all engine state. Low
  DGROUP is largely excluded; hash equality does not prove complete RAM/frame/audio equality.

## Next

1. Make a persistent comparison harness around `tools/selfplay.sh both` and `FIST_SIMHASH`.
   The existing `both` verdict comparison alone is insufficient; retain each target's ordered logs.
2. Use an explicit monotonic step/phase key. DGROUP:0x452 is 16-bit and resets at phase changes;
   do not align streams solely by its value. Fail missing/extra samples as well as differing ones.
3. Run all battles to `FIST_STOP_ON_OUTCOME=1` with isolated data directories and watchdogs.
   At the first hash mismatch compare raw state; use `FIST_SIMDUMP` for a bounded reproduction.
4. Add framebuffer/DAC and audio comparisons at deterministic boundaries; audit excluded state
   whose values can affect output. Correct active UB from the asm, not compiler flags.

## Accept

All battles reach outcomes on both targets; complete presented-frame and PCM sequences, timing
and verdicts match. Internal representation differences alone do not fail acceptance. Record exact coverage and revision. Follow with board:0029's current endurance
gate; oracle fidelity and relocated-state residuals remain board:0017.
