# Stage-2/3 reference material — the ORIGINAL under DOSBox

Captured from `armoredfist/FIST.RUN` running under DOSBox (headless xvfb; `tools/refcapture.sh`).
These are the visual/behavioural targets the port must match (Stage 2 = bit-identical; Stage 3 = every
surface reachable).

## Reference frames (in `ref/`)
- `ref/boot_frame.png` — early boot/loading frame (white + palette-load speckles).
- `ref/main_menu.png` / `ref/main_menu_full.png` — the **main menu** (~48 s after launch). Title art:
  Abrams tank + Apache/Hind helicopters + explosions over voxel terrain, version string **"AF-FD 1U1Q"**.
  Menu overlay (7 items, top→bottom):
    1. SELECT PLAYER
    2. CAMPAIGNS
    3. BATTLES
    4. REVIEW
    5. SETTINGS
    6. ABOUT FIST
    7. QUIT
  Mouse-driven (game REQUIRES a mouse). Each item + every sub-screen it opens is in the Stage-3
  exhaustive-coverage scope (see CLAUDE.md Goal). Maps/missions reached via CAMPAIGNS/BATTLES; the
  mission/level EDITOR is reached from within (locate exact path during Stage 3).

## Notes
- Native-resolution (exact 320×200×8) fb+palette dumps for bit-verify need either a DOSBox screenshot
  key (no xdotool here yet) or a memory-dump/instrumented DOSBox / QEMU +gdb — TODO for Stage 2.
- The frames above are scaled X-window grabs (good for qualitative targets, not pixel-exact compares).
