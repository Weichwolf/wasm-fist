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
    /* SOUND device-registration service fns UNRECOVERED by Ghidra -> loader-shim helpers (iter 8).
     * The driver init FUN_0000_0078 calls DGROUP:0xd4 (=0xf69:0x2287=0x11917=FUN_1000_1917 owner-tag
     * MEMMGR search) and DGROUP:0xf4 (=0xf69:0x19ea=0x1107a=FUN_1000_107a IRQ/timer-ISR register).
     * See fist_sb.c + docs/audio.md §14. */
    if (linear == 0x11917) { extern void fist_snd_1917(void); return (code *)fist_snd_1917; }
    if (linear == 0x1107a) { extern void fist_snd_107a(void); return (code *)fist_snd_107a; }
    /* MULTI-MODULE: a target inside a loaded overlay's range -> that module's FUN via its fmap. */
    for (int i = 0; i < fist_ovl_n; ++i) {
        struct fist_ovl *o = &fist_ovl_tab[i];
        if (linear >= o->base && linear < o->base + o->size) {
            uint32_t off = linear - o->base;             /* module-relative offset (base-0 fmap key) */
            if (o->fmap) {
                void *fn = lookup_ovl_fun(o, off);
                if (getenv("FIST_SND_CALLTRACE") && o->name[0]=='S' && o->name[1]=='O')
                    fprintf(stderr, "[sndcall] SOUNDDVR+0x%x %s\n", off, fn?"(fmap)":"(miss)");
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
    /* FIST_ROSTER (diagnostic, default OFF): the display-element dirty/paint walkers (208a/209e via
     * 206f; 2006/201a via 1ff5) store the current roster entry near-offset E in DAT_1000_fe08
     * (g_mem+0x1fe08) right before dispatching a per-element method.  The element's class base is
     * word0 = word[DGROUP:E]; its PAINT method = word[DGROUP:(word0+0x3e18)], its DIRTY method =
     * word[DGROUP:(word0+0x423c)].  This probe logs every dispatched (word0, method) pair to enumerate
     * the live roster.  KEY FINDING (this recon): the op-0x24 windshield poster 6f1f is the *DIRTY*
     * (+0x423c) method of class word0==0xe8 (slot DGROUP:0x4324 = 0x73b9 -> 6f1f), NOT a +0x3e18 paint
     * method of class 0x50c as previously believed.  Class 0xe8 is ABSENT from the port's mission
     * roster (FIST_ROSTER_SCAN confirms no word0==0xe8 element) because the in-mission game loop
     * FUN_0000_e4bb -- which builds the 0x2d1d cockpit template that carries the windshield element --
     * is never reached (its marker c78c stays 0x50, not 0x100).  Read-only, behaviour-neutral. */
    static int rst = -1;
    extern int g_fist_after_map;
    if (rst < 0) rst = getenv("FIST_ROSTER") ? 1 : 0;
    /* FIST_ROSTER_SCAN: one-shot scan (at the cockpit-paint dispatch off==0x4937) of the display-list
     * working array DGROUP:0x3800..0x4000 for any element whose word0 (class base) is 0xe8 (the
     * player-vehicle windshield-viewport class -> dirty 0x73b9 -> 6f1f -> df0e -> op 0x24).  Also prints
     * the e4bb marker c78c (0x100 iff the in-mission game loop ran) and the 0x2d1d cockpit-template
     * descriptor.  Read-only. */
    if (rst && getenv("FIST_ROSTER_SCAN") && g_fist_after_map && off == 0x4937) {
        static int scanned = 0;
        if (!scanned) { scanned = 1;
            uint8_t *dg = g_mem + 0x1c000;
            fprintf(stderr, "[rscan] e4bb-marker c78c=0x%04x  2d1d-desc srcseg[+4]=0x%04x cnt[+8]=0x%04x\n",
                *(uint16_t*)(dg+0x78c), *(uint16_t*)(dg+0x2d1d+4), *(uint16_t*)(dg+0x2d1d+8));
            fprintf(stderr, "[rscan] scanning DGROUP:0x3800..0x4000 for word0==0xe8 ...\n");
            for (uint32_t o = 0x3800; o < 0x4000; o += 2) {
                uint16_t w = *(uint16_t *)(dg + o);
                if (w == 0xe8)
                    fprintf(stderr, "[rscan]  @0x%04x word0=0xe8 rec: %04x %04x %04x %04x %04x %04x\n", o,
                        *(uint16_t*)(dg+o), *(uint16_t*)(dg+o+2), *(uint16_t*)(dg+o+4),
                        *(uint16_t*)(dg+o+6), *(uint16_t*)(dg+o+8), *(uint16_t*)(dg+o+0xa));
            }
            fprintf(stderr, "[rscan] done.\n");
        }
    }
    if (rst && seg == 0 && (!getenv("FIST_ROSTER_MISSION") || g_fist_after_map)) {
        uintptr_t base = (uintptr_t)g_mem;
        uintptr_t fe08 = (uintptr_t)(uint32_t)(*(uint32_t *)(g_mem + 0x1fe08));  /* piVar host ptr */
        (void)base;
        /* fe08 holds a rebased NEAR offset E (small).  Roster entry at DGROUP:E -> class base word0 =
         * word[DGROUP:E]; method = word[DGROUP:(word0 + 0x3e18)] (paint) or +0x423c (dirty). */
        uint16_t E = (uint16_t)fe08;
        if (fe08 < 0x1c000) {                 /* near offset, not a host pointer */
            uint8_t *dg = g_mem + 0x1c000;
            uint16_t word0 = *(uint16_t *)(dg + E);
            if ((uint32_t)word0 + 0x4240 < 0x1c000) {   /* class base near, in DGROUP range */
                uint16_t slotP = *(uint16_t *)(dg + word0 + 0x3e18);
                uint16_t slotD = *(uint16_t *)(dg + word0 + 0x423c);
                int isP = (slotP == off), isD = (slotD == off);
                if (isP || isD) {
                    static uint32_t seen[1024]; static int nseen; int dup = 0;
                    uint32_t key = ((uint32_t)word0 << 16) | off;
                    for (int i = 0; i < nseen; i++) if (seen[i] == key) { dup = 1; break; }
                    if (!dup) {
                        if (nseen < 1024) seen[nseen++] = key;
                        fprintf(stderr, "[roster] E=0x%04x word0=0x%04x paint(+3e18@0x%04x)=0x%04x dirty(+423c@0x%04x)=0x%04x -> %s 0x%04x\n",
                                E, word0, (uint16_t)(word0+0x3e18), slotP, (uint16_t)(word0+0x423c), slotD, isP?"PAINT":"DIRTY", off);
                    }
                }
            }
        }
    }
    /* FIST_CURTRACE (diagnostic, default OFF): trace the 22dd render-pass cursor-walk handler sequence.
     * 22dd dispatches `fist_icall_near(0, word[DGROUP:word[DGROUP:0x4a86]])` -- the cursor 0x4a86 points
     * into a per-render-PHASE script array (word[0x4a88+d549]); each handler advances 0x4a86 by 2.  Log
     * every distinct near target after map-load with the current d548/d549/a814 so we can see which render
     * PHASE the walk runs.  KEY (this iteration, oracle-proven): the original's settled cockpit renders in
     * render phase d549=0x1c (its script @0x6c82 includes the object renderer 0x286e), while the port stays
     * in phase 0x1e (d549=0x1e from 67e3) because it reads d548 as a positive WORD -> 22dd never RESEEDs
     * (never consults d549).  Read-only, behaviour-neutral. */
    if (seg == 0 && getenv("FIST_CURTRACE")) {
        extern int g_fist_after_map;
        if (g_fist_after_map) {
            uint8_t *dg = g_mem + 0x1c000;
            uint16_t cur = *(uint16_t *)(dg + 0x4a86);
            /* only log the 22dd-family script handlers (offsets < 0x4400, the render-pass range) */
            if (off < 0x4400) {
                static uint32_t seen[2048]; static int nseen; int dup = 0;
                for (int i = 0; i < nseen; i++) if (seen[i] == off) { dup = 1; break; }
                if (!dup) {
                    if (nseen < 2048) seen[nseen++] = off;
                    fprintf(stderr, "[cur] tgt=0x%04x cursor=0x%04x d548=0x%02x d549=0x%02x a814=%d\n",
                        off, cur, *(uint8_t*)(dg+0x1548), *(uint8_t*)(dg+0x1549), *(uint8_t*)(dg+0xe814));
                }
            }
        }
    }
    /* FIST_20EA (diagnostic, default OFF): the FUN_0000_20ea display-list child-walk (patch 142) sets
     * DGROUP:0x3e08 = bx (child cursor) right before each dispatch of word[DGROUP:(word[DGROUP:bx]+0x444e)].
     * A near call satisfying off == that formula IS a 20ea dispatch -> log (bx,node,off) so the native-vs-
     * wasm dispatch sequence can be diffed: the first divergence pins the corrupt display node.  Read-only. */
    { static int t20 = -1; if (t20 < 0) t20 = getenv("FIST_20EA") ? 1 : 0;
      if (t20 && seg == 0) {
        uint8_t *dg = g_mem + 0x1c000;
        uint16_t bx = *(uint16_t *)(dg + 0x3e08);
        uint16_t node = *(uint16_t *)(dg + bx);
        uint16_t expect = *(uint16_t *)(dg + (uint16_t)(node + 0x444e));
        if (off == expect) {
            static long n; if (n < 200)
                fprintf(stderr, "[20ea] #%ld bx=0x%04x node=0x%04x off=0x%04x %s\n",
                    n, bx, node, off, (off >= 0x1c000 || off < 0x40) ? "WILD" : "");
            n++;
        }
      } }
    return fist_icall(((uint32_t)seg << 4) + off);
}

long fist_icall_trapcount(void){ return g_trap_count; }
