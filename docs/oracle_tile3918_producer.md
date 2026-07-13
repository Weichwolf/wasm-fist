# Oracle recon — the DEFINITIVE tile-3918 producer + the mission-TCB camera source

> ## CORRECTION 2 (2026-07-13, patch-290 iteration) — the `4f60`/`ac64` GATE IS DISPROVEN BY BYTE EVIDENCE (the port already reproduces it exactly), AND `bc9c` is DEFINITIVELY not the producer. Tile 3918 is a STATIC atmospheric depth-fog LUT built by a LATE map-load pass that the oracle backtrace never captured.
>
> **This supersedes CORRECTION 1's "the build-time `4f60` is the gate" claim.** Measured directly from the
> port's own `89b0` map-load (temporary `FIST_PALDUMP2` seam, reverted) against the committed oracle RAM
> dumps (extender DGROUP @ guest-phys `0x131000`):
>
> **(A) The `4f60`/`ac64` premise is DISPROVEN — the port's display palette is ALREADY byte-exact.**
> - port `4f60` (the halved display palette `ac70` searches) == oracle `4f60` **768/768** bytes (band
>   `[80..255]` **528/528**, reserved `[0..79]` **240/240**, both all-zero in the reserved band).
> - port `5260` (the sorted display palette, = bc9c's blend source at build time) == oracle `5260` **768/768**.
> - port `ac64` = **80** == oracle `ac64` = **80** (both = `28a5` = mission-TCB `[+0x54]`).
> - port `ac70` distance LUTs `a060`/`a460`/`a860` == oracle **3072/3072**.
> - **Build-time `ac64` = 80 CONFIRMED (CORRECTION 1's "ac64<63" was an artifact):** the pure `bc9c`
>   search region (tile rows 14..255) is cleanly in `[80..255]`; the only sub-80 values (37..79) live
>   EXCLUSIVELY in the `bc06` LOD region (rows 0..13, which does not call `ac70`). CORRECTION 1 read the
>   raw 24-colour mission palette `5598` instead of the 176-colour sorted display palette `5260`/`4f60`
>   that `a033` actually builds and `bc9c`/`ac70` actually consult.
>
> **(B) With ALL of bc9c's inputs proven byte-exact, bc9c STILL gives only 32.6% (100% symmetric).**
> - port `bc9c` → tile **32.6%** match (21333/65536), **100.0% symmetric** (`mov [ecx],bl; xchg ch,cl;
>   mov [ecx],bl` writes `t[a][b]==t[b][a]` by construction; inner loop starts `ch=cl` = upper triangle
>   + mirror).
> - oracle tile is **15.7% symmetric** (9194/58564 in rows 14..255). A symmetric writer cannot make a
>   15.7%-symmetric buffer, and correct inputs don't help → **bc9c is not the producer**, full stop.
>
> **(C) Tile 3918 is a STATIC atmospheric DEPTH-FOG LUT, not a pairwise blend.** Structure (measured):
> for each column `c`, tile rows 14..~90 are a CONSTANT base-band index and rows ~91..255 are a monotonic
> BRIGHTENING ramp (haze) — e.g. col 80: rows14-90=`80`, then 81,82,…,155; col 100: rows14-90=`85`, then
> 92,…,195. Row = depth/distance, col = colormap value. The tile is **byte-identical across two
> different-camera settled frames** (`activate` vs `act4d0e`, TCB `[+0x2c]`=`0x9248a` vs `0x92f40`) →
> **built once at map-load, sampled per-frame** by `6977` (builds 256 row-pointers `4a60[i]=3918+i*0x100`,
> `38ed`=`0x100`, `391c`=`0x10000`) → renderer `6980` (op-0x09 per-frame entry `3931`→`85d0`→`6980`).
>
> **(D) By dump time the bc9c symmetric fill is GONE — everything is post-processed.** At dump `bc90`
> (0x40000 block) is only **3.1% symmetric** (201 distinct) and `bc94` **2.1%** — neither holds bc9c's
> symmetric output. The oracle writer-backtrace (`tile3918_writers_backtrace.txt`) caught ONLY `bc9c`
> (`bcf2`/`bcf6`, tileoff `0e00..ffff`) + `bc06` (`bc2a`/`bc30`/`bc74`/`bc7a`, `0000..0dff`) inside its
> ARMED WINDOW → **the real fog-LUT producer runs AFTER that window and was never traced.**
>
> **(E) Secondary layer:** the port's colormap `85b8` still collapses to **89 distinct** (oracle **254**) —
> the tile's per-column base index is a colormap→palette remap the port also gets wrong, an independent bug.
>
> **CORRECTED FRONTIER (patch-290 pins, no engine change — forcing a fix on the disproven bc9c/4f60 model
> would be a band-aid):**
> 1. Re-instrument the oracle to trace phys `0x175200` writers PAST the `bc9c`/`bc06` pass all the way to
>    the dump (arm-late / never-disarm), to CATCH the post-bc9c fog-LUT producer. Candidate families:
>    `bd0e`/`bd62` (shade `ac70(pal·brightness)` into `bc94`) generalized to 256 depth levels, and a
>    distance/haze attenuation pass keyed on the mission fog colour.
> 2. Fix the colormap `85b8` collapse (89→254 distinct) — its `643c` decode + `9ec0`/`ac70` reduce — since
>    the fog LUT's per-column base index is derived from it.
> 3. Only then is the tile reachable byte-exact; the shim's `bc90→3918` alias (which forces bc9c to stomp
>    the tile symmetric) must be RETIRED in favour of driving the real producer into `3918`.
>
> The `4f60`/`5260`/`ac64`/dLUT reconstruction is DONE (byte-exact); it is a correct prerequisite, not the
> producer. Everything below (and CORRECTION 1) is superseded on the "producer" question.
>
> ---
>
> ## CORRECTION 1 (2026-07-13, implementation iteration) — the "symmetric pairwise palette-blend" model does NOT reproduce the observed tile; the real gate is UPSTREAM (the build-time display palette)
>
> Reconstructing bc9c per this doc and byte-comparing against the oracle ref
> (`scratch/oracle/oracle_tile3918.bin`, re-verified == phys `0x175200` in FOUR RAM dumps) DISPROVES the
> symmetric-blend producer as the *complete* story. Measured facts:
> - **The oracle tile is NON-SYMMETRIC** — only 9252/65536 cells satisfy `t[r][c]==t[c][r]` (15.7 % in the
>   pure-bc9c region rows/cols 14–255). But bc9c writes a **symmetric** pair (`[ecx]=bl; xchg ch,cl; [ecx]=bl`
>   ⇒ `t[ch][cl]==t[cl][ch]`). A symmetric writer cannot produce a 15.7%-symmetric buffer.
> - The **window/low-16 reconciliation** ("bc9c fills the `0x40000`-aligned block, tile `[3918]=0x44200`
>   windows it at linear `+0x4200`=row 66") is also **disproven**: the port's aligned symmetric fill windowed
>   at `0x4200` matches the oracle **1.1 %**; the best window offset is `0` at **32.6 %** — i.e. even col-0
>   alignment only gets the ~½ the symmetry allows, and the values are substantially off.
> - **ROOT CAUSE (oracle-proven), the tile is built from a BUILD-TIME display palette the port does not
>   reproduce.** Oracle `t[i][i]=ac70(pal[i])`: diag = `[63,53,51,48,73,75,…]` — **low** indices, with
>   `pal[0]=white ⇒ 63`. So at build time `4f60[63]=white` and the search start `ac64<63`. But:
>   * the **dump-time** `4f60` has entries **0..79 ZERO, 80..255 = mission colours** (176 nonzero) and the
>     dump-time reserved-count `28a5=ac64=80` — a DIFFERENT palette state than at build;
>   * the **port's** `a033` builds `4f60 = mission_pal>>1` (only entries 0..23 nonzero, since the mission
>     `5598` palette is only 24 colours) and reads the dump-time `ac64=80` ⇒ ac70 searches the empty 80..255
>     reserved range ⇒ returns a CONSTANT (diag=80, or 0 with ac64=0). Three different `4f60` states, none
>     the build-time one.
> - ⇒ **The true frontier is upstream: the extender's DISPLAY-PALETTE (`4f60`) construction + the
>   reserved-count/`ac64` handling at map-load** — the full 256-entry display palette with system colours in
>   the reserved 0..79 band (white@63) that `bc9c`/`ac70` consult at build time. The tile (and the terrain
>   colours) cannot be byte-correct until `4f60` at build time is correct. `bc9c` (patch 289, the dropped
>   `ac70` return) is a correct **prerequisite**, not the producer.
> - **Patch 289 stands** (asm-verified bug fix; tile empty→175 distinct real blended indices; mission-path
>   only; 19/19 verify PASS, mainmenu `3a6ff1c5`, native↔wasm bit-identical). It does NOT achieve byte-match
>   and no terrain flow is accepted. The **mission-TCB camera repoint (deliverable 2) was NOT pursued** —
>   without a byte-correct tile it cannot produce coherent terrain, so it is deferred behind the 4f60 gate.
>
> The rest of this doc (below) is the prior recon; treat its "symmetric palette-blend" claim as the
> **partial/incorrect** premise this correction supersedes.

---

**Date:** 2026-07-13 · **Method:** CR3-aware flat-address watch + fixed-phys tile-writer trace **with
20-word near-call backtraces** on a live AZER1 mission of the ORIGINAL, driven BATTLES→OK→ACCEPT (armed
before ACCEPT = map-load), plus offline disassembly of the extender image (`re_out/fist_image.bin`) and
content analysis of the RAM dumps. **Scope:** recon + oracle tooling ONLY; **no engine/ext/patch change**
(pristine `re_out/fist.c`=`61453e42`, `fist_ext.c`=`0051cb56`, `fist_mga.c`=`75c6d726` — unchanged; verify
19/19 unaffected by construction).

This **ENDS the tile-3918 flip-flop** and **CORRECTS the prior premises** (`docs/oracle_tile_writer_traced.md`
"bc06 upsample of the colormap", `docs/oracle_map_layout.md` "6980 reads 85bc+0x100000"). The paging-move
confound the last ~8 iterations chased **does not exist for this buffer** — proven below.

---

## HEADLINE — tile 3918 is a 256×256 pairwise PALETTE-BLEND LUT built by extender `bc9c` at map-load; its physical location is STABLE (a data-segment base, not paging)

### (0) The paging confound is DISPROVEN — the tile is at a FIXED physical address
The instrumented DOSBox was extended with a **CR3-aware flat watch** (`fist_flat_to_phys`, a 2-level guest
page-table walk re-run on every CR3/paging-enable change) and the **fixed-phys tile-writer trace was given
backtraces**. Result (`tools/oracle/samples/flat3918_cr3walk.txt`, `docs` reproduce below):

- The extender's DGROUP lives at **guest-phys `0x131000`** (every documented scalar reads back byte-exact
  there: `3918`=`0x00044200`, `85b8`=`0x00474e60`, `85bc`=`0x00074e60`, `bc90`=`0x10000`, `bc94`=`0x20000`,
  `8494`=`0x800`/2048, `8490`=`0xb`/detail-11, `8498`=`0x400000`).
- The extender's "flat pointer" `0x44200` (`DAT_0000_3918`) is a **data-segment-relative offset**: the
  extender data segment base is **`0x131000`**, so ext-ptr `0x44200` ⇒ **CPU linear = physical = `0x175200`**
  (`0x131000 + 0x44200`). This is a **segment base, NOT paging** — the CR3 walk finds the region **identity
  mapped with paging OFF at dump** (`flatpg 0x175xxx -> physpg 0x175xxx`).
- **The 212-distinct tile content sits at phys `0x175200` at dump AND is the target the map-load bc-cluster
  writes** — the same physical address, stable across map-load and dump (`ref[:4096]` of
  `scratch/oracle/oracle_tile3918.bin` matches phys `0x175200` exactly; distinct=212, nonzero=65536).
- ⇒ The prior "physical frame moves under paging" premise is **wrong for this buffer**. The fixed-phys watch
  at `0x175200` was correct all along; it just lacked a backtrace and a stability proof, both now supplied.
- **Gotcha the CR3 tool exposed:** watching the raw ext-ptr `0x44200` as a CPU linear address catches a
  DIFFERENT, identity-mapped buffer (engine/driver scratch), not the tile. The correct CPU linear address to
  watch is `0x175200` (= ext-ptr + data-seg-base `0x131000`). This is why prior flat-vs-phys traces
  disagreed — they were watching two different address spaces.

### (1) The producer — `bc9c` (dominant) + `bc06` (upsample), cs=`0x2b`, at map-load
The 6 tile-range writers (`tools/oracle/samples/tile3918_writers_backtrace.txt`), all extender 32-bit-PM
(`cs=0x2b`), disassembled from `re_out/fist_image.bin`:

| ext IP | count | tile off | role |
|---|---|---|---|
| `bcf2`/`bcf6` | 29403 / 32791 | `[0e00..ffff]` | **`FUN bc9c`** — the pairwise palette-blend LUT fill (dominant) |
| `bc2a`/`bc30` | 5376 each | `[0000..0dff]` | **`FUN bc06`** — bilinear averaging LOD-upsample, axis 1 |
| `bc74`/`bc7a` | 5024 / 5728 | `[0000..0dff]` | `FUN bc06` — bilinear averaging LOD-upsample, axis 2 |

**`FUN bc9c` (the tile filler) — the exact algorithm** (disasm `bc9c..bd0d`):
```
ecx = ds:0xbc90                     ; dest buffer base; = tile 0x44200 during the build (bc90 is REUSED)
loop cl = 0..255 (outer), ch = 0..255 (inner):     ; ecx' low 16 bits = (ch<<8)|cl -> sweeps 0..0xffff
    bx  = word[cl*3 + 0x5598] ; dh = byte[cl*3 + 0x559a]     ; palette[cl] RGB
    ax  = word[ch*3 + 0x5598] ; dl = byte[ch*3 + 0x559a]     ; palette[ch] RGB
    ac68 = (R_cl + R_ch)/2 ; ac69 = (G_cl+G_ch)/2 ; ac6a = (B_cl+B_ch)/2   ; 50% blend (stc;adc;rcr)
    call 0xac70                     ; ac70 = nearest-palette-index reverse-lookup (dist LUTs @a060/a460/a860,
                                    ;        palette copy @0x4f60) -> returns index in bl
    byte[ecx] = bl                  ; tile[cl + 256*ch] = blended index
```
So **`tile[cl + 256*ch] = nearest_palette_index( (palette[cl] + palette[ch]) / 2 )`** — a **256×256 pairwise
50%-blend / translucency LUT** over the 256-entry **6-bit VGA mission palette at `ds:0x5598`** (verified:
values 0..63, entry 0 = white `(63,63,63)`). This is what the voxel renderer uses to blend/dither two
colormap indices. It explains the content analysis exactly: 212 distinct, a smooth 2-D gradient, NOT a
downsample/crop of the colormap (0.7 %/0.8 % match) — because it is a LUT, not a texture.

**`FUN bc06`** (disasm `bc06..bc8c`) doubles a square source plane (dim from `ds:0x5578`) via bilinear
averaging (`mov al,[esi]; mov [edi],al; add ah,al; rcr ah,1; mov [edi+1],ah`) on both axes; it fills the
first `0xe00` bytes of the tile region (a smaller LOD block), while `bc9c` fills the rest.

`bc9c` also calls `bd0e`/`bd62` which build **depth-shade LUTs** into `ds:0xbc94` (a 32×256 palette×brightness
table) — a *different* buffer, not the tile.

### (2) SOURCE PLANES — corrected
- **The tile's source is the MISSION PALETTE `ds:0x5598`** (256 × 6-bit RGB), blended pairwise. **NOT** the
  heightmap, **NOT** the colormap texture `85b8` directly, **NOT** `85bc+0x100000`.
- The colormap `85b8` and heightmap `85bc` feed the **renderer** (`9200` samples them per column); the tile
  is a **secondary LUT** the renderer indexes with *pairs of colormap indices* to blend adjacent terrain
  texels. The port's `6980`-reads-`85bc+0x100000` path is the wrong producer (retire it, as prior docs said);
  the port's "upsample colormap into the tile" idea is also wrong — the tile is a palette blend, not a
  texture upsample.

### (3) WHEN it runs — map-load, via a TCB-inbox op
Backtrace + disasm of the caller chain:
- `bc9c` is invoked through the extender's **op-service far-call gate** (return `bf04`, a `call FWORD` thunk).
- The gate is the **`0x0f30` dispatcher**: `ebx = ds:0xc93 (current TCB); ebx = [TCB+0x3f2] (command inbox);
  call [table + op]` — i.e. the tile build is a **TCB-inbox op** (`+0x3f2`), exactly the op mechanism in
  `CLAUDE.md`.
- The whole thing runs **inside the map-load** `call 0x89b0` (ext `0x10d4`, return `0x10d9` in the backtrace)
  — the same `89b0` the port already runs (but where the port leaves the tile EMPTY, distinct=1).

---

## THE MISSION-TCB CAMERA SOURCE (deliverable 2)

The op-0x09 terrain renderer `FUN_0000_9200` (the `9260`/`9275` `stosb` writers, per
`docs/oracle_activate.md`) reads its projection from the render-param block **`ds:0x90a8..0x90f8`** (ray-start
`0x90d4`/`0x90d8`, du/dv `0x90b8`/`0x90bc`, horizon `0x90f0`, columns `0x90f8`=81, fb `0x90a8`/`0x90ac`; the
column sampler base is SMC-patched to tile `0x3918` at `92a1`: `mov eax,ds:0x3918; mov ds:0x934c/0x9372,eax`).

**That block is populated per frame by the projection setup at `0x85d0`, which reads the camera DIRECTLY from
the current mission TCB** (`mov ebx,ds:0xc93`):

| TCB field | transform | render var | meaning |
|---|---|---|---|
| `[TCB+0x2c]` dword | `<< 13` | `0x90d4` (`0x90cc` pre-shift) | camera **X** (world) |
| `[TCB+0x30]` dword | `<< 13`, neg | `0x90d8` (`0x90d0`) | camera **Y** |
| `[TCB+0x34]` dword | clamp `0x7f00`, `<< 17` | `0x90dc` | camera **Z / height** |
| `[TCB+0x38]` word | `<< 16`, neg | `0x90e0` | **pitch / horizon** |
| `[TCB+0x3e]` word | `0xffffffff / v` | `0x90c0` (`0x90c8`=raw) | **focal / FOV** (raw `0x100`) |
| `[TCB+0xcf]` byte | → `ds:0x395d` | (SMC) | detail/shade |
| `[TCB+0xd0]` byte | → `ds:0x9356/0x937a` (at `92d8`) | (SMC) | shade |

### The mission TCB is at guest-phys `0x10000` — DISTINCT from the intro TCB (`0x90000`)
Located by matching all five camera fields to the live render params across **two independent settled frames**
(`activate.ram.bin`, `act4d0e.ram.bin`); it is a valid TCB (`+0x3f2`=op `4`, `+0x490/492/494` params
`0x220/7/1`, `+0xba` name field). It is **not** the intro TCB at `0x90000` (whose `+0x2c`/`+0x30` differ) and
**not** engine `4e1c` (zero in both original and port, per `docs/oracle_activate.md`).

**Live camera values** (raw TCB fields = the `0x90cc/0x90d0/0x90dc/0x90e0/0x90c8` pre-shift dumps):

| field | frame A (activate) | frame B (act4d0e) |
|---|---|---|
| X `[+0x2c]` | `0x0009248a` | `0x00092f40` |
| Y `[+0x30]` | `0x00110603` | `0x00110740` |
| Z `[+0x34]` | `0x6900` (→`0x90dc=0xd2000000`) | `0x6d00` (→`0xda000000`) |
| pitch `[+0x38]` | `0x933d` (→`0x90e0=0x6cc30000`) | `0x9412` (→`0x6bee0000`) |
| focal `[+0x3e]` | `0x0100` | `0x0100` |

X/Y/Z/pitch **vary** between frames (live camera), focal is constant — confirming this is the live camera,
not a stale mirror. **HONEST caveat:** the current-TCB pointer slot `ds:0xc93` reads a stale value
(`0x4c606700`) at dump (the render already completed and the extender moved on), and the exact
segment-base arithmetic that maps `ds:0xc93` → phys `0x10000` at render time is not pinned; but the TCB
**struct** (phys `0x10000`), the **field offsets**, and the **live values** are pinned decisively by the
5-field cross-frame match + the `0x85d0` disassembly.

---

## The exact port-side fix for the NEXT iteration (a faithful OBSERVED reconstruction)
1. **Build the tile at map-load** = drive extender `bc9c` (`m_ext_FUN_0000bc9c` / the op that dispatches it
   through `0x0f30`/`89b0`): fill `tile[cl + 256*ch] = ac70_nearest_index( (pal[cl]+pal[ch])/2 )` from the
   **256-entry 6-bit palette at ext `0x5598`** (with `bc06` doing the leading LOD block). Point the dest
   (`ds:0xbc90`) at the tile `0x44200` during the build. **Source = the palette, not the height/colormap.**
   Prereq: the mission palette at `0x5598` must be loaded (the `.PAL`), and the `ac70` distance LUTs
   (`a060/a460/a860`) + palette copy (`0x4f60`) initialized.
2. **Feed `9200` the mission camera** from the TCB (fields `+0x2c/0x30/0x34/0x38/0x3e`), transformed by the
   `0x85d0` setup into `ds:0x90d4/0x90d8/0x90dc/0x90e0/0x90c0`. The `FIST_R3D2` seam currently uses the
   degenerate intro TCB (`0x90000`) — repoint it at the **mission TCB** and apply the shifts above.
3. Then `9200` (op 0x09) samples the filled tile through the real projection → coherent terrain.
   Order: (1) fills the tile; (2) supplies the camera; both at `4d0e=0` (per `docs/oracle_activate.md`).

## Tooling added / changed (reproducible)
- `tools/oracle/dosbox_vga_terrain_trace.patch` — adds (a) `fist_flat_to_phys` CR3/page-table walk +
  `FIST_WATCHFLAT`/`FIST_WATCHFLATSPAN` CR3-aware flat watch → `<prefix>.flatwriters.txt` (histogram + per-IP
  20-word backtrace + the final flat→phys page map); (b) a **20-word backtrace** on the fixed-phys tile
  recorder → `<prefix>.tilewriters.txt`.
- `tools/oracle/trace_flat3918.sh` — drives BATTLES→OK→ACCEPT, arms before ACCEPT, watches CPU-linear
  `0x175200` (= ext-ptr `0x44200` + data-seg base `0x131000`).
- Samples: `tools/oracle/samples/tile3918_writers_backtrace.txt`, `.../flat3918_cr3walk.txt`.

## Reproduce
```bash
# rebuild instrumented dosbox (README_terrain.md build steps) then:
DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/trace_tilebuild.sh 160 100 205 128 40 186 60 12 12 20 5 8 \
   scratch/oracle/tb2                       # -> tb2.tilewriters.txt (bc-cluster + backtraces), tb2.ram.bin
DOSBOX=/tmp/debs/dosbox-fist FIST_WATCHFLAT=0x175200 FIST_WATCHFLATSPAN=0x10000 \
   bash tools/oracle/trace_flat3918.sh ... scratch/oracle/flatL175   # CR3 map == identity (stability proof)
# offline: disassemble the producer + camera setup from the committed extender image
objdump -b binary -m i386 -M intel -D --start-address=0xbc9c --stop-address=0xbd0e re_out/fist_image.bin
objdump -b binary -m i386 -M intel -D --start-address=0x85d0 --stop-address=0x8642 re_out/fist_image.bin
```
