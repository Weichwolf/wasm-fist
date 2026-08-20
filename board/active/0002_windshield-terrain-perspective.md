Type: bug
Area: render
Tags: voxel oracle

The windshield renders the terrain in a correct first-person voxel-space
perspective — sky above, a horizon line, terrain receding below — instead of the
current output, which fills the whole windshield with near-field terrain and no
sky/horizon. On the AZER1 clean-M1 spawn (dashboard AE=0 vs the provenance
reference oracle_azer1_windshield_dashAE0.png), the windshield rows 0-95 differ
75.5% (23196/30720 px, meanAE 51.6).

The colormap chain that feeds the render is proven faithful, so the fault is in
the terrain-specific stages: the heightmap [0x85bc] / reduce [0x85b8] build, the
camera/projection (85d0 -> 8120), or the 9200 per-column texel walk. The visual
signature (no horizon, terrain everywhere) points first at camera height (TCB
+0x34 alt) / height-projection scale: rays never escape to sky.

## Comments

Per-row diff vs the AE=0 reference: sky rows 0-4 are BIT-EXACT (0/1600) — the
689a sky/perspective resample is correct. Terrain rows 6-84 differ ~288/320 every
row (~90%), meanAE 43-82. Colour multiset rows 10-85: port 131/129 distinct vs
oracle 84, only 24 shared — the port scatters MORE colours (noisy) = a wrong-INDEX
signature, not a wrong-colourmap one.

Colormap chain verified faithful this round (objdump of the extender image +
port-only replay): file-load of 532.PAL (member 28 of the PAL.RES RESOURCE1
archive, seek 22048 read 768 into ext+0x5598), 9f10 luma-sort, [5598]@bc9c
sorted==[5260], [4f60]==[5260]>>1, ac70 nearest-match (0 divergences over 40960
live calls; its literal sub 0xf is self-modifying code, patch 238 restores the
real target), bc9c blend LUT, and bdc4 the 2x upsample (reads the matrix at
bc90 & 0xffff0000 + prev*256+cur — the same aligned base bc9c writes). The
FIST_BC90DUMP "+0x4200 shift" was a dump-window artifact, not a build error.

oracle_bc9c_matrix_blockB is MISPROVENANCED: its diagonal plateaus at 80..83,
impossible for the sorted-palette pipeline (which yields a linear diagonal); do
not trust it for a bc9c diff. Recapture from the original at a known tick if a
bc9c diff is ever needed.

Camera is FROZEN across posts (frame-timer DGROUP:0x452 stays 1; 459a's spin never
pumps the cooperative tick) — but a single spawn frame should still hold the
initial camera. Next: dump the LIVE TCB camera (ea2e:ea2c +0x2c X /+0x30 Y /+0x34
alt /+0x38 heading /+0x3e focal) at the op-0x2c spawn and sanity-check it; the
shim already has FIST_MISSFB_PROBE for this. If alt/focal are wrong, that is the
terrain-everywhere cause.

Port camera at the AZER1 op-0x24/0x2c spawn render (FIST_MISSFB_PROBE): TCB +0x2c/
+0x30/+0x34 (X/Y/alt) = 583982/1142557/12800, +0x38/+0x3a/+0x3c/+0x3e (heading/roll/
pitch/focal) = 26729/0/-256/256, detail +0xcd/+0xcf = 1/0. alt=12800 matches the
shim's oracle-anchored terrain-follow (h=43, (43<<8)+1792). Prime suspect is
pitch=-256 (looking DOWN): a wrong pitch pushes the horizon above the top edge, so
terrain fills the whole view — exactly the observed symptom. The camera is a SHIM
reconstruction (the 32-bit flight model that writes TCB +0x34/+0x3c is paged out of
fist_image.bin), so the decisive datum is the ORIGINAL's TCB camera at the AZER1
spawn — an instrumented-DOSBox / QEMU guest-RAM capture. Blocked on that oracle
capture (dosbox-fist not built here); until then, verify 85d0->8120->9200 against
the asm to rule the projection in or out port-only.

Port-only verification narrows it further (this round): 85d0 (camera setup) is
asm-exact (objdump 0x85d0: X<<0xd, Y<<0xd neg, alt clamp 0x7f00 <<0x11, heading
<<0x10 neg, 0xffffffff/focal; reads 0x2c/0x30/0x34/0x38/0x3e, NOT pitch 0x3c — so
the earlier pitch=-256 lead is a dead end). The heightmap [0x85bc] is smooth and
coherent (adjacent-cell mean-abs-diff 0.82, min15/max96/mean53, cloud-like relief)
— correctly decoded, not the noise source. 9200 (the per-column sampler) is
faithful: patch 286 restores its self-modifying colormap base (asm `mov al,[eax+
disp]` with disp patched to DAT_0000_3918) and its index math (shld/shld =
(Yhi<<8)|Xhi) matches. So the colormap it samples and the sampler are both right.
Remaining suspect: 8120 (projection -> the param_1/param_2 ray deltas 9200 steps
by) and 8deb (viewport + the per-column horizon/sky-skip table [0x9114]). The probe
showed the per-column horizon (910c) mostly 0 -> terrain from the top edge, vs the
original's large sky band; patch 341 already touched 8120 (signed imul), so verify
8120/8deb next against the asm.

8120 (projection) and the whole render math are also faithful: 8120 DOES read
pitch [c93+0x3c] (movzx = unsigned, matching the asm — so -256 reads as 65280 the
same both sides) and roll [c93+0x3a], and patch 341 already rebuilds all four
projection-coefficient tables at the correct module base (g_mem+ext_base+0x9450/
9454/9650/9654 + idx*4; Ghidra base-lost two as a host function pointer + a bare
address, over-scaled the other two). So 85d0 + 8120 + 9200 + heightmap are all
asm-faithful — the render MATH is correct.

That isolates the defect to the CAMERA STATE fed into the render: the TCB fields
at the spawn frame (X/Y +0x2c/+0x30, alt +0x34, heading +0x38, pitch +0x3c). alt
is a SHIM reconstruction (terrain-follow (h<<8)+eye, the flight model is paged
out); X/Y/heading/pitch are engine-set at mission spawn. The decisive datum is the
ORIGINAL's spawn TCB camera, which needs an instrumented-DOSBox / QEMU guest-RAM
capture at the AZER1 spawn — the same oracle-RAM capability INDIA3 (0006) waits on.
Building that capability unblocks both. A quick shim experiment (sweep the
reconstructed alt / eye-height and re-render) can test whether alt-too-low alone
raises the horizon into view, but matching the original's terrain CONTENT still
needs its full X/Y/heading.
