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
