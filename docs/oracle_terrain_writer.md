# Oracle recon — the cockpit TERRAIN writer, resolved from the ORIGINAL

**Date:** 2026-07-12 · **Method:** instrumented DOSBox VGA-write trace of a live AZER1 mission ·
**Scope:** read-only recon, NO port/patch changes (pristine `re_out/fist.c` = `61453e42`,
`fist_mga.c` = `75c6d726`, `fist_ext.c` = `0051cb56` — unchanged).

## HEADLINE — the terrain is drawn by the EXTENDER, not the engine

**The cockpit terrain-viewport pixels are written by Doug-Huffman-extender 32-bit PM code at
`fist_image.bin` offset `0x9200` = our decompile symbol `FUN_0000_9200` (`re_out/fist_ext.c:13718`),
dispatched by `FUN_0000_82b8` (`fist_ext.c:12994`).** This RESOLVES the render-location flip-flop:
the engine-side `2322` render pass draws only the cockpit/HUD (which is why the port's `FIST_ENGFB`
saw a near-uniform clear — the terrain was never rendered because the *extender* renderer is the
terrain producer, and it is not driven in the port). The prior "extender `93c0` is a flat map-view
blitter / `9482` is dead" observation was correct **but about the wrong functions** — the terrain
renderer is `9200`/`9130`/`92c0`, a different cluster in the same image, reached by an **indirect
method-vector** (detail-level selected), which is why direct-call searches for it kept missing.

### How it was proven (decisive, not inferred)

Instrumented DOSBox (`core=normal` so `reg_eip` is exact) logs a per-instruction-pointer histogram
of every write to VGA `0xA0000` over a ~5 s window on a **settled AZER1 terrain frame** (screenshot
`scratch/oracle/terrain.frame.png` = cockpit + sky/desert + instruments, a genuine mission frame).
Evidence table (`tools/oracle/samples/terrain_writers.txt`), ranked by write count:

| rank | linear IP | cs | image offset | writes | fb off range | what |
|---|---|---|---|---|---|---|
| **1** | `0x10009260` | `0x2b` (32-bit flat) | **extender +0x9260** | **2,641,608** | `[0656..6b65]` | terrain texel `stos` (unrolled #1) |
| **2** | `0x10009275` | `0x2b` | **extender +0x9275** | **2,641,608** | `[0657..6b66]` | terrain texel `stos` (unrolled #2) |
| 3 | `0x00051052` | `0x4ec3` (engine) | engine blitter | 159,541 | `[931a..e8c8]` | cockpit panel `rep movs` |
| 4… | `0x0004fxxx`/`0x0005xxxx` | `0x4ec3` | engine | ≤46k each | `8303+ / be65+ / e767+` | HUD / instruments / gauges (RLE + fills) |

- `cs=0x2b` + linear base `0x10000000` = the extender's flat PM segment (documented base;
  under DOSBox it is loaded at guest-physical `0x131000` — verified by signature match in the RAM dump).
- Writer #1/#2 alone = **~46 % of all framebuffer writes**, 16× the busiest engine writer, and their
  framebuffer offset range `[0656..6b65]` maps to the **3D-view rows (~5–137)** — i.e. the terrain
  window. The engine (`cs=0x4ec3`) writers all land in the cockpit/HUD offsets (`0x8303+`, `0xbe65+`,
  `0xe767+` = rows 130–200). This spatial split is the frame you see in `terrain.frame.png`.

## The writer's structure — a per-column texture-mapped terrain renderer (voxel ground)

Disassembly of `fist_image.bin` (32-bit) at the two sibling LOD entries:

```
; FUN_0000_9200  (opaque LOD — the one that fired)          ; FUN @0x9130 = blended/translucent LOD
9200: ebx=[0x90d4] edx=[0x90d8] edi=[0x90a8]  ecx=0         ; identical setup
9220: (per COLUMN, ecx=0..[0x90f8])                         ; [0x90f8]=view width = 81 columns
9223:   eax=[0x9114]; movzx eax,[ecx+eax]  ; per-column SKY-SKIP / horizon (voxel y-buffer)
9231:   or eax,eax; je …; edi+=eax; ecx=eax; {edx+=esi;ebx+=ebp} loop   ; advance over sky
923f:   ecx=[0x90f0]>>1 ; ecx-=eax          ; textured span = (colheight/2 - skip)
9250:   (per TEXEL, ecx times, 2× unrolled):
9252:     xor eax,eax; shld eax,edx,8; shld eax,ebx,8   ; eax = texcoord from fixed-pt u(edx)/v(ebx)
925a:     mov al,[eax+0x7fffffff]          ; SAMPLE colormap  (0x7fffffff = self-modified base)
9260:     stos byte es:[edi]               ; <<< THE DOMINANT WRITER
9261:     edx+=esi ; ebx+=ebp              ; step u,v per pixel
   …      (2nd unrolled copy at 926f/9275)
927a:   loop 0x9250
927e:   edi+=[0x90ac]; edi+=[0x910c]; ebx-=[0x90bc]; edx+=[0x90b8]   ; next column
929e:   loop next column
```

- The blended sibling (`0x9130`, entry via `0x8460`) inserts `cmp al,0xb4; ja +; adc al,[edi]; rcr al,1`
  before `stos` — index-≤0xb4 texels are **averaged with the existing pixel** (translucency/smoothing).
- `0x92c0` (entry via `0x8470`) is a third LOD variant. All three are the classic **texture-mapped
  ground / voxel** signature: per-column horizon occlusion + affine `(u,v)` texel walk with
  perspective step.

### Self-modifying colormap base (data-flow smoking gun)
The stubs `0x91e5` / `0x9119` / `0x92a1` do `mov eax,[0x3918]; mov [sampler0_disp],eax;
mov [sampler1_disp],eax` — they **patch the colormap base `DAT_0000_3918` into the `mov al,[eax+disp]`
sampler displacements** at run time (opaque: `0x925c`/`0x9271`; blended: `0x918c`/`0x91a9`).
Captured live: **all four patched displacements = `0x00044200` = `DAT_0000_3918`.**

## Data flow (live values captured from the settled frame — extender data region)

Extender image at guest-phys `0x131000`; all `0x90xx`/`0x91xx`/`0x3918` are extender-flat offsets.

| symbol | offset | live value | meaning |
|---|---|---|---|
| `DAT_0000_3918` | `+0x3918` | `0x00044200` | **colormap/texture base** (flat linear; low DOS mem, engine-loaded) |
| `DAT_0000_9114` | `+0x9114` | `0x00007568` | **per-column horizon/skip table** base (flat linear) |
| `DAT_0000_90f8` | `+0x90f8` | `81` (`0x51`) | **view width** = number of columns |
| `DAT_0000_90f0` | `+0x90f0` | `288` (`0x120`) | column height → `>>1` = **144** textured rows/col |
| `DAT_0000_90a8` | `+0x90a8` | `0xf00a0650` | dest start (low16 `0x0650` = first fb offset) |
| `DAT_0000_90ac` | `+0x90ac` | `32` | per-column dest advance |
| `DAT_0000_90d8` | `+0x90d8` | `0x385fa2a0` | **u** accumulator init (fixed-point) |
| `DAT_0000_90d4` | `+0x90d4` | `0x9ee19cb8` | **v** accumulator init (fixed-point) |
| `DAT_0000_90b8` | `+0x90b8` | `0xfffcdbc4` (-205884) | per-column u step |
| `DAT_0000_90bc` | `+0x90bc` | `0x00fffb10` | per-column v step |
| (register) `esi` | — | per-texel u step | set by caller / `FUN_0000_8120` |
| (register) `ebp` | — | per-texel v step | set by caller / `FUN_0000_8120` |

**Projection / camera setup** = `FUN_0000_82b8`'s prologue math at `0x8282` (centers u/v around the
view centre): `u -= (colheight/2)*esi; v += (width/2)*vstep; u -= (width/2)*ustep`. The per-texel
steps `esi`/`ebp` and the initial `u/v` and steps are produced upstream by **`FUN_0000_8120`** (the
setup call shared by all three LOD entries) from the tank camera (position / heading / altitude).
That is the camera→projection chain the port must drive.

### First-column ground truth
- Framebuffer dest base low16 = `0x0650`; the writer's first written offset = `0x0656` ⇒ **column-0
  sky-skip = 6**; textured span = `144 − 6 = 138` texels; **first terrain pixel index = `0xdc`**
  (writer #1 `firstval`).
- Column count 81, per-column advance `0x90ac`=32; total written offset span `[0x0656..0x6b66]`.

### HONEST caveat on the sampled DATA contents
The colormap (`0x44200`) and horizon table (`0x7568`) are **flat-linear** addresses in the extender's
paged space. The RAM dump (`pmemsave`-equivalent, guest-physical) was taken without a CR3/page-table
capture, so reading those offsets as identity guest-physical is **not guaranteed**: `0x44200`
identity-reads as plausible image data (209 distinct/4 KB), but `0x7568` identity-reads as 16-bit
code — so the low-mem paging is **not** a clean identity map for the table region, and the raw
table/colormap *bytes* here are **inconclusive**. The pointer VALUES above are solid (read from the
located extender image); their contents need a follow-up capture that also snapshots CR3 and walks
the extender page tables (or read them via the extender FILEMGR load path in `fist_ext.c`). This does
not affect the headline: the writer identity, its algorithm, and its input-state addresses are proven.

## The engine (`cs=0x4ec3`) writers — cockpit/HUD only (for completeness)
~18 distinct engine IPs, all cockpit furniture: RLE sprite decoders (`ac … aa … e2` lodsb/stosb),
`rep stosb` fills, `rep movs` panel blits, at fb offsets `0x8303+/0xbe65+/0xe767+` (rows 130–200).
None touch the terrain-view rows. Their code bytes are captured in
`tools/oracle/samples/terrain_writers.txt` for offline matching to `fist_dat_image.bin` if needed.

## Next reconstruction target (for the porting iteration)
1. **Drive `FUN_0000_82b8` in `fist_ext.c`** (the opaque LOD wrapper: `FUN_0000_8120` setup →
   `FUN_0000_9200` render → `FUN_0000_82d0` teardown). It is reached via an **indirect method-vector**
   (extender dispatch table, `fist_ext.c:17347+`), detail-level selected — find the selector that
   installs `0x82b8` vs the blended `0x8460` / `0x8470`.
2. **Populate its input state**: colormap base `DAT_0000_3918`, horizon table `DAT_0000_9114`,
   `0x90a8/0x90ac/0x90d4/0x90d8/0x90b8/0x90bc/0x90f0/0x90f8`, and the register steps `esi`/`ebp` —
   all produced by `FUN_0000_8120` from the camera. `FUN_0000_9200` itself is already decompiled and
   looks faithful (`fist_ext.c:13718`).
3. **Verify the self-modify** patches `DAT_0000_3918` into the samplers (or, in the port's
   host-pointer model, that the sampler dereferences the colormap correctly).

## Reproduce
```bash
# builds a LOCAL instrumented DOSBox (no sudo) — see tools/oracle/README_terrain.md
DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/trace_terrain.sh
# -> scratch/oracle/terrain.{writers.txt,ram.bin,vram.bin,pal.bin,frame.png}
```
