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

The proposed isolated library modification was unbuilt, unpublished and discarded
when the user prohibited softgl changes. Neither checkout has tracked library
changes. The needed functionality already exists.

The shared owned adapter now configures and observes the profile through
`fist_renderer_create_multisample` and `fist_renderer_get_profile`. Its single
private-header boundary checks actual sample buffers, sample count and all three
started flags. It preserves the previously current context while querying GL.
Partial/all worker-start failures destroy the idle incomplete context and fail.
Invalid dimensions are checked before a width/height product can wrap on wasm32.

Targeted native and WASM production checks currently pass seven required groups
without skips: complete analytic single-sample/four-sample frames; twelve
sequential context lifecycles at 640x360,17x11 and1x1, with two complete changing
color readbacks each; invalid constructors/atomic profile errors; actual total
and partial pthread-start failures plus recovery; bad commands; and output I/O
failure. Linker wrapping injects faults at the real pthread boundary without
dependency edits. Native failure runs observe 9 failed starts for total failure,
and 4 failures/5 successful starts for partial failure; WASM observes 6 and 3/3.
Those include the library's temporary initial pool, as documented above.
An additional actual native draw to `/dev/full` exits with status1 and empty
stdout: a failed framebuffer write/close cannot report rendering success.

Each 921600-byte frame matches independent oriented-triangle coverage, top-left
edge ownership and rounded sample averaging at every RGBA byte. Four samples
produce exactly 0, 64, 128, 191, 255, including two subpixel triangles absent from the
single-center-sample frame. Native and real Chromium frames agree completely:
single-sample SHA256 daf33c416da6e8ad69c268a63e48dd0e3ee5cdac23dffbc96db7faeb5ac7735e;
four-sample SHA256 140f79c35f51eecfba55422062a30f1bbfbd99bdabaddf41537b78da93696960.
Chromium additionally passes all six successful modes, actual 2 GiB shared-memory
isolation and every canvas RGBA byte against the completed bottom-first C frame.
Both actual native/browser images and an 8x diagnostic coverage comparison have
been viewed: the diagonal gains mixed-coverage edge pixels, and both tiny shapes
appear at quarter coverage without spurious background pixels. The profile query
also preserves a pending GL error. Targeted LLVM19 tidy passes, and the complete
107-unit strict format/tidy gate has now completed with exit 0 from the frozen
original-free source. The complete production gate exits 0 in 5585.37 seconds:
56/56 native CTest cases pass in 1541.63 seconds; all 54 WASM scripts complete,
including 52 unittest suites with 387 groups. The new seven-group renderer and
eleven-group owned terrain suites pass without skips on WASM. Optional original
reference cases remain explicitly skipped in this original-free snapshot; this
does not establish complete-game independence or the final WASM streak.

The fresh production Chromium gate passes all six modes in 17.07 seconds, with
complete analytic coverage and canvas checks across 1843200 RGBA bytes. Both
fresh full browser images have been viewed and match the retained complete native
frames byte-for-byte. The enlarged diagnostic comparison confirms mixed-coverage
diagonal edges and both quarter-covered subpixel shapes. These are integration
images, not owned terrain/game visuals or performance measurements.

All seven groups also pass under AddressSanitizer/UndefinedBehaviorSanitizer with
production fast math and under Memcheck, without skips. All nine actual Memcheck
process logs report zero errors and zero bytes/blocks in use at exit, including
both startup-failure modes, recovery, repeated contexts and I/O failures.
Evidence/reproduction artifacts
live under `/tmp/wasm-fist-render-profile-review`.

The sequential ten-stage gate runs as user service
`wasm-fist-render-profile-gates.service`, from
`/tmp/wasm-fist-render-profile-source` into
`/tmp/wasm-fist-render-profile-production`. It snapshots 696 files at base
05d6b02973677e687e8f31d1e5e72adf29fa1d33 with unchanged softgl
1d17a944d873eb3e9eee551a96b46776366d7c3d. Its runner verifies source hashes
before/after each command and records actual exit status, command, elapsed time
and log hash. All ten stages are terminal with exit 0; the service is inactive
with MainPID 0 and ExecMainStatus 0. `audit.py` validates all receipts/log hashes,
complete suite coverage, required compiler flags on all 107 owned units for both
targets, unchanged source/pin, browser frames and the nine Memcheck logs, and
reports `full_bounded_acceptance: true`. The accepted audit is
`/tmp/wasm-fist-render-profile-review/acceptance-audit.json`.

## Next

Consume this accepted profile in owned terrain scene 0135. Keep the private API
coupling in the existing adapter and repeat actual profile/failure checks when
updating the dependency. Reproduce the production and integration checks with:

```
FIST_REWRITE_BUILD_ROOT=/tmp/wasm-fist-render-profile-production bash tools/build.sh all
python3 tools/check_style.py --build-dir /tmp/wasm-fist-render-profile-production/native
python3 tests/test_render_profile.py --target all --build-root /tmp/wasm-fist-render-profile-production
python3 tests/verify_browser.py --build-dir /tmp/wasm-fist-render-profile-production/wasm --render-profile --output-dir /tmp/wasm-fist-render-profile-review/reproduced-browser --port 8139
python3 tests/test_render_profile.py --target native --native-probe /tmp/wasm-fist-render-profile-sanitized/fist_render_profile_probe
python3 tests/test_render_profile.py --target native --native-probe /tmp/wasm-fist-render-profile-review/memcheck-profile.sh
```

## Accept

Accepted: unmodified pinned library sources; shared owned profile implementation; actual
four-sample framebuffer, three started helpers and caller on both targets;
successful lifecycle/failure and raster behavior checks; complete strict style
and production gates; and reviewed native/browser images with compact evidence.

Owned terrain runtime/native/browser visuals, representative geometry/material
costs, complete simulation/audio/presentation frame measurements and documented
60-FPS reserve remain required. This profile integration is not their acceptance.
