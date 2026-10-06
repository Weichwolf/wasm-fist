# Rewrite boundaries

The annotated tag `reference/reconstruction-v1` records the reconstruction at `349ad31`.
Development proceeds on `rewrite/softgl`; master remains unchanged. Historical source, patches
and verification tools remain available without participating in the rewrite build graph.

| Location | Owner and boundary |
| --- | --- |
| `src/sim/` | Runtime object allocation, typed vehicle state, deterministic random/motion/contact, weapon control, untargeted M1 launch, ordered unit collision, shell/effect lifecycle, M1 ground damage/destruction/retirement and manual driving stages. |
| `src/app/` | Owned scenario/player session, integer input/time controller and common scene drawing. |
| `src/assets/` | Validated original-format decoders producing typed host data. |
| `src/render/` | softgl context, camera, terrain/models, HUD and framebuffer ownership. |
| `src/audio/` | Shared sound events, sample decoding and final PCM mixing. |
| `src/platform/native/` | Native window/input/audio device/storage and presentation. |
| `src/platform/wasm/` | Browser canvas/input/audio/storage and presentation. |
| `tools/rewrite/` | Build/style/probe tools for owned rewrite code. |
| `deps/softgl/` | Pinned renderer dependency, with no game state. |
| `board/reference/`, `re_out/`, `patches/`, legacy shims/tools | Historical evidence only. |

The renderer, diagnostic browser presentation, shared FSG envelope/metadata decoder and
KLC/resource/palette readers, owned unit definitions/registry/roster and directional sprite models,
complete terrain bundle loader and terrain inspection scene exist so far. See
`docs/scenario-format.md`, `docs/terrain-format.md`, `docs/terrain-loading.md`,
`docs/terrain-scene.md`, `docs/unit-definitions.md` and `docs/model-format.md`.
Unit definitions own immutable complete
snapshots; typed identity/pose fields do not initialize or simulate vehicles. Other chunk views
retain undecoded gameplay records honestly.
Models own palettes, complete raw streams, validated sprite/part views and all 32 directional
record mappings. The model loader copies data before storage sources reuse their buffers;
catalog selection and vehicle composition/rendering are delivered; original mission model
palette mapping remains open.
Other directories document ownership, not implemented subsystems. No placeholder game APIs promise behavior they lack.
The diagnostic triangle uses fixture geometry and colors, not reconstructed game assets.

The runtime and renderer are C11 throughout. meshoptimizer belongs to softgl's offline tools
and is neither compiled nor linked by the rewrite.
RGBA8 pixels are tightly packed, bottom row first; browser presentation flips them for ImageData.
The render thread owns the context and borrowed framebuffer view. softgl_read_rgba8 flushes worker
output before exposing the view. WASM uses SIMD128 and pthread tile workers, with a prestarted three-worker pool
matching the renderer's automatic WASM helper-worker limit; native softgl also uses tile workers. Context access remains single-owner. The controlled preview has SDL2 native and browser
input/time/presentation loops; shared `app/driving` owns all simulation and held-key state.

The terrain scene owns its temporary mesh/texture until framebuffer readback completes workers.
The inspection camera uses recovered periodic coordinates and an explicit preview pose; it does
not supply missing vehicle/cockpit behavior. Browser preview inputs use MEMFS on both Node and
Chromium; platform JavaScript fetches/hash-checks files and presents completed C output only.

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

The continuous driving preview is documented in [driving scene](driving-scene.md). Its native
device loop currently lives in the preview tool; production platform packaging follows the
complete mission interface. It supplies no alternate simulation. This manual stage displays
only the player, with weapon/ammunition/reload feedback. Targeting, other-unit AI, combat,
objectives and audio remain open. The [runtime object pool](object-pool.md) now owns arena
occupancy and registry binding. [Untargeted M1 launch](projectile-launch.md) now returns typed
initialized shell and muzzle payloads using that owner. [Ordered unit collision](unit-collision.md)
borrows live typed poses through the common `sim/world.h` pose and preserves type/registry/random
contracts. [Untargeted shell flight](projectile-flight.md) now consumes launch payloads through
current-position ground/unit queries and expiry, keeps impacts pending until actual damage, and
owns post-damage explosion allocation and complete explosion/muzzle retirement. Live world
installation/scheduling, remaining target damage rules, fire command/eligibility, effect drawing
and audible PCM remain subsequent simulation work. [M1 ground damage](vehicle-damage.md) consumes
pending primary hits against all four ground classes, preserving source/aspect scaling, ordered
random reactions, selected-player feedback, immediate destruction/effects/wreck/roster/census and
four-update type-19 retirement. Selected-player loss/UI/takeover, later wreck updates and audible
producer consumers remain explicit required boundaries; no complete playable battle is claimed.
