/* Ghidra-decompilation compatibility layer for the Armored Fist ENGINE (FIST.DAT).
 *
 * Lets the pristine 16-bit-real-mode decompile (re_out/fist_decomp.c, via the mechanically
 * assembled re_out/fist.c) compile as native gcc -m32 / emcc C. Adapted from wasm-dd2's
 * ghidra_compat.h, but for a 16-bit DOS target (x86:LE:16 Real Mode) instead of a 32-bit PE.
 *
 * WIDTH DOCTRINE (faithfulness — read this):
 *   The engine is 16-bit. Ghidra typed register/word values as `undefined2`/`ushort`/`short`
 *   (16-bit) and composed far pointers / 20-bit linear addresses as `int`/`uint`/`undefined4`
 *   (32-bit, via CONCAT22 seg:off). gcc -m32 matches every one of these widths natively:
 *     char/byte/undefined1 = 8, short/ushort/undefined2 = 16, int/uint/undefined4 = 32.
 *   So the mapping below is width-faithful for values Ghidra typed explicitly. The ONE residual
 *   faithfulness gap is a value that must WRAP at 16 bits but which Ghidra held in `int`
 *   (native 32-bit) — the arithmetic then wraps at 2^32, not 2^16. Those are NOT silently
 *   "fixed" here; they are a Stage-2 patch class ("16-bit wraparound -> uint16_t"), same as the
 *   DD2 patch discipline. This header changes NO semantics; it only supplies types/macros.
 */
#ifndef GHIDRA_COMPAT_H
#define GHIDRA_COMPAT_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/* ---- Ghidra base integer typedefs (widths per the doctrine above) ---- */
typedef uint8_t  undefined;
typedef uint8_t  undefined1;
typedef uint16_t undefined2;
typedef uint32_t undefined3;   /* 3-byte value carried in a 32-bit slot (CONCAT31 result) */
typedef uint32_t undefined4;
typedef uint8_t  byte;
typedef uint8_t  uchar;
#ifndef __USE_MISC   /* if <sys/types.h> already provided these via __USE_MISC, don't clash */
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint32_t ulong;
#endif
typedef uint32_t uint3;
typedef int32_t  int3;
typedef int64_t  longlong;
typedef uint64_t ulonglong;
typedef int64_t  int5, int6, int7;
typedef uint64_t uint5, uint6, uint7;
typedef uint64_t undefined5, undefined6, undefined7, undefined8;

/* x87 80-bit gap-tier (FPU in_ST0..7 residuals). 128-bit where available so >>112 etc. compile;
 * on 32-bit gcc without __int128, fall back to 64-bit (dead on the paths that need exactness). */
typedef long double float10;
typedef long double longdouble;   /* Ghidra x87 80-bit type (driver units emit in_ST0/ST1 as this) */
#ifdef __SIZEOF_INT128__
typedef unsigned __int128 unkbyte10, unkuint10;
#else
typedef unsigned long long unkbyte10, unkuint10;
#endif

/* `code` is Ghidra's "this is executable, call through it" type. `code *p; (*p)(a,b,c);`
 * K&R empty parens = unspecified args, so a code* is callable with any argument list. */
typedef int code();

/* ---- calling-convention / attribute keywords Ghidra emits (no-ops on a flat target) ---- */
#define __allregs        /* the custom all-GP-registers-in/out prototype model (ApplyConv.java) */
#define __cdecl
#define __stdcall
#define __fastcall
#define __thiscall
#define __regparm3
#define near
#define far

/* ---- p-code CONCAT: little-endian concat of two parts into a wider value ----
 * CONCATab: `a` is the HIGH part (a bytes), `b` is the LOW part (b bytes). */
#define CONCAT11(a,b) (((uint16_t)(uint8_t)(a)  << 8)  | (uint8_t)(b))                 /* 1|1 -> 2 */
#define CONCAT12(a,b) (((uint32_t)(uint8_t)(a)  << 16) | (uint16_t)(b))                /* 1|2 -> 3(in u32) */
#define CONCAT21(a,b) (((uint32_t)(uint16_t)(a) << 8)  | (uint8_t)(b))                 /* 2|1 -> 3(in u32) */
#define CONCAT22(a,b) (((uint32_t)(uint16_t)(a) << 16) | (uint16_t)(b))                /* 2|2 -> 4  (seg:off) */
#define CONCAT31(a,b) (((uint32_t)((a) & 0xffffffu) << 8) | (uint8_t)(b))              /* 3|1 -> 4 */
/* extras (declared for completeness; harmless if unused) */
#define CONCAT13(a,b) (((uint32_t)(uint8_t)(a)  << 24) | ((b) & 0xffffffu))
#define CONCAT44(a,b) (((uint64_t)(uint32_t)(a) << 32) | (uint32_t)(b))
#define CONCAT24(a,b) (((uint64_t)(uint16_t)(a) << 32) | (uint32_t)(b))                 /* 2|4 -> 6(in u64) */
#define CONCAT26(a,b) (((uint64_t)(uint16_t)(a) << 48) | ((uint64_t)(b) & 0xffffffffffffULL)) /* 2|6 -> 8 */

/* ---- 32-bit-extender Ghidra intrinsics (FLAT32 unit; all off the KDV present path) ----
 * segment(seg,off): the x86 far-address builder -> a flat linear (the extender is flat).
 * LocalDescriptorTableRegister(x): LLDT (load LDT selector) -- a privileged no-op in our model. */
#define segment(s,o) ((uint32_t)((uint32_t)(uint16_t)(s) * 16u + (uint32_t)(o)))
#define LocalDescriptorTableRegister(x) ((void)(x))

/* ---- p-code SUB: extract a narrower value at byte offset o from a wider one ----
 * SUBwn(x,o): take n bytes out of a w-byte value x, starting at byte offset o. */
#define SUB21(x,o) ((uint8_t) ((uint16_t)(x) >> ((o) * 8)))   /* 2 -> 1 */
#define SUB42(x,o) ((uint16_t)((uint32_t)(x) >> ((o) * 8)))   /* 4 -> 2 */
#define SUB41(x,o) ((uint8_t) ((uint32_t)(x) >> ((o) * 8)))   /* 4 -> 1 */
#define SUB84(x,o) ((uint32_t)((uint64_t)(x) >> ((o) * 8)))   /* 8 -> 4 */

/* ---- zero/sign extension ---- */
#define ZEXT12(x) ((uint16_t)(uint8_t) (x))
#define ZEXT14(x) ((uint32_t)(uint8_t) (x))
#define ZEXT24(x) ((uint32_t)(uint16_t)(x))
#define SEXT12(x) ((uint16_t)(int8_t)  (x))
#define SEXT14(x) ((uint32_t)(int8_t)  (x))
#define SEXT24(x) ((uint32_t)(int16_t) (x))

/* ---- flag idioms (carry / signed-overflow) Ghidra emits for the residual flag pseudo-vars ----
 * Standard definitions; exact only matters at Stage 2 but they are correct as written. */
#define CARRY1(a,b)   ((((uint32_t)(uint8_t)(a)  + (uint32_t)(uint8_t)(b))  >> 8)  & 1u)
#define CARRY2(a,b)   ((((uint32_t)(uint16_t)(a) + (uint32_t)(uint16_t)(b)) >> 16) & 1u)
#define CARRY4(a,b)   ((uint32_t)(((uint64_t)(uint32_t)(a) + (uint64_t)(uint32_t)(b)) >> 32))
#define SCARRY1(a,b)  ((int)((((int8_t) (a) ^ (int8_t) ((a)+(b))) & ((int8_t) (b) ^ (int8_t) ((a)+(b)))) < 0))
#define SCARRY2(a,b)  ((int)((((int16_t)(a) ^ (int16_t)((a)+(b))) & ((int16_t)(b) ^ (int16_t)((a)+(b)))) < 0))
#define SCARRY4(a,b)  ((int)((((int32_t)(a) ^ (int32_t)((a)+(b))) & ((int32_t)(b) ^ (int32_t)((a)+(b)))) < 0))
#define SBORROW1(a,b) ((int)((((int8_t) (a) ^ (int8_t) (b)) & ((int8_t) (a) ^ (int8_t) ((a)-(b)))) < 0))
#define SBORROW2(a,b) ((int)((((int16_t)(a) ^ (int16_t)(b)) & ((int16_t)(a) ^ (int16_t)((a)-(b)))) < 0))
#define SBORROW4(a,b) ((int)((((int32_t)(a) ^ (int32_t)(b)) & ((int32_t)(a) ^ (int32_t)((a)-(b)))) < 0))
#define ABS(x)   ((x) < 0 ? -(x) : (x))
#define NAN(x)   ((x) != (x))

/* ==================================================================================
 * FLAT MEMORY MODEL  (the 16-bit-segmented -> portable-C bridge)
 * ----------------------------------------------------------------------------------
 * One flat byte array models the whole real-mode address space. A Ghidra global named
 * DAT_SEG_OFF (or FUN_SEG_OFF) lives at linear address (SEG << 4) + OFF; the assembler
 * (tools/assemble_fist.py) emits, into re_out/fist.c, one accessor #define per DAT symbol
 * that reads/writes g_mem at that linear address. FUN_SEG_OFF stay real C functions.
 *
 * Size: FIST_MEM_SIZE = 0x100000 = the full real-mode 1 MB. The engine image is 0x3390c
 * bytes (loaded into the low end at startup); DGROUP / stack (SS ~ 0x2ba9) / heap grow
 * above it, and the VGA frame buffer at linear 0xA0000 is inside the 1 MB — all covered.
 * The >1 MB A20-wrap / HMA region is NOT modeled (flag any access there as OPEN).
 * ================================================================================== */
/* The low 1 MB (0x100000) is the 16-bit engine's real-mode space.  Above it we host the
 * Doug-Huffman EXTENDER kernel module (the KDV intro-FMV player, 32-bit flat) + its heap:
 *   0x100000..0x110000  extender image (0xbf90) + FLAT32 off-image scratch (0x4000)
 *   0x110000..0x160000  extender MEMMGR heap (KDV chunk + 64000 decode + 768 palette buffers)
 * so g_mem grows to 0x180000.  The engine's own footprint (<= reg-file 0xf0014, VGA fb 0xA0000)
 * is unchanged and entirely below 0x100000 -- growing the array does not perturb it. */
#define FIST_MEM_SIZE 0x200000u
extern uint8_t g_mem[FIST_MEM_SIZE];
#define GMEM(lin) ((void *)(g_mem + (lin)))   /* linear address -> host pointer into g_mem */

/* ---- Doug-Huffman EXTENDER kernel module (re_out/fist_ext.c; FLAT32 assemble unit) ----
 * Compiled + linked as a THIRD module (like the .DVR overlays but 32-bit flat).  Its base-0
 * static refs are rebased to g_mem + fist_ext_base + off; runtime flat POINTER slots hold HOST
 * pointers into g_mem (native_main seeds framebuffer/TCB/heap).  Hosts the intro KDV player. */
#define FIST_EXT_BASE      0x100000u   /* g_mem linear where the extender image is placed */
#define FIST_EXT_IMG_SIZE  0xbf90u     /* re_out/fist_image.bin (49040 B) */
#define FIST_EXT_SPAN      0x10000u    /* image + FLAT32 off-image scratch window (fmap key range) */
#define FIST_EXT_HEAP      0x110000u   /* extender MEMMGR bump-heap base (linear) */
#define FIST_EXT_HEAP_TOP  0x1f0000u   /* extender MEMMGR bump-heap top  (linear).  Grown from 0x160000
                                        * to hold the extender task-setup allocator FUN_0000_84c0's
                                        * ~300 KB of persistent palette-reduction buffers (the 64 KB
                                        * color-distance matrix [0xbc90] + bc94/3909/390d/3918) PLUS the
                                        * per-map D32.KLC heightmap.  The map's full LOD build (detail
                                        * 2048^2 = 4 MB) needs a further multi-MB heap -- deferred with
                                        * the 643c buffered-read FILEMGR reversal (next frontier). */
extern uint32_t fist_ext_base;                         /* set at init (native_main) */
/* fist_ext_fmap[] / _n declared after `struct fist_fent` (below). */

/* ==================================================================================
 * DOS "voodoo" trap shims — HONEST STUBS for this (assemble) stage.
 *   swi(n)      Ghidra software-interrupt intrinsic  (INT n).  171 sites.
 *   in(port)    Ghidra port-input  intrinsic.                   31 sites.
 *   out(port,v) Ghidra port-output intrinsic.                   41 sites.
 * These become fist_dos.c / fist_vga.c / fist_input.c / fist_sb.c later. For now the
 * implementations live in tools/native_main.c and are declared here as no-ops that
 * announce themselves (return 0 / no state change) so the unit links and no behaviour is
 * silently invented. INT numbers seen: 0x21(DOS) 0x33(mouse) 0x2f 0x16 0x10 0x1..0x5.
 * Ports seen: 0x40-0x43(PIT) 0x60/0x64(kbd) 0x3c0/0x3c4/0x3c8/0x3c9/0x3da(VGA) 0x201(joy)
 * 0x6b/0x6c/0x8a/0x9f (DMA/PIC) plus SB/GUS. All enumerated as they surface.
 * ================================================================================== */
int  swi(int intno);         /* residual: un-threaded INT (data mis-read as code); see native_main */
int  in(int port);           /* port read  (fist_vga.c) */
void out(int port, int val); /* port write (fist_vga.c) */

/* INT dispatcher: the <callotherfixup targetop=swi> rewrites every `int n` to marshal registers
 * through the g_mem reg-file at linear 0xF0000.. and call this (rendered by Ghidra as a call to
 * magic address 0xF0100, mapped here by assemble_fist.py RULE 0). Implemented in fist_dos.c. */
void fist_int_dispatch(void);

/* ==================================================================================
 * INDIRECT-CALL DISPATCHER  (16-bit `call far/near [mem]` through g_mem-stored code ptrs)
 * ----------------------------------------------------------------------------------
 * The engine stores code pointers in g_mem and dispatches through them (jump tables,
 * saved-INT-vector chaining, engine-internal virtual dispatch). A raw stored value is a
 * 16-bit real-mode SEG:OFF (or a bare near OFFSET), NEVER a host address -- calling through
 * it directly jumps into a small integer and segfaults (the `call far [DGROUP:0x12] -> NULL`
 * crash). assemble_fist.py rewrites every such site to resolve the stored pointer's LINEAR
 * address ((SEG<<4)+OFF) through this dispatcher, which returns a real host `code *`:
 *   - linear matches a known engine FUN_ (fist_fmap[]) -> that C function;
 *   - linear in the saved-INT-vector MAGIC range -> a trampoline that chains fist_int_dispatch;
 *   - else -> an honest trap trampoline (logs the unmapped linear, returns 0 -- never a raw jump).
 * fist_icall_far(dword)  : dword = CONCAT22(seg,off); linear = seg*16 + off  (FAR call [mem]).
 * fist_icall_near(seg,off): linear = seg*16 + off, seg = enclosing function's CS (NEAR call [mem]).
 * fist_icall(linear)     : core resolver.
 * ================================================================================== */
struct fist_fent { uint32_t lin; void *fn; };       /* {linear (SEG<<4)+OFF -> &FUN} table entry */
extern const struct fist_fent fist_fmap[];           /* sorted by lin; emitted into fist.c */
extern const unsigned fist_fmap_n;
extern const struct fist_fent fist_ext_fmap[];       /* extender module {offset -> &FUN} map */
extern const unsigned fist_ext_fmap_n;
code *fist_icall(uint32_t linear);                   /* fist_icall.c */
code *fist_icall_far(uint32_t farptr);
code *fist_icall_near(uint16_t seg, uint16_t off);
void  fist_int_chain(unsigned vec);                  /* fist_dos.c: run the saved INT n handler */

/* ==================================================================================
 * MULTI-MODULE OVERLAY DISPATCH  (INT 21h AH=4B AL=03 driver overlays)
 * ----------------------------------------------------------------------------------
 * The engine loads two 16-bit MZ driver overlays at runtime -- MGAVIDEO.DVR (video)
 * and SOUNDDVR.DVR (sound) -- via INT 21h AH=4B AL=03 (load-overlay). fist_dos.c reads
 * the real .DVR, loads its load-module at the engine-supplied load_seg<<4, applies the
 * MZ relocations (factor = reloc from the param block), and REGISTERS it here as an
 * overlay with a runtime range [base, base+size). Each decompiled driver unit
 * (re_out/fist_mga.c / fist_snd.c, base-relative like the engine) exports its own
 * {module-offset -> &FUN} fmap + a base global; the assemble step wires the fmap and
 * base slot to the overlay's uppercase basename via fist_ovl_known[] below.
 *
 * fist_icall then dispatches a resolved LINEAR call target that falls inside a loaded
 * overlay's range to that module: module_off = linear - base -> the module fmap ->
 * &FUN (and sets the module's base global so its g_mem accessors resolve). An overlay
 * that is loaded but whose decompiled unit is not (yet) wired traps honestly with a
 * precise "overlay NAME+0xOFF (no fmap)" message -- never a silent skip or raw jump.
 * ================================================================================== */
struct fist_ovl {                       /* a loaded overlay's runtime record */
    char     name[16];                  /* uppercase basename, e.g. "MGAVIDEO.DVR" */
    uint32_t base;                      /* runtime linear load base = load_seg<<4 */
    uint32_t size;                      /* load-module size in bytes */
    const struct fist_fent *fmap;       /* module {offset -> &FUN} map (NULL until wired) */
    unsigned  fmap_n;
    uint32_t *base_slot;                /* the module unit's base global (accessors add it) */
    int       dumped;                   /* diagnostic: FIST_DUMP_VECS one-shot per module */
};
struct fist_ovl_known {                 /* compile-time driver unit registration (assemble-emitted) */
    const char *name;
    const struct fist_fent *fmap;
    const unsigned *fmap_n;
    uint32_t *base_slot;
};
extern const struct fist_ovl_known fist_ovl_known[];  /* NULL-name terminated; emitted into fist.c */
/* Register a just-loaded overlay (fist_dos.c AH=4B -> fist_icall.c registry). Returns 0 / -1. */
int  fist_ovl_register(const char *name, uint32_t base, uint32_t size);
/* Load a 16-bit MZ overlay file at load_seg<<4, apply its relocs (factor), register it (fist_dos.c). */
int  fist_load_overlay(const char *name, uint16_t load_seg, uint16_t reloc);

/* Cooperative timer pump (tools/native_main.c): runs the engine's installed INT-8 (PIT) ISR when a
 * host SIGALRM tick is pending, guarded against re-entry. Called from the engine's timer busy-waits
 * (FUN_1000_3346/3352, via patch 017) and from the port-I/O shims so DGROUP:0x452 advances -- the
 * single-threaded-host model of "the PIT IRQ fires asynchronously during execution". The ISR entry
 * linear is captured from DOS set-vector 0x08 (fist_dos.c) via fist_set_int8_handler(). */
void  fist_timer_pump(void);
void  fist_set_int8_handler(uint32_t linear);

/* Saved-INT-vector MAGIC: INT 21h AH=35 (get-vector) hands the engine a synthetic, CALLABLE
 * seg:off. seg = FIST_INTVEC_SEG, off = the interrupt number n. When the engine later far-calls
 * that saved pointer, fist_icall_far computes linear = FIST_INTVEC_LIN + n and routes it back to
 * fist_int_dispatch as "chain to INT n". FIST_INTVEC_LIN sits above the engine (<0x1c000) and the
 * reg-file (0xF0000..0xF0200), so it never collides with a real FUN_ or data linear. */
#define FIST_INTVEC_SEG 0xFE00u
#define FIST_INTVEC_LIN 0xFE000u

/* Doug-Huffman EXTENDER SERVICE GATE. The engine's env-config decoder FUN_0000_d99b decodes the
 * extender-crafted config blob (PSP:0x2c) into DGROUP:0xea14.. and, via FUN_0000_e339, far-calls the
 * decoded pointer [DGROUP:0xea16] with the arg block [ea1a..ea1e]. In the original that pointer is
 * the extender's real-mode->32-bit-PM callback trampoline (oracle capture: 0x0762:0x1179 = linear
 * 0x8799) into the Doug-Huffman extender's PM service dispatcher. In our port the extender role IS
 * the shim, so fist_icall routes this linear to fist_extender_gate() (native_main), which implements
 * the service (op = DGROUP:0xea10): op 0 = create-task (allocate + return a real task segment). The
 * blob is the pristine oracle capture, so the gate linear IS 0x8799; recognised here like the saved-
 * INT-vector magic above (0x8799 is mid-instruction of a FUN_ body -> never a real indirect target). */
#define FIST_EXTGATE_LIN 0x08799u
int fist_extender_gate(void);         /* native_main: the extender create-task/service handler */

/* ---- DOS loader environment (native_main sets these up before app_entry) ----
 * The Doug-Huffman extender/DOS loads the engine with a PSP + environment block and installs the
 * DGROUP service-vector table (relocated far-ptrs into the CRT service segment 0xf69). In our flat
 * base-0 model the loader role lives in native_main: it seeds a PSP at FIST_PSP_SEG (env seg at
 * PSP:0x2c) and the base-0-rebased DGROUP init image (oracle-confirmed, samples/dgroup_0x0_0x100.bin).
 * FIST_PSP_SEG is the DOS-entry ES = PSP segment the engine stores into DGROUP:0x68 (DAT_1000_c068). */
#define FIST_PSP_SEG 0x9800u          /* linear 0x98000: above heap-top 0x90000, below VGA 0xA0000 */
#define FIST_ENV_SEG 0x9900u          /* linear 0x99000: DOS environment block */
void fist_install_dgroup(void);       /* native_main: (re)install DGROUP service table after CRT clear */
void fist_ensure_dlist_vecs(void);    /* native_main: install display-list element method vectors (si=0x174) */

/* ---- other Ghidra pseudo-intrinsics the decompile emits ---- */
#define LOCK()   ((void)0)        /* x86 bus-LOCK prefix: no-op on a single-threaded host */
#define UNLOCK() ((void)0)
#define POPCOUNT(x) __builtin_popcountll((unsigned long long)(x))   /* x86 POPCNT */
#define ROUND(x)    __builtin_rint((double)(x))    /* x87 FRNDINT: round to nearest (current mode) */
#define builtin_strncpy(d,s,n) strncpy((char *)(d), (const char *)(s), (size_t)(n))
void halt_baddata(void);     /* HONEST STUB (native_main): decode reached bad/undefined data */
int  to_bcd(long long v);    /* x86 BCD conversion intrinsic (real impl in native_main) */

#endif /* GHIDRA_COMPAT_H */
