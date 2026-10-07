# Target proximity

`fist_planar_proximity` owns the complete a17e/08e8 directional distance estimate in shared
C11. Target discovery b011 uses this service instead of the 0541 navigation measurement.
Both are in `src/sim/geometry.c`; they have distinct algorithms and callers. Replacing the
proximity estimate with Euclidean distance changes actual target selection. This prerequisite
does not supply runtime target identities, aim acquisition or the complete command bank.

## Recovered contract

Pinned DOS image SHA256
`d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5`.
a17e calls 08e8 and returns far. The complete 08e8 method reads two XY points from SI/DI,
computes wrapped DWORD DI-minus-SI differences, and folds each signed negative lane with
CDQ/XOR. It omits the increment of a usual two's-complement absolute value:
nonnegative lanes stay unchanged; negative lanes become `abs(delta)-1`. Thus -1 folds to zero
and INT32_MIN to 2147483647. The smaller folded lane is logically halved and added to the
larger; ties retain the same result. Maximum output is 3221225470, within unsigned DWORD.

The full result returns in DX:AX and is copied to DS:045a/0458. TEST DX defines the zero flag
from its high word and clears carry. Input SI/DI, coordinates, RNG and unrelated DGROUP stay
unchanged. The C API returns that complete unsigned value with local scratch and immutable
value parameters. It uses unsigned subtraction/complement/shifts, with no signed overflow,
floating approximation, guessed precision constants or shared register storage.

Argument order matters because the negative fold is directional. The API follows the other
numeric helper's `target-source` order. Actual discovery passes candidate XY as SI/source and
actor XY as DI/target. It rejects nonzero DH, then packs distance bits 8..23 into its word
range comparison. Default/link ranges and preference/secondary selection remain caller work;
the primitive does not silently clamp or normalize those results.

## Reaching evidence

The original b011 near wrapper calls the complete f69:b378 service at physical 1aa08. With
actor XY (0,0) and two eligible opposing M1 candidates, the actual return selects registry 2:

| Candidate XY | Original proximity / packed range | Navigation distance / packed range |
| --- | --- | --- |
| (-25600,-25600), registry 1 | 38400 / 150 | 36203 / 141 |
| (-37120,0), registry 2 | 37120 / 145 | 37120 / 145 |

The constructed fixture has three actual pool allocations, complete saved payloads and a
declared manual actor with no old target. The original e1f0/e21c wrappers produce effective
source altitude 6144 and target altitude 5888 from the real class tables. Both actual PM
op-58/8030 returns on a flat installed 512 plane are visible. The adapter transfers the real
kernel EAX, then executes all original wrapper/discovery continuation and the b011 near return.
No instruction, comparison or visibility return is patched or substituted. Registry 2, nearest
word 145 and winner-update byte 2 are required, with unchanged seeds/cursor and unrelated
65536-byte DGROUP state. Complete numeric scratch and actual call-stack destinations are
declared. Existing range 1000 admits both candidates.

Notification gate word 6da2 is explicitly zero in this fixture. Actual bf3c admission therefore
returns before issuing a device request; its code still executes. This is a declared scalar/
selection proof, not coverage of notification playback, secondary targets, priority changes,
old-target invalidation, full device setup or complete discovery acceptance. Those remain
under 0081 and must be consumed with runtime identities before wiring the parent phase.

## Verification

`tests/original_proximity_oracle.py` executes the complete unchanged scalar a17e/08e8 return,
including exact result scratch, preserved SI/DI, full DGROUP/input/RNG and TEST DX flags.
Its reaching observer executes the real scan plus both aim wrappers and PM visibility calls.
`tests/test_proximity.py` checks every low-word axis/diagonal in six signed directions, DWORD
boundaries, ties, wrapped absolute positions, packed range edges, full mode-byte independence,
deterministic full-width positions and 17491 original opposing-side pairs in both directions
from every one of the 47 pinned scenarios. Source bytes are owned and poisoned/freed by the
existing geometry probe before use. Malformed/trailing/count/missing/argument failures have
no partial output. The original navigation API/probe interface remains available unchanged.

```sh
PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache \
  /tmp/wasm-fist-decoder-oracle/bin/python tests/test_proximity.py \
  --target all --originals --oracle
python3 tools/check_style.py
CTEST_PARALLEL_LEVEL=4 bash tools/build.sh all
```

Exact accepted domain, memory/build evidence and limitations belong in WI 0092. Continue the
complete runtime aiming/identity/discovery and heading/class/audio contracts under 0081;
the first playable mission and full rewrite acceptance remain open.
