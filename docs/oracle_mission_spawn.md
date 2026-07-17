# Oracle frame-matched AZER1 mission SPAWN capture — the port's black terrain is a RENDER bug, not a flight-model gap

> ## 9200 V-SPAN — FINAL VERDICT (2026-07-17, 12th/last colour iteration) — **THE MECHANISM IS *NOT* THE
> HORIZON TABLE (9114) NOR THE COUNT (90f0); IT IS THAT THE RECONSTRUCTED 9200 IS A CONSTANT-SCALE AFFINE
> WHILE THE ORACLE'S WINDSHIELD TERRAIN IS A PERSPECTIVE RENDER. The perspective renderer is NOT in any
> available image (paged-out Doug-Huffman extender PM) → NOT asm-derivable → definitive pin, no patch.**
> Repro: `python3 tools/oracle/vspan_derive.py` (uses the committed frame-matched samples + two default-build
> port dumps `FIST_BBDUMP`/`FIST_MISSFB_FBIDX`). Engine PRISTINE unchanged (`61453e42`/`0051cb56`/`75c6d726`).
>
> **PORT MACHINERY VALIDATED (the first time the port's terrain is reproduced exactly, offline).** `FUN_0000_9200`
> is an asm-exact CONSTANT-SCALE affine (Mode-7) walk called ONCE for the whole window (`82b8 = {8120();
> 9200(p1,p2); 82d0()}` passes a single `param_1/param_2 = ebp/esi` straight through; 9200's outer loop sweeps
> all 81 scanlines with a per-scanline BASE step `90b8/90bc` but a CONSTANT inner step `esi/ebp`). Its clean
> per-pixel map is `V(r,c)=90d8 + r·90b8 + c·esi`, `U(r,c)=90d4 − r·90bc + c·ebp`, `idx=tile[(V>>24)&255,(U>>24)&255]`.
> Simulating that map with the **default-build globals** (`90d8=0x2dc548e0 90d4=0x7b463980 90b8=0x0088f59a
> 90bc=0x00d84852`, `esi=(90c0=0x01000000·9104=0x6c24295e)>>32=7087145`, `ebp=(·9108=0x447acd50)>>32=4487885`,
> `90f0=288 90f8=81 9114=ext:0x7568`) reproduces the port's rendered terrain window (288×81 @ screen (16,5))
> at **99.3–99.8 %**. So the port's affine + the shim-guessed `esi/ebp` + 8120's `90b8/90bc/90d8/90d4` = EXACTLY
> what the port draws.
>
> **TASK OPTIONS (a) HORIZON / (b) COUNT ARE RULED OUT.** The window is genuinely 288×81 (the 99.3 % alignment
> is at the full 288-wide window; `90f0=288`=viewport width, `90f8=81`=scanlines, both = `TCB+0x1e/+0x22`, set
> by 78e7). The horizon table `9114`→ext `0x7568` only skips the leftmost `horizon[r]` px per scanline — the
> simulation IGNORES horizon and still matches 99.8 %, so `9114` is negligible for the terrain band. Neither
> `90f0` nor `9114` is the mechanism.
>
> **THE V-SPAN CLAIM REFINED.** The prior pin ("port marches V to only row 126") is INACCURATE: the validated
> port simulation samples tile V-rows **45–209** (165 distinct) — it DOES reach the light rows. The real
> difference is the port's affine screen→tile map lands on DARKER texels (terrain mean idx 127) than the
> oracle's map (mean 186) in the same window.
>
> **NO AFFINE REPRODUCES THE ORACLE.** port-params 0.4 %; broad offset-only search 2.9 %; general 6-param
> hill-climb incl. rotation 4.5 % — while the same machinery self-reproduces the PORT at 99.8 %. CAVEAT: the
> texture-lookup match surface is pathological (a 1-tile-pixel error scrambles ~all indices); the optimiser
> recovers even *synthetic* known-affines only ~10–13 %, so this is necessary but not sufficient on its own.
>
> **DECISIVE OPTIMISER-INDEPENDENT EVIDENCE — the oracle is PERSPECTIVE, the port is FLAT AFFINE.** Per-screen
> -row (top→bottom) luminance: PORT `69 68 68 67 66 65 64 62 60 59 58` (flat) vs ORACLE `135 138 145 138 119
> 99 103 94 96 91 92` (a strong ~42-lum top→bottom depth gradient). Per-row horizontal texture activity
> (texel/pixel scale proxy): PORT bottom/top ratio **0.92** (constant scale = affine) vs ORACLE **0.51**
> (declines toward the viewer = perspective magnification). The oracle's depth-varying texture scale + depth
> shading is the classic voxel/perspective signature a constant-scale affine cannot produce.
>
> **MECHANISM (pinned).** The windshield terrain in the running game is a PERSPECTIVE render (per-texel depth
> divide → the tile step grows toward the viewer; + depth shading). The asm-exact affine `9200` (`edx+=esi`
> constant, one call) has no room for that — and no affine parametrisation of `9200`'s inputs reproduces it.
> The true perspective routine is absent from `re_out/fist_image.bin` and both `.DVR`s (the paged-out
> Doug-Huffman extender PM code, consistent with the "task+0x3f2 read by nothing in FIST.DAT" finding). It is
> therefore **NOT asm-derivable** — the honest close of the mission-terrain-colour investigation. Secondary,
> AE-neutral (do NOT band-aid): the render-camera TCB `+0x3a/+0x3c` pitch/roll are never written on the
> mission path (stale 0/−256), but fixing them only re-parametrises the affine and cannot add perspective.


> ## FRAME-MATCHED VERDICT (2026-07-17, the definitive re-capture) — **THE FRAME-SKEW IS ELIMINATED, THE
> ORACLE SPAWN IS NOW SELF-CONSISTENT, AND IT OVERTURNS THE 9200-VADDR PREMISE: the camera pitch/roll fix
> does NOT make the port's terrain indices match. Every 9200 input (camera → globals → depth-step → tile) is
> byte-exact yet the port samples the tile's DARK rows (mean 123) while the oracle samples the LIGHT rows
> (mean 185). The residual is a DEEPER STRUCTURAL difference in the 9200 texel-walk / its paged-out per-row
> depth-step, NOT the camera, NOT the V-base, NOT frame-skew, NOT the tile, NOT the palette.** Engine PRISTINE
> (61453e42/0051cb56/75c6d726); NO change lands (a camera fix REGRESSES the metric → doctrine forbids it).
>
> **(A) TOOLING FIX — frame-matched capture.** `capture_mission_spawn.sh` now DERIVES the reference from the
> `.vram.bin`+`.pal.bin` that the instrumented DOSBox writes ATOMICALLY inside the ONE SIGUSR2 handler
> (`fist_dump()` writes ram+vram+dac at the same guest instruction). The old flaw was ONLY the reference PNG
> (X11 `import -window root`, grabbed a few frames earlier on the moving camera). The derived reference is
> trivially self-consistent (raw→DAC == PNG by construction). New outputs:
> `tools/oracle/samples/oracle_mission_spawn_framematched{_idx.bin,.png,.vram.bin}` (idx[n]=vram[((n>>2)<<4)|(n&3)],
> mode-13h chain-4). Chain-4 proof: max nonzero vram off 0x3e7f3 == ((63999>>2)<<4)|3. The committed DAC
> `oracle_mission_spawn_dac.pal.bin` is byte-identical to the frame-matched pal → the palette was already
> stable/matched. The old X11 ref `ref/mission_azer1_spawn_native320.png` differs from the frame-matched
> derived PNG by **AE 9948/64000** (the skew, exactly as predicted; static cockpit matches, moving terrain differs).
>
> **(B) THE FRAME-MATCHED ORACLE SPAWN GROUND TRUTH (self-consistent, one SIGUSR2 tick).** TCB@phys0x10000
> X=584402 Y=1141913 alt=12544 head=26729 **+0x3a(pitch)=512 +0x3c(roll)=256** +0x3e(foc)=256. Projection
> (ext base phys0x131000): `90d4=b1c0a498 90d8=39331d90` (**V-base hi=0x39 = tile row 57**, NOT the docs'
> speculated row 122) `9104=7ff62180 9108=fcdbd542 90b8=fff9b7aa 90bc=00ffec42 90c0=01000000 90f8=81
> 3918=00044200`. Derived frame terrain rows8-88: **mean 162, band(idx>=80) mean 185, distinct 94**; band top
> idx **179(3158)/151(2611)/163(1556)/228(1215)/156(1123)/183(969)** = the LIGHT/HIGH tile rows. **So the
> RAM globals (row 57) ARE self-consistent with the VRAM (light terrain) — the "row 57 samples the dark rows"
> claim in the 9200-VADDR verdict was wrong; the V-address MARCHES from row 57 up into the light rows.**
>
> **(C) PORT vs FRAME-MATCHED ORACLE (spawn op-0x24 post #1, `/tmp/fist_fc/fist_native`, DEFAULT build).**
> Compared the port raw 0xA0000 index buffer (`FIST_MISSFB_FBIDX/FBDUMP`) against the frame-matched oracle
> `oracle_mission_spawn_framematched_idx.bin`, terrain band rows8-88 idx>=80 (22055 px):
> | port config | band exact-match | port band mean | full-frame idx-AE |
> |---|---|---|---|
> | DEFAULT | 83 (**0.38%**) | 127.3 | 33145 |
> | +camera inject (`FIST_ISO_3A=512 FIST_ISO_PITCH=256`) | 8 (**0.04%**) | 129.5 | 33220 |
> | +camera **+** oracle live tile (`FIST_ISO_TILE`) | 2 (**0.01%**) | 123.0 | — |
> oracle band mean = **185.3**; port band top idx **102/144/118/97/99/105** = the tile's DARK/LOW rows.
> **DECISIVE:** the camera inject makes the port's OWN 8120 emit the frame-matched oracle globals byte-exact
> (`FIST_R3D_GDUMP`: default `90d8=2dc548e0`(row45) `9104=6c24295e 9108=447acd50`; inject `90d8=39331d90`(row57)
> `9104=7ff62180 9108=fcdbd542` — all == oracle; esi/ebp therefore oracle-exact `0x007ff621/0xfffcdbd5`), and
> the oracle tile is byte-injected — **yet the port still renders mean-123 dark terrain, 0.01% match.** The
> camera fix does NOT reduce the residual; it slightly WORSENS it.
>
> **(D) THE PIN (brutally honest).** With byte-exact camera+globals+depth-step+tile and a self-consistent
> frame-matched reference, the port's 9200 V-address marches from row 57 up to only ~row 126 (linear
> edx+=esi, ~144 texels × ~0.5/texel), sampling tile rows 57-126 (idx 80-144, mean 123). The oracle reaches
> tile rows up to ~248 (idx 179 dominant, mean 185). **Same esi, same texel count, DIFFERENT V-span → the
> oracle's real 9200 accumulates V with a perspective-GROWING step that the port's decompiled/shim 9200 +
> its inline `esi=(90c0*9104)>>32; 9200(ebp,esi)` per-row-step reconstruction does NOT replicate.** This
> confirms the rendered-index verdict's localization (the paged-out per-row depth-step / the 9200 texel-walk)
> as the TRUE residual, and refutes camera/V-base/frame-skew/tile/palette. **Owner: voxel-9200-vaddr /
> voxel-depth-step / voxel-projection.** Part B (port camera): the render TCB +0x3a/+0x3c are never written on
> the mission path (the flight-model rotated-camera bridge a20d/a192/0459 threads only heading +0x38, not
> pitch/roll) → stale 0/-256 → 90d8 row 45. A real defect, but AE-neutral-to-worse → not landed (no band-aid).
> Repro: `DOSBOX=/tmp/debs/dosbox-fist tools/oracle/capture_mission_spawn.sh` (frame-matched); port
> `FIST_ISO=1 FIST_ISO_3A=512 FIST_ISO_PITCH=256 FIST_ISO_TILE=<fm_tile> FIST_MISSFB_FBIDX FIST_MISSFB_FBDUMP`.


> ## RAW-INDEX VERDICT (2026-07-17, the "raw 0xA0000 vs port" iteration) — **THE 8-ITERATION PARADOX IS
> RESOLVED, AND *NOT* IN THE "PNG-INVERSION IS THE FLAW" DIRECTION. The port's rendered terrain indices
> genuinely DIFFER from the oracle's RAW VGA index buffer (0.31% match) — the PNG-inversion was NOT the flaw.
> The render is STRUCTURALLY DIFFERENT: with byte-exact oracle inputs injected, the port's 9200 samples the
> tile's DARK (low) rows while the oracle samples the LIGHT (high) rows. This is a 9200 texel-ROW addressing
> bug, NOT a DAC/palette/LUT bug.** Engine PRISTINE (61453e42/0051cb56/75c6d726); no change lands (band-aids
> forbidden; the fix is a faithful 9200-inner-loop reversal, owner: voxel-projection / voxel-depth-step).
>
> **(1) RAW oracle index-buffer extraction (ground truth, NOT PNG-inverted).** `capture_mission_spawn.sh`
> dumps `<pfx>.vram.bin` (raw `vga.mem.linear`, 256 KB) + `<pfx>.pal.bin` (DAC) at the SIGUSR2 tick. The
> mode is **DOSBox `vga.mode=3` = M_VGA (mode-13h CHAINED)** (from `<pfx>.writers.txt`). DOSBox's chain-4
> store spreads framebuffer pixel `n` (n=0..63999) to `vga.mem.linear` at **`lin = ((n>>2)<<4) | (n&3)`** —
> proven decisively: the frame's max nonzero offset is `0x3e7f3`, which is EXACTLY `((63999>>2)<<4)|3`, and
> the whole-vram nonzero count 56894 == the PNG's 56897, distinct 136 == 136. **Oracle index buffer =
> `idx[n] = vram[((n>>2)<<4)|(n&3)]`.**
>
> **(2) SANITY CHECK PASSES — raw → DAC == ref PNG.** Mapping the extracted `idx` through the dumped DAC
> reproduces the freshly-captured `oracle_spawn.png` at **86.4% overall, 98.7% on the STATIC cockpit (rows
> 88-200), 90.9% sky**; all 64000 PNG colours are present in the DAC (0 unresolved). The ~31% terrain
> residual is pure FRAME-SKEW (the capture grabs the PNG, THEN fires SIGUSR2 after a `sleep 1`, so the vram
> is a slightly later voxel frame — the camera drove ~500 units). So the committed DAC + the ref PNG + the
> vram are MUTUALLY CONSISTENT for the static frame — the docs-(4) "mutually inconsistent captures" worry
> is disproven for the palette/DAC; the layout+DAC are correct.
>
> **(3) THE PRIMARY DELIVERABLE — RAW port-vs-oracle index match (both from real 0xA0000).** Port buffer =
> `FIST_MISSFB_FBIDX FIST_MISSFB_FBDUMP` → `/tmp/fb_idx.bin` (spawn post #1, deterministic). Region match:
> cockpit **76.7%**, sky **54%**, **terrain (rows 8-88) 7.6%**, and in the true terrain band (idx>=80) only
> **0.31%** — essentially IDENTICAL to the prior 0.2% PNG-inverted number. **=> The PNG-inversion was NOT
> the methodology flaw; the render genuinely diverges.** Injecting byte-exact oracle projection globals +
> horizon (`FIST_ISO_PROJ FIST_ISO_HZ`) + the FRAME-MATCHED oracle tile (`FIST_ISO_TILE`, extracted from the
> same RAM dump @ phys 0x175200) leaves the band-match at **0.00%** — every 9200 input byte-exact, output
> still wrong.
>
> **(4) THE PIN (precise, decisive).** The oracle terrain output indices are ALL in the tile (only sky/fog
> idx 0-33/251/252 aren't) → **no colour LUT; the render is pure tile sampling.** Oracle terrain is dominated
> by idx **179 (3069px) / 151 (2627) / 163 (1494) / 156 / 228 / 183** — which live in the tile's **HIGH rows
> (179 @ rows 122-248, 151 @ 101-242, 163 @ 116-242) = the LIGHT/textured band**. The port (even with the
> oracle tile injected) is dominated by idx **102 (922) / 144 / 118 / 105 / 97** — the tile's **LOW rows (97
> @ 14-155, 102 @ 14-186, 80 @ 3-100) = the DARK band** (port idx 80 count in the render ≈ 0 though the tile
> is 80-dominated; oracle idx 80 ≈ 321). **Same tile, byte-exact projection → `FUN_0000_9200` addresses the
> wrong colormap ROW (the `(edx>>24)` V-address is systematically LOW).** This confirms the colour-fidelity
> iteration's "port samples the DARK rows of a correct tile" thesis with the RAW ground truth, and refutes
> "palette / DAC / PNG-inversion" as the cause. Repro seams (all default-OFF): oracle `capture_mission_spawn.sh`
> (`.vram.bin`+`.pal.bin`), de-interleave `idx[n]=vram[((n>>2)<<4)|(n&3)]`; port `FIST_MISSFB_FBIDX/FBDUMP`;
> inject `FIST_ISO=1 FIST_ISO_PROJ=1 FIST_ISO_HZ=1 FIST_ISO_TILE=<64KB@0x175200>`.
>
> ## 9200 DEPTH-STEP VERDICT (2026-07-17, the "capture param_1/param_2" iteration) — **THE 9200 DEPTH-STEP
> IS ALREADY FAITHFUL. The task premise ("the shim guesses the per-row depth step wrong") is REFUTED by
> disassembly + camera-injection + offline sim. NO shim/engine change warranted; the divergence is UPSTREAM
> in the TCB camera pitch/roll, and there is a deeper ground-truth INCONSISTENCY that overturns the
> 6-iteration inject premise.** GROUND TRUTH captured/derived; engine PRISTINE (61453e42/0051cb56/75c6d726).
>
> **(0) 82b8/8120/9200 layout, from the disassembly of `re_out/fist_image.bin` (extender flat, base 0):**
> `82b8 = 8120(projection) -> 9200(render) -> 82d0(teardown)` — ONE call renders the WHOLE frame; there is
> NO paged-out per-column caller. **9200 is affine-per-scanline (Mode-7 style):** outer loop = `90f8`(=81)
> iterations, inner = `90f0/2 - horizon` texels ×2 px. Tile lookup = `M[(edx>>24)<<8 | (ebx>>24)]`, tile
> base = `DAT_0000_3918` (host ptr; patch 286 rebased the SMC `0x7fffffff`). **esi/ebp (=param_2/param_1)
> are the per-INNER-texel step; `90b8/90bc` are the per-OUTER step.** 8120's tail (asm 0x8239-0x8251):
> `esi = high32(90c0 * 9104)`, `ebp = high32(90c0 * 9108)` (signed imul), then `90b8 = (ebp*90b4)>>16`,
> `90bc = (esi*90b4)>>16`. The shim (`native_main.c` ~1217) calls `m_ext_FUN_0000_8120()` THEN computes
> esi/ebp with this EXACT formula and calls `9200(ebp,esi)` — a faithful reproduction of 82b8's body.
>
> **(1) THE REAL param_1/param_2 (ground truth) = `param_1(ebp)=0xfffcdbd5`, `param_2(esi)=0x007ff621`.**
> Derived from the RAM-captured oracle globals `90c0=0x01000000 9104=0x7ff62180 9108=0xfcdbd542` and
> INDEPENDENTLY cross-validated: the RAM-captured `90b8=0xfff9b7aa == 2*ebp` and `90bc=0x00ffec42 == 2*esi`
> EXACTLY (an independent capture confirms the derivation). The shim's formula reproduces these bit-exactly
> when fed the oracle 90c0/9104/9108, so the shim's depth-step is NOT a guess — it is the asm.
> (A DOSBox 9200-entry breakpoint would read the same ESI/EBP = the same values; it adds nothing beyond
> what the RAM-captured globals already pin, and would NOT resolve the inconsistency in (4).)
>
> **(2) WHY the DEFAULT port diverges = the TCB CAMERA PITCH/ROLL, not the depth-step.** `FIST_R3D_GDUMP` on
> the DEFAULT build shows the port's own 8120 produces `9104=6c24295e 9108=447acd50` (9108 sign FLIPPED vs
> oracle 0xfcdbd542) → wrong-direction sampling. Root cause: 8120 reads TCB `+0x3c`(roll)/`+0x3a`(pitch);
> the port has `+0x3a=0 +0x3c=-256`, the oracle spawn has `+0x3a=512 +0x3c=256` (already flagged ✗ in the
> camera-orientation table below). **Injecting the oracle angles (`FIST_ISO=1 FIST_ISO_3A=512
> FIST_ISO_PITCH=256`) makes the port's OWN 8120 reproduce ALL oracle projection globals byte-exact**
> (`9104/9108/90d4/90d8/90b8/90bc` all == oracle). So the port's 8120 + depth-step are faithful; the lever
> is the camera pitch/roll init (owner: camera-orientation / flightmodel-*), NOT 9200.
>
> **(3) BUT the camera is AE-NEUTRAL.** Full-frame AE (spawn, `ref/mission_azer1_spawn_native320.png`):
> default `33253`, camera-inject `33277` (≈identical). So fixing the camera angles does NOT fix the terrain
> colour either — consistent with the camera-orientation iteration's "identical output" note.
>
> **(4) THE DEEPER INCONSISTENCY (overturns the 6-iteration inject premise).** With the correct camera →
> byte-exact oracle globals AND the oracle live tile injected (`FIST_ISO_TILE=oracle_bc9c_matrix_blockB`),
> the port's 9200 STILL gives band(idx>=80) exact-match **0.000%** vs the oracle PNG-inverted indices.
> Direct comparison of the port terrain band (rows 8-88): port 150 distinct, mean **124**, range 83-255;
> oracle PNG 176 distinct, mean **183**, range 80-252. The port samples the tile RICHLY (150 distinct, close
> to oracle's 176) but SYSTEMATICALLY DARKER (mean 124 vs 183) and positionally unaligned. **Since every
> 9200 input (esi/ebp/90d4/90d8/90b8/90bc/90f0/90f8/9114/horizon + the oracle tile) is byte-exact yet the
> output is still wrong, the captured "oracle globals + oracle DAC + ref PNG" are MUTUALLY INCONSISTENT** —
> the globals the 6 iterations injected cannot reproduce the PNG frame. The residual is therefore NOT the
> depth-step and NOT reachable by injecting these captures; it is either (a) a wrong-tick globals capture
> (the RAM dump not frame-matched to the PNG) or (b) the tile ADDRESS/WINDOW geometry (`[3918]` offset /
> the `0xf200` window / bc06 rows 0..13 from `docs/oracle_bc90_capture.md`) so the same tile is sampled at
> different addresses. **The next productive step is a frame-matched re-capture (globals + DAC + VRAM at the
> IDENTICAL SIGUSR2 tick) so the three ground-truths are self-consistent, THEN re-diff — not another
> depth-step experiment.** Banked seams unchanged; new one-liner repro of this verdict:
> `FIST_ISO=1 FIST_ISO_3A=512 FIST_ISO_PITCH=256 FIST_ISO_TILE=<blockB.bin> FIST_MISSFB_FBIDX
> FIST_MISSFB_FBDUMP` -> `/tmp/fb_idx.bin`, invert the ref PNG through the DAC, band-diff rows 8-88.


> ## RENDERED-INDEX COMPARISON VERDICT (2026-07-17, the experiment that resolves the 6-iteration paradox) —
> **THE PORT'S RENDERED TERRAIN INDICES DO *NOT* MATCH THE ORACLE'S. The residual is the 9200 SAMPLER, NOT
> the DAC palette — and NOT the projection globals/camera/tile either. This OVERTURNS the prior "59% palette
> + 41% sampler" decomposition: it is ~100% the texel-walk.**
>
> **Method (both at the AZER1 spawn frame, same camera).** PORT index buffer = 0xA0000 dumped raw
> (`FIST_MISSFB_FBIDX FIST_MISSFB_FBDUMP` -> `/tmp/fb_idx.bin`, the 8-bit palette indices BEFORE fist_vga's
> DAC). ORACLE index buffer = the committed spawn ref `ref/mission_azer1_spawn_native320.png` INVERTED
> through the committed oracle DAC `tools/oracle/samples/oracle_mission_spawn_dac.pal.bin` (6->8 bit
> `(v<<2)|(v>>4)`). The inversion is EXACT and unambiguous: nearest-DAC distance = **0.0 for all 64000 px**,
> and DAC[80..255] (the terrain band) is **176/176 distinct RGB** -> every terrain-band index is recovered
> uniquely (the only DAC RGB-collisions are in the cockpit band 4/5/26/27/28/47/54, irrelevant to terrain).
>
> **RESULT (terrain rows 8-88):**
> - **Zero-mask (terrain silhouette) is 100% IDENTICAL** — port 303 black px == oracle 303 black px at the
>   identical positions. The projection/coverage is correct.
> - **Indices DIFFER** — overall exact-index match **10.4%**; in the *true* oracle terrain band (idx>=80)
>   the match is **0.2%**. The low-index matches (3,7,18,19) are the sky/haze band, not the terrain.
> - Port terrain indices are COLLAPSED into a narrow LOW band (mean ~117, mostly 83-106); the oracle spans
>   textured HIGH indices (mean ~183, 130-225). Port per-screen-row mean is nearly CONSTANT with depth
>   (121->108 over rows 8-88); the oracle's varies (189->130). Not a spatial shift (best dx=dy=0), not a
>   clean remap (port->oracle function purity 35%), moderate corr 0.74.
>
> **The tile CONTENT is not the cause.** The port's live bc9c block-base tile == the oracle blockB tile on
> the top values (80/111/102/125/97/105/144 identical); both span 80-255. `FIST_TILEWRAP=0xf200` does NOT
> improve the index match (9.7% ~ baseline). So the SAME tile is sampled at DIFFERENT addresses.
>
> **The projection globals are not the cause (decisive).** Injecting the byte-exact captured oracle globals
> (`FIST_ISO=1 FIST_ISO_PROJ=1 FIST_ISO_HZ=1`) — VERIFIED via the new `FIST_R3D_GDUMP` seam to reach 9200
> as 90d4=b1c0a498 90d8=39331d90 90b8=fff9b7aa 90bc=00ffec42 9104=7ff62180 9108=fcdbd542, all == the docs'
> oracle capture — leaves the port at meanidx 117 / band-match **0.02%** (i.e. WORSE, not better). Camera
> orientation inject (`FIST_ISO_3A=512 FIST_ISO_PITCH=256`) gives the IDENTICAL output (it produces the same
> globals). Scaling the depth-step params (`FIST_R3D_PSCALE`) is not the fix either: 2/1 -> 0.5%, 1/2 -> 0%.
>
> **LOCALIZATION (the precise pin).** With byte-exact oracle globals AND byte-exact tile AND identical
> silhouette, `FUN_0000_9200` still samples the wrong texels. The one input NOT recoverable from the image
> is 9200's `param_1`/`param_2` — the per-row DEPTH STEP passed by `FUN_0000_82b8`'s caller, which is
> **Doug-Huffman-extender 32-bit-PM code PAGED OUT of `fist_image.bin`**. The shim reconstructs it inline
> (`native_main.c` ~1219) as `esi=(90c0*9104)>>32, ebp=(90c0*9108)>>32; 9200(ebp,esi)` — a best-guess that
> does NOT reproduce the oracle ray march. NB 8120 DOUBLES 9104/9108 at its tail (`9104*=2; 9108*=2`, ext
> 0x8120 12967/12972) then uses iVar4/iVar5=(90c0*9104/9108)>>32 to build the per-COLUMN step 90b8/90bc; the
> shim reuses those SAME quantities as the per-ROW step — plausibly the defect, but not fixable by a uniform
> scale (the ratio/sign is also off). **Next experiment = capture the REAL param_1/param_2 at 9200's entry
> in the running original** (instrument the DOSBox 9200-entry, read ESI/EBP/stack) — that is the ground truth
> the reconstruction lacks. Banked repro seams (default-OFF): `FIST_MISSFB_FBIDX`/`FBDUMP` (port indices),
> `FIST_R3D_GDUMP` (9200 globals), `FIST_R3D_PSCALE` (param scale), `FIST_ISO_PROJ`/`FIST_ISO_HZ` (oracle
> globals), `capture_mission_spawn.sh` (oracle DAC/RAM/VRAM). **STOP re-testing the palette (5260) — it can
> only matter once the indices match, which they do not.**


> ## ORACLE TERRAIN DAC CAPTURE + VERDICT (2026-07-17, palette iteration) — **THE PALETTE PREMISE IS
> CONFIRMED (532.pal is the WRONG terrain palette) AND ITS FAITHFUL SOURCE IS FOUND: the oracle's live VGA
> DAC[80..255] at spawn == the port's own ext+0x5260 (SORTED-DISPLAY palette), byte-exact 528/528. BUT
> swapping the shim merge 5598→5260 REGRESSES the frame — the palette was NEVER the dominant residual; the
> 9200 SAMPLER/INDEX is (~80% of the terrain colour spread). Per the camera-orientation precedent, the
> default keeps 5598; the faithful 5260 is a reproduction seam (`FIST_MISSFB_PAL5260=1`).**
>
> **Captured the oracle's actual VGA DAC at the spawn frame.** The instrumented DOSBox already dumps
> `vga.dac.rgb` (256×3, 6-bit) on SIGUSR2 → `<prefix>.pal.bin` (`tools/oracle/capture_mission_spawn.sh`;
> sample committed `tools/oracle/samples/oracle_mission_spawn_dac.pal.bin`, md5 `d7017fa1`). Terrain band
> DAC[80..255] mean 8-bit = **(119.5,104.1,79.8)**; e.g. idx80=(4,0,12) idx125=(60,65,77) idx180=(142,121,36)
> idx255=(255,255,219).
>
> **What it IS (numerically diffed vs the loaded game palettes, ext base phys 0x131000):**
> - **oracle DAC[80..255] == port ext+0x5260 : 528/528 byte-exact** (mean (119.5,104.1,79.8) = oracle).
> - oracle DAC[80..255] == port ext+0x5598 (532.pal) : **14/528** (mean (113.9,83.1,56.5)) → **532.pal is
>   NOT the terrain palette.** The full 768 DAC also matches the guest palette shadow @ phys 0x532d0
>   (768/768) and the oracle's own ext+0x5260 @ phys 0x136260 (band-exact).
> - ext+0x4f60 (5260>>1) : 4/528. So the DAC band is unambiguously **5260**, the SORTED-DISPLAY palette.
>
> **ASM source of 5260 (FUN_0000_9f65, `re_out/fist_ext.c`):** the map-load palette-finalise calls
> FUN_0000_9f10 (sort ext+0x5598 IN-PLACE by luminance R+2G+B), then copies the sorted table into ext+0x5260
> (`puVar6=&DAT_0000_5260; for(0xc0 dwords) *puVar6=*puVar5;` — 768 B from 5598) and into the per-frame
> display palette TCB+0xea. So 5260 is the display palette the ORIGINAL uploads to the DAC — real
> game-computed data (the port's own op-0x18 map-load produces it), NOT an oracle paste.
>
> **THE HONEST TWIST — the faithful palette does NOT fix the colour (it regresses it):**
> - Default (5598): spawn md5 `b15766c0`, terrain mean rows8-88 **(107.6,78.7,56.6)**, full SAD 5.29M.
> - Faithful (5260): spawn md5 `0a2b1d59`, terrain mean **(81.7,68.5,50.6)** — DARKER, FURTHER from oracle
>   (122.5,111.9,89.0); full SAD **5.53M** (worse). full-AE diff-px 33253→33180.
> - **Why:** per-index analysis — at the positions where the port renders each terrain index i, the oracle's
>   colour there is ~(120,110,80) for MANY distinct i (80,82,83,84,87,89,91,…). The port renders SCRAMBLED
>   indices vs the oracle, so no single palette reconciles them: |oracle_at_portpx − 5260[i]| = 142/px vs
>   |… − 5598[i]| = 133/px. An IDEAL per-index remap (the docs' "59% palette") leaves **23.6/chan
>   sampler-locked** (~80% of the terrain colour spread 29.4) — and that ideal remap is NOT the faithful
>   palette. 5598 was masking the sampler bug by being a brighter (wrong) table.
> - **CONCLUSION:** the "59% palette / 41% sampler" decomposition was an ideal-remap UPPER BOUND, not
>   realizable with the true palette. The dominant, true residual is the **9200 sampler / tile3918 index**
>   (the port samples different texels than the oracle) — owners: colormap-groundtruth / voxel-projection /
>   terrain-color-fidelity. The palette is faithful (5260) and one flag (`FIST_MISSFB_PAL5260=1`) away, but
>   landing it regresses the metric with no colour benefit → not landed (camera-orientation precedent).
>   Repro: `tools/oracle/capture_mission_spawn.sh` (oracle DAC) + `FIST_MISSFB_PALDUMP`/`FIST_PALDUMP`
>   (port 5260/5598) + `FIST_MISSFB_FBIDX FIST_MISSFB_FBDUMP` (port indices).


> ## CAPTURE + INJECT VERDICT (2026-07-17, camera-orientation iteration) — **THE COLOUR RESIDUAL IS NOT THE
> CAMERA/ORIENTATION/PROJECTION. Proven by direct injection of the ORACLE spawn camera + projection + tile
> + horizon — the port terrain STILL renders (110,81,58) vs oracle (122,111,89).** The residual is
> DOWNSTREAM: ~59% is the PALETTE, ~41% is a residual sampler/index difference (both deep subsystems).
>
> **Captured oracle spawn render-camera TCB orientation** (RAM dump `SIGUSR2` at frame-1, TCB @ phys 0x10000,
> X@+0x2c — X=584282 Y=1142097 alt=12800, matches the committed spawn):
>
> | field | oracle spawn | port live spawn | match? |
> |---|---|---|---|
> | +0x38 heading | **26729** (0x6869) | 26729 | ✓ |
> | +0x3a | **512** (0x0200) | 0 | ✗ |
> | +0x3c | **256** (0x0100) | -256 (0xff00) | ✗ |
> | +0x3e foc | **256** (0x0100) | 256 | ✓ |
>
> **Captured oracle spawn projection globals** (extender base phys 0x131000; the state 9200 walks):
> `90b8=0xfff9b7aa 90bc=0x00ffec42 90d4=0xb1c0a498 90d8=0x39331d90 90b4=0x00020000 90c0=0x01000000
> 9104=0x7ff62180 9108=0xfcdbd542 90e8=0x02000000`, view width `90f8=81`, colheight `90f0=288`,
> horizon table `9114→0x7568` = `[6,4,3,2,1, 0×64, 1,2,3,4,5,6,7,8,8,9,9]`, colormap base `3918=0x44200`.
> **Oracle live tile3918** (ext-flat 0x44200 = phys 0x175200, 64 KB): 212 distinct, min idx 37 max 249,
> top idx 80(6926)/111(1703)/102(1539)/125(1513)/97(1245).
>
> **INJECT-CONFIRMATION (FIST_ISO_* diagnostic seams in `tools/native_main.c`, default OFF; port spawn frame,
> `FIST_MISSFB_N=1`, terrain band rows 8-88, oracle ref `ref/mission_azer1_spawn_native320.png`):**
> - **The port's 8120 is BYTE-EXACT correct.** Injecting the oracle orientation (`FIST_ISO_3A=512
>   FIST_ISO_PITCH=256`) makes the port's 8120 produce the oracle projection globals BYTE-FOR-BYTE
>   (90d4=b1c0a498, 90d8=39331d90, 90b8=fff9b7aa, 90bc=00ffec42 — all identical to the oracle). So the
>   only camera defect is the port's wrong live 0x3a/0x3c; 8120 transforms it faithfully.
> - Yet the colour barely moves: baseline terrain mean **(107.6,78.7,56.6)** → orientation-inject
>   **(110.5,80.7,58.4)** → +projection+horizon **(110.5,80.7,58.4)** (identical — because injected
>   orientation already yields the oracle projection) → +oracle-tile **(110.4,81.0,58.6)**. Oracle =
>   **(122.5,111.9,89.0)**. Every full injection saturates ~76% short of the oracle colour.
> - **Coverage is fine** (port 319 / oracle 318 cols filled; the "narrow window" premise is outdated).
> - **A spatial shift search is flat** (142–148 mean-abs-diff/px for every dy∈[-8,8], dx∈[-8,8]) → NOT a
>   projection/alignment offset; it is a uniform colour shift (oracle brighter + greener everywhere).
> - **Decomposition** (port tile+proj indices vs oracle ref): an IDEAL per-index palette drops the residual
>   from ~147 to **60.5** mean-abs-diff/px → **~59% of the colour error is the PALETTE** (532.pal is likely
>   the WRONG terrain palette for AZER1 — the port hardcodes it in the shim merge), and **~41% (60.5/px) is
>   a residual sampler/index difference** that survives even with the oracle tile + oracle projection
>   injected (the port's 9200 samples slightly different texels; camera XY is NOT injected — port
>   583982/1142557 vs oracle 584282/1142097).
>
> **CONSEQUENCE.** The task premise "the residual is the CAMERA/depth-projection" is **DISPROVEN by
> injection**. Fixing the port's wrong orientation (0x3a/0x3c) is a genuine defect but does NOT fix the
> colour (it slightly WORSENS the per-pixel AE: inject AE 5.37M vs baseline 5.29M, because the
> palette/sampler residual dominates and the current port orientation happens to align marginally better).
> The orientation is written by the extender flight-model (absent in the port); the render TCB +0x2c/0x30/
> 0x34 are bridged by dd15/a84c but +0x38..+0x3e (orientation) are NOT — that bridge is extender-role
> (a20d rotated camera / flight-model init), a deep subsystem, not a clean base-loss. **No engine/default
> change lands this iteration** (doctrine: no band-aid; the fix doesn't move the acceptance metric and would
> regress the deterministic spawn md5 with no colour benefit). Owners of the real frontier: the terrain
> PALETTE (which .PAL AZER1's colormap uses vs the hardcoded 532.pal merge) + the 9200 sampler/index
> fidelity. Diagnostic seams committed: `FIST_ISO_3A` (TCB+0x3a), `FIST_ISO_PROJ` (force oracle projection
> globals post-8120), `FIST_ISO_HZ` (oracle horizon), `FIST_ISO_TILE=<64KB file>` (force the 3918 colormap).


> ## VERDICT (2026-07-17) — **THE ORACLE RENDERS FULL VOXEL TERRAIN + SKY AT FRAME-1 / SPAWN.** The
> port's black windshield is a **render-wire + camera-Z bug fixable NOW** — it does NOT need the
> extender flight-model to first drive/settle the vehicle. Proven by a genuine frame-matched DOSBox
> capture of the ORIGINAL at the earliest mission frame, compared against the port's deterministic
> frame `6bc113e6` at the SAME sim state.

## Method (genuine, non-circular, 1:1 320×200)
`tools/oracle/capture_mission_spawn.sh` drives the ORIGINAL (`LOADGAME FIST.RUN`) under the instrumented
DOSBox (`tools/oracle/dosbox_vga_terrain_trace.patch`, `/tmp/debs/dosbox-fist`) through
BATTLES→OK→ACCEPT, then polls the SDL surface every 0.3 s. The `LOADING MISSION WITH SUPER DETAIL…`
photo grabs are large PNGs (~30–40 KB); the **first windshield frame** drops to ~14–22 KB. That
transition **is** frame-1 / spawn. The grab is the `output=surface scaler=none aspect=false` mode-13h
surface (`-trim` rect asserted **320×200**), 6→8-bit DAC = `(v<<2)|(v>>4)` — the same expander
`fist_vga.c`/`FIST_MISSFB` use, so it is byte-comparable to the port dump.
**Non-circular:** the reference is DOSBox rendering the ORIGINAL, never a copy of the port; its md5
(`75d40a0d…`) ≠ the port frame-1 md5 (`6bc113e6…`).

At the exact frame-1 grab, `SIGUSR2` dumps guest RAM; the live camera is read by scanning for the
spawn XY.

## What the oracle draws at frame-1 / spawn
- **Full voxel TERRAIN is rendered** — brown/tan hills, 91 distinct colours, only 1.2 % black in the
  windshield (rows 8–88). It is NOT a fade-in / black / cockpit-only frame.
- **SKY is present** — blue cloud band (e.g. RGB `93,97,117`) across the top of the windshield.
- The LOADING photo → windshield transition is a hard cut; the very first windshield frame already has
  the terrain drawn (camera barely moved: odometer ≈ spawn, XY within ~500 units of spawn).

## The camera at frame-1 / spawn (guest RAM, live TCB mirror @ phys `0x1002c`, robust across 2 runs)
| | X | Y | alt | terrain h = alt/≈292 |
|---|---|---|---|---|
| **oracle spawn (frame-1)** | 584312 | 1142051 | **12800** | **43.8** |
| mission-file spawn record (`0x034d14`) | 583982 | 1142557 | 0 | — |
| **port live spawn** (per task) | 583982 | 1142557 | **3328** | 11.4 |
| oracle settled (task) | — | — | 26880 | 92 (h≈97) |

- The oracle camera Z **tracks the terrain height under the camera**: at spawn `h=43 → alt≈12800`
  (`43×292≈12556`), settled `h=97 → alt≈26880`. This "climb" is the **tank driving from low ground
  (h=43) onto higher ground (h=97)** — it is applied per-frame (instant terrain-follow at spawn), NOT a
  multi-second altitude ramp.
- **The port's spawn alt (3328) is itself wrong** — far below the oracle's 12800; 3328 corresponds to
  h≈11.4, i.e. the port camera is set well BELOW the terrain surface it sits on. The port likely sets
  camera Z to a fixed eye offset WITHOUT adding `terrain_height × scale`.

## The CORRECT comparison (port frame-1 `6bc113e6` vs oracle frame-1, both at spawn)
Cockpit chrome is already registered (proof the frames are aligned), so the whole delta is the missing
terrain+sky render — not a sim-position mismatch:

| region | metric |
|---|---|
| terrain/windshield rows 8–88 (25600 px) | **AE = 23438** |
| port terrain black | **87.2 %** (vs oracle **1.2 %**) — the window is empty, not garbage-terrain |
| port / oracle distinct terrain colours | 74 / 91 |
| sky band rows 8–20 | port 90 % black; oracle blue `93,97,117` |
| cockpit gearstick/gauge chrome (rows 108–150, cols 88–160) | **AE = 0** (frames registered) |

The prior frame-1-vs-**settled** terrain AE was 24618; this frame-1-vs-**frame-1** AE (23438) confirms
the magnitude was not driven by the sim-position mismatch — it is the missing render.

## Consequence for the frontier
The port's black terrain is **fixable now**, independent of the extender flight/momentum model:
1. **Render-wire:** the windshield voxel writer (extender `9200`, dispatched via op-0x24) must actually
   draw into `0xA0000` — the oracle proves terrain renders at the exact spawn camera state.
2. **Camera-Z init:** set the spawn camera alt to follow terrain height (`h×scale + eye`) so it matches
   the oracle's ~12800 (not the current 3328); the port camera is presently below the surface. This is
   an instant per-frame terrain-follow, NOT the gradual flight-model settle.

Whether (1) alone suffices, or (2) is also required for 9200 to produce non-black pixels, is the
port-side next step (set the port camera alt to 12800 and re-check 9200's output) — outside this
capture task.

## Artifacts committed
- `ref/mission_azer1_spawn_native320.png` — genuine 1:1 320×200 oracle frame-1 (md5 `75d40a0d…`).
- `tools/oracle/samples/oracle_mission_loading.png` — the LOADING photo (context; the frame BEFORE spawn).
- `tools/oracle/samples/oracle_mission_settled_ref.png` — a settled windshield (context; terrain evolved).
- `tools/oracle/capture_mission_spawn.sh` — reproducible capture + live-camera read.

## Repro
```bash
# needs the instrumented dosbox (tools/oracle/README_terrain.md) at /tmp/debs/dosbox-fist
DOSBOX=/tmp/debs/dosbox-fist tools/oracle/capture_mission_spawn.sh
# port frame-1 for the AE comparison (md5 6bc113e6):
SCR=$(mktemp -d); cp -a armoredfist "$SCR/af"
setarch -R env FIST_DATADIR="$SCR/af" FIST_COOP_TICK=1 FIST_TICK_HZ=25000 \
  FIST_MOUSE="200:160:100:0;800:160:100:1;1400:160:100:0;3000:205:128:0;3600:205:128:1;4200:205:128:0;5400:40:186:0;6000:40:186:1;6600:40:186:0;7200:40:186:0" \
  FIST_MISSFB=/tmp/port_f1.ppm FIST_MISSFB_N=30 /tmp/fist_native
```

## HONEST STATUS after patch 295c3bf (self-measured 2026-07-17) — voxel terrain DRAWS at spawn, but only PARTIALLY covers the windshield
Patch 295c3bf (shim, in-mission-guarded) wired 9200 + the oracle-exact camera-Z terrain-follow. Verified by direct measurement (not the agent's window-only figure):
- **Spawn frame (op-0x24 post #1) is DETERMINISTIC: md5 `6678719e`, 3/3 identical.** Camera alt = 12800 (oracle-exact, `(h<<8)+1792`, h=43). REAL win — the voxel terrain draws for the first time, and the camera-Z is asm/oracle-exact.
- **BUT the full 320-wide windshield (rows 8-88) is 91.6% black (23450/25600)** vs the oracle's 1.2% (303/25600). The agent's "black 87%→1%" was measured on the ~81-col voxel WINDOW only (`0x90f8=81 cols`, fb+0x650), NOT the full width — misleading for the full frame. Only ~2150 px are non-black at spawn vs the oracle's ~25300. So the port draws terrain in a NARROW sub-window; the oracle fills the FULL windshield width.
- **Later op-0x24 posts are NON-deterministic** (post #30 md5 varies `20692847`/`aac4b376`) because the live camera XY BLOWS UP (66M/331M) — the wall-clock-dependent flight-model over-run (only visible now that terrain renders). Post #1/spawn is stable; later frames need the camera-XY determinism (a separate frontier).
- verify.sh both = 24/24 (in-mission guard → menu flows unaffected); pristine engine unchanged.
**RANKED spawn-frame residuals (vs ref/mission_azer1_spawn_native320.png):** (1) **render coverage** — the voxel terrain covers only ~2150 px of the windshield vs the oracle's ~25300 (the ~81-col window / horizon / clip — WHY does the port cover a fraction of the width the oracle fills? = the biggest residual); (2) **tile palette ~2× too dark** (browns 60,32,12 vs 125,109,77 — the tile3918/85b8 colormap-collapse, 175 vs 212 distinct — NOT block A which is byte-exact); (3) **sky band** (blue 93,97,117 rows 8-20, `.SKY`-fed, absent); (4) camera projection / horizon position; (5) later-post camera-XY determinism.

> ## CORRECTION (2026-07-17, colour-fidelity iteration) — RESIDUAL #2 IS **NOT** A PALETTE BUG **NOR A TILE-CONTENT BUG**; IT IS THE RENDERER SAMPLING THE **DARK ROWS** OF A CORRECT TILE (the depth-row / camera-projection geometry = residual #1/#4). No palette or bc9c fix warranted this iteration.
>
> **Method (decisive, mostly offline; live spawn frame deterministic md5 `b15766c0`, FULL-frame AE 33253).**
> Captured the default 92a2692 spawn (`setarch -R FIST_COOP_TICK=1 FIST_TICK_HZ=25000 …
> FIST_MISSFB_N=1`, ext image `re_out/fist_image.bin` present — it is gitignored, `make kernel-image` in a
> fresh worktree first or g_ext_ready stays false and NO op-0x24 post fires). Dumped the fb indices
> (`FIST_MISSFB_FBIDX/FBDUMP`), the merged DAC (`pal_merged.bin`), the live tile (`FIST_MTXDUMP`) and the
> bc9c block-base (`FIST_BBDUMP`), and mapped everything through the loaded 532.pal.
>
> **(a) PALETTE RULED OUT.** The mission DAC band 80..255 is byte-exact 532.pal (`FIST_MISSFB_PALCMP`:
> eng782 == ext5598 in the terrain band, e.g. idx220), the 6→8 expansion is correct VGA bit-replication,
> and **532.pal CONTAINS the oracle browns** — the oracle's (125,109,77) is idx 90/106/170 (dist ≤13),
> its (117,101,48) is **exactly idx 180**. So the light tans the oracle shows are reachable; the palette
> is not the problem.
>
> **(b) TILE CONTENT RULED OUT.** The port bc9c block-base (`FIST_BBDUMP`) windowed at **0xf200 == oracle
> blockB rows 14..255 = 61952/61952 (100%)** (rows 0..13 = bc06 LOD, the known separate gap), and — mapped
> through 532.pal — the port block-base and the oracle blockB have **IDENTICAL colour distribution**: same
> mean RGB **(120,88,61)** and identical top-5 indices+counts (idx80 6885/6926, idx111 1703, idx102 1539,
> idx125 1513, idx97 1245). The 175-vs-212 distinct is only in the rare tail. bc9c is fine for colour.
>
> **(c) THE REAL DEFECT = the renderer samples the DARK rows of the tile.** The port RENDER's terrain-band
> mean is **(109,78,54)** — BELOW the tile-content mean (120,88,61); its top rendered indices are 83
> (65,48,12), 118 (85,40,24), 151 (44,28,24) = the DARK end. The oracle RENDER is **(122,106,71)** — ABOVE
> the tile mean, GREENER than the tile content, i.e. it samples the LIGHT/green rows. Since the tile the two
> read is byte-equal in content, the divergence is **which texels the ray-march / depth-projection selects**
> (the tile's row axis IS the depth-shade ramp; the port picks deeper/darker rows). Empirically **no window
> offset fixes it**: `FIST_TILEWIN`/a faithful wrap-around rotation (new default-OFF `FIST_TILEROT` seam,
> tested 0x4200/0xf200) move the render mean by ≤6 and never reach (122,106,71); FULL AE stays 33248..33279.
>
> **VERDICT.** Residual #2 as originally phrased ("tile palette 2× too dark") is a **red herring** — the
> per-pixel (60,32,12) vs (125,109,77) top-colour was a geometry artifact (different coords sample different
> texels). Colour fidelity is **GATED behind residual #1/#4** (the windshield coverage + camera/depth
> projection): once the port samples the same texels as the oracle, the identical tile+palette will produce
> the identical colours. Owners: camera-settle-render / windshield-render-wire / voxel-render-coverage.
> **No engine/ext/shim change lands** (the `FIST_TILEROT` experiment did not help → reverted; pristine
> `61453e42`/`0051cb56`/`75c6d726` unchanged).

## 9200-VADDR VERDICT (2026-07-17, 10th colour iteration) — 9200 IS FAITHFUL; the root is the CAMERA PITCH/ROLL (V-base too low) + a FRAME-SKEWED oracle capture (the reference is internally inconsistent, which invalidated the "byte-exact components don't compose" paradox).
`FUN_0000_9200` disassembled 0x9200-0x92a0 instruction-by-instruction == the Ghidra C (V=(edx>>24), U=(ebx>>24), esi/ebp per-texel step, 90b8/90bc per-scanline step, 90d4/90d8 base) — NO base-loss/sign/width/scale error; an offline asm-faithful sim of the walk matches the port's RUNTIME output (rows 57-136) → 9200 is a faithful reproduction. 8120 (which computes 90d4/90d8/esi/ebp/90b8/90bc) is also asm-exact (patch 341).
**ROOT (two things, neither in 9200):**
1. **The V-base `90d8` is too low.** Port default `90d8=0x2dc548e0` (V-base high byte 0x2d = tile row 45); the oracle's light terrain (idx 179/151/163) lives in tile rows 101-248, needing 90d8 high byte ~0x7a (row ~122). 90d8 is set by 8120 reading the **TCB camera pitch/roll `+0x3a`/`+0x3c`** (port `+0x3a=0/+0x3c=-256` → `9108=0x447acd50`, SIGN-FLIPPED vs oracle `0xfcdbd542`). So the port's spawn camera pitch/roll is WRONG → V-base too low → samples the tile's DARK/LOW rows. Owner = camera-orientation/flightmodel (fix +0x3a/+0x3c at the render tick), NOT 9200.
2. **The oracle capture is FRAME-SKEWED (the paradox's real cause).** `capture_mission_spawn.sh` fires SIGUSR2 after a `sleep 1`, so the RAM-captured globals (90d8=row 57), the ref PNG (needs row 122), and the VRAM are from DIFFERENT voxel frames of the moving mission camera. So the "byte-exact oracle globals" never corresponded to the ref PNG — that is why injecting them (+ oracle tile) gave the ref PNG a 0% match. The 10-iteration "components don't compose" paradox is a FRAME-MATCHING artifact, not a render bug.
**NEXT (the definitive fix path):** (a) FRAME-MATCHED oracle re-capture — dump globals + DAC + raw VRAM + the PNG all at ONE SIGUSR2 tick (no sleep-skew) so the ground truth is self-consistent; determine the oracle's spawn camera pitch/roll (+0x3a/+0x3c) and V-base at THAT frame. (b) Fix the port's spawn camera pitch/roll to match → V-base correct → 9200 samples the right rows. (c) Verify against the frame-matched raw index. Do NOT patch 9200 (faithful) or re-chase palette/tile/depth-step (all byte-exact). pristine UNCHANGED (61453e42/0051cb56/75c6d726).
