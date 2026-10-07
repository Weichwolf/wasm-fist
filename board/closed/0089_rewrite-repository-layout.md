Type: Work item
Title: Separate the rewrite checkout from the frozen reconstruction
Depends: 0085, 0088

## Contract

Keep active hand-written development on master and the exact decompiled/patched reference on
ghidra. README contains only a brief project description; remove GitHub workflows. Put rewrite
tests/probes/fixtures directly under tests and build/style/serving/provisioning under tools.
Only softgl belongs in deps. Remove re_out, patches and legacy reconstruction tools from master;
provision pinned original instruction images from the immutable reference into a dedicated /tmp
cache. Original game files stay ignored/read-only. Preserve every existing behavior expectation,
strict compiler/style gate, immutable reference commit and renderer pin.

## Evidence

Before removal, generated code, patches, legacy tests/tools and ref agree with ghidra commit
349ad31a9fd21b350d435651bb2e90afda40cf60. Its tag reference/reconstruction-v1 is unchanged.
The old rewrite test suite occupied tools/rewrite; root tests contained reconstruction tests.
The engine/video instruction images are tracked in frozen Git; the kernel image was ignored
and must be regenerated with that commit's exact extractor from provisioned FIST.RUN.

## Next

Build/test both targets sequentially through the moved commands; enforce strict LLVM 19.1.x
checks. Regenerate all three reference images into an empty /tmp cache and compare existing
pins. Exercise fail-closed corrupt/out-of-/tmp caches and required original regressions using
the new owner. Verify actual browser/native preview command paths. Check original files,
reference refs and softgl unchanged; retain compact receipts, clean obsolete logs, commit/push.

## Accept

Complete native/WASM build gates and strict format/tidy pass without weakening checks.
Required original oracles and actual presentation gates pass from the new paths. No reference
code/dependency or generated cache is restored to master. The frozen ghidra/tag and pinned
softgl remain unchanged; no game completion or final independent WASM streak is claimed.

## Verified checkpoint

On 2026-10-07 the native production build passes all 35 CTest gates in 324.18 seconds.
Strict LLVM 19.1.x clang-format and clang-tidy pass for all 79 owned translation units and
all owned headers. CMake changes are exclusively source/test path replacements; all compiler
flags, renderer options and test definitions remain unchanged. Of 131 moved files, 124 change
only repository paths. Seven additional integrations update script roots, style ownership,
reference-image provisioning and the browser tooling documentation pointer. No owned C behavior
or existing test expectation changes.

An empty dedicated /tmp cache regenerates all three instruction images byte-for-byte against
the existing original images and pins. Engine/video images come from frozen Git; the kernel
uses the exact frozen extractor and read-only FIST.RUN. Truncated cached data, invalid cache
locations, unknown image paths and missing FIST.RUN/reference data reject without publishing
a kernel image. The provider never requires generated engine code in the rewrite checkout.

Required original mission-ready evidence passes six groups, zero skips, in 32.777 seconds:
3072 ground returns, 98 complete registry passes, 8543 physical observations, 5185 actual height
transfers, 5868 tree RNG calls and 80 releases. Original order-boundary evidence passes three
groups in 1.068 seconds, 376 complete chunk returns and 109040 bytes. Required both-target
vehicle-start passes seven groups in 90.717 seconds; canonical driving passes seven groups in
22.576 seconds, 123 fixtures/924 complete boundaries/907 objects/60 rejections per target.
These required gates have no skips. Native required original vehicle/model selection passes
six groups in 16.251 seconds and terrain assets pass 17 groups in 2.326 seconds: 22 complete
planes/3146880 pixels/32 palettes and all 47 scenario bundles. Original-file provisioning
passes its four preservation/validation groups. Required original sprite composition passes
six groups, zero skips, in 42.429 seconds on both targets, covering all 34 families and
6432 complete bitmaps through the moved kernel-image owner.

Actual Chromium renderer, terrain and canonical TRAIN1 driving gates pass through the moved
server/checker paths. Complete terrain presentation is 640x400, 25053 distinct colors and
RGBA FNV1a b85f2028. Driving checks complete opaque frames, actual movement, weapon selection,
reload, pause, focus loss, shutdown and startup failure. Actual SDL TRAIN1 input/presentation
passes against isolated /tmp assets. Reviewed native and browser paused captures show textured
terrain, the selected vehicle and the authored weapon/ammunition/pause HUD. No rendering changes
are introduced.

All 419 provisioned original files are unchanged. ghidra and the peeled reference tag retain
349ad31a9fd21b350d435651bb2e90afda40cf60; annotated tag object 91278feaf9240b6035e9fed0c75c28ff766d3912
and softgl pin 7963be1d5b5e1bebbe97ece2c655228c8bc0a838 are unchanged. The reference retains every
one of the 1289 removed legacy files. Master has only softgl in deps, no third_party/re_out/patches,
and no workflows. External DOSBox binary/source and Ghidra/JDK references remain under /tmp;
duplicate frozen copies and obsolete captures/build output have been removed. Historical board
evidence remains unchanged. The full regular WASM gate also completes successfully: all 33
Python suites (252 cases) and both Node pixel probes pass. The regular constructed
contracts retain their 41 expected optional-original skips; the explicitly required
original gates above complete without skips. These gates verify their recorded bounded scope,
not full game acceptance.

Receipts and latest reviewed visual evidence are under /tmp/wasm-fist-0089-review. This layout
change does not deliver C mission readiness, living battle, audio or final game acceptance.
The final complete independently confirmed WASM streak remains zero.
