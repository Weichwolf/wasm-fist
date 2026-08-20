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

RESOLVED via 0007 (the guest-RAM oracle now exists). capture_tcb_camera.sh 93
captured the ORIGINAL's render-time TCB camera at the AZER1 spawn:
  original: X=583982 Y=1142557 alt=12800 head=26729 pitch(+3a)=384 roll(+3c)=256..384 foc=256 det=1
  port:     X=583982 Y=1142557 alt=12800 head=26729 pitch(+3a)=0   roll(+3c)=-256    foc=256 det=1
X/Y/heading/foc/det MATCH -- the sole camera divergence is PITCH (+0x3a: 384 vs 0)
and ROLL (+0x3c: 256 vs -256). Those are exactly the fields native_main gates
behind FIST_TILEFILL (the paged-out flight model writes them; oracle-anchored
spawn seed). Fix: apply pitch=384/roll=256 on the DEFAULT op-0x24 render path (a
reconstruction like the existing alt/focal seeds), together with the sky gate
[0x395c]. Verify the windshield against oracle_azer1_windshield_dashAE0.png.

Applying the correct pitch=384/roll=256 plus forcing the sky ([0x395c]) still
leaves ~75% and OVER-produces sky (22791 px vs the original's 9304) -- so pitch/roll
alone is not the whole fix and the FIST_FORCE395C sky-build (689a filling the tile)
is not yet the faithful one. Two follow-ups: (a) the guest-RAM oracle also dumped
the original's VRAM at the 9200 render, but FIST.RUN uses VGA MODE-X (planar,
vertical-stripe when read linearly), so the reference frame needs 4-plane
de-interleaving before it can be diffed pixel-exact -- do that to get a same-moment
reference (better than the later burst f04). (b) read the original's [0x395c] /
[0x622c] / [0x8490] out of oracle_azer1_tcb.pass00.ram.bin (paging on, ext ds base
0x10000000, cr3 0xe000) to settle the .MEG-tier question and the correct sky value.
Saved: tools/oracle/samples/oracle_azer1_tcb_camera.txt.

DECISIVE oracle state (page-walked out of oracle_azer1_tcb.pass00.ram.bin, cr3
0xe000, ext ds linear 0x10000000): the ORIGINAL at the AZER1 render has
  [0x395c] = 1   (sky ON;   port = 0)      <- the confirmed sky bug, target value 1
  [0x8490] = 11  (detail;   port = 11)     matches
  [0x622c] = 0x89a3 ("PAL.RES")            resource archive set
Both voxel bugs are now oracle-confirmed with TARGET values: sky [0x395c]=1, and
camera pitch(+0x3a)=384 / roll(+0x3c)=256.

Mechanism puzzle for the sky: [0x395c] is written ONLY by the four .MEG find-first
branches (13155-13170), and 4.MEG/8.MEG set 395c=1 but ALSO 8490=9/10 -- yet the
original has 395c=1 with 8490=11, so either the original found 4/8.MEG (395c=1) and
8490 was recomputed to 11 afterwards, or another path sets 395c. The original finds
a .MEG with no .MEG file on disk (same data), so its find-first for "4.MEG"/"8.MEG"
resolves via a FILEMGR fallback the port skips -- capture the original's file-open
trace + [0x622c] AT the .MEG-probe time (another dosbox-fist run) to see what it
resolves, then make the port resolve the same. Apply the camera pitch/roll (an
oracle-anchored spawn seed like the existing alt/focal) and the sky, then verify
the port's op-0x24 windshield against a dashboard-AE=0 stock-burst frame.

SKY ROOT-CAUSE DEFINITIVE (corrects the earlier ".MEG probe" attribution). The
guest-RAM oracle + a DOS find-first trace (dosbox-fist, [megtrace]) proved the
ORIGINAL also finds NO .MEG file (4/8/16/40.MEG all "not found") yet still has
[0x395c]=1 -- so the .MEG probe (89b0 @0x89e7-0x8a56, sets 395c only on a .MEG
FOUND) is NOT the setter. The real setter is at ext 0x7660, inside a Ghidra
DECOMPILE GAP (0x7490..0x76fd is not decompiled):
    7666 mov eax,0x6877 ; 766b mov [0x3958],eax   ; sky-render fn ptr = 0x6877
    7670 mov BYTE [0x395c],1                        ; 395c = 1 by DEFAULT
    7677 cmp BYTE [ebx+0xcc],0 ; 767e je 0x7695     ; ebx = [0xc93] (TCB)
    7680 mov eax,0x689a ; 7685 mov [0x3958],eax     ; else fn ptr = 0x689a
    768a mov al,[ebx+0xcc] ; 7690 mov [0x395c],al   ; and 395c = TCB[+0xcc]
The port never runs this (it is not in the decompile), so [0x395c] stays 0 and no
sky is built. FIX: reconstruct the 0x7660 sky-setup in the shim (set [0x3958] and
[0x395c] per TCB[+0xcc], faithfully to this asm) at the right map-load point, then
the already-faithful sky pipeline builds the sky (proven: a dummy 4.MEG that forces
395c=1 flips the port from 28 to 22887 sky-ish px). Next: find where 0x7660 is
entered (call site / fn-ptr table) to wire the reconstruction, and apply the
camera pitch/roll=384/256 seed; verify the windshield vs a dashboard-AE=0 burst.

Dispatch pinned: 0x7660 is reached as a SEPARATE op -- op-table (fist_image.bin
@0xcb3) entry 0x22 -> 0x10da (`call 0x7660; ret`), distinct from the map-load
entry 0x0c -> 0x10ca (`call 0x89b0`). So the sky-setup runs when the engine posts
that op; the port never runs it because 0x7660 lives in the decompile gap. FIX
options: (a) reconstruct FUN_0000_7660 in the shim (read its full asm from 0x7660
onward -- it also touches TCB[+0xcc]/[+0xd1] and sets the render-fn pointer
[0x3958]) and invoke it at map-load right after 89b0, or (b) wire the op-0x22
handler to it. Reconstruction is faithful (matches the pinned asm, like the 689a
reconstruction), not a band-aid. Then the sky builds and, with camera pitch/roll
=384/256 applied, verify the windshield vs the AE=0 burst reference.

## Comments

TWO fixes landed + matrix-verified (native 159/0 each) + native<->wasm byte-
identical (0 diff): (1) commit 1bce4c3 reconstructs the missing 0x7660 sky-setup
so [0x395c]=1 and 89b0 builds the 5.SKY source -> the windshield gains sky;
(2) commit 8340772 seeds the render camera pitch/roll to the oracle values 384/256.
The port went from terrain-top-to-bottom (28 sky-ish px, 75.5% vs original) to a
sky+horizon+terrain view (22791 sky-ish). Neither fix perturbs any dashboard crop
(all stay AE=0) and no OOM.

Remaining for bit-exactness: the sky currently over-produces vs the SETTLED burst
f04 (22791 vs 9304) but f04 is a later frame, not the spawn -- so a SAME-MOMENT
displayed-frame reference is needed to judge/tune. Next: extend dosbox-fist to dump
0xA0000 at VGA vsync right after the mission's first render (the displayed spawn
frame, not the 9200-entry off-screen buffer), then diff the port's op-0x24 spawn
against it; if it still differs, the residual is inside the sky-resample 689a
overlay extent or the terrain/sky horizon split.

THIRD bug isolated (the two fixes are correct but exposed it): with the camera +
[0x395c]=1 fixes applied, the port flipped from terrain-top-to-bottom to
SKY-top-to-bottom -- the whole windshield is (noisy) sky, no horizon, no terrain.
Visual: port = all sky; original = sky top / horizon / tan terrain bottom. Root
cause: the sky-resample 689a (shim reconstruction, native_main ~1222 "fills the
WHOLE 256x256 colormap tile @[0x3918]") overwrites the terrain colormap tile that
9200 samples (build/fist_ext.c 9200 samples DAT_0000_3918 + (Yhi<<8|Xhi)) -- so
once the sky source is built (395c=1), 9200 reads sky everywhere. The horizon /
sky-vs-terrain composite is not respected. This was MASKED before by 395c=0 (no
sky built, so the tile kept its bc9c/bdc4 terrain colours). Keep both landed fixes
(oracle-correct: original has 395c=1 + pitch/roll 384/256) and fix 689a: it must
fill a SEPARATE sky buffer / only the sky rows, not clobber the terrain tile 9200
reads. Decisive datum: dump the ORIGINAL's [0x3918] tile contents (terrain vs sky)
at the render via dosbox-fist (page-walk the RAM dump) to see what 9200 should
sample; and whether 689a/6980 writes a different buffer in the original.

Tile-composition pinned (dosbox-fist page-walk of the ORIGINAL [0x3918] tile ptr
0x44200 + the port's FIST_MTXDUMP): the map-load tile is 74% terrain (idx>=80,
bc9c/bdc4) but 689a OVERWRITES it with sky at render time. Post-689a the ORIGINAL
tile is 87% sky (idx<80) / 13% terrain -- 9200 then samples sky in the upper rays
and the retained terrain in the lower rays (the tan bottom third). The port's 689a
reconstruction fills ~the WHOLE tile with sky (the 13% terrain band is clobbered
too) -> 9200 reads sky everywhere. So the fix is bounded: make 689a (native_main
fist_ext_689a) leave the terrain rows the original leaves -- i.e. composite sky
only above the terrain horizon, matching the original's 87/13 split -- rather than
fill the full 256x256. Compare the port's post-689a tile to the original's 0x44200
tile row-by-row to find the exact fill extent 689a should use.

Chain fully resolved (isolation test: FIST_FORCE395C=0 keeps terrain-everywhere,
=1 gives sky-everywhere): the 0x7660 reconstruction correctly sets [0x3958]=0x689a
(sky-render fn ptr; oracle TCB[+0xcc]=1) and [0x395c]=1, and the render dispatches
indirectly through [0x3958] -> 689a (no direct `call 0x689a` in the asm; it runs
via the fn-ptr; 6980 is called from 0x3946). The remaining bug is purely the SHIM
689a reconstruction (native_main fist_ext_689a): it OVER-FILLS the colormap tile
[0x3918] with sky (~100%) whereas the ORIGINAL's post-689a tile is 87% sky / 13%
terrain (dosbox-fist page-walk of 0x44200). So 9200 reads sky in every ray instead
of sky-upper + terrain-lower. FIX (bounded, deep): reconstruct 689a faithfully from
the pinned asm (objdump 0x689a) so its perspective resample leaves the terrain band
the original leaves -- target = the saved /tmp oracle tile (element-wise), 87/13
split. The camera + sky-setup fixes stay (both oracle-correct); 689a fidelity is
the last windshield piece.

BREAKTHROUGH -- 6980 (the terrain overlay) is the missing piece. 689a fills the
tile with sky; 6980 (NovaLogic voxel raycaster, called from ext 0x3946) then
overlays terrain on the near rows -- but the port only runs 6980 under FIST_TILEFILL
(it needs paged-out seeds), so by default the sky is never overwritten by terrain.
Running FIST_TILEFILL (689a+6980) flips the port from sky-top-to-bottom to a PROPER
first-person voxel view: sky+clouds top, a horizon (~row 48), tan terrain bottom --
structurally correct, and the windshield min-diff drops 75% -> 56% vs the oracle
frames (saved: tools/oracle/samples/port_azer1_windshield_6980.png). So the render
pipeline is 689a(sky) -> 6980(terrain overlay), both needed; the port runs 689a by
default (via [0x3958]) but gates 6980.
FIX PATH: run 6980 in the default render. Its seeds are the ray-depth ramps
[0x3a24]/[0x3e24] (all-1 in fist_image.bin, paged-boot-filled constants -- the port
never builds them; banked in tools/oracle/samples/voxel6980_ramps.bin) plus a
contiguous HM+colormap buffer at [0x85bc]+0x100000. The 56% residual = the ramp/
buffer fidelity: find how the original boot-fills 3a24/3e24 (395e recomputes 4224/
4624 from them) and build them faithfully, then 6980 runs bit-exact by default. This
is the last windshield piece; camera + sky-setup + 689a are done.
