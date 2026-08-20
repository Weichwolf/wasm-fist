
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
