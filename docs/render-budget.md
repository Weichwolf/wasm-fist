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

The public softgl context API supports `softgl_create_multisample(..., 4)`;
the current game renderer still calls the single-sample creation function.
The verified1d17a94 library and newest observed549ae30 have identical library
content. Their automatic worker initialization reserves the caller and caps
helpers at three only under `__EMSCRIPTEN__`. Native still chooses the logical
CPU count as its helper count. This native default does not implement the
required shared four-thread target and must be corrected in a separate verified
dependency change. Explicit internal hints count helpers, and
`sg_thread_count` returns the configured helper count, excluding the caller.
Check successfully started workers as well as configured counts when validating
the actual native/browser profile. No60-FPS claim follows from API availability.
