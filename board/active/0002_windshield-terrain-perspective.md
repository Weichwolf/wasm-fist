
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
