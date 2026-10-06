Type: feature
Title: Original game assets are ignored and provisioned from a pinned archive

## Contract

Keep original game files outside Git tracking. Provide one reproducible installation path from the
user-supplied Armored-Fist_DOS_EN.zip URL, preserving existing installations and local save state.

## Evidence

- Base revision `a469760`; `.gitignore` now ignores `/armoredfist/`. All 419 previously tracked
  files were removed from the index with `git rm -r --cached`; existing files remain on disk.
- `make provision` delegates to `tools/provision_game.py`. It downloads/cache-checks archive SHA-256
  `a8d8fcb64cc525ddb1562cccfef4cbe94cc3c8d3fcb380d638ce59c28527ea8c`, extracts the entire game into
  a temporary sibling directory, and installs only after verification. Existing destinations are
  preserved. `--archive` supports offline installation; `--url` supports refreshed links to the same
  pinned archive; `--destination` supports isolated installations.
- A live download using the script's urllib path and an offline installation both succeeded.
  All 418 archive files match the preserved local originals and both provisioned copies bytewise.
  The additional local `FISTDATA/.FPL` remains intact. Evidence:
  `scratch/provision-game/{original-files.json,installed/,network/,proof.json}`.
- Four provisioning regressions cover complete extraction, existing save preservation, checksum
  rejection without installation/cache publication, incomplete archives without partial installs,
  and members outside the game directory. `make provision` preserves the existing game directory;
  `git ls-files armoredfist` is empty and `git check-ignore armoredfist/FIST.DAT` succeeds.
- `bash tools/check_flow.sh '^(intro|mainmenu)$'`: 51 tests pass, exact patches pass, native/WASM
  builds succeed, 2 selected flows pass / 0 fail, exit 0. Evidence: `scratch/verify/run.Q986NB/`.
  These checks validate this bounded provisioning change, not complete sequence parity.

## Next

Use `make provision` in fresh checkouts; maintain local originals read-only and run isolated copies.
Continue complete frame/PCM parity in 0034, startup attribution in 0036 and shared timing in 0026.

## Accept

Original assets are ignored and untracked without deleting local files; a verified live or offline
archive supplies all original game files. Existing installations remain unchanged and scoped
native/WASM verification passes.
