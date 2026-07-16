# Oracle bc9c/bc90 LIVE capture — the 3× colormap flip-flop is settled: the bdc4 matrix is NOT bc9c's output

> ## FINDING (2026-07-16, bc9c-faithfulness settled) — **THE PORT'S bc9c/ac70 ARE ALREADY BYTE-FAITHFUL. The `port_bc9c_matrix.bin` "176 distinct / diag all-80" is a DUMP-WINDOWING artifact, NOT a code bug. NO bc9c/ac70 patch was warranted.**
>
> **Method (offline, decisive).** Dumped the port's bc9c inputs at the exact bc9c call (gdb break on
> `m_ext_FUN_0000_bc9c`, mission AZER1): `5598==5260==sorted-display` (0..79 zero, 80..255 reduced),
> `4f60==5260>>1`, `a060/a460/a860` static, `ac64=80 ac60=255` — all **byte-identical** between bc9c-time
> and the post-89b0 `FIST_PALDUMP`. Reconstructed bc9c+ac70 offline in python (SMC target-subtrahend
> `sub al,target>>1`, `dist=a060[ΔR]+a460[ΔG]+a860[ΔB]`, symmetric-pair fill). Dumped the port's actual
> bc9c **block base** (the 64 KB-aligned buffer bc9c writes — `[bc90] & 0xffff0000 | (ch<<8|cl)`, NOT the
> `+0x4200`-windowed tile pointer that `FIST_BC90DUMP`/`FIST_MTXDUMP` read) via the new default-OFF
> `FIST_BBDUMP` seam.
>
> **RESULT.**
> - **Port bc9c block-base == the faithful offline reconstruction: 65536/65536 = 100.0%.** The patch-238
>   SMC-distance + patch-289 ac70-return are correct; bc9c/ac70 have no residual bug.
> - **Port bc9c block-base, windowed at offset `0xf200`, == the oracle block-B sample on the bc9c region
>   (rows 14..255) = 61952/61952 = 100.0% byte-exact.** The ONLY mismatch under that window is rows 0..13
>   (3584 bytes = exactly `14*256`) — **bc06's LOD-upsample region** (`FUN_0000_bc06`, called with the
>   heightmap ptr `85bc`), which the port writes into a SEPARATE buffer, so the tile's rows 0..13 stay
>   bc9c's `M[242..255]` instead of bc06's output.
>
> **So the port's bc9c matrix IS the oracle's, up to (a) the `0xf200` tile window and (b) bc06 rows 0..13.**
> The committed `port_bc9c_matrix.bin` (diag all-80, 176 distinct) is the `+0x4200`-windowed dump of a
> DIFFERENT alignment — a measurement artifact, not the "collapse" it appeared to be.
>
> **Residual / next blocker (NOT bc9c):** byte-matching the FULL block-B needs (1) the tile-pointer/window
> geometry pinned — empirically `blockB[i] = M[(i+0xf200)&0xffff]`, i.e. oracle tile row 14 aligns with
> `M[0]`; the `[bc90]`/`[3918]` offset that yields it is the extender-allocator (`84c0`) layout follow-up
> the CAVEAT below already flags; (2) `bc06`'s heightmap-LOD block to overlap the tile buffer at rows 0..13
> the way the original's bump allocator lays it out. Both are memory-layout/LOD work, not a `bc9c`/`ac70`
> code fix. Repro: `FIST_BBDUMP=<f>` on the mission flow dumps bc9c's true block base.

**Date:** 2026-07-16 · **Method:** a LIVE build-time capture in the running ORIGINAL under an instrumented
DOSBox (`tools/oracle/dosbox_vga_terrain_trace.patch`, extended this iteration with a `host_writeb` RAM
last-writer hook + a boot-time arm), driving BATTLES→OK→ACCEPT into the AZER1 map-load
(`tools/oracle/trace_bc90.sh`). This replaces the three prior OFFLINE settled-dump iterations
(reduce-LUT → bdc4 → bc9c-matrix) that never converged. Engine PRISTINE (`61453e42`/`0051cb56`/`75c6d726`);
only `tools/native_main.c` gained one default-OFF diagnostic (`FIST_BC90DUMP`).

## Address model (stated + sanity-checked, per the task's rigor requirement)
- Extender **data-segment base = `0x131000`** (from `trace_flat3918`, re-confirmed here: the pointer vars
  `[5578]=[557c]=0x30`, `[85b8]=0x474e60`, `[bc90]=0x10000` all read self-consistently at `EXTBASE+off`).
- `[bc90]` variable @ phys `0x131000+0xbc90 = 0x13cc90`; its value at settle = **`0x10000`** → the buffer it
  points at ("block A") is at phys `0x10000+0x131000 = 0x141000`.
- The buffer bc9c actually WRITES (per the live per-byte last-writer) is at phys **`0x175200`** = ext-flat
  `0x44200` ("block B" = the `tile3918` terrain tile). Sanity: both blocks are full of plausible palette
  indices 0..255.

## What was captured LIVE (overwrite-robust per-byte last-writer, owners identified)
| Buffer | phys | distinct | symmetric | diagonal[0:8] | row0 | owning code |
|---|---|---|---|---|---|---|
| **Block B = bc9c's real output** | 0x175200 | **212** | 13.8% | 63,53,51,48,73,75,59,57 | all 63 | **bc9c** (extip 0xbcf2/0xbcf6 inner stores + 0xbc74/7a shading), 4–6 IPs, no foreign overwriter |
| **Block A = the matrix bdc4 consumes** | 0x141000 | **256** (all 0..255) | 1.3% | 135,120,115,55,110,232,119,99 | 135,128,130,144… | a **memcpy** (`rep movsd`, flatip **0x4708**, cs 0x0008 low seg) called from extender low code (~0x5edc←0x85a4), src `fs:[0x1f0]`, **before the menu** |
| **PORT bc9c output** (`FIST_BC90DUMP`) | — | **176** | 9.0% | all 80 | all 80 | port `89b0`→`bc9c` into the aliased `[bc90]`=tile buffer |

## VERDICT — the colormap matrix is a PRELOADED table, not bc9c
1. **bc9c's LIVE output is block B (0x44200 = the tile3918 terrain tile), NOT block A.** Proven by the
   per-byte last-writer: extip **0xbcf2/0xbcf6** (bc9c's two symmetric-pair inner stores, disassembled from
   `re_out/fist_image.bin`) own ~the whole 64K at phys 0x175200, reproduced across `aa10` + a fresh rebuild.
2. **bdc4 does NOT consume bc9c's output.** The settled oracle colormap (`[85b8]=0x474e60`) has **254
   distinct** values; a 212-distinct matrix (block B) **cannot** produce 254 distinct outputs, so bdc4 read
   a ≥254-distinct matrix = **block A** (256 distinct). This is distinct-count arithmetic, immune to the
   settled-vs-build-time blindness that broke the prior offline iterations.
3. **Block A is built BEFORE the menu by a memcpy, not by bc9c.** Armed at menu, at t=8, at t=4, and finally
   **from process START** (`FIST_MEMARM_BOOT`), the watch on phys/linear 0x141000 shows: **0 writers during
   the whole BATTLES→ACCEPT cascade** (both physical AND CR3-aware flat, paging=0 identity), and the boot-arm
   run identifies a **single owner** — a `rep movsd` block copy at **lin 0x4708** filling all 64K from
   `fs:[0x1f0]`, called from the extender's low code segment (cs 0x0008) during early init.

### Consequences
- **CORRECTION 2 (`docs/oracle_colormap_reduce.md`) mis-attributed block A to bc9c.** The "oracle bc90 is an
  asymmetric-256 matrix" is REAL (block A = 256-distinct, 1.3% symmetric), but it is **NOT bc9c's output** —
  bc9c writes the 212-distinct tile (block B). So "fix bc9c symmetric→asymmetric" targets the WRONG buffer
  for the colormap.
- **The port's bc9c IS unfaithful** (176 vs 212 distinct, diag all-80 vs [63,53,51,48…], only **0.8%
  byte-equal**, 0.0% on the [0x20..0xff]² core) — but that governs the **terrain TILE** (renderer 9200),
  not the colormap. Neither the oracle nor the port bc9c is "perfectly symmetric" (13.8% / 9.0%).
- **The real colormap frontier = block A's source.** bdc4 upsamples the colormap through a **preloaded 64K
  blend table** that the extender copies (memcpy @0x4708 ← `fs:[0x1f0]`) into `[bc90]=0x10000` before the
  menu. The port instead rebuilds `[bc90]` per-map with bc9c (wrong buffer + wrong values). Next iteration:
  locate the source of that copy (a baked table in the extender image, or an earlier one-time build) and
  reproduce it in the port; stop trying to fix the colormap by editing bc9c.

## Samples committed (`tools/oracle/samples/`)
- `oracle_bc9c_matrix_blockB_0x175200.bin` — bc9c's live output (md5 `bddaf448…`).
- `oracle_bdc4_matrix_blockA_0x141000.bin` — the matrix bdc4 consumes (md5 `28f3e3ad…`).
- `port_bc9c_matrix.bin` — the port's bc9c output (md5 `5e005515…`).

## Repro
```bash
# build the instrumented dosbox (README_terrain.md), then:
DOSBOX=/tmp/debs/dosbox-fist FIST_TILEPHYS=0x175200 tools/oracle/trace_bc90.sh   # oracle bc9c output (block B)
DOSBOX=/tmp/debs/dosbox-fist FIST_MEMARM_BOOT=1 FIST_TILEPHYS=0x141000 \
  FIST_WATCHPHYS=0x141000 tools/oracle/trace_bc90.sh                             # block A builder (memcpy @0x4708)
# port bc9c matrix (flaky op-0x18 mission entry, retry):
FIST_DATADIR=<fresh armoredfist copy> FIST_TICK_HZ=25000 \
  FIST_MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0" \
  FIST_BC90DUMP=/tmp/portbc90.bin ./fist_native
```
