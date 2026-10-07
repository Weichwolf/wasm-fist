# Terrain visibility

`fist_ground_visible` in `src/sim/ground.c` owns the complete terrain visibility service
needed by original target discovery. It is shared by native and WASM, borrows the existing
installed height plane and uses the ground sampler's validation, coordinate transform and
row/column indexing. Numeric scratch is local; caller poses, height pixels and RNG survive.
Class-specific source/target aim offsets and runtime target identities remain caller work.
This prerequisite does not wire an incomplete command bank or claim a playable battle.

## Original evidence

Pinned protected-mode image SHA256
`102a5465e03f3b0397041b1bd783d33992ac2c14bb6e42a713af610ec08c3ab1`.
The dispatch table uses operation **byte offsets**: entry d0b for operation 58 contains
1103, whose actual CALL executes 8030..811e. The mailbox pointer is c93; source XYZ are
d2/d6/da and target XYZ are de/e2/e6. Operation 70's 11cb entry is a different service.

The original DOS e1f0/e21c wrapper fills this mailbox from both object poses. It adds target
height from the e588 class table, except type 26 uses its mode-specific 9eaf table, and source
height from e5c0. These zero-extended word offsets are added to DWORD altitudes before the
PM request. This document/API starts at that effective-position boundary. The later target
discovery owner must recover and consume the complete wrapper and identity contracts.

Map-loader instructions 8acf/8ad4 update the two SHLD count bytes at 8105/8109 to the installed
map detail. The image's initial value ten is not a permanent 1024-pixel assumption. The
unchanged original setup also updates contact/center-height samplers, so all use one plane.

## Complete numerical behavior

- Subtract source XY from target XY with wrapped DWORD arithmetic and interpret the results
  as signed. Reject displacement at or above +262144 or below -262144 on either axis. Exactly
  -262144 is accepted; exactly +262144 is rejected. Rejection is a valid invisible result.
- Convert XY to the existing map sampler's 32-bit representation: X shifted left 13 and
  negated Y shifted left 13. Convert altitude difference by a wrapped 16-bit left shift.
- Always subdivide at least once. Arithmetic halve all three deltas, doubling the division
  count, until both XY steps are strictly between -0x03000000 and +0x03000000. Signed halving
  rounds negative steps downward; replacing it with float interpolation changes samples.
- Begin at the source and add the complete XYZ step before each sample. Visit precisely
  division-count minus one interior points. Neither endpoint is tested; even equal XY
  receives one sample. At the range limit there are at most 63 samples.
- Sample the installed plane with wrapped coordinates. Compare the height byte shifted left
  24 against the running altitude shifted initially left 16, both unsigned DWORDs. Equality
  blocks the ray. Fractional altitude, retained low word and wrap therefore affect visibility.
- Stop at the first obstruction, or report visible after all interior points. Original EAX
  returns 0 or ffffffff; the shared semantic API returns a boolean without leaking registers.

The C API reports invalid pointers/height planes separately and leaves its output unchanged.
It allocates no memory and changes no simulation/height inputs. Pose headings are irrelevant
to this terrain-only service. Existing immutable ground-contact and height behavior is reused.

## Verification and remaining work

`tests/original_visibility_oracle.py` executes the pinned original map-detail setup and the
complete unchanged 1103/8030 service with real mailbox parameters. It requires the actual
complete return/stack boundary and exact 0/ffffffff results. After every call it compares
the whole kernel image, mailbox, DGROUP and unrelated stack; only the original 8020..802b
numeric scratch and actual call-stack words are permitted writes. Height memory is read-only.
No instruction or output is replaced by an observation hook.

`tests/test_visibility.py` compares those results with both production targets over byte-height,
fractional/unsigned altitude, displacement/subdivision and coordinate-wrap cases; actual
interior wall/end-point fixtures; deterministic full-DWORD positions; and all eight original
height maps at 512/1024/2048/4096 installed sizes. The corpus includes all 47 pinned scenarios
and 17491 saved opposing-side candidate pairs per detail. Saved XYZ are explicitly supplied
as effective service inputs; this does not claim prepared runtime aiming or target selection.
Missing/truncated/trailing/count/argument input fails without partial output. API failures
preserve the output. The probe owns decoded input and poisons/frees its source before use.

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_visibility.py \
  --target native --originals --oracle
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_visibility.py \
  --target wasm --originals
python3 tools/check_style.py
bash tools/build.sh all
```

Exact accepted results belong in WI 0091. Full target discovery, stable runtime references,
all heading/command/class callbacks and consuming battle/audio remain in WI 0081/0041/0065.
The complete rewrite goal and independent final ten-run WASM acceptance remain open.
