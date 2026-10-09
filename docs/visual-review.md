# Visual review

Graphics are a primary acceptance concern. Every change that can affect visible
output needs actual native and Chromium review alongside relevant behavior tests.
For simulation-only changes, smoke-check the real scene; for rendering, camera,
asset, effect or HUD changes, compare before/after images and moving scenes.
Different hashes prove different output, not improved graphics.

Review the player's view at its normal size, then inspect details. Check terrain
texture clarity, seams and horizon; recognizable vehicle silhouettes, hull/turret
assembly, ground contact and occlusion; camera framing, clipping and movement;
visible firing, impact, smoke and destruction feedback when implemented; and HUD
contrast, readability and correct state changes. Include turning, slopes,
independent turret movement, pause/resume and supported display sizes. Expand to
multiple environments and all vehicle classes as their real presentation becomes
available. Missing consumers or visual coverage remain explicit open work.

Keep images, motion captures and logs under a dedicated `/tmp` directory. Record
the source/dependency versions, original asset manifests, mission, camera/settings,
device actions, capture names/hashes and concrete findings. Sampled still frames
help reveal pose and framing issues; they cannot establish smooth motion. Review
continuous movement and frame pacing separately before accepting those qualities.
Do not accept a visually worse result merely because numerical checks pass.
Intentional visual changes may update image expectations with reviewed evidence.

## Current baseline

On 2026-10-09, actual TRAIN1 images from the frozen 0129 draft were inspected:
13 native SDL captures and six Chromium captures, plus a sampled driving sequence.
The source is `/tmp/wasm-fist-contact-integration-source-v3`, based on `d48c92e`;
softgl is pinned to `f93dbe9e744b48fa01d7f8eb8a44d2ff510b2d18`.
Captures and original asset manifest are under
`/tmp/wasm-fist-0129-review/v4/visual-priority`.

The terrain and assembled M1 are visible, independent turret rotation appears in
the driven view, and weapon/ammunition/pause/reload labels are readable. The
terrain is very soft, and the small vehicle limits visible detail. Camera framing
and terrain clarity are the first quality follow-ups in 0046. These observations
do not accept continuous-motion quality, other environments/classes, battlefield
effects or the complete game. Those reviews remain open.

Reproduce the ordinary original-scene captures with the existing gates:

```sh
python3 tests/prepare_driving_preview.py --mission \
  --scenario armoredfist/FISTDATA/TRAIN1.FSG \
  --build-root /tmp/wasm-fist-contact-production-build-v3 \
  --output-dir /tmp/wasm-fist-visual-review/assets
python3 tests/verify_driving_native.py --mission \
  --scenario /tmp/wasm-fist-visual-review/assets/TRAIN1.FSG \
  --assets /tmp/wasm-fist-visual-review/assets \
  --native-preview /tmp/wasm-fist-contact-production-build-v3/native/fist_driving_preview \
  --output-dir /tmp/wasm-fist-visual-review/native
python3 tests/verify_browser.py --driving \
  --build-dir /tmp/wasm-fist-contact-production-build-v3/wasm \
  --output-dir /tmp/wasm-fist-visual-review/browser --port 8147
```

The browser gate requires matching assets and their manifest in the build's
`assets/` directory. Use a separate preview directory when another job owns it.
The captured baseline used a separate directory containing the unchanged WASM
preview and copied verified TRAIN1 inputs, avoiding writes to the running gate.
