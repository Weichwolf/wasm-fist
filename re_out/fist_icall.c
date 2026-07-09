/* re_out/fist_icall.c -- Armored Fist indirect-call dispatcher.
 *
 * Runtime target of the `call far/near [mem]`-through-g_mem-stored-code-pointer rewrites that
 * assemble_fist.py emits (see ghidra_compat.h "INDIRECT-CALL DISPATCHER"). A 16-bit real-mode
 * stored code pointer is a SEG:OFF (far) or a bare OFFSET (near, implied CS = the caller's code
 * segment) -- it is NEVER a host address. This file turns a resolved LINEAR target ((SEG<<4)+OFF)
 * into a real host `code *` the call site can invoke, WITHOUT ever jumping to a raw stored value:
 *   1. linear in the engine FUN_ map (fist_fmap[], binary search) -> that C function;
 *   2. linear in the saved-INT-vector MAGIC range -> a trampoline that chains fist_int_dispatch;
 *   3. otherwise -> an honest trap trampoline (log the unmapped linear once, return 0 safely).
 *
 * This resolves BOTH the engine's own stored-pointer dispatch AND the saved-DOS-vector chaining
 * (INT 21h AH=35 get-vector hands out FIST_INTVEC_SEG:n; the engine's later far-call lands here).
 */
#include "ghidra_compat.h"
#include <stdio.h>
#include <stdlib.h>

/* ---- trace ---- */
static int g_trace = -1;
static int traceon(void){ if(g_trace<0) g_trace = getenv("FIST_TRACE_TRAPS")?1:0; return g_trace; }

/* ---- binary search of the sorted {linear -> &FUN} map emitted into fist.c ---- */
static void *lookup_fun(uint32_t lin)
{
    unsigned lo = 0, hi = fist_fmap_n;
    while (lo < hi) {
        unsigned mid = lo + ((hi - lo) >> 1);
        uint32_t m = fist_fmap[mid].lin;
        if (m == lin) return fist_fmap[mid].fn;
        if (m < lin)  lo = mid + 1; else hi = mid;
    }
    return NULL;
}

/* ---- INT-chain trampoline: run the saved DOS INT n handler (== our shim for that vector) ---- */
static unsigned g_pending_vec;   /* set by fist_icall immediately before the site calls the tramp */
static int int_chain_tramp(void){ fist_int_chain(g_pending_vec); return 0; }

/* ---- honest trap trampoline: an unmapped indirect target. Log (deduped) + return 0, never jump. */
static uint32_t g_trap_lin;
static long     g_trap_count;
static uint32_t g_trap_seen[512];
static int      g_trap_nseen;
static int trap_tramp(void)
{
    ++g_trap_count;
    if (traceon()) {
        int dup = 0;
        for (int i = 0; i < g_trap_nseen; ++i) if (g_trap_seen[i] == g_trap_lin) { dup = 1; break; }
        if (!dup) {
            if (g_trap_nseen < (int)(sizeof g_trap_seen/sizeof g_trap_seen[0]))
                g_trap_seen[g_trap_nseen++] = g_trap_lin;
            fprintf(stderr, "[icall] TRAP unmapped indirect call -> linear 0x%05x "
                            "(no FUN_ there; returning 0)\n", g_trap_lin);
        }
    }
    return 0;
}

/* ---- overlay/driver-module registry (INT 21h AH=4B AL=03 loaded modules) ---- */
#define FIST_MAX_OVL 8
struct fist_ovl fist_ovl_tab[FIST_MAX_OVL];
int fist_ovl_n;

/* case-insensitive basename compare (DOS names are upper-case on disk) */
static int name_eq(const char *a, const char *b){
    for (; *a && *b; ++a, ++b){
        int ca = *a, cb = *b;
        if (ca>='a'&&ca<='z') ca-=32; if (cb>='a'&&cb<='z') cb-=32;
        if (ca != cb) return 0;
    }
    return *a == *b;
}

/* Register a just-loaded overlay; wire its decompiled fmap + base slot from fist_ovl_known[] if the
 * driver unit is compiled into this build. Sets the module's base global so its (base-relative)
 * g_mem accessors resolve at the runtime load address. */
int fist_ovl_register(const char *name, uint32_t base, uint32_t size)
{
    if (fist_ovl_n >= FIST_MAX_OVL) return -1;
    struct fist_ovl *o = &fist_ovl_tab[fist_ovl_n++];
    size_t k=0; for (; name[k] && k<sizeof(o->name)-1; ++k) o->name[k]=name[k]; o->name[k]=0;
    o->base = base; o->size = size; o->fmap = NULL; o->fmap_n = 0; o->base_slot = NULL;
    for (const struct fist_ovl_known *w = fist_ovl_known; w->name; ++w){
        if (name_eq(w->name, o->name)){
            /* wire only if the driver unit is actually compiled in (weak-undefined -> NULL slots) */
            if (w->fmap && w->fmap_n && w->base_slot){
                o->fmap = w->fmap; o->fmap_n = *w->fmap_n; o->base_slot = w->base_slot;
                *w->base_slot = base;                       /* module accessors add this base */
            }
            break;
        }
    }
    if (traceon())
        fprintf(stderr, "[icall] overlay '%s' registered: base 0x%05x size 0x%x fmap=%s(%u fns)\n",
                o->name, base, size, o->fmap ? "wired" : "NONE", o->fmap_n);
    return 0;
}

/* binary search a module's {module-offset -> &FUN} map */
static void *lookup_ovl_fun(const struct fist_ovl *o, uint32_t off){
    unsigned lo=0, hi=o->fmap_n;
    while (lo<hi){ unsigned mid=lo+((hi-lo)>>1); uint32_t m=o->fmap[mid].lin;
        if (m==off) return o->fmap[mid].fn; if (m<off) lo=mid+1; else hi=mid; }
    return NULL;
}

/* honest overlay-miss trampoline: target is inside a loaded overlay but not (yet) wired/mapped. */
static char g_ovl_miss[64];
static int ovl_miss_tramp(void){
    if (traceon()) fprintf(stderr, "[icall] TRAP overlay call -> %s (no fmap entry; returning 0)\n", g_ovl_miss);
    return 0;
}

/* DIAGNOSTIC (FIST_DUMP_VECS=1): on the first overlay miss, scan the engine DGROUP for every far
 * vector (off:seg) whose seg lands inside a loaded overlay's range, and print the (module-relative)
 * offsets.  This enumerates every driver method vector the init installed -> the seed list to feed
 * SeedDriverVecs (FIST_DRIVER_SEED_OFFS) so those offsets become driver functions in the fmap. */
extern unsigned char g_mem[];
static void dump_module_vecs(struct fist_ovl *o){
    if (o->dumped) return; o->dumped = 1;
    if (!getenv("FIST_DUMP_VECS")) return;
    const uint32_t DG = 0x1c000;
    uint16_t seg_lo = (uint16_t)(o->base >> 4);
    uint16_t seg_hi = (uint16_t)((o->base + o->size) >> 4);
    fprintf(stderr, "[vecdump] %s base 0x%05x seg [0x%04x..0x%04x]:\n", o->name, o->base, seg_lo, seg_hi);
    for (uint32_t a = DG; a < DG + 0x1000; a += 2){
        uint16_t off = *(uint16_t*)(g_mem + a);
        uint16_t seg = *(uint16_t*)(g_mem + a + 2);
        if (seg >= seg_lo && seg <= seg_hi){
            uint32_t lin = ((uint32_t)seg << 4) + off;
            if (lin >= o->base && lin < o->base + o->size)
                fprintf(stderr, "[vecdump]   DGROUP:0x%03x = %04x:%04x -> +0x%x\n",
                        a - DG, seg, off, lin - o->base);
        }
    }
}

/* ---- core resolver: linear target -> host code* ---- */
code *fist_icall(uint32_t linear)
{
    if (linear >= FIST_INTVEC_LIN && linear < FIST_INTVEC_LIN + 0x100) {
        g_pending_vec = linear - FIST_INTVEC_LIN;
        return (code *)int_chain_tramp;
    }
    /* Doug-Huffman extender service gate (e339's `lcall [DGROUP:0xea16]`) -> the shim handler. */
    if (linear == FIST_EXTGATE_LIN)
        return (code *)fist_extender_gate;
    /* MULTI-MODULE: a target inside a loaded overlay's range -> that module's FUN via its fmap. */
    for (int i = 0; i < fist_ovl_n; ++i) {
        struct fist_ovl *o = &fist_ovl_tab[i];
        if (linear >= o->base && linear < o->base + o->size) {
            uint32_t off = linear - o->base;             /* module-relative offset (base-0 fmap key) */
            if (o->fmap) {
                void *fn = lookup_ovl_fun(o, off);
                if (fn) return (code *)fn;
                /* FAR-ENTRY THUNK.  Many driver method vectors point at a 4-byte far wrapper
                 *   E8 rel16 CB   =  call near <inner> ; lret
                 * around a decompiled near function (the driver's public far entry point).  Ghidra
                 * recovers the inner near function but not the thunk, so the thunk offset misses the
                 * fmap.  Resolve it faithfully to the inner near function -- executing the thunk IS a
                 * near-call into that function then a far-return, which the inner __allregs C function
                 * already models.  (e.g. MGAVIDEO 0x2d1b -> call 0x2d1f -> FUN_0000_2d1f.) */
                if (off + 4 <= o->size && !getenv("FIST_NO_THUNK")) {
                    unsigned char *b = g_mem + linear;
                    if (b[0] == 0xE8 && b[3] == 0xCB) {
                        int rel = (short)(b[1] | (b[2] << 8));
                        uint32_t inner = (off + 3 + rel) & 0xFFFF;
                        void *fn2 = lookup_ovl_fun(o, inner);
                        if (fn2) return (code *)fn2;
                    }
                }
                /* LEADING-NOP MID-ENTRY.  Some registered method vectors point one or more bytes
                 * past the recovered function start when Ghidra pinned the start on a leading `nop`
                 * (0x90) it emitted before the real entry -- e.g. the mouse-cursor handlers d5ac =
                 * MGAVIDEO 0x2f04 = FUN_0000_2f03 + 1 (the 0x2f03 byte is a nop).  Executing from the
                 * function start runs the harmless nop(s) then the identical body, so resolve to the
                 * containing function iff every intervening byte is a nop. */
                for (uint32_t d = 1; d <= 4 && d <= off; ++d) {
                    if (g_mem[linear - d] != 0x90) break;
                    void *fn3 = lookup_ovl_fun(o, off - d);
                    if (fn3) return (code *)fn3;
                }
            }
            dump_module_vecs(o);
            snprintf(g_ovl_miss, sizeof g_ovl_miss, "%s+0x%x", o->name, off);
            return (code *)ovl_miss_tramp;
        }
    }
    void *fn = lookup_fun(linear);
    if (fn) return (code *)fn;
    /* leading-nop mid-entry into an engine function (same class as the overlay case above). */
    for (uint32_t d = 1; d <= 4 && d <= linear; ++d) {
        if (g_mem[linear - d] != 0x90) break;
        void *fn2 = lookup_fun(linear - d);
        if (fn2) return (code *)fn2;
    }
    g_trap_lin = linear;
    return (code *)trap_tramp;
}

/* FAR call [mem]: the stored dword is CONCAT22(seg,off) -> linear = seg*16 + off. */
code *fist_icall_far(uint32_t farptr)
{
    uint16_t seg = (uint16_t)(farptr >> 16), off = (uint16_t)farptr;
    return fist_icall(((uint32_t)seg << 4) + off);
}

/* NEAR call [mem]: only the offset is stored; the segment is the caller's CS (passed by the site). */
code *fist_icall_near(uint16_t seg, uint16_t off)
{
    return fist_icall(((uint32_t)seg << 4) + off);
}

long fist_icall_trapcount(void){ return g_trap_count; }
