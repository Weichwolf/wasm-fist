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
Project audit at `bf26d8f` found that `tools/wasm_gate.sh` used an absolute checkout path and accepted
any clean result with at least 176 PASS lines. The gate now derives the root, pins the revision and
WASM/tool hashes, rejects tracked changes, clears flow filters, checks producer status plus an exact
dynamic summary, and retains every run under one evidence directory. `tools/consecutive.sh` owns this logic;
`wasm_gate.sh` only selects WASM. No historical streak was carried forward.
Build review found C/C++ failures could reuse stale objects when stderr lacked `error:`.
Node/browser builders now require a successful compiler exit; `tests/test_build.py` reaches all
four failure paths. Source inventory and warning policy remain duplicated across the three builders.
Real node/browser builds pass (`scratch/review/build-integrity/`); node WASM bytes are unchanged.
All 32 tool tests pass (`scratch/verify/run.we53cO/tests-final.log`). The same engine binaries pass
the complete existing 178-flow cross-target matrix; original sequence/coverage requirements remain open.

## Next

1. Build an explicit surface→flow→original-reference inventory. Derive expected flow names from
   that inventory and fail omissions/duplicates; a hardcoded minimum count can hide missing coverage.
2. Fill coverage through owning items; add representative visual comparisons for each surface class.
   Palette/frame/audio masks must follow a proved contract, never conceal a failing surface.
3. Split compiler policy between generated engine C and hand-written shims. Remove current shim
   warnings, then compile shims with `-Wall -Wextra -Werror`; retain only documented generated-code
   suppressions for the mechanical decompile. Share the source inventory; keep native/node/browser
   optimization, diagnostics and linker configuration explicit.
4. Run `bash tools/verify.sh both` explicitly: separate native/wasm runs skip cross-target
   comparisons in several branches. Complete parity evidence, then obtain ten clean WASM passes and the
   required independent confirmation. Rebuilds or material matrix changes require a fresh streak.

## Accept

Every surface in AGENTS.md is covered; exact original/native/WASM checks and visual checks pass.
Ten complete consecutive runs on the final revision are retained with no unexplained skips or failures.
Historical passes, prefix parity and an equal verdict cannot substitute for this result.
