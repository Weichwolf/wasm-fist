# Rewrite boundaries

The annotated tag `reference/reconstruction-v1` records the reconstruction at `349ad31`.
Development proceeds on `master`; `ghidra` retains that decompiled and patched reconstruction.
The former `rewrite/softgl` branch remains available as history. Historical source, patches and
verification tools remain available without participating in the rewrite build graph.

| Location | Owner and boundary |
| --- | --- |
| `src/sim/` | Runtime object allocation, typed saved-mission payloads/physical roster/retained initialization RNG, vehicle/short-actor/tree state, deterministic random/motion/contact, weapon control, untargeted M1 launch, ordered unit collision, shell/effect lifecycle, all collision-reachable M1 target damage, ground retirement, world tick prefix/current-registry traversal and manual driving stages. |
| `src/app/` | Owned scenario/player session, integer input/time controller and common scene drawing. |
| `src/assets/` | Validated original-format decoders producing typed host data. |
| `src/render/` | softgl context, camera, terrain/models, HUD and framebuffer ownership. |
| `src/audio/` | Shared sound events, sample decoding and final PCM mixing. |
| `src/platform/native/` | Native window/input/audio device/storage and presentation. |
| `src/platform/wasm/` | Browser canvas/input/audio/storage and presentation. |
| `tools/` | Build, strict style, serving and original-file provisioning. |
| `tests/` | Rewrite behavior tests, C probes, fixtures, browser gates and instruction oracles. |
| `deps/softgl/` | Pinned renderer dependency, with no game state. |
| `board/reference/` | Historical work-item evidence. |
| `ghidra` branch | Frozen generated engine, patches, shims and reconstruction tools. |
| `/tmp/wasm-fist-reference-images/` | Verified instruction images provisioned from the immutable reference commit/originals. |

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
requires COOP/COEP headers from `tools/serve.py`. Main remains the render-context owner.

Renderer pin: `f93dbe9e744b48fa01d7f8eb8a44d2ff510b2d18`, delivered and verified in
[WI 0116](../board/closed/0116_production-renderer-policy.md). CMake includes `libsoftgl`
directly. Both target build graphs are pure C and exclude offline tools, including their
meshoptimizer sources. Initial integration evidence remains in
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
installation/scheduling, later target class updates, fire command/eligibility, effect drawing
and audible PCM remain subsequent simulation work. [M1 ground damage](vehicle-damage.md) consumes
pending primary hits against all four ground classes, preserving source/aspect scaling, ordered
random reactions, selected-player feedback, immediate destruction/effects/wreck/roster/census and
four-update type-19 retirement. Selected-player loss/UI/takeover, later wreck updates and audible
producer consumers remain explicit required boundaries; no complete playable battle is claimed.
[Remaining M1 targets](other-damage.md) share source validation and word-width arithmetic with
ground damage. They own typed 5/6/26/27 snapshot fields, preserve the real wreck no-op and expose
exact reactions, distinct census, authored effects and retained/released identities. Snapshot
restoration does not replace class initialization/AI. Death/debris/smoke lifetimes and the common
world tick/current-entry traversal are delivered in [world stepping](world-step.md).
[Saved-mission installation](mission-world.md) now owns typed payloads, the physical roster and
retained file-order initialization RNG, including all TRAIN1 objects and overwritten orphans.
It reuses the existing allocation and typed state owners; unsupported classes reject the whole
installation. [Primary fire dispatch](primary-fire.md) consumes the untargeted M1 station-zero
branch and publishes complete shell/muzzle payloads into canonical physical world slots. It
shares failure history and retains the actual panel/component/reload ordering; voice/sound
candidates still require their audio consumers. The app still uses its player-only baseline.
Terrain/contact/take-control of the installed world, complete living methods/command eligibility,
battle drawing, audible PCM and mission outcomes remain required before a playable battle.
[Canonical combat visits](mission-combat.md) consume the delivered flight/damage/lifetime owners,
publish every returned payload before another current entry and share one physical roster.
Selected fatal damage suspends before impact continuation for actual player-loss UI/takeover.
Unknown living methods reject explicitly; the continuous scene still uses its player-only
baseline, so these class-boundary probes do not establish a playable battle.
