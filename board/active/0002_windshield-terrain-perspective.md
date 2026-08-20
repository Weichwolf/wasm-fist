
## Comments

DECISIVE port-only finding -- OVERTURNS the "oracle_bc9c misprovenanced / chain
faithful" retraction (commit 24f1997). Re-diffed the bc9c matrix diagonals
empirically:
  - The misprovenance argument was WRONG on its premise: it claimed the sorted
    pipeline MUST yield a linear diagonal M[ch][ch]=ch and the oracle's "80..83
    plateau" was impossible. Empirically NEITHER diagonal is linear (port 0/176,
    oracle 13/176), and the oracle does NOT plateau -- it rises cleanly 80->249.
    So the "impossible plateau" basis for discarding the oracle is a
    mischaracterization.
  - The REAL defect is in the PORT and is port-only (no oracle needed): the port's
    bc9c matrix has ENTIRE ROWS ch=190..255 ALL ZERO (nonzero=0/256 for every row
    >=190; sustained-zero from exactly ch=190 to 255 = 66 rows). The matrix is
    built only for ch=80..189. rows 80..189 are populated + rising (diag 102..241);
    190..255 are blank.
  - This is intrinsically wrong for a terrain colormap and MATCHES the visible
    bug: M[ch][*]=0 for the brightest quarter of terrain indices -> those colours
    render as palette index 0 (black) -> "port renders BLACK where original renders
    GRAY", terrain mis-coloured. The oracle diagonal stays high there (200->186,
    220->210, 255->249), i.e. the ORIGINAL populates rows 190..255.
Two port-only hypotheses for the 190-cutoff:
  (A) bc9c's build loop upper bound is ~190 instead of 256 (a ported loop-bound
      defect) -- note ac60=0xff=255 is the intended upper bound, so a stop at 190
      would be a real truncation.
  (B) [5598][190..255] are themselves zero (the C32.KLC->532.pal decode truncates
      the palette at 190) -> bc9c legitimately emits zero rows. This REVIVES the
      [5598]-decode lead with a SPECIFIC testable prediction: [5598][190..255]==0.
NEXT: dump the port's live [5598] at bc9c-entry (FIST_BC9CENTRY) and check entries
190..255. Zero => decode truncation (fix the C32.KLC/532.pal decode); nonzero =>
bc9c loop-bound bug (fix the build range). Either way the cutoff at ch=190 is the
concrete, port-only handle -- no disputed oracle capture required.

RETRACTION (same session, doctrine: correct without ego) -- the "190-cutoff =
port defect" finding above is ITSELF a capture-window artifact, proven decisively:
  - port_bc9c_matrix.bin's nonzero data occupies EXACTLY file bytes 0..0xbdff =
    48640 = 65536 - 0x4200 (= 0xbe00). The zero region is EXACTLY the last 0x4200
    bytes. Sustained-zero starts at file-row 190 = 256 - (0x4200>>8) = 256 - 66.
  - So the port matrix was dumped from the +0x4200 WINDOW (base+0x4200, per the
    BC90DUMP note): the read ran 65536 bytes from real_base+0x4200, so the last
    0x4200 bytes fell PAST the 64KB matrix buffer and read as zero. The "black
    rows 190..255" are the window overrun, NOT an unbuilt matrix region. bc9c's
    loop is a byte-counter upper-triangle fill (ch from bc90.low, cl from ch, both
    wrapping at 0) that DOES cover the full range; nothing truncates at 190.
META-CONCLUSION (the real blocker, explains the whole thread's oscillation): every
port-vs-oracle bc9c-MATRIX conclusion in this investigation is CONFOUNDED by
mismatched capture windows -- the port dump is +0x4200-windowed (sheared: file
offset = real offset - 0x4200, so file-row R = real-row R+66, and the last 0x4200
bytes are past-buffer zeros), while the oracle sample's window/offset is
undocumented. The recurring pattern -- "misprovenanced plateau", "0% heightmap",
"190-cutoff" -- is the SAME failure each time: comparing two dumps taken at
different, unverified offsets/stages. No offset-based matrix diff is trustworthy.
CORRECTED NEXT (methodology fix, not a code hunt):
  (1) Re-dump the PORT bc9c matrix at the RAW 64KB base (NOT the +0x4200 window)
      so file offset == real matrix offset == ch*256+cl -- a clean 256x256.
  (2) Pin the oracle sample's capture offset, or recapture it at the same raw base
      + same map/tick; only then is a diagonal diff valid.
  (3) BETTER -- sidestep the mis-windowed intermediates entirely and anchor on the
      PROVENANCE-CLEAN framebuffer: oracle_azer1_windshield_dashAE0.png (real
      DOSBox, known dashboard-AE0 state) vs port_azer1_windshield_6980.png (75.5%
      diff). Work backward from that clean ground truth, not from these dumps.
The camera + sky-setup fixes remain landed + matrix-verified independent of all this.

Progress (raw-base dump built + a NEW stage discipline found). Added FIST_RAWMTX
to native_main.c (fires in the FIST_MISSFB_PROBE block): dumps 65536 B from the
true matrix base (bc90 & 0xffff0000), file offset ch*256+cl == matrix entry.
Reproducible via the run_mission MC_MOUSE drive + FIST_MISSFB_PROBE=1.
  - CONFIRMS the retraction: at bc90=0x82e0000 (64KB-aligned) the raw diagonal is
    M[200]=102, M[255]=103 (NONZERO at high ch) -- the earlier "rows 190..255 zero"
    was purely the +0x4200 tile-window overrun, not an unbuilt region.
  - BUT a port-only internal-consistency check EXPOSES a stage error: the dumped
    matrix is 98% ASYMMETRIC (269/15400 symmetric in 80..255). bc9c's blend is
    provably symmetric in (ch,cl) -- ac68=cl.R+ch.R, ac69=cl.G+ch.G, ac6a=cl.B+ch.B
    -- and it dual-writes M[ch][cl]=M[cl][ch], so a TRUE bc9c matrix MUST be
    symmetric. 98% asymmetry => render-time [bc90] does NOT point at the clean bc9c
    matrix: by op-0x24 render time [bc90] has become the bdc4-UPSAMPLED tile (or a
    reused buffer), not the symmetric blend LUT at bc9c-exit.
CORRECTED NEXT (stage discipline, extends the methodology fix): capture the matrix
at bc9c-EXIT, not at render time. Add a one-shot dump at FUN_0000_bc9c's return
(mirror FIST_BC9CENTRY, which already hooks bc9c entry) writing 65536 B from
bc90&0xffff0000 -- verify THAT dump is symmetric (the correctness gate), then its
diagonal is the clean palette-encoding to compare against a provenance-matched
oracle. Symmetry is now the port-only correctness invariant for any bc9c capture.

DECISIVE port-only CLEARANCE of bc9c (stage-correct capture, oracle-independent).
Added FIST_BC9CEXIT (dump at bc9c-EXIT before bd0e/bd62; snippet in
tools/oracle/diag/bc9c_exit_dump.txt since build/ is ephemeral). The AZER1
bc9c-exit matrix is:
  - 100% SYMMETRIC (15400/15400 in 80..255) -- passes the correctness gate, so it
    IS the clean bc9c blend matrix (render-time [bc90] was the bdc4 tile: 98% asym).
  - diagonal EXACTLY LINEAR M[ch][ch]=ch for all 175 terrain indices (80->80 ...
    255->255), 0 zeros. This is the mathematical signature of a correct sorted-
    palette blend: [5598]==[5260] => self-blend matches self => M[ch][ch]=ch.
  - combined with ac70 verified (0/40960) and [5260] oracle-proven byte-exact
    (528/528), the off-diagonal M[ch][cl]=ac70(blend(ch,cl)) is faithful by
    construction. => bc9c is FAITHFUL, fed correct inputs, produces the correct
    matrix. CLEARED as a suspect, port-only, no oracle needed.
This also resolves the whole misprovenance saga: oracle_bc9c_matrix_blockB's
non-linear diagonal (80,83,97,...,249) is a WINDOWED capture (base+0x4200), exactly
like the render-time / port_bc9c_matrix.bin dumps -- NOT the raw symmetric matrix.
The saga was capture-window confusion end to end; the underlying port bc9c was
faithful all along.
VOXEL DEFECT NARROWS DOWNSTREAM of bc9c: the tile [0x3918]=base+0x4200 is built by
bd0e -> bd62 -> bdc4 (upsample) FROM this faithful matrix. NEXT SUSPECT: bd0e/bd62
(what they do to the matrix) + bdc4's tile upsample + 9200's indexing into the tile
+ the heightmap [0x85bc]. Clean sample saved: tools/oracle/samples/
port_bc9c_exit_symmetric.bin (the faithful reference for anything downstream).

bd0e RETRACTION + infra win (asm-verified, doctrine: code is the truth). A strong
port-only hypothesis -- bd0e's shade table is 100% CONSTANT down the shade axis
(176/176 terrain colors identical across all 32 shade levels; T[shade][color] =
[5598][color].R, 6-bit: 80->1,160->32,255->63) -- looked like a lost ac70-return
(the port stores (char)uVar7 instead of the match). DISPROVEN by the original asm:
  bd29: mov (%eax),%bx        ; bl = [5598+color*3] = source R
  bd4e: call 0xac70           ; ac70 -> match in AL
  bd54: mov %bl,(%ecx)        ; stores BL (source R), NOT AL -- ac70's return IS discarded
The ORIGINAL itself stores the source R and discards ac70's match. The port's
`*puVar4 = (char)uVar7` (= bl) matches bd54 EXACTLY -> bd0e is FAITHFUL, the shade-
constant table is correct, NOT a bug. bd0e CLEARED as a suspect.
INFRA WIN (unblocks the rest): the extender image fist_ext.c decompiles from is
re_out/fist_image.bin (FIST.RUN, x86:LE:32-bit FLAT base 0, 0xbf90 bytes). Any ext
function FUN_0000_XXXX is at raw offset 0xXXXX, disassembled with `objdump -b binary
-m i386 --start-address=0xXXXX ... re_out/fist_image.bin`. (fist_dat_image.bin is the
16-bit ENGINE -- a DIFFERENT image; that mismatch is why earlier objdumps at 0xbd0e
showed unrelated code.) Every downstream voxel function (bd62, bdc4 tile upsample,
9200 sampler, 82b8/8120/9200 render) is now DIRECTLY asm-verifiable. NEXT: asm-verify
bdc4 (tile [0x3918] upsample from the faithful matrix) and 9200's indexing -- the
remaining downstream suspects, now with the asm in hand.

bdc4 horizontal loop asm-verified FAITHFUL + a strategic reframe. Original bdc4
(fist_image.bin @0xbdc4, 32-bit) horizontal 2x-upsample:
  bdf3 mov (%esi),%al ; bdf5 mov %al,(%edi)        ; dst[0]=cur source pixel
  bdf7 mov (%eax),%ah ; bdf9 mov %ah,0x1(%edi)     ; dst[1]=M[prev][cur], eax=matrix_base|(prev<<8)|cur
i.e. it inserts the bc9c blend M[prev][cur] between adjacent source pixels. The
port's `*puVar15=uVar1; puVar16[-1]=*(CONCAT31(iVar6>>8,uVar1))` resolves to exactly
dst[0]=cur, dst[1]=M[prev][cur] -- MATCHES. bdc4's horizontal pass is faithful,
interpolating via the (already-faithful) bc9c matrix. (The vertical pass be05+ is
long; not exhaustively hand-diffed, but the primitive is the same M[a][b] blend.)
STRATEGIC REFRAME: the voxel render is a MIX of (1) ported decompile functions
[bc9c, bd0e, bdc4 -- all asm-verified faithful so far] and (2) SHIM RECONSTRUCTIONS
of the extender's op-0x18 setup, hand-written in native_main.c (block-A preload,
camera seed +0x3a/+0x3c, sky-setup 3958/395c, the 84c0 task-setup allocator gate).
If the pure ported functions keep verifying faithful while the windshield stays
75.5% wrong, the defect likely lives in the SHIM RECONSTRUCTIONS or an unchecked
function -- NOT the blend/upsample math. NEXT: (a) finish 9200 (sampler indexing)
asm-diff; (b) audit the native_main.c op-0x18 voxel reconstructions against what the
original extender setup actually does (now asm-readable via fist_image.bin) -- the
camera/sky/preload seeds are the hand-written surface most likely to diverge.

DAC cleared + the real blocker named (pose-confound). Two findings via direct
port capture (FIST_MISSFB_MERGEPAL, AZER1 spawn) vs oracle_azer1_windshield_dashAE0:
  1. The terrain DAC is NOT the bug. [5598] (raw) and [5260] (sorted, via
     FIST_MISSFB_PAL5260) give IDENTICAL terrain output (21.9% multiset overlap,
     32/85 shared colours, both). Reason: [5598]@bc9c is already luma-SORTED
     (0 inversions), so [5260]==[5598] -- the 9f10 sort is a no-op on this palette,
     and the DAC choice cannot be the divergence. (Supersedes the "5260 is the
     faithful DAC / 5598 regresses" tension -- moot, they're equal here.)
  2. FUNDAMENTAL CONFOUND for windshield verification: the terrain band overlap
     (21.9%; port has MORE colours, 129 vs oracle 85) is NOT trustworthy as a
     render-fidelity metric because the windshield terrain depends on the exact
     CAMERA POSE (position/heading/pitch), and dashboard-AE0 matching does NOT
     imply pose matching (the dashboard is camera-independent chrome). The port
     and oracle may simply be at different camera poses -> the terrain differs for
     that reason alone, not necessarily a render bug.
This reframes 0002's verification requirement: a windshield comparison is only
valid at a POSE-MATCHED capture (identical camera position+heading+pitch on both
sides). The burst tool matches the dashboard, not the pose. CORRECTED NEXT: capture
BOTH at the deterministic SPAWN pose -- the first render frame before any AI/physics
drift, where the camera sits at the fixed spawn point -- OR read the oracle's camera
pose (TCB +2c/30/34 pos, +38/3a/3c ang from a guest-RAM capture) and force the port
to that exact pose before the windshield diff. Only a pose-matched terrain diff can
tell a render bug from a pose difference. (bc9c/bd0e/bdc4 remain asm-verified faithful;
this says the current windshield METRIC is confounded, not that those are wrong.)

MAJOR REFRAME -- the windshield defect is the CAMERA, not the tile-build (confirms
the pose-confound + the shim-reconstruction hypothesis, now concrete). The shim
DOCUMENTS it (native_main.c:1350): "the LIVE mission CAMERA is degenerate (TCB Y off
~40x, alt ~9x -- residual #2)". Read the port's live spawn pose (FIST_MISSFB_PROBE)
vs the KNOWN oracle pose (hardcoded in the FIST_R3D2_V18 harness / docs/
oracle_terrain_writer.md: X=609696 Y=1112229 alt=29184 head=19745 pitch=0 roll=128
foc=256 detail=0):
    field        port(live)   oracle    divergence
    alt (+34)    12800        29184     2.3x too SMALL
    pitch(+3a)   384          0         hand-seeded WRONG
    roll (+3c)   256          128       hand-seeded WRONG (2x)
    heading(+38) 26729        19745     differs
    X/Y(+2c/30)  583982/1142557 609696/1112229  ~3% (close)
So the pure render funcs (bc9c/bd0e/bdc4/6980/9200 -- asm-verified/likely faithful)
are fed a WRONG CAMERA: the shim HAND-SEEDS the angles (pitch=384/roll=256) as a
band-aid, but the oracle has pitch=0/roll=128, and alt is 2.3x off. A wrong camera
makes 9200 walk the wrong part of the terrain at the wrong altitude/attitude ->
the whole windshield diverges regardless of render fidelity. This is THE windshield
defect and it lives in the SHIM camera reconstruction, exactly as the reframe predicted.
DOCTRINE FIX (no band-aids): make the camera-setup faithful -- the original computes
the pose via its flight model + 85d0 camera; the port hand-seeds it because that
path isn't running/ported. NEXT: (a) trace where the port sets TCB +2c/30/34/38/3a/3c
(the seed site ~1516 + any engine writes) and why alt is 2.3x + angles are hand-set;
(b) find the original's camera-setup (flight model / 85d0 inputs) via fist_image.bin
asm and port it so the pose is COMPUTED = oracle, removing the hand-seed. Then re-run
the (existing) FIST_R3D2_V18 pose-matched harness -- note it currently doesn't
trigger under the MC_MOUSE drive (op-0x54 gate); its trigger needs repair to validate
render fidelity at the oracle pose against oracle_9200_framematched_pass08.idx.bin /
oracle_mission_spawn_framematched_idx.bin (pose-matched oracle FB samples that EXIST).

BREAKTHROUGH -- the REAL residual isolated (pose-matched, non-confounded), after
clearing SIX capture/reconstruction confounds this session. First, a CAMERA
RETRACTION (last iteration trusted the wrong oracle): the port's LIVE spawn pose
MATCHES the GENUINE dosbox-fist guest-RAM capture oracle_azer1_tcb_camera.txt
(r92cam-now at 9200-render: X=583982 Y=1142557 alt=12800 head=26729 pitch=384
roll=256 foc=256) -- EXACTLY (X/Y/alt/head/pitch/roll all match). The camera is
CORRECT, not degenerate; the "residual #2 / Y off 40x" comment is STALE, and the
docs/oracle_terrain_writer.md camera (X=609696/alt=29184/pitch=0/roll=128, hardcoded
in FIST_R3D2_V18) is a DIFFERENT/misprovenanced capture -- the 6th capture-provenance
confound of the session.
With the camera confirmed correct, a DAC-independent INDEX comparison of the port's
live windshield FB vs the pose-matched oracle 9200 render (oracle_9200_framematched_
pass08.idx.bin) gives the true residual:
  - SKY (rows 0-4): BIT-EXACT (top indices identical 0:452/32:350/3:218/33:184).
  - TERRAIN (rows 5-85): both render terrain (port 23172, oracle 22692 px >=80),
    but only 10.6% index match. Port mean idx=129, oracle mean=184 -> terrain is
    +55 DARKER, with a BROAD spread (-80..+150), NOT a constant offset -> per-pixel
    the port samples DIFFERENT tile locations, not a uniform lighting shift.
ROOT (self-documented + verified): the port NEVER BUILDS the ray tables [0x3a24]/
[0x3e24] (shim comment native_main.c:1400 "port never builds them"; live values are
ray3a24[0..3]=1/1/1/1, ray3e24[0]=1 -- all 1s, UNBUILT). 9200's per-column texel
walk uses 3a24 (x DAT_90c0) + 3e24 as the "detail base curve" increment (consumed at
build/fist_ext.c:4367-4368); with degenerate all-1 tables the walk samples the wrong
terrain -> the +55-darker broad-spread residual. The tile [0x3918] itself IS built
(nz=65536, distinct=175). So THE windshield bug is the UNBUILT RAY TABLES, not the
camera / tile / palette / matrix.
NEXT (concrete fix path): find the original's 3a24/3e24 "detail base curve" builder
(only CONSUMERS appear in build/fist_ext.c:4367 -> the builder is unported or in a
Ghidra gap) via fist_image.bin asm -- search for stores to 0x3a24/0x3e24; port it so
the ray tables are built faithfully; re-run the index comparison (target: terrain
match >> 10.6%, ideally bit-exact like the sky). This is the first REAL, actionable
voxel defect of the session -- everything upstream (camera/sky/matrix/tile/DAC) is
verified faithful.

RAY-TABLE RETRACTION + refined root (causal test). Injecting the oracle ray tables
(scratch/oracle/oracle_3a24.bin/oracle_3e24.bin -- confirmed a REAL curve, distinct
=251, quadratic, vs the port/static-image all-1s) into the port's live render via
FIST_ISO_RAY3A24/RAY3E24 changed the terrain match by 0.0% (still 10.6%). So the ray
tables are IRRELEVANT to the default render: their only consumer 395e is called from
6980 (asm 0x6992), and 6980 is NOT in the default chain (8deb->85d0->8120->9200) --
it runs only behind FIST_TILEFILL. The "port never builds the ray curve" observation
is TRUE but not the live-render bug. (The static image ships all-1s at 0x3a24/0x3e24;
the real curve is built at runtime by a writer NOT in the ext image -- engine-side --
but that only matters on the 6980 path.)
REFINED ROOT: the port's default op-0x24 render walks the STALE map-load tile
[0x3918] (the bc9c/bdc4 blend matrix) with 8120+9200, and NEVER runs 6980 (the
NovaLogic voxel raycaster) to build a fresh terrain tile per-frame. The shim's
FIST_TILEFILL seam exists precisely to inject a 6980 call "so 9200 walks a
freshly-built terrain tile instead of the STALE map-load tile" (native_main.c:1560).
So the likely live-render defect is the MISSING per-frame 6980 tile-build: 9200
samples a blend-matrix tile instead of a raycast terrain tile -> +55-darker,
broad-spread terrain, 10.6% match. (Camera correct + sky bit-exact still hold.)
NEXT: asm-verify whether the ORIGINAL op-0x24 render path calls 6980 per-frame
before 9200 (disassemble the 8deb/85d0/render dispatch in fist_image.bin; check the
call graph into 6980 @0x6980 and its caller 82b8/8120 region), and identify what the
original's 9200 actually samples. If the original rebuilds the tile via 6980 each
frame and the port skips it, port that call (with 6980's real inputs, not the
FIST_TILEFILL reconstruction) -- that is the concrete fix.

RENDER ARCHITECTURE mapped (asm call-graph, fist_image.bin). The original render is
TWO separate functions, not one:
  - TILE-BUILD: FUN_0000_3931 (called by 0x10e5): 85d0(camera) -> if [0x395d]==0:
    call [0x3958](sky fn ptr, =0x6877) THEN call 0x6980 (voxel raycaster, builds the
    terrain tile); else ([0x395d]!=0): call 0x686f + 0x6c00 (alt path). 6980 has
    EXACTLY ONE caller: 0x3946, inside this function.
  - TILE-SAMPLE: FUN_0000_82b8 (op-0x24 dispatch, called indirectly via the far-reloc
    vector): 8120(0x82c0, projection) -> 9200(0x82c5, texel walk into fb). 9200 has
    EXACTLY ONE caller: 0x82c5, inside 82b8.
So the original BUILDS the terrain tile via 6980 (in 3931) and SAMPLES it via 9200
(in 82b8) as two separate steps. The port's default op-0x24 render runs only the
82b8 sample chain (8deb->85d0->8120->9200); 6980 runs only behind FIST_TILEFILL. If
the port never runs FUN_0000_3931's 6980-build per frame, 9200 samples a STALE /
wrong tile -> the +55-darker terrain. Static [0x395d]=1 routes 3931 AWAY from 6980
(to 686f/6c00); the runtime [0x395d] value decides whether 6980 or the alt path runs.
NEXT (precise diagnostics): (1) does the port invoke FUN_0000_3931 (via 0x10e5) in
the mission loop? (2) what is [0x395d] at render time (0 -> 6980 terrain path, !=0 ->
686f/6c00)? (3) what are 686f/6c00 -- the alt render the [0x395d]!=0 path runs?
Instrument the port for 3931-entry + [0x395d], and asm-read 0x10e5 to see when 3931
fires. This pins whether the fix is "call 3931/6980 per frame" or "set [0x395d]
correctly so the existing dispatch reaches 6980".

FULL RENDER ARCHITECTURE (definitive, native_main.c:2611-2632 RECON + asm call-graph).
The per-frame 3D voxel render is EXTENDER-SIDE and MULTI-OP: the engine (FIST.DAT)
only POSTS display objects + camera/present ops; the extender does the projection via
an op-dispatch table (fist_image.bin:0xcb3 + op -> trampoline -> render fn):
    op 0x08 -> 8df0/FUN_3931 -> 6980          tile-BUILD (voxel raycaster)
    op 0x0c -> 78f0 -> 85d0 + 93c0            colormap perspective texture-map ([0x85b8])
    op 0x10 -> 7940 -> 8fa0                   rotated variant
    op 0x24 -> 82b8 -> 8120 + 9200            tile-SAMPLE (dominant fb writer, [0x3918])
    op 0x44 -> 7660                           SKY  (port: BIT-EXACT)
Port status per op: SKY (0x44) + SAMPLE (0x24) work (sky bit-exact; terrain painted
but +55 darker). The terrain divergence is in the TILE-BUILD half (op 0x08->6980
and/or op 0x0c->93c0), NOT the sample. A prior-session RECON already found the op-0x0c
path's colormap [0x85b8] holds only {0,4} across its 4 MB -- i.e. the C32.KLC colormap
DATA is not delivered by the extender map-load (89b0/643c), even though the LOD build
completes structurally. So the deepest gate is the COLORMAP DATA PATH feeding the
tile-build, upstream of 6980/93c0.
This UNIFIES the session's findings: camera correct, sky bit-exact, bc9c/bd0e/bdc4
(the blend LUT) faithful, 9200 sample faithful -- but the terrain COLORMAP that the
build ops consume (C32.KLC -> [0x85b8]/[0x3918] terrain texels) is not faithfully
delivered by the map-load. NEXT: trace the C32.KLC colormap decode in the extender
map-load (89b0 -> 643c ...) via fist_image.bin asm; verify what [0x85b8]/[0x3918]
should hold vs the port's {0,4}/blend-matrix; that data-delivery is the concrete
windshield fix, and it is asm-verifiable end to end.

DEFINITIVE ROOT (this unifies everything): the extender per-frame RENDER OP-DISPATCH
is NOT WIRED in the port -- it is a LOG-AND-RETURN STUB. The port's ext op-service is
manual `if(op==X)` handlers for 0x4c/0/0x80/0x18(map-load)/0x78, plus ENV-GATED
diagnostic handlers for 0x0c (FIST_R3D2/R3D) and 0x24 (FIST_MISSFB). Every render op
falls through (native_main.c:2880) to `fprintf("[ext] service op..."); return 0` when
its diagnostic env var is absent:
    op 0x08 (tile-build -> 3931 -> 6980)   : NOT handled at all -> stub
    op 0x0c (colormap   -> 78f0 -> 93c0)   : only under FIST_R3D2/R3D -> else stub
    op 0x24 (sample     -> 82b8 -> 9200)   : only under FIST_MISSFB   -> else stub
So in a NORMAL mission (no diagnostic env), the port renders NO voxel terrain -- the
+55-darker terrain I measured was the FIST_MISSFB shim RECONSTRUCTION, not the
engine's real op flow. The whole windshield surface is unimplemented op-dispatch,
scaffolded by diagnostic seams; this is why the 159-flow matrix does not (and cannot
yet) assert windshield fidelity. DOCTRINE FIX (no stubs): wire the extender's PM-gate
op-table dispatch -- the engine posts op N, the PM gate FUN_0000_0f30 does
`movzx ebx,bx; mov ebx,[ebx+0xcb3]; call *ebx` (op-table @ fist_image.bin:0xcb3),
trampolining to the render fn (0x08->3931/6980, 0x0c->78f0/93c0, 0x24->82b8/9200,
0x44->7660). Faithfully running that dispatch for every posted op -- instead of
log-and-return -- makes the real per-frame render run, with 6980 building the tile
and 9200 sampling it, exactly as the original. That is the concrete, doctrine-correct
windshield implementation; the session's upstream verifications (bc9c/bdc4 faithful,
camera correct, sky bit-exact) mean the pieces are ready to be driven by the real
dispatch. NEXT: confirm FUN_0000_0f30 (op-table dispatcher) is available/portable in
the shim, read the op-table @0xcb3 entries + their trampolines, and wire op-service to
dispatch via it (patch or shim) instead of the log-and-return fallthrough.

OP-TABLE fully resolved (byte-offset table @fist_image.bin:0xcb3, entry = dword at
0xcb3+op; each trampoline is `call fn(; call fn2); ret`):
    op 0x08 -> 0x10e0 -> call 0x8df0 ; call 0x3931     tile-build (viewport + camera+6980)
    op 0x0c -> 0x10eb -> call 0x78f0                   colormap perspective
    op 0x10 -> 0x10f7 -> call 0x7940                   rotated variant
    op 0x18 -> 0x10ca -> 0x89b0                        MAP-LOAD (confirmed)
    op 0x24 -> 0x82c0 -> call 0x8120 (then 9200)       sample (mid-82b8)
    op 0x44 -> 0x10da -> call 0x7660                   sky
    op 0x60 -> 0x10f1 -> call 0x8650                   (camera-Z terrain-follow)
CORRECTED op-0x08 structure: the trampoline calls TWO fns -- 8df0 then 3931. 8df0 is
VIEWPORT/PROJECTION setup (sets 90ec fb-ptr, 90f0/90f4/90f8 viewport dims from the
TCB rect +0x16/+0x18/+0x1a/+0x1c, 9114 horizon table from TCB+0xcd detail) -- NOT the
tile-build. 3931 IS the tile-build: 85d0(camera); if [0x395d]==0 { [0x3958](sky);
6980() } else { 686f(); 6c00() }. So op 0x08 = 8df0(viewport) + 3931(camera+sky+6980).
To wire op 0x08 faithfully the tile-build 6980 needs: (a) [0xc93]=mission TCB, (b) the
TCB VIEWPORT RECT +0x16..+0x1c populated (prior RECON: "never populated, all 0" -- the
viewport-dim frontier; 8df0 would set dims=0 otherwise), (c) [0x395d]==0 to reach 6980.
So the windshield implementation decomposes into: [1] wire the op-table dispatch
(f30/0xcb3 -> trampolines) replacing log-and-return; [2] populate the TCB viewport rect
(engine-side FUN_0000_ddff writes TCB+0x1e/+0x22 from word[0x156a] rect [+6/+8/+a/+c],
per native_main.c:2862 -- itself needs that rect populated); [3] deliver the C32.KLC
colormap so 6980's input is terrain texels not {0,4}. Each is asm-verifiable. NEXT:
attempt [1] as a DIAGNOSTIC first -- at op 0x08, call m_ext 8df0+3931 with [0xc93]=TCB
+ [0x395d]=0 + a forced 320x200 viewport, check crash-free + whether [0x3918] changes
and the terrain index-match improves; that isolates whether the tile-build path (given
viewport+TCB) produces the right tile, before committing the full dispatch wiring.

CAUSAL CONFIRMATION (forward progress, not a retraction): running the tile-BUILD
(6980) before the 9200 sample -- via FIST_TILEFILL -- lifts the terrain index match
from 10.6% -> 33.6% (3x) vs oracle_9200_framematched_pass08.idx.bin. This CONFIRMS
the decomposition causally:
  [1] wire the tile-build (6980):  10.6% -> 33.6%  (+23%, CONFIRMED it matters)
  [3] deliver the C32.KLC colormap: the remaining ~66% gap
FIST_TILEFILL still uses RECONSTRUCTED 6980 inputs (seeded ramps 3a24/3e24 from
voxel6980_ramps.bin + a contiguous HM+colormap buffer at HM_base+0x100000), so the
residual is the fidelity of those inputs -- chiefly the colormap source [0x85b8]+
0x100000, which the port's Route-1 map-load delivers as {0,4} instead of C32.KLC
terrain texels. So the confirmed next target is the COLORMAP DELIVERY: the extender
maps the terrain colormap at HM_base+0x100000, but the port allocs [0x85bc]/[0x85b8]
separately AND the C32.KLC colour data isn't decoded into it. NEXT: trace the
extender map-load's C32.KLC -> colormap decode (89b0/643c) in fist_image.bin asm,
find where the terrain texels should land at [0x85b8](+0x100000), and why the port
gets {0,4}; deliver it faithfully, then re-measure (target: 33.6% -> bit-exact like
the sky). The tile-build wiring itself is now causally proven worth landing.
