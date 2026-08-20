
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
