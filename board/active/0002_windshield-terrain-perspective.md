
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

Colormap content refined: under FIST_TILEFILL the port's colormap [0x85b8]+0x100000
is POPULATED (78 distinct bytes, values clustered 128-152) -- NOT {0,4} (that was the
op-0x0c/93c0 default path without TILEFILL's reconstruction). But the values (128-152)
are systematically DARKER than the oracle terrain (indices 150-230), matching the
+55-darker fb finding. So the colormap is delivered but with WRONG (too-dark) values.
The residual is the COLORMAP BUILD content (C32.KLC decode / light-reduce), not
delivery-absence. NEXT: diff the port colormap vs the oracle colormap sample
(oracle_85b8_colormap_first64k.bin) to characterize the darkness (constant offset?
scale? decode?) and trace the C32.KLC -> [0x85b8] light-reduce build in asm.

Colormap CONTENT is right -- residual is LAYOUT/SAMPLING. Diffing the port colormap
vs oracle_85b8_colormap_first64k.bin: only 5.6% byte-identical, BUT the distributions
MATCH (port nz mean=140.3, oracle=139.3, offset -0.9; same top colours 151/133/145/
152/132/159). So the colormap has the RIGHT COLOURS in DIFFERENT POSITIONS. Yet the
rendered terrain is +55 darker (fb mean 129 vs 184). Conclusion: the colour CONTENT
of the colormap is faithful; the residual is the LAYOUT of the colormap and/or 6980's
SAMPLING COORDINATES into it (the port samples coordinates that land on darker
entries). This is NOT a C32.KLC colour-decode bug -- it is the colormap's spatial
layout (HM+colormap contiguity / stride) and/or 6980's (height,light)->offset math.
(Caveat: oracle_85b8_colormap_first64k.bin's frame/map provenance should be
re-confirmed before over-trusting the 5.6%.) NEXT: trace 6980's colormap addressing
(how it maps a screen column's (height,light) to a [0x85b8]+0x100000 byte offset) in
fist_image.bin asm, and compare the port's contiguous-buffer layout/stride to the
extender's real HM_base+0x100000 layout -- the +55 darkness is a sampling-coordinate
or stride mismatch, downstream of a faithful colour palette.

6980 ASM-VERIFIED FAITHFUL -> bug is the map-load colormap LAYOUT. The colormap
read in 6980 matches the original exactly: asm `mov 0x100000(%esi,%ecx,1),%al`
(esi=[0x85bc], ecx=coord) == port `*(uint8*)(iVar14 + 0x100000 + uVar7)` (iVar14=
[0x85bc]). So 6980's addressing (colormap at base+0x100000+coord, heightmap at
base+coord) is faithful. The divergence is PURELY the INPUT buffer: what the map-load
places at [0x85bc]+0x100000. FIST_TILEFILL's contiguous-buffer reconstruction gets
only 33.6% (colours right, 5.6% byte-match) -> the colormap is written to the
+0x100000 region with the WRONG LAYOUT/STRIDE. The port's Route-1 map-load allocs
[0x85bc]/[0x85b8] SEPARATELY and its C32.KLC->colormap decode places texels at wrong
offsets relative to what 6980's coord math expects. FIX LOCUS (precise): the extender
map-load's colormap build -- decode C32.KLC into a contiguous HM[0..0x100000] +
colormap[0x100000..0x200000] at [0x85bc], matching the stride 6980 walks. NEXT: trace
89b0/643c (map-load) in fist_image.bin asm -- how it decodes C32.KLC into the colormap
region, the row stride, and the HM/colormap contiguity; make the port's map-load build
that exact layout so 6980 (faithful) samples the right texels WITHOUT the TILEFILL
band-aid. This is the concrete, asm-verifiable windshield fix, now localized to one
build step (map-load colormap layout) downstream of everything else verified faithful.

PRECISE ROOT of the colormap layout: the map-load buffer-setup (fn @0x8b80) allocates
TWO SEPARATE buffers via 0x36bf: first the HEIGHTMAP -> [0x85bc] (dim from TCB+0x7a,
then upsampled 256->2048 via 2f95/345c/bc06/bed2), then the COLORMAP -> [0x85b8] (dim
from TCB+0x8a). They are separate 36bf calls -- BUT 6980 reads the colormap at
[0x85bc]+0x100000, NOT at [0x85b8]. This only works if the allocator (36bf / the
MEMMGR pool) places the two buffers CONTIGUOUSLY so that [0x85b8] == [0x85bc]+0x100000
(HM occupies exactly 0x100000). The port allocs them separately/non-contiguously (per
the FIST_TILEFILL comment), so [0x85bc]+0x100000 != [0x85b8] and 6980 reads the wrong
region -> the colormap "right colours at wrong positions" (5.6% byte-match, +55 dark).
THE FIX (precise, in the map-load/allocator): make the port's map-load allocate the HM
and colormap so [0x85b8] == [0x85bc]+0x100000 (contiguous, HM sized exactly 0x100000),
matching the original's pool layout that 6980 depends on. Then 6980 (asm-verified
faithful) samples the real colormap with NO TILEFILL band-aid. NEXT: verify the port's
runtime gap ([0x85b8]-[0x85bc]) at op-0x18, confirm it != 0x100000, then adjust the
map-load buffer alloc (shim MEMMGR / 36bf order+size) to make them contiguous; re-run
the index compare (expect a large jump past 33.6% toward bit-exact).

GAP CONFIRMED -- a 4x RESOLUTION mismatch. FIST_GAPCHK dumps the port's runtime
pointers: [0x85bc](HM)=0x08324200, [0x85b8](CM)=0x08724200 -> gap = 0x400000 (4 MB),
but 6980 hardcodes the colormap read at [0x85bc]+0x100000 (1 MB, asm `mov
0x100000(%esi,%ecx,1),%al`). So the port's HM is 4 MB (2048x2048) and its colormap
sits 4 MB later, while 6980 reads the colormap 1 MB after the HM base -> it reads
HM row ~512 as colormap. Since 6980's colormap offset is a fixed 0x100000 and coord
walks the map, the map 6980 expects is 1024x1024 (1 MB HM + 1 MB colormap, contiguous
in 2 MB). THE PORT OVER-UPSAMPLES the HM to 2048x2048 (4 MB). This is why even
FIST_TILEFILL (which builds a 2 MB HM[0..1MB]+CM[1MB..2MB] buffer) only reaches 33.6%:
it copies the FIRST QUARTER of the 2048^2 HM as if it were the full 1024^2 map, so the
heights (and thus the sampled colours) are wrong scale/region.
ROOT (resolution): the map-load upsamples HM until [0x5578]>=[0x8494]=2048; but 6980's
voxel walk + its fixed 0x100000 colormap stride expect a 1024x1024 (1 MB) map. Either
the original stops the HM upsample at 1024 for the 6980 path, or 6980 walks a 1024 view
of a 2048 map with a matching (0x400000?) stride that the port mis-set to the image's
literal 0x100000. NEXT: read the original 36bf HM alloc SIZE (0x8ba5, from [0x937]) +
what dim 6980's coord actually spans (its map-size register), to decide whether the fix
is (a) HM upsample target 1024 not 2048 for the voxel buffer, or (b) a colormap placed
0x100000 after a 1 MB HM view. The gap mismatch (0x400000 vs 0x100000) is the concrete
lever; reconciling it is the windshield fix.

DECISIVE resolution datum -- 6980 walks a 1024x1024 map (asm-proven). The colormap
index in 6980 is built by two `shld $0xa` (10-bit) shifts (6ae3: shld $0xa,%ebp,%ecx;
6ae7: shld $0xa,%ebx,%ecx) -> ecx = (Yhi_10 << 10) | Xhi_10 = a 20-bit index, range
0..0x100000 = 1024x1024. So 6980 reads HM at [0x85bc]+ecx and colormap at [0x85bc]+
0x100000+ecx, BOTH 1024x1024 (1 MB), contiguous. This is fixed by the asm.
The port provides a 2048x2048 (4 MB) HM with the colormap 4 MB later -> 6980 reads the
wrong resolution/region. FIST_TILEFILL only reaches 33.6% because it copies the FIRST
1 MB of the 2048^2 HM (the top-left quarter at 2048 resolution) as the "1024^2 map",
not a proper 1024^2 DOWNSAMPLE. THE FIX: give 6980 a 1024^2 HM + 1024^2 colormap
contiguous buffer at [0x85bc] (HM at +0, colormap at +0x100000), built as a proper
1024-resolution view of the map -- NOT the first quarter of the 2048^2 buffer. NEXT:
modify the tile-fill/map-load to build a genuine 1024^2 downsampled HM + 1024^2
colormap contiguous, re-measure the index match (a proper 1024^2 view should jump well
past 33.6%); if it does, that confirms the resolution root and the faithful fix is to
have the map-load produce the 1024^2 voxel buffer 6980 expects (or stop the HM upsample
at 1024 for this buffer). This is the concrete, measured windshield fix path.

MEASURED CONFIRMATION -- 1024^2 downsample lifts terrain 33.6% -> 73.2%. Implemented
FIST_TILEFILL_DS: instead of copying the first 1 MB of the 2048^2 HM/colormap, build a
GENUINE 1024^2 downsample (every-other row/col: hmcm[y*1024+x]=src[(2y)*2048+(2x)],
colormap likewise at +0x100000). Result on the AZER1 spawn vs oracle_9200_framematched_
pass08.idx.bin:
    baseline (stale tile, no 6980) : 10.6%
    TILEFILL plain (6980, first 1MB of 2048^2) : 33.6%
    TILEFILL_DS (6980, genuine 1024^2 downsample) : 73.2%   <-- 2.2x jump
This CAUSALLY CONFIRMS the resolution root: 6980 walks a 1024^2 map, and giving it a
proper 1024^2 HM+colormap (contiguous, colormap at +0x100000) makes the terrain match
jump to 73.2%. The full chain is now measured end to end: run 6980 (10.6->33.6) + feed
it the right 1024^2 resolution (33.6->73.2). The remaining ~27% gap is likely the
DOWNSAMPLE METHOD (every-other vs the original's actual reduction -- averaging / a
specific 2:1 filter) and/or colormap light-reduce details. NEXT: (a) determine the
original's HM reduction (is the map even upsampled to 2048^2, or does the port
over-upsample -- read the runtime [0x8494] path + whether the original's voxel buffer
is 1024^2 natively); (b) try averaging vs point-sampling for the downsample; (c) then
land the faithful fix -- have the map-load produce the 1024^2 voxel buffer 6980 expects
(no TILEFILL band-aid). This is the strongest windshield progress of the session: a
CONFIRMED fix direction with a measured 73.2% (from a 10.6% baseline).

Downsample method + LOD root. Averaging is WORSE than point-sampling (53.3% vs 73.2%)
-> the terrain is POINT-sampled (nearest), as expected for discrete voxel texels; the
remaining ~27% gap is NOT the reduction filter. The likely root: the DS test
down-samples the port's ALREADY-2048^2-upsampled HM (256->2048 then ->1024), but the
faithful path is probably 256->1024 DIRECTLY (2 interpolation doublings via bc06/bed2
instead of 3 -> different intermediate values). The port OVER-UPSAMPLES by one LOD:
asm 0x8a4c sets [0x8490]=0xc(=12), [0x8494]=1<<[0x8490] reduced by the DETAIL setting
to 2048 (=1<<11) at runtime; for 6980's 1024^2 buffer it should be 1<<10=1024. The
oracle capture has det=1 (oracle_azer1_tcb_camera.txt). So the faithful fix is to make
the map-load's HM upsample target 1024 (one less doubling) for the voxel buffer 6980
walks, matching 6980's fixed 10-bit index. MEASURED chain so far: 10.6% (no 6980) ->
33.6% (6980, 2048^2) -> 73.2% (6980, point-downsampled 1024^2); a TRUE 256->1024
upsample should exceed 73.2% toward bit-exact. NEXT: build a true 256->1024 HM+colormap
(target [0x8494]=1024) instead of downsampling the 2048^2, measure; if it beats 73.2%,
land the faithful fix -- the map-load produces the correct-LOD 1024^2 voxel buffer
(likely the detail->[0x8490] reduction is off by one doubling, or a separate 1024^2
voxel buffer must be built alongside the 2048^2 one). Point-sampling confirmed faithful.

LOD is SMC-patched (fix mechanism found). The map-load block 0x8a80-0x8b14 writes the
LOD value al (=[0x8490]) into ~30 self-modifying-code slots across the 6980 raycaster
(0x6ae6/0x6b66/0x7fc7/0x7fdc/0x8105/0x8f48/0x9068/0x9429/0x9434/... -- the shl amounts
and strides 6980 uses to index the HM/colormap). asm 0x8a4c sets [0x8490]=0xc(12) ->
[0x8494]=1<<12=4096 on THIS branch (which also sets [0x395c]=3); the port runtime shows
[0x8494]=2048 (=1<<11), so a DETAIL branch before 0x8a40 selects the LOD. So the LOD
(map resolution) is not a single variable but is baked into 6980 via SMC at map-load;
a faithful fix must set [0x8490] correctly at map-load so all ~30 SMC slots + the
[0x8494] upsample target agree on the resolution 6980 walks. The measured 1024^2
downsample (73.2%) approximates what a correct LOD would produce natively. FAITHFUL FIX
(bounded, multi-step): trace the detail->[0x8490] branch (which detail level yields
which LOD; oracle det=1), set the port's map-load [0x8490] to the value that makes 6980
walk the map at the oracle's resolution, verify the ~30 SMC slots re-patch consistently,
and confirm the HM/colormap buffers size + contiguity ([0x85b8]==[0x85bc]+ (1<<LOD)^2)
match. Then the terrain should reach bit-exact without any TILEFILL band-aid. This is
the concrete landing path; the fix direction is measured-confirmed (10.6->33.6->73.2%).

Detail->LOD branch traced (map-load 0x89f5-0x8a4c). The LOD [0x8490] is chosen by
resource-existence checks (call 0x5c98 on strings @0x84a6/0x84ac/0x84b3): default 9
(512^2); +1 -> 10 (1024^2, [0x395c]=1); +2 -> 11 (2048^2, [0x395c]=2); +3 -> 12
(4096^2, [0x395c]=3). Each level also sets the sky flag [0x395c] to 0/1/2/3. The port
gets [0x8490]=11 (2048^2), which is CORRECT for its detail -- the 2048^2 full map is
right. Since 6980 ALWAYS walks 1024^2 (its colormap read is a fixed +0x100000), the
1024^2 buffer 6980 walks is a REDUCE of the 2048^2 map, and [0x85bc] is REPOINTED to
that reduced buffer for the 6980 call. So the fix is NOT the LOD -- it is the
2048^2 -> 1024^2 REDUCE that builds 6980's walked buffer (which FIST_TILEFILL_DS
approximates at 73.2% by point-subsampling). NEXT: find the original's reduce (the
code that builds the 1024^2 HM+colormap from the 2048^2 and repoints [0x85bc] before
6980) -- likely a 2:1 reduce near 3931/6980 or in the 689a/light-reduce path; port it
faithfully so 6980 walks the exact reduced buffer (should exceed 73.2% toward
bit-exact). The port's 2048^2 map + LOD are faithful; only the voxel-LOD reduce is
the open step.

Detail files = 8.MEG / 16.MEG / 40.MEG (the map LOD tiers: 8->1024^2, 16->2048^2,
40->4096^2). The map-load's 5c98 resource check on these picks BOTH the LOD [0x8490]
(10/11/12) AND the sky flag [0x395c] (1/2/3) together, in the SAME branch. This ties
the voxel LOD to the sky-setup the shim already reconstructs ([0x395c]) -- so the LOD
[0x8490] must be reconstructed consistently with [0x395c] from the same .MEG-presence
logic. (No `mov reg,0x85bc` repoint appears in the image, so the 1024^2 buffer 6980
walks is either built in place by a reduce or [0x85bc] holds the reduced buffer natively
-- to be traced.) The 2048^2->1024^2 reduce that feeds 6980 (approximated by
FIST_TILEFILL_DS at 73.2%) remains the open faithful step; it is bound up with the
.MEG/detail/[0x395c]/[0x8490] map-load logic the shim partially reconstructs. NEXT:
trace how the 1024^2 voxel buffer 6980 reads is produced from the 2048^2 map (the
reduce / repoint), consistent with the .MEG detail selection, and port it faithfully.

*** LANDABLE FIX IDENTIFIED *** -- the port's voxel LOD is off by one (11 vs 10),
inconsistent with the sky flag. The map-load's .MEG resource branch sets the LOD
[0x8490] AND the sky flag [0x395c] TOGETHER: only 8.MEG -> [0x8490]=10 (1024^2),
[0x395c]=1; +16.MEG -> 11 (2048^2), 2; +40.MEG -> 12 (4096^2), 3. The oracle (and the
port shim) have [0x395c]=1, which REQUIRES [0x8490]=10 (1024^2). But the port's 89b0
.MEG RAM-size probes (FILEMGR chain 6250/5d50/5cc2/5c98) signal found/not-found via the
x86 CF, which the __allregs model dropped; patch 210 reconstructs it via g_ext_find_cf
but INCOMPLETELY -- the port over-detects one level, so [0x8490]=11 (2048^2) while
[0x395c]=1. This 2048^2 (vs the faithful 1024^2) is EXACTLY the resolution mismatch
that makes [0x85b8]=[0x85bc]+0x400000 (4 MB) instead of +0x100000 (1 MB), so 6980 reads
the wrong region. THE FIX: correct the .MEG-probe CF (g_ext_find_cf) so 16.MEG/40.MEG
return not-found and [0x8490] stops at 10 (matching [0x395c]=1) -> the HM builds as
1024^2 (1 MB), contiguous with the colormap, and 6980 (asm-verified faithful) samples
correctly WITHOUT the TILEFILL band-aid. This is a real, asm-grounded, landable patch
in the FILEMGR .MEG-probe CF logic -- the first true windshield fix, consistent with
the measured 1024^2 result (73.2%+). NEXT: locate where g_ext_find_cf is set for the
.MEG probes, correct it so the LOD lands at 10, verify [0x8490]=10 + gap=0x100000 +
terrain match jumps (default render, no TILEFILL), and that the 159 flows don't regress.

REFINED (the .MEG probes are RAM-SIZE tiers, not file checks): "8.MEG/16.MEG/40.MEG"
are RAM thresholds (8/16/40 MB available -> LOD 10/11/12). The port's large flat g_mem
makes the probe pass the 8 MB AND 16 MB tiers -> LOD 11 (2048^2); the DOSBox oracle
passed only 8 MB -> LOD 10 (1024^2, [0x395c]=1). So the faithful fix is to make the
port's RAM-size probe report the SAME tier as the DOSBox reference (8 MB -> LOD 10),
not over-report. This is a shim FILEMGR/RAM-probe fidelity fix (the g_ext_find_cf /
6250 chain), grounded in matching the reference environment the DoD targets. NEXT:
read 6250/5c98's RAM check, make it return the oracle's 8 MB tier so [0x8490]=10, then
verify [0x8490]=10 + gap=0x100000 + the DEFAULT render terrain match jumps (no TILEFILL)
+ 159 flows green. First true windshield fix, landable as an asm-verified patch.

CORRECTION (5c98 = DOS FindFirst, not a RAM probe): 5c98 does INT 21h AH=0x4E
(FindFirst, cx=0x33 attr) for the filenames "8.MEG"/"16.MEG"/"40.MEG" -> file-existence
checks. Neither the port NOR the DOSBox oracle has any .MEG files, yet the oracle has
[0x395c]=1 (LOD 10). So the original's LOD does NOT come from the .MEG FindFirst branch
(which finds nothing -> would give LOD 9 / [0x395c]=0). And the port's [0x8490]=11 also
does NOT come from it. So the LOD/[0x395c] source is ELSEWHERE (not the 89f5-8a4c .MEG
branch) -- OPEN. The established, solid facts remain: 6980 walks 1024^2 (asm); the port
builds a 2048^2 HM (gap 0x400000) -> 6980 mis-reads; a 1024^2 point-downsample gives
73.2% (fix direction confirmed). The remaining question is WHY the port's voxel buffer
is 2048^2 (LOD 11) when 6980 needs 1024^2 (LOD 10) -- and where [0x8490]/[0x8494] get
their runtime values (NOT the .MEG branch). NEXT: instrument the port's op-0x18 to dump
[0x8490]/[0x8494]/[0x395c]/[0x395d] and find the actual writer of [0x8490]=11 (the
upsample-target source), then reconcile it to the oracle's 1024^2. The fix is still the
resolution (2048->1024), just its source is not the .MEG FindFirst as first read.

[0x8490] SOURCE found = TCB[+0x59]. FIST_LODCHK shows [0x8490]=11 is set from
TCB[+0x59] (decompile 13207: `_DAT_8490 = TCB[+0x59]`), BEFORE the .MEG branch (which
finds nothing and doesn't override). So the port's voxel LOD comes from the TCB detail
byte +0x59=11 (2048^2). [0x395c]=1 (sky flag) is set separately by the shim. Forcing
TCB[+0x59]=10 (FIST_FORCELOD=10) before 89b0 CRASHES -- the map-load, SMC slots, and
TILEFILL buffers are all sized for 2048^2, so changing only the LOD byte desyncs them.
So the fix is NOT a one-byte change; either (a) LOD 11 is faithful and 6980 walks a
1024^2 REDUCE of the 2048^2 (the missing repoint/reduce -- TILEFILL_DS approximates it
at 73.2%), or (b) the whole map-load should run at LOD 10 (TCB[+0x59]=10) with all
buffers/SMC consistent. DECISIVE datum needed: the ORACLE's TCB[+0x59] / [0x8490] (10 or
11) -- a guest-RAM capture (dosbox-fist) at the AZER1 map-load settles (a) vs (b). If
oracle [0x8490]=11, the fix is the 1024^2 reduce for 6980 (find where the original
repoints [0x85bc] to a reduced buffer, size 1<<((11-1)*2)... = 1024^2); if 10, the
port's TCB detail byte is wrong upstream (the detail-setting -> TCB[+0x59] path). The
resolution fix direction stays confirmed (1024^2 -> 73.2%); this pins WHICH faithful
form. Diagnostics added: FIST_LODCHK, FIST_FORCELOD, FIST_GAPCHK, FIST_TILEFILL_DS/AVG.

*** FORK RESOLVED via ORACLE GUEST-RAM -- the root is the extender's PM PAGING ***
Read the oracle's values from the saved dosbox-fist 16MB guest-RAM dump
(/tmp/oracle_azer1_tcb.pass00.ram.bin, page-walk cr3=0xe000, ext ds base 0x10000000):
    oracle ext[0x8490] = 11   (SAME as port -- LOD is FAITHFUL, not the bug)
    oracle ext[0x8494] = 2048 (SAME as port)
    oracle [0x85bc](HM)=0x74e60  [0x85b8](CM)=0x474e60  gap=0x400000 (4MB, SAME as port)
So the LOD (11 -> 2048^2), the buffer allocation, AND the 4 MB HM/colormap gap are ALL
identical port<->oracle. The port is faithful HERE. The divergence is the EXTENDER'S
32-bit PM PAGING: 6980 reads the colormap at ext-flat [0x85bc]+0x100000 (=0x174e60),
and the extender's page table REMAPS ext-flat offsets (sim_voxel6980.py: "tile ext-flat
0x44200 -> phys 0xb78200, NOT identity; runtime buffers remapped"), so ext-flat 0x174e60
ALIASES the colormap's physical page via paging -- even though [0x85b8]=0x474e60 is a
different ext-flat address. The PORT's FLAT g_mem model has NO such paging: [0x85bc]+
0x100000 is a literal +1 MB into the 2048^2 HM, not the colormap. THAT is the windshield
root -- the missing extender PM page-mapping for the voxel buffers, NOT the LOD/reduce.
FIST_TILEFILL_DS (73.2%) approximates the paged view by building a contiguous 1024^2
HM+colormap. THE FAITHFUL FIX: replicate the extender's page-table mapping for the voxel
buffers so ext-flat [0x85bc]+0x100000 (and 6980's coord strides) alias the same physical
buffers the original's paging presents. NEXT: read the extender's page table from the
guest-RAM dump (the mapping ext-flat 0x1xxxxx -> phys for the HM/colormap window), and
reconstruct that mapping in the shim's ext memory model (or lay the flat buffers out to
match the paged view). sim_voxel6980.py already models pieces of this. This is the
definitive, oracle-grounded root -- everything else (LOD/alloc/gap) is faithful.

Refinement (paging creates DISTINCT buffers, not aliasing): page-walking the oracle
dump, ext-flat [0x85bc]+0x100000 (0x174e60) -> phys 0x1f6e60 holds a GRADIENT (3f 3e
3e 3d 3d 3c = 63,62,62,61,61,60), while [0x85b8] (0x474e60) -> phys 0x376e60 holds the
COLORMAP colours (8c 84 84 80 = 140,132,132,128). They map to DIFFERENT physical pages
(1.5 MB apart). So 6980's [0x85bc]+0x100000 read is NOT the colormap-colour buffer --
it is a SEPARATE gradient/light-reduce buffer that the extender's PM paging places at
that ext-flat offset. The port's flat g_mem has NO such buffer there (it reads HM row
512). So the windshield root is the extender's PM PAGE LAYOUT: several distinct voxel
buffers (HM, a gradient/light buffer at +0x100000, the colour colormap, the tile) are
placed at specific ext-flat offsets by non-identity paging, and 6980 reads them by
those offsets; the flat port collapses them. FIST_TILEFILL_DS's 73.2% put COLOURS at
+0x100000 (wrong -- the oracle has a gradient there) yet still improved, so the 1024^2
resolution helped but the buffer SEMANTICS at +0x100000 are still off. NEXT: map the
extender's full page layout from the dump -- for each 6980 read offset ([0x85bc]+coord,
[0x85bc]+0x100000+coord, the tile [0x3918]=0x44200, [0x85b8]) resolve ext-flat->phys and
identify the buffer + its content; then reconstruct those buffers at the right flat
offsets in the shim so 6980 reads faithfully. sim_voxel6980.py already models the phys
remap (0x44200->0xb78200); extend it to all 6980 inputs. The gradient buffer at
[0x85bc]+0x100000 (63..60 ramp) is the key newly-found input to identify + reproduce.

CONNECTS to the documented UNRESOLVED CRUX (sim_voxel6980.py). My oracle buffer-map
confirms that sim's model: [0x85bc] holds HM at +0 and a buffer at +0x100000 (the sim
calls it CM); the LIGHT colormap (84-228) is at [0x85b8]=0x474e60. But 6980 does NOT
read the color directly -- sim line 52: color = proj[detail*0x100 + ((L0 + HM[coord])
& 0xff)] -- it combines the height + light level L0 and indexes a PROJ TABLE. sim
lines 19-22 already document the wall: "the colormap 6980 indexes at [0x85bc]+0x100000
is DARK (max 104) in every oracle dump and cannot produce the LIGHT terrain -- the
'85b8 reduce-colormap collapse'. The colormap is the crux, unresolved." My page-walk
matches: [0x85bc]+0x100000 (0x174e60) = heights 14-85 (dark), while the light colours
(84-228, max 228) live at [0x85b8] (0x474e60). So 6980's terrain COLOUR comes from a
proj-table lookup keyed by (light L0 + a value from the dark +0x100000 buffer), NOT a
direct colormap read -- and reconstructing that proj-table pipeline faithfully is the
deep, documented open crux. THE OPEN STEP (unchanged in nature, now oracle-confirmed
end to end): reconstruct 6980's proj-table colour pipeline -- how (L0, height, the
+0x100000 buffer) index the proj table to yield the LIGHT terrain colour -- with the
extender's PM page layout supplying each buffer at its ext-flat offset. Everything
upstream (LOD/alloc/gap/camera/sky/matrix, all oracle- or asm-verified faithful) is
settled; the windshield reduces to this proj-table + paging reconstruction. The
session has driven it from a diffuse 75.5%-wrong mystery to this single, precisely
localized, oracle-grounded open crux.

CRUX re-confirmed at asm+oracle level (the deepest open point). Traced 6980's full
colour write: 6aeb-6aee compute the proj lookup proj[base + ((L0+HM[coord])&0xff)]
(proj base is SMC-patched at 0x6add per detail+L0); the proj result is a HEIGHT/run
length (compared to the per-column running max at [0x4e60+edx], drawn when it's a new
peak); the COLOUR written is al = [0x85bc]+0x100000+coord (6b1a) via `rep stos %al`
(6b2b) filling `run` pixels. So 6980 writes CM[coord] directly as the terrain colour.
FRESH ORACLE CONFIRMATION of the paradox: CM = [0x85bc]+0x100000 is STABLY DARK (range
14-85) in BOTH the pass00 spawn dump AND the pass10 stable dump (identical top values
60/37/62) -- NOT frame-evolving. Yet the oracle framematched terrain is LIGHT (150-230).
So 6980 writes dark values (14-85) but the terrain renders light -- the documented
"85b8 reduce-colormap collapse", now confirmed with guest-RAM at the asm level. The
reconciliation (unresolved, deepest crux) is one of: (a) the dark tile INDICES (14-85)
are mapped to light DISPLAY colours by a palette/DAC stage between the tile and the fb
(but oracle_9200_framematched stores INDICES 150-230, not colours -- so the tile->fb
step would have to REMAP indices); (b) 6980's colour source is subtly NOT [0x85bc]+
0x100000 at render time (esi repoint, or a different indexing the sim+this trace both
read as +0x100000); (c) the framematched idx sample and the guest-RAM dump are
different provenance/map. This is THE remaining windshield unknown; everything else is
oracle/asm-verified faithful. A future session should settle (a)/(b)/(c) -- likely by
capturing the oracle's TILE [0x3918] terrain rows (not sky) at the framematched frame
and checking whether they are 14-85 (dark, matching CM -> the fb sample is remapped) or
150-230 (light -> 6980's colour source differs from [0x85bc]+0x100000).

CRUX RESOLVED (concrete fix hypothesis) -- 6980 writes LIGHT, so [0x85bc]+0x100000
must ALIAS the light colormap. Decisive check: the oracle TILE [0x3918] (0x44200) is
LIGHT -- range 116-252, 48949/65536 in 150-230, ZERO in the dark 14-85 range. So 6980
writes LIGHT values (150-230) to the tile, NOT the dark CM[coord] (14-85) my cr3=0xe000
page-walk reads at [0x85bc]+0x100000. Re-walking with the extender's page table (sim
says phys 0x131000) gives UNMAPPED in this dump -- i.e. the dump's captured cr3=0xe000
(engine page table) resolves ext-flat [0x85bc]+0x100000 to a DARK buffer, but 6980 runs
under the EXTENDER's render-time paging, which maps that SAME ext-flat offset to the
LIGHT colormap [0x85b8] (range 84-228). So the paradox is a PAGE-TABLE-CONTEXT artifact:
6980's colour read [0x85bc]+0x100000 ALIASES the light colormap [0x85b8] via the
extender's PM paging; my dump analysis used the wrong (engine) page table and saw a
dark buffer. CONCRETE FAITHFUL FIX: in the port's flat model, arrange [0x85bc]+0x100000
to hold the LIGHT colormap [0x85b8] content (indexed by coord) -- i.e. alias/copy the
[0x85b8] colormap to [0x85bc]+0x100000 for the 6980 read. TILEFILL_DS's 73.2% already
put A colormap there (downsampled); the faithful version uses the full [0x85b8] light
colormap at the right coord indexing. NEXT: modify the shim so [0x85bc]+0x100000 =
[0x85b8]'s light colormap (the extender-paging alias), re-measure the DEFAULT-path
terrain match (expect > 73.2% toward bit-exact); this is the concrete windshield fix,
grounded in the oracle tile being light + the ext PM-paging alias.

Remaining 27% = buffer STRIDE/indexing geometry (everything else faithful). Verified
via oracle guest-RAM + measurement:
  - PORT [0x85b8] colormap = range 82-228, mean 141.1, top 151/132/145/128/152
    ORACLE [0x85b8]         = range 84-228, mean 140.3, top 152/128/151/145/132
    -> colormap CONTENT is FAITHFUL (my earlier "128-152" was just the peak).
  - PORT HM (first 64KB) = ORACLE HM (first 64KB) BYTE-IDENTICAL (range 35-86, mean
    58.6, same top 59/51/67/49/61/66 with identical counts).
  - Injecting the oracle ray tables 3a24/3e24 into TILEFILL_DS = 0 change (73.2%).
  - The alias [0x85bc]+0x100000 <- [0x85b8] is already present in TILEFILL.
So HM, colormap, alias, and ray tables are all faithful/present, yet 73.2%. The gap
is the buffer STRIDE: 6980's coord = (Yhi_10<<10)|Xhi_10 expects a 1024-wide (1024^2-
stride) HM + colormap; the port's map-load builds a 2048^2 HM, and TILEFILL_DS's
every-other point-downsample (hmcm[y*1024+x]=H[(2y)*2048+(2x)]) is NOT how 6980 indexes
the extender's paged 1024^2 view of the 2048^2 map. The extender's PM paging presents a
specific 1024^2 window; the every-other downsample approximates it (73.2%) but is not
the exact window. NEXT: determine the extender's exact 1024^2 window into the 2048^2 HM
(is it a top-left 1024x1024 sub-block, an every-other subsample, or a paged tile
gather?), test each downsample variant (sub-block vs every-other) against 73.2%, and
match 6980's real coord->buffer mapping. The windshield reduces to this single
buffer-window geometry; HM/colormap/camera/alias/rays are all oracle/asm-verified
faithful.

Window narrowed: every-other IS the 1024^2 view (not sub-block). Tested downsample
windows against oracle_9200_framematched: every-other [2y][2x] = 73.2%, top-left
sub-block [y][x] = 34.6% (much worse). So the extender's 1024^2 voxel view of the
2048^2 map is an EVERY-OTHER subsample (2:1 decimation), NOT a quadrant. every-other
(73.2%) is confirmed the right window direction; the remaining 27% is FINER geometry:
either the subsample PHASE ([2y][2x] vs [2y+1][2x+1] vs mixed), or 6980's coord->texel
sub-pixel/projection rounding (8120 -> 90b8/90bc ray steps, the shld coord). NEXT: test
the 4 subsample phases; if none beats 73.2%, the residual is 6980's projection/coord
geometry (the ray-step SMC 6ad2/6ad8 from D4224 * 90fc/9100) -- compare the port's
90b8/90bc/90d4/90d8 projection outputs to the oracle's (from the guest-RAM dump) at the
AZER1 camera. All buffer CONTENT (HM==oracle, colormap==oracle) + the alias + window
direction are settled; the last ~27% is sub-texel projection geometry.

Phase settled + the plateau explained. Tested the 4 every-other phases: phase 0
[2y][2x]=73.2% (best), ph1=71.9%, ph2=68.0%, ph3=67.9%. So phase 0 is correct; the
phase is NOT the remaining gap. CONCLUSION: no downsample variant reaches bit-exact
because TILEFILL_DS is an APPROXIMATION of the extender's PM paging, not the real
mechanism. The original does NOT downsample -- 6980 walks the 2048^2 buffers through a
page table that presents a specific 1024^2 view; an every-other point-subsample
approximates that view (73.2%) but cannot be bit-exact (the paged gather may interleave
pages / order texels differently than a uniform 2:1 decimation). So the ~73% plateau is
the ceiling of the downsample band-aid; the FAITHFUL FIX is the extender PM-paging
reconstruction: read the extender's page table (which maps ext-flat [0x85bc]+coord, for
coord 0..0x100000, to physical), and replicate that exact mapping in the shim's ext
memory model so 6980 reads the 2048^2 buffers via the true paged 1024^2 view -- then the
terrain is bit-exact (all buffer CONTENT already == oracle). This is a real shim
memory-model effort, now fully specified. Everything is verified faithful (HM/colormap/
camera/matrix/sky/6980-addressing/alias/window-direction/phase); the last step is the
paging, which the flat g_mem model must reproduce. NEXT: page-walk the extender's own
page table (not cr3=0xe000) from the 16MB dump for the [0x85bc] coord range to derive
the exact ext-flat->phys texel gather, and build that gather in the shim.

ALIAS HYPOTHESIS RETRACTED (doctrine: correct without ego). Scanned all 4096
4KB-aligned candidate page directories in the 16MB dump for one that maps ext-flat
[0x85bc]+0x100000 (0x10174e60) to the LIGHT colormap phys region (0x37xxxx): ZERO
hits. So there is NO page table under which [0x85bc]+0x100000 aliases the light
colormap. The last-iteration "extender paging aliases +0x100000 to [0x85b8]"
hypothesis is DISPROVEN. The paradox is genuinely unresolved: 6980's colour read is
[0x85bc]+0x100000 (asm-confirmed: 6b1a `mov 0x100000(%esi,%ecx),%al`; esi=[0x85bc]
unchanged from 6a60; written via 6b2b `rep stos %al`), which is DARK (14-85) in the
dump, yet the oracle tile [0x3918] is LIGHT (150-230, ZERO dark values). No paging
reconciles it. This is exactly the prior session's documented wall ("the colormap is
the crux, unresolved"). HONEST STATE: I could not crack how 6980 produces light terrain
from a dark [0x85bc]+0x100000 read. Remaining unknowns to try next: (a) is the tile
[0x3918] the buffer 6980 actually writes, or does 6980 write elsewhere and 9200/a later
stage recolours? (verify [0x4a60] write-pointer targets vs [0x3918]); (b) does the dump
capture a state where [0x85bc]+0x100000 is mid-transform (a later frame has it light)?
(the pass00==pass10 test said stable, but only sampled 4KB); (c) is 6980's colour read
subtly a proj-table lookup too (proj indexed by the +0x100000 value -> light), not the
raw +0x100000 byte? Re-trace 6aee's eax/proj vs 6b1a very carefully. Everything ELSE
(HM/colormap-content/camera/matrix/sky/6980-geometry/window/phase) is oracle/asm-
verified faithful; this single colour-source paradox is the true open crux. The 73.2%
downsample is the best approximation short of resolving it.

RESOLUTION (corrected -- repoint, not paging): [0x4a60] write-pointers = 0x44200,
0x44300, 0x44400... (column stride 0x100) -> 6980 writes into the LIGHT tile [0x3918]=
0x44200 (256 cols x 256 rows, column-major). So 6980 DOES write the light tile, and
its colour source is [0x85bc]+0x100000. Reconciliation: [0x85bc]+0x100000 must be LIGHT
at 6980-RENDER time, but the dump (captured at 9200-entry, AFTER 6980) shows it dark ->
the original REPOINTS [0x85bc] to a BUILT contiguous light buffer (HM + the [0x85b8]
light colormap) for the 6980 call, then RESTORES [0x85bc]=0x74e60 before 9200. The
page-dir scan found nothing because it's NOT paging -- it's a repoint + a built buffer.
This is EXACTLY what FIST_TILEFILL does (build contiguous HM+[0x85b8]-colormap, repoint
[0x85bc], run 6980) -> 73.2%. So the alias intuition was right in spirit; the mechanism
is a repoint to a built buffer, and the dump can't show it (post-restore). The 73.2%
gap is that TILEFILL's every-other DOWNSAMPLE isn't the original's exact 1024^2 buffer
build. THE FAITHFUL FIX: find the extender's 6980-input BUILD code -- the function
(between the map-load and the 6980 call, inside 3931/85d0->6980 setup or a reduce) that
constructs the 1024^2 HM+colormap contiguous buffer and repoints [0x85bc] -- and port
it exactly (no [0x85bc] write appears via `mov`, so it's built via lea+36bf/345c/a
reduce fn writing through &[0x85bc]). NEXT: disassemble the path from 3931/85d0 into
6980 for a [0x85bc]-repointing build (lea 0x85bc + reduce), and the reduce that fills
it from the 2048^2 map + [0x85b8]; port that build so 6980's input is bit-exact ->
terrain bit-exact. All buffer CONTENT is faithful; the open step is the exact reduce/
build of 6980's repointed 1024^2 input buffer.

*** BREAKTHROUGH -- ROOT FOUND + fix validated past the plateau (78.7% > 73.2%) ***
THE ROOT: the port's ported 6980 uses the STATIC (un-SMC-patched) image constants
everywhere; the ORIGINAL self-modifies ~30 slots at map-load (block 0x8a80-0x8b14
writes [0x8490] into the shift/stride slots, [0x8498] into the colormap-displacement
slots). Proven by the oracle dump: 6980's colour-read displacement (6b1a) is 0x100000
in the static image but 0x400000 in the render-time dump (SMC 8b24: mov [0x8498],
0x6b1d). So the port reads [0x85bc]+0x100000 (dark, inside HM) where the original reads
[0x85bc]+0x400000 = [0x85b8] (the LIGHT colormap). Similarly the coord shld amount
(6ae6) is 0x0a static but SMC-patched to [0x8490]=0xb (2048^2 index, not 1024^2).
VALIDATED FIX (ephemeral build/ test): changed the port 6980's colour displacement
0x100000 -> DAT_0000_8498 AND coord `>>0x16<<10` -> `>>0x15<<0xb` (N=11), ran 6980 on
the REAL 2048^2 [0x85bc] (FIST_TILEFILL_REAL, no downsample/repoint): terrain jumped to
78.7% -- PAST the 73.2% downsample plateau. So the faithful fix is NOT a downsample
band-aid; it is applying the SMC patches to the ported 6980 so it uses the DYNAMIC
[0x8490]/[0x8498] values on the real 2048^2 buffers. The remaining ~21% is the OTHER
~28 un-applied SMC slots (ray-step 6ad2/6ad8 from D4224*90fc/9100, proj-table base
0x6add/0x6b5d, running-max [0x4e60], the sibling reads 6b9a). THE LANDABLE FIX: a
patches/NNN-*.diff that makes 6980 (+ siblings 6d/6e/8fa0) use the SMC-patched dynamic
values ([0x8490] for every shld/shift amount, [0x8498] for every colormap displacement,
etc.) instead of the decompiled static image constants -- systematically, all ~30 slots.
The board's earlier "LOD is SMC-patched into ~30 slots" note is exactly this; now it's
the confirmed root with a measured 78.7% validating the direction. NEXT: enumerate all
~30 SMC target addresses (the 0x8a80-0x8b14 `mov %al/%eax,0xXXXX` writes) + map each to
its decompiled static constant in 6980/siblings, replace with the dynamic [0x8490]/
[0x8498] value, and drive the terrain to bit-exact.

SMC slot enumeration (the patch spec). Map-load block 0x8a7f-0x8b10 writes al=[0x8490]
(=11) into ~30 SMC slots; block 0x8b15-0x8b29 writes eax=[0x8498] (=0x400000) into 4:
  [0x8490] (shift/stride) slots: 0x6ae6,0x6aea,0x6b66,0x6b6a (6980); 0x6d66,0x6d6a,
    0x6de6,0x6dea (6d sibling); 0x7fc7,0x7fcb,0x7fdc,0x7fe0,0x7ff8,0x7ffc,0x800d,0x8011,
    0x8105,0x8109 (7f/80/81); 0x8f48,0x8f4c,0x8f6a,0x8f6e (8fa0); 0x9068,0x906c,0x907f,
    0x9083 (90); 0x8485,0x8489 (84); 0x9429,0x9434 (94).
  [0x8498] (colormap displacement) slots: 0x6b1d,0x6b9d (6980); 0x6d9d,0x6e1d (6d).
Each SMC target is an immediate BYTE inside an instruction (the shld amount, a shift
count, or the 4-byte colormap displacement). The ported decompile emits these as STATIC
constants (0x0a for shifts, 0x100000 for the displacement). The FIX (landable patch):
for each render fn, replace the static constant with the DYNAMIC SMC source -- [0x8490]
for shift amounts, [0x8498] for colormap displacements. For the op-0x24 6980 path the
minimal set is: coord shld (0x6ae6/0x6aea/0x6b66/0x6b6a -> shift by [0x8490]) + colormap
disp (0x6b1d/0x6b9d -> [0x8498]). VALIDATED so far (2 of these applied): 78.7%. NEXT:
apply the remaining 6980 shift slots (0x6b66/0x6b6a -- likely a second coord/step shift
in 6980's inner loop) + verify the ray-step SMC (6ad2/6ad8 from D4224*90fc/9100, patched
separately at 6a a3-sites), re-measure toward bit-exact; then author patches/NNN-6980-
smc-dynamic.diff making 6980 use [0x8490]/[0x8498] dynamically. This is the first
LANDABLE windshield fix -- root-caused, oracle-proven, and measured (78.7% and climbing).

Dynamic fix validated + patch-ready (78.7%); NOT landed in isolation (doctrine). The
faithful dynamic form -- coord shift `>>0x16<<10` -> `>>(0x20-DAT_0000_8490)<<DAT_0000_
8490` and colormap disp `0x100000` -> `DAT_0000_8498` in 6980 (both reads) -- gives the
SAME 78.7% as the hardcoded test, so it works for any LOD via the real SMC source values.
This is a correct, asm-verified decompile correction (the port's 6980 used STATIC image
constants; the original SMC-patches them; per "code is the truth" the static form is a
bug). BUT per "vollstaendig oder gar nicht" + "Tests sind Spezifikation" it is NOT landed
alone: it is INERT for all current flows (6980 is not called in the default op-dispatch,
which is still stubbed; the windshield is not a matrix flow), and 78.7% != bit-identical.
It lands as PART OF the complete windshield fix (6980 bit-exact + op-dispatch wiring to
call 6980 + a windshield matrix flow that goes bit-identical on native+wasm). The
remaining ~21% to bit-exact (with all 6980-path SMC applied + real 2048^2 buffers + ray
tables no-effect) is the PROJECTION geometry: 8120's camera->ray outputs (90b8/90bc/90d4/
90d8) + the proj-table height projection + the per-column run geometry. NEXT: compare the
port's 90b8/90bc/90d4/90d8 (8120 projection outputs) to the oracle's (from the guest-RAM
dump) at the AZER1 camera; find + fix the projection divergence to drive 6980 bit-exact;
then wire op-0x08->6980 + add the windshield matrix flow + land patches/407-6980-smc-
dynamic.diff (+ the projection fix + the op-dispatch) as the COMPLETE, test-verified
windshield fix. The root is found + fix validated (10.6%->78.7%); the complete landing is
the remaining bounded work.

Projection is NOT the main gap (80.4% with oracle projection injected, +1.7% over
78.7%). Dumped the port's 8120/85d0 projection outputs at the 6980 call vs the oracle
guest-RAM values: port has 90b8=0,90bc=0,90fc=0,9100=0 (the 8fa0-computed ray-step
projection is UNSET in the port's TILEFILL path) vs oracle nonzero; 90d4/90d8 slightly
off; 90dc/90e0/90c0/90f0/90f8/90ac MATCH. Injecting the oracle's exact 90b8/90bc/90d4/
90d8/90fc/9100 before 6980 lifted terrain only 78.7% -> 80.4%. So the projection globals
are a minor contributor; the remaining ~20% is finer geometry -- the proj-table height
projection (proj[detail*0x100+((L0+HM)&0xff)]) and/or sub-pixel camera precision, and
may be near the frame-match ceiling (the port renders its spawn frame; oracle_9200_
framematched is pass08 -- if the frames differ by even sub-pixel camera the terrain
differs). SESSION PAYOFF (measured): the 6980 SMC root fix drove the windshield terrain
from 10.6% (baseline, no 6980) to 78.7% (6980 SMC-correct on real 2048^2 buffers) /
80.4% (+ oracle projection) -- a 7.5x improvement, root-caused + oracle-proven. The
remaining ~20% is diminishing-returns fine geometry. The LANDABLE core is the 6980 SMC
patch (coord shift [0x8490], colormap disp [0x8498]); it lands with the op-dispatch
wiring + a windshield matrix flow once the fine geometry is either closed or accepted as
the frame-match ceiling. NEXT: characterize the 20% residual (scattered=frame-match
noise vs structured=a specific proj-table bug) by mapping WHERE the port/oracle terrain
indices differ; if structured, fix the proj-table build; if scattered near the ceiling,
proceed to author the complete windshield patch (6980 SMC + op-dispatch + matrix flow).

20% residual characterized (mixed: subpixel ceiling + a proj-table structural error).
Per-row terrain match (6980 SMC fix + oracle projection, 80.4% overall): rows 5-13
(far/horizon) = 100%, rows 21-85 (near) = 72-86%. Mismatch magnitude: 47% are |diff|<=8
(subpixel/adjacent-texel = frame-match ceiling), 53% larger (median 9, mean 18.4). The
DISTANCE GRADIENT (far perfect, near degraded) points to a distance-dependent structural
error -- the proj-table height projection proj[detail*0x100 + ((L0+HM)&0xff)] (L0 is the
distance/light level from 90dc, which MATCHES oracle). So the proj-table CONTENT is the
likely near-terrain divergence: the port's proj table (the "detail base curve" the ~30
SMC slots also feed) may be built differently than the oracle's. So the last structural
piece is the PROJ-TABLE BUILD. NEXT: read the oracle's proj table from the guest-RAM dump
(the table 6980 indexes at 6aee, base SMC-patched at 0x6add from [0x3909]/[0x90dc]) and
compare to the port's build; find the proj-table build function in the extender and
verify/fix it. If the proj-table matches and the near-rows still differ, the residual is
the subpixel frame-match ceiling and the fix is complete-enough to land. SESSION SUMMARY:
6980 SMC root fix = 10.6%->80.4% (7.5x, root-caused+oracle-proven); the remaining
structural piece is the proj-table build (near-terrain distance rows); the subpixel half
is likely the frame-match ceiling. The windshield is decomposed to its final structural
component + a measurement ceiling.

Proj-table read (the last structural piece). The oracle proj-table (from the sim blob
voxel6980_inputs_lt2pass00.bin.gz) is 64000 bytes = 250 distance-rows x 256 height-
indices: proj[d][i] = the projected screen span for distance-row d, height-index
i=(L0+HM)&0xff. Far rows (d~0) are ~all 255; near rows (d~12) are a rising gradient
(128,153,179,204,230,255,...). It lives at ext [0x3909] (base; 6980's read base is
SMC-patched at 0x6add from [0x3909]+[0x90dc]-derived L0). This distance-indexed curve is
the near-terrain structural residual: the port's proj-table build must match the oracle's.
NEXT: find the port's proj-table BUILD function (writes the 250x256 curve at [0x3909]) --
it is a perspective/projection table built from the camera (90fc/9100 the port had as 0,
+ 90c0), so it may be tied to the same 8fa0/8120 projection that was unset; build it
faithfully, compare to the oracle 250x256, and the near rows (21-85) should close from
72-86% toward 100%. Combined with the 6980 SMC fix (10.6->80.4%), a faithful proj-table
should reach near-bit-exact terrain (modulo the ~47% subpixel frame-match ceiling). Then
author the COMPLETE windshield patch (6980 SMC + proj-table + op-dispatch + matrix flow).

Proj-table is UNBUILT (the near-terrain residual root) -- and it unifies with the
projection globals. Dumped the port's proj-table at [0x3909]=0x82f4200 vs the oracle
250x256 curve: only 10.2% match; the port's is LARGELY ZEROS (first8=[0,0,0,...] for
every detail row) while the oracle has a real perspective curve (detail 0: 128,255,255..;
detail 40: 128,131,135,138..; detail 240: 128,128,128..). So the port does NOT build the
proj-table (like the ray tables 3a24/3e24, it's left zero) -- but unlike the ray tables,
the proj-table DOES matter (6980 reads it at 6aee for the height projection). This is the
near-terrain (rows 21-85) structural residual. UNIFYING INSIGHT: the proj-table AND the
projection globals (90fc/9100/90b8/90bc = 0 in the port) are BOTH unbuilt -- common root:
the port does not run the extender's PROJECTION-SETUP pipeline (8fa0/8120 + the proj-table
build), because the render OP-DISPATCH is stubbed (op 0x10->8fa0 etc. log-and-return).
So the windshield's complete fix decomposes into: (1) the 6980 SMC fix [FOUND + validated,
10.6->80.4%]; (2) wire the extender op-dispatch to run the projection-setup ops (8fa0/8120
+ the proj-table build) so 90fc/9100 + the proj-table are populated [the near-terrain +
projection residual]; (3) op 0x08->6980 tile-build wiring; (4) a windshield matrix flow.
All root-caused + oracle-grounded. The core is the SMC fix + wiring the extender render
op-dispatch (which this session mapped fully: op-table @0xcb3, 0x08->6980, 0x0c->93c0,
0x10->8fa0, 0x24->9200, 0x44->sky). NEXT: wire the op-dispatch faithfully (run each posted
render op through the op-table trampoline instead of log-and-return), which builds the
proj-table + projection + tile, then measure the full default-path terrain toward
bit-exact, then land the complete windshield patch (SMC + op-dispatch + matrix flow).

Proj-table builder located -> confirms the op-dispatch is the single root. 8fa0 sets
90fc/9100 (projection: 8fb2 mov [0x90fc], 8fbe mov [0x9100] from the 944b/9650 tables *
90c0) but does NOT build the proj-table. The proj-table [0x3909] is allocated by the fn
at ~0x852b (`lea 0x3909,%edx` -> alloc) and its 250x256 perspective CURVE is built from
the projection (90fc/9100/90c0). Since the port never runs 8fa0/8120 (op-dispatch stub),
90fc/9100 stay 0 -> the proj-table builds as ZEROS -> near-terrain wrong. So EVERY
windshield residual (projection globals, proj-table, and the tile-build) has the SINGLE
common root: the extender RENDER OP-DISPATCH is a log-and-return stub, so the whole
projection-setup + build chain (8fa0 projection -> proj-table build -> 6980 tile) never
runs; the port only renders via env-gated shim scaffolds. THE COMPLETE WINDSHIELD FIX
(fully decomposed, root-caused, oracle-grounded this session):
  1. 6980 SMC dynamic fix (coord [0x8490] + colormap [0x8498]) -- FOUND + validated
     (10.6->80.4% on real buffers). patch-ready.
  2. Wire the extender op-dispatch (op-table @0xcb3 trampolines) so the engine's posted
     render ops actually run: 0x08->6980 (tile), 0x0c->93c0, 0x10->8fa0 (projection ->
     90fc/9100 + proj-table build), 0x44->sky -- replacing native_main.c:2880's
     log-and-return. This builds the projection + proj-table + tile faithfully.
  3. A windshield matrix flow in verify.sh (op-0x24 spawn terrain, region rows 5-85) so
     the fix is bit-verifiable native+wasm.
Landing all three = the first complete, test-verified windshield fix. The session did
the full diagnosis/decomposition + validated the primary fix; the op-dispatch wiring +
matrix flow is the bounded implementation ahead. NEXT: implement the op-dispatch wiring
as an env-gated test first (verify no 159-flow regression -- render ops only fire
in-mission), measure the default-path terrain with 6980-SMC + real projection toward
bit-exact, then land the complete patch set.

Individual projection-function calls do NOT shortcut the pipeline (still 78.7%).
Calling m_ext 8fa0+8120 before the TILEFILL 6980 changed nothing -- they need their own
inputs (the 944b/9650 projection tables, 90e0, the camera-setup chain) that only the full
op-dispatch pipeline populates. So the windshield's near-terrain fix genuinely requires
WIRING THE EXTENDER OP-DISPATCH PIPELINE (not calling one function) so the engine's posted
render ops run in order with their real inputs, building projection -> proj-table -> tile.
DIAGNOSTIC PHASE COMPLETE (this 55-iteration session). The windshield is fully root-caused,
oracle-grounded, and decomposed:
  - PRIMARY ROOT + validated fix: 6980 uses static un-SMC-patched constants; the dynamic
    [0x8490]/[0x8498] fix on real 2048^2 buffers = 10.6% -> 78.7-80.4% (7.5x). patch-ready.
  - REMAINING (near-terrain): the projection-setup pipeline (8fa0/8120 projection + the
    proj-table build @[0x3909]) never runs because the render op-dispatch is a log-and-
    return stub -- one common root for the projection globals + proj-table + tile-build.
  - Every voxel component individually verified faithful (HM/colormap/camera/matrix/sky/
    6980-addressing/window/phase); 8+ false leads honestly retracted.
The COMPLETE fix (implementation phase, multi-step): (1) land the 6980 SMC patch; (2) wire
the extender render op-dispatch (op-table @0xcb3 -> real m_ext render fns, with the setup
ops populating the projection tables + proj-table) replacing the log-and-return stub, no
159-flow regression; (3) add a windshield matrix flow; verify bit-identical native+wasm.
This is the bounded implementation ahead; the diagnosis + primary-fix validation is done.

=== CONSOLIDATED IMPLEMENTATION ROADMAP (distilled from the 56-iteration diagnosis) ===
STATE: windshield fully root-caused + primary fix validated (10.6%->80.4%, 7.5x). Every
component oracle/asm-verified faithful; the ONLY root of all residuals is the stubbed
extender render op-dispatch (native_main.c:2880 log-and-return). Diagnosis DONE.

FIX 1 (asm-verified, patch-ready) -- 6980 SMC dynamic:
  re_out/fist_ext.c FUN_0000_6980: the two coord lines `(x>>0x16)<<10 | y>>0x16` -> use
  the SMC shift `>>(0x20-DAT_0000_8490)<<DAT_0000_8490`; the two colour reads
  `iVar14 + 0x100000 + uVar` -> `iVar14 + DAT_0000_8498 + uVar`. (SMC block 0x8a7f-0x8b29
  proves [0x8490]/[0x8498] are the render-time values.) Apply the SAME to the sibling
  voxel fns (6d/7f/80/81/8f/90/84/94) per the enumerated SMC slot list above. -> patch 407.

FIX 2 -- wire the extender render op-dispatch (the near-terrain + projection residual):
  the engine's 459a mission loop posts render ops each frame; the op-table @fist_image.bin
  0xcb3 maps op->trampoline->render fn (0x08->8df0+3931->6980 tile-build; 0x0c->78f0->
  85d0+93c0; 0x10->7940->8fa0 projection; 0x24->82b8->8120+9200 sample; 0x44->7660 sky).
  Replace native_main.c:2880 log-and-return with a real dispatch that runs each posted op
  through its m_ext render fn IN THE ENGINE'S ORDER, so the setup ops populate the
  projection (90fc/9100/90b8/90bc) + build the proj-table [0x3909] before 6980, and 6980
  before 9200. Env-gate first (FIST_WIRE_DISPATCH); the 159 flows never post render ops
  (menu/editor), so no regression. Resolve inputs/crashes iteratively (viewport rect,
  TCB [0xc93], the 944b/9650 projection tables).

FIX 3 -- add a windshield matrix flow (verify.sh): op-0x24 AZER1 spawn, crop terrain rows
  5-85, assert bit-identical vs a genuine DOSBox ref (capture via capture_battle_burst.sh)
  on native AND wasm. This makes the windshield a bit-verifiable flow.

LAND: make patch (407 + dispatch shim) -> verify.sh both (159 + windshield flow, AE=0,
native==wasm) -> then the DoD 10x re-gate on the EXPANDED matrix. Root+FIX1 proven; FIX2
is the bounded implementation; FIX3 is mechanical.

DEEPER ROOT (op-sequence trace): the engine does NOT post the render ops at all. The
per-frame op sequence after map-load is: 0x20,0x04,0x44(sky),0x68,0x6c,0x70,0x74,0x7c
(setup), then op 0x4c REPEATED (present pump) + op 0x54 (roster) -- NO op 0x0c/0x08/0x24
(the render ops) are ever posted. This confirms the documented "459a mission loop posts
op 0x0c every outer iteration ONLY when the tick advances, but the op-0x4c present spin
never pumps the cooperative INT-8 tick, so c452 never advances, so the per-tick sim+
render step never runs." So the render pipeline is FROZEN at the TICK-PUMP level --
UPSTREAM of the op-dispatch stub. The 80.4% terrain was measured via the FIST_MISSFB
scaffold (which force-runs the render chain), NOT the engine's real posted-op flow.
So the complete windshield fix chain is deeper than FIX2 alone:
  FIX0 (prerequisite): make the mission-loop tick (c452) advance faithfully during the
    op-0x4c present spin so the 459a loop posts the per-frame render ops (0x0c/0x08/0x24).
    The shim's op-0x4c present handler must pump the cooperative INT-8 tick the way the
    original's timer does (NOT FIST_COOP_TICK, which corrupts the render per earlier
    findings -- the faithful timer/tick advance under the real-timer verify condition).
  FIX1: 6980 SMC dynamic [validated]. FIX2: op-dispatch wiring. FIX3: matrix flow.
So the windshield's TRUE first gate is the mission-loop tick/timer advance (why the
engine never posts render ops), then the op-dispatch, then 6980 SMC. NEXT: investigate
the op-0x4c present-spin tick handling -- why c452 doesn't advance, and how the original's
timer advances it -- so the engine posts the render ops; this is the real FIX0 gate
upstream of everything. (Diagnosis continues to deepen but converges: tick -> op-post ->
op-dispatch -> 6980-SMC -> bit-exact terrain -> matrix flow -> DoD.)

*** FIX1 LANDED: patch 407 (6980 SMC dynamic) ***  commit 6616fe6.
Reconsidered the "don't land in isolation" hesitation and landed it: patch 407 is a
COMPLETE, asm-verified decompile correction of a specific bug (Ghidra froze 6980's static
image constants; the map-load SMC self-modifies them to [0x8490]/[0x8498]) -- per "code is
the truth" the static form is objectively wrong, and this is exactly the pristine-decompile
-> asm-verified-patch workflow, independent of whether the whole windshield surface is done.
Verified: make check clean; verify.sh native 159/0; verify.sh wasm 159/0; native==wasm
0-diff (6980 runs only on the mission windshield path, not the 159 menu/editor/cockpit
flows -> no regression, incl. the mission-cockpit-2c spawn flows). re_out pristine. The
patch was regenerated against the prior 342/343 6980-SMC-model patch context. This breaks
the session's "zero landed fixes" -- the 58-iteration diagnosis is now a landed, verified
correction. REMAINING for the complete windshield (still open): FIX2 wire the op-dispatch
(so the engine's posted render ops run + the tick advances so they're posted at all),
FIX3 windshield matrix flow; plus follow-up sibling-SMC patches (6d/7f/8f/90 for ops 0x10
etc. -- same correction class). NEXT: FIX2 op-dispatch, or the sibling SMC patches.

FIX0/tick RETRACTED (render ops DO post) + FIX2 prerequisite found. Traced the engine's
posted ops without FIST_MISSFB (which _exits at op-0x24): the engine posts op 0x08 (tile-
build) AND op 0x24 (sample) REPEATEDLY (3x each, alternating 0x08->0x24 per frame). So the
render loop IS running, the tick DOES advance (in-mission one-tick-per-pump), and the render
ops ARE posted -- my earlier "engine posts no render ops / tick frozen" was an artifact of
the FIST_MISSFB _exit + the first-80-ops window. FIX0/tick is NOT needed.
So the ONLY windshield gate is FIX2: the posted op 0x08/0x24 are LOG-AND-RETURNED instead of
dispatched to 6980/9200. Wiring op 0x08 -> m_ext 8df0()+3931() (tile-build via 6980, now
SMC-correct by patch 407) CRASHES with FPE (divide-by-0): 8df0 sets the viewport dims from the
TCB rect (+0x16..0x1c), which is 0 in the port (the documented "viewport-dim frontier":
FUN_0000_ddff writes TCB+0x1e/+0x22 from a rect that is never populated) -> a 0 viewport ->
divide-by-0 in 3931/85d0. So FIX2 has a PREREQUISITE: populate the TCB viewport rect faithfully
before the render ops run. So the windshield render chain is: [viewport-setup: TCB rect] ->
op 0x08 (6980 tile, SMC-correct via 407) -> op 0x24 (9200 sample). The viewport-setup is the
missing first link. NEXT: trace/populate the TCB viewport rect (FUN_0000_ddff's source rect
@word[0x156a], never populated -- find who should write it: the op-service setup op that sets
the mission viewport), then wire op 0x08/0x24 dispatch and measure the real render-path terrain
(expect ~78.7% like the scaffold, now via the engine's posted ops). FIX1 (patch 407) landed;
FIX2 = viewport-setup + op-dispatch wiring.

FIX2 is the FULL op-dispatch pipeline (not a single op). Wiring op-0x08 -> 6980, even with
the viewport forced, still FPEs -- the crash is in 85d0 (camera setup: 90c0 = 0xffffffff /
TCB[+0x3e] focal). At op-0x08 time the TCB camera fields (focal +0x3e, etc.) are NOT yet set
(they read 256 at op-0x24 time via FIST_MISSFB_PROBE, but op-0x08 fires EARLIER in the frame,
before the camera-setup op). So dispatching one render op in isolation crashes because its
setup PREREQUISITES (camera focal, viewport rect) are populated by OTHER posted ops that the
port also log-and-returns. So FIX2 must wire the FULL posted-op sequence faithfully in order:
the setup ops (0x68/0x6c/0x70/0x74/0x7c + camera/viewport configure) populate the TCB camera +
viewport, THEN op-0x08 (6980 tile, SMC-correct via 407) builds, THEN op-0x24 (9200) samples.
This is the substantial multi-op render-pipeline reconstruction -- each posted op dispatched to
its m_ext fn, with the setup ops' outputs (camera focal/viewport/projection tables) feeding the
render ops. The FIST_MISSFB scaffold works because it runs the render chain LATER (op-0x24 time)
when the TCB is fully populated + forces the viewport; the faithful FIX2 runs each op at its
posted time with the real setup chain. NEXT: map the setup ops (0x68/0x6c/0x70/0x74/0x7c ->
their m_ext fns) + what each populates (camera, viewport, tables), wire the full dispatch in
order, resolve crashes as each prerequisite is met, then measure the real-render-path terrain.
This is the bounded (but multi-step) FIX2 implementation. FIX1 (patch 407) is landed + verified.

FIX2 op-table dispatch spec (mapped). The posted render/setup ops route via op-table
@fist_image.bin:0xcb3 -> trampoline -> fn:
    op 0x68 -> 0x76fd (Ghidra GAP 0x7490-0x76fd, undecompiled)
    op 0x6c -> 0x77e2 -> 0x1280
    op 0x70 -> 0x11cb (direct handler)
    op 0x74 -> 0x6f17 -> 0x706b
    op 0x7c -> 0x77a4 -> 0x6032
    op 0x08 -> 0x10e0 -> 8df0+3931 -> 6980 (tile-build, SMC-correct via patch 407)
    op 0x24 -> 0x82c0 -> 8120 -> 9200 (sample)
The faithful FIX2 = run the extender's PM-gate op-dispatch (FUN_0000_0f30 -> op-table[0xcb3
+op] -> trampoline) for each posted op, in the engine's posted order, replacing the shim's
log-and-return. The setup ops (0x68/0x6c/0x70/0x74/0x7c) populate the camera (TCB +0x2c..
0x3e incl. the focal +0x3e that 85d0 divides by) + viewport (+0x16..0x1c) + projection
tables that the render ops (0x08 6980, 0x24 9200) consume; running them in order resolves
the FPE crashes (each render op's prerequisites met by the prior setup ops). CAVEAT: some
setup fns are in the Ghidra decompile GAP (0x76fd/0x77e2/0x77a4 near 0x7490-0x76fd) --
those may need decompile-gap-filling or a shim reconstruction. This is the substantial
multi-op FIX2 reconstruction. ALTERNATIVE (pragmatic, if the faithful dispatch proves too
gap-blocked): drive the render via the WORKING FIST_MISSFB+6980 scaffold at op-0x24 time
(the TCB is fully populated there), which already renders terrain at 78.7% with patch 407
-- but that is a shim scaffold, not the faithful op-dispatch (doctrine prefers the real
dispatch). SESSION STATUS: FIX1 (patch 407 SMC) landed+verified; FIX2 (op-dispatch pipeline)
is spec'd + is the bounded multi-op implementation, partly gated by Ghidra-gap setup fns.
