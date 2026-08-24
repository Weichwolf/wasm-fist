Type: refactor
Area: ghidra-decompile
Parent: 0003

The Ghidra decompile threads the 16-bit SEGMENT-REGISTER dataflow (CS/ES/DS/SS) faithfully, so the
`re_out/` decompile emits real segment values instead of uninitialised `unaff_CS`/`unaff_ES` (and dropped
far-pointer/string-op bases) -- collapsing the single largest patch class (~184 of 397 patches) to near
zero AND removing the determinism root that fragilises native<->wasm terrain (board:0003) and the terrain
object-ref cascade (board:0007), because the reads become DEFINED values identical on both targets.

## The pattern (measured, 2026-08-24)

Of 397 patches, **184 (46%) are segment-register-dataflow-LOSS** -- Ghidra dropping the CS/ES/DS/SS a
16-bit instruction implicitly carried.  Sub-classes (overlapping):
  far-ptr/call/jmp seg reconstruction : 109   (lcall/ljmp/CONCAT22(seg,off) with the seg dropped)
  ES-class (unaff_ES / rep-ES / far-ES):  70
  CS-class (unaff_CS / `mov [mem],cs`)  :  67
  string-op (rep movs/stos) seg base    :  53
  DS-class (unaff_DS)                    :  24
Every one is a hand-restore of a segment the decompile lost.  The two engine code segments are
**CS=0xf69** (87 explicit `= 0xf69` in re_out) and **CS=0x1000** (27), plus a third FUN_e000_ region.

## NovaLogic idiom (why the code is segment-dense) -- Cosmo's point #1

On a 386 the engine is hand-written 16-bit real-mode asm that leans on the segment registers as a fourth
address dimension: `mov [mem],cs` / `push cs` SAVE the running code segment so a later `lcall [mem]:off`
RECONSTRUCTS a far pointer back into this code WITHOUT recomputing the segment -- a standard slow-hardware
trick (cheap segment reuse, no relocation math per call).  Likewise ES/DS are loaded once and reused across
rep-string blits.  Ghidra's decompiler, run without CS/ES context, cannot see these implicit carries, so it
emits `unaff_*` (read-uninitialised) and drops the far bases -- and every such site became a patch.

## Root cause

`tools/ghidra/PrepAnalysis.java` sets the segment-register CONTEXT for DS (0x1c00) and SS (0x2ba9) over
the whole image, but **leaves CS and ES UNSET** ("ES left unset (varies)").  With CS unset, `mov [mem],cs`
has no value -> `unaff_CS`; with ES unset, ES-based refs/rep-string bases are dropped.  Setting DS/SS is
exactly what made DGROUP refs resolve to real DAT_ symbols; the same mechanism, extended to CS (and a
best-effort ES), should resolve the segment-carry class the same way.

## Approach (dedicated session -- Ghidra IS required; it is NOT installed in the current env)

1. Extend PrepAnalysis to set the CS context register per CODE-SEGMENT region.  Design challenge: the flat
   load image aliases the same bytes under multiple seg:off (e.g. linear 0x12ebe is both FUN_1000_2ebe and
   0xf69:0x362e), and Ghidra recovered part of the 0xf69 cluster under CS=0x1000 (mis-base, per patch
   notes).  Options to evaluate: (a) CS=0xf69 over the dominant main-code range + CS=0x1000 over the
   0x10000.. range keyed to the FUN_SSSS_ decode segment; (b) per-function CS from the far-call that
   reaches it (reference-driven); (c) load the image as SEGMENT-aware blocks rather than one flat block so
   CS is intrinsic.  Verify against the 87/27 explicit-CS ground truth already in the patches.
2. Best-effort ES: ES is genuinely dynamic, but the dominant cases (ES:=DS at entry, ES:=CS after push cs,
   ES:=a loaded far seg) are locally recoverable; seed the common entry pattern and measure the drop.
3. Re-derive the decompile (make image + the Ghidra headless decompile), then re-apply patches -F0: every
   patch that only restored a now-correctly-threaded segment will FAIL to apply (its target line changed) or
   become a no-op -> triage each: DELETE the ones the context fix subsumes, KEEP the genuinely-semantic
   ones.  Target: 184 -> a small residue.
4. Full 10x re-gate (this touches the whole decompile -> every flow rebuilds).

## Why this is the highest-leverage item

- Consolidation (Cosmo #2): one Ghidra-config change retires up to ~184 hand-patches -- the bulk of the
  maintenance surface -- and every future 16-bit surface stops generating this patch class.
- Determinism root (board:0003): the terrain `unaff_CS`/`unaff_ES` reads (FUN_0000_02c5 c686=cs,
  FUN_0000_134e c3e2=cs, FUN_1000_2ebe c434=es) become DEFINED constants -> native and wasm read the same
  value -> the code-layout fragility (UB coincidence) is gone at the source, and patch 412 lands.  NOTE:
  the earlier global `unaff_CS=0xf69` prototype did NOT converge native==wasm (still 206) -- but it was
  CONFOUNDED (it forced ALL 67 CS sites to 0xf69 when some are 0x1000); the per-site-correct CS a proper
  context fix produces is the real test, and is the reason this is a decompile fix, not a source hack.
- board:0007 (terrain object-ref cascade) is the same lost-register-dataflow defect -> likely co-resolved.
- Ghidra-param optimisation (Cosmo #3) generalises to every 32-bit-DOS / 16-bit-segmented target: CS/ES
  context is the missing half of the DS/SS context already proven here.

## Board coupling

Parent 0003 (audio/terrain determinism) and 0007 (terrain baseloss) are DOWNSTREAM consequences resolved
by this root fix.  This item is gated only on a Ghidra install (headless decompile); it needs no oracle.

RESULT (CS half, 2026-08-24) -- IT WORKS.  Added tools/ghidra/SetCSContext.java (postScript: baseline CS =
page-segment per 64 KB page; override the far-call-discovered non-zero cluster's CERTAIN sub-window with
CS=seg -- e.g. 0xf690..0xffff => CS=0xf69).  Ran it on the reused project + re-export (no re-analysis):
  unaff_CS declarations: 134 (re_out) -> 0 (scratch/csctx)  -- ALL eliminated.
  terrain site FUN_0000_02c5: `DAT_1000_c686 = unaff_CS` -> `DAT_1000_c686 = 0xf69` (the exact value the
    hand-patches restore); other sites correctly resolve `= 0x1000` under CS=0x1000.
So the CS-class (~67 patches) is subsumed by one Ghidra postScript, per Cosmo's rule "what Ghidra detects
correctly needs no patch".  NEXT: (a) ES context (unaff_ES still present at c434 = FUN_1000_2ebe, ES:=DS at
entry -> baseline ES=DGROUP 0x1c00 like DS; measure); (b) full integration: swap the CS(+ES)-context
decompile into re_out, re-triage the now-redundant segment patches (-F0 apply -> drop the ones subsumed,
keep the semantic ones), rebuild, verify native==wasm + full 10x re-gate; (c) confirm whether the defined
c686/c3e2/c434 also collapses the board:0003 terrain determinism (native reads the constant, not garbage).

RESULT (CS+ES, 2026-08-24) -- 312 unaff -> 0, and 207/397 patches self-identify as redundant.  With
FIST_ES_CTX=1 (ES baseline = DGROUP 0x1c00, the ES:=DS pattern) added to SetCSContext:
  unaff_CS 134->0, unaff_ES 176->0  => ALL 312 unaff register decls eliminated.
  terrain sites: c686=0xf69, c3e2=0xf69, c434=0x1c00 (all DEFINED, were unaff).
Swapped the CS+ES decompile into re_out, `make assemble` OK (2269 funcs, 0 unaff in fist.c), then applied
all 397 patches individually: 190 apply clean, **207 FAIL because their target segment is now correct**.
So ~half the patch base is subsumed.  CAVEAT: the 207 need triage -- most are redundant (drop), but some
may be semantic patches whose CONTEXT merely shifted (rebase), and the GLOBAL ES=0x1c00 baseline is
aggressive (string-ops with a buffer ES would mis-resolve) -> must be validated by build+matrix, and ES may
need to be per-function (SegmentFixup-style provable ES:=DS) rather than global.  DECISIVE next test: build
the CS+ES decompile + the 190 applying patches and check terrain-azer1 native==wasm (board:0003) + a broad
matrix smoke -- if green, the 207 were genuinely redundant and board:0003's UB determinism is fixed at the
Ghidra root; if it regresses, ES is too aggressive / some patches need rebasing.

MECHANISM PROVEN + PIPELINE-INTEGRATED; MIGRATION MEASURED (2026-08-24).  SetCSContext.java is now in the
decompile.sh POST chain (after SegWrapFixup, before ApplyConv) with FIST_ES_CTX default-on.  Verified via
reused-project + SetCSContext + ApplyConv re-thread (scratch/csctx_conv):
  - ALL 312 unaff register decls eliminated (unaff_CS 134->0, unaff_ES 176->0);
  - ApplyConv re-threads signatures WITH the cs/es context resolved (sig-changed converges 1256->8 over 5
    passes) -> consistent prototypes+bodies, unaff=0;
  - `make assemble` OK (2269 funcs, 0 unaff in fist.c).
Patch migration measured against this decompile: of 397 patches 236 apply EXACTLY, 22 rebase with fuzz,
139 are candidate-redundant (their target segment is now correct).  BUT the migration is NOT a mechanical
auto-triage: (a) a patch failing to apply does not prove it redundant -- some are semantic patches whose
CONTEXT merely shifted and must be rebased BY HAND; (b) crude --fuzz=3 application MANGLES prototypes
(observed: `FUN_1000_223c(...,param_4,;` -- a hunk placed at the wrong line), so fuzz-rebase is unsafe.
Dropping the 139 wholesale fails the build (missing still-needed prototype/DAT/semantic patches conflict
with changed bodies).  So the migration is exactly the "dedicated session" this item scopes: per-patch
review -- for each non-exact patch decide redundant(drop) vs semantic(rebase by hand), rebuild, verify
native==wasm, full 10x re-gate.  What is DONE and banked: the Ghidra root fix itself (SetCSContext.java,
pipeline-integrated) -- the hard part; per Cosmo, "what Ghidra now detects correctly needs no patch", and
the ~139 segment patches are that set.  NOTE for board:0003: with the fix, the terrain sites read DEFINED
c686=0xf69 / c434=0x1c00 on BOTH targets (no UB) -- the determinism test is now a clean build away, pending
the patch migration.  DOSBox memrec trace (Cosmo's SQL-trace idea) can supply ground-truth CS/ES per site
to VALIDATE the Ghidra values and, if a build is produced, decide board:0003 W-vs-W' against the oracle.

HONEST CORRECTION (2026-08-24, counterexample-driven) -- the "~184 patches redundant" claim was WRONG.
Verified per-patch against the assembled CS/ES decompile: of the 161 non-exact patches, only ~25 are even
CANDIDATES for pure-constant subsumption; **136 do POINTER-BASING** (`g_mem+(seg<<4)+off` / far-ptr
reconstruction) that the CS/ES context does NOT fix.  Decisive counterexample -- patch 011-4cb9-thunk-esbase
on the new decompile:
  new decompile: `*param_1 = 0xea; *(undefined2*)(param_1+3) = param_2;`  (param_1 still a RAW host pointer)
  patch 011 restores: `dst = g_mem+(ES<<4)+DI`, ES from the stack (caller CS), SMC store based into g_mem.
The CS context eliminated the `unaff_CS` PSEUDO-VAR (the symptom), but the patch's real work -- BASING the
DI offset as g_mem+(seg<<4)+off and threading ES-from-stack -- is still absent, so the patch is STILL
NEEDED (rebase, not delete).  Most segment patches are this shape.  So:
  - What CS/ES context genuinely delivers: 312 unaff pseudo-vars -> 0 (a real decompile-quality win) and it
    DOES subsume the PURE `mov [mem],cs`->constant cases (the terrain c686/c3e2/c434 sites, ~10-25 patches).
  - What it does NOT deliver: wholesale retirement of the segment class.  ADOPTING it costs REBASING ~136
    still-needed pointer-basing patches whose context shifted -- a large migration to gain ~25 retirements
    plus a cleaner decompile.
REVISED VALUE THESIS: board:0010's payoff is NOT patch-count reduction; it is (a) a cleaner pristine
decompile for all FUTURE work (no unaff pseudo-vars), and (b) the DETERMINISM fix -- IF the now-DEFINED
terrain sites (c686=0xf69, c434=0x1c00 on both targets, no UB) actually converge board:0003 terrain
native==wasm.  That determinism test is the real prize and REQUIRES the migration to build.  DECISION: keep
SetCSContext.java pipeline-integrated (it is correct and improves the decompile), but treat the migration as
a deliberate, per-patch REBASE effort (not a delete-139 shortcut), justified primarily by the board:0003
determinism payoff -- which should be confirmed FIRST (e.g. via the DOSBox trace giving ground-truth, or a
minimal targeted build) before committing to rebasing 136 patches.

MIGRATION IS ALL-OR-NOTHING (tested 2026-08-24).  Attempted a partial build (new CS/ES decompile + ONLY
the 233 exact-applying patches, no fuzz) to test the board:0003 determinism payoff cheaply.  It does NOT
build: (a) conflicting-types errors because SIGNATURE patches (in the skipped set) and BODY patches are
INTERDEPENDENT -- a skipped signature patch leaves the auto-generated prototype conflicting with a body a
different applied patch changed; (b) a malformed prototype parse error at the assembled fist.c (`param_4,;`)
that appears on the new decompile even without fuzz -- either assemble_fist.py mishandles a re-threaded
signature or an exact-applied patch corrupts it (a migration-session task to isolate).  CONCLUSION: the
patch set cannot be partially migrated; board:0003 determinism CANNOT be tested via a partial build.  The
two ways to confirm the board:0003 payoff remain: (1) the FULL migration (rebase all ~397 patches onto the
new decompile -> build both targets -> terrain native==wasm), or (2) the terrain ORACLE frame (board:0002)
to arbitrate whether committed-W or defined-W' is faithful (the DOSBox trace can supply ground-truth per
site, but the terrain frame itself needs the frame-matched capture).  Both are dedicated work; neither is a
safe partial increment.  What is DONE this session and banked: SetCSContext.java (correct, pipeline-
integrated, 312 unaff->0), the honest scope correction (~25 subsumed not 184; 136 pointer-basing still
needed), and the confirmed DOSBox write-trace capability.  The migration + determinism confirmation is the
scoped next dedicated session, justified by the board:0003 payoff, to be entered deliberately.

ASSEMBLE-FIX UNBLOCKS THE MIGRATION STARTING POINT (2026-08-24).  With the assemble_fist.py wrapped-
signature fix landed, the CS/ES decompile now assembles CLEANLY: 0 malformed `,;` prototypes (was the
build-blocking parse error), the 243 `param_N,`-continued lines are legitimate wrapped signatures.  Patch
apply on the clean assembled fist.c: 233 exact / 164 need rebase (context-shifted by the re-threading, NOT
corruption).  So board:0010's remaining work is now precisely: rebase 164 context-shifted patches (mostly
the 136 pointer-basing ones that are still semantically needed) onto the CS/ES decompile, build both
targets, verify native==wasm + terrain determinism (board:0003), full 10x re-gate.  DE-RISKED THIS SESSION:
(1) SetCSContext.java eliminates 312 unaff pseudo-vars (integrated in decompile.sh), (2) assemble_fist.py
handles wrapped signatures (landed, native .text byte-identical), (3) migration scope measured precisely
(233 exact / 164 rebase, 0 parse errors) with the honest split (~25 subsumable, 136 pointer-basing needing
rebase).  The 164-patch rebase is the dedicated task; it is now mechanical-per-patch (clean base, no tooling
blockers), justified by the board:0003 determinism payoff.

TRUE MIGRATION COST = ~189 HAND-REBASES (decisive, 2026-08-24).  Attempted automatic 3-way rebase
(git merge-file --diff3, base=committed fist.c, target=CS/ES fist.c) on every failing patch: only 9
auto-merge; 189 CONFLICT.  Root cause: ApplyConv re-threads EVERY function's register params (variable
names + expression structure change throughout each affected function), so a patch's target CONTEXT
diverges too far from the committed base for context-based merge -- the change is NOT localized to the
segment sites.  So adopting the CS/ES context for the EXISTING patch base costs ~189 genuine hand-rebases,
to gain: a cleaner decompile + ~25 subsumed patches + POSSIBLY the board:0003 determinism fix.

REVISED RECOMMENDATION (honest cost/benefit): do NOT start the 189-rebase migration until the board:0003
determinism payoff is CONFIRMED, because that payoff is the only thing justifying the cost.  The payoff
question -- is the defined-value terrain frame W' more faithful than the committed garbage-coincidence
frame W? -- is decided by the TERRAIN ORACLE (board:0002), which breaks the circularity WITHOUT the full
migration: capture the original's AZER1 terrain under the instrumented DOSBox, compare to native-W and
defined-W'.  IF W' is oracle-faithful -> the migration is justified (it corrects terrain toward the
original AND fixes native==wasm determinism) -> do the 189 rebases.  IF W is faithful -> board:0010 is only
a cleaner-decompile refactor whose 189-rebase cost is NOT justified for the existing patch base (keep
SetCSContext for FUTURE fresh decompiles only).  So the true next step for this whole line is the terrain
oracle (board:0002), not the migration.

WHAT STANDS regardless: SetCSContext.java (pipeline-integrated, correct -- future fresh decompiles are
clean) and the assemble_fist.py wrapped-signature fix (landed, native .text byte-identical) are both
correct improvements independent of the migration decision.  The DOSBox write-trace (board:0002 tool) is
the arbiter to run next.

CRITICAL CORRECTION -- SetCSContext does NOT fix board:0003 determinism (2026-08-24).  My earlier note
("terrain sites now read DEFINED c686=0xf69 on both targets -> determinism test is a clean build away") was
WRONG.  board:0003's proven characterisation: the terrain-site reads are DEAD on wasm (emcc -O2 elides them,
VALUE-INVARIANT) and LIVE on native (-O0 reads + uses them).  So DEFINING the values (which is all
SetCSContext does) makes native USE the defined value (-> W') while wasm still ELIDES it (-> W) -> native
!= wasm by 206.  Confirmed by the prior data point: native-defined vs wasm-defined = 206; wasm-defined ==
wasm-garbage == W.  Therefore SetCSContext, applied to the migration, would make terrain native != wasm
(the current native==wasm holds ONLY because both hit W by garbage/elision coincidence, and defining breaks
that coincidence on the native side).  board:0003 determinism is a native-O0-reads / wasm-O2-elides CODEGEN
divergence, INDEPENDENT of whether the value is defined -- SetCSContext does not touch it.

REVISED board:0010 COST/BENEFIT (honest): the migration costs ~189 hand-rebases and delivers ONLY a
cleaner decompile (312 unaff->0) + ~25 subsumed patches -- NOT a determinism fix (it may regress terrain to
native!=wasm 206).  That is very likely NOT worth 189 rebases for the EXISTING patch base.  DECISION:
KEEP SetCSContext + the assemble fix for FUTURE fresh decompiles (a from-scratch decompile is then clean,
and new patches are written against defined segments), but DO NOT migrate the existing patch base.  The
board:0003 determinism fix is a SEPARATE problem: resolve why native -O0 keeps a terrain-site read that
emcc -O2 proves dead, then (oracle-arbitrated W vs W') either force both to keep the faithful value or
force both to drop a spurious read.  That is the real board:0003 work; board:0010 was a detour that
cleaned the decompile but does not solve determinism.  Net honest outcome: SetCSContext + assemble fix
landed as decompile-quality improvements; the "consolidation eliminates 184 patches / fixes determinism"
thesis is RETRACTED.

INSTALLINTFIXUP INT-RETURN ES/DS THREADING LANDED (2026-08-24) -- the pipeline-fix family grows.  Found +
fixed a real InstallIntFixup gap: it threaded INT register INPUTS + read back AX/BX/CX/DX/SI/DI/CF as
OUTPUTS but NOT ES/DS -> INT 21h AH=35 (get-vector) / AH=2f (get-DTA) returned ES:BX with only BX threaded;
their ES stayed the pre-INT value, so `mov [mem],es` after such INTs decompiled as unaff_ES (uninitialised)
instead of the shim's deterministic return.  Fix: read ES (0xf0010) + DS (0xf000e) back after the fixup
call.  Validated: FUN_1000_2ebe c434 changed `= unaff_ES` -> `uVar3 = uRam000f0010; c434 = uVar3` (reads
the threaded INT-return ES = the shim's FIST_INTVEC_SEG, deterministic native==wasm).  Correct-by-
construction + matrix-safe (fresh-decompile only).  This is the FAITHFUL mechanism for INT-return ES,
refining SetCSContext's blanket ES=DGROUP baseline for INT sites -- so the ES half of the pipeline fix is
now TWO complementary mechanisms: InstallIntFixup (INT-return ES/DS, faithful, deterministic) + SetCSContext
(CS/ES saved-segment context).  UNIFIED PIPELINE-FIX FAMILY for the board:0003 149 + board:0007 root:
(1) SetCSContext -- CS/ES segment context (mov [mem],cs saves); (2) InstallIntFixup ES/DS -- INT-return
segments; (3) assemble_fist.py wrapped-signature fix -- forward decls; remaining: the far-ptr/string-op
pointer-basing patches (~136) still need rebasing (genuinely semantic, not pipeline-subsumable).  Each
pipeline fix reduces the unaff/uninit surface a fresh decompile carries, shrinking the eventual migration.

ES-THREADING EXPERIMENT = NO-OP (2026-08-24, honest): tested threading ES as a GP param in ApplyConv
(FIST_THREAD_ES, matrix-safe scratch).  Result: 0 functions got an ES param -> no effect.  With SetCSContext
resolving the ES CONTEXT, no function reads ES-before-write (the decompiler's condition for an input param),
so ES-threading is redundant; DGROUP intact (3304 DAT_ refs).  So the segment-threading route does NOT
resolve the far-ptr/string-op pointer-basing patches (~136) -- those are genuinely SEMANTIC (they restore
`g_mem+(seg<<4)+off` pointer BASING that the decompile drops, a pointer-model issue, not a segment-VALUE
issue that context/threading fixes).  Reverted the experiment (inert).  CONCLUSION of the pipeline-fix
momentum: the EFFECTIVE, safe, bounded pipeline fixes are LANDED -- SetCSContext (CS/ES segment context),
InstallIntFixup ES/DS (INT-return segments), assemble_fist.py (wrapped forward-decls).  The remaining 149
surface (~136 far-ptr/string-op pointer-basing) is semantic and needs per-patch rebasing in the migration,
NOT a pipeline fix.  The 57 (host-stack-ptr AX/DI in the reticle descriptor) is the SS-relative-stack model,
also deep.  So the pipeline-root well is genuinely dry: what could be fixed at the Ghidra/tooling root IS
fixed; the rest is the dedicated migration + the deep pointer-model work.
