Type: feature
Title: Original guest RAM can be captured and compared with port state

## Contract

Capture original mission-time guest RAM, VRAM, palette and camera state for diagnosis. Rebuild the
instrumented DOSBox from pinned original source and repository patches; reject unverified source.

## Evidence

`third_party/dosbox-fist` and `tools/oracle/dosbox_vga_terrain_trace.patch` provide instrumented
captures. `tools/oracle/capture_tcb_camera.sh` drives mission entry with FIST_R9200CAP and emits
camera metadata, full guest RAM, VRAM, palette and terrain inputs. The first AZER1 capture resolved
the previously unknown spawn camera.

- Restoration at base `0e211fa` plus the saved worktree patches: `bash tools/oracle/build_oracle.sh`
  creates `third_party/dosbox-build/dosbox-0.74-3/` and `third_party/dosbox-fist` from upstream archive
  SHA-256 `c0d13dd7ed2ed363b68de615475781e891cd582e8162b5c3669137502222260a` (Debian upstream mirror).
  The sequence/timing/KDV builder entry points delegate to this single recipe. All 16 instrumented
  source files must match `tools/oracle/oracle_sources.sha256` before compiling or installing.
  `DOSBOX_TREE` and `DOSBOX_OUTPUT` permit isolated rebuilds without replacing the default oracle.
- The former counter/block/exception/timestamp patches contained prose hunk headers; the KDV-stage
  hunk failed with zero fuzz and timing's context-free VGA insertions landed in unrelated code.
  They now use actual source context. Diagnostic bodies are retained. The reconstructed CPU/VGA
  hashes describe this reproducible recipe, not the unavailable historical working tree.
- `DOSBOX_TREE=$PWD/scratch/dosbox-restore/clean-build/dosbox-0.74-3
  DOSBOX_OUTPUT=$PWD/scratch/dosbox-restore/clean-build/dosbox-fist bash tools/oracle/build_oracle.sh`
  rebuilds from an empty directory with zero fuzz and passes every source hash. An intentionally
  changed CPU source is rejected before compilation; the installed binary remains unchanged.
- `FIST_CPU_TRACE=$PWD/scratch/dosbox-restore/cpu-88.txt FIST_CPU_TRACE_WINDOW=6354:6356
  FIST_KDV_STAGE=$PWD/scratch/dosbox-restore/stages.txt FIST_KDV_STAGE_N=90
  FIST_KDV_PROFILE=$PWD/scratch/dosbox-restore/profile.txt bash tools/oracle/capture_sequence.sh 10
  scratch/dosbox-restore/original-10s` produces complete 698-frame/442058-sample output.
  `cpu_trace.py ... --start 2b:11dd --stop 2b:7135` reproduces 1316 instructions/35524 cycles;
  callback 88 copies at 6365.540100 ms, as recorded in 0026. This does not prove all historical bytes.
- Two complete 30000-ms runs, one with OPL/speaker/sequence diagnostics and one from the independent
  clean build without them, match every frame, palette, timestamp and PCM record: 2100 frames and
  1324058 stereo samples. `compare_sequences.py ... --end-ms 30000` exits 0. The final menu was
  visually checked. Evidence: `scratch/dosbox-restore/{menu-diagnostics,menu-clean-build}/`.
  First nonzero stereo sample 18887 and 27 speaker-counter/54 type commands reproduce 0003/0026's
  recorded diagnostics; see `restoration-proof.json`.
- `bash tools/oracle/capture_tcb_camera.sh 93 $PWD/scratch/dosbox-restore/camera` reaches AZER1.
  Independently checked all 14 complete `.cap`/VRAM/palette/source captures, camera timestamps,
  CR3/paging records and the 16-MiB pass-00 guest-RAM dump. Pass 13's chained VGA image was visually
  checked using `capture_mission_spawn.sh`'s address conversion. Original game hashes remain intact.
  Build/capture logs and `cpu-88-proof.json` are under `scratch/dosbox-restore/`.
- Required port runtime asset regenerated read-only from `armoredfist/FIST.RUN` with
  `make kernel-image`; original checksum `0x088c5c9d` matches. Native/WASM validation:
  `bash tools/check_flow.sh '^(intro|mainmenu)$'`, 47 tests, 586 exact patches, both builds,
  2 selected flows passed/0 failed, exit 0. Evidence: `scratch/verify/run.GItDVW/`.
  This verifies the restoration prerequisites and selected flows; full matrix/sequence parity
  remains with 0012/0034. The PATH compiler prerequisite and reaching failure are recorded in 0033.

## Preserve

Resolve engine-linear addresses through current segment bases and CR3; port DGROUP 0x1c000 is not
a fixed guest physical address. Use FIST_WATCHFLAT with FIST_MEMARM_BOOT/FISTLOG for writes.
For stage proofs, snapshot inputs and outputs at the same call boundary (board:0002).
This item was formerly numbered 0007; memory-manager work formerly numbered 0024 is now 0025.

## Next

Use the restored oracle for 0034/0036/0026's first-output-difference investigation. Complete temporal
parity and final mixed port PCM remain with their owners; a restored diagnostic oracle does not
close those contracts.

## Accept

The original can be investigated at mission-time boundaries with original-attributed guest memory
and complete diagnostic captures. The reproducible oracle restoration has bounded proof above.
