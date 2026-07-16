# Oracle recon — the 85b8 colormap "collapse" (89 vs 254) is NOT the reduce; it is the SPATIAL UPSAMPLE

**Date:** 2026-07-16 · **Method:** offline reproduction of the extender reduce/upsample pipeline against
the committed oracle RAM dump (`scratch/oracle/activate.ram.bin`, ext DGROUP @ guest-phys `0x131000`) + a
live port `FIST_CMDUMP`/`FIST_PALDUMP`/`FIST_MTXDUMP`/`FIST_AC70PROBE` capture at op-0x18 map-load +
standalone C harnesses that run the BUILT `ac70`/`bdc4` verbatim. Engine PRISTINE
(`61453e42`/`0051cb56`/`75c6d726`); only `tools/native_main.c` gained two default-OFF, op-0x18-gated
diagnostics (`FIST_MTXDUMP`, `FIST_AC70PROBE`).

## THE PRIOR PREMISE IS CORRECTED
The prior recon (`docs/stage1.md`) framed this as an *"irreconcilable static contradiction — identical
faithful reduce inputs, yet oracle=254 / port=89"* and pinned the resolution on an oracle **reduce-point
capture** to catch a hypothesised build-time `4f60` that differs from the settled-frame dump. **Both halves
are now settled decisively, and the hypothesis is DISPROVEN:**

### (1) The reduce INPUTS are byte-identical AND stable (no hidden build-time 4f60)
- port `4f60` == oracle `4f60` **768/768**; port `5260` == oracle `5260` **768/768**; both a060/a460/a860
  distance tables == the baked extender image **1024/1024** each; `ac64`=`28a5`=**80** both.
- `4f60` is written **exactly once** by `a033` (`4f60 = 5260>>1`) and `5260` **exactly once** by `9f70`'s
  `rep movs 5598→5260`; **no other writer exists** (image scan). So the settled-frame `4f60` **IS** the
  build-time `4f60` — the "dump 4f60 = 32 distinct ≠ build 4f60" premise was a mis-measurement (oracle
  `4f60` is **176 distinct**, band [80..255] = **175**, NOT 32).
- The reduce-time source `5598` = `C32.KLC` embedded palette `>>2` = **252 distinct** (the port keeps it at
  op-0x18 end; the oracle overwrites 5598 to 25 distinct *after* the reduce, which is why the settled dump
  looked collapsed — but that is post-reduce and irrelevant).

### (2) The reduce LUT is INHERENTLY 81-distinct — it CANNOT be the 254 source
`9e60` builds `85c0[i] = ac70(nearest 4f60 index in band [ac64..255] of 5598[i])`. With the (identical)
oracle `4f60` + `C32pal>>2` source, the LUT is **81 distinct at EVERY ac64 (0/15/40/63/80)** — the AZER1
desert colours cluster onto 81 of the 176 band entries no matter where the band starts. LUT[1..8] =
`145,128,152,169,183,204,133,121` (bit-matches patch 288). `9ec0` remaps each colormap texel through this
LUT ⇒ the base colormap can hold **at most ~81 distinct** display indices. **The reduce is faithful and is
physically incapable of producing 254 — the richness is NOT in the reduce.**

### (3) The richness is SPATIAL (the upsample), and the port omits it
Rendered PNGs of both 2048² colormaps are the **same AZER1 desert, pixel-aligned** (same craters/ridges),
but:
- oracle colormap = **254 distinct, SMOOTH gradient** (fine ramps, e.g. `128→132→140→145→151` between
  adjacent texels); base-512 (every-4th sample) = **206–234 distinct at ALL 16 phases**.
- port colormap = **89 distinct, BLOCKY** (4-wide constant runs `166 166 166 166` with abrupt jumps);
  base-512 = **80–88 distinct at all phases** ≈ the 81-value LUT replicated.
So the oracle produces a finely-graduated terrain (≈ the 252-distinct raw-texel richness), while the port
replicates the coarse 81-value LUT. The gradation the oracle adds — a bilinear/dither spatial smoothing in
the **`bdc4` colormap upsample** (512²→2048² via the pairwise-blend matrix `bc90`) — is the divergence.
This is the task's **option 3 (a post-reduce dither/detail pass the port omits)**, now proven, NOT the
reduce inputs (option "build-time 4f60") the prior recon assumed.

## SECONDARY (real, but not the colormap's cause): the port's blend matrix `bc90` is CORRUPTED
`bdc4` interpolates via the pairwise-blend matrix `M[a][b] = ac70((pal[a]+pal[b])/2)` at `ds:0xbc90`
(the shim aliases `bc90 → tile3918` during map-load). Dumped port matrix (`FIST_MTXDUMP`):
- oracle blend matrix (the TILE `0x44200`, windowed) diagonal = `80,80,81,81,82,82,…` (**identity**, as
  it must: `M[i][i]=ac70(pal[i])=i`), ~symmetric.
- **port matrix diagonal = `102,102,103,102,107,111,118,…` (≈ i+22, WRONG) and only ~10% symmetric** —
  `bc9c` writes a *perfectly symmetric* matrix, so the dumped tile3918 is CORRUPTED (overwritten/mis-built).
This UNIFIES the colormap issue with the documented `docs/oracle_tile3918_producer.md` tile corruption
(same buffer). **However it is NOT the colormap's cause:** the BUILT `ac70` is correct (standalone harness
returns the identity `80,81,82,…`), and the BUILT `bdc4` run in a standalone harness with a *correct
symmetric matrix* still yields **89 distinct (blocky)** — so a perfect matrix does not fix the colormap.
The corruption's exact origin (a `bc9c`-time state / a post-`bc9c` overwrite of the aliased tile3918)
could not be pinned this iteration because the mission-start path (op 0x18) is the documented flaky/crashy
frontier — the `FIST_MTXDUMP`/`FIST_AC70PROBE` runs reach op-0x18 only intermittently and often SIGSEGV in
the mission cascade before the seam.

## PINNED NEXT FIX (ordered)
1. **The `bdc4` colormap upsample is the colormap frontier, NOT the reduce.** Reconstruct `FUN_0000_bdc4`
   (asm `0xbdc4..0xbed1`, TWO passes: pass1 horizontal double via `M[left][cur]`; pass2 vertical double
   with the `test al,1` **parity-dither** selecting `M`-lookups) EXACTLY from asm and verify real-`bdc4`
   (with a correct symmetric `M`) yields the smooth ~254-distinct gradient. The port's C `bdc4` preserves
   originals / replicates (blocky 89) → it is the unfaithful reconstruction to fix. (A separable
   horizontal-then-vertical model of `bdc4` also only reaches ~92, confirming the real algorithm's
   dither/diagonal interpolation is the missing richness.)
2. **Separately** fix the `bc90` blend-matrix corruption (asymmetric +22 diagonal) — same root as the
   `tile3918` corruption; needs a `bc9c`-time capture (drive op-0x18 reliably, e.g. via a
   crash-hardened/pattern-init mission run or a hook inside the map-load) to see whether `bc9c` fills it
   wrong or a later pass overwrites the shim-aliased tile3918.

## Repro
```bash
# port capture (default make native, flaky mission entry — retry):
FIST_DATADIR=<fresh armoredfist copy> FIST_TICK_HZ=25000 \
FIST_MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0" \
FIST_CMDUMP=/tmp/portcm.bin FIST_PALDUMP=/tmp/portpal.bin FIST_MTXDUMP=/tmp/portmtx.bin ./fist_native
# oracle: scratch/oracle/activate.ram.bin, ext base 0x131000; colormap ptr [0x85b8]=0x474e60 (+base),
#   tile/blend-matrix at 0x44200 (+base), 4f60/5260/a060 at those ext offsets.
```

## CORRECTION 2 (3rd colormap iteration — bdc4 is FAITHFUL; frontier relocates to bc9c; STOP offline-chasing)
The prior section pinned `FUN_0000_bdc4` (the colormap spatial upsample) as the unfaithful reconstruction.
That is **DISPROVEN**: an instruction-exact emulator of `0xbdc4..0xbed1` AND the Ghidra C (`fist_ext.c:16965`)
AGREE bit-for-behavior — both give distinct=89 on a symmetric blend matrix and distinct=256 on the oracle's
matrix. **bdc4's C == the asm; it needs no change.** The prior "harness proof bdc4 is broken" was doubly
flawed: its base input was the port's already-blocky FINAL output downsampled, and its `correctM` was a
SYMMETRIC blend matrix that is NOT what the oracle feeds bdc4.

**Relocated frontier:** bdc4 reads the 64K-aligned matrix at `[0xbc90]` (=0x10000). Decisive test (faithful
bdc4, varying only the matrix, on a blocky base): symmetric `correctM` -> 89; port `3918` -> 154; **oracle's
real `bc90` matrix -> 256 distinct.** So the collapse is that the port's `bc9c` writes a SYMMETRIC blend
`M[a][b]=ac70((pal[a]+pal[b])/2)`, but the oracle's `bc90` is an **ASYMMETRIC 256-distinct table with a
non-identity diagonal `[124,104,138,139,...]`**.

**⚠️ CAVEAT — this CONFLICTS with the prior "bc9c byte-proven faithful / tile3918 (0x44200) byte-matches the
oracle 212-distinct" claim (docs/oracle_tile3918_producer.md).** The colormap root has now flip-flopped
THREE times this session (reduce-LUT -> bdc4 -> bc9c-matrix), each on OFFLINE/settled-RAM-dump analysis, and
each disproving the last. That pattern says offline analysis of the settled dump is NOT converging — the
settled dump is blind to build-time state, and "the oracle matrix" reconstructed offline may not be what
bc9c should produce. **RESOLUTION REQUIRES the not-yet-built instrumented `dosbox-fist` to capture the
oracle's `bc90` matrix (and bc9c's live inputs) AT BUILD TIME in the running original** — both colormap
agents were blocked on exactly this (dosbox-fist absent, port op-0x18 flaky). Until that tooling exists,
do NOT sink more offline colormap recon iterations; the honest next step for the colormap is BUILDING the
instrumented DOSBox capture, not another settled-dump reinterpretation. Pivoted away this iteration.
