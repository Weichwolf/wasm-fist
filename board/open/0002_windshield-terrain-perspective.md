Type: bug
Title: The terrain raycaster and projection produce the original tile and framebuffer
Parent: 0001
Depends: 0024

## Contract

Prove the complete tile-build → projection → framebuffer chain from matched original inputs.
An isolated writer proof and matching colour statistics are partial evidence only.

## Evidence

- Original sequence: op 0x08 builds via 8df0 → 3931 → 85d0 / sky resample / 6980;
  op 0x24 presents via 8120 → 9200 → 82d0. Patch 580 and the gate rewrite restored it.
- Map load self-modifies the detail shifts and buffer immediates. Static `shld 10` does not prove
  a fixed map size; observed live shifts include 11. Preserve these image-backed values.
- Op 0x44 loads the real ramp data; detail 0=low, 1=medium, all others=high. Oracle camera seeds,
  palette merging and the former FIST_TERRAIN scaffolding were retired. Do not restore them.
- AZER1 r92sync.pass01: 23172 writer pixels minus 801 independently identified HUD-overdraw pixels
  = 22371 compared pixels; all matched. This isolates 9200, not the full renderer.
- A deferred tile snapshot once paired frame F+1's tile with frame F's globals. Synchronous capture
  fixed that false mismatch. Settled colour statistics alone do not prove per-texel equality.

## Next

1. Inspect `FIST_6980PROVE` before adding a harness: it already loads a C69G bundle and calls the
   raycaster. Its diagnostic coverage/statistics are not an exact full-tile acceptance gate.
2. Extend it into a portable, strict replay: validate lengths, load every consumed global, ramp,
   map, light colormap, SMC immediate and initial tile from one call boundary; invoke 6980; compare
   every defined output byte. Preserve sky/pre-existing tile bytes rather than assuming zero.
3. Use `tools/oracle/capture_6980_framematched.sh` and
   `tools/oracle/sim_voxel6980_framematched.py` to check bundle semantics. Verify synchronization
   before interpreting a mismatch; cover the detail fill paths 6b03/6b83/6d83/6e03.
4. Isolate 8120 similarly, then run the uninjected 0x08→0x24 chain. Check the first frame's attitude,
   heading readout, map-dot parity and radar phase at matched ticks.

## Accept

Exact tile and projection equality on both targets, then zero-diff composed frames including DAC.
Cover detail changes, night maps and changing camera pose; add hermetic fixtures to the matrix.
Do not mask mismatches or inject oracle inputs in the final end-to-end proof.

## Evidence entry points

`FIST_R92REPLAY`, `FIST_TILESTAT`, `FIST_6980PROVE`; `tools/oracle/capture_9200_framematched.sh`;
`tools/oracle/samples/voxel6980_framematched_pass08.bin.gz` (locate the bundle before use).
