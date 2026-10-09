Type: Work item
Title: Continuous visual improvement
Depends: 0041

## Contract

Improve terrain, models, lighting, camera/HUD and presentation continuously while preserving tested gameplay behavior and useful performance. Visible quality is an acceptance priority: inspect actual native and browser output and movement, rather than inferring quality from numerical results or image hashes. Follow `docs/visual-review.md`.

## Evidence

The controlled TRAIN1 scene now presents original terrain and an assembled player vehicle in native SDL and Chromium, including steering, independent turret rotation and weapon/pause/reload HUD states. Actual images from the frozen 0129 draft were reviewed on 2026-10-09: 13 native captures and six browser captures under `/tmp/wasm-fist-0129-review/v4/visual-priority`. Terrain and HUD render completely; the weapon and status text is readable. Terrain detail is very soft and the player occupies little screen space, limiting vehicle detail and readability. The current view demonstrates a controlled preview, not complete battlefield presentation or final graphics quality.

## Next

Use the real controlled scene as the baseline. Review camera composition and terrain clarity first, with actual before/after native/browser images and movement. Record content/settings and timings; make one intentional visual improvement per bounded review and retain behavior regressions. Expand review to other terrain, all four ground classes, combat effects and full battle content as their consuming presentation becomes available; missing content remains open.

## Accept

Each visual change has actual native/browser before/after review, moving-scene review and recorded performance/quality tradeoffs. Review terrain seams/detail, vehicle recognition/contact/occlusion, camera clipping and stability, combat feedback and HUD legibility at supported sizes. Required terrain/model/HUD content remains visible and functional. A changed frame hash or a passing simulation test cannot replace this review. Record concrete defects and follow-up work; final quality review feeds 0047.
