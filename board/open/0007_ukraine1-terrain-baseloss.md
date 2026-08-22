Type: bug
Area: engine
Tags: baseloss terrain

UKRAINE1 (and likely other maps) render their in-mission voxel terrain (FIST_TERRAIN op-0x24 path)
crash-free like AZER1/SAUDI1/CYPRUS1/INDIA1/SYRIA1 do -- once the UKRAINE1-exposed base-loss CASCADE in
the b274/bd09/c31e call chain is rebased onto g_mem+0x1c000.

## Comments

UKRAINE1 FIST_TERRAIN segfaults (rc=139); AZER1 does not, though both have IDENTICAL map layout (dim=2048,
same HM/CM pointers) -- so it is NOT a map-dimension overflow.  It is a per-map BASE-LOSS CASCADE: UKRAINE1's
mission sim reaches engine code paths AZER1 doesn't, exposing dormant Ghidra base-losses (16-bit DGROUP
near-offsets deref'd as host pointers).  Root-caused via FIST_SEGV_BT + addr2line (native -g):
  1. FUN_0000_b274 (fist.c:29548): `*(byte*)(DAT_2000_a3b2+0x16)` -- a3b2 is a DGROUP near ptr (asm 0xb289
     `mov si,ds:0xe3b2; test [si+0x16],8`).  fault 0xa371.
  2. FUN_0000_b274 (fist.c:29553): `param_1[1]` / `*param_1` -- param_1 is a DGROUP near ptr (asm 0xb279/
     0xb283 `movzx dx,[bx+1]`/`[bx]`, bx=param_1).  fault 0xabb9.  (Both 1+2 fixed in a prototype:
     g_mem+0x1c000+(uint16)off; AZER1 terrain stayed rc=0 frame-OK; UKRAINE1 then advanced to #3.)
  3. FUN_0000_bd09 (fist.c:30994): `*(byte*)(param_3+0x19)` (and param_3+0x1a at :31001) -- param_3 is a
     DGROUP near ptr.  fault 0x19.  (bd09:30996 already rebases a DIFFERENT deref, so this is a missed one.)
  4. likely FUN_0000_c31e (the caller) has the same class.
FIX: rebase each near-ptr deref in the b274/bd09/c31e chain onto g_mem+0x1c000+(uint16)off, asm-verified vs
re_out/fist_dat_image.bin.  RISK/GATE: b274/bd09 are in-mission engine fns possibly reached by the covered
mission-cockpit flows -- the rebase CHANGES the deref'd value (from a wrong host-ptr read to the correct
DGROUP read), so it must be re-gated 10x on the 165-flow matrix to prove it does not perturb the settled
mission-cockpit ref frames (native==wasm holds either way -- both targets read the same address -- but the
vs-oracle AE could shift).  The prototype was reverted to keep the matrix verified; this is a focused
cascade-sweep + re-gate sub-project.  Once landed, add terrain-ukraine1 (+ sweep the other 41 maps) to the
matrix.

CORRECTION -- BLANKET b274/bd09 REBASE IS WRONG (patch 411 tried + REVERTED).  Rebasing every param_1/
param_3/a3b2 deref onto g_mem+0x1c000 fixed the UKRAINE1 crash (rc 139->no-segv) and kept AZER1 rc=0, BUT
REGRESSED the 5 terrain flows to native!=wasm (206-byte diff) -> 160/165.  Root: b274/bd09 receive
POLYMORPHIC args -- the TERRAIN render path passes an ALREADY-REAL host pointer (g_mem+...), while the
UKRAINE1 crash path passes a DGROUP NEAR offset.  `(uint16_t)(uintptr_t)param` truncates a real pointer to
16 bits, and native vs wasm have different g_mem base addresses -> the truncation diverges -> 206-byte
native!=wasm.  So the base-loss is NOT in b274/bd09 (they correctly receive a pointer from the terrain
caller); it is the UKRAINE1-path CALLER (c31e or its caller) that passes a raw near-offset instead of
rebasing it before the call.  CORRECT FIX: find the caller on the UKRAINE1 path that supplies the near
offset to bd09/b274 and rebase it THERE (or thread the right pointer), leaving b274/bd09's deref of a real
pointer intact -- asm-verify which arg the crashing caller passes.  Patch 411 reverted; matrix restored to
the verified 165/165.  The blanket-rebase lesson: check for polymorphic (near-offset vs host-ptr) callers
before rebasing a deref.

MULTI-SITE ROOT: FUN_0000_b274 is called with INCONSISTENT arg conventions across the decompile.
  - callers 26190 `b274(param_1 + -0x64a3)` and 26786 `b274(param_1 + -0x64c1)`: param_1 there is a
    FULL host pointer -> b274's param_1 is a full pointer (the working TERRAIN/frame path -- unpatched
    `*param_1` host deref is correct).
  - caller 30991 (inside bd09) `b274(word[g_mem+0x1c000+...] + param_1)`: a DGROUP NEAR offset (the
    UKRAINE1 crash path via c31e's object-action dispatch, which calls the method near with di=object
    offset).
  So b274 receives BOTH full pointers AND near offsets -> a blanket rebase (patch 411) truncated the full
  pointers -> terrain native!=wasm.  Same split for bd09/param_3 (c31e dispatch passes the near di).
FAITHFUL FIX (multi-site, board:0007): the ORIGINAL convention is uniformly NEAR (DS-relative) -- so
UNIFY: make callers 26190/26786 pass NEAR offsets (un-rebase their param_1 to (uint16)(ptr - g_mem -
0x1c000) + -0x64xx), THEN rebase b274/bd09's derefs (as patch 411 did).  Requires asm-verifying param_1's
origin at 26190/26786 (are they already-DGROUP objects whose full ptr = g_mem+0x1c000+off? then the
un-rebase is exact).  This unifies the b274/bd09/c31e cluster to one convention + re-gate 10x.  Larger
than a single deref rebase; the crash is real but the fix is a coordinated cluster change, not a one-liner.
The verified 165/165 matrix is intact (patch 411 reverted).

SYSTEMIC ROOT (bd09-only test): rebasing ONLY bd09's param_3 ALSO regresses AZER1 terrain (diff=206) --
so bd09 too receives a FULL pointer on the terrain path, not just b274.  bd09 has no direct C callers; it
is a VTABLE METHOD (registered {0xbd09,&FUN_0000_bd09}) dispatched via icall.  c31e dispatches it with a
NEAR di (asm `call [si-0x1ab0]`, DI=object offset), but the TERRAIN object-paint dispatcher reaches the
same method with a FULL pointer.  => This is NOT a b274/bd09-local base-loss; it is a SYSTEMIC calling-
convention inconsistency across the object-method dispatch surface: dispatchers pass a near DI, but the
decompiled method bodies inconsistently treat the object arg as a near offset (raw `*(param+N)` -> crash on
the near path) OR as a full pointer (works on the terrain path).  A blanket rebase in the method breaks the
full-ptr dispatch; passing a full ptr from c31e breaks methods that already rebase.
CORRECT FIX (systemic, board:0007): audit the object-method dispatch convention -- make ALL dispatchers
pass the SAME representation (full g_mem+0x1c000+off pointer is the port-natural choice) AND make every
dispatched method treat its object arg the same way, so no method is polymorphic.  This is a convention
sweep across c31e + the terrain paint dispatcher + the vtable methods, asm-anchored, with a 10x re-gate.
Substantial; the UKRAINE1 crash is the first symptom.  Reverted all local edits; matrix intact 165/165.
The one-deref-rebase and blanket-rebase approaches are both DISPROVEN.

c31e FULL-PTR DISPATCH: byte-neutral but INSUFFICIENT (tested + reverted).  Changing c31e to pass the
object as a full g_mem pointer (dg+di) instead of the raw near offset kept AZER1 terrain byte-identical
(diff=0 -- so bd09 does raw-deref its object arg, and a full ptr is the right representation) BUT did NOT
fix UKRAINE1 (still SIGSEGV at b274 via bd09).  So bd09's param_3 was never the whole story: bd09 calls
b274 with `word[g_mem+0x1c000+...] + param_1`, and bd09's OWN param_1 comes from an __allregs register the
c31e icall does not set -> b274 gets a garbage arg on the UKRAINE1 object path.  DEEPER CRUX: the crux is
the __allregs REGISTER-PASSING convention for icall-dispatched vtable methods -- which of ax/bx/dx/di hold
param_1/2/3 when a method is reached via `call [vector]`, and whether the port's icall shim threads them.
The near-vs-full object-arg is only one facet.  This needs an audit of fist_icall_near's register model +
the vtable-method signatures, asm-anchored, not a per-call rebase.  All single-site fixes (deref rebase,
blanket rebase, c31e full-ptr) are now DISPROVEN; the fix is the __allregs icall register convention.
Matrix intact 175/175.
