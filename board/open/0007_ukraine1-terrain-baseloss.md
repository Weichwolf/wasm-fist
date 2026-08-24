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

DEFINITIVE ROOT CAUSE (asm-anchored, re_out/fist_dat_image.bin).  Disassembled the c31e dispatch and the
UKRAINE1 vtable target bd09:

  c31e:  mov [0x9bdb],ax ; mov [0x9bd9],bx ; di=si (object) ; si=word[si] ; shl si,1 ; call [si-0x1ab0]
         -> the ONLY thing handed to the method is DI = the object near-offset; ax/bx are stashed to
            DGROUP (0x9bdb/0x9bd9), NOT reloaded; cx/dx inherited.  Port passes exactly (int)di.
  bd09:  test [di+0x19],4 ...             -> bd09's param_3 = DI (the object)
         movzx bx,[di+0x19]; shl bx,1; mov bx,[bx-0x6119]; add bx,ax; call b274
                                          -> bd09's param_1 = AX (c31e's live ax); b274 gets word[..]+ax

TWO faults at the SAME dispatch, which is why every single-site fix failed:
  (1) di is passed as a NEAR offset (int)di, but bd09's decompile derefs param_3 as a HOST pointer
      (*(byte*)(param_3+0x19)) -> first SIGSEGV.  (c31e full-ptr `dg+di` fixes THIS one, AZER1 diff=0.)
  (2) bd09.param_1 = c31e's live AX; b274 takes `byte *param_1` and receives word[DGROUP-table]+param_1,
      which is only a valid host pointer if AX is g_mem+0x1c000-based.  The c31e path does not guarantee
      that, so fixing (1) merely moves the crash into b274 -> second SIGSEGV (observed).

WHY NO SINGLE-SITE FIX EXISTS: Ghidra's __allregs model (a NO-OP macro; params are positional C args that
ApplyConv.java assigned from registers) PRUNES unused registers and RENUMBERS per function.  Evidence:
c31e's param_3 = SI (object), but bd09's param_3 = DI (object) -- the same "3rd param" slot maps to a
DIFFERENT register in each function.  So there is NO stable positional register->param map to marshal
through an indirect `call [vtable]`: each target has its own pruned map.  The terrain paint dispatcher
happens to hand bd09 host-pointer param_1 AND param_3 already (AZER1 works unpatched); c31e does not.

THE ONLY SOUND FIX (scoped, multi-session): a register-context marshaling layer for vtable dispatch --
give every icall-dispatched method the full GP-register context (ax,bx,cx,dx,si,di and the dword pairs)
and have each method read its params from that context by register, not by pruned position.  Equivalent to
restoring the __allregs "all-GP-in/out" ABI at the indirect-call boundary that Ghidra could not thread
because it did not know the target signature.  A per-target trampoline is the band-aid; the context ABI
is the fix.  Anything less reproduces one of the disproven single-site attempts.  Matrix intact 175/175.

FULL CASCADE MAPPED + bd09 MARSHALING WORKED (tested, reverted -- changes a passing flow unverifiably).
This round proved the mechanism end-to-end and destroyed two standing assumptions:

MECHANISM (definitive): __allregs is a NO-OP macro -> the decompile is plain cdecl.  c31e dispatches
`(*fn)((int)di)` -- ONE stack arg.  A vtable target that reads >1 param gets STACK GARBAGE for the rest.
Empirically confirmed: instrumenting bd09 on UKRAINE1 printed param_3=0xffe0fdf8 (a live STACK address,
not a near offset, not a host ptr) -- pure uninitialised-stack garbage.  Methods that read the object as
param_1 work by luck (di lands in param_1); bd09 reads object=param_3 AND param_1=ax, so it gets garbage.

ASSUMPTION 1 DESTROYED -- "the terrain path calls bd09 with a full ptr (polymorphism)":  FALSE.  bd09 has
NO direct C caller (only the icall fmap entry {0xbd09u,&FUN_0000_bd09}); it is 100% c31e-dispatched, type
0x1a only.  Patch 411's terrain regression came from its b274 edit (b274 HAS terrain-path callers
26190/26786), NOT from bd09.  So bd09 itself can be fixed freely.

ASSUMPTION 2 DESTROYED -- "the 5 passing terrain flows render bd09 correctly":  FALSE and important.  On
AZER1, c31e DOES dispatch bd09 (type 0x1a objects exist), but with the 1-arg call bd09.param_3 = stack
garbage whose byte[+0x19]&4 happened to be SET -> bd09 early-returns as a silent NO-OP.  The 5 "passing"
terrain flows are native==wasm but SKIP bd09's per-object work -> a concrete oracle-fidelity gap (see
board:0002).  Fixing bd09 changed AZER1 by 206 bytes (bd09 now does real work) -- unverifiable as
oracle-correct without the DOSBox oracle, so NOT landable this loop.

WORKED FIX (correct, reverted): c31e marshals the bd09 target with its true register signature
`((void(*)(int,undefined4,int))fn)((int)(uint16)param_1 /*ax*/, 0, (int)di /*object*/)`; bd09 rebases the
object via obj=dg+(uint16)param_3, computes the b274 arg as a host ptr dg+(uint16)(word[DG:..]+ax), and
passes ba33 the object as a NEAR offset (uint16)param_3 (ba33 wants near per patch 258).  param_2's high
word only feeds a gated damage event (be8b->e2c2), not sourced by c31e -> 0 (native==wasm-safe).  RESULT:
the crash moved OFF bd09 to the NEXT site -> the marshaling is correct, the cascade continues.

THE CASCADE (each site is the SAME host-vs-near object-ref polymorphism across the terrain vs mission
path, which is why every single-site fix either crashes downstream or regresses terrain):
  c31e  -- passes 1 arg; must marshal per-target register->param maps (bd09 done)
    -> bd09  -- object=param_3(near), param_1=ax; DONE (worked example above)
      -> b274  -- INTERNAL base-loss: `*(byte*)(DAT_2000_a3b2 + 0x16)` derefs a DGROUP near offset as a
                  host ptr (asm b289: mov si,[0xe3b2]; test [si+0x16],8).  fault 0xa371 = 0xa35b+0x16.
                  DAT_2000_a3b2 is written `= param_3` (30828) and `= param_4` (65283) -- host on the
                  terrain path (AZER1's b274 callers work), near on the mission path.  b274 is SHARED with
                  the terrain path -> touching it regressed patch 411.  Needs the object-ref represention
                  unified BEFORE b274, not a b274-local rebase.
      -> ... (further sites expected past b274)

CONCLUSION: the fix is not a chain of per-site rebases (each is polymorphic and shared) but a ONE-TIME
UNIFICATION of the object-reference representation across the whole object-method subsystem: pick the
port-natural full g_mem pointer for every object handle, make c31e (and the terrain paint dispatcher, and
the roster walkers) all produce that representation, and make every method+helper (bd09/b274/ba33/...)
consume it -- so no function is polymorphic across paths.  Multi-session, asm-anchored, individually
re-gated.  Matrix intact 175/175 (all prototypes reverted).


SHARED ROOT with board:0003 (2026-08-24): the __allregs-prune that this item names as the terrain
object-ref root ALSO causes board:0003's terrain FB code-layout-fragility -- fist_ext.c has 183 uninit
Ghidra register pseudo-vars (unaff_/extraout_/in_ESP) read as garbage; native (garbage, recompile-
dependent) vs wasm (spec-zero) diverge under any hot-path recompile (proven: -ftrivial-auto-var-init=zero
makes native terrain code-layout-invariant).  One fix -- restore the dropped register writes -- serves both.
