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
