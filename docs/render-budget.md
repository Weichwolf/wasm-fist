# Rendering target and measurement

Target **60 FPS at 640x360, genuine 4x MSAA and four total render threads** on
this machine: three helper workers plus the calling render thread, identically
for native and WASM. The
frame interval is 16.67 ms. This is a target, not an accepted measurement of the
current 640x400 single-vehicle preview or completed game.

The user's initial estimate of 100000-200000 triangles and 20-30 materials per
frame was a rough 30-FPS starting point. Use that range to construct representative
scenes and measure this faster machine; do not assume it proves 60 FPS or blindly
halve the geometry count. Record actual limits for the intended materials/content.

Measure native and WASM separately after warmup, with fixed owned scene/content,
camera path, resolution, exact MSAA setting, total thread count and softgl pin.
Record rendered geometry/materials, visible coverage/overdraw, alpha-tested
content, shading cost, complete frame times and median/p95/p99 behavior. Include
simulation, audio and actual device/browser presentation when assessing game FPS.
Maintain explicit reserve for effects, overdraw and demanding materials; tune
content/culling/batching/LOD against measured cost and reviewed visual quality.

Verify the requested sample count and thread count in the actual renderer.
Disabling MSAA, rendering a smaller image, counting only raster work or dropping
required content cannot establish the target. Compare actual native/browser images
and movement alongside timings. Performance acceptance remains open until complete
representative owned scenes meet the target with documented reserve.

## Current integration evidence

The owned renderer now provides `fist_renderer_create_multisample(..., 4)` and
`fist_renderer_get_profile`. The adapter requires an actual four-sample
framebuffer and three successfully started helpers before exposing the context.
The existing single-sample preview remains available through the same adapter;
the owned terrain scene is the next consumer of the target profile under 0135.

softgl remains unchanged at 1d17a944d873eb3e9eee551a96b46776366d7c3d. The
newest observed 549ae30 has identical library content. Upstream initializes an
automatic pool before returning a context: native uses CPU-count helpers, while
WASM caps helpers at three. The owned adapter replaces that idle pool with three
helpers before submitting any draw. This limits rendering threads; it does not
prevent temporary automatic thread creation during native context construction.
Private API coupling stays in `src/render/softgl_profile.c`. Explicit internal
hints and `sg_thread_count` count configured helpers, excluding the caller;
the adapter separately counts successful starts and rejects partial startup.

Native and real Chromium diagnostic frames establish four-sample coverage,
including subpixel shapes, against independent geometry at every RGBA byte.
Context lifecycle, startup-failure recovery and strict style checks pass.
The complete production and memory gates pass under closed 0134: 56 native tests,
54 WASM scripts, six actual browser modes, and all seven renderer groups under
sanitizers and Memcheck. All nine Memcheck processes have zero errors and zero
unreleased bytes/blocks.
These integration diagnostics do not establish owned terrain quality or 60 FPS.
