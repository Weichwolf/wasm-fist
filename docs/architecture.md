# Rewrite boundaries

The annotated tag `reference/reconstruction-v1` records the reconstruction at `349ad31`.
Development proceeds on `rewrite/softgl`; master remains unchanged. Historical source, patches
and verification tools remain available without participating in the rewrite build graph.

| Location | Owner and boundary |
| --- | --- |
| `src/game/` | Typed world/vehicle/mission state, fixed-step simulation, AI, rules and outcomes. |
| `src/assets/` | Validated original-format decoders producing typed host data. |
| `src/render/` | softgl context, camera, terrain/models, HUD and framebuffer ownership. |
| `src/audio/` | Shared sound events, sample decoding and final PCM mixing. |
| `src/platform/native/` | Native window/input/audio device/storage and presentation. |
| `src/platform/wasm/` | Browser canvas/input/audio/storage and presentation. |
| `tools/rewrite/` | Build/style/probe tools for owned rewrite code. |
| `deps/softgl/` | Pinned renderer dependency, with no game state. |
| `board/reference/`, `re_out/`, `patches/`, legacy shims/tools | Historical evidence only. |

The renderer, diagnostic browser presentation, shared FSG envelope/metadata decoder and
KLC/resource/palette readers and complete terrain bundle loader exist so far. See
`docs/scenario-format.md`, `docs/terrain-format.md` and `docs/terrain-loading.md`; unit/chunk
views retain undecoded gameplay records honestly.
Other directories document ownership, not implemented subsystems. No placeholder game APIs promise behavior they lack.
The diagnostic triangle uses fixture geometry and colors, not reconstructed game assets.

The runtime and renderer are C11 throughout. meshoptimizer belongs to softgl's offline tools
and is neither compiled nor linked by the rewrite.
RGBA8 pixels are tightly packed, bottom row first; browser presentation flips them for ImageData.
The render thread owns the context and borrowed framebuffer view. softgl_read_rgba8 flushes worker
output before exposing the view. WASM uses SIMD128 and pthread tile workers, with a prestarted three-worker pool
matching the renderer's automatic WASM helper-worker limit; native softgl also uses tile workers. Context access remains single-owner. SDL2 interactive native integration is next.

Use explicit typed units and a fixed simulation step independent of display pacing as simulation
is reached. Both platforms consume one simulation/audio path; guest registers and memory layouts
do not belong in new game state. Validate original format adapters where compatibility is needed;
platform persistence owns storage only. Define behavioral replay assertions before broad coverage.
Visual evolution may deliberately change frames; original frame/PCM identity is not required.

The first playable milestone installs decoded terrain and a real vehicle, renders them through softgl,
then adds movement, controls, camera/HUD, weapons/objectives and audible events. Every step needs
original behavior/data evidence and bounded cross-platform verification. Missing asset/gameplay
knowledge must remain an explicit open task.

The pinned dependency requires pthreads for functional worker completion: its attempted no-pthread
configuration retains a nonzero expected worker count after failed thread creation and hangs at
flush. The rewrite selects the dependency's supported pthread build. Browser serving therefore
requires COOP/COEP headers from `tools/rewrite/serve.py`. Main remains the render-context owner.

Renderer pin: `7963be1d5b5e1bebbe97ece2c655228c8bc0a838` (latest published `origin/master`,
rechecked on 2026-10-06). CMake includes `libsoftgl` directly. Both target build graphs are pure C
and exclude offline tools, including their meshoptimizer sources. Verification is recorded in
[WI 0050](../board/closed/0050_softgl-dependency-verification.md).
