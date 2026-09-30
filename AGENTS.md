# Armored Fist — working rules

Target: original DOSBox/QEMU, native and WASM emit identical complete frames (indices, palette,
time) and mixed PCM from matched start state, timed input and devices. Memory traces diagnose defects.

- Follow `board/README.md`. Work every WI yourself, one bounded step at a time; verify old claims.
- Find the first output difference. Recover its original register/segment/width/flag contract.
  No guessed constants, missing-work stubs, masks or guards that hide defects.
- KISS/DRY: one owner per state/output/verification contract. Reuse across targets.
- Generated engine C in `re_out/` stays pristine. Use asm-backed `patches/NNN-*.diff` with rationale.
  Edit hand-written shims/tools directly. `build/` is disposable and has one writer; build sequentially.
- Use local originals first: `third_party/dosbox-fist`, source `third_party/dosbox-build/`.
  `FIST_WATCHFLAT` follows guest paging; port DGROUP `0x1c000` is not guest physical `0x1c000`.
- Keep `armoredfist/` read-only; run isolated copies. Match capture boundaries before comparing.
- Prove a reaching failure, fix its cause, regress both targets. Filters give partial evidence.
  Complete required coverage before acceptance; commands and WI format live in the board.
- Tests specify behavior; change expectations only when original evidence proves them wrong.
  Missing output/references, unequal lengths and incomplete runs fail.
- Keep WIs current; preserve IDs and requirements. Git retains history.
- Commit/push each verified bounded change; exclude unfinished experiments.
  Repository text is English. No generated-by/co-author attribution.

Done: every menu, mission/map, setting/device, input/link and editor create→save→reload→simulate
flow matches complete original frame/audio sequences. Require visual checks and ten consecutive
clean full WASM runs independently confirmed by a subagent; failure resets the streak. See 0029.
