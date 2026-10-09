Type: Work item
Title: Compact retained terrain blocks and verify renderer cost
Depends: 0135

## Contract

Reduce avoidable vertex work in the owned terrain scene using public GL APIs and
retained, tightly indexed block geometry. Keep softgl completely unchanged.
Preserve the canonical surface, all height bits, sample-aligned colors, periodic
seams, conservative visibility, source independence and atomic scene ownership.
This step does not introduce LOD, change materials or reduce image quality.

Actual frame cost must be measured on both targets with 640x360, genuine 4x MSAA
and three helpers plus the caller. The complete 60-FPS game target remains open;
do not infer it from vertex counts or a single baked terrain material.

## Evidence

During active 0135, each 32-cell block references 33x33 vertices in the global
1025-wide grid. Its inclusive index range is 32x1025+33 = 32833, although only
1089 vertices occur in its indices. The unchanged upstream indexed draw path
transforms that inclusive range for this client-array geometry. Public GL buffer
storage is also required for the existing upstream buffer revision caches;
client arrays do not activate those caches.

A complete eight-frame native Callgrind run exits zero at the requested profile,
drawing 65 blocks and 133120 triangles per frame. It records exactly 17073160
calls each to `sg_process_vertex_prepared` and `sg_process_vertex`, matching
32833x65x8. The two functions account for 53.77% of recorded instruction events.
These are instruction and call counts, not wall-clock FPS measurements. Callgrind
instrumentation changes thread scheduling and cannot prove production capacity.
Kernel perf sampling is unavailable at the current perf_event_paranoid setting;
the diagnosis required no kernel configuration change.

Temporary reproduction command, complete frame/profile, annotation, call counts
and the side-16-through-4096 layout derivation are retained under
`/tmp/wasm-fist-owned-scene-review/perf-terrain`. Compact 32-cell block storage
would retain 1115136 vertices for a 1024-square map, including identical boundary
duplicates: 6.14% above the canonical unique grid, instead of transforming an
index span 30.15 times the referenced vertex count on each block submission.

## Next

Finish and publish 0135 before changing its frozen implementation. Store each
block's exact canonical positions/UVs contiguously and use public retained vertex
and index buffers. Bound memory explicitly, preserve every boundary sample and
triangle, and report actual retained storage rather than hiding duplicates in
statistics. Release every GL resource in its owning context; failed installation
must consume errors, preserve the installed scene and recover at every measured
allocation boundary. No buffer allocation or upload belongs in a normal draw.

Compare complete before/after frames on each target at all map families and
boundary/motion poses. Measure warmed motion separately on Native and WASM after
the complete regressions finish, recording submitted geometry, actual profile,
timing scope and system load. Assess public-buffer caching with both changing
and repeated cameras; repeated-camera hits alone do not prove playable motion.

## Accept

Every existing owned scene/asset contract passes without skips on both production
targets. Complete before/after frame comparisons and actual native/browser motion
show no cracks or visual regression. Actual compact ranges and retained memory
match their independently derived sizes. Source release, context switches,
replacement, unload and every allocation failure recover correctly. Required
strict style, full builds and sanitizer/verbose Memcheck gates pass. Production
motion measurements establish the actual cost change; unchanged dependency status
and pin are verified. Commit/push this bounded success and retain the separate
material, LOD, representative-content and complete-game performance work.
