# Native platform ownership

Native presentation, input, audio-device delivery and storage belong here. The current probe is
headless under `tools/rewrite/`. Interactive SDL2 integration is part of WI 0041; no window/device
implementation exists yet. Gameplay and mixing stay in shared C11 modules.
