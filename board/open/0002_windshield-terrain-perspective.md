Type: bug
Area: render
Tags: voxel oracle
Depends: 0007

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

Camera-sweep experiments against the AE=0 framebuffer reference (no guest RAM
needed, so this line of attack is NOT blocked on 0007): overriding the render
camera changes the windshield (a wildly different camera differs 71% from the
default render, so the shim's op-0x24 voxel chain IS drawing it and IS
camera-responsive), yet NEITHER the default NOR any camera override produces the
original's SKY band — every variant fills terrain top-to-bottom and stays 75.5%
different, with the sky rows 5-45 ~78% wrong. So the sky/horizon is missing
independent of camera position/orientation. That points at the horizon/sky-skip
geometry rather than the camera: the per-column sky-skip 910c (read from the baked
table [0x9114] = (&PTR_748c)[detail], detail 1 -> image 0x7568) came out mostly 0
in the probe, and 90f0 (the per-column fill height) / 8deb (viewport setup) decide
how far terrain fills. Verify 8deb + the 0x7568 sky-skip table + 90f0 against the
asm next — a port-only step.

ROOT-CAUSE LEAD (port-only, not blocked on 0007): the missing sky is gated by
[0x395c]. FIST_FORCE395C=1 flips the port from ~no sky (28 sky-ish px) to sky
(22887 px; oracle has 9304) -- proving [0x395c] is the port's sky gate. [0x395c]
(and the map detail [0x8490]) are set in 89b0 (fist_ext.c ~13150-13172) ONLY when
one of the memory-tier probes 5c98("4.MEG"/"8.MEG"/"16.MEG"/"40.MEG") reports found
(built code threads the FILEMGR carry: uVar10 = g_ext_find_cf, branch on CF clear).
There are NO .MEG files in the data (only 8.SKY), so all four probes fail and the
port leaves [0x395c]=0 -> no sky, terrain top-to-bottom.

The contradiction to resolve next: the burst-captured ORIGINAL runs on the SAME
data (no .MEG files) yet renders sky, so the original sets [0x395c]/[0x8490] via a
path the port does not replicate -- a 5c98 fallback, a free-memory check behind the
".MEG" name, or a shim gap in g_ext_find_cf for these probes. Determine what the
original resolves for the .MEG tier (does 5c98 for "N.MEG" test a file or free
memory?), then make the port pick the same tier. Forcing 395c alone still leaves
75.2% (too much sky, and [0x8490] detail likely also wrong), so the fix is the
correct tier (395c AND 8490 together), not just a nonzero 395c.

The .MEG resolution is confirmed a faithful file-probe: 5c98 -> 5cc2(0x4e00) =
find-first, built code threads the carry, and NO .MEG file exists in the data
(only 8.SKY; note .DTL files + LHA.EXE/UNPACK.BAT suggest packed distribution).
The hard contradiction stands: the original on the SAME data renders sky (needs
395c!=0, and 13289's [0x3911] sky build is engine decompile, not shim). So the
original resolves a .MEG tier without a .MEG file -- most likely a 5cc2 secondary
search ([0x622c] resource archive) that the port skips (the shim clears [0x622c]=0
at init, and 13180 sets it to "PAL.RES" only AFTER these probes). Settling this
needs the original's [0x395c]/[0x8490]/[0x622c] at map-load = a guest-RAM capture
(0007). This is the point where the voxel re-converges on 0007.
