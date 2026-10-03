# Work queue

Read `../AGENTS.md`, then the current WI and dependencies. Directory is state: `open/active/closed`.
One developer owns every WI. `active/` marks current focus, not a proven diagnosis.
Follow explicit priorities first; otherwise continue 0034.

## Workflow

1. Reproduce **Next** on the current code. Historical evidence requires its recorded revision.
2. Find the first differing output; recover the producer contract from the original.
3. Add the reaching regression and fix the cause. Implementation recommendations remain hypotheses.
4. Run `bash tools/check_flow.sh '^flow-name$'`; omit the filter for the entire existing matrix.
   It runs tests, exact patch checks, both builds and cross-target comparisons. Keep artifacts under
   `/tmp/wasm-fist-<uid>-<workspace>/verify/run.*` (override with `FIST_WORKDIR`).
   Do not edit running scripts or overwrite tested binaries.
5. Keep disposable builds, captures, isolated games and logs under `/tmp`. Delete obsolete raw
   artifacts after each bounded success and retain compact result summaries; temporary files may
   disappear between sessions, so version the source fixture and reproduction command.
6. Record command, revision, result and evidence path. Complete required coverage, commit and push.
   Close only when **Accept** is proved; matching prefixes and historical gates do not establish it.

## Contracts and work order

Each contract has one implementation path and one WI recording its proof. Other WIs consume that
evidence. IDs divide contracts, not developers. Dependencies order execution.

| Order | WI | Contract / supporting items |
| --- | --- | --- |
| 1 | 0034 → 0036 → 0012 | Complete synchronized capture → attributed start state → full-run parity; strict checks 0033. |
| 2 | 0001 | Whole windshield, all missions/detail/night modes; terrain stage proofs 0002. |
| 2 | 0027 | All vehicle consoles, instruments/radar and loss/switch transitions. |
| 3 | 0017 | Simulation, objectives, resolved outcomes/debrief; recover guest/host address semantics. |
| 4 | 0003 | One final PC-speaker/OPL/SB mixer for WAV, sequence and browser; content fixtures 0011. |
| 5 | 0026 | Shared PIT/retrace/interrupt/I/O time contract and browser pacing; supplies 0034/0003. |
| 5 | 0004 | Profiles, campaign progression and persisted reload behavior. |
| 5 | 0005 | Gameplay input/link; keyboard/mouse 0030, analog joystick 0031, serial 0032. |
| As reached | 0009/0010/0014/0015/0019/0022/0023/0025/0028 | Services, dispatch, segments, widths, pointers and memory ownership. |
| As reached | 0013/0035 | Code/memory diagnostics; Oracle capture 0024 delivered. |
| Final | 0029 | Every menu/screen/dialog/setting and editor create→save→reload→simulate; surface inventory, visuals and ten-run WASM gate. |

Compare complete indexed frames, all palette entries, timing and mixed PCM under matched inputs.
Memory traces remain diagnostic. Editor/persistence files retain their round-trip contracts.
Equal verdicts, silent WAVs, histograms, no-trap samples and masked/prefix comparisons prove only their scope.

## WI format

RFC 822 header: `Type`, `Title`, optional `Parent`/`Depends`; blank line, Markdown body.
One stable ID in `board/*/NNNN_*.md`; no duplicate Status field. Keep **Contract**, current scoped
**Evidence**, ordered **Next**, **Accept**. Split independently deliverable work; link its owner instead
of copying its recipe. Closed items retain bounded proof and follow-up owner. Mark disproven bugs.

Pre-consolidation detail is retained at `7fb23e5f26716b0dd1d90c30b3fb7a8807ac9bee`:
`git show <revision>:<path>`. IDs survived; 0012 reopened for unproved full-run coverage, 0030–0032
split 0005. Retired `0012_fire_cascade_reference.md` sketches are superseded by 0016/0017 and patches.
