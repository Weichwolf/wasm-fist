Type: Work item
Title: Update pinned softgl with correct native/WASM integration semantics
Depends: 0040

## Contract

Update only the pinned renderer dependency and required compiler integration,
separately from owned-content development. Preserve shared C11, only softgl in
deps/, no runtime meshoptimizer and all owned warning/fast-math checks. Verify
strict style, complete production native/WASM regressions and actual presentation.
The 60-FPS/640x360/4x-MSAA/four-total-thread profile remains a performance target,
not acceptance inferred from a dependency update.

## Evidence

The first current upstream pin a95534e builds natively, passes103-unit style and
all54 native CTests, then fails WASM compilation: MSAA/fragment paths use SSE
intrinsics, while this integration enabled only msimd128. Upstream's own
wasm/CMakeLists.txt explicitly enables the SSE compatibility path. Adding
msse4.1 to softgl's WASM compile options resolves the actual compile failure;
no MSAA or checks are disabled.

The pilot then exposes infinity/fast-math warnings in hierarchical depth and
cluster bounds. Preserve required IEEE infinities, signed zero and combiner
association through a library-only interface with fno-finite-math-only,
fsigned-zeros and fno-associative-math, mirroring upstream's documented semantic
policy. All requested flags and production fast math remain on owned simulation
and audio. No warning is suppressed. Actual WASM compile command order confirms
these semantic options follow ffast-math.

The next fetched upstream pin is17cd36e7f67dbfd479a38e1c5a01279ac4259654.
Only experimental evidence changed sincea95534e; library sources are unchanged.
The isolated source is /tmp/wasm-fist-softgl-update-source at base2df1dec plus
the new pin and required CMake integration. Rejected first-run receipts and
compact failure evidence remain under /tmp/wasm-fist-softgl-update-review.
Final corrected frozen gates run under its v3 directory. No full dependency
acceptance or measured60-FPS result is claimed yet.

## Complete verification

The corrected v3 native configure/build and LLVM19.1.7 style gate pass
(103 owned translation units). All54 native production CTests pass in1290.35s.
The complete `bash tools/build.sh all` exits0 in4185.13s, including52 WASM
Python scripts:50 unittest suites/369 tests, two plain gates and two Node
integration checks. No actual compiler warning/error or failed suite is present.

All seven sequential stages complete with actual exit0 receipts, exact commands,
log hashes and623 frozen source-file hashes. Production compile databases prove
C11, requested warnings/alias/fast-math flags and Werror on owned code; only the
library receives the three IEEE/association options after ffast-math. All actual
library WASM commands include msimd128/msse4.1/pthread. Neither build graph links
meshoptimizer or upstream offline tools.

Native SDL and real Chromium scenes pass input, pause, focus, weapon state and
shutdown checks. All13 canonical native captures and six browser captures were
actually visually inspected. Terrain, transparency/depth and readable HUD remain
coherent. Blurred terrain and the small original sprite remain quality debt;
these optional original-based compatibility scenes do not establish owned-game
independence, full gameplay or60-FPS/4x-MSAA acceptance.

Tests ran at17cd36e7f67dbfd479a38e1c5a01279ac4259654. During that run upstream
advanced to1d17a944d873eb3e9eee551a96b46776366d7c3d. The published pin uses that
newest observed revision. Both have the exact same `libsoftgl` Git tree,
6abb3115138dd11d71ecf5c032d3f2e219b9d72f; all259 changed files are experiments
outside the compiled library. The library CMake has no Git-derived version or
metadata generation. Every frozen production source/configuration hash matches
the current checkout. Content-equivalence therefore covers the published pin;
no new production source or compiler option is substituted after the gates.
The recorded tested/published pins remain explicit rather than claiming another
test execution. Root source equivalence, tree identity and actual capture hashes
are verified in /tmp/wasm-fist-softgl-update-review/v3/acceptance-audit.json.
Original tested-pin receipts/source remain preserved there.

## Next

Continue0131's owned terrain/material quality, complete original-free runtime
loading and representative native/WASM renderer profiling. The current preview
uses a single-sample context; genuine4x MSAA, four total render threads and60-FPS
complete frame timing still require explicit runtime integration and measurement.
The complete game and independent ten-run WASM streak remain open at0.

## Accept

The exact pin builds and passes full required gates on both targets with
correct SIMD128 and IEEE semantics and no weakened checks. Native/browser actual
presentation passes and is visually reviewed. Source/pin/flags and actual
complete results are reproducible. Performance, new asset quality and complete
game acceptance remain separate open requirements.
