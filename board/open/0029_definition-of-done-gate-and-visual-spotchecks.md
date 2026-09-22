Type: feature
Title: The complete port passes the current exhaustive correctness gate

## Contract

On one identified revision, cover every required surface and pass the entire WASM matrix ten
consecutive times, independently confirmed by a subagent. Any failed or incomplete run resets the
streak. Native parity, original references and visual checks are separate required evidence.

## Evidence

Historical 609 log `scratch/oracle/wasm_gate_609.log`: ten clean 178-flow runs. The 610 record has
one complete 178-flow pass on each target, not a fresh ten-run streak. Reported killed background
processes do not prove an OOM cause without system evidence and do not waive the gate.
Missing coverage includes dynamic terrain/projection, all cockpits, mission sample audio, campaign
progression, gameplay input/link, browser pacing and full mission tails (owning items in README).

## Next

1. Build an explicit surface→flow→original-reference inventory. Derive expected flow names from
   that inventory and fail omissions/duplicates; a hardcoded minimum count can hide missing coverage.
2. Harden `tools/wasm_gate.sh`: check process exit status as well as summaries, fix the tested build
   identity, and preserve all ten complete results. Recommendation: bounded foreground chunks may
   form one pass only if every expected flow completes once on the same build.
3. Fill coverage through owning items; add representative visual comparisons for each surface class.
   Palette/frame/audio masks must follow a proved contract, never conceal a failing surface.
4. Run `bash tools/verify.sh both` explicitly: separate native/wasm runs skip cross-target
   comparisons in several branches. Complete parity evidence, then obtain ten clean WASM passes and the
   required independent confirmation. Rebuilds or material matrix changes require a fresh streak.

## Accept

Every surface in AGENTS.md is covered; exact original/native/WASM checks and visual checks pass.
Ten complete consecutive runs on the final revision are retained with no unexplained skips or failures.
Historical passes, prefix parity and an equal verdict cannot substitute for this result.
