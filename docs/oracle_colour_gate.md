# The mission-terrain COLOUR gate — 6980 reads a LIGHT colormap; the port's dark C32 is the defect

> ## HARD VERDICT (2026-07-17) — **THE MISSION-TERRAIN COLOUR IS SOLVABLE, not a paged-out-data
> frontier.** A frame-matched capture of `FUN_0000_6980`'s OWN render-time state (world camera + depth
> ramps + the colormap it dereferences, snapshotted DURING 6980, not at 9200-entry) proves, three
> independent ways, that **the colormap 6980 reads at render time is LIGHT** (mean 172, max 228). The
> port reads DARK because its flat `[0x85bc]+0x100000` holds the raw dark `C32.KLC` (max 104); the
> original's colormap slot holds the **bdc4 LIGHT reduce** (max 228) at render time. This is the
> documented `85b8` collapse, now pinned from the 6980 side with a decisive, geometry-independent proof.
> Engine PRISTINE (`61453e42`/`0051cb56`/`75c6d726`/`1e0cfd38`). Only the DOSBox oracle
> (`FIST_R6980CAP`) + `tools/oracle/` + one banked sample changed; no engine/native_main edit.

**Method:** extended the instrumented DOSBox seam (`tools/oracle/dosbox_vga_terrain_trace.patch`,
`FIST_R6980CAP`, driven by `tools/oracle/capture_6980_framematched.sh`) to, for each settled AZER1 spawn
frame: (1) snapshot 6980's world camera + `3a24/3e24/4224/4624` depth ramps at the first 6980-body write;
(2) accumulate 6980's actual tile stores (`6b2b`/`6bab` `rep stos al`) into a shadow — these ARE the bytes
it read at the colour load `6b1a`/`6b9a` `mov al,[esi+ecx+0x100000]`; (3) a read-side hook on the colour
load recording the REAL effective phys the CPU dereferences; (4) dump the render-time heightmap/colormap/
reduce. Reproduction: `python3 tools/oracle/sim_voxel6980_framematched.py .`
Banked frame-matched bundle: `tools/oracle/samples/voxel6980_framematched_pass08.bin.gz`.

---

## (A) GATE A — is the colormap 6980 reads LIGHT or DARK at render time? **LIGHT. Definitively.**

`FUN_0000_6980` (image `0x6980..0x6bf6`) reads colour with a FIXED displacement, no SMC redirect (verified
in both column loops):
```
6b1a / 6b9a:  mov al, BYTE PTR [esi+ecx*1+0x100000]   ; esi=[0x85bc], ecx=coord
6b2b / 6bab:  rep stos BYTE PTR es:[edi], al          ; stores that byte VERBATIM into the tile
```
So **6980's tile stores == its colour reads.** Three independent proofs the reads are LIGHT:

| proof | result |
|---|---|
| **direct store capture** (every settled pass 01..15) | stores mean **172-174**, min 116-123, **max 228**, ~38-41 distinct — LIGHT |
| **value-set containment** (geometry-independent) | 6980's 38 distinct store values: **38/38 present in the LIGHT reduce** (`[0x85bc]+0x400000`, max 228); **0/38 present in the dark C32** (`[0x85bc]+0x100000`, max 104) — the dark C32 physically CANNOT emit any colour 6980 writes |
| **read-side phys hook** | the colour load's real phys target (1MB page `[0x100000,0x200000)`) contains **39/39** of 6980's store values |

**The paradox that burned 14 prior iterations, resolved:** in EVERY full-RAM dump / `mem_readb` view, flat
`[0x85bc]+0x100000` (= `0x174e60`) resolves to a DARK page (CR3-walk phys `0x1f6e60`, max 104). But the
read hook proves that AT 6980's read moment the SAME flat slot returns LIGHT. The Doug-Huffman extender
**re-pages the colormap slot** — during the per-frame render it maps `[0x85bc]+0x100000` to the LIGHT bdc4
reduce; by the frame boundary (where every prior dump was taken) it maps back to the dark raw `C32`. That
is why 14 iterations of ram-dump analysis found the colormap dark: **they all watched the wrong (frame-
boundary) page.** `mem_readb` at ANY point (even mid-6980) also returns the dark page — only the CPU's real
PM-paged read returns light — so this is invisible to every offline flat/`mem_readb` probe and required the
read-side instruction hook.

### The port defect (pinned)
The port's flat `g_mem[[0x85bc]+0x100000]` holds the decoded **dark `C32.KLC`** (mean 47, max 104). The
original's colormap read there returns the **bdc4 LIGHT reduce** (`[0x85b8]`=`[0x85bc]+0x400000`, mean 141,
max 228, byte-superset of all 6980 store values). So the fix is exactly: **6980's colour read must return
the light reduce, not the raw C32.** This is the same `85b8`/`bdc4` reduce the colormap frontier documents
(`memory: colormap-groundtruth` part 2) — now confirmed as the windshield-terrain colour crux from 6980's
own dereference, ending the "85b8 is not the windshield residual" ambiguity of that note's part 2.

## (B) GATE B — are the depth ramps `3a24`/`3e24` reconstructable? **Yes, by banking (engine constants).**

- `3a24[250]` / `3e24[250]` are `0x00000001×256` in `re_out/fist_image.bin` (paged-boot-filled, absent from
  the image, confirmed by full-image scan). Captured render-time values are smooth monotonic ramps
  (`3a24`: `197d 2fff 3e32 4e6b 60b6 …`; `3e24`: `eef4d 1c1f86 247118 …`) with **no closed-form formula**.
- **They are engine CONSTANTS:** byte-identical across two independent capture runs (`r69` vs `lt2`),
  camera- and frame-independent. So they are reconstructable **by banking the 250-dword tables** (like a
  resource), not by formula.
- The derived tables ARE reconstructable from them: `4224[d] = (3a24[d]·90c0)>>24` reproduces **byte-exact**
  (the `FUN_0000_395e` `mul`/`shrd 0x18` idiom), and `4624[d] = 3e24[d]` (a copy). The projection table
  `[0x3909]` is likewise built by `395e` from `3a24/3e24 + 90c0` — reconstructable once the ramps are banked
  (the port's earlier 1.75% `395e` mismatch was from feeding it the image's all-1 ramps).

## (C) Byte-exact whole-tile reproduction — honest status

| stage | vs pass08 | note |
|---|---|---|
| 689a (rows 160-255) | ~100% of band = **55.55% whole-tile** | PINNED (`sim_lighttile_689a.py`), unchanged |
| 6980 (rows 0-159), oracle ground truth | **100%** | the captured 6980 **shadow == the settled tile** where 6980 wrote (byte-exact, direct) |
| 6980 (rows 0-159), OFFLINE from-scratch sim | **~1-2%** | my offline march over-draws 3.3× and samples darker cells |

The from-scratch offline reproduction is NOT byte-exact — but the block is an **offline-reconstruction
limitation, not missing data.** The render-time heightmap and colormap are paged NON-CONTIGUOUSLY into a
phys page that differs from the flat/dump view; reproducing the exact per-coord sample offline needs the
render-time page tables (which differ from the dump-time CR3 the walk recovers). The PORT does **not** need
any of that: it executes the real 6980 over its own flat `g_mem` — it only needs the correct buffer CONTENT
(the light reduce at the colour slot + the banked ramps), then its native reads land correctly. The colour
question the gate asks (is the data present and light) is answered YES; the offline byte-match is a separate
reconstruction-fidelity task, not a solvability blocker.

## VERDICT + integration plan — COLOUR IS SOLVABLE

The mission-terrain colour is a **bounded reconstruction**, in priority order:
1. **DOMINANT — the colormap content.** Make 6980's colour read (`[0x85bc]+0x100000`) return the **bdc4
   LIGHT reduce** (max 228), not the raw dark `C32` (max 104). This is the `85b8` collapse: the port aliases
   `[bc90]`→tile3918 across `89b0` so `bdc4` reads the wrong buffer (see colormap-groundtruth part 2, which
   already root-caused the port's 85b8 89→256 collapse). Land that bdc4 fix AND ensure 6980's colour slot is
   fed the reduce (whether by populating `[0x85bc]+0x100000` with the reduce, or pointing 6980's colour base
   at `[0x85b8]`). Byte-exact oracle: the banked light reduce in `voxel6980_framematched_pass08.bin.gz`.
2. **Bank the depth ramps.** Add `3a24[250]`/`3e24[250]` as a constant data table (paged-boot values, banked
   here); build `4224/4624/[0x3909]` via the faithful `395e` (byte-exact from the ramps).
3. **SMC-model 6980's proj lookup** (`re_out/fist_ext.c` ~10980/11005): thread the `[0x3909]` VALUE (not the
   label address) and the per-depth `+0x100`-incremented SMC base, so the port doesn't deref the static
   `0x7fffffff` placeholder.
4. **Wire the per-frame fill** `3931 → 689a → 6980` before `9200` in op-0x24 (per
   `memory: mission-terrain-tile-pipeline`), with the 689a source (`ds:0x3911` = raw 5.SKY, proven byte-exact).

None of these is paged-out unreachable data: (1) is a buffer-content fix already half-root-caused; (2) is a
250-dword constant banked here; (3)/(4) are asm-faithful reconstructions of code present in `fist_ext.c`.

## Repro
```bash
python3 tools/oracle/sim_voxel6980_framematched.py .          # gate-A containment verdict, self-contained
DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/capture_6980_framematched.sh   # re-capture (needs instr. dosbox)
# stderr [r6980] lines = per-pass store LIGHT/dark; [r6980rd2] = the real colour-read phys
```
