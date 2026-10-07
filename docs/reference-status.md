# Frozen reconstruction status

Tag `reference/reconstruction-v1` points to `349ad31a9fd21b350d435651bb2e90afda40cf60`:
"Execute reached VGA panning through shared original drawing and calendar owners".
This is a reference snapshot, not a declaration of full original compatibility.

Branch `ghidra` retains exactly this decompiled and patched revision. Active rewrite development
uses `master`; the annotated reference tag and historical code remain unchanged.

The publication receipt recorded 260 tests, 611 exact patch checks, six resource-start cases and
178 flows without failures. Native/WASM builds ran sequentially and all 419 original files were
unchanged. The receipt explicitly leaves whole runtime startup/frame/audio acceptance open.
This historical bounded evidence has not been rerun for the rewrite.

The former queue, including synchronized frame/audio, missions, input/devices/link, editor and
full acceptance requirements, is preserved under `board/reference/`. Agreement in bounded port
checks does not establish original bit identity or complete functionality. Original isolated
DOSBox/QEMU runs and recovered code remain evidence for formats and gameplay; no generated engine
code is linked into new CMake targets.

At handoff, running old verification/publication owners were terminated before branch preparation.
Their outputs and handoff receipts remain under `/tmp`; unfinished private experiments are excluded.
The local detached reference worktree is `/tmp/wasm-fist-reference`. Historical build instructions
remain in `docs/reconstruction-README.md` and in the tag.
