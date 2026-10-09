Type: Work item
Title: Owned renderer MSAA and three-helper profile
Depends: 0133

## Contract

Use unmodified upstream softgl; every adaptation belongs in wasm-fist. Implement
genuine 4x MSAA and three successfully started helpers plus the caller on native
and WASM. The target profile is 640x360. Keep the existing single-sample diagnostic
contract available, with the same helper limit. Expose actual profile observations
through owned renderer types; callers must not depend on private softgl structures.

## Evidence

The public context API supports softgl_create_multisample(width,height,4).
The existing internal worker API supports sg_workers_shutdown(context),
sg_workers_init(context,3) and sg_thread_count(context). Its explicit hint and
count refer to helpers, excluding the calling render thread. Dependency tests
already reconfigure created contexts this way. The native automatic default
uses CPU-count helpers; the WASM default reserves the caller and caps helpers
at three. Use an explicit application profile instead of changing either default.

At pin 1d17a944d873eb3e9eee551a96b46776366d7c3d, state.c creates the framebuffer
and initializes the automatic pool before returning a context. Configure the
owned profile before submitting any draw: shut down that idle initial pool,
initialize with hint 3, and inspect the result. This does not prevent upstream's
temporary automatic worker creation during context construction; the four-thread
profile governs rendering after configuration. Do not imply a different library
initialization behavior.

workers.c sets pool nworkers before pthread_create and records each successful
start separately in workers[].started. sg_thread_count returns nworkers, even
when a start failed. A configured count alone cannot prove the required profile.
Check both the count and all three started flags in the sole owned adapter.
state.c exposes the actual framebuffer through GL_SAMPLE_BUFFERS and GL_SAMPLES;
query those rather than reporting the requested constructor arguments.

These are read-only source findings, not tested owned profile integration. The
proposed isolated library modification was unbuilt, unpublished and discarded
when the user prohibited softgl changes. Neither checkout has tracked library
changes. The needed functionality already exists.

## Next

Keep any pinned internal API coupling in one owned renderer adapter. Configure
and verify a context before exposing it to rendering. On absent/partial worker
startup or a mismatched sample count, destroy the still-idle context and return
failure; silently using fewer helpers or fewer samples is not acceptance.
Dependency headers remain system headers, without relaxing checks on owned C.

Add meaningful native/WASM profile, lifecycle and complete-image tests. Check
opaque interiors, mixed-coverage diagonal edges and genuinely subpixel geometry
against the single-sample diagnostic to establish raster behavior, not just a
configuration number or changed hash. Repeated creation/destruction and two
contexts must not leak helpers or retain another context's framebuffer. Exercise
worker-start failure through owned test tooling without editing the dependency.
Review actual native and browser captures. Run strict LLVM19 style and the full
sequential native/WASM game build gate. Do not add a build writer while the current
0133 gate is active.

## Accept

Unmodified pinned library sources; shared owned profile implementation; actual
four-sample framebuffer, three started helpers and caller on both targets;
successful lifecycle/failure and raster behavior checks; complete strict style
and production gates; and reviewed native/browser images with compact evidence.

Owned terrain runtime/native/browser visuals, representative geometry/material
costs, complete simulation/audio/presentation frame measurements and documented
60-FPS reserve remain required. This profile integration is not their acceptance.
