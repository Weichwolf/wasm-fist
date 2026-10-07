# Owned height-field resampling

`src/assets/heightfield.c` owns the numerical square-height resampling used before ground
installation. It accepts a complete decoded `fist_klc_image` and an explicit requested side,
then returns an independent owned image. It uses no original engine code at runtime.

The original protected-mode map loader `89b0` loads the height KLC into the block at `85bc`.
At `8bbd..8c16`, it repeatedly calls `bc06` to double or `bed2` to halve both dimensions
until they equal the selected runtime side at `8494`. The setup derives that side as
`1 << [8490]` at `8a56..8a63`. The initial detail byte comes from TCB +59h;
existing `4.MEG`, `8.MEG`, `16.MEG` and `40.MEG` marker files override it to 9, 10, 11
and 12 respectively, giving 512, 1024, 2048 and 4096. These are DOS file-existence checks,
not a measured host-memory budget.
The API does not choose a gameplay/detail setting. The caller supplies it explicitly.

## Numerical contract

Doubling first expands columns, then rows. For each input row, even columns preserve the
source sample. Odd columns contain `floor((left + right) / 2)`, with the right neighbor
wrapping to column zero at the seam. Even output rows preserve those expanded rows. Odd
rows average the current and following expanded rows, wrapping to row zero at the seam.

The original `ADD AL,AH; RCR AH,1` carries the ninth sum bit into the result, so values near
255 must not wrap before division. Each axis rounds down separately. For the four corners
`0,1 / 1,2`, the new odd/odd sample is zero; one direct four-corner average would give one.
There is no random height perturbation in this operation. Repeated doubling preserves
original knots at spacing `2^steps`.

Halving selects `source[2*row, 2*column]` exactly; it does not average four samples. Both
operations require square planes. Sides need not themselves be powers of two: original
24-square stamps also work. Source and target must have an exact power-of-two ratio.
An unchanged dimension still creates an independent copy. Palette bytes remain unchanged.
The numerical stamp/colormap fixtures exercise these height operations only; original
colormap enlargement `bdc4` uses a different palette interpolation and remains separate.

`fist_heightfield_resample()` returns -1 for null/aliased arguments, missing source pixels,
non-square/zero dimensions, invalid ratios, size overflow or allocation failure. It publishes
output only after all stages succeed and preserves the source. The caller supplies a complete
plane, an output that owns no image, and distinct source/output objects. Release output with
`fist_klc_destroy()`. No source storage or palette view is retained.

## Verification

```sh
bash tools/build.sh all
python3 tools/check_style.py
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_heightfield.py --originals
```

Default native/WASM gates compare complete dimensions, palettes and pixels against independent
separable fixtures. Every pair of byte values appears on each axis, including carry and
rounding boundaries. Tests cover one-cell periodic fields, asymmetric seams, staged rounding,
unchanged copies, repeated up/down sizing, non-power-of-two sides, exact knot selection,
malformed/truncated inputs, null/alias contracts and 32-bit size overflow.

Each stage checks source preservation, then overwrites and releases all input pixels/palette
before observing output or continuing. Allocation failure is real: the native probe runs
under a 64 MiB address-space limit; the WASM probe has a bounded 64 MiB heap and non-aborting
allocation. An 8192-square request reaches intermediate allocation failure and must preserve
both the source and the unpublished output. These are probe limits, not gameplay quality limits.

`--originals` requires pinned Unicorn 2.1.4 and the frozen extender image, plus all 18 square
original planes. Four rectangular SKYs are outside this square-height contract. The buffered
original KLC decoder independently supplies inputs, verified against the existing source and
decoded hashes. All eight Dxx height maps are tested at each actual runtime side and through
4096 → original-side round trips. Numerical tests also compare actual original resampler returns.

The new oracle executes complete `bc06..bc8c RET` and `bed2..bf02 RET`, with their normal
in-place buffers and dimension globals. A separate return address checks completion and stack
balance. No instruction hooks, patches or replacement calls are used. Every output byte is
compared with native/WASM; missing originals, incomplete runs and requested skipped coverage fail.
Original files are read-only and their hashes must remain unchanged.

For native memory instrumentation:

```sh
clang -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function \
  -fno-strict-aliasing -ffast-math -Werror -O1 -g -fsanitize=address,undefined \
  -fno-omit-frame-pointer -Isrc -Itests \
  src/assets/heightfield.c src/assets/klc.c tests/probe_io.c \
  tests/heightfield_probe.c -o /tmp/wasm-fist-0062-heightfield-sanitized
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 \
  PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_heightfield.py --target native \
  --native-probe /tmp/wasm-fist-0062-heightfield-sanitized --originals --no-memory-tests
```

The explicit `--no-memory-tests` excludes only RLIMIT_AS, which conflicts with sanitizer shadow
memory; both production targets must pass that test separately. This prerequisite does not
change displayed frames. The terrain preview still uses decoded base planes. Height/slope
sampling, installed vehicle state, collisions and interactive mission integration remain open.
