Type: bug
Area: decompile
Tags: wasm-divergence undefined-behaviour
Title: every length and loop counter the engine computes is an integer, not a pointer

Every length, count and loop counter in the port is an integer type, so that no loop bound depends on
pointer arithmetic that never denotes an object and the two compilers (gcc -O0 native, clang/binaryen
-O2 wasm) produce the same control flow.

## Why this is a capability

Ghidra renders `repnz scasb` / `sub cx,di` idioms as pointer differences and keeps the result typed
as a pointer: `pcVar7 + (-1 - (int)pcVar10)`, compared against `(char *)0x9`, decremented to
`(char *)0x0` in a `for`.  gcc -O0 computes it literally.  The wasm backend at -O2 is entitled to
assume a pointer derived from an object plus an offset is never null: the `!= (char *)0x0` bound
folds to `true`, a backward scan runs below its buffer until it meets the byte it looks for anywhere
in DGROUP, a copy loop loses its exit and is emitted as `loop { store8 ; br 0 } ; unreachable`.  The
result is a native/wasm DIVERGENCE that the frame compare does not see when the affected value is a
bookkeeping record, and a "memory access out of bounds" when it is not.

## Found (patch 589)

21bd, the base-name builder of the driver loader: on wasm the `\` scan yielded a stem of 191 for
"BACKLAND.BIN" and skipped the copy (native copied "BACKLAND"), and with [0x550] uninstalled (the
audio flows after 586) the copy loop ran off the end of linear memory.  Rewritten in 16-bit offsets
as the asm has it; `mov ax,0x2020` had also become `(undefined2 *)&DAT_1000_e020` (the low 16 bits of
a host address written as the eight-space fill).

## The audit

`grep -n '(char \*)0x[0-9a-f]\|(undefined2 \*)0x\|(int \*)0x\|(byte \*)0x\|(uint \*)0x' build/fist.c`
minus the `(uintptr_t)` casts the patches introduced deliberately: 185 sites in ~60 functions at
HEAD (2891 x15, 1bd8 x10, ae3c/4e5b/4237 x6, b7c4/56f8/fd79 x5, 2e0d/1f43/4268/32a6/2486 x4, ...).
Most are near offsets passed as pointer-typed arguments (harmless: no arithmetic).  The class to fix
is the subset used as a bound or a counter: a pointer compared with a small constant, a pointer
decremented in a loop step, a pointer difference added to a pointer.  Each is an asm read of the
`scas`/`cmp cx` it came from; the native==wasm gate on the flow that reaches it is the proof.

b355 (patch 593) is the class's sim-side case: `inc word [di+0x1b] ; test word [di+0x1b],0x3f ; mov
ax,[di+0x1d] ; cmp ax,0x80 ; jbe` -- a 16-bit counter and a 16-bit throttle that Ghidra read as 32-bit
ints, so the throttle compare took the two bytes above it along and fired the type-0x11 child spawn
(and its RNG draw) at the fifth sim step of AZER1, where the original's word 0 does not.  The first
divergence the seeded replay (board:0017) reported.
