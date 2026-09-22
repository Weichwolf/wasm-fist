# Armored Fist — working rules

Target: original DOSBox/QEMU, native and WASM produce identical presented frames and PCM audio for
matched initial state, timed input and device configuration. Compare complete sequences and timing;
internal memory/write traces are diagnostic, not an independent equality requirement.

- Start at `board/README.md`; take one bounded step from its current WI. Verify old claims against
  current code and evidence. Update the WI instead of appending session transcripts.
- Find the first differing output, trace its producer, recover the original register/segment/width/
  flag contract. No guessed constants, missing-work stubs, output masks or guards that hide defects.
- Generated engine C in `re_out/` stays pristine. Corrections are asm-backed `patches/NNN-*.diff`
  with rationale; `build/` is disposable. Hand-written shim and tools may be edited directly.
- Use local binaries/source first. Oracle: `third_party/dosbox-fist`, source `third_party/dosbox-build/`.
  `FIST_WATCHFLAT` follows guest paging; port DGROUP `0x1c000` is not guest physical `0x1c000`.
- Keep `armoredfist/` read-only; use isolated copies for runs. Match capture boundaries before diffing.
- Prove a reaching failure, fix it, run its regression on both targets. `make check` verifies patches;
  `make native`, `make wasm`, `bash tools/verify.sh both` verify builds and explicit cross-target parity.
  Filtered runs are partial evidence; complete required regression coverage before acceptance.
- Tests specify behavior. Change an expectation only when original evidence proves it wrong.
  Missing output, unequal lengths, incomplete runs and missing required references must fail.
- Commit and push each accepted bounded change with its verification evidence. Do not mix unfinished
  experiments into it. Repository text is English; no generated-by or co-author attribution.
- Board IDs remain stable; path is state (`open/active/closed`). Preserve requirements, evidence and
  a concrete next action. Recommendations are hypotheses until measured. Git retains old detail.

Done: every menu, mission/map, setting/device, input/link and editor create→save→reload→simulate flow
is covered against the original. Require full-run frame/audio equality, visual spot-checks and ten
consecutive clean full WASM matrix runs independently confirmed by a subagent; failure resets the
streak. A passing subset or historical gate does not establish completion. See board:0029.
