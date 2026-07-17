# Oracle RUNTIME 9200 renderer capture — the port HAS the right renderer code (definitive)

> ## VERDICT (2026-07-17) — **the oracle's RUNTIME code at flat `0x10009260` is BYTE-IDENTICAL to the
> static `re_out/fist_image.bin` @ `0x9200`. There is NO paged-out perspective renderer. The 12-iteration
> "perspective renderer is absent from every image / not asm-derivable" premise is OVERTURNED: the
> executed terrain renderer IS the affine (Mode-7) `FUN_0000_9200` the port already decompiled. The
> mission-windshield divergence is therefore an INPUT-STATE (camera → projection globals) problem, NOT a
> missing/wrong renderer.**
>
> Engine PRISTINE unchanged (`61453e42` / `0051cb56` / `75c6d726`) — this is a read-only capture + analysis,
> no code fix. Reproduce: `DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/trace_terrain.sh` → the runtime
> renderer bytes land at guest-phys `0x131000 + 0x9200`. Committed capture:
> `tools/oracle/samples/oracle_runtime_9200_renderer.bin` (3840 B = linear `0x10009100..0x1000a000`).

## The question this settles
For 12 iterations the mission terrain colour was blocked on a hypothesis: the port's `FUN_0000_9200`
(from `fist_image.bin` @ 0x9200) is an asm-exact **constant-scale affine** walk, but the oracle's
windshield terrain looked **perspective** (depth-luminance gradient + texture-magnification signature).
The prior verdict concluded the *real* perspective renderer must be installed into the running extender's
paged 32-bit PM space by relocation/SMC at load — i.e. **absent from the extracted static image, not
asm-derivable**. This capture tests that head-on: **is the oracle's runtime code at flat `0x10009260`
the SAME as the static `fist_image.bin` @ 0x9200, or DIFFERENT?**

## Method — extract the runtime code from guest-physical RAM
- The instrumented DOSBox (`tools/oracle/dosbox_vga_terrain_trace.patch`) confirms the dominant VGA-`0xA0000`
  writer in a live AZER1 mission: **`lin=0x10009260 cs=002b eip=0x9260 count=3,462,228`** and its 2×-unrolled
  sibling `eip=0x9275` (same count). `cs=0x2b` = the extender's flat 32-bit PM code selector, base
  `0x10000000` (linear IP = base + eip). These are the two `stos` writers of the terrain texel loop.
- The SIGUSR2 dump's `code@lin-16` bytes are **unusable** for this: the dump fires with `cpu.pmode=0
  vga.mode=3` (the guest is momentarily in real/text mode at dump time), so `mem_readb(0x10009260)`
  translates through the then-current (paging-off) mapping and reads **all zeros** (the committed
  `terrain_writers.txt` shows exactly this).
- **But the extender image is resident in guest-physical RAM.** The `.ram.bin` (16 MB `MemBase`) contains
  the decrypted extender loaded verbatim. `re_out/fist_image.bin` is the **decrypted, base-0** extender
  image (`app-off == flat linear`; the loader maps flat `0x10000000` → guest-phys `0x131000`). Searching
  the RAM dump for the distinctive 22-byte `0x9200` prologue
  (`8b1dd49000008b15d89000008b3da890000033c9…`) locates the runtime copy:

  | phys hit | implied load base | note |
  |---|---|---|
  | **`0x13a200`** | **`0x131000`** | the documented extender base — the live renderer |
  | `0x13a130` | `0x130f30` | partial 22-byte prologue alias, diverges after (65 B diff over the fn) — NOT the renderer |

  So the runtime `0x9200` renderer is at phys `0x131000 + 0x9200 = 0x13a200`.

## Result — SAME code (byte-identical except the documented SMC data patch)
Diff of `fist_image.bin[0x9100..0xa000]` vs guest-phys `[0x131000+0x9100 .. +0xa000]` = **50 differing
bytes out of 3840**, entirely accounted for and NON-algorithmic:

| kind | bytes | detail |
|---|---|---|
| **SMC colormap-base displacement** | 24 (6 dwords) | `0x7fffffff` → `0x00044200` at `0x918c/0x91a9/0x925c/0x9271/0x934c/0x9372` — the documented self-modify that patches `DAT_0000_3918` (colormap base) into the six `mov al,[eax+disp]` samplers of the three LOD variants. A **data pointer**, not an opcode change. |
| **live projection globals** | 20 | `0x9100..0x9117` (the `0x910c` skip + `0x9114` horizon-table pointer + neighbours) — runtime-populated camera/projection state; static BSS = 0. Not code. |
| other | 6 | `0x9429/0x9434` (`0a`→`0b`, adjacent setup-fn data) and `0x9eae..0x9eb2` (data past the renderer). Outside the executed render loop. |

**The executed terrain renderer (`0x9200`, opaque LOD — the one whose `stos` at `0x9260`/`0x9275` is the
3.46M-write dominant terrain writer) is byte-for-byte the static affine.** Runtime disassembly at phys
`0x13a200` is identical to the static, differing only at the resolved sampler displacement:

```
9200: mov ebx,[0x90d4]   9206: mov edx,[0x90d8]   920c: mov edi,[0x90a8]   9212: xor ecx,ecx
9220: (per column ecx=0..[0x90f8]) push ecx/ebx/edx
9223: mov eax,[0x9114]; movzx eax,[ecx+eax]     ; per-column horizon skip
9231: or eax,eax; je 923f; {edi+=eax; ecx=eax; edx+=esi; ebx+=ebp} loop   ; skip sky
923f: ecx=[0x90f0]>>1; ecx-=eax                 ; textured span
9250: (per texel, 2x unrolled)
9252:   xor eax,eax; shld eax,edx,8; shld eax,ebx,8
925a:   mov al,[eax+0x44200]   ; <- RUNTIME (static: [eax+0x7fffffff]); colormap sample
9260:   stos es:[edi],al       ; <<< THE DOMINANT TERRAIN WRITER (eip 0x9260)
9261:   edx+=esi; ebx+=ebp      ; CONSTANT per-texel step  (esi/ebp fixed all frame)
   … 2nd unrolled copy 9267..9275 …
927a: loop 0x9250
927e: edi+=[0x90ac]; edi+=[0x910c]; ebx-=[0x90bc]; edx+=[0x90b8]   ; next column base step
929e: jne 0x9220
92a0: ret
```

- The blended LOD `0x9130` and LOD3 `0x92c0` are the **same affine family** (constant `esi/ebp`; `0x9130`
  only inserts `cmp al,0xb4; adc al,[edi]; rcr al,1` averaging before `stos`). The writer trace shows
  **only** `0x9260/0x9275` fired → the oracle used the `0x9200` opaque LOD, exactly what the port drives.
- **No per-texel divide anywhere in the render loop.** A full `objdump` scan of the whole `0x9100..0xa000`
  cluster finds exactly two divides — `0x9bf7 fdivr` and `0x9ff6 div bl` — both in the **projection/setup**
  code (the `0x8120`-family camera math that computes the affine parameters ONCE per frame), never inside
  the `0x9250`/`0x9180` texel loops. The perspective divide the prior verdict expected in the renderer is
  **not there** because the render is affine by construction.

## What this means for the port (integration path — NO new renderer needed)
The port already has the correct renderer (`m_ext_FUN_0000_9200` in `re_out/fist_ext.c`, the `FIST_R3D2`
seam). The divergence is entirely in the **projection input state** the renderer reads:
`esi`/`ebp` (per-texel u/v step), `0x90b8`/`0x90bc` (per-column base step), `0x90d4`/`0x90d8` (u/v init),
`0x90a8`/`0x90ac`/`0x910c` (dest + advances), `0x90f0`/`0x90f8` (colheight/width), `0x9114` (horizon
table). These are produced by **`FUN_0000_8120` from the extender TCB camera** (position / heading /
altitude / pitch / roll), and by the horizon/occlusion table builder. Live values captured this run
(dump-time, so possibly a settled frame): `90f8=77 90f0=292 90ac=28 90b8=0xfe783bff 90bc=0x0079fffe
90d4=0x215ae000 90d8=0xd563e000 9114=0x000075b9 3918=0x00044200`.

**The faithful fix is to drive `FUN_0000_82b8`'s `8120` projection setup with the real mission TCB camera
(so `esi/ebp/90b8/90bc/90d4/90d8/9114` match the oracle), not to reconstruct a nonexistent perspective
renderer.** The "no affine reproduces the oracle" result of the prior iteration reflects the port feeding
the affine the WRONG projection parameters (the degenerate intro-TCB-derived state), not a code mismatch.

## Reconciling the "perspective signature"
The prior optimiser-independent evidence (depth-luminance gradient; bottom/top texture-activity ratio
0.51 vs the port's 0.92) was read as "impossible for an affine." Given that the oracle demonstrably
renders with THIS affine code, that reading was over-strong: the oracle's affine — with its real camera
parameters, its horizon-skip per column, and a colormap tile that has a real vertical brightness gradient
— produces those metrics. The next question (why the oracle's affine parameters yield that image while the
port's don't) is a **projection-parameter / camera-state** question, cleanly separated from the now-settled
renderer-code question.

## Reproduce
```bash
make kernel-image                                   # re_out/fist_image.bin (decrypted, base-0)
DOSBOX=/tmp/debs/dosbox-fist bash tools/oracle/trace_terrain.sh   # -> scratch/oracle/terrain.ram.bin (16MB)
# runtime renderer = guest-phys 0x131000 + 0x9200 ; diff vs fist_image.bin[0x9200:]
python3 - <<'PY'
img=open('re_out/fist_image.bin','rb').read(); ram=open('scratch/oracle/terrain.ram.bin','rb').read()
B=0x131000; diff=[o for o in range(0x9100,0xa000) if img[o]!=ram[B+o]]
print(len(diff),"byte diffs (all SMC-displacement + live-data, none algorithmic)")
PY
```
Committed capture: `tools/oracle/samples/oracle_runtime_9200_renderer.bin`.
