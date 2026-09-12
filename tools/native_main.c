#define _GNU_SOURCE
/* Native 32-bit runtime harness for the Armored Fist ENGINE (re_out/fist.c).
 *
 * Owns the flat memory image (g_mem), loads the extracted engine image, sets up the INT-exit
 * unwind (INT 21h/4Ch and the FIST_MAXINTS safety cap longjmp back here), drives the engine entry
 * point app_entry(), then dumps the VGA framebuffer (FIST_FBDUMP=path) so the rendered frame is
 * observable. The DOS/BIOS/mouse INT handlers live in fist_dos.c; VGA + port I/O in fist_vga.c.
 *
 * Build: tools/build_native.sh   (gcc -m32)
 */
#include "ghidra_compat.h"
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>

/* Portability seam: the SIGSEGV/backtrace + mprotect(FIST_FBTRAP) diagnostics and the SIGALRM/setitimer
 * host timer are host-OS (Linux) facilities absent under emscripten/wasm.  Under __EMSCRIPTEN__ they are
 * compiled out and the deterministic seam (fist_timer_pump / FIST_TICK_HZ) drives the tick cooperatively
 * (one tick per pump).  Everything else (setjmp/longjmp exit, gettimeofday watchdog, the shims) is
 * portable C shared byte-for-byte between the native and wasm builds -- so the frame they render is too. */
#ifndef __EMSCRIPTEN__
#include <execinfo.h>
#include <sys/mman.h>
#endif

/* Diagnostic SIGSEGV backtrace (gated by FIST_SEGV_BT) -- gdb can't keep up with the fast tick. */
extern long fist_dump_framebuffer(const char *path);   /* fist_vga.c (fwd for segv_bt) */
extern uint32_t fist_mga_base;                          /* re_out/fist_mga.c (mga overlay load offset) */
extern int  fist_vga_mode(void);

/* -----------------------------------------------------------------------------------------------
 * fist_preload_blockA -- reproduce the extender's block-A preload (the COLORMAP blend matrix).
 *
 * GROUND TRUTH (docs/oracle_bc90_capture.md; LIVE build-time DOSBox capture): the 64K colour-blend
 * matrix bdc4 reads at ext-flat [bc90]=0x10000 ("block A", 256 distinct) is NOT built by bc9c at
 * map-load -- it is PRELOADED before the menu by a `rep movsd` memcpy in the Doug-Huffman extender's
 * own resident code (lin 0x4708, cs 0x0008), src fs:[0x1f0], filling all 64K.  That extender code is
 * NOT part of fist_image.bin (which is only the 32-bit APP image, data-base file 0x583a) -- the
 * extender's role in this port is the hand-written shim, so the preload is reproduced here.
 *
 * The 64K SOURCE is BAKED in the game's own data file DSOUNDS.BIN at offset 0xe00 -- proven byte-exact
 * vs tools/oracle/samples/oracle_bdc4_matrix_blockA_0x141000.bin.  The engine (FIST.DAT; the "DSOUNDS"
 * string is at fist_dat_image.bin 0x306) opens DSOUNDS.BIN at boot and the extender copies its [0xe00:]
 * region into [bc90].  We reproduce that copy from the REAL DSOUNDS.BIN (faithful: the game's own data,
 * NOT a hardcoded oracle-sample paste).  Returns the number of bytes copied.
 */
static long fist_preload_blockA(void *dst)
{
    if (!dst) return 0;
    const char *dd = getenv("FIST_DATADIR"); if (!dd) dd = "armoredfist";
    char p[512]; FILE *f = 0;
    const char *sub[] = { "/FISTDATA/DSOUNDS.BIN", "/DSOUNDS.BIN", 0 };
    for (int i = 0; sub[i] && !f; i++) { snprintf(p, sizeof p, "%s%s", dd, sub[i]); f = fopen(p, "rb"); }
    if (!f) f = fopen("armoredfist/FISTDATA/DSOUNDS.BIN", "rb");
    if (!f) { fprintf(stderr, "[blockA] DSOUNDS.BIN not found under datadir '%s'\n", dd); return 0; }
    long n = 0;
    if (fseek(f, 0xe00, SEEK_SET) == 0) n = (long)fread(dst, 1, 0x10000, f);
    fclose(f);
    if (n != 0x10000) fprintf(stderr, "[blockA] short read %ld/65536 from DSOUNDS.BIN\n", n);
    return n;
}

#ifndef __EMSCRIPTEN__
/* ---- FIST_FBTRAP: write-protect a framebuffer page to catch whoever draws the menu box ---- */
volatile int   g_fbtrap_req = 0;          /* set at menu-enter */
static   void *g_fbtrap_page = 0;
static   int   g_fbtrap_hits = 0;
void fbtrap_arm_hook(void) {
    if (!g_fbtrap_req || g_fbtrap_page) return;   /* only when requested and not already armed */
    const char *e = getenv("FIST_FBTRAP"); if (!e) { g_fbtrap_req = 0; return; }
    long ps = sysconf(_SC_PAGESIZE);
    long off = atol(e);                            /* fb byte offset to guard (e.g. row*320+col) */
    uintptr_t a = (uintptr_t)(g_mem + 0xA0000 + off);
    g_fbtrap_page = (void*)(a & ~(uintptr_t)(ps-1));
    mprotect(g_fbtrap_page, ps, PROT_READ);
}
/* ---- FIST_OBJTRAP: single-step mprotect watchpoint on a DGROUP object word (catch the crash-bucket
   wild-writer at full native speed; gdb-on-mission can't reach op-0x2c in-budget). Arm at the op-0x2c gate
   on g_mem+0x1c000+<off>; every write to that page faults -> log the writer EIP for writes to the exact
   target word, single-step past it, re-protect. */
static void    *g_objtrap_page = 0;
static uintptr_t g_objtrap_target = 0;
static unsigned long g_objtrap_pend_eip = 0;
void objtrap_arm(unsigned off) {
    if (g_objtrap_page) return;
    long ps = sysconf(_SC_PAGESIZE);
    uintptr_t a = (uintptr_t)(g_mem + 0x1c000 + off);
    g_objtrap_target = a;
    g_objtrap_page = (void*)(a & ~(uintptr_t)(ps-1));
    mprotect(g_objtrap_page, ps, PROT_READ);
}
static void wwtrap_reprotect(void);
void objtrap_trap(int sig, siginfo_t *si, void *uc) {   /* SIGTRAP: after the single-stepped write */
    ucontext_t *u = (ucontext_t *)uc;
    wwtrap_reprotect();                                                             /* re-arm wwtrap range */
    if (g_objtrap_page) mprotect(g_objtrap_page, sysconf(_SC_PAGESIZE), PROT_READ);  /* re-arm */
    if (g_objtrap_pend_eip) {
        unsigned short v = *(unsigned short *)g_objtrap_target;
        unsigned char *dg_ = g_mem + 0x1c000;
        fprintf(stderr, "[objtrap] dg:%04lx <- 0x%04x  by EIP=0x%08lx  t=%u clock=%02x:%02x:%02x\n",
                (unsigned long)(g_objtrap_target - (uintptr_t)dg_), v, g_objtrap_pend_eip,
                *(uint16_t *)(dg_ + 0x452), dg_[0x6da6], dg_[0x6da7], dg_[0x6da8]);
        g_objtrap_pend_eip = 0;
    }
    u->uc_mcontext.gregs[16 /*REG_EFL*/] &= ~0x100UL;   /* clear TF */
}
/* ---- FIST_WWTRAP: write-protect a RANGE of g_mem (the static sprite-data region) at the op-0x2c gate to
   catch the crash-bucket wild-writer whose target is garbage-dependent. Any in-mission write to the range
   faults -> log EIP + fault addr + ret-cands, unprotect the whole range, single-step (TF) past the write,
   re-protect in the SIGTRAP handler. FIST_WWTRAP="<hexoff>:<hexlen>" (g_mem-relative). */
static void    *g_wwtrap_base = 0;   /* page-aligned range start */
static size_t   g_wwtrap_len  = 0;   /* page-rounded length */
static int      g_wwtrap_hits = 0;
static void wwtrap_reprotect(void){ if (g_wwtrap_base) mprotect(g_wwtrap_base, g_wwtrap_len, PROT_READ); }
void wwtrap_arm(unsigned off, unsigned len) {
    if (g_wwtrap_base) return;
    long ps = sysconf(_SC_PAGESIZE);
    uintptr_t a = (uintptr_t)(g_mem + off);
    uintptr_t b = a & ~(uintptr_t)(ps-1);
    uintptr_t e = (a + len + ps - 1) & ~(uintptr_t)(ps-1);
    g_wwtrap_base = (void*)b; g_wwtrap_len = (size_t)(e - b);
    mprotect(g_wwtrap_base, g_wwtrap_len, PROT_READ);
    fprintf(stderr, "[wwtrap] armed RO [g_mem+0x%lx .. +0x%lx)\n",
            (unsigned long)(b-(uintptr_t)g_mem), (unsigned long)(e-(uintptr_t)g_mem));
}

static void segv_bt(int sig, siginfo_t *si, void *uc) {
    ucontext_t *u = (ucontext_t *)uc;
    unsigned long eip = (unsigned long)u->uc_mcontext.gregs[14 /*REG_EIP*/];
    /* wwtrap: fault on the guarded sprite-data range -> log the writer, single-step past it, re-protect */
    if (g_wwtrap_base && si->si_addr >= g_wwtrap_base &&
        (char*)si->si_addr < (char*)g_wwtrap_base + g_wwtrap_len) {
        unsigned long esp2 = (unsigned long)u->uc_mcontext.gregs[7];
        unsigned long *sp2 = (unsigned long *)esp2;
        fprintf(stderr, "[wwtrap] WRITE to g_mem+0x%lx  EIP=0x%08lx  ret-cands:",
                (unsigned long)((char*)si->si_addr - (char*)g_mem), eip);
        int sh=0; for (int i=0;i<80 && sh<10;i++){unsigned long v=sp2[i]; if(v>0x08048000&&v<0x08800000){fprintf(stderr," 0x%08lx",v);sh++;}}
        fprintf(stderr, "\n");
        mprotect(g_wwtrap_base, g_wwtrap_len, PROT_READ|PROT_WRITE);
        u->uc_mcontext.gregs[16 /*REG_EFL*/] |= 0x100UL;   /* TF -> SIGTRAP re-protects */
        if (++g_wwtrap_hits > 4000) { g_wwtrap_base = 0; }  /* runaway guard: stop trapping */
        return;
    }
    /* objtrap: fault on the watched page -> unprotect, single-step (TF) past the write, re-protect in SIGTRAP */
    if (g_objtrap_page && si->si_addr >= g_objtrap_page &&
        (char*)si->si_addr < (char*)g_objtrap_page + sysconf(_SC_PAGESIZE)) {
        if ((uintptr_t)si->si_addr == g_objtrap_target) g_objtrap_pend_eip = eip;  /* write to c834+0 */
        else g_objtrap_pend_eip = 0;
        mprotect(g_objtrap_page, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE);
        u->uc_mcontext.gregs[16 /*REG_EFL*/] |= 0x100UL;   /* set TF -> SIGTRAP after the write */
        return;
    }
    /* fb-trap: if the fault is in our guarded page, log the writer EIP + stack, unprotect, retry */
    if (g_fbtrap_page && si->si_addr >= g_fbtrap_page &&
        (char*)si->si_addr < (char*)g_fbtrap_page + sysconf(_SC_PAGESIZE)) {
        unsigned long esp2 = (unsigned long)u->uc_mcontext.gregs[7];
        unsigned long *sp2 = (unsigned long *)esp2;
        fprintf(stderr, "[fbtrap] write to fb+0x%lx  EIP=0x%08lx  ret-cands:",
                (unsigned long)((char*)si->si_addr - (char*)(g_mem + 0xA0000)), eip);
        int sh=0; for (int i=0;i<64 && sh<8;i++){unsigned long v=sp2[i]; if(v>0x08048000&&v<0x08800000){fprintf(stderr," 0x%08lx",v);sh++;}}
        fprintf(stderr, "\n");
        mprotect(g_fbtrap_page, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE);
        g_fbtrap_page = 0;   /* one-shot per arm; timer re-arms */
        if (++g_fbtrap_hits > 200) g_fbtrap_req = 0;
        return;
    }
    unsigned long ebp = (unsigned long)u->uc_mcontext.gregs[6  /*REG_EBP*/];
    unsigned long esp = (unsigned long)u->uc_mcontext.gregs[7  /*REG_ESP*/];
    fprintf(stderr, "\n[segv] signal %d fault-addr %p EIP=0x%08lx EBP=0x%08lx ESP=0x%08lx\n",
            sig, si->si_addr, eip, ebp, esp);
    /* Mission reticle/vehicle state at the crash (DIAGNOSTIC): identifies which vehicle-type reticle
     * setter routed (the mga-blit/reticle debt) -- 2d32=type*2, 2d34=player vehicle, d550=reticle
     * sprite id, d548=reticle phase.  Reads only; fires within the SEGV handler so it captures the
     * exact crash-time state (the crash is in the first paint, before the in-mission tick pump). */
    { unsigned char *dg = g_mem + 0x1c000;
      unsigned short ty = *(unsigned short*)(g_mem+0x22d32);
      fprintf(stderr, "[segv] mission: 2d32(type*2)=0x%04x 2d34(veh)=0x%04x d550(reticle)=0x%04x d548=0x%02x builder[6d7c+ty]=0x%04x 3ae0(player)=0x%04x\n",
              ty, *(unsigned short*)(g_mem+0x22d34), *(unsigned short*)(g_mem+0x1d550),
              *(unsigned char*)(g_mem+0x1d548), *(unsigned short*)(dg+0x6d7c+(unsigned short)ty),
              *(unsigned short*)(g_mem+0x23ae0));
      { unsigned short pv=*(unsigned short*)(g_mem+0x23ae0);
        if(pv) fprintf(stderr,"[segv] player[0x16]=0x%04x (&8=%d take-cmd-gate) type=0x%04x\n",
                 *(unsigned short*)(dg+(unsigned short)(pv+0x16)), (*(unsigned short*)(dg+(unsigned short)(pv+0x16))>>3)&1,
                 *(unsigned short*)(dg+pv)); }
      fprintf(stderr, "[segv] roster 2d3c[0..15]:");
      for (int r=0;r<0x10;r++){ unsigned short slot=*(unsigned short*)(g_mem+0x22d3c+r*2);
        unsigned short typ = slot ? *(unsigned short*)(dg+slot) : 0xffff;
        fprintf(stderr," %04x(t=%04x)", slot, typ); }
      fprintf(stderr,"\n");
      /* Sprite DIRECTORY dump (2b1e crash root): 260c resolves recseg:recoff = far-ptr at
       * [DGROUP:0x4f0]<<4 + id.  A {0,0}/wild entry -> 2b1e blits garbage -> SEGV.  Dump the directory
       * segment word + the first 40 far-ptr records so a wild/zero entry (vs AZER1) is visible. */
      { unsigned short dseg = *(unsigned short*)(dg+0x4f0);
        /* dump the roster-slot-4/5 object bodies (oracle=valid vehicles; port=garbage type 0x0f00) to
         * distinguish under-registration (stale) vs build-desync -- compare vs scratch/oracle/azer3_spawn */
        for (int r=4;r<=5;r++){ unsigned short so=*(unsigned short*)(dg+0x6d3c+r*2);
          if(!so) continue; fprintf(stderr,"[segv] roster%d obj@0x%04x words +0..+1e:",r,so);
          for(int i=0;i<16;i++) fprintf(stderr," %d",*(short*)(dg+(unsigned short)(so+i*2))); fprintf(stderr,"\n"); }
        fprintf(stderr,"[segv] sprite-dir [0x4f0]=seg 0x%04x  records(off:seg):", dseg);
        unsigned base = (unsigned)dseg<<4;
        for (int i=0;i<40;i++){ unsigned short off=*(unsigned short*)(g_mem+base+i*4);
          unsigned short seg=*(unsigned short*)(g_mem+base+i*4+2);
          if (off||seg) fprintf(stderr," [%d]%04x:%04x", i, off, seg); }
        fprintf(stderr,"\n"); } }
    /* mga overlay seg capture (DIAGNOSTIC, FIST_SEGV_MGA): the m_mga_FUN_0000_2004 blitter faults on
     * `mov es,[0x1590]` (ES base-loss).  The scanline-seg WORD lives at the mga DRIVER's DS:0x1590 --
     * unknown statically (driver DS set at its own init, != fist_mga_base, != engine DGROUP).  Dump the
     * candidates + windows so the real allocated seg (0x3xxx/0x9xxx heap range) can be identified, plus
     * the compiled-C frame args (param_2/param_3 = span-out / src-sprite struct ptrs off ESP).  Reads only. */
    if (getenv("FIST_SEGV_MGA")) {
      fprintf(stderr, "[segv/mga] fist_mga_base=0x%08x  fault-addr=%p\n", fist_mga_base, si->si_addr);
      unsigned base = fist_mga_base;
      fprintf(stderr, "[segv/mga] cand engine DGROUP:0x1590 word=0x%04x  mga+0x1590 word=0x%04x\n",
              *(unsigned short*)(g_mem+0x1c000+0x1590), *(unsigned short*)(g_mem+base+0x1590));
      fprintf(stderr, "[segv/mga] window mga+0x1588..0x15a0:");
      for (unsigned o=0x1588;o<=0x15a0;o+=2) fprintf(stderr," %04x", *(unsigned short*)(g_mem+base+o));
      fprintf(stderr, "\n[segv/mga] window DG+0x1588..0x15a0:");
      for (unsigned o=0x1588;o<=0x15a0;o+=2) fprintf(stderr," %04x", *(unsigned short*)(g_mem+0x1c000+o));
      /* C-frame args: at gcc -O0, params are at [ebp+8],[ebp+12],[ebp+16]; deref each as a WORD ptr */
      unsigned long *fp=(unsigned long*)ebp;
      if (ebp>0x1000) {
        for (int a=2;a<=4;a++){ unsigned long p=fp[a];
          fprintf(stderr,"[segv/mga] arg%d=0x%08lx", a-1, p);
          if (p>0x08048000 && p<0x08800000) fprintf(stderr," -> *word=0x%04x", *(unsigned short*)p);
          /* p is a near-offset (raw low value); dump word[DG:p] and word[mga:p] = the struct's seg word */
          if (p < 0x10000) fprintf(stderr," | DG:%04lx word=0x%04x  mga:%04lx word=0x%04x",
              p, *(unsigned short*)(g_mem+0x1c000+p), p, *(unsigned short*)(g_mem+base+p));
          fprintf(stderr,"\n"); }
      }
    }
    /* EBP-chain walk: gcc -O0 gives every C fn a real frame, so the chain is exact (FIST_SEGV_EBP). */
    if (getenv("FIST_SEGV_EBP")) {
        unsigned long fp = ebp;
        fprintf(stderr, "[segv] EBP-chain return addrs:\n");
        for (int i = 0; i < 40 && fp > 0x1000 && fp < 0xffffffff; i++) {
            unsigned long *f = (unsigned long *)fp;
            unsigned long ret = f[1];
            if (ret > 0x08048000 && ret < 0x08800000) fprintf(stderr, "  #%d 0x%08lx\n", i, ret);
            unsigned long nf = f[0];
            if (nf <= fp) break;   /* frames grow downward toward higher addresses on unwind */
            fp = nf;
        }
    }
    /* Walk saved return addresses off the stack (frameless engine: scan a window) */
    unsigned long *sp = (unsigned long *)esp;
    fprintf(stderr, "[segv] stack return-addr candidates (map with nm /tmp/fist_native):\n");
    int shown = 0;
    for (int i = 0; i < 256 && shown < 20; i++) {
        unsigned long v = sp[i];
        if (v > 0x08048000 && v < 0x08800000) { fprintf(stderr, "  0x%08lx\n", v); shown++; }
    }
    if (getenv("FIST_DGDUMP")) {
        uint8_t *dg = g_mem + 0x1c000;
        int voff[] = {0x684,0x336,0x560,0x564,0x594,0x5fa,0x5fe,0x70a,0x782,0x724,0};
        for (int i=0; voff[i]; i++)
            fprintf(stderr, "[dg] 0x%03x = %04x:%04x\n", voff[i],
                    *(uint16_t*)(dg+voff[i]+2), *(uint16_t*)(dg+voff[i]));
        int doff[] = {0xf6c4,0xf6cc,0xf6ee,0xf790,0};
        for (int i=0; doff[i]; i++) {
            uint16_t *d = (uint16_t*)(dg+doff[i]);
            fprintf(stderr, "[dg] descr 0x%04x: w0=%04x w1=%04x w2=%04x w3=%04x w4=%04x\n",
                    doff[i], d[0],d[1],d[2],d[3],d[4]);
        }
    }
    if (getenv("FIST_DGDUMP")) {
        uint16_t pseg = *(uint16_t*)(g_mem+0x1c782);
        uint8_t *pb = g_mem + ((uint32_t)pseg<<4);
        fprintf(stderr,"[pal] 0x782 seg=%04x buf[0..17]:", pseg);
        for (int i=0;i<18;i++) fprintf(stderr," %02x", pb[i]);
        fprintf(stderr,"  0x786=%02x\n", g_mem[0x1c786]);
    }
    { const char *fb = getenv("FIST_FBDUMP"); if (fb) fist_dump_framebuffer(fb); }
    if (getenv("FIST_ROWDUMP")) {
        uint16_t *rt = (uint16_t*)(g_mem + 0x1c000 + 0x7c8);
        uint16_t c794 = *(uint16_t*)(g_mem + 0x1c000 + 0x794);
        fprintf(stderr, "[rowdump] c794=%04x rt[0]=%u [1]=%u [2]=%u [31]=%u [62]=%u [63]=%u [100]=%u\n",
                c794, rt[0], rt[1], rt[2], rt[31], rt[62], rt[63], rt[100]);
    }
    { const char *rw = getenv("FIST_FBRAW");
      if (rw) { FILE *f = fopen(rw, "wb"); if (f) { fwrite(g_mem + 0xA0000, 1, 64000, f); fclose(f); } } }
    _exit(139);
}
#else  /* __EMSCRIPTEN__: no mprotect/SIGSEGV diagnostics -- the fb-trap hook is a no-op */
volatile int g_fbtrap_req = 0;
void fbtrap_arm_hook(void) { }
#endif

uint8_t g_mem[FIST_MEM_SIZE];

#define IMAGE_PATH "re_out/fist_dat_image.bin"
#define IMAGE_SIZE 0x3390c   /* 211212 bytes, the extracted flat FIST.DAT image */

/* ---- INT-exit unwind target (set by fist_dos.c on INT 21h/4Ch or the FIST_MAXINTS cap) ---- */
jmp_buf g_fist_exit;
volatile int g_fist_exit_code;

/* ---- shim entry points ---- */
extern void fist_int_dispatch(void);        /* fist_dos.c -- target of the swi callotherfixup */
extern long fist_dump_framebuffer(const char *path);   /* fist_vga.c */
extern int  fist_vga_mode(void);

/* halt_baddata(): Ghidra "decode reached bad/undefined data" intrinsic. Honest stub: announce once. */
void halt_baddata(void) {
    static int warned;
    if (!warned) { warned = 1; fprintf(stderr, "[fist] STUB halt_baddata() reached (undefined-data decode)\n"); }
}
/* to_bcd(): x86 binary->packed-BCD (CMOS clock off the x87 stack). Faithful impl. */
int to_bcd(long long v) {
    int b = 0, shift = 0; long long u = v < 0 ? -v : v;
    while (u && shift < 32) { b |= (int)(u % 10) << shift; u /= 10; shift += 4; }
    return b;
}
/* swi(): the Ghidra INT intrinsic is now replaced at decompile time by the register-threaded
 * callotherfixup (-> fist_int_dispatch). This stub only exists so any INT the analyzer left
 * un-disassembled (data mis-read as code) still links; a live call announces itself. */
int swi(int intno) {
    fprintf(stderr, "[fist] swi(0x%02x) reached (un-threaded INT residue)\n", intno & 0xff);
    return 0;
}

/* engine entry: app_entry @ linear 0x4 (Ghidra 0000:0004). __allregs -> 6 GP-register params. */
extern void app_entry(undefined2, undefined2, undefined2, undefined2, undefined2, undefined2);

void fist_input_pump(void);              /* scripted deterministic input (defined below) */
extern int g_menu_ready;

/* ====================== THE TIME BASE ======================
 * The engine installs its OWN INT-8 (PIT) ISR (DOS set-vector 0x08 -> FUN_1000_30f8 @ linear 0x130f8),
 * calibrates the PIT to one vertical-retrace period (2fd3) and re-arms it at every retrace inside the
 * ISR: the INT-8 IS the vblank, [0x452] is the 60 Hz frame tick it derives, and everything time-driven
 * (the sim's per-tick step, the driver's DAC service, the sound sequencer) hangs off that interrupt.
 * The port keeps ONE clock for all of it -- a count of PIT clocks (1193182 Hz) in fist_vga.c
 * (board:0026): every port access and every cooperative pump is one count, the channel-0 counter and the
 * VGA retrace status derive from the count, and the interrupt fires when channel 0 wraps.  The host is
 * single-threaded and MUST NOT run engine C from a signal handler, so the interrupt is delivered from
 * the pump (fist_int8_fire), guarded against re-entry; a wrap that lands while the ISR runs is held and
 * delivered when it returns, as the PIC would.  No SIGALRM, no "one tick per pump": native and wasm step
 * the same clock the same way, so [0x452] and the whole sim evolve in lockstep on both. */
#define BIOS_TICK_LIN 0x46C
unsigned short g_fist_a18e_bx=0;
long g_min_a296 = 0x7fffffff;
long g_min_los = 0x7fffffffL;
unsigned short g_fist_op54_proj=0;
long g_op58_n=0, g_op58_oor=0, g_op58_occ=0, g_op58_vis=0;   /* board:0012: op-0x58 LOS call/return census */
int  g_a296_loaded = 0;           /* set once the mission is in-mission AND a296>0 (its roster spawned) */
/* The ENGINE's own end-of-mission state, latched the tick it appears.  byte[DGROUP:0xe814] is the flag
 * FUN_0000_459a's loop exits on and word[DGROUP:0x6da0] is the outcome code written just before it by
 * every one of the six writers, so this is the goal's "resolved victory/defeat state" read from the
 * engine rather than inferred from a roster count.  The flag lives for only a few ticks -- the mission
 * loop leaves on the next iteration -- so it has to be sampled every pump, not on a stride.  board:0017 */
int  g_mission_over = 0;
int  g_mission_outcome = -1;
long g_peak_a296 = 0;             /* the roster's high-water mark (reported only; "resolved" is g_mission_over) */
static void start_timer(void){
    fprintf(stderr, "[fist] time base: the PIT/VGA clock (one count per port access or pump; INT-8 on the channel-0 wrap)\n");
}
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
/* Browser harness: the engine runs BLOCKING in a Web Worker (no ASYNCIFY -- the browser rejects the
 * giant decompiled functions once asyncify inflates their local count).  Each ~frame the pump posts a
 * COPY of the 320x200x8bpp VGA framebuffer + the 256*3 6-bit palette to the main thread, which renders
 * it to a canvas (web/index.html).  Input is delivered via the shared input state the port already
 * emulates (INT 33h/INT 9), driven from the worker message handler between engine yields. */
EMSCRIPTEN_KEEPALIVE unsigned char *fist_web_fb(void){ return g_mem + 0xA0000; }
EMSCRIPTEN_KEEPALIVE unsigned char *fist_web_palette(void);   /* fist_vga.c */
EMSCRIPTEN_KEEPALIVE int fist_web_in_mission(void){ return g_mem[0x1c000 + 0x1549] == 0x1c; }
EM_JS(void, fist_web_post_frame_js, (unsigned char *fb, unsigned char *pal), {
  var f = HEAPU8.slice(fb, fb + 64000);
  var p = HEAPU8.slice(pal, pal + 768);
  postMessage({ t:'frame', fb:f.buffer, pal:p.buffer }, [f.buffer, p.buffer]);
});
void fist_web_post_frame(void){   /* posts unconditionally; the caller sets the ~60Hz cadence */
  extern void fist_web_force_palette(void);
  fist_web_force_palette();        /* in-mission the retrace-poll DAC upload may lag the render; force it */
  fist_web_post_frame_js(g_mem + 0xA0000, fist_web_palette());
}
/* Web audio: the engine runs blocking in the worker, so the worker PUSHES generated OPL PCM (mono s16)
 * to the main thread each pump (like the video frame post), which feeds a Web Audio AudioContext.
 * fist_web_audio_pull drains the OPL ring (fist_opl.c) since the last post.  board:0003 */
extern int fist_web_audio_pull(short *dst, int max);
extern int fist_web_audio_rate(void);
static short g_web_aud[8192];
EM_JS(void, fist_web_post_audio_js, (short *buf, int n, int rate), {
  var a = HEAP16.slice(buf >> 1, (buf >> 1) + n);   /* copy off the heap */
  postMessage({ t:'audio', pcm: a.buffer, rate: rate }, [a.buffer]);
});
void fist_web_post_audio(void){
  int n = fist_web_audio_pull(g_web_aud, 8192);
  if (n > 0) fist_web_post_audio_js(g_web_aud, n, fist_web_audio_rate());
}
/* One vblank of live web play (called from the INT-8, board:0026): hold the worker until the wall clock
 * has reached this much machine time (Atomics.wait on a private buffer -- the engine blocks the worker
 * anyway, and a sleep keeps the core free), then post the frame and the audio that this interrupt's
 * span produced.  A tab that fell far behind (hidden) resynchronises instead of racing to catch up. */
EM_JS(void, fist_web_sleep_ms_js, (double ms), {
  if (ms <= 0) return;
  if (self.__fsleep === undefined) {
    try { self.__fsleep = new Int32Array(new SharedArrayBuffer(4)); } catch (e) { self.__fsleep = null; }
  }
  if (self.__fsleep) { Atomics.wait(self.__fsleep, 0, 0, ms); return; }
  var end = performance.now() + ms; while (performance.now() < end) {}   /* no SAB (not cross-origin isolated): spin */
});
void fist_web_vblank(void){
  extern unsigned long long fist_clock_now(void);
  static double t0 = -1; static unsigned long long c0 = 0;
  double now = emscripten_get_now();
  unsigned long long c = fist_clock_now();
  if (t0 < 0) { t0 = now; c0 = c; }
  double target = t0 + (double)(c - c0) * (1000.0 / 1193182.0);
  if (target - now > 250.0) { t0 = now - (double)(c - c0) * (1000.0 / 1193182.0); target = now; }  /* resync */
  if (target > now) fist_web_sleep_ms_js(target - now);
  fist_web_post_frame();
  fist_web_post_audio();
}
/* Live mouse: mirror the FIST_MOUSE transition->flags delivery (movement 0x01; L press/rel 0x02/0x04;
 * R press/rel 0x08/0x10) so a browser mouse event drives the engine's INT-33h handler faithfully. */
static void deliver_mouse_event(unsigned flags, unsigned vx, unsigned vy, unsigned btn);
EMSCRIPTEN_KEEPALIVE void fist_web_mouse(int x, int y, int btn){   /* x in 0..639 (mode-13h virtual), y in 0..199 */
  static unsigned last_btn = 0; static int last_x = -1, last_y = -1;
  unsigned vx = (x < 0 ? 0 : x > 639 ? 639 : x), vy = (y < 0 ? 0 : y > 199 ? 199 : y);
  unsigned nb = (unsigned)btn & 3;
  if ((int)vx != last_x || (int)vy != last_y){ deliver_mouse_event(0x01, vx, vy, nb); last_x = vx; last_y = vy; }
  unsigned pressed = nb & ~last_btn, released = last_btn & ~nb;
  if (pressed & 1) deliver_mouse_event(0x02, vx, vy, nb);
  if (released & 1) deliver_mouse_event(0x04, vx, vy, nb);
  if (pressed & 2) deliver_mouse_event(0x08, vx, vy, nb);
  if (released & 2) deliver_mouse_event(0x10, vx, vy, nb);
  last_btn = nb;
}
/* Live keyboard: push a BIOS key (AX = scancode<<8 | ascii) into the INT-16h buffer (fist_dos.c).
 * Weakly referenced so a build without the fist_dos.c key buffer still links (mouse-only). */
extern void fist_dos_push_key(int ax) __attribute__((weak));
EMSCRIPTEN_KEEPALIVE void fist_web_key(int ax, int down){ if (down && fist_dos_push_key) fist_dos_push_key(ax); }
/* Pump the input SharedArrayBuffer (written by the main thread) each frame; the worker set the SAB
 * view on self.__fin before starting the engine.  Layout (Int32): [0]=x [1]=y [2]=buttons [3]=mouseSeq
 * [4]=keyHead [5]=keyTail [6..]=key ring {scancode<<1 | down}. */
EM_JS(void, fist_web_pump_input_js, (void), {
  var F = self.__fin; if (!F) return;
  /* mouse: deliver ONE queued event per poll so a press and its release land on DIFFERENT engine
   * frames (like the FIST_MOUSE script spaces its steps) -- else a fast down+up registers no click. */
  if (F[1] !== F[0]){ var i = (F[1] % 120) * 3; _fist_web_mouse(F[8+i], F[8+i+1], F[8+i+2]); F[1] = (F[1]+1)|0; }
  /* keyboard: drain the key ring (BIOS keys are edge events). */
  while (F[5] !== F[4]){ var k = F[400 + (F[5] % 32)]; F[5] = (F[5]+1)|0; _fist_web_key(k >> 1, k & 1); }
});
void fist_web_pump_input(void){ fist_web_pump_input_js(); }
static volatile int g_web_mode = 0;
/* Browser start: set web-mode (enables the per-pump frame post) then run the engine main loop
 * (non-returning; blocks the worker, which is fine -- the UI thread stays live). */
EMSCRIPTEN_KEEPALIVE void fist_web_start(void){ g_web_mode = 1; extern int main(int,char**);
    static char a0[]="fist"; char *av[1]={a0}; main(1,av); }
#endif

/* ---- the engine's installed INT-8 (PIT) ISR entry, captured at DOS set-vector 0x08 ---- */
static uint32_t g_int8_lin;          /* linear (SEG<<4)+OFF of the installed handler */
static int      g_int8_set;          /* becomes 1 once the engine installs its ISR */
static int      g_in_isr;            /* re-entry guard: never run the ISR from inside itself */
static long     g_isr_runs;          /* diagnostic count */

void fist_set_int8_handler(uint32_t linear){
    if (linear == FIST_INTVEC_LIN + 8) {    /* the engine hands back the AH=35 vector it saved: BIOS again */
        g_int8_set = 0;
        fprintf(stderr, "[fist] INT-8 (PIT) vector restored to the BIOS handler\n");
        return;
    }
    g_int8_lin = linear; g_int8_set = 1;
    fprintf(stderr, "[fist] INT-8 (PIT) ISR installed @ linear 0x%05x -> cooperative pump armed\n", linear);
}

/* Run the engine's installed INT-8 ISR for each pending host tick (capped), guarded against re-entry.
 * The ISR (FUN_1000_30f8) is an __allregs C function reached through the indirect-call dispatcher; it
 * saves/restores its own state and its tick bookkeeping does not depend on incoming register operands,
 * so we invoke it with zeroed register args. It runs on the current (main) stack -- NOT from a signal. */
/* Dump the framebuffer (+ optional dev diagnostics) and exit -- shared by the FIST_DUMPTICK engine-
 * frame-timer trigger and the FIST_RUNMS wall-clock watchdog.  Runs in main context (safe for the
 * PPM writer).  The [0x452] value is logged so a capture's tick-phase is reproducible. */
static void fist_dump_and_exit(const char *why){
    fprintf(stderr, "[fist] %s: dumping frame + exiting (video-mode=0x%02x, [0x452]=%u)\n",
            why, fist_vga_mode(), *(uint16_t*)(g_mem+0x1c452));
    { extern long g_min_a296, g_peak_a296; extern int g_a296_loaded, g_mission_over, g_mission_outcome;
      /* 0x1a6ae, 0x1a6bb and 0x1a60b write these three; 3/4/5 come from the other flag-writers
       * (0x6115, the 0x15e4c abort handler, the 0x15e8c timeline task). */
      static const char *const oc[] = { "VICTORY (opposing side destroyed)", "DEFEAT (own side lost)",
                                        "TIME EXPIRED", "outcome 3", "ABORTED", "outcome 5" };
      const char *ocs = (g_mission_outcome >= 0 && g_mission_outcome < 6) ? oc[g_mission_outcome] : "outcome ?";
      /* board:0017 -- report the words the verdict gate at 1a678 actually branches on (own/other alive
       * byte[0x6d38]/[0x6d39], objectives left/peak word[0x978e]/[0x9790]) and the player's registry
       * slot (word[0xdfbc], index 0): a run that left the sim with NO verdict and player=0000 is sitting
       * at the engine's own PL: vehicle-selector prompt after the player's vehicle was destroyed -- the
       * second mission-end path, which tools/selfplay.sh must not file under TIMEOUT. */
      { unsigned char *dg = g_mem + 0x1c000; unsigned pobj = *(uint16_t*)(dg + 0x6d34);
      fprintf(stderr, "[outcome] a294=%d a296=%d  loaded=%d min_a296=%ld peak_a296=%ld  over=%d code=%d  alive=%u/%u obj=%u/%u verdict=%u/%u player=%04x(type=%04x dead=%d)  %s\n",
              *(uint16_t*)(g_mem+0x1c000+0xe294), *(uint16_t*)(g_mem+0x1c000+0xe296),
              g_a296_loaded, g_min_a296, g_peak_a296, g_mission_over, g_mission_outcome,
              dg[0x6d38], dg[0x6d39], *(uint16_t*)(dg+0x978e), *(uint16_t*)(dg+0x9790),
              *(uint16_t*)(dg+0x6da0), *(uint16_t*)(dg+0x6da2), pobj,
              pobj ? *(uint16_t*)(dg+pobj) : 0, pobj ? ((dg[(uint16_t)(pobj+0x19)] & 4) ? 1 : 0) : -1,
              g_mission_over        ? ocs :
              g_a296_loaded         ? "(mission loaded, not resolved)" : "(mission never loaded a roster)"); }
      extern long g_min_los; fprintf(stderr,"[range] min cross-unit |dx|+|dy| after first kills = %ld (0x40000=%d threshold)\n",g_min_los,0x40000);
      extern long g_op58_n,g_op58_oor,g_op58_occ,g_op58_vis; fprintf(stderr,"[op58] LOS calls=%ld  out-of-range=%ld  occluded=%ld  VISIBLE=%ld\n",g_op58_n,g_op58_oor,g_op58_occ,g_op58_vis);
 }
            { extern void fist_sb_flush(void); fist_sb_flush(); }   /* finalize any SB PCM/WAV capture */
            { extern void fist_opl_flush(void); fist_opl_flush(); } /* finalize any OPL FM PCM/WAV capture */
            { extern void fist_snd_diag(void); fist_snd_diag(); }   /* sequencer-fed diagnostic */
            /* board:0001 cause-3: dump g_mem so native vs wasm can be diffed to locate where the sound
               sequencer state first diverges (both targets internally deterministic; the divergence is
               native-vs-wasm systematic).  FIST_MEMDUMP=<path>. */
            { const char *md = getenv("FIST_MEMDUMP");
              if (md) { FILE *f = fopen(md, "wb"); if (f) { fwrite(g_mem, 1, FIST_MEM_SIZE, f); fclose(f); } } }
            const char *fb = getenv("FIST_FBDUMP");
            if (fb) fist_dump_framebuffer(fb);
            { const char *rw = getenv("FIST_FBRAW");
              if (rw) { FILE *f = fopen(rw, "wb"); if (f) { fwrite(g_mem + 0xA0000, 1, 64000, f); fclose(f); } } }
            if (getenv("FIST_ROWDUMP")) {
                uint16_t *rt = (uint16_t*)(g_mem + 0x1c000 + 0x7c8);
                uint16_t c794 = *(uint16_t*)(g_mem + 0x1c000 + 0x794);
                fprintf(stderr, "[rowdump] c794=%04x rt[0]=%u [1]=%u [2]=%u [31]=%u [62]=%u [63]=%u [100]=%u\n",
                        c794, rt[0], rt[1], rt[2], rt[31], rt[62], rt[63], rt[100]);
            }
            if (getenv("FIST_CURIDX")) {
                uint8_t *fbp = g_mem + 0xA0000;
                fprintf(stderr, "[curidx] fb(0,0)=%u (2,2)=%u (5,5)=%u (7,0)=%u  cursor state:"
                        " d5da(w)=%u d5dc(h)=%u d5e0(spr)=%04x d5e2(mask)=%04x c716(seg)=%04x\n",
                        fbp[0], fbp[2*320+2], fbp[5*320+5], fbp[7*320],
                        *(uint16_t*)(g_mem+0x1c000+0x5da), *(uint16_t*)(g_mem+0x1c000+0x5dc),
                        *(uint16_t*)(g_mem+0x1c000+0x5e0), *(uint16_t*)(g_mem+0x1c000+0x5e2),
                        *(uint16_t*)(g_mem+0x1c000+0x716));
                { uint16_t c716=*(uint16_t*)(g_mem+0x1c000+0x716); uint16_t spr=*(uint16_t*)(g_mem+0x1c000+0x5c4);
                  uint8_t *sp=g_mem+((uint32_t)c716<<4)+spr;
                  fprintf(stderr,"[curidx2] d5c2(colormask)=%02x d5c4(spr)=%04x sprite bytes:",g_mem[0x1c000+0x5c2],spr);
                  for(int i=0;i<12;i++)fprintf(stderr," %02x",sp[i]); fprintf(stderr,"\n"); }
            }
            if (getenv("FIST_MEMPROBE")) {
                #define GW(o) (*(uint16_t*)(g_mem+0x1c000+(o)))
                fprintf(stderr, "[memprobe] sig[0]=%04x  freehead[0x16e2]=%04x freecnt[0x16dc]=%u"
                        "  pool16d4[+a]=%04x[+c]=%04x[+10]=%04x[+18]=%04x"
                        "  pool16f6[+a]=%04x[+c]=%04x[+10]=%04x[+18]=%04x"
                        "  pool1718[+a]=%04x[+c]=%04x[+10]=%04x[+18]=%04x\n",
                        GW(0), GW(0x16e2), GW(0x16dc),
                        GW(0x16d4+0xa),GW(0x16d4+0xc),GW(0x16d4+0x10),GW(0x16d4+0x18),
                        GW(0x16f6+0xa),GW(0x16f6+0xc),GW(0x16f6+0x10),GW(0x16f6+0x18),
                        GW(0x1718+0xa),GW(0x1718+0xc),GW(0x1718+0x10),GW(0x1718+0x18));
                fprintf(stderr, "[memprobe] svc[0x0a]=%04x:%04x svc[0x0e]=%04x:%04x "
                        "dev[0x4fc]=%02x  0xd4=%04x:%04x 0xf4=%04x:%04x  near[0x3e8]=%04x[0x306]=%04x\n",
                        GW(0x0a),GW(0x0c),GW(0x0e),GW(0x10),
                        g_mem[0x1c000+0x4fc], GW(0xd4),GW(0xd6),GW(0xf4),GW(0xf6),
                        GW(0x3e8),GW(0x306));
                #undef GW
            }
            if (getenv("FIST_DESCRDUMP")) {
                uint16_t w0 = *(uint16_t*)(g_mem + 0x1c000 + 0x9f1c);
                fprintf(stderr, "[descrdump] DGROUP:0x9f1c word0(.MS3 seg)=%04x word[0x9f1e]=%04x\n",
                        w0, *(uint16_t*)(g_mem + 0x1c000 + 0x9f1e));
                if (w0) { uint8_t *p = g_mem + ((uint32_t)w0<<4);
                    fprintf(stderr, "[descrdump] .MS3 seg bytes: %02x %02x %02x %02x %02x %02x %02x %02x  '%.8s'\n",
                            p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],(char*)p); }
            }
            if (getenv("FIST_INDBG")) {
                #define GW(o) (*(uint16_t*)(g_mem+0x10000+(o)))
                #define GB(o) (*(uint8_t*)(g_mem+0x10000+(o)))
                fprintf(stderr,"[indbg] c738word=0x%04x c739=0x%02x | d5d9(2f03gate)=0x%02x d5d6=0x%02x d5b6=0x%02x dd9c=0x%02x d5d7=0x%02x\n",
                    GW(0xc738), GB(0xc739), GB(0xd5d9), GB(0xd5d6), GB(0xd5b6), GB(0xdd9c), GB(0xd5d7));
                fprintf(stderr,"[indbg] dd9e(y)=%u dda0(x)=%u dd9c(b)=%u | d5ca(cy)=%u d5cc(cx)=%u c2a8=%u c738=%d(0x%02x) | isr_runs=%ld d5b8=%u d5ba=%u d5b6=%u | d5ac=%08x c2b0=%08x d5b0=%08x\n",
                    GW(0xdd9e),GW(0xdda0),GW(0xdd9c), GW(0xd5ca),GW(0xd5cc),GB(0xc2a8),(signed char)GB(0xc738),GB(0xc738),
                    g_isr_runs, GW(0xd5b8),GW(0xd5ba),GW(0xd5b6),
                    *(uint32_t*)(g_mem+0x1d5ac),*(uint32_t*)(g_mem+0x1c2b0),*(uint32_t*)(g_mem+0x1d5b0));
            }
            if (getenv("FIST_MKRDBG")) {
                uint8_t *D = g_mem + 0x1c000;
                fprintf(stderr,"[mkrdbg] c61a=%08x c61e=%08x c622=%08x  2dd0(count)=%u 2db6(sel)=%u [0x452]=%u\n",
                    *(uint32_t*)(D+0x61a), *(uint32_t*)(D+0x61e), *(uint32_t*)(D+0x622),
                    *(uint16_t*)(D+0x6dd0), *(uint16_t*)(D+0x6db6), *(uint16_t*)(D+0x452));
                uint16_t rec=0x6dda;
                for(int i=0;i<4;i++){
                    fprintf(stderr,"[mkrdbg] mission %d rect(rec+0x19..0x1f): %u %u %u %u  st(rec+0x3b)=%u\n",
                        i, *(uint16_t*)(D+(uint16_t)(rec+0x19)),*(uint16_t*)(D+(uint16_t)(rec+0x1b)),
                        *(uint16_t*)(D+(uint16_t)(rec+0x1d)),*(uint16_t*)(D+(uint16_t)(rec+0x1f)),
                        D[(uint16_t)(rec+0x3b)]);
                    rec+=0xfd;
                }
            }
            if (getenv("FIST_PALDUMP")) {
                uint16_t pseg = *(uint16_t*)(g_mem+0x1c782);
                uint8_t *pb = g_mem + ((uint32_t)pseg<<4);
                fprintf(stderr,"[paldump] 0x782 seg=%04x buf[0..23]:", pseg);
                for (int i=0;i<24;i++) fprintf(stderr," %02x", pb[i]);
                fprintf(stderr,"\n[paldump] buf[36..47](idx12-15):");
                for (int i=36;i<48;i++) fprintf(stderr," %02x", pb[i]);
                fprintf(stderr,"  fade786=%02x\n", g_mem[0x1c786]);
            }
            if (getenv("FIST_GEOMDUMP")) {
                /* Deterministic (tick-anchored via FIST_DUMPTICK) probe of the mission viewport geometry:
                 * word[DGROUP:0x7ac0] = the 0x1c-byte SRC descriptor (60d9->mga 22bf SOURCE), word[0x7aa4]
                 * = the DEST viewport rect.  Nonzero here => the viewport pipeline is live. */
                uint8_t *D = g_mem + 0x1c000;
                fprintf(stderr,"[geomdump] [0x452]=%u  src 0x7ac0(0x1c bytes):",
                        *(uint16_t*)(D+0x452));
                for (int i=0;i<0x1c;i+=2) fprintf(stderr," %04x", *(uint16_t*)(D+0x7ac0+i));
                fprintf(stderr,"\n[geomdump] dst 0x7aa4(0x1c bytes):");
                for (int i=0;i<0x1c;i+=2) fprintf(stderr," %04x", *(uint16_t*)(D+0x7aa4+i));
                fprintf(stderr,"\n[geomdump] ptrs d552(0x1552)=%04x d556a(0x156a)=%04x  d548(0x1548 flags)=%02x\n",
                        *(uint16_t*)(D+0x1552), *(uint16_t*)(D+0x156a), D[0x1548]);
            }
            _exit(0);
}

/* board:0012 -- MISSION-TIME window for the cooperative tick.  The pre-cockpit LOAD phase is held (the
 * engine must reach the spawn frame with [0x452] still at the load value, native and wasm alike), but once
 * the cockpit view has been entered ONCE the PIT must keep running for the rest of the mission: the engine
 * switches d549 between the mission viewports (0x1c cockpit, 0x1e/0x20/0x22 the map/external/kill views)
 * and the old `d549==0x1c` gate froze [0x452] the moment the view changed -> the frame loop spun forever
 * in the per-frame AI (a930->a19a->0927) with time stopped.  Sticky: latch on the first cockpit frame. */
/* The cooperative pump: one PIT count of machine time (board:0026).  Every port access and every engine
 * spin-wait calls it, so it must cost next to nothing; the clock delivers the INT-8 at each channel-0
 * wrap (fist_int8_fire), and everything that only needs looking at once per interrupt -- the scripted
 * input, the dump/watchdog checks, the diagnostics -- runs from there (fist_pump_slow). */
void fist_timer_pump(void){
    extern void fist_clock_advance(unsigned); extern int g_int8_force;
    g_int8_force = 1;      /* an explicit pump (a spin-wait) delivers its interrupt even under the frame-schedule replay */
    fist_clock_advance(1);
    g_int8_force = 0;
}

/* Once per INT-8 (from fist_int8_fire, after the ISR) and once per BIOS tick before the engine has its
 * vector: the per-interrupt bookkeeping and diagnostics.  [0x452] only moves in the ISR, so a per-tick
 * check sees every value it takes; the mission-outcome latch samples the verdict word, which lives for
 * many ticks. */
static void fist_pump_slow(void){
    /* board:0012 -- CROSS-TARGET self-play parity trace (FIST_SIMTRACE2=N, default OFF).  Unlike the
     * native-only SIMTRACE block below (inside the #else), this runs on BOTH native and wasm so an
     * in-mission run's tick-by-tick object-registry fingerprint is diffable native<->wasm for ANY mission
     * (FIST_FSG_BATTLE selects it).  Reads g_mem only; behaviour-neutral. */
    { static long strace=-2, hbk=-1; static int plive=-1,pa=-1,pb=-1,pg=-1;
      if (strace==-2){ const char*e=getenv("FIST_SIMTRACE2"); strace=e?atol(e):-1; }
      /* board:0012 cross-target DGROUP hash (FIST_SIMHASH=1): one line per engine tick with a 32-block
       * FNV-1a fingerprint of the object/AI region, so the FIRST diverging tick AND the block that
       * carries it can be found by diffing the native and wasm logs.  Read-only, env-gated. */
      /* board:0012 one-shot raw dump of the object region at a chosen tick (FIST_SIMDUMP=<tick>,
       * FIST_SIMDUMP_OUT=<path>) so native and wasm can be diffed byte-for-byte.  Read-only, env-gated. */
      { static int sd=-2; static int done=0;
        if (sd==-2){ const char*e=getenv("FIST_SIMDUMP"); sd = e?atoi(e):-1; }
        if (sd>=0 && !done && g_mem[0x1c000+0x1549]==0x1c &&
            *(unsigned short*)(g_mem+0x1c452) == (unsigned short)sd) {
          const char *o = getenv("FIST_SIMDUMP_OUT"); FILE *f = fopen(o?o:"/tmp/fist_simdump.bin","wb");
          if (f){ fwrite(g_mem+0x1c000,1,0xf000,f); fclose(f); }
          done = 1; fprintf(stderr,"[simdump] wrote DGROUP 0x0000..0xefff at t=%d\n", sd); } }
      { static int sh=-1; static unsigned pt=0xffffffffu;
        if (sh<0) sh = getenv("FIST_SIMHASH") ? 1 : 0;
        if (sh && g_mem[0x1c000+0x1549]==0x1c) {
          unsigned t2 = *(unsigned short*)(g_mem+0x1c452);
          if (t2 != pt) { pt = t2;
            fprintf(stderr,"[simhash] t=%u", t2);
            /* blocks over DGROUP 0x9000..0xefff: the object rosters, display table and AI scratch.  The
             * LOW DGROUP is deliberately excluded -- it holds the far-vector table the shim installs
             * with real host addresses, which differ between the targets by construction. */
            for (int b=0;b<48;b++){ unsigned h=2166136261u; const unsigned char *p=g_mem+0x1c000+0x9000+b*0x200;
              for (int q=0;q<0x200;q++){ h^=p[q]; h*=16777619u; }
              fprintf(stderr," %08x",h); }
            /* board:0012 cont.65f: a 49th column over an EXPLICIT set of low-DGROUP RENDER words.  The
             * bulk of the low DGROUP stays excluded (it holds the shim's far-vector table with real host
             * addresses, which differ by construction), but the sim region is now bit-identical on the
             * missions that used to diverge, and everything still differing lives down here -- the MGA
             * sprite clip words 260c computes (0x1586..0x158f, patch 114) and the display-list pointers
             * around them.  Enumerating them keeps the fingerprint free of host addresses while letting
             * the same first-differing-tick walk continue into the render side. */
            { static const unsigned short rw[] = {
                /* ONLY the MGA sprite CLIP block 260c computes (patch 114).  The other low-DGROUP words
                 * the cont.65f dump flagged -- 0x03e2, 0x0686, 0x16b0, 0x2663, 0x2672, 0x3ae2 -- are
                 * FAR-VECTOR slots or their high halves (native shows 0x081f/0x080a there, the top halves
                 * of real host addresses, against 0 on wasm), so they differ by construction and must
                 * stay out of any fingerprint. */
                0x1586,0x1587,0x1588,0x1589,0x158a,0x158b,0x158c,0x158d,0x158e,0x158f };
              unsigned h=2166136261u;
              for (unsigned q=0;q<sizeof(rw)/sizeof(rw[0]);q++){ h^=g_mem[0x1c000+rw[q]]; h*=16777619u; }
              fprintf(stderr," R%08x",h); }
            fprintf(stderr,"\n"); } } }
      if (strace>0 && g_mem[0x1c000+0x1549]==0x1c) {
        unsigned char *dg=g_mem+0x1c000; unsigned t=*(unsigned short*)(dg+0x452);
        unsigned short *fbc=(unsigned short*)(dg+0xdfbc);
        int live=0,goals=0; for(int i=0;i<0xb6;i++){ unsigned short s=fbc[i*2]; if(!s)continue; live++;
          if(dg[(unsigned short)(s+0x17)]&0x08) goals++; }
        int a=*(unsigned short*)(dg+0xe294), b=*(unsigned short*)(dg+0xe296);
        long bucket=t/strace; unsigned short ps=fbc[0]; long px=0,py=0; int pctl=0;
        if(ps){ px=*(int*)(dg+(unsigned short)(ps+4)); py=*(int*)(dg+(unsigned short)(ps+8)); pctl=dg[(unsigned short)(ps+0x17)]; }
        if (live!=plive||a!=pa||b!=pb||goals!=pg||bucket!=hbk){
          fprintf(stderr,"[simtrace2] t=%u live=%d goals=%d a294=%d a296=%d player{slot=%04x X=%ld Y=%ld f17=%02x}%s",
            t,live,goals,a,b,ps,px,py,pctl,(live!=plive||a!=pa||b!=pb||goals!=pg)?"  <<CHANGE":"");
          /* board:0012 per-TYPE census of the display table: which spawn class differs native<->wasm. */
          { static int types=-1; if (types<0) types = getenv("FIST_SIMTYPES") ? 1 : 0;
            if (types) { int h[64]; for(int q=0;q<64;q++) h[q]=0;
              for (int i2=0;i2<0xb6;i2++){ unsigned short s2=fbc[i2*2]; if(!s2) continue;
                unsigned short ty=*(unsigned short*)(dg+s2); h[ty<64?ty:63]++; }
              fprintf(stderr,"  types=");
              for(int q=0;q<64;q++) if(h[q]) fprintf(stderr,"%02x:%d,",q,h[q]); } }
          fprintf(stderr,"\n");
          plive=live;pa=a;pb=b;pg=goals;hbk=bucket;
        }
      }
    }
#ifdef __EMSCRIPTEN__
    /* LIVE web play: the wall-clock pacing, the frame post and the audio post ride on the INT-8 (one per
     * vblank, fist_int8_fire -> fist_web_vblank); this slow path runs once per interrupt too, so feeding
     * ONE queued browser input event here lands a press and its release on different engine frames.
     * board:0001/0003 */
    if (g_web_mode) { void fist_web_pump_input(void); fist_web_pump_input(); }
#else
    { int in_mission = (g_mem[0x1c000 + 0x1549] == 0x1c);
      /* Cheap (NO I/O -> non-perturbing) mission-outcome tracker: any fprintf in the hot pump changes the
       * pump/tick ratio and breaks the timing-sensitive menu/mission-load, so record a296 silently and
       * report once at exit (fist_dump_and_exit).  board:0012 */
      /* The roster size is PER MISSION -- AZER1 fields ~16 vehicles, TRAIN1 fields 3 -- so the old
       * `a296 >= 15` gate silently reported every small-roster mission as "never loaded", and would
       * have reported a RESOLVED one that way too.  Gate on being in-mission with a live roster and
       * keep the high-water mark instead.  board:0017 */
      { int b = *(uint16_t *)(g_mem + 0x1c000 + 0xe296);
        if (in_mission && b > 0) g_a296_loaded = 1;
        if (b > g_peak_a296) g_peak_a296 = b;
        if (g_a296_loaded && b < g_min_a296) g_min_a296 = b; }
      { extern int g_mission_over, g_mission_outcome; unsigned char *dg = g_mem + 0x1c000;
        /* Latch on the VERDICT, not on the mission-over flag: a5dc writes 0x6da0 and starts the countdown
         * word 0x6da2 (0x3c / 5 / 2, from 0xffff) in the same instant, and the countdown stays visible for
         * 2..60 supervisor rounds of 8 ticks each -- whereas 0xe814 is set by the LAST decrement and
         * consumed by the mission loop's very next `testb 0xe814` (45c8), which leaves the mission and
         * flips 0x1549 before any pump can observe the flag with in_mission still true.  That is how a
         * TIME EXPIRED on AZER1 went unlatched and the run sat at the post-mission menu.  Outcomes 3/4/5
         * (6115, 15e46, 15e86) write 0x6da0 and 0xe814 without a countdown, so the flag stays as the
         * second trigger.  board:0017 */
        /* NOT gated on in_mission: byte[0x1549] cycles 00/1c/20/22 within a tick (it is a phase byte, not
         * a mission-state flag), so a pump that lands on any phase but 0x1c missed the verdict -- AZER1's
         * TIME EXPIRED went unlatched that way while AZER2/3/5/6 latched by phase luck.  The roster
         * high-water (g_a296_loaded) says a mission has run; 0x6da2 is 0xffff from 47b2 at mission entry
         * until the verdict, so nothing stale can latch. */
        if (g_a296_loaded && !g_mission_over && (*(uint16_t *)(dg + 0x6da2) != 0xffff || dg[0xe814] != 0)) {
          g_mission_over = 1; g_mission_outcome = *(uint16_t *)(dg + 0x6da0);
          /* FIST_STOP_ON_OUTCOME=1: stop the moment the engine resolves the mission.  This is the
           * self-play sweep's terminating condition -- a mission runs for as long as it takes rather
           * than to a tick cap, and cannot be cut off before it resolves.  board:0017 */
          { static int soo = -1; if (soo < 0) soo = getenv("FIST_STOP_ON_OUTCOME") ? 1 : 0;
            if (soo) fist_dump_and_exit("mission resolved"); } } }
      /* FIST_WATCHBYTE=<dgroup-off>[,<off>...] (diagnostic, default OFF): poll up to 8 DGROUP bytes every
       * pump and print each CHANGE with the tick and mission clock.  Needs no arming and no page
       * protection -- FIST_OBJTRAP (mprotect at the op-0x2c gate) never fires in the self-play flow, which
       * silently produced "no writer" for bytes written every tick.  board:0017 */
      { static int nwb = -1; static unsigned woff[8]; static int wprev[8];
        if (nwb < 0) { nwb = 0; const char *e = getenv("FIST_WATCHBYTE");
          while (e && *e && nwb < 8) { woff[nwb] = (unsigned)strtoul(e, (char **)&e, 0); wprev[nwb] = -1; nwb++;
            while (*e == ',' || *e == ' ') e++; } }
        if (nwb > 0) { unsigned char *dg = g_mem + 0x1c000;   /* ungated: vectors are watched from boot */
          for (int i = 0; i < nwb; i++) { int v = dg[woff[i] & 0xffff];
            if (v != wprev[i]) { fprintf(stderr, "[watchbyte] dg:%04x %02x -> %02x  t=%u clock=%02x:%02x:%02x\n",
                woff[i] & 0xffff, wprev[i] & 0xff, v, *(uint16_t *)(dg + 0x452), dg[0x6da6], dg[0x6da7], dg[0x6da8]);
              wprev[i] = v; } } } }
      /* DIAGNOSTIC (FIST_FIXFACTION): test the aa08 side-filter hypothesis -- force byte[obj+0x16] bit3
       * = the unit's SIDE (byte[type-0x19ec]&1), so [0x16]&8 cleanly separates factions.  If units then
       * engage the OTHER side (combat -> deaths, a296 drops), the faction bit was a real blocker.  Few
       * writes/tick -> non-perturbing.  NOT a shipped patch -- an experiment. board:0012 */
      if (in_mission && getenv("FIST_FIXFACTION")) {
        unsigned char *dg = g_mem + 0x1c000; unsigned short *fbc = (unsigned short*)(dg+0xdfbc);
        for (int i=0;i<0xb6;i++){ unsigned short s=fbc[i*2]; if(!s) continue;
          unsigned short t=*(unsigned short*)(dg+s);
          unsigned char side=dg[(unsigned short)(t-0x19ec)]&1;
          unsigned char *f16=&dg[(unsigned short)(s+0x16)];
          *f16 = (unsigned char)((*f16 & ~8) | (side<<3)); }
      }
      /* DIAGNOSTIC (FIST_DUMP_END): trace the mission-over supervisor (1000:a5dc) guard by guard.
       * The flag byte[DGROUP:0xe814] is what FUN_0000_459a exits on, word[0x6da0] is the outcome code
       * and word[0x6da2] the teardown countdown; 0x6da6:0x6da7:0x6da8 is the mission clock and
       * 0x6d38/0x6d3a the own-side live and peak counts.  Env-gated, read-only, printed every
       * FIST_DUMP_END pumps.  board:0017 */
      { static long endn = -1, endt = 0;
        if (endn < 0) { const char *e = getenv("FIST_DUMP_END"); endn = e ? atol(e) : 0; }
        if (endn > 0 && in_mission && (endt++ % endn) == 0) {
          unsigned char *dg = g_mem + 0x1c000;
          fprintf(stderr,
            "[end] t=%ld out=%04x tmr=%04x | 6da6=%02x 6da7=%02x 6da8=%02x | 6dab=%02x 6d3a=%02x 6d38=%02x 6db0=%02x"
            " | live=%u peak=%u prev=%u | e814=%02x a296=%u\n",
            endt-1,
            *(unsigned short*)(dg+0x6da0), *(unsigned short*)(dg+0x6da2),
            dg[0x6da6], dg[0x6da7], dg[0x6da8],
            dg[0x6dab], dg[0x6d3a], dg[0x6d38], dg[0x6db0],
            *(unsigned short*)(dg+0x978e), *(unsigned short*)(dg+0x9790), *(unsigned short*)(dg+0x9792),
            dg[0xe814], *(unsigned short*)(dg+0xe296));
        } }
      /* DIAGNOSTIC (FIST_DUMP_REG): one-shot dump of the 0x9fbc object registry once in-mission, to
       * resolve the tree object model for the plant-tree editor harness (tree type discriminator +
       * body/coord layout). Reads only; env-gated; no effect on any flow. */
      { static int dumped_reg = 0;
        if (in_mission && !dumped_reg && getenv("FIST_DUMP_REG")
            && *(unsigned short*)(g_mem+0x1c452) >= (unsigned)(getenv("FIST_DUMP_REG_T")?atoi(getenv("FIST_DUMP_REG_T")):0)) {
          dumped_reg = 1;
          unsigned char *dg = g_mem + 0x1c000;
          fprintf(stderr, "[dumpreg] in-mission; DAT_2000_530a(tree count)=%u\n",
                  *(unsigned short*)(g_mem+0x2530a));
          unsigned short *fbc = (unsigned short*)(dg + 0xdfbc);   /* DAT_2000_9fbc {slot,val} x0xb6 */
          if(getenv("FIST_VTDUMP")){ for(int ty=0;ty<0x30;ty++){ unsigned short m=*(unsigned short*)(dg+(unsigned short)(ty*2-0x1bac)); if(m) fprintf(stderr,"[vt] type %02x -> upd %04x%s\n",ty,m,(m==0xbab4)?"  BAB4-DESPAWN":(m==0xb51f)?"  B51F-WEAPON":""); } }
          for (int i=0;i<0xb6;i++){
            unsigned short s=fbc[i*2], v=fbc[i*2+1];
            if(!s) continue;
            unsigned short t=*(unsigned short*)(dg+s);
            unsigned char side=dg[(unsigned short)(t-0x19ec)]&1;   /* b21d side flag is indexed by TYPE */
            unsigned short moff=*(unsigned short*)(dg+(unsigned short)(t*2-0x1bac));  /* c0e5 update-method offset */
            fprintf(stderr,"[dumpreg] [%3d] slot=%04x val=%04x t=%04x side=%d upd=%04x body:",i,s,v,t,side,moff);
            for(int b=0;b<0x2a;b+=2) fprintf(stderr," %04x",*(unsigned short*)(dg+(unsigned short)(s+b)));
            /* combat/AI state snapshot: [0x17]ctl [0x26]hdg [0x30]desHdg [0x49]goal(32b) [0x89]aim [0x8b]tgtBrg
               [0x92]fireTmr [0x94]candTmr [0x97]target [0xa8]reload -- to see engagement/aim/reload posture. */
            fprintf(stderr," | c17=%02x hdg26=%04x des30=%04x goal49=%08x aim89=%04x tbrg8b=%04x ftmr92=%02x cand94=%02x tgt97=%04x rld_a8=%02x",
              dg[(unsigned short)(s+0x17)],*(unsigned short*)(dg+(unsigned short)(s+0x26)),*(unsigned short*)(dg+(unsigned short)(s+0x30)),
              *(unsigned int*)(dg+(unsigned short)(s+0x49)),*(unsigned short*)(dg+(unsigned short)(s+0x89)),*(unsigned short*)(dg+(unsigned short)(s+0x8b)),
              dg[(unsigned short)(s+0x92)],dg[(unsigned short)(s+0x94)],*(unsigned short*)(dg+(unsigned short)(s+0x97)),dg[(unsigned short)(s+0xa8)]);
            /* board:0012 steering-chain probe: obj+0x1b(rosterIdx) 0x40(flags,&2=bearing-enable) 0x42(animfrm)
               0x43(animstate); the goal-resource chain DAT_5798=word[DAT_3d2a+[0x1b]*2] that ac7e reads to set bit2. */
            { unsigned char rix=dg[(unsigned short)(s+0x1b)];
              fprintf(stderr," || i1b=%02x f40=%04x(b2=%d) a42=%02x s43=%02x 3d2a[%02x]=%04x 5798=%04x",
                rix,*(unsigned short*)(dg+(unsigned short)(s+0x40)),(*(unsigned short*)(dg+(unsigned short)(s+0x40))>>1)&1,
                dg[(unsigned short)(s+0x42)],dg[(unsigned short)(s+0x43)],
                rix,*(unsigned short*)(g_mem+0x23d2a+rix*2),*(unsigned short*)(g_mem+0x25798)); }
            fprintf(stderr,"\n");
          }
        }
      }
      /* DIAGNOSTIC (FIST_SIMTRACE=N): every N engine ticks ([0x452]) print live-object count, the two
       * side unit-counts (a294/a296), and a fingerprint of all live object bodies -- to see whether the
       * per-tick sim (c0e5) ADVANCES (fp changes = movement/state; a294/a296 drop = destroys). Reads only. */
      { static long strace=-2, hb=-1; static int plive=-1,pa=-1,pb=-1,pg=-1,pu38=-1,pu3a=-1;
        if (strace==-2){ const char*e=getenv("FIST_SIMTRACE"); strace=e?atol(e):-1; }
        /* gate on a loaded roster, not on byte[0x1549]==0x1c: the discriminator leaves 0x1c when the
         * player's vehicle is destroyed and the engine switches the player to another platoon vehicle
         * (word[0x6d34] changes), and the trace went dark exactly there.  board:0017 */
        { extern int g_a296_loaded; if (strace>0 && g_a296_loaded) {
          unsigned char *dg=g_mem+0x1c000;
          unsigned t=*(unsigned short*)(dg+0x452); int st1549=dg[0x1549];
          unsigned short *fbc=(unsigned short*)(dg+0xdfbc);
          int live=0,goals=0,firereq=0,cool=0,tgt=0,tcnt=0,cand=0; for(int i=0;i<0xb6;i++){ unsigned short s=fbc[i*2]; if(!s)continue; live++;
            if(dg[(unsigned short)(s+0x17)]&0x08) goals++;
            if(dg[(unsigned short)(s+0x17)]&0x80) firereq++;   /* [0x17]&0x80 = fire-request set */
            if(dg[(unsigned short)(s+0x92)]) cool++;            /* [0x92] = fire timer/countdown */
            if(*(unsigned short*)(dg+(unsigned short)(s+0x97))) tgt++; if(dg[(unsigned short)(s+0x94)]) tcnt++; if(*(unsigned short*)(dg+(unsigned short)(s+0x9d))) cand++; }  /* [0x97] = acquired target */
          /* board:0007 -- of the acquired targets ([0x97]), how many are OBJECTIVE-flagged structures?
           * The a6e3 accept filter rejects only [0x17]&0x40 and [0x16]&0x01, and the census shows the
           * enemy objectives carry f17=1c/f16=4e -- they pass it.  So if tgtobj stays 0 the exclusion is
           * upstream, in whatever builds the candidate ([0x9d]) list, not in the accept test. */
          int tgtobj=0, tgtstruct=0;
          for(int i=0;i<0xb6;i++){ unsigned short s=fbc[i*2]; if(!s)continue;
            unsigned short tv=*(unsigned short*)(dg+(unsigned short)(s+0x97)); if(!tv)continue;
            if(dg[(unsigned short)(tv+0x17)]&0x08) tgtobj++;
            { unsigned short tt=*(unsigned short*)(dg+tv); if(tt==0x1a||tt==0x1b||tt==0x03) tgtstruct++; } }
          /* DGROUP-relative offset of DAT_2000_XXXX = XXXX + 0x4000 (DAT base seg 0x2000 = DGROUP 0x1c00 + 0x400). */
          int a=*(unsigned short*)(dg+0xe294), b=*(unsigned short*)(dg+0xe296);
          /* board:0007 -- THE OUTCOME GATE (asm 1a678..1a6c7) READS NONE OF a294/a296.  It tests, in
           * order: byte[0x6dab]!=0 -> return (editor//no-verdict mode); byte[0x6d3a]==0 -> return (the
           * player side never had a unit, i.e. pre-spawn); byte[0x6d38]==0 -> DEFEAT (outcome 1);
           * word[0x9790]==0 -> return (no objective ever existed); word[0x978e]!=0 -> return, else
           * VICTORY (outcome 0).  0x6d38/0x6d39 are the ALIVE counts per side written as one word by
           * the counter at 1a6049 (slot table 0x6d3c, 2x16 near ptrs, a slot is alive when word[di]
           * != 0x17); 0x6d3a/0x6d3b are their running high-water marks.  Tracing a294/a296 instead of
           * these is why the verdict looked unreachable: they are a different pair entirely. */
          int u38=dg[0x6d38], u39=dg[0x6d39], u3a=dg[0x6d3a], u3b=dg[0x6d3b], edt=dg[0x6dab];
          int g8e=*(unsigned short*)(dg+0x978e), g90=*(unsigned short*)(dg+0x9790);
          int ov=*(unsigned short*)(dg+0x6da0), oc=*(unsigned short*)(dg+0x6da2);
          int cmm=dg[0x6da6], css=dg[0x6da7], csub=dg[0x6da8];   /* the mission clock MM:SS:sub (4712 steps it) */
          /* board:0007 -- one-shot census of the OBJECTIVE-flagged objects (byte[+0x17]&8), the set the
           * victory gate counts.  Per object: type word[obj], team bit (b1df keys it on
           * byte[word[obj]-0x19ec]&1), damage accumulator byte[+0x1a] and its threshold byte[+0x1b]
           * (bd09: destroyed when the 8-bit add carries or acc >= threshold), and the destroyed flag
           * byte[+0x19]&4.  A threshold of 0 or an already-set destroyed bit would mean the object can
           * never take the bd36 edge that clears the objective bit. */
          { static int censused=0;
            if (!censused && g8e>0) { censused=1;
              for(int i=0;i<0xb6;i++){ unsigned short so=fbc[i*2]; if(!so) continue;
                if(!(dg[(unsigned short)(so+0x17)]&0x08)) continue;
                unsigned short ty=*(unsigned short*)(dg+so);
                int team=dg[(unsigned short)(ty-0x19ec)]&1;
                fprintf(stderr,"[objcensus] slot=%04x type=%04x team=%d acc=%u thr=%u dead=%d f16=%02x f17=%02x f19=%02x\n",
                  so,ty,team,dg[(unsigned short)(so+0x1a)],dg[(unsigned short)(so+0x1b)],
                  (dg[(unsigned short)(so+0x19)]&4)?1:0,dg[(unsigned short)(so+0x16)],
                  dg[(unsigned short)(so+0x17)],dg[(unsigned short)(so+0x19)]);
              }
              /* the side slot table 0x6d3c: 16 own then 16 other near offsets, with type and ammo words */
              for(int sd=0;sd<2;sd++){ fprintf(stderr,"[objcensus] side%d:",sd);
                for(int k=0;k<16;k++){ unsigned short o=*(unsigned short*)(dg+0x6d3c+sd*32+k*2); if(!o) continue;
                  fprintf(stderr," %04x(t%02x hp%u ammo %u/%u/%u/%u)",o,*(unsigned short*)(dg+o),dg[(unsigned short)(o+0x3a)],
                    *(unsigned short*)(dg+(unsigned short)(o+0xaf)),*(unsigned short*)(dg+(unsigned short)(o+0xb1)),
                    *(unsigned short*)(dg+(unsigned short)(o+0xb3)),dg[(unsigned short)(o+0xb5)]); }
                fprintf(stderr,"\n"); }
              /* the player object's team, for comparison */
              { unsigned short ps0=*(unsigned short*)(dg+0x6d34);
                if(ps0){ unsigned short pty=*(unsigned short*)(dg+ps0);
                  fprintf(stderr,"[objcensus] PLAYER slot=%04x type=%04x team=%d\n",ps0,pty,dg[(unsigned short)(pty-0x19ec)]&1); } }
            } }
          long bucket=t/strace;
          /* player tank = registry index 0 (slot c05c, t=0); track its position (obj+4,+8 = 32-bit X,Y)
           * to see whether AUTO CONTROL is DRIVING it (position moves) or it sits idle. */
          /* the player is word[DGROUP:0x6d34] (every `cmp 0x6d34,%di` is-player test in the engine); registry
           * index 0 (word[0xdfbc]) only HAPPENS to hold it from mission start and is reused by the next spawn
           * once the vehicle is destroyed -- reading fbc[0] here misreported a projectile as the player. */
          unsigned short ps=*(unsigned short*)(dg+0x6d34); long px=0,py=0; int pctl=0;
          short vh=0,vs=0,vx=0,vy=0,f26=0,f30=0;
          /* board:0017 -- the player's DRIVE group.  [0x3e] is the drive-state index into 7c1d's table-2
           * (0 = none; 2/4 = a376/a3a8 turn; 6/8 = a3e2/a3ec heading step; 0xc = a3f6 steer) -- the unit
           * AI in the 0x1000 cluster WRITES it (171d0..18a92, gated on the control-device setting
           * word[0x8b43]); [0x3f]&8 is the AUTO/MANUAL label toggle; [0x40]&1 is set by aae8 when a
           * manual drive routine runs; [0x38] is speed; [0x30]/[0x8b] heading words; [0xa0] the
           * input-device sub-state (a487).  cmd3e counts LIVE objects with a nonzero [0x3e], i.e. how
           * many units the AI is commanding through that table at all. */
          int p3e=0,p3f=0,p40=0,p38=0,p30=0,p8b=0,pa0=0,p19=0,p86=0,cmd3e=0,dev=*(unsigned short*)(dg+0x8b43);
          for(int i=0;i<0xb6;i++){ unsigned short s=fbc[i*2]; if(s&&dg[(unsigned short)(s+0x3e)]) cmd3e++; }
          if(ps){ px=*(int*)(dg+(unsigned short)(ps+4)); py=*(int*)(dg+(unsigned short)(ps+8)); pctl=dg[(unsigned short)(ps+0x17)];
            p3e=dg[(unsigned short)(ps+0x3e)]; p3f=dg[(unsigned short)(ps+0x3f)]; p40=*(unsigned short*)(dg+(unsigned short)(ps+0x40));
            p38=*(short*)(dg+(unsigned short)(ps+0x38)); p30=*(short*)(dg+(unsigned short)(ps+0x30)); p8b=*(short*)(dg+(unsigned short)(ps+0x8b));
            pa0=dg[(unsigned short)(ps+0xa0)]; p19=dg[(unsigned short)(ps+0x19)]; p86=dg[(unsigned short)(ps+0x86)];
            vh=*(short*)(dg+(unsigned short)(ps+0x55)); vs=*(short*)(dg+(unsigned short)(ps+0x57));
            vx=*(short*)(dg+(unsigned short)(ps+0x59)); vy=*(short*)(dg+(unsigned short)(ps+0x5b));
            f26=*(short*)(dg+(unsigned short)(ps+0x26)); f30=*(short*)(dg+(unsigned short)(ps+0x30)); }
          if (live!=plive||a!=pa||b!=pb||goals!=pg||u38!=pu38||u3a!=pu3a||bucket!=hb){
            fprintf(stderr,"[simtrace] t=%u live=%d goals=%d a294=%d a296=%d firereq=%d cool=%d tgt=%d tcnt=%d cand=%d tgtobj=%d tgtstruct=%d  gate{alive=%d/%d hw=%d/%d obj=%d/%d edit=%d verdict=%d/%d clock=%02x:%02x:%02x st=%02x}  player{slot=%04x X=%ld Y=%ld f17=%02x h55=%d s57=%d vx59=%d vy5b=%d f26=%d f30=%d drive{3e=%d 3f=%02x 40=%04x 38=%d 30=%d 8b=%d a0=%d 19=%02x 86=%d} dev=%d cmd3e=%d}%s\n",
              t,live,goals,a,b,firereq,cool,tgt,tcnt,cand,tgtobj,tgtstruct,u38,u39,u3a,u3b,g8e,g90,edt,ov,oc,cmm,css,csub,st1549,ps,px,py,pctl,vh,vs,vx,vy,f26,f30,p3e,p3f,p40,p38,p30,p8b,pa0,p19,p86,dev,cmd3e,(live!=plive||a!=pa||b!=pb||goals!=pg||u38!=pu38||u3a!=pu3a)?"  <<CHANGE":"");
            plive=live;pa=a;pb=b;pg=goals;hb=bucket;pu38=u38;pu3a=u3a;
          }
        } }
      }
    }
#endif
    { extern void fbtrap_arm_hook(void); fbtrap_arm_hook(); }
    { extern void fist_sb_pump(void); fist_sb_pump(); }   /* SB auto-init stream: raise completion IRQ (FIST_SB) */
    { extern void fist_opl_pump(void); fist_opl_pump(); }  /* OPL FM: advance the synth in emulated time (FIST_OPL/FIST_SB) */
    /* Dev watchdog: FIST_RUNMS=<ms> dumps the framebuffer (FIST_FBDUMP) and exits after a wall-clock
     * deadline -- lets a first-light frame be captured while the engine is in its (non-returning) main
     * loop.  Runs in main context (safe for the PPM writer). */
    /* NB 64-bit: the deadline is an ABSOLUTE epoch-millisecond value (~1.78e12).  On the -m32 native
     * build `long` is 32-bit, so tv.tv_sec*1000 overflows it -> the truncated deadline is negative for
     * ~25-day epoch windows (bit 31 of epoch-ms flips every 2^31 ms), silently disabling this watchdog
     * (the `> 0` guard below fails) and hanging every FBDUMP display flow until `timeout`.  Use long long. */
    static long long g_deadline_ms = -2;
    if (g_deadline_ms == -2) {
        const char *r = getenv("FIST_RUNMS");
        g_deadline_ms = r ? atoll(r) : -1;
        if (g_deadline_ms > 0) {
            struct timeval tv; gettimeofday(&tv, 0);
            g_deadline_ms += (long long)tv.tv_sec*1000LL + tv.tv_usec/1000LL;
        }
    }
    /* Phase-pinned dump (DETERMINISM for tick-phase-dependent screens): FIST_DUMPTICK=N dumps the frame
     * when the engine's own INT-8 frame-timer counter [DGROUP:0x452] first reaches N -- NOT on a wall-clock
     * deadline.  [0x452] is bumped by the engine ISR (one per drained host tick), so its value is a pure
     * function of engine progress, IDENTICAL on native and wasm at the same logical point.  The campaign
     * mission-marker layer (FUN_1000_be47) is drawn only when [0x452]&0xf==0 and the selected marker blinks
     * on [0x452]&0x10, so its visible state depends solely on [0x452] mod 0x20 -- pinning the dump to a
     * fixed [0x452] makes that layer bit-identical across targets.  (The FIST_MOUSE script is pump-gated
     * and completes well before N; choose N large enough that both targets have settled into the modal
     * loop -- see the campaign-missions flow comment in tools/verify.sh.)  Takes precedence over FIST_RUNMS. */
    static long g_dumptick = -2;
    if (g_dumptick == -2) { const char *d = getenv("FIST_DUMPTICK"); g_dumptick = d ? atol(d) : -1;
        /* [0x452] is a 16-bit engine counter, so any cap above 0xffff can NEVER be reached and the run
         * would spin to its wall-clock watchdog instead.  Say so rather than hang silently. */
        if (g_dumptick > 0xffff) fprintf(stderr, "[fist] WARNING: FIST_DUMPTICK=%ld exceeds the 16-bit "
            "range of [0x452] and can never fire; use FIST_STOP_ON_OUTCOME or FIST_RUNMS.\n", g_dumptick); }
    if (g_dumptick > 0) {
        uint16_t cur = *(uint16_t*)(g_mem + 0x1c452);
        if ((long)cur >= g_dumptick) fist_dump_and_exit("FIST_DUMPTICK");
    }
    if (g_deadline_ms > 0) {
        struct timeval tv; gettimeofday(&tv, 0);
        long long now = (long long)tv.tv_sec*1000LL + tv.tv_usec/1000LL;
        if (now >= g_deadline_ms) fist_dump_and_exit("FIST_RUNMS watchdog");
    }
    /* The palette upload / fade step / DAC animation run inside the engine's own INT-8 handler
     * (31c3 -> [DGROUP:0x5e4] = MGA 0be2, patch 575) each drained tick below -- no shim retrace here. */
    /* EXTENDER frame-ready handshake (we ARE the Doug-Huffman extender).  On the mission cockpit path
     * FUN_1000_a84c sets d549(0x1549)=0x1c + FUN_1000_795c sets d548(0x1548)=1 ("cockpit view, waiting
     * for the next frame").  The extender's 32-bit-PM flight model -- which is NOT in FIST.DAT -- signals
     * "frame data ready" by OR-ing bit7 into that handshake byte (via the TCB+8 pointer d99b installed),
     * i.e. d548: 1 -> 0x81.  That is exactly the gate FUN_1000_795c tests to run a20d(camera) + df0e ->
     * aa10=0x24 -> e339 -> the extender 9200 windshield voxel render.  Emulate the always-ready extender
     * here.  GATED on d549==0x1c (the cockpit view, only ever set on the in-mission 459a path) so it is
     * strictly behaviour-neutral for the 19 menu/pre-mission verify flows. */
    /* d548 frame-ready: the ENGINE's own terminator FUN_0000_23ce ("orb $0x80,[0x1548]", patch 194 --
     * entry 5 of the phase-0x1c render script) is the only writer, as on the original.  The shim used
     * to force the bit here and in the op-0x4c gate; that was an APPROXIMATION and it is now OFF BY
     * DEFAULT.  Measured: 23ce is reached 134204 times in AZER1 to [0x452]=12000, so the emulation's
     * stated premise ("d548 never flips 1->0x81") does not hold; and with patch 537 in, a full
     * 47-mission sweep gives 41/47 with the forcing disabled -- the same count and the same six
     * failures (INDIA2 INDIA3 INDIA5 SYRIA1 TRAIN3 TRAIN4) as with it enabled.  Those six are the
     * precise list of where the handshake is still genuinely broken.
     * FIST_D548EMU=1 restores the old forcing for A/B comparison only.  board:0012 */
    { static int sr=-1, emu=-1; if(sr<0) sr=getenv("FIST_SIMRUN")?1:0;
      if(emu<0) emu=getenv("FIST_D548EMU")?1:0;
      uint8_t h = g_mem[0x1c000 + 0x1548];
      if (!emu) goto d548_emu_done;
      if (g_mem[0x1c000 + 0x1549] == 0x1c && (h & 0x7f) != 0 &&
          ((sr && (h & 0x80) == 0) || h == 1))
          g_mem[0x1c000 + 0x1548] = (uint8_t)(sr ? (h | 0x80) : 0x81);
      d548_emu_done: ; }
    fist_input_pump();
}

/* The INT-8: the engine's installed ISR (FUN_1000_30f8), then the sound driver's timer work and the
 * OPL synth for the machine time that passed since the previous interrupt.  Called by the clock
 * (fist_vga.c) at every channel-0 wrap; never re-entered -- a wrap met while the ISR runs (its own
 * 0x3da polls and DAC uploads move the clock) is one held interrupt, delivered on return, as the PIC's
 * single edge latch would.  Before the engine installs its vector the wrap only advances the BIOS tick. */
static int g_int8_held;
static unsigned long long g_int8_last_clock;

/* The interrupt frame.  An IRQ is transparent to the code it lands in: the CPU pushes FLAGS, the ISR
 * pushes every register it touches and iret restores the lot.  In the flat model those registers are
 * the shim lanes -- the CF and the non-AX outputs the __allregs prototypes thread through globals --
 * plus the INT reg-file at 0xf0000, which is the registers of a DOS/BIOS call in flight (the ISR fires
 * from inside fist_icall and the port pumps, i.e. anywhere).  31c3 leaves its timer carry in g_fist_cf
 * for 30f8, and a dispatch met by the interrupt then read that carry as its own callee's clc/stc -- a
 * c33c phase handler's `stc` came back as "found" and 2471 linked a node to itself (AZER4, board:0026). */
#define FIST_ISR_LANES(X) \
    X(g_fist_cf) X(g_fist_c0e5_si) X(g_fist_iter_si) X(g_fist_r_cx) \
    X(g_fist_b71_dx) X(g_fist_b71_cx) X(g_fist_b71_bx) X(g_fist_baf_dx) X(g_fist_0927_dx) \
    X(g_fist_0541_cx) X(g_fist_0541_dx) X(g_ext_find_cf) X(g_fist_ev_node) X(g_fist_paintbp) \
    X(g_fist_a19e_al) X(g_fist_evax) X(g_fist_paintax) X(g_fist_rot_h) X(g_fist_rot_dx) X(g_fist_rot_cx) \
    X(g_fist_render_si) X(g_fist_b1df_ax) X(g_fist_0578_bx) X(g_fist_0578_cx) X(g_fist_c8e8_si) X(g_fist_02e8_si) X(g_fist_177f_bx) \
    X(g_fist_ctx_bx) X(g_fist_03a9_dx) X(g_fist_fp_dx) X(g_fist_fp_cx) X(g_fist_r48_dx) X(g_fist_r48_cx) \
    X(g_fist_3e29_cx) X(g_fist_ext_esi) X(g_fist_ext_ecx) X(g_fist_ext_edx) X(g_fist_ext_edi) \
    X(g_fist_op50_si) X(g_fist_op50_dx) X(g_fist_op50_cx) X(g_fist_op50_edx) X(g_fist_op50_esi) X(g_fist_op50_edi) X(g_ext_edx) \
    X(g_fist_ext_edx_out) X(g_fist_1345_bp) X(g_fist_054c_bx) X(g_fist_054c_cx) X(g_fist_054c_dx) \
    X(g_fist_render_di) X(g_fist_render_dx) X(g_mga_fade_es)
#define FIST_ISR_SAVE(v)   __typeof__(v) isr_##v = v;
#define FIST_ISR_RESTORE(v) v = isr_##v;
extern unsigned char g_fist_cf, g_ext_find_cf, g_fist_a19e_al;
extern unsigned short g_fist_c0e5_si, g_fist_iter_si, g_fist_ev_node, g_fist_paintbp, g_fist_evax,
    g_fist_paintax, g_fist_rot_h, g_fist_rot_dx, g_fist_rot_cx, g_fist_render_si, g_fist_b1df_ax,
    g_fist_0578_bx, g_fist_0578_cx, g_fist_c8e8_si, g_fist_02e8_si, g_fist_177f_bx, g_fist_ctx_bx, g_fist_03a9_dx, g_fist_fp_dx,
    g_fist_fp_cx, g_fist_r48_dx, g_fist_r48_cx, g_fist_3e29_cx, g_fist_ext_ecx, g_fist_ext_edx,
    g_fist_ext_edi, g_fist_1345_bp, g_fist_054c_bx, g_fist_054c_cx, g_fist_054c_dx, g_fist_render_di,
    g_fist_render_dx, g_mga_fade_es;
extern int g_fist_r_cx;
extern uint16_t g_fist_b71_dx, g_fist_b71_cx, g_fist_b71_bx, g_fist_baf_dx, g_fist_0927_dx,
    g_fist_0541_cx, g_fist_0541_dx;
extern uint32_t g_fist_ext_esi, g_fist_ext_edx_out;
extern uint16_t g_fist_op50_si, g_fist_op50_dx, g_fist_op50_cx;
extern uint32_t g_fist_op50_edx, g_fist_op50_esi, g_fist_op50_edi, g_ext_edx;
#define FIST_ISR_REGFILE_LIN 0xf0000u
#define FIST_ISR_REGFILE_LEN 0x40u

void fist_int8_fire(void){
    extern unsigned long long fist_clock_now(void);
    FIST_ISR_LANES(FIST_ISR_SAVE)
    uint8_t isr_regfile[FIST_ISR_REGFILE_LEN];
    memcpy(isr_regfile, g_mem + FIST_ISR_REGFILE_LIN, FIST_ISR_REGFILE_LEN);
    if (!g_int8_set) {                      /* the BIOS INT 8 until the engine takes the vector; after
                                               that 30f8 chains to it through [0x432] (fist_dos.c INT 08) */
        (*(volatile uint32_t*)(g_mem+BIOS_TICK_LIN))++;
        fist_pump_slow();
        FIST_ISR_LANES(FIST_ISR_RESTORE)
        memcpy(g_mem + FIST_ISR_REGFILE_LIN, isr_regfile, FIST_ISR_REGFILE_LEN);
        return; }
    if (g_in_isr) { g_int8_held = 1; return; }
    do {
        g_int8_held = 0;
        code *fn = fist_icall(g_int8_lin);
        if (!fn) return;
        { extern void fist_queue_check(const char*); fist_queue_check("pre-isr"); }
        g_in_isr = 1;
        ((int(*)(int,int,int,int,int,int,int,int,int,int))fn)(0,0,0,0,0,0,0,0,0,0);
        g_isr_runs++;
        { extern long g_ready_vblanks; extern int g_menu_ready; if (g_menu_ready) g_ready_vblanks++; }
        { extern void fist_snd_isr_tick(void); fist_snd_isr_tick(); } /* drive the SOUNDDVR timer-ISR music sequencer (FIST_SB) */
        { unsigned long long now = fist_clock_now();
          extern void fist_opl_tick_counts(unsigned);            /* the OPL synth for the counts that passed */
          fist_opl_tick_counts((unsigned)(now - g_int8_last_clock)); g_int8_last_clock = now; }
        /* board:0001 cause-2 (audio cadence determinism): g_in_isr stays 1 ACROSS the snd/opl ticks --
           fist_opl_tick's note dispatch writes OPL regs via out(), whose pump must not deliver a nested
           interrupt (it is held instead), so the whole ISR+audio step is atomic on both targets. */
        g_in_isr = 0;
        { extern void fist_queue_check(const char*); fist_queue_check("post-isr"); }
        fist_pump_slow();
#ifdef __EMSCRIPTEN__
        if (g_web_mode) fist_web_vblank();                       /* pace to the wall clock, post frame + audio */
#endif
    } while (g_int8_held);
    FIST_ISR_LANES(FIST_ISR_RESTORE)                             /* iret */
    memcpy(g_mem + FIST_ISR_REGFILE_LIN, isr_regfile, FIST_ISR_REGFILE_LEN);
}

/* DIAGNOSTIC (FIST_QCHK=1): validate the event-queue free-list + ready-list invariants each pump so a
 * corruption is caught DETERMINISTICALLY at its source instead of chasing the flaky downstream SEGV.
 * Node pool = 63 nodes @ DGROUP:0x18ea stride 0x12 (0x18ea..0x202e); links/handles = 16-bit DGROUP
 * offsets, 0xffff sentinel; tail-slot addr 0x18e4; template/current d8e0 initial 0x2058. */
static int qchk_valid_node(unsigned o){
    return (o >= 0x18ea && o <= 0x202e && ((o - 0x18ea) % 0x12) == 0);
}
void fist_queue_check(const char *where){
    static int on = -1; if (on < 0) on = getenv("FIST_QCHK") ? 1 : 0;
    if (!on) return;
    #define QG(o) (*(uint16_t*)(g_mem + 0x1c000 + (uint16_t)(o)))
    uint16_t d8e0=QG(0x18e0), d8e2=QG(0x18e2), d8e4=QG(0x18e4), d8e6=QG(0x18e6), d8e8=QG(0x18e8), d8de=QG(0x18de);
    /* Arm only after the free-list init (3446) has run: canonical post-init signature has tail=0x18e4 or a
     * valid node, free-head 0xffff-or-valid, ready-head 0xffff-or-valid, and is not all-zero. */
    static int armed = 0;
    if (!armed){
        int inited = (d8e6==0x18e4 || qchk_valid_node(d8e6)) &&
                     (d8e8==0xffff || qchk_valid_node(d8e8)) &&
                     (d8e4==0xffff || qchk_valid_node(d8e4)) &&
                     (d8e6|d8e8|d8e4) != 0;
        if (inited) armed = 1; else { return; }
    }
    int bad = 0; const char *why = "";
    /* free list walk */
    { unsigned o=d8e8, n=0; while (o != 0xffff){ if(!qchk_valid_node(o)){bad=1;why="free-node-invalid";break;} if(++n>65){bad=1;why="free-cycle";break;} o=QG(o); } }
    /* ready list walk */
    if(!bad){ unsigned o=d8e4, n=0; while (o != 0xffff){ if(!qchk_valid_node(o)){bad=1;why="ready-node-invalid";break;} if(++n>65){bad=1;why="ready-cycle";break;} o=QG(o); } }
    /* tail must be a node or the head-slot address 0x18e4 */
    if(!bad && d8e6!=0x18e4 && !qchk_valid_node(d8e6)){bad=1;why="tail-invalid";}
    /* current/template must be a node or 0x2058 */
    if(!bad && d8e0!=0x2058 && !qchk_valid_node(d8e0)){bad=1;why="current-invalid";}
    if (bad){
        fprintf(stderr, "[qchk] CORRUPT (%s) at %s: d8de=%04x d8e0=%04x d8e2=%04x d8e4(ready)=%04x d8e6(tail)=%04x d8e8(free)=%04x isr_runs=%ld\n",
                why, where, d8de, d8e0, d8e2, d8e4, d8e6, d8e8, g_isr_runs);
        _exit(77);
    }
    #undef QG
}

/* ====================== SCRIPTED DETERMINISTIC INPUT (mouse) ======================
 * The main menu is EVENT-DRIVEN, not polled: at boot FUN_1000_392a installs a mouse EVENT HANDLER
 * (INT 33h fn 0x14, ES:DX = 0f69:4348 = FUN_1000_39d7, call mask 0x1f = movement + all button events).
 * A DOS mouse driver far-calls that handler on each event with AX = condition flags, CX = virtual x
 * (0..639 for a 320-wide mode), DX = virtual y (0..199), BX = button state, SI/DI = raw mickeys.  The
 * handler records position (DGROUP dd9e=y, dda0=x>>1) and, on a button event (AX & 0x7e), far-calls the
 * menu's button sub-handler which hit-tests the 7 items.  We ARE the mouse driver, so scripted input is
 * literally synthesizing those events and far-calling the captured handler -- fully faithful.
 *
 * DETERMINISM: the menu idle loop is a fixed point (renders the same frame every iteration -- proven
 * native<->wasm bit-identical), so injecting an event during it gives a timing-independent result.  We
 * key the script on the pump count AFTER menu-enter (fist_ensure_dlist_vecs); the exact count differs
 * native vs wasm but the *outcome* (which item, which sub-screen) does not, so both converge to the same
 * stable sub-screen frame.  FIST_MOUSE selects the script:
 *   FIST_MOUSE="t:x:y:b; t:x:y:b; ..."   t = vblanks (INT-8s, 70.09 Hz) after the menu entry; x,y = PIXEL pos (0..319,0..199);
 *   b = button mask (bit0=left, bit1=right).  Steps fire in order as t is crossed; each move/button
 *   transition is delivered as the corresponding event(s).  A step with the same pos+buttons re-asserts
 *   position (idempotent).  't' can be scaled by FIST_INPUT_SCALE (default 1). */
int      g_menu_ready = 0;                 /* set by fist_ensure_dlist_vecs (menu-enter) */
static uint32_t g_mouse_handler_lin = 0;   /* captured INT 33h fn 0x14/0x0c handler (linear) */
static unsigned g_mouse_mask = 0;
static unsigned g_vx = 0, g_vy = 0, g_vbtn = 0;   /* driver virtual mouse state (CX/DX/BX for fn 3) */

void fist_input_set_mouse_handler(uint32_t handler_lin, unsigned mask){
    g_mouse_handler_lin = handler_lin; g_mouse_mask = mask;
    fprintf(stderr, "[input] mouse event handler installed @ linear 0x%05x mask=0x%02x\n", handler_lin, mask);
}
void fist_input_mouse_state(unsigned *vx, unsigned *vy, unsigned *b){ *vx=g_vx; *vy=g_vy; *b=g_vbtn; }
void fist_input_mouse_setpos(unsigned vx, unsigned vy){ g_vx=vx; g_vy=vy; }

/* exact-match resolve a handler entry to its C function, tolerating a mid-function entry a few bytes
 * past the recovered start (e.g. FUN_1000_39d7 begins with a 1-byte `nop` before the registered
 * 0f69:4348 entry -- running from the start executes the harmless nop then the identical body). */
static code *resolve_entry(uint32_t lin){
    for (unsigned d=0; d<8 && d<=lin; d++){
        unsigned lo=0, hi=fist_fmap_n; uint32_t want=lin-d;
        while(lo<hi){ unsigned mid=(lo+hi)>>1; uint32_t m=fist_fmap[mid].lin;
            if(m==want) return (code*)fist_fmap[mid].fn;
            if(m<want) lo=mid+1; else hi=mid; }
    }
    return 0;
}

/* Far-call the engine's mouse event handler with a synthesized event.  Param order is the __allregs
 * order recovered for FUN_1000_39d7: (AX flags, CX x, DX y, BX buttons). */
static int g_in_mouse_cb = 0;
static void deliver_mouse_event(unsigned flags, unsigned vx, unsigned vy, unsigned btn){
    if (!(g_mouse_handler_lin) || (flags & g_mouse_mask) == 0) return;
    code *h = resolve_entry(g_mouse_handler_lin);
    if (!h) { fprintf(stderr,"[input] handler 0x%05x unresolved\n", g_mouse_handler_lin); return; }
    g_vx=vx; g_vy=vy; g_vbtn=btn;
    g_in_mouse_cb = 1;
    ((int(*)(int,int,int,int))h)((int)flags,(int)vx,(int)vy,(int)btn);
    g_in_mouse_cb = 0;
}

/* Parsed script step */
#define MAX_MSTEP 32
static struct { long t; unsigned x,y,b; } g_mstep[MAX_MSTEP];
static int  g_mstep_n = -1;    /* -1 = not parsed yet; 0 = no script */
static int  g_mstep_i = 0;     /* next step to fire */
long g_ready_vblanks = 0;      /* INT-8s (vblanks, 70.09 Hz) since the menu entry -- the script's time unit (board:0026) */
static unsigned g_last_btn = 0;

static void parse_script(void){
    g_mstep_n = 0;
    const char *s = getenv("FIST_MOUSE"); if(!s) return;
    long scale = 1; { const char *sc=getenv("FIST_INPUT_SCALE"); if(sc){ long v=atol(sc); if(v>0) scale=v; } }
    while (*s && g_mstep_n < MAX_MSTEP){
        while (*s==' '||*s==';'||*s=='\t') s++;
        if(!*s) break;
        long t=strtol(s,(char**)&s,0); if(*s==':')s++;
        long x=strtol(s,(char**)&s,0); if(*s==':')s++;
        long y=strtol(s,(char**)&s,0); if(*s==':')s++;
        long b=strtol(s,(char**)&s,0);
        g_mstep[g_mstep_n].t=t*scale; g_mstep[g_mstep_n].x=(unsigned)x;
        g_mstep[g_mstep_n].y=(unsigned)y; g_mstep[g_mstep_n].b=(unsigned)b;
        g_mstep_n++;
    }
    fprintf(stderr, "[input] FIST_MOUSE script: %d step(s)\n", g_mstep_n);
    for(int i=0;i<g_mstep_n;i++)
        fprintf(stderr, "  [%d] t=%ld pos=(%u,%u) btn=%u\n", i, g_mstep[i].t, g_mstep[i].x, g_mstep[i].y, g_mstep[i].b);
}

/* Called each pump (from fist_timer_pump).  Fires due script steps once the menu is up. */
void fist_input_pump(void){
    if (g_in_mouse_cb) return;                 /* re-entry guard (handler runs engine code) */
    /* FIST_CAM_SELFTEST: once the menu is up (STRSEG string resource loaded), run the REAL campaign-list
     * builder FUN_0000_ef5e -- which for each file A.CAM..L.CAM opens it (bd14), decrypts it via the
     * extender op-0x80 gate (fist_extender_gate above), and parses the "DESC:" line into the campaign
     * NAME buffer -- then dump the parsed names.  This exercises the op-0x80 .CAM decrypt + the whole
     * parser chain end-to-end on real engine code (not a standalone stub).  Expected roster:
     * TRAINING / OVERWATCH / CROSSED SWORDS / AEGIS / CERTAIN FURY / FIRE HAMMER / BURNING FROST. */
    if (getenv("FIST_CAM_SELFTEST")) {
        static int camtest_done = 0;
        extern int g_menu_ready;
        if (!camtest_done && g_menu_ready) {
            extern void FUN_0000_ef5e(unsigned long, unsigned short, int);
            camtest_done = 1;
            fprintf(stderr, "[camtest] menu ready, STRSEG=0x%04x -- running FUN_0000_ef5e (real decrypt+parse)\n",
                    *(uint16_t *)(g_mem + 0x1c000 + 0x70));
            FUN_0000_ef5e(0, 0, 0);
            for (int i = 0; i < 12; i++) {
                uint16_t off = *(uint16_t *)(g_mem + 0x2b941 + i * 2);
                if (off == 0) break;
                uint8_t flag = g_mem[0x1c000 + (uint16_t)off];
                fprintf(stderr, "[camtest] campaign %2d: loaded=%d name='%s'\n",
                        i, flag, (char *)(g_mem + 0x1c000 + (uint16_t)off + 1));
            }
            if (getenv("FIST_CAM_EXIT")) { fprintf(stderr, "[camtest] done -- exiting\n"); exit(0); }
        }
    }
    if (g_mstep_n < 0) parse_script();
    if (g_mstep_n == 0 || !g_menu_ready || !g_mouse_handler_lin) return;
    while (g_mstep_i < g_mstep_n && g_ready_vblanks >= g_mstep[g_mstep_i].t){
        unsigned nx=g_mstep[g_mstep_i].x, ny=g_mstep[g_mstep_i].y, nb=g_mstep[g_mstep_i].b;
        unsigned vx = nx*2, vy = ny;           /* mode-13h virtual coords: x doubled, y 1:1 */
        fprintf(stderr, "[input] step %d @vblank%ld [0x452]=%u -> move (%u,%u) btn %u->%u\n",
                g_mstep_i, g_ready_vblanks, *(uint16_t *)(g_mem + 0x1c452), nx, ny, g_last_btn, nb);
        /* movement event first (bit0) */
        deliver_mouse_event(0x01, vx, vy, g_last_btn);
        /* button transitions: left=bit0 of mask, right=bit1 */
        unsigned pressed  = nb & ~g_last_btn;
        unsigned released = g_last_btn & ~nb;
        if (pressed  & 1) deliver_mouse_event(0x02, vx, vy, nb);   /* left  pressed  */
        if (released & 1) deliver_mouse_event(0x04, vx, vy, nb);   /* left  released */
        if (pressed  & 2) deliver_mouse_event(0x08, vx, vy, nb);   /* right pressed  */
        if (released & 2) deliver_mouse_event(0x10, vx, vy, nb);   /* right released */
        g_last_btn = nb;
        g_mstep_i++;
    }
    /* FIST_AUTOBATTLE (board:0012): DETERMINISTIC AZER1 reach with no click-timing dependence.  The menu
     * SCREEN transitions are pump/render-paced, so an instrumented run's clicks land on the wrong screen.
     * Instead of timing clicks, FORCE the two transient menu modals to their OK/ACCEPT outcome the instant
     * they are active -- detected by their spin flags: the cc33/cb7c battle-list spins on
     * DAT_2000_a85d==0xff (set <2 = OK), the 7088 briefing (id DAT_2000_4be5==0) spins on
     * DAT_2000_4be2==0 (set =1 = ACCEPT).  BATTLES is the FIST_MOUSE click above (main menu = robust
     * fixed point).  FIST_FSG_BATTLE=AZER1 pins the battle in FUN_0000_4754.  No fprintf here -> keeps the
     * hot pump non-perturbing so the shipped timing is preserved. */
    { static int ab = -1; if (ab < 0) ab = getenv("FIST_AUTOBATTLE") ? 1 : 0;
      if (ab && g_mstep_i >= g_mstep_n && g_mem[0x1c000 + 0x1549] != 0x1c) {
        uint16_t *a85d = (uint16_t *)(g_mem + 0x2a85d);
        uint16_t *be2  = (uint16_t *)(g_mem + 0x24be2);
        uint16_t *be5  = (uint16_t *)(g_mem + 0x24be5);
        if (*a85d == 0xff) *a85d = 0;                          /* battle-list active -> force OK */
        else if (*be5 == 0 && (*be2 & 0xff) == 0) *be2 = 1;    /* briefing active   -> force ACCEPT */
      }
    }
}

static void load_image(void) {
    FILE *f = fopen(IMAGE_PATH, "rb");
    if (!f) { fprintf(stderr, "FATAL: cannot open %s (run from repo root)\n", IMAGE_PATH); exit(1); }
    size_t n = fread(g_mem, 1, IMAGE_SIZE, f);
    fclose(f);
    fprintf(stderr, "[fist] loaded %zu bytes of engine image into g_mem[0..0x%zx]\n", n, n);
}

/* ---- DOS-loader role: DGROUP service-vector table + PSP + environment block ----
 * The real Doug-Huffman extender/DOS, at load, (a) installs the engine's DGROUP service-dispatch
 * table -- 4-byte far-ptrs off:seg into the CRT/service segment (runtime seg load_seg+0xf69, base-0
 * seg 0xf69), reached by the engine via `call far [DGROUP:0xNN]`; and (b) hands the engine a PSP
 * (ES=PSP seg) whose 0x2c word points at the DOS environment block. Both are null in the static
 * image (re_out/fist_dat_image.bin) and would trap (indirect call -> linear 0 / env read -> host 0)
 * without them. The DGROUP image below is the oracle-confirmed runtime table
 * (tools/oracle/samples/dgroup_0x0_0x100.bin) rebased to our base-0 model (segment 0x2382->0xf69,
 * load_seg 0x1419->0, PSP placeholder 0x1409->FIST_PSP_SEG). DGROUP:0x12 = 0xf69:0x1b2 -> linear
 * 0xf842 (FUN_0000_f842), which the indirect-call dispatcher resolves via its FUN map. */
#define DGROUP_LIN 0x1c000u
static const uint8_t g_dgroup_init[0x70] = {
  0xc3,0xcb,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x5f,0x01,0x69,0x0f,0x8d,0x01,0x69,0x0f,0xb2,
  0x01,0x69,0x0f,0xd0,0x01,0x69,0x0f,0xd9,0x01,0x69,0x0f,0xff,0x01,0x69,0x0f,0x24,0x02,0x69,0x0f,
  0x42,0x02,0x69,0x0f,0x79,0x02,0x69,0x0f,0xa2,0x02,0x69,0x0f,0xd0,0x02,0x69,0x0f,0xfb,0x02,0x69,
  0x0f,0x00,0x00,0x8a,0x01,0x00,0x00,0x90,0x01,0x00,0x00,0x04,0x00,0x69,0x0f,0x99,0x01,0x00,0x00,
  0x7b,0x01,0x00,0x00,0x0e,0x00,0x69,0x0f,0x2a,0x00,0x69,0x0f,0x14,0x03,0x69,0x0f,0x6a,0x03,0x69,
  0x0f,0x76,0x03,0x69,0x0f,0x96,0x03,0x69,0x0f,0x00,0x98,0x00,0x00,0x00,0x00,0x69,0x0f
};
/* fist_apply_dgroup_relocs(): apply the engine's OWN DGROUP relocation table (loader/CRT role).
 * The table lives at seg 0x3352:0 = linear 0x33520 (f738 sets DGROUP:0x74=0x3352, the table segment,
 * which FUN_0000_f7c3 loads into BX; then FUN_0000_f7ef applies the near/offset section and
 * FUN_0000_f842 -- the DGROUP:0x12 service -- applies the far off:seg sections). The real Doug-Huffman
 * extender + CRT walk this table at load and populate the engine's DGROUP indirect-call VECTOR TABLE:
 * the 11 service vectors (DGROUP:0x0a-0x36 -> seg 0xf69) AND every device/stream METHOD vector
 * (DGROUP:0x78-0x4b8), including the driver-filename BUILDER at DGROUP:0x31a = 0xf69:0x560a = linear
 * 0x14c9a = FUN_1000_4c9a (patch 006). Table format (asm-verified from f7ef/f842):
 *   near section (first): base word (skip), then (off,val) pairs until off==0; DGROUP[off]=val+addend
 *     where addend=(base-load_seg)*16 -- = 0 in our base-0 model (load_seg=0, base=0).
 *   far sections (repeat): seg word, then (off,val) pairs until off==0; DGROUP[off]=val, DGROUP[off+2]=seg.
 * All segments in the image are already base-0 (0x0000 code seg, 0x0f69 service seg), so relocation is
 * the identity for our model, exactly reproducing the oracle-captured runtime DGROUP (validated slot-
 * for-slot against tools/oracle/samples/dgroup_0x0_0x100.bin: service vectors 0x0a/0x0e/0x12/... and
 * STRSEG 0x70 all match; only the four non-table CRT/loader stores 0x00/0x68/0x74/0x76 differ, and
 * those are seeded separately here / by f738). Ghidra dropped the x86 string-op segment bases in the C
 * f7ef/f842 (DS=table-seg, ES=DGROUP), so the engine's own appliers are inert -- the loader role applies
 * the table faithfully instead, from the game's own relocation data (nothing invented). */
#define RELOC_TAB_LIN 0x33520u
/* Apply ONE relocation section of the engine's DGROUP reloc table (at seg 0x3352:si). is_far selects
 * FUN_0000_f842 (0xf842) semantics (leading seg word, then (off,val) pairs -> DGROUP[off]=val,
 * DGROUP[off+2]=seg) vs FUN_0000_f7ef (0xf7ef) NEAR semantics: BOTH section kinds begin with a leading
 * WORD which is skipped.  For near it is a BASE PARAGRAPH -- the applier computes addend=(base-load_seg)*16
 * and stores DGROUP[off]=val+addend (all near base_para=0 and load_seg=0 in our base-0 model => addend 0).
 * (The old code did NOT skip the near leading word, so a near apply read the leading 0x0000 as the first
 * `off` and terminated at 0 entries -- silently inert; asm-verified fix vs f7ef 0xf7fa..0xf815.)
 * Returns the number of (off,val) entries applied. */
int fist_apply_reloc_section(uint16_t si, int is_far) {
    #define RW(a) (*(uint16_t*)(g_mem + (a)))
    uint32_t p = RELOC_TAB_LIN + si;
    uint16_t seg = 0, addend = 0;
    uint16_t lead = RW(p); p += 2;              /* leading word: far=section seg, near=base para */
    if (is_far) seg = lead; else addend = (uint16_t)(lead << 4);
    int n = 0;
    for (;;) {
        uint16_t off = RW(p);
        if (off == 0) break;
        RW(DGROUP_LIN + off) = (uint16_t)(RW(p + 2) + addend);
        if (is_far) RW(DGROUP_LIN + off + 2) = seg;
        p += 4; ++n;
    }
    #undef RW
    return n;
}
/* fist_clear_reloc_section(si): the NEAR-CLEAR applier FUN_0000_f81d (0xf81d, DGROUP:0x0e) -- teardown
 * counterpart of fist_apply_reloc_section(si,0).  Asm 0xf828..0xf83a: `mov ds,bx; add si,2` (skip the
 * leading base word), then for each (off,val) pair write DGROUP[off]=0 (skipping the val word).  Same
 * offsets the near install populated, cleared to 0.  Returns the number of entries cleared. */
int fist_clear_reloc_section(uint16_t si) {
    #define RW(a) (*(uint16_t*)(g_mem + (a)))
    uint32_t p = RELOC_TAB_LIN + si + 2;        /* skip the leading base word */
    int n = 0;
    for (;;) {
        uint16_t off = RW(p);
        if (off == 0) break;
        RW(DGROUP_LIN + off) = 0;
        p += 4; ++n;
    }
    #undef RW
    return n;
}
/* fist_apply_reloc_at(seg, si, is_far): apply ONE relocation section located at an ARBITRARY segment
 * (g_mem + (seg<<4) + si) rather than the engine's fixed table -- the driver-LOCAL reloc apply.  The
 * MGAVIDEO driver init (FUN_0000_0009, asm 0x86: `mov bx,[DGROUP:0x70e]; xor si,si; lcall [DGROUP:0x12]`)
 * runs the FAR reloc applier FUN_0000_f842 over the method-vector reloc section it copied (via f98b) into
 * the buffer whose allocated segment sits at DGROUP:0x70e.  f842 semantics (asm 0xf842): `mov ds,bx;
 * lods` leading seg word -> cx; then loop `lods off; if 0 stop; DGROUP[off]=<section value word>;
 * DGROUP[off+2]=cx`.  Ghidra rendered the engine's f842 inert (dropped its x86 string-op DS/ES segment
 * bases), so apply the section faithfully here.  The section's leading seg word is ALREADY the driver
 * load-seg: the copied template's seg-word site (module linear 0x52c0) is an MZ reloc, relocated at LOAD
 * by the AH=4B overlay loader -- so applying verbatim installs the driver method vectors as far pointers
 * off:load_seg into the overlay range (e.g. DGROUP:0x540 = 0xe8:load_seg = the VGA mode-set
 * FUN_0000_00e8).  Returns the number of (off,val) entries applied. */
int fist_apply_reloc_at(uint16_t seg, uint16_t si, int is_far) {
    #define RW(a) (*(uint16_t*)(g_mem + (a)))
    uint32_t p = ((uint32_t)seg << 4) + si;
    uint16_t s = 0;
    if (is_far) { s = RW(p); p += 2; }
    int n = 0;
    for (;;) {
        uint16_t off = RW(p);
        if (off == 0) break;
        RW(DGROUP_LIN + off) = RW(p + 2);
        if (is_far) RW(DGROUP_LIN + off + 2) = s;
        p += 4; ++n;
    }
    #undef RW
    return n;
}
/* The driver-overlay-subsystem sections (0xec device-method vectors incl. 0xd0; 0x258 near vectors;
 * 0x274 stream/overlay vtable incl. the filename BUILDER 0x31a = 0xf69:0x560a = FUN_1000_4c9a) are
 * NOT installed here at load: the engine installs them itself, at the correct time (in FUN_1000_06bc /
 * FUN_1000_4b16, just before the driver EXEC-load and AFTER the SOUND.CFG parse), via the DGROUP:0x0a/
 * 0x12 relocation services -- whose C bodies Ghidra rendered inert (dropped string-op segment bases).
 * Patches on 06bc/4b16 call fist_apply_reloc_section() at exactly those points so the vectors go live
 * only when they must (installing them at load fires not-yet-initialised method vectors and corrupts
 * the SOUND.CFG parse path). See patches/022. */

/* fist_install_dgroup(): (re)install the DGROUP service-vector table. Called from the CRT init
 * (FUN_0000_f738, via patches/001) AFTER its BSS clear zeroes DGROUP -- the real loader's relocated
 * service far-ptrs (seg = load_seg+0xf69) are re-established here because f738's `rep stosw` wipes
 * them and the engine's own reloc-applier can't recompute the load_seg-relative segment. Base-0
 * image; DGROUP:0x12 -> 0xf69:0x1b2 = linear 0xf842. Also (re)asserts DGROUP:0x68 = PSP seg. */
void fist_install_dgroup(void) {
    memcpy(g_mem + DGROUP_LIN, g_dgroup_init, sizeof g_dgroup_init);
    /* DGROUP:0x70 = the engine's far string/resource data segment. The engine reads it as SS:0x70
     * (SS=DGROUP=0x1c00 after the CRT stack fixup) in 60+ sites as a data segment holding filename
     * templates (e.g. "CONFIG.DVR"/"SOUND.CFG" at +0x7e/+0x89) which it copies into filename buffers
     * (FUN_1000_4c9a etc.). Oracle ground truth: at runtime DGROUP:0x70 = base_seg + 0x2d74
     * (base_seg 0x1519 -> stored value 0x428d; 0x428d-0x1519 = 0x2d74; the word at that segment is
     * the engine's link-time far data group, image offset 0x2d740 = "...CONFIG.DVR\0SOUND.CFG..."").
     * It is BSS (0) in the static image and is never set by a plain store in the engine (the CRT
     * init computes it from the load base), so the loader role seeds the base-0 paragraph here. */
    *(uint16_t *)(g_mem + DGROUP_LIN + 0x70) = 0x2d74;
    /* SMC callback-list terminator sentinel: seg 0x1000 : 0x19c0 (linear 0x119c0) = 0xdb8b (`mov bx,bx`).
     * The engine keeps a self-modifying-code linked list of far-CALL/JMP thunks (a callback registry)
     * in an un-decompiled scratch region at seg 0x1000 ~0x19bb; the list HEAD is 0x19bb and the fixed
     * empty-list TERMINATOR is the thunk at 0x19c0, whose leading word must be 0xdb8b so the list walk
     * (FUN_1000_4cd0, `cmpw [si+5],0xdb8b`) stops. FUN_1000_06bc builds the head (0x19bb) via 4cb9 with
     * fwd->0x19c0 and appends the first node (0x19c9); without the terminator the walk spins forever.
     * The engine's startup establishes this sentinel in that SMC scratch region -- which our decompile
     * does not cover (Ghidra left 0x1970..0x1a13 as raw un-analyzed bytes, so the init that writes it is
     * not in the executed C). Ground truth is derivable but not observable via the oracle (the original
     * faults under QEMU BEFORE reaching 06bc -- verified: 0x119bb/0x119c0 are pristine 0x71.../0x554f in
     * the live crash dump). Seed it in the loader role, analogous to the DGROUP runtime-established
     * slots above. The 5-byte thunks are inert for our C-compiled control flow (we never execute g_mem
     * as code); only the list-management memory must stay faithful. TODO: locate the exact engine init
     * and demote this to a patch. */
    *(uint16_t *)(g_mem + 0x119c0) = 0xdb8b;
    /* DGROUP:0x240.. (CPU class, speed rating, video chipset, mouse version, XMS/EMS free, DOS version,
     * conventional/disk free ..) are NOT written by the engine: they come from LOADGAME's hardware/OS
     * detection script, handed over through the FIST.RUN PSP -- seeded below in setup_dos_env
     * (the PSP + environment block), where the whole handshake lives.  board:0017 */
    /* FRAME/EVENT SCHEDULER method vectors -- reloc-table section si=0x1d8 (leading seg 0xf69):
     *   DGROUP:0x3fe=0xf69:0x3e0b  0x402=0x3e65  0x406=0x3e05  0x412=0x3ed6
     *   DGROUP:0x40a=0xf69:0x3f17 (the frame/vsync POLL FUN_1000_35a7 that FUN_1000_38cc spins on)
     *   0x40e=0x422c  0x416=0x423c (=FUN_1000_38cc)  0x41a=0x424e  0x41e=0x4265
     * These are far pointers into the engine's own service segment 0xf69 -- the SAME KIND as the 12
     * service vectors 0x0a..0x36 already in g_dgroup_init -- so the real Doug-Huffman CRT reloc walk
     * installs them at load.  The engine's own section-0x1d8 sites do NOT re-install them at runtime:
     * FUN_1000_3446 (scheduler INIT) calls the far applier f842 (DGROUP:0x12) with bx==0, which f842
     * BAILS on (`or bx,bx; je`); FUN_1000_3485 (scheduler SHUTDOWN) calls the CLEAR applier f860
     * (DGROUP:0x16), which zeroes them.  So the install is a boot/loader-role responsibility, done here
     * from the game's own reloc data (base-0 => identity), exactly like the service table above.
     * Without it DGROUP:0x40a stays 0 -> fist_icall_far(0) traps -> FUN_1000_38cc's frame-wait spins
     * forever (the intro-animation player FUN_0000_e584 hangs before its first frame). */
    fist_apply_reloc_section(0x1d8, 1);
    fprintf(stderr, "[fist] DGROUP service table installed (DGROUP:0x12 -> 0xf69:0x1b2 = 0xf842; "
                    "DGROUP:0x70 = 0x2d74 str-seg; SMC cb-list terminator 0x1000:0x19c0 = 0xdb8b; "
                    "video chipset DGROUP:0x246 = 0x56 -> MGAVIDEO; frame-scheduler vecs 0x3fe..0x41e "
                    "incl. vsync-poll 0x40a=0xf69:0x3f17 = FUN_1000_35a7)\n");
}

/* Frame-scheduler CARRY flag: FUN_1000_35a7 (the frame/vsync poll) returns status in the x86 CF via
 * stc/clc+lret (CF=1 = frame boundary reached / event-queue empty).  The __allregs model returns AX,
 * not flags, so the reconstructed FUN_1000_35a7 (patch 073) writes CF here and FUN_1000_38cc (patch
 * 074) spins on it -- faithful to `do { call far [DGROUP:0x40a] } while(jae)`. */
unsigned char g_fist_cf;

/* PATCH 335: the c0e5 per-object UPDATE walk (patch 243) dispatches each object method keeping the LIST
 * CURSOR live in SI and the object near-offset in DI (passed as arg0).  A few of those methods (b5e7 and
 * its mid-entry twin b60f) additionally read `[si+0x10]` off the live cursor on their object-destruct
 * branch (DAT_2000_5bd7 = (-[di+0x10]-[si+0x10])>>0xc).  The __allregs C dispatch threads only DI, so the
 * cursor SI is lost; c0e5 publishes it here per iteration and the method reads it (faithful to the live
 * register).  A DGROUP near-offset into the 0xdfbc object list. */
unsigned short g_fist_c0e5_si;

/* board:0017 -- FIST_RNGTRACE=<path>: every LFSR result (0291, `rng <ax> t=<tick>`) and every sim
 * step (c0e5, `sim t=<tick>`) in order, the port's side of the oracle's FIST_REGTRACE at 0x11447/
 * 0x11454/0x1d275/0x24288 -- the two streams set against each other give the tick where the poll
 * cadence or a sim draw first differs.  Off unless the variable is set; no engine state touched. */
static FILE *g_rngtrace_f; static int g_rngtrace_init;
static FILE *rngtrace_file(void) {
    if (!g_rngtrace_init) { const char *p = getenv("FIST_RNGTRACE"); g_rngtrace_init = 1;
        if (p && (g_rngtrace_f = fopen(p, "w"))) setvbuf(g_rngtrace_f, NULL, _IOLBF, 1 << 16); }   /* the dump path _exit()s */
    return g_rngtrace_f;
}
void fist_rng_trace(unsigned short v) {
    FILE *f = rngtrace_file();
#ifndef __EMSCRIPTEN__
    if (f) fprintf(f, "rng %04x t=%u from=%p<%p\n", v, *(uint16_t *)(g_mem + 0x1c452), __builtin_return_address(1), __builtin_return_address(2));   /* 0291's caller and its caller (-O0 frames) */
#else
    if (f) fprintf(f, "rng %04x t=%u\n", v, *(uint16_t *)(g_mem + 0x1c452));
#endif
}
/* FIST_RNG_SCHEDULE=<file>: one integer per line = the oracle's poll count before each of its mission
 * sim steps (from the FIST_REGTRACE P/S markers).  Active from the seed point: the live poll's LFSR
 * step is suppressed (fist_icall_near) and the schedule's count is stepped here before each sim step,
 * so the sim draws exactly the oracle's values as long as it draws the same number of times. */
int g_rng_sched_active; static int *g_rng_sched; static int g_rng_sched_n, g_rng_sched_i;
int fist_rng_noop(void) { return 0; }
static void rng_sched_load(void) {
    const char *p = getenv("FIST_RNG_SCHEDULE"); FILE *f; int cap = 0, v;
    if (!p || !(f = fopen(p, "r"))) return;
    while (fscanf(f, "%d", &v) == 1) { if (g_rng_sched_n == cap) { cap = cap ? cap * 2 : 4096; g_rng_sched = realloc(g_rng_sched, cap * sizeof *g_rng_sched); } g_rng_sched[g_rng_sched_n++] = v; }
    fclose(f); g_rng_sched_active = 1;
    fprintf(stderr, "[rngsched] %d sim steps scheduled\n", g_rng_sched_n);
}
void fist_poll_trace(void) {
    FILE *f = rngtrace_file(); if (f) fprintf(f, "poll t=%u\n", *(uint16_t *)(g_mem + 0x1c452));
}
static void frame_sched_load(void); static void phase_sched_load(void);
void fist_mission_trace(void) {   /* 4754, the battle load: the first mission draw (4779) follows */
    FILE *f = rngtrace_file(); if (f) fprintf(f, "mission t=%u\n", *(uint16_t *)(g_mem + 0x1c452));
    { static int done; if (done) return; done = 1;
      { const char *sd = getenv("FIST_RNG_SEED");
        if (sd) { unsigned v[5]; if (sscanf(sd, "%x,%x,%x,%x,%x", &v[0], &v[1], &v[2], &v[3], &v[4]) == 5) {
            *(uint16_t *)(g_mem + 0x1df82) = (uint16_t)v[0];
            for (int i = 0; i < 4; i++) *(uint16_t *)(g_mem + 0x1df84 + 2 * i) = (uint16_t)v[1 + i];
            if (f) fprintf(f, "seed t=%u\n", *(uint16_t *)(g_mem + 0x1c452)); }
          rng_sched_load(); frame_sched_load(); phase_sched_load(); } } }
}
/* The sync point: the extender's op-0x18 MAP-LOAD entry (the oracle's 0x100010ca) -- before the
 * mission's own initialisation draws.  FIST_RNG_SEED=<ptr>,<w84>,<w86>,<w88>,<w8a> (hex): the LFSR
 * state the oracle had there; the schedule's first count is the polls before the first sim step. */
/* FIST_PHASE_SCHEDULE=<file>: one integer per render = the oracle's render-phase dispatches (22f7)
 * in that 22dd call -- its INT-8 budget word[0x450] expired after that many; with the INT-8s held the
 * port's budget never expires, so the schedule zeroes it after the same count. */
static int *g_phase_sched; static int g_phase_n, g_phase_i, g_phase_cnt, g_phase_active;
static void phase_sched_load(void) {
    const char *p = getenv("FIST_PHASE_SCHEDULE"); FILE *f; int cap = 0, v;
    if (!p || !(f = fopen(p, "r"))) return;
    while (fscanf(f, "%d", &v) == 1) { if (g_phase_n == cap) { cap = cap ? cap * 2 : 4096; g_phase_sched = realloc(g_phase_sched, cap * sizeof *g_phase_sched); } g_phase_sched[g_phase_n++] = v; }
    fclose(f); g_phase_active = 1;
    fprintf(stderr, "[phasesched] %d renders scheduled\n", g_phase_n);
}
static int g_phase_cur;
void fist_render_trace(void) {   /* every 22dd entry, phase loop or not: one schedule entry per call */
    FILE *f = rngtrace_file(); if (f) fprintf(f, "render t=%u\n", *(uint16_t *)(g_mem + 0x1c452));
    g_phase_cnt = 0;
    if (g_phase_active) { g_phase_cur = (g_phase_i < g_phase_n) ? g_phase_sched[g_phase_i] : 0x7fffffff;
        if (g_phase_i == g_phase_n) fprintf(stderr, "[phasesched] schedule exhausted at t=%u\n", *(uint16_t *)(g_mem + 0x1c452));
        g_phase_i++; }
}
void fist_phase_trace(void) {   /* after each 22f7 phase dispatch */
    FILE *f = rngtrace_file(); if (f) fprintf(f, "phase t=%u\n", *(uint16_t *)(g_mem + 0x1c452));
    if (g_phase_active && ++g_phase_cnt >= g_phase_cur) *(uint16_t *)(g_mem + 0x1c450) = 0;
}
void fist_render_end_trace(void) { }
void fist_map_trace(void) {
    FILE *f = rngtrace_file(); if (f) fprintf(f, "map t=%u\n", *(uint16_t *)(g_mem + 0x1c452));
}
/* FIST_FRAME_SCHEDULE=<file>: one integer per render = the oracle's sim steps before it (its 22dd
 * markers).  At the mission loop's top (459a's cooperative pump, patch 295) the port pumps until
 * [0x452] has moved by that many ticks -- zero for the renders the original squeezed in without a tick
 * passing -- so the sim/render interleaving is the oracle's.  Active with the RNG schedule. */
static int *g_frame_sched; static int g_frame_n, g_frame_i, g_frame_active;
static void frame_sched_load(void) {
    const char *p = getenv("FIST_FRAME_SCHEDULE"); FILE *f; int cap = 0, v;
    if (!p || !(f = fopen(p, "r"))) return;
    while (fscanf(f, "%d", &v) == 1) { if (g_frame_n == cap) { cap = cap ? cap * 2 : 4096; g_frame_sched = realloc(g_frame_sched, cap * sizeof *g_frame_sched); } g_frame_sched[g_frame_n++] = v; }
    fclose(f); g_frame_active = 1;
    { extern int g_int8_replay; g_int8_replay = 1; }
    fprintf(stderr, "[framesched] %d renders scheduled\n", g_frame_n);
}
void fist_frame_tick(void) {
    if (!g_frame_active) { fist_timer_pump(); return; }
    if (g_frame_i >= g_frame_n) { if (g_frame_i == g_frame_n) fprintf(stderr, "[framesched] schedule exhausted at t=%u\n", *(uint16_t *)(g_mem + 0x1c452)); g_frame_i++; fist_timer_pump(); return; }
    /* the loop's own accounting: sim steps this frame = [0x452] - word[0x6ce0] (2ce0, the tick at the
     * last paint); the render's port accesses may already have moved the clock, so pump the rest. */
    /* the clock still runs (retrace edges, the PIT reads) but its INT-8s are held (g_int8_replay);
     * the schedule fires exactly n of them here, so the loop's `2ce2 += [0x452] - 2ce0` is the oracle's. */
    { int n = g_frame_sched[g_frame_i++]; int guard = 1000; extern void fist_int8_fire(void);
      uint16_t t0 = *(uint16_t *)(g_mem + 0x1c452);
      while ((uint16_t)(*(uint16_t *)(g_mem + 0x1c452) - t0) < (uint16_t)n && guard-- > 0) fist_int8_fire(); }
}
void fist_sim_trace(void) {
    FILE *f = rngtrace_file();
    /* FIST_STEP0_DUMP=<path>: g_mem's first megabyte at the first mission sim step -- the port's state at
     * the sync point, against the oracle's FIST_REGTRACE_DUMPLIN=1d275 RAM image. */
    { static long hits, n = -1, every = -1; extern int g_fist_after_map;
      if (g_fist_after_map) { ++hits;
        if (n < 0) { const char *dn = getenv("FIST_STEPN_DUMP"); n = dn ? atol(dn) : 1; const char *de = getenv("FIST_STEPEVERY_DUMP"); every = de ? atol(de) : 0; }
        /* the Nth mission sim step (FIST_STEPN_DUMP, default 1), or every k steps into <file>.<hit> (FIST_STEPEVERY_DUMP) */
        { const char *dp = getenv("FIST_STEP0_DUMP"); char nm[512]; const char *fn = 0;
          if (dp) { if (every) { if ((hits - 1) % every == 0) { snprintf(nm, sizeof nm, "%s.%ld", dp, hits); fn = nm; } } else if (hits == n) fn = dp; }
          if (fn) { FILE *df = fopen(fn, "wb"); if (df) {
              const char *dz = getenv("FIST_STEP_DUMPLEN");   /* a DGROUP window (from 0x1c000) instead of the megabyte */
              if (dz) fwrite(g_mem + 0x1c000, 1, strtoul(dz, 0, 16), df); else fwrite(g_mem, 1, 0x100000, df); fclose(df);
              fprintf(stderr, "[step0] g_mem dumped at t=%u hit %ld -> %s\n", *(uint16_t *)(g_mem + 0x1c452), hits, fn); } } }
        { static long stop = -1; if (stop < 0) { const char *st = getenv("FIST_STOP_AT_STEP"); stop = st ? atol(st) : 0; }
          if (stop && hits == stop) { fprintf(stderr, "[step0] FIST_STOP_AT_STEP %ld reached at t=%u\n", stop, *(uint16_t *)(g_mem + 0x1c452)); fflush(NULL); exit(0); } } } }
    if (g_rng_sched_active) {
        int n = (g_rng_sched_i < g_rng_sched_n) ? g_rng_sched[g_rng_sched_i] : 0;
        if (g_rng_sched_i == g_rng_sched_n) fprintf(stderr, "[rngsched] schedule exhausted at t=%u\n", *(uint16_t *)(g_mem + 0x1c452));
        g_rng_sched_i++;
        while (n-- > 0) { extern undefined4 __allregs FUN_0000_0291(void); FUN_0000_0291(); }
    }
    if (f) fprintf(f, "sim t=%u\n", *(uint16_t *)(g_mem + 0x1c452));
}

/* PATCH 292: the mission-object-roster iterator FUN_0000_c33c returns its updated SI register (always
 * 0xffff on both the found and exhausted exits -- asm 0xc381/0xc386 `mov si,0xffff`) here; its caller
 * FUN_0000_378e stores it into word[DGROUP:0x4b9e] (asm 0x37a7 `mov [0x4b9e],si`) so the NEXT iterator
 * call sees si!=0 and skips the re-INIT (word[0x4b9e]==0 is the once-per-frame INIT trigger). */
unsigned short g_fist_iter_si;

/* PATCH 324: the reticle-graticule line-clip helper FUN_0000_3d99 (asm 0x3d99) draws two clipped
 * line-halves and returns the FIRST half's result in CX (asm: `push ax` after half-1's `call 0xdb5`,
 * then `pop cx` after half-2) and the SECOND half's result in AX.  The __allregs model threads only AX
 * (the return); Ghidra dropped the CX output, so its caller FUN_0000_3d4c stored a stale sign-flag value
 * into word[DGROUP:0x6c38] instead of half-1's clipped-X.  3d99 publishes half-1's db5 result here; 3d4c
 * (its only reticle caller, synchronous) reads it into word[0x6c38] so 3a68 can pass it as 3a92's column
 * (cx).  Set-then-read in the same call chain -> a single global is exact. */
int g_fist_r_cx;

/* PATCH 328: the flight-model BEARING/RANGE math chain (0541 -> 0731/077e bearing + 0927 -> 0b71/0baf
 * range) returns its results in AX *and* the non-AX registers CX/DX/BX, which the __allregs prototype
 * model (AX/EAX only) drops.  Each is published by the producer and consumed by its single, synchronous
 * caller one call later, so a plain global is exact (same discipline as g_fist_r_cx above):
 *   g_fist_b71_dx/cx/bx : FUN_0000_0b71's 48-bit squared-delta (ax:dx:cx) + the accumulator word
 *                         selector BX (asm 0xb90/0xb92/0xba4/0xba6/0xb96/0xbaa) -> read by FUN_0000_0927.
 *   g_fist_baf_dx       : FUN_0000_0baf's DX (provably 0 on every exit -- asm 0xbf8 `mov dx,cx` with
 *                         cx==0, and 0xc2d `pop dx` restoring the 0 input) -> read by FUN_0000_0927's
 *                         `shl ax,1; rcl dx,1` (0x989) and `mov dl,ah` (0x9a5) result paths.
 *   g_fist_0927_dx      : FUN_0000_0927's range HIGH word (the DX of its dx:ax return) -> read by 0541.
 *   g_fist_0541_cx/dx   : FUN_0000_0541's range_lo/range_hi (asm 0x548 `mov cx,ax` + 0927's DX) -> read
 *                         by ab91/abb7/abde/ac09 for their `shl edx,8; mov dl,ch; mov [di+0x53],dx`
 *                         range store. */
uint16_t g_fist_b71_dx, g_fist_b71_cx, g_fist_b71_bx;
uint16_t g_fist_baf_dx;
uint16_t g_fist_0927_dx;
uint16_t g_fist_0541_cx, g_fist_0541_dx;

/* Extender FILEMGR find/open CARRY flag (patch 210).  The extender's find-first chain
 * FUN_0000_6250 / 5d50 / 5cc2 / 5c98 signals found (clc, CF=0) vs not-found (stc, CF=1)
 * purely through the x86 CF; the __allregs model returns AX/void, not flags, so Ghidra
 * dropped the CF at every level (5c98's in_CF hardcoded 0, 5d50 void, 5cc2/6250 no CF)
 * -> 89b0's four .MEG RAM-size probes always read "found" and bumped the detail/LOD word
 * [0x8490] from the TCB base 0x0b up to 0xc.  The reconstructed chain publishes each
 * function's clc/stc here; the immediate synchronous call order (89b0->5c98->5cc2->5d50
 * ->6250) makes a single global exact.  Distinct from g_fist_cf (engine seg-0 paint CF);
 * no engine code runs between producer and consumer in this extender-local chain. */
unsigned char g_ext_find_cf;

/* Event-queue node handle returned by FUN_1000_3566 (asm 0x13566: `call 0xf69:0x3ed6` returns the
 * freshly-allocated queue node in BX).  The __allregs model returns void, so the reconstructed 3566
 * (patch 126) publishes BX here and the hit-test FUN_0000_1eb4 (asm 0x1f20/0x1f39 `mov [bx],0x200/0x400`)
 * writes the synthesized LEAVE/ENTER event record into THAT node -- not into the old hover element
 * (the patch-128 `oldfe0c` write was a reconstruction bug that clobbered the element node's word0). */
unsigned short g_fist_ev_node;

/* Current PAINT-walk base (bp) published by the 209e dirty-walk before each paint-method dispatch.
 * The container child-walk FUN_0000_208a reads bp as a live register in the asm (preserved from 209e
 * through `call [di+0x423c]`), but the __allregs model passes no args to paint methods, so patch 137
 * shortcut-read the GLOBAL DAT_1000_fafe instead -- which is the MENU base (f7b8) while the modal dialog
 * is up (the cursor redraw updates fafe), so the dialog's container repaint after a row-select walked
 * the wrong element list and the red selection bar never moved.  Publish the real walk bp here. */
unsigned short g_fist_paintbp;
/* board:0012 patch 513: the 209e dirty-walk's live AX.  459a/45f7 call 206f with the 4691 event/poll
 * word in AX; 206f leaves it alone and 209e clobbers only AL (the cell byte after `shr al,cl`), so the
 * handlers the walk dispatches run with (AH from the caller):(AL from the cell).  The port's dispatch is
 * arg-less, so 206f publishes g_fist_evax and 209e composes g_fist_paintax for the handlers to read --
 * the same mechanism patch 142 uses for the walk base BP. */
/* board:0012 patch 514: ad3b's al=1/0 gear-direction selector, which a19e's one-argument dispatch
 * cannot carry (patch 283 recorded it as dropped). */
unsigned char g_fist_a19e_al;
unsigned short g_fist_evax;
unsigned short g_fist_paintax;

/* PATCH 309: the 3-in/3-out rotation FUN_0000_0459 (a192 thunk -> 0459) camera-delta trig.
 * a20d feeds a192(ax=word[di+0x10], dx=0xaaa) with the vehicle heading cx=word[di+0x38] (asm 0x1a232
 * `mov cx,[di+0x38]`), but a192's C sig was 2-arg so Ghidra DROPPED the cx input; and 0459 returns 3
 * values (ax=X, dx=Y, cx=Z rotation deltas -- asm 0x4c1/0x4c3/0x4c4 `mov ax,dx; pop dx; pop cx`) but
 * __allregs returns only ax, so the dx/cx outputs were dropped (the `extraout_` multi-return class).
 * a20d sets g_fist_rot_h = cx (the heading) before calling a192; 0459 reads it as its 3rd (cx) input
 * and publishes the dx/cx rotation outputs here (the immediate a20d/a192/0459 chain is synchronous, so
 * a single global set is exact).  Distinct from the object-projection a192 callers (addb/60861 etc.),
 * which don't set g_fist_rot_h and ignore the dx/cx outputs -> unchanged (non-crashing, imprecise-as-
 * before for those dormant paths). */
unsigned short g_fist_rot_h;    /* input: 0459's cx (heading) */
unsigned short g_fist_rot_dx;   /* output: 0459's dx (Y delta) */
unsigned short g_fist_rot_cx;   /* output: 0459's cx (Z delta) */

/* PATCH 310: the mission-object DEPTH-SORT render-record builder threads three registers as LIVE
 * values through TWO indirect dispatches the __allregs flat model passes no args across:
 *   2471(di=node bx) -> c33c(di) -> `call [DGROUP:0xe6f5+bx]` (phase handler, e.g. c4df)
 *   c4df(si=object, di=node) -> `call [STRSEG:type*2+0x358a/..]` (per-type record method, e.g. c681)
 *   c681 -> c962 -> ca2f  (build the ~0x32-byte record INTO the node at ES:di, ES=DS=DGROUP)
 * The dest cursor di (2471 node pool DGROUP:0x4ba2..0x6a92) is advanced by ca2f/c962 and flows BACK to
 * 2471 (asm 0x24a2 uses the c33c-returned di for the node link).  di is the single source of truth here
 * (in/out); si = the source object near-offset; dx = the per-type record code (c4df dl=byte[type-0x1b74]).
 * All are DGROUP near offsets (2471 asm 0x2481 `push ds; pop es` => ES=DGROUP). */
unsigned short g_fist_render_si;   /* c4df->method: source object near-offset */
/* PATCH 462 (board:0007/0012): dropped multi-register outputs of the object-spawn/aim chain. */
unsigned short g_fist_b1df_ax;     /* b1df: AX = display-table index of the freshly spawned object */
unsigned short g_fist_0578_bx;
unsigned short g_fist_0578_cx;     /* PATCH 594: 0578's exit CX = 0927's range LOW word (a265's CH) */
unsigned short g_fist_c8e8_si;     /* PATCH 595: c8e8's exit SI (advanced past its copy; c64a's second add) */
unsigned short g_fist_02e8_si;     /* PATCH 563: SI = the CRT number printer's output cursor (02e8 -> 541b -> 030b) */
unsigned short g_fist_177f_bx;     /* PATCH 571: 177f's BX out -- the largest free block when the MEMMGR request fails */
unsigned short g_fist_ctx_bx;      /* PATCH 575: 024f/026e's BX out -- the surface / text context they replaced */
unsigned short g_fist_03a9_dx;     /* 03a9: DX = M*cos(A) (AX = M*sin(A) is the return) */
unsigned short g_fist_fp_dx;       /* PATCH 468: 0d13/0d55/0db5/0df7/0e22 DX lane (exponent in/out) */
unsigned short g_fist_fp_cx;       /* PATCH 468: 0df7/0e22 CX lane (divisor/multiplier exponent in) */
unsigned short g_fist_r48_dx, g_fist_r48_cx;   /* PATCH 469: 1322/129f 48-bit result lanes (cx:dx:ax) */
unsigned short g_fist_3e29_cx;     /* PATCH 469: 3e29 CX out (projected screen x) */
uint32_t g_fist_ext_esi;           /* PATCH 471: op-0x40 (e132) ESI lane = polygon linear address */
unsigned short g_fist_ext_ecx, g_fist_ext_edx, g_fist_ext_edi;   /* PATCH 471: op-0x40 CX/DX/DI lanes */
uint32_t g_fist_ext_edx_out;       /* PATCH 573: op-0x3c (e115) EDX result = the uploaded record's address in the model block */
unsigned short g_fist_1345_bp;     /* PATCH 471: 1345 BP out = the MEMMGR list header (0x16d4/0x16f6/0x1718) */
unsigned short g_fist_054c_bx;     /* PATCH 473: 054c/bbc6 BX out = the pitch (077e over the Z delta) */
/* PATCH 542: 054c also returns the 32-bit RANGE in CX:DX -- asm 0x558 `push %ax ; push %dx` then
 * 0x56f `pop %dx ; pop %cx`, so at return CX = the range LOW word and DX = the range HIGH word.
 * FUN_0000_9fe5 consumes both (`a004 mov %ch,%cl ; a006 mov %dl,%ch` = range>>8). */
unsigned short g_fist_054c_cx, g_fist_054c_dx;
unsigned short g_fist_render_di;   /* 2471<->writers: dest node cursor (in/out, advanced by ca2f/c962) */
unsigned short g_fist_render_dx;   /* c4df->method->c962: per-type record code (dl=byte[type-0x1b74]) */

/* fist_ensure_dlist_vecs(): install reloc section si=0x174 -- the DISPLAY-LIST ELEMENT method vectors
 * DGROUP:0x344..0x394 (far pointers into the engine service seg 0f69, incl. DGROUP:0x388 = 0f69:0x306c
 * = FUN_1000_26fc, the RESOURCE OPEN the driver's element-load thunk calls to open+alloc+read a screen
 * resource -- e.g. MAINMENU.MRL -- and store its loaded segment into the display-list descriptor's
 * word0).  In the original the Doug-Huffman CRT installs this at load (via the working far applier
 * f842) from FUN_1000_223c; in our port f842's C body is inert (Ghidra dropped its string-op segment
 * bases) AND applying it at the 223c CRT-init point corrupts the not-yet-ready DGROUP (breaks a later
 * f842 caller FUN_1000_5c3a -- see patch 091).  So install it in the loader role at the correct, later
 * time: menu/screen-enter (FUN_0000_e714), after all boot+intro init, before the first element paint
 * that dispatches through these vectors.  Idempotent (applies once; the section values are base-0
 * identity, so re-apply would be harmless anyway). */
static int g_dlist_vecs_done;
void fist_ensure_dlist_vecs(void) {
    if (g_dlist_vecs_done) return;
    g_dlist_vecs_done = 1;
    g_menu_ready = 1;           /* menu-enter: scripted input may now be delivered */
    int n = fist_apply_reloc_section(0x174, 1);
    fprintf(stderr, "[fist] display-list element method vectors installed "
                    "(reloc section si=0x174: DGROUP:0x344..0x394 -> seg 0f69; %d entries; "
                    "0x388 = resource-open FUN_1000_26fc)\n", n);
}

/* ---- Doug-Huffman EXTENDER SERVICE GATE (extender role of the shim) ----------------------------
 * The engine reaches this via FUN_0000_e339: `lcall [DGROUP:0xea16]` on the pointer FUN_0000_d99b
 * decoded from the extender's config blob.  In the original that pointer is the extender's low-memory
 * real-mode->32-bit-PM callback trampoline (oracle: 0x0762:0x1179 = linear 0x8799); it stack-switches
 * and far-jumps into the Doug-Huffman extender's PM service dispatcher.  Here the extender role IS the
 * shim, so fist_icall routes linear FIST_EXTGATE_LIN (0x8799) to this handler.  e339 pushes the arg
 * block (word[ea1a],word[ea1c],word[ea1e], dword 0) for the gate and RETURNS the gate's AX, so this
 * C function's return value becomes AX (-> d99b stores it as the task segment at DGROUP:0xea2e).
 *
 * Service selector = DGROUP:0xea10 (aa10), set by the caller (d99b/db3f/...) before each e339 call:
 *   op 0    -> CREATE TASK.  Allocate a paragraph-aligned, ZEROED, real-mode-addressable task-control
 *              block of word[ea1a] BYTES (ea1a=0x0f30=3888 for the intro/title task; the TCB is used
 *              at offsets up to ~0x492/0x3f2, so ea1a is a byte size).  Return its segment in AX.
 *              d99b then fills the TCB (entry seg:off, context, the four intro asset-name templates
 *              D02.PCX/C02.PCX/502.PAL/5.SKY).  In the original the extender carves the TCB from its
 *              OWN reserved low DOS memory (below the child engine, e.g. seg 0x0762); our base-0 model
 *              has no space below the engine, so the extender pool is the free gap ABOVE the engine's
 *              MEMMGR heap-top (0x9000) and below the PSP (0x9800) -- 32 KB, real-mode addressable.
 *   op != 0 -> task-control op (aa10 = 4/0x1c/0x20/0x58/...).  The engine's e339 tail only enters the
 *              task scheduler ([DGROUP:0x58]) when the TCB's first word is nonzero, which it is NOT on
 *              this single-task init path (nothing sets task[0]), so these are observably no-ops here.
 *              Logged (never silently swallowed).  A cooperative-task scheduler is a later-stage
 *              concern and is not on the path to first light. */
#define EXT_TASK_POOL_BASE 0x9000u    /* seg; linear 0x90000 = free gap above heap-top, below PSP 0x9800 */
#define EXT_TASK_POOL_TOP  0x9800u
static uint16_t g_ext_next_seg = EXT_TASK_POOL_BASE;

/* ---- EXTENDER KDV-PLAYER MODULE (re_out/fist_ext.c, FLAT32 unit) --------------------------------
 * The intro FMV (armoredfist/FISTDATA/TITLE.KDV) is decoded + blitted by the Doug-Huffman extender's
 * OWN 32-bit-flat code -- NOT by FIST.DAT.  That code is decompiled (re_out/fist_kernel_decomp.c ->
 * fist_ext.c) and linked as a third module here.  Its self-contained KDV cluster:
 *   m_ext_FUN_0000_11cb  = OPEN wrapper  (reads the asset name from TCB+0xBA = "TITLE.KDV", opens it)
 *   m_ext_FUN_0000_11dd  = DECODE+PRESENT wrapper (reads a chunk, RLE-decodes an IMAGE frame, uploads
 *                          the palette via the VGA DAC, and blits 320x200 to the framebuffer [0x917])
 * MEMORY MODEL: the extender is flat, linked base 0; we place its image at FIST_EXT_BASE in g_mem and
 * its flat POINTER slots hold HOST pointers into g_mem so the real bump-allocator / RLE decoder /
 * present blit run with native derefs (no per-deref rebase).  Seeds (module-relative, rebased to
 * FIST_EXT_BASE):  [0x807]=0 identity map ; [0x917]=&g_mem[0xA0000] framebuffer ; [0xc93]=&g_mem[TCB]
 * ; [0x90b]/[0x90f]=MEMMGR heap base/top.  (The engine's 16-bit create-task builds the TCB at linear
 * 0x90000 and e584 writes "TITLE.KDV" into TCB+0xBA, so [0xc93] points the KDV OPEN at it.) */
extern void m_ext_FUN_0000_11cb(int, int, int, int, int);   /* KDV OPEN wrapper */
extern void m_ext_FUN_0000_11dd(int, int, int);             /* KDV DECODE+PRESENT wrapper */
extern void m_ext_FUN_0000_6f17(int, int, int, int, int, int); /* KDV close/free wrapper */
extern void m_ext_FUN_0000_89b0(unsigned, unsigned, unsigned); /* op-0x18 MAP-LOAD setup (terrain/pal/sky) */
extern void m_ext_FUN_0000_84c0(unsigned);                     /* extender task-setup allocator (bc90 matrix etc.) */
extern void m_ext_FUN_0000_8deb(void);                         /* voxel viewport setup (90a8/90f0/90f8/9114) */
extern void m_ext_FUN_0000_85d0(void);                         /* voxel camera setup (TCB 0x2c/0x30/0x34 -> proj) */
extern void m_ext_FUN_0000_82b8(int, int);                     /* OPAQUE-LOD voxel terrain render (8120->9200->82d0) */
extern void m_ext_FUN_0000_8120(void);                         /* voxel camera->projection (steps 90b8/90bc/90d4/90d8) */
extern void m_ext_FUN_0000_9200(int, int);                     /* voxel per-column texel walk (the terrain writer) */
static int  g_ext_ready;      /* module image loaded + slots seeded */
static int  g_kdv_open;       /* TITLE.KDV opened (once) */
static long g_kdv_frames;     /* frames decoded (diagnostic) */
int         g_ext_eof;        /* KDV read-chunk CF-out: set by patch-084 (708b EOF/err) -> read here */
static int  g_kdv_done;       /* intro finished (EOF reached) -> op 0x78 returns "animation complete" */
extern int  g_fist_ext_int;   /* fist_dos.c: route INT 21h through the flat-mode FILEMGR while set */
static const char g_ext_path_root[1] = { 0 };  /* FILEMGR path-root = empty string */
static uint8_t    g_ext_dta_buf[128];           /* FILEMGR DTA (find-first result buffer) */
static uint8_t    g_ext_kdv_tcb[0x1000];        /* dedicated extender current-TCB (see gate below) */

static void ext_module_init(void) {
    FILE *f = fopen("re_out/fist_image.bin", "rb");
    if (!f) { fprintf(stderr, "[ext] FATAL: cannot open re_out/fist_image.bin\n"); return; }
    size_t n = fread(g_mem + FIST_EXT_BASE, 1, FIST_EXT_IMG_SIZE, f);
    fclose(f);
    /* register in the overlay table: wires fist_ext_fmap / fist_ext_base from fist_ovl_known["EXT"]
     * (so module accessors resolve and fist_icall dispatches any ext-internal indirect call). */
    fist_ovl_register("EXT", FIST_EXT_BASE, FIST_EXT_SPAN);
    uint8_t *xb = g_mem + FIST_EXT_BASE;                       /* extender module base */
    *(uint32_t *)(xb + 0x807) = 0;                                              /* identity map */
    *(uint32_t *)(xb + 0x917) = (uint32_t)(uintptr_t)(g_mem + 0xA0000);         /* framebuffer */
    *(uint32_t *)(xb + 0xc93) = (uint32_t)(uintptr_t)(g_mem + 0x90000);         /* current TCB */
    /* board:0012 -- MEMMGR heap base MUST be 0x10000-aligned in HOST space.  FUN_0000_3772 (the bump
     * allocator) aligns the cursor by `(cursor + A-1) & ~(A-1)` on the ABSOLUTE host-pointer value
     * (max A = 0x10000).  g_mem's host base has different low bits native vs wasm, so an unaligned heap
     * base makes the alignment padding base-dependent -> the whole extender heap relayouts between the
     * two targets (colormap et al. shift), and a sibling allocation lands OOB on wasm (native tolerates
     * the in-bounds miss).  The real Doug-Huffman extender maps its heap at a page-aligned linear base;
     * replicate that so the per-target layout is identical relative to the heap base. */
    uint32_t heap_base = (uint32_t)(uintptr_t)(g_mem + FIST_EXT_HEAP);
    heap_base = (heap_base + 0xffffu) & ~0xffffu;                               /* align host base to 0x10000 */
    *(uint32_t *)(xb + 0x90b) = heap_base;                                      /* heap base (aligned) */
    *(uint32_t *)(xb + 0x90f) = (uint32_t)(uintptr_t)(g_mem + FIST_EXT_HEAP_TOP);/* heap top */
    *(uint32_t *)(xb + 0x2f50) = 0;   /* MEMMGR cursor 0 => (re)init from [0x90b] on first alloc */
    *(uint32_t *)(xb + 0x2f54) = 0;   /* MEMMGR live-block count */
    /* FILEMGR path tables (normally set by the extender's own FILEMGR init, which this port does not
     * run).  The resolver FUN_00005cc2/5d50 builds a search path = [0x6234] path-root + filename and
     * issues INT 21h find/open.  Seed the path-root to an EMPTY string (host pointer to a NUL) so the
     * branch is taken but no prefix is prepended -> the bare filename ("TITLE.KDV") is opened, which
     * open_ci() resolves under $FIST_DATADIR/FISTDATA.  Resource-dir [0x622c] and alt-drive [0x6238]
     * are cleared (skip the secondary search).  The DTA pointer [0x927] is a real host buffer that
     * find-first fills (file size at +0x1a, read by the KDV OPEN). */
    *(uint32_t *)(xb + 0x6234) = (uint32_t)(uintptr_t)g_ext_path_root;   /* path root = "" */
    *(uint32_t *)(xb + 0x622c) = 0;                                      /* resource dir: none */
    *(uint32_t *)(xb + 0x6238) = 0;                                      /* alt drive/path: none */
    *(uint32_t *)(xb + 0x0927) = (uint32_t)(uintptr_t)g_ext_dta_buf;     /* DTA buffer */
    g_ext_ready = (n == FIST_EXT_IMG_SIZE);
    fprintf(stderr, "[ext] KDV module loaded @g_mem+0x%x (%zu B), heap 0x%x..0x%x, fb->0x%x TCB->0x%x %s\n",
            FIST_EXT_BASE, n, FIST_EXT_HEAP, FIST_EXT_HEAP_TOP, 0xA0000, 0x90000,
            g_ext_ready ? "READY" : "(SHORT READ)");
}


int g_fist_after_map = 0;   /* set once op 0x18 (map load) has fired -> roster probe gate */
/* The 32-bit app's EDX as the last service left it.  The extender preserves the app's registers across
 * its mode switches, so a service that reads EDX before writing it sees the previous service's exit
 * value -- 8650 (op 0x60) does exactly that for its flatness window (`sub ebx,edx` with only DL set),
 * which is why the original presses a crater into some terrain and not into other, equally flat
 * terrain.  Kept by the handlers whose exit EDX the asm fixes: op 0x54 (-(y << 13)), 8650's own paths,
 * ac70's best distance, bed2/bc06.  The engine's 16-bit DX between two services is not carried (the
 * decompile does not materialise it); at the mission-start stamps the preceding service is always the
 * spawn's op 0x54, whose value the engine leaves alone (oracle traces of AZER1/TRAIN2/CYPRUS1).  board:0002 */
uint32_t g_ext_edx;
/* The op-0x50 register lanes (patch 584): 2471/3eb3 post the node's coordinate pair (SI = node+0xa) and
 * altitude (DX = word[node+6]) in the registers; ba7d answers in AX (screen y), CX (x), EDX (depth),
 * ESI/EDI (the far/near depth rows).  AX travels as the gate's return, the rest through these. */
uint16_t g_fist_op50_si, g_fist_op50_dx, g_fist_op50_cx;
uint32_t g_fist_op50_edx, g_fist_op50_esi, g_fist_op50_edi;
void fist_dbg_op2c(void) { __asm__ __volatile__(""); }   /* clean gdb breakpoint at the op-0x2c gate */
void fist_dbg_op18(void) { __asm__ __volatile__(""); }   /* clean gdb breakpoint at the first op-0x18 map-load (arm d548 watchpoint here) */
void fist_dbg_fbwild(void) { __asm__ __volatile__(""); }   /* clean gdb breakpoint when a blit dest lands outside the framebuffer (wild write) */
extern void m_ext_FUN_0000_8df0(void), m_ext_FUN_0000_3931(void), m_ext_FUN_0000_82d0(void);
/* The extender's heightmap index (0x8480 and every sampler: `shld eax,edx,DETAIL ; shld eax,ebx,DETAIL ;
 * mov al,[esi+eax]`): the top DETAIL bits of the y and x fixed-point coordinates, y-major.  DETAIL is
 * [ext+0x8490] (11 for the 2048-square maps); the map loader 89b0 patches it into the shld immediates
 * of every sampler at map load.  A 10 baked in here indexed a 1024-square map -- half the rows, every
 * sample from the wrong place -- and the ground clamp (op 0x1c), the height service (op 0x54) and the
 * line of sight (op 0x58) all answered for the wrong terrain.  board:0002 */
static inline uint32_t fist_hm_index(uint32_t y, uint32_t x, unsigned d) {
    return ((y >> (32 - d)) << d) | (x >> (32 - d));
}

int fist_extender_gate(void) {
    uint8_t *dg = g_mem + DGROUP_LIN;
    uint16_t op = *(uint16_t *)(dg + 0xea10);
    /* board:0012 EXPERIMENT: complete the frame-present handshake INSIDE the op-0x4c gate (not only in
     * the PIT pump at line ~711), because the in-mission 459a present-poll spins on op-0x4c WITHOUT
     * re-entering fist_timer_pump -> d548 never flips 1->0x81 -> the loop never advances back to the
     * per-tick sim c0ca -> the mission freezes after ~3 spawn frames.  The real flight model OR-s bit7
     * into d548 to signal "frame ready"; emulate that here so the present completes every op-0x4c and
     * the frame loop keeps running the sim. */
    /* OFF BY DEFAULT (see the note in fist_timer_pump): FIST_D548EMU=1 restores it for comparison. */
    if (getenv("FIST_D548EMU") && getenv("FIST_SIMRUN") && op == 0x4c && g_mem[0x1c000 + 0x1549] == 0x1c) {
        uint8_t h = g_mem[0x1c000 + 0x1548];
        if ((h & 0x80) == 0 && (h & 0x7f) != 0) g_mem[0x1c000 + 0x1548] = h | 0x80;  /* OR bit7 into waiting 1/2/3 (oracle) */
    }
    if (op == 0x18) { g_fist_after_map = 1;
        if (getenv("FIST_DBG_OP18")) { static int o=0; if(!o){o=1; extern void fist_dbg_op18(void); fist_dbg_op18();} } }
    /* board:0012 op-0x58 LINE-OF-SIGHT service -- faithful port of the 32-bit-PM handler decoded from
     * fist_image.bin @0x802e (board/0012_fire_cascade_reference.md): a DDA terrain ray-cast between the
     * object (TCB+0xd2/0xd6/0xda) and candidate (TCB+0xde/0xe2/0xe6) over the mission heightmap
     * [ext+0x85bc] (== the op-0x24 voxel render's).  Returns 0xffffffff (clear LOS) / 0 (occluded or out
     * of range); e21c takes (uint16), aa08 gates != 0.  NOTE: currently returns 0 for all pairs because
     * the units' Z [obj+0xc] is NOT terrain-following (the absent 32-bit-PM flight model does not sit units
     * on the ground -- proven: forcing terrain-follow Z makes candidates VISIBLE).  Wiring the per-unit
     * terrain-follow Z (like the camera-alt at the op-0x24 block) will activate this LOS -> target lock. */
    /* board:0012 op-0x54 TERRAIN-HEIGHT service.  The engine posts the DGROUP near offset of a position
     * pair (X,Y dwords) in the TCB inbox (TCB+0x3f2, written by e1d1/e1a6/adcd/9db1 as obj+4) and stores
     * the returned byte as that object's ground height [obj+0xd]; 9e2b slews the hull to it and e1a6
     * returns [obj+0xd]-[obj+0x18] (the slope) so the AI can tell up- from down-hill.  Same heightmap and
     * index packing as the decoded op-0x58 LOS handler below (ext+0x85bc, the op-0x24 voxel map).  It used
     * to answer only the projectile query (g_fist_op54_proj), leaving every OBJECT height 0. */
    /* board:0021 op 0x20 -- publish the engine's object-list view to the extender globals.
     * asm 0x0f8b (the service table at file offset 0xcb3 maps op/4 -> handler; op 0x20 -> 0x0f8b):
     *     f8b: mov 0xc93,%ebx            ; the TCB
     *     f91: movzwl 0x26(%ebx),%eax    ; the engine's DGROUP SEGMENT
     *     f95: shl $0x4,%eax             ; -> linear
     *     f98: add 0x807,%eax            ; + the guest-RAM base
     *     f9d: mov %eax,0xca1            ; [0xca1] = the DGROUP flat base
     *     fa2: movzwl 0x28(%ebx),%edi ; add %eax,%edi ; mov %edi,0xc99   ; the object list
     *     fae: movzwl 0x2a(%ebx),%edi ; add %eax,%edi ; mov %edi,0xc9d   ; a second list
     * In our model the "flat base" is the host address of DGROUP, so [0xca1] is g_mem+0x1c000 and the
     * two list pointers are that plus their TCB near offsets.  Without this op the port left
     * [0xc99]/[0xca1] at ZERO (measured), so op 0x1c below had no list to walk. */
    /* op 0x20 is posted during MISSION INIT, before g_fist_after_map goes true (the engine seeds the
     * TCB right above the post: t[0x26]=0x1c00 the DGROUP segment, t[0x28]=0x6d3c the object list,
     * t[0x2a]=0xdfbc), so it must NOT carry the after-map gate the render services use. */
    if (op == 0x20 && g_ext_ready) {
        *(uint16_t*)(dg + 0xea10) = 0;
        uint8_t *xb20 = g_mem + FIST_EXT_BASE;
        uint32_t tcb20 = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        uint8_t *tcb = g_mem + tcb20;
        uint32_t base = (uint32_t)(uintptr_t)dg;                       /* f95/f98: the DGROUP flat base */
        *(uint32_t*)(xb20+0xca1) = base;                               /* f9d */
        *(uint32_t*)(xb20+0xc99) = base + *(uint16_t*)(tcb+0x28);      /* fa2/fa6/fa8 */
        *(uint32_t*)(xb20+0xc9d) = base + *(uint16_t*)(tcb+0x2a);      /* fae/fb2/fb4 */
        return 0;
    }
    /* board:0021 op 0x1c -- the PER-UNIT GROUND CLAMP, asm 0x1109-0x11a4.  This is the writer of
     * byte[obj+0x1d] that board:0018 was missing; the type-00/01/02 step methods then copy it into the
     * ground byte ([obj+0xd]) and the object Z is ground<<8, which is the scale op-0x58 compares.
     *     1109: mov 0xc99,%edi ; 110f: mov $0x20,%ecx        ; 32 list entries, stride 2
     *     1116: movzwl (%edi),%edi ; 1119: or %di,%di ; je next
     *     111e: add 0xca1,%edi                               ; -> the object
     *     1124: cmpw $0x3,(%edi) ; ja next                   ; ONLY object types 0..3
     *     ... 0x7fa0 twice (slopes along [obj+0x26] and [obj+0x10]) -> [obj+0x32/0x34], [obj+0x22/0x24]
     *     1190: call 0x8480 ; 1195: mov %al,0x1d(%edi)       ; the terrain height
     * 0x8480 is `shld $0xa` twice + a heightmap byte read -- the same index packing the op-0x54 and
     * op-0x58 handlers above already use.  0x7fa0 samples the heightmap at four points around the
     * object along a heading taken from the 512-entry table at extender 0x9450 (entry a and a+128,
     * i.e. the cos/sin pair), each `sar $6`, and returns the two slopes <<23. */
    /* board:0002 op 0x44 -- the DETAIL / terrain-table setup service.  The engine posts it (measured:
     * 2 calls per mission with FIST_OPHIST) and the shim used to return 0, which is why the voxel ramp
     * tables were never loaded and the windshield rendered from a stale tile.
     *
     * The extender's handler is a two-hop, 0x10da -> 0x7660, and 0x10da is op 0x44 in the located
     * service table (board:0021).  asm 0x7660:
     *
     *   7670: c6 05 5c 39 00 00 01   movb $0x1,0x395c            ; default detail
     *   7677: 80 bb cc 00 00 00 00   cmpb $0x0,0xcc(%ebx)        ; TCB+0xcc
     *   767e: 74 15                  je   0x7695
     *   7680: b8 9a 68 00 00         mov  $0x689a,%eax           ; the sky-resample routine ...
     *   7685: a3 58 39 00 00         mov  %eax,0x3958            ; ... into the sky fn pointer
     *   768a: 8a 83 cc 00 00 00      mov  0xcc(%ebx),%al
     *   7690: a2 5c 39 00 00         mov  %al,0x395c
     *   7695: 8b 1d 93 0c 00 00      mov  0xc93,%ebx             ; the TCB
     *   769b: 8a 83 d1 00 00 00      mov  0xd1(%ebx),%al         ; TCB+0xd1 = detail level 0/1/2
     *   76a1: 3c 00 / 75 1b          cmp  $0x0,%al ; jne ...
     *   76a5: be b6 76 00 00         mov  $0x76b6,%esi           ; "low.dtl" (also "medium.dtl","high.dtl")
     *   76aa: b8 20 3a 00 00         mov  $0x3a20,%eax           ; destination
     *   76af: e8 7e e9 ff ff         call 0x6032                 ; DOS file load (INT 21h AH=4Eh)
     *
     * The three files are FISTDATA/{LOW,MEDIUM,HIGH}.DTL, each exactly 2052 bytes, and they map onto
     * the extender globals contiguously -- which is why nothing in the image ever addresses 0x3a24 or
     * 0x3e24, and why table B is exactly table A * 150 (both ship precomputed):
     *
     *     file[0x000] -> ext+0x3a20   the count      LOW=125  MEDIUM=190  HIGH=250
     *     file[0x004] -> ext+0x3a24   table A        256 dwords
     *     file[0x404] -> ext+0x3e24   table B        256 dwords
     *
     * NOT gated on g_fist_after_map: like op 0x20 (board:0021) this runs during mission setup, and the
     * render gate would stop it from ever firing.
     *
     * The 0x3958 store is performed as the asm does.  Note `FUN_0000_689a` does NOT exist in
     * build/fist_ext.c (it is in the undecompiled part of the extender), but the port already holds
     * 0x689a there at render time and ext+0x395d is non-zero, so FUN_0000_3931 takes its 686f/6c00
     * branch and never dispatches through the pointer.  Writing it changes nothing today and keeps the
     * transcription faithful. */
    if (op == 0x44 && g_ext_ready) {
        uint8_t  *xb44 = g_mem + FIST_EXT_BASE;
        uint32_t  tcb44_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        uint8_t  *tcb44 = g_mem + tcb44_lin;
        xb44[0x395c] = 1;                                        /* 7670 */
        if (tcb44[0xcc] != 0) {                                  /* 7677 */
            *(uint32_t*)(xb44 + 0x3958) = 0x689a;                /* 7680/7685 */
            xb44[0x395c] = tcb44[0xcc];                          /* 768a/7690 */
        }
        {   static const char *const dtl_lc[3] = { "low.dtl", "medium.dtl", "high.dtl" };
            static const char *const dtl_uc[3] = { "LOW.DTL", "MEDIUM.DTL", "HIGH.DTL" };
            unsigned lvl = tcb44[0xd1];                           /* 769b */
            if (lvl > 2) lvl = 2;                                 /* 76a1/76c0: 0 -> low, 1 -> medium, anything else -> high */
            {
                const char *dd = getenv("FIST_DATADIR"); if (!dd) dd = "armoredfist";
                char pth[512]; FILE *f = 0;
                const char *const *cand[2] = { dtl_lc, dtl_uc };
                for (int c = 0; c < 2 && !f; c++) {
                    snprintf(pth, sizeof pth, "%s/FISTDATA/%s", dd, cand[c][lvl]); f = fopen(pth, "rb");
                    if (!f) { snprintf(pth, sizeof pth, "%s/%s", dd, cand[c][lvl]); f = fopen(pth, "rb"); }
                }
                if (f) {
                    size_t n = fread(xb44 + 0x3a20, 1, 2052, f);  /* 76aa/76af: count + both tables */
                    fclose(f);
                    if (getenv("FIST_DTLLOG"))
                        fprintf(stderr, "[dtl] op44 loaded %s (%zu B) -> ext+0x3a20; count=%u\n",
                                cand[0][lvl], n, *(uint32_t*)(xb44 + 0x3a20));
                } else if (getenv("FIST_DTLLOG")) {
                    fprintf(stderr, "[dtl] op44: %s not found under '%s'\n", dtl_uc[lvl], dd);
                }
            }
        }
        *(uint16_t*)(dg + 0xea10) = 0;
        return 0;
    }
    /* board:0025 op 0x3c -- MESH-RECORD UPLOAD (ext 0x11f4, patch 573).  The engine's 1a45 posts every
     * record of a .M00/.M08/.M16/.M32 file with cx = the byte count and dx:si = the record's scratch
     * segment:0; the handler appends it to the extender's single model block (3439 on the handle at
     * ext 0x123b, growing it per call) and returns eax = -1 with EDX = the record's address, which the
     * engine stores at es:[2] -- the pointer 2e49 posts to the op-0x40 rasterizer.  The 16-bit
     * registers reach the gate through the patch-471 lanes; EDX goes back through g_fist_ext_edx_out
     * (published by the patched 11f4).  Posted during the ACCEPT mission cascade, before the map's
     * after-map gate -- like op 0x20/0x44. */
    if (op == 0x3c && g_ext_ready) {
        extern uint32_t m_ext_FUN_0000_11f4(unsigned, unsigned, unsigned);
        *(uint16_t*)(dg + 0xea10) = 0;
        return (int)m_ext_FUN_0000_11f4(g_fist_ext_ecx, g_fist_ext_edx, (unsigned)g_fist_ext_esi);
    }
    /* board:0025 op 0x4c -- .MAL PALETTE REMAP (ext 0x123f -> 9e60, patch 573).  19d1 posts it after
     * loading a model's 768-byte palette to seg:0 with cx=dx=seg, si=0, di=0x300: 9e60 writes the
     * 256-entry nearest-colour LUT (against the mission palette the op-0x18 map load reduced to) at
     * seg:0x300, and 1a45 translates every mesh texel through it.  asm 123f: esi = (dx<<4)+si+[0x807],
     * edi = (cx<<4)+di+[0x807], call 9e60.  ([0x807] = the guest-RAM window = g_mem here.) */
    if (op == 0x4c && g_ext_ready) {
        extern void m_ext_FUN_0000_9e60(unsigned char, unsigned char *, unsigned char *);
        uint8_t *xb4c = g_mem + FIST_EXT_BASE;
        uint32_t base4c = *(uint32_t*)(xb4c + 0x807);
        *(uint16_t*)(dg + 0xea10) = 0;
        m_ext_FUN_0000_9e60(0, g_mem + base4c + ((uint32_t)g_fist_ext_edx << 4) + (uint16_t)g_fist_ext_esi,
                               g_mem + base4c + ((uint32_t)g_fist_ext_ecx << 4) + g_fist_ext_edi);
        return 0;
    }
    if (op == 0x1c && g_ext_ready && g_fist_after_map) {
        *(uint16_t*)(dg + 0xea10) = 0;
        uint8_t *xb1c = g_mem + FIST_EXT_BASE;
        uint8_t *hm1c = (uint8_t*)(uintptr_t)(*(uint32_t*)(xb1c+0x85bc));
        uint32_t list = *(uint32_t*)(xb1c+0xc99), base = *(uint32_t*)(xb1c+0xca1);
        if (!hm1c || !list || !base) return 0;
        const int32_t *trig = (const int32_t *)(xb1c + 0x9450);
        const unsigned d1c = *(uint32_t*)(xb1c + 0x8490);
        uint8_t *lp = (uint8_t *)(uintptr_t)list;
        for (int n = 0; n < 0x20; n++, lp += 2) {                       /* 1114/1199/119e */
            uint16_t noff = *(uint16_t *)lp;                            /* 1116 movzwl (%edi),%edi */
            if (noff == 0) continue;                                    /* 1119 or/je */
            uint8_t *o = (uint8_t *)(uintptr_t)(base + noff);           /* 111e add 0xca1 */
            if (*(uint16_t *)o > 3) continue;                           /* 1124 cmpw $0x3 ; ja */
            for (int pass = 0; pass < 2; pass++) {                      /* 1142 and 116e */
                int32_t ebx = (int32_t)((uint32_t)*(int32_t *)(o+4) << 13);   /* 1134/1160 shl $0xd */
                int32_t edx = -(int32_t)((uint32_t)*(int32_t *)(o+8) << 13);  /* 1131/1137 */
                int16_t ang = *(int16_t *)(o + (pass ? 0x10 : 0x26));    /* 1139 / 1165 movswl */
                uint32_t eax = (uint32_t)(-(int32_t)((uint32_t)(int32_t)ang << 16)); /* shl 16 ; neg */
                uint32_t ti = eax >> 23;                                 /* 7fa0 shr $0x17 */
                int32_t ebp = trig[ti] >> 6, edi2 = trig[ti + 128] >> 6; /* 7faa/7fac/7fb2/7fb5 */
                int32_t bx2, dx2; uint8_t h0, h1; int32_t ecx, ebx2;
                bx2 = ebx - edi2; dx2 = edx + ebp;                       /* 7fbe/7fc0 */
                h0 = hm1c[fist_hm_index((uint32_t)dx2, (uint32_t)bx2, d1c)];
                bx2 += edi2*2; dx2 -= ebp*2;                             /* 7fcf..7fd5 */
                h1 = hm1c[fist_hm_index((uint32_t)dx2, (uint32_t)bx2, d1c)];
                ecx = (int32_t)((uint32_t)(int32_t)(int8_t)(uint8_t)(h0 - h1) << 23); /* 7fe1/7fe4/7fe7 */
                bx2 -= edi2; dx2 += ebp;                                 /* 7fea/7fec */
                bx2 -= ebp;  dx2 -= edi2;                                /* 7fef/7ff1 */
                h0 = hm1c[fist_hm_index((uint32_t)dx2, (uint32_t)bx2, d1c)];
                bx2 += ebp*2; dx2 += edi2*2;                             /* 8000..8006 */
                h1 = hm1c[fist_hm_index((uint32_t)dx2, (uint32_t)bx2, d1c)];
                ebx2 = (int32_t)((uint32_t)(int32_t)(int8_t)(uint8_t)(h0 - h1) << 23); /* 8012/8015/8018 */
                /* 801b `pop ebx` hands the FIRST pair's slope back in EBX and leaves the second in ECX:
                 * 114b/1177 store ECX (the second) at +0x34/+0x24, 1152/117e EBX (the first) at
                 * +0x32/+0x22.  They were crossed here, which flipped the hull's pitch and roll against
                 * the oracle's trajectory (oracle pitch 384 -> 1920 while the port fell to -768). */
                if (pass == 0) { *(uint16_t *)(o+0x34) = (uint16_t)((uint32_t)ebx2 >> 16);  /* 114b */
                                 *(uint16_t *)(o+0x32) = (uint16_t)((uint32_t)ecx >> 16); }  /* 1152 */
                else           { *(uint16_t *)(o+0x24) = (uint16_t)((uint32_t)ebx2 >> 16);  /* 1177 */
                                 *(uint16_t *)(o+0x22) = (uint16_t)((uint32_t)ecx >> 16); }  /* 117e */
            }
            {   int32_t ebx = (int32_t)((uint32_t)*(int32_t *)(o+4) << 13);   /* 1182/118b */
                int32_t edx = -(int32_t)((uint32_t)*(int32_t *)(o+8) << 13);  /* 1185/1188/118e */
                o[0x1d] = hm1c[fist_hm_index((uint32_t)edx, (uint32_t)ebx, d1c)];   /* 8480 ; 1195 mov %al,0x1d(%edi) */
            }
        }
        return 0;
    }
    if (op == 0x54 && g_ext_ready && g_fist_after_map) {
        *(uint16_t*)(dg + 0xea10) = 0; uint8_t *xb54=g_mem+FIST_EXT_BASE; uint8_t *hm54=(uint8_t*)(uintptr_t)(*(uint32_t*)(xb54+0x85bc));
        uint32_t tcb54 = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        uint16_t pp = g_fist_op54_proj ? (uint16_t)(g_fist_op54_proj + 4)
                                       : (uint16_t)*(uint32_t*)(g_mem + tcb54 + 0x3f2);
        int32_t X=*(int32_t*)(dg+pp),Y=*(int32_t*)(dg+(uint16_t)(pp+4));
        uint32_t idx=fist_hm_index((uint32_t)(-(int32_t)((uint32_t)Y<<13)), (uint32_t)X<<13, *(uint32_t*)(xb54+0x8490));
        g_ext_edx = (uint32_t)(-(int32_t)((uint32_t)Y<<13));   /* 11bb-11c1: the app's EDX on exit (op 0x60 reads it stale) */
        return hm54?(int)hm54[idx]:0; }
    if (op == 0x58 && g_ext_ready && g_fist_after_map) {
        /* board:0012 e339 clobber fix: a SERVICE op consumes its selector so e339's task-scheduler tail
         * (far-jmp [DGROUP:0x58] when aa10!=0 && TCB!=0) does NOT overwrite the LOS result with a trap-0.
         * Without this the op-0x58 return never reaches aa08 (targets never lock). */
        *(uint16_t*)(dg + 0xea10) = 0;
        uint8_t *xb = g_mem + FIST_EXT_BASE;
        uint32_t tcb_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        uint8_t *tcb = g_mem + tcb_lin;
        uint8_t *hm  = (uint8_t*)(uintptr_t)(*(uint32_t*)(xb+0x85bc));
        int32_t ox=*(int32_t*)(tcb+0xd2), oy=*(int32_t*)(tcb+0xd6), oz=*(int32_t*)(tcb+0xda);
        int32_t cx=*(int32_t*)(tcb+0xde), cy=*(int32_t*)(tcb+0xe2), cz=*(int32_t*)(tcb+0xe6);
        /* board:0012 unit-Z terrain-follow (camera-Z class stand-in for the absent overlay unit-altitude:
         * the flight-model per-unit ground-clamp is paged out, exactly like the camera-Z at op-0x24).
         * Use the LOS's OWN fixed-10 terrain index so endpoints sit on the sampled terrain + eye. */
        { static int standin = -1; if (standin < 0) standin = getenv("FIST_LOS_STANDIN") ? 1 : 0;
        if (hm && standin) {
            uint32_t oi = fist_hm_index((uint32_t)(-(int32_t)((uint32_t)oy<<13)), (uint32_t)ox<<13, *(uint32_t*)(xb+0x8490));
            uint32_t ci = fist_hm_index((uint32_t)(-(int32_t)((uint32_t)cy<<13)), (uint32_t)cx<<13, *(uint32_t*)(xb+0x8490));
            oz = ((int32_t)hm[oi]<<8) + 1792;
            cz = ((int32_t)hm[ci]<<8) + 1792;
        } }
        { extern long g_min_los,g_min_a296; long ad=(cx>ox?cx-ox:ox-cx)+(cy>oy?cy-oy:oy-cy); if(g_min_a296<16 && ad<g_min_los) g_min_los=ad; }
        { extern long g_op58_n; g_op58_n++; }
        int32_t dx=cx-ox, dy=cy-oy, dz=cz-oz;
        if (!hm || dx>=0x40000 || dy>=0x40000 || dx<=-0x40000 || dy<=-0x40000) { extern long g_op58_oor; g_op58_oor++; return 0; } /* out of range */
        dx<<=13; dy<<=13; dy=-dy; dz<<=16;                                    /* scale; Y flip */
        int32_t ecx=1;
        for(;;){ ecx<<=1; dx>>=1; dy>>=1; dz>>=1;                             /* normalise the step */
            if (dx>=0x3000000 || dy>=0x3000000 || dx<=-0x3000000 || dy<=-0x3000000) continue; break; }
        ecx--;
        int32_t sdx=dx, sdy=dy, sdz=dz;
        int32_t rx=ox<<13, ry=-(oy<<13), rz=oz<<16;                           /* march from object */
        for (int32_t i=0; i<ecx; i++) {
            rx+=sdx; ry+=sdy; rz+=sdz;
            uint32_t idx = fist_hm_index((uint32_t)ry, (uint32_t)rx, *(uint32_t*)(xb+0x8490));   /* 8102/8106 shld,DETAIL */
            uint32_t h = (uint32_t)hm[idx] << 24;
            if (h >= (uint32_t)rz) { extern long g_op58_occ; g_op58_occ++; return 0; } /* terrain occludes */
        }
        { extern long g_op58_vis; g_op58_vis++; }
        return -1;                                                           /* 0xffffffff = clear LOS */
    }
    /* FIST_DBG_OP2C: clean gdb breakpoint at the first op-0x2c gate (crash-bucket secondary-viewport paint) */
    if (op == 0x2c && g_fist_after_map && getenv("FIST_DBG_OP2C")) {
        static int once=0; if(!once){ once=1; extern void fist_dbg_op2c(void); fist_dbg_op2c(); }
    }
    if (op == 0x2c && g_fist_after_map && getenv("FIST_OBJTRAP")) {
        static int once=0; if(!once){ once=1; extern void objtrap_arm(unsigned);
            objtrap_arm((unsigned)strtoul(getenv("FIST_OBJTRAP"),0,0)); }
    }
    if (op == 0x2c && g_fist_after_map && getenv("FIST_WWTRAP")) {
        static int once=0; if(!once){ once=1; extern void wwtrap_arm(unsigned,unsigned);
            const char *e=getenv("FIST_WWTRAP"); char *c=0;
            unsigned o=(unsigned)strtoul(e,&c,0), l=(c&&*c==':')?(unsigned)strtoul(c+1,0,0):0x3000;
            wwtrap_arm(o,l); }
    }
    /* FIST_ROSTERTRACE (diag): log roster slot-4/5 object + its type at each op-gate -> pinpoint WHEN the
     * crash-bucket garbage type (0x0f00) appears (build-time vs a later corrupting step). Survives the
     * pre-op-0x2c-paint crash window. */
    if (getenv("FIST_ROSTERTRACE")) {
        static uint16_t p4=0xffff,p5=0xffff,t4=0xffff,t5=0xffff;
        uint16_t s4=*(uint16_t*)(dg+0x6d3c+8), s5=*(uint16_t*)(dg+0x6d3c+10);
        uint16_t ty4=s4?*(uint16_t*)(dg+s4):0, ty5=s5?*(uint16_t*)(dg+s5):0;
        static uint16_t pdir=0xffff, prec=0xffff;
        uint16_t dir=*(uint16_t*)(dg+0x4f0);
        uint16_t rec0 = *(uint16_t*)(g_mem + ((uint32_t)dir<<4) + 2);  /* first record's seg (valid~0x4b16) */
        if (s4!=p4||s5!=p5||ty4!=t4||ty5!=t5||dir!=pdir||rec0!=prec){
            fprintf(stderr,"[rtrace] op=0x%02x slot4=0x%04x(t=0x%04x) slot5=0x%04x(t=0x%04x) [0x4f0]dir=0x%04x rec0seg=0x%04x\n",op,s4,ty4,s5,ty5,dir,rec0);
            p4=s4;p5=s5;t4=ty4;t5=ty5;pdir=dir;prec=rec0;
        }
    }
    /* FIST_DGDUMP=<prefix> (diagnostic, default OFF): dump the low DGROUP window (0x1c000..0x1e200, 8.5KB)
     * to <prefix>.<tag>.bin at well-defined mission points so native vs wasm DGROUP can be byte-diffed to
     * classify the mission op-0x50 divergence (host-pointer leak vs tick-timing).  Tags: map1 = right after
     * the (single) op-0x18 map load; roster1 = first op-0x54 roster op; op50 = first op-0x50 post. */
    { static const char *pfx = (const char*)1; if (pfx == (const char*)1) pfx = getenv("FIST_DGDUMP");
      if (pfx) {
        static int did_map=0, did_roster=0, did_op50=0; const char *tag=0;
        if (op==0x18 && !did_map){ did_map=1; tag="map1"; }
        else if (op==0x54 && !did_roster){ did_roster=1; tag="roster1"; }
        else if (op==0x50 && !did_op50){ did_op50=1; tag="op50"; }
        if (tag){ char path[512]; snprintf(path,sizeof path,"%s.%s.bin",pfx,tag);
          FILE *f=fopen(path,"wb"); if(f){ fwrite(g_mem+0x1c000,1,0x10000,f); fclose(f);
            fprintf(stderr,"[dgdump] wrote %s (op=0x%x)\n",path,op); } }
      } }
    /* FIST_VEHWATCH (diagnostic, default OFF): poll the player-vehicle node veh+8 (Y) at every op-gate
     * entry; report the FIRST transition to a wild value (>0x01000000; correct AZER1 Y=0x116F1D). */
    if (getenv("FIST_VEHWATCH")) {
        static uint32_t vprev = 0; static int reported = 0;
        uint16_t nd = *(uint16_t *)(dg + 0x6d34);
        if (nd) {
            uint32_t v = *(uint32_t *)(g_mem + 0x1c000 + (uint16_t)(nd + 8));
            if (v != vprev) {
                if (v > 0x01000000 && !reported) {
                    fprintf(stderr, "[vehwatch] veh+8 WENT WILD: %u (0x%08x) at op=0x%x (was 0x%08x)\n",
                            v, v, op, vprev);
                    reported = 1;
                }
                vprev = v;
            }
        }
    }
    /* FIST_V55WATCH (diagnostic, default OFF): poll player(6d34)+0x55 (the cockpit azimuth-needle field,
     * patch 311) at every op-gate; log each value transition with the op.  Reveals whether +0x55 is set
     * once (parser only) or overwritten by a post-load init (and at which op). */
    if (getenv("FIST_V55WATCH")) {
        static int v55prev = 0x7fffffff; static uint16_t ndprev = 0xffff;
        uint16_t nd = *(uint16_t *)(dg + 0x6d34);
        if (nd) {
            int v = *(int16_t *)(dg + (uint16_t)(nd + 0x55));
            if (v != v55prev || nd != ndprev) {
                fprintf(stderr, "[v55watch] focus=0x%04x +55=%d (was %d) at op=0x%x\n", nd, v, v55prev==0x7fffffff?-999:v55prev, op);
                v55prev = v; ndprev = nd;
            }
        }
    }
    /* ------------------------------------------------------------------------------------------------
     * MISSION-PALETTE ARCHITECTURE (residual #1, RESOLVED this iteration -- see docs/stage1.md).
     * The ORIGINAL uses ONE physical VGA DAC carrying BOTH the cockpit/HUD colours AND the terrain
     * gradient.  The two producers occupy DISJOINT, complementary index ranges (proven from the DATA):
     *   - the ENGINE cockpit/HUD palette word[DGROUP:0x782] defines colours ONLY for idx 0..79 (51 coloured
     *     entries span 13..79) and is BLACK for 80..255 -- the engine leaves the terrain band undefined;
     *   - the terrain colormap (532.pal's C32.KLC -> tile3918) samples ONLY idx 80..255 (min index=80, 0%
     *     of texels below 80); 532.pal (loaded by the extender at op 0x18 into ext+0x5598) is fully coloured.
     * So the single coherent mission DAC = word[0x782][0..79] (cockpit) + 532.pal[80..255] (terrain), a
     * partial merge at the data-dictated boundary 80 (NOT the predecessor's full-palette EXTPAL swap, which
     * made the cockpit noise).  The FIST_MISSFB_MERGEPAL seam below builds exactly this DAC for measurement.
     *
     * NOT applied as a default behavioural change THIS iteration -- MEASURED, standalone it REGRESSES the
     * settled mission frame with NO offsetting benefit (terrain is not rendered), because two SEPARATE
     * residuals still block a coherent frame:
     *   (A) the op-0x24 RENDER-WINDSHIELD service (8deb->85d0->8120->9200 terrain into 0xA0000) can't be
     *       wired: the LIVE mission CAMERA is degenerate (TCB Y off ~40x, alt ~9x -- residual #2, a192->0459),
     *       so 9200 overpaints the cockpit -- a wired default render measures full AE 59997 / terrain 25361 /
     *       cockpit 32344 vs the no-render baseline 35212 / 22349 / 11963 (the terrain-region AE WORSENS,
     *       25361 vs 22349: degenerate-camera terrain matches the reference LESS than a black window does);
     *   (B) the port's COCKPIT PAINT writes stray >=80 indices (idx195/232/254, ~6.4-11.4k px/frame) that
     *       the REFERENCE cockpit does NOT have (ref rows 88-200 = 0% 532.pal terrain colours, 25% dark),
     *       so applying 532.pal to the global DAC colours those cockpit pixels brown -> +998 cockpit AE.
     * Landing the partial merge belongs WITH the camera+render fix (so terrain lands in idx-80..255 pixels
     * while the cockpit-paint >=80 bug is separately corrected).  The proven mechanism is banked in the
     * FIST_MISSFB_MERGEPAL measurement seam. */
    /* FIST_OP0C_BT (diagnostic, default OFF, behaviour-neutral): characterize the per-frame mission
     * render post op 0x0c.  The engine's 459a mission loop posts op 0x0c every outer iteration
     * (db99/patch193 -> 78f0 -> 85d0 + 93c0 flat-affine MAP-WARP of colormap 85b8).  This seam proves
     * the loop is a TIGHT RE-POST SPIN with NO simulation advance: it dumps the caller backtrace once,
     * then samples the engine frame-timer [DGROUP:0x452] + render cursor [0xea86] + camera heading
     * [0x7b1c] + the current-TCB camera (ea2e:ea2c +0x2c X / +0x34 alt).  OBSERVED: [0x452] stays 1 and
     * the camera stays frozen across MILLIONS of posts -- because 459a's spin never pumps the cooperative
     * INT-8 tick, so c452 never advances, so 459a's `2ce2 += (c452-2ce0)` elapsed-tick delta is 0, so the
     * per-tick sim step (c0ca/461b flight model) NEVER runs.  See docs/stage1.md. */
    if (op == 0x0c && getenv("FIST_OP0C_BT")) { static long n; n++;
        if (n == 1) {
        void *bt[24]; int k = backtrace(bt, 24);
        fprintf(stderr, "=== op0c backtrace (aa2c=%04x aa2e=%04x d548=%02x d549=%02x) ===\n",
            *(uint16_t*)(dg+0xea2c), *(uint16_t*)(dg+0xea2e), dg[0x1548], dg[0x1549]);
        backtrace_symbols_fd(bt, k, 2); }
        if ((n % 100000) == 1) fprintf(stderr, "[op0c] n=%ld frame[0x452]=%u cursor[0xea86]=%04x 7b1c(head)=%u tcb+2c(X)=%d tcb+34(alt)=%d\n",
            n, *(uint16_t*)(g_mem+0x1c452), *(uint16_t*)(dg+0xea86),
            *(uint16_t*)(g_mem+0x23b1c),
            *(int32_t*)(g_mem + (((uint32_t)*(uint16_t*)(dg+0xea2e)<<4)+*(uint16_t*)(dg+0xea2c)) + 0x2c),
            *(int32_t*)(g_mem + (((uint32_t)*(uint16_t*)(dg+0xea2e)<<4)+*(uint16_t*)(dg+0xea2c)) + 0x34)); }
    /* FIST_R3D2_V18 -- DETERMINISTIC terrain-pipeline validation at MAP-LOAD time (avoids the flaky,
     * slow op-0x0c gate).  After 2 op-0x18 map-loads have completed (both KLC planes populated), on the
     * next non-0x18 op run the FULL voxel pipeline ONCE with the oracle camera + oracle ray tables:
     * 8deb viewport -> 85d0 camera -> 6980 tile build -> 8120 projection -> 9200 render.  Then compare
     * the built tile against the oracle tile and dump the framebuffer.  Proves builder+renderer fidelity
     * independent of the engine's per-frame dispatch + the flaky object walk. */
    if (getenv("FIST_R3D2_V18") && g_ext_ready) {
        static int nseen18 = 0, done = 0;
        if (op == 0x18) nseen18++;
        else if (nseen18 >= 1 && !done && op == 0x54) {   /* first roster op after the (single) map-load */
            done = 1;
            uint8_t *xb = g_mem + FIST_EXT_BASE;
            uint32_t tcb_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
            uint8_t *tcb = g_mem + tcb_lin;
            uint32_t save_c93 = *(uint32_t*)(xb+0xc93);
            *(uint32_t*)(xb+0xc93) = (uint32_t)(uintptr_t)tcb;
            /* inject oracle camera */
            *(int32_t*)(tcb+0x2c)=609696; *(int32_t*)(tcb+0x30)=1112229; *(int32_t*)(tcb+0x34)=29184;
            *(uint16_t*)(tcb+0x38)=19745; *(uint16_t*)(tcb+0x3a)=0; *(uint16_t*)(tcb+0x3c)=128;
            *(uint16_t*)(tcb+0x3e)=256; *(uint8_t*)(tcb+0xcf)=0; *(uint8_t*)(tcb+0xcd)=1;
            /* inject oracle ray tables 3a24/3e24 (detail base curve; port never builds them) */
            { const char *r3=getenv("FIST_R3D2_RAY3A24"), *r7=getenv("FIST_R3D2_RAY3E24");
              if (r3){FILE*f=fopen(r3,"rb"); if(f){fread(xb+0x3a24,1,256*4,f);fclose(f);}}
              if (r7){FILE*f=fopen(r7,"rb"); if(f){fread(xb+0x3e24,1,256*4,f);fclose(f);}}
              *(uint32_t*)(xb+0x90c4)=0; }
            m_ext_FUN_0000_8deb(); m_ext_FUN_0000_85d0();
            /* force oracle viewport (8deb yields 0 from an empty TCB rect) */
            *(uint32_t*)(xb+0x90f8)=81; *(uint32_t*)(xb+0x90f0)=288; *(uint32_t*)(xb+0x90ac)=0x140-288;
            *(uint32_t*)(xb+0x90a8)=(uint32_t)(uintptr_t)(g_mem+0xA0000+0x650);
            extern void m_ext_FUN_0000_6980(void);
            m_ext_FUN_0000_6980();
            uint32_t cmb=*(uint32_t*)(xb+0x3918);
            { uint8_t*t=(uint8_t*)(uintptr_t)cmb; long nz=0; int h[256]={0},nd=0;
              for(int i=0;i<65536;i++){if(t[i])nz++; if(!h[t[i]]){h[t[i]]=1;nd++;}}
              fprintf(stderr,"[v18] TILE-BUILD nz=%ld/65536 distinct=%d\n",nz,nd);
              const char*tr=getenv("FIST_R3D2_TILEREF");
              if(tr){FILE*rf=fopen(tr,"rb"); if(rf){static uint8_t ref[0x10000]; size_t n=fread(ref,1,0x10000,rf);fclose(rf);
                if(n==0x10000){long same=0; int hr[256]={0},ndr=0; for(int i=0;i<0x10000;i++){if(t[i]==ref[i])same++; if(!hr[ref[i]]){hr[ref[i]]=1;ndr++;}}
                  fprintf(stderr,"[v18] TILE-vs-ORACLE identical=%ld/65536 (%.1f%%) ref-distinct=%d\n",same,100.0*same/65536,ndr);}}} }
            m_ext_FUN_0000_8120();
            { int32_t c0=*(int32_t*)(xb+0x90c0),v04=*(int32_t*)(xb+0x9104),v08=*(int32_t*)(xb+0x9108);
              int32_t esi=(int32_t)(((int64_t)c0*v04)>>32), ebp=(int32_t)(((int64_t)c0*v08)>>32);
              m_ext_FUN_0000_9200(ebp,esi); }
            { uint8_t*fb=g_mem+0xA0000; int vh[256]={0}; int vnd=0; long vnz=0;
              for(long r=5;r<86;r++)for(long c=0;c<320;c++){uint8_t p=fb[r*320+c]; if(p)vnz++; if(!vh[p])vnd++; vh[p]++;}
              fprintf(stderr,"[v18] RENDER viewport rows5-86 nonzero=%ld distinct=%d\n",vnz,vnd); }
            { extern void out(int,int); uint8_t*mp=xb+0x5598; out(0x3c8,0); for(int pi=0;pi<768;pi++) out(0x3c9,mp[pi]); }
            const char*dp=getenv("FIST_R3D2_DUMP"); if(dp) fist_dump_framebuffer(dp);
            *(uint32_t*)(xb+0xc93)=save_c93;
        }
    }
    /* FIST_OPHIST -- temporary op histogram + 3918 tile-fill probe (read-only diagnostic). */
    if (getenv("FIST_OPSEQ")) { static int n; if (n < 4000) { n++;
        uint32_t t = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        fprintf(stderr, "[opseq] %02x  +5a='%.16s' +6a='%.16s' +7a='%.16s' +3f2=%08x\n", op,
                (char*)(g_mem+t+0x5a), (char*)(g_mem+t+0x6a), (char*)(g_mem+t+0x7a), *(uint32_t*)(g_mem+t+0x3f2)); } }
    if (getenv("FIST_OPHIST")) {
        static long ophist[0x100]; static long total;
        static int firstseen[0x100];
        ophist[op & 0xff]++; total++;
        if (!firstseen[op&0xff]) { firstseen[op&0xff]=1; fprintf(stderr,"[ophist] FIRST op 0x%02x (total=%ld)\n", op, total); }
        if ((total % 2000) == 0 || (op==0x40) || (op==0x24) || (op==0x0c)) {
            uint8_t *xb = g_mem + 0x100000; /* FIST_EXT_BASE */
            uint32_t cm = *(uint32_t*)(xb+0x3918);
            long nz=0; int seen[256]={0}, dist=0;
            if (cm) { uint8_t *t=(uint8_t*)(uintptr_t)cm; for(int i=0;i<65536;i++){ if(t[i]){nz++;} if(!seen[t[i]]){seen[t[i]]=1;dist++;} } }
            fprintf(stderr,"[ophist] total=%ld op40(ad1e)=%ld op24(9200)=%ld op08(tile)=%ld op0c=%ld op18=%ld op54=%ld op60=%ld op44=%ld | tile3918 nz=%ld dist=%d\n",
                total, ophist[0x40], ophist[0x24], ophist[0x08], ophist[0x0c], ophist[0x18], ophist[0x54], ophist[0x60], ophist[0x44], nz, dist);
        }
    }
    /* ------------------------------------------------------------------------------------------------
     * FIST_MISSFB -- SETTLED mission-frame FB capture (default OFF, behaviour-neutral for the 19 flows).
     * The in-mission 459a loop posts op 0x24 (the windshield-render post) exactly once, then spins on the
     * op-0x4c present pump WITHOUT re-entering fist_timer_pump (so the FIST_RUNMS/FIST_DUMPTICK wall-clock
     * watchdogs never fire in-mission -- prior FB-dump seams could not capture a settled mission frame).
     * The deterministic "settled" mission frame is therefore the Nth op-0x24 post (FIST_MISSFB_N, default
     * 1): a pure post-count -> DETERMINISTIC and native<->wasm-safe.  At that post:
     *   - FIST_MISSFB_PROBE: log the LIVE mission TCB camera (ea2e:ea2c +0x2c X /+0x30 Y /+0x34 alt /
     *     +0x38 heading /..) + the terrain-pipeline state (tile3918 fill, ray tables 3a24/3e24) so the
     *     camera/ray residual can be pinned vs the oracle (docs/oracle_terrain_writer.md: X=609696
     *     Y=1112229 alt=29184 head=19745 roll=128 foc=256 detail=0).
     *   - FIST_MISSFB_RENDER: run the real extender windshield render from the LIVE TCB (NO oracle
     *     injection): 85d0 camera -> 8120 projection -> 9200 texel walk into the cockpit windshield window
     *     (81 cols x 288 col-height, dest fb+0x650 -- the camera-INDEPENDENT cockpit windshield geometry,
     *     docs/oracle_terrain_writer.md).  Off by default the FB is dumped AS-IS (whatever the engine's
     *     cockpit/HUD paint chain drew).
     *   - upload the mission DAC palette (ext+0x5598, 6-bit) so the dumped colours are the mission palette;
     *   - dump 0xA0000 (FIST_MISSFB=path) via the shared 6->8 VGA expander, then _exit(0).                */
    /* ------------------------------------------------------------------------------------------------
     * DEFAULT-PATH WINDSHIELD VOXEL RENDER (extender role).  The real Doug-Huffman extender's op-0x24
     * PM service renders the perspective voxel terrain into 0xA0000 (oracle-proven the dominant AZER1
     * fb writer; docs/oracle_terrain_writer.md + voxel-render-is-engine-side pin).  The engine posts
     * op 0x24 from df0e once the cockpit view is active; here we emulate the extender by driving the
     * decompiled render chain 8deb(viewport) -> 85d0(camera from TCB 0x2c/0x30 XY, 0x34 alt) ->
     * 8120(projection) -> 9200(per-column texel walk).  This runs on the CANONICAL default build (no
     * env gate) whenever a mission map is loaded, so gameplay draws terrain.
     *
     * CAMERA-Z TERRAIN-FOLLOW (asm 0x8650 + oracle spawn frame).  The absent 32-bit-PM flight model
     * would write TCB+0x34 (the render altitude 85d0 reads) each frame as the terrain surface height
     * under the camera XY plus the vehicle eye height.  No [c93+0x34] write exists in fist_image.bin
     * (verified -- it is paged out), so the value is oracle-anchored: at spawn h=heightmap[camXY]=43,
     * oracle alt=12800 = (43+7)<<8, i.e. alt = (h<<8) + FIST_EYE_HT where FIST_EYE_HT = 7<<8 = 1792
     * (the M1A2 eye height, 7 world units, in the extender's <<8 world scale).  Heightmap index math =
     * 0x8650's own: tile = (camXY<<13)>>(32-detail), idx = ((tileY&mask)<<detail)+(tileX&mask). */
    /* board:0002 op 0x50 -- the OBJECT PROJECTION (service 1266 = `movzx esi,si ; add esi,[0xca1] ; call
     * ba7d`, fist_ext.c FUN_0000_ba7d): every render node's world position to a screen position and a
     * depth row, once per node per frame from the depth sorter 2471 (and 3eb3 for 2908/290e's record).
     * The shim used to return 0, so every node sat at (0,0) at depth 0 and the models never drew in place. */
    if (op == 0x50 && g_ext_ready && g_fist_after_map) {
        extern unsigned m_ext_FUN_0000_ba7d(const uint8_t *, uint16_t);
        *(uint16_t *)(dg + 0xea10) = 0;
        { int r = (int)m_ext_FUN_0000_ba7d(dg + g_fist_op50_si, g_fist_op50_dx);
          if (getenv("FIST_OP50TRACE")) { static long n; if (n++ < 400) fprintf(stderr, "[op50] t=%u si=%04x X=%d Y=%d h=%u dx=%u -> y=%d x=%d depth=%08x rows=%u/%u\n",
              *(uint16_t*)(dg+0x452), g_fist_op50_si, *(int32_t*)(dg+g_fist_op50_si), *(int32_t*)(dg+g_fist_op50_si+4), dg[g_fist_op50_si+9], g_fist_op50_dx,
              (int16_t)r, (int16_t)g_fist_op50_cx, g_fist_op50_edx, g_fist_op50_esi, g_fist_op50_edi); }
          return r; }
    }
    /* board:0002 op 0x60 -- the MAP STAMPS (service 10f1 = `call 8650`, fist_ext.c FUN_0000_8650): the
     * objective and crater images e043 posts for a mission (a position and two 16-byte image names in
     * the TCB) pressed into the height- and colormaps.  The shim used to return 0 for the 16-odd posts
     * of every mission, so the terrain carried no objectives. */
    if (op == 0x60 && g_ext_ready && g_fist_after_map) {
        uint8_t  *xb  = g_mem + FIST_EXT_BASE;
        uint32_t tcb_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        uint32_t save_c93 = *(uint32_t*)(xb+0xc93);
        extern unsigned m_ext_FUN_0000_8650(void);
        unsigned r;
        *(uint32_t*)(xb+0xc93) = (uint32_t)(uintptr_t)(g_mem + tcb_lin);
        *(uint16_t *)(dg + 0xea10) = 0;                    /* served: e339 restarts through [0x58] otherwise */
        g_fist_ext_int = 1;                                /* 643c opens the two .KLC through the flat FILEMGR */
        r = m_ext_FUN_0000_8650();
        g_fist_ext_int = 0;
        *(uint32_t*)(xb+0xc93) = save_c93;
        if (getenv("FIST_OP60TRACE"))
            fprintf(stderr, "[op60] stamp '%.16s' / '%.16s' at (%d,%d) -> %d\n", (char*)(g_mem + tcb_lin + 0x5a),
                    (char*)(g_mem + tcb_lin + 0x6a), *(int32_t*)(g_mem + tcb_lin + 0xd2), *(int32_t*)(g_mem + tcb_lin + 0xd6), (int)r);
        return (int)r;
    }
    /* board:0002 op 0x08 -- the TERRAIN TILE BUILD, posted once per frame right before op 0x24.  The
     * service table (fist_image.bin 0xcb3) maps it to 0x10e0 = `call 8df0 ; call 3931`: the viewport
     * from the TCB, then 3931 = the camera (85d0), the sky/tile resampler through [0x3958] (0x6877) and
     * the raycaster 6980 into the 256-square tile that op 0x24's 9200 perspective-maps.  Read off the
     * original's block trace (scratch/oracle/blktrace); the shim used to return 0 for it, so every
     * windshield frame sampled a tile nobody had built. */
    if (op == 0x08 && g_ext_ready && g_fist_after_map) {
        uint8_t  *xb  = g_mem + FIST_EXT_BASE;
        uint32_t tcb_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        *(uint32_t*)(xb+0xc93) = (uint32_t)(uintptr_t)(g_mem + tcb_lin);
        *(uint16_t*)(dg + 0xea10) = 0;
        m_ext_FUN_0000_8df0();
        m_ext_FUN_0000_3931();
        return 0;
    }
    /* board:0002 op 0x24 -- the WINDSHIELD RENDER.  The service table maps it to 0x82c0 = `call 8120 ;
     * call 9200 ; call 82d0`: 8120 projects the camera 85d0 set at op 0x08 into the per-row texel steps
     * and leaves the per-pixel steps in ESI/EBP (asm 0x8239-0x8251: 90c0*9104 >> 32 and 90c0*9108 >> 32,
     * which the decompile computes but does not return), 9200 walks the tile 3918 into the windshield
     * window, 82d0 projects the target marker TCB+0xd2 for the HUD.  One frame period of machine time
     * per render (board:0026).  What used to be here -- 8deb/85d0 again, oracle-anchored camera seeds
     * (altitude, focal, pitch, roll) and the FIST_TERRAIN / FIST_TILEFILL / FIST_ISO / FIST_INJECT_*
     * scaffolds -- modelled a chain the original never runs; the tile is op 0x08's, the camera the
     * engine's (dd15 writes the TCB every frame). */
    /* board:0002 op 0x5c -- the MAP INSET (0x7990): the destination surface, width and height from the
     * TCB (+2:+4 seg:off, +0x1e, +0x22) into 90ec/90f0/90f4/90f8, then the camera (85d0), the top-down
     * colormap walk through the shade LUT (8e80) and the platoon/objective markers (79d7).  Posted by
     * dc12 on the view-0x20 frames; the shim never served it -- the inset showed the buffer's residue. */
    if (op == 0x5c && g_ext_ready && g_fist_after_map) {
        uint8_t  *xb  = g_mem + FIST_EXT_BASE;
        uint32_t tcb_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        uint8_t  *tcb = g_mem + tcb_lin;
        uint32_t save_c93 = *(uint32_t*)(xb+0xc93);
        extern void m_ext_FUN_0000_85d0(void), m_ext_FUN_0000_8e80(void), m_ext_FUN_0000_79d7(unsigned, unsigned short);
        *(uint32_t*)(xb+0xc93) = (uint32_t)(uintptr_t)tcb;
        *(uint16_t*)(dg + 0xea10) = 0;
        *(uint32_t*)(xb+0x90ec) = ((uint32_t)*(uint16_t*)(tcb+2) << 4) + *(uint16_t*)(tcb+4) + *(uint32_t*)(xb+0x807);   /* 7996-79a9 */
        *(uint32_t*)(xb+0x90f0) = *(uint16_t*)(tcb+0x1e);                                                         /* 79ae */
        *(uint32_t*)(xb+0x90f4) = *(uint16_t*)(tcb+0x1e) >> 1;                                                    /* 79b7 */
        *(uint32_t*)(xb+0x90f8) = *(uint16_t*)(tcb+0x22);                                                         /* 79be */
        m_ext_FUN_0000_85d0();
        m_ext_FUN_0000_8e80();
        m_ext_FUN_0000_79d7(0, 0);
        *(uint32_t*)(xb+0xc93) = save_c93;
        return 0;
    }
    /* board:0002 op 0x2c -- the SECONDARY viewport (the map inset; view 0x20 on the alternate frames):
     * the service table maps it to 0x8460 = `call 8120 ; call 9130 ; call 82d0`, 9200's twin with a
     * 50% blend into the destination (fist_ext.c FUN_0000_9129).  The shim never served it. */
    if (op == 0x2c && g_ext_ready && g_fist_after_map) {
        uint8_t  *xb  = g_mem + FIST_EXT_BASE;
        uint32_t tcb_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        uint32_t save_c93 = *(uint32_t*)(xb+0xc93);
        *(uint32_t*)(xb+0xc93) = (uint32_t)(uintptr_t)(g_mem + tcb_lin);
        m_ext_FUN_0000_8120();
        { int32_t c0 = *(int32_t*)(xb+0x90c0), v04 = *(int32_t*)(xb+0x9104), v08 = *(int32_t*)(xb+0x9108);
          int32_t esi = (int32_t)(((int64_t)c0*v04)>>32), ebp = (int32_t)(((int64_t)c0*v08)>>32);
          extern void m_ext_FUN_0000_9129(int, int);
          m_ext_FUN_0000_9129(ebp, esi); }
        m_ext_FUN_0000_82d0();
        *(uint32_t*)(xb+0xc93) = save_c93;
        /* falls through to the op-0x2c capture diagnostics below (FIST_MISSFB2C) */
    }
    if (op == 0x24 && g_ext_ready && g_fist_after_map) {
        uint8_t  *xb  = g_mem + FIST_EXT_BASE;
        uint32_t tcb_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        uint32_t save_c93 = *(uint32_t*)(xb+0xc93);
        *(uint32_t*)(xb+0xc93) = (uint32_t)(uintptr_t)(g_mem + tcb_lin);
        m_ext_FUN_0000_8120();
        { int32_t c0 = *(int32_t*)(xb+0x90c0), v04 = *(int32_t*)(xb+0x9104), v08 = *(int32_t*)(xb+0x9108);
          int32_t esi = (int32_t)(((int64_t)c0*v04)>>32), ebp = (int32_t)(((int64_t)c0*v08)>>32);
          m_ext_FUN_0000_9200(ebp, esi); }
        m_ext_FUN_0000_82d0();
        *(uint32_t*)(xb+0xc93) = save_c93;
        /* The frame's machine time (board:0026).  The mission loop 459a does not wait for the retrace:
         * it renders, counts the [0x452] ticks that passed, steps the sim once per tick, renders again --
         * on the oracle machine that is ~1.1 frames per tick (the 9200 pass captures move the camera
         * one sim step almost every frame), i.e. one windshield render per vblank.  A port with an
         * infinitely fast CPU rendered 34 frames per vblank instead, all of them unseen.  So one render
         * costs one frame period here: the clock steps a vblank (the INT-8 in it lands mid-render, as
         * the real one preempted the renderer), and the loop's tick arithmetic sees what the oracle's did. */
        { extern void fist_clock_advance(unsigned); extern unsigned fist_clock_frame_counts(void);
          /* FIST_FRAME_COUNTS=<n> (diagnostic): another per-frame cost, to show the sim's indifference --
           * AZER1 resolves at the same tick with 8000, 17025 and 46500 (board:0026). */
          static long fc = -1; if (fc < 0) { const char *e = getenv("FIST_FRAME_COUNTS"); fc = e ? atol(e) : 0; }
          fist_clock_advance(fc > 0 ? (unsigned)fc : fist_clock_frame_counts()); }
        /* The terrain band of the DAC (indices [TCB+0x54]..255) is the engine's own business: dad2 points
         * [0x78e]:[0x790] at TCB+0xea -- the luminance-sorted 532.pal 9f70 leaves there -- and the MGA
         * method [0x56c] (0410) copies it into the palette buffer [0x782].  The merge that used to sit
         * here re-copied ext+0x5598 over that band on every frame, and 0x5598 holds the SKY's PCX
         * palette by then (643c reads it there after the sky image), which painted the terrain
         * blue-grey. */
    }
    if (op == 0x24 && getenv("FIST_OP24TRACE") && g_ext_ready) {
        static int nt = 0;
        if (nt < 14) {
            uint32_t tl = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
            uint8_t *t = g_mem + tl;
            fprintf(stderr,"[op24t #%d] glob 9614/18/1c=%d/%d/%d  TCB+2c/30/34=%d/%d/%d  +d2/d6/da=%d/%d/%d\n",
                ++nt, *(int32_t*)(dg+0x9614),*(int32_t*)(dg+0x9618),*(int32_t*)(dg+0x961c),
                *(int32_t*)(t+0x2c),*(int32_t*)(t+0x30),*(int32_t*)(t+0x34),
                *(int32_t*)(t+0xd2),*(int32_t*)(t+0xd6),*(int32_t*)(t+0xda));
        }
    }
    /* FIST_RETDUMP (diagnostic, default OFF): at op-0x24 post N (settled AZER1 frame, same anchor as
     * FIST_MISSFB) dump the reticle geometry descriptor at word[0x156a]=0x156c and its SOURCE struct at
     * word[0x1552], then _exit(0).  This is the bit-verify seam for the reticle-geometry-populator work.
     * FINDING (this iteration): the descriptor WRITER FUN_0000_2322 (patch 186) is faithful -- it copies
     * width/height into desc[+4]/[+a]/[+c] FROM the source struct d552; those land 0 only because the
     * SOURCE (the child view record 0x8e91, one of 0x8e91/0x8ead/0x8ec9 under container 0x8e84) is never
     * populated.  Its rect is written by MGAVIDEO mga 0x2004 = DGROUP:0x68c (asm 0x21a8-0x21c9: the sprite
     * bounding-box -> child[+4/+6/+8/+a/+14]), which is UNINSTALLED in the port (vec [0x68c]=mga:0 no-op,
     * device-alloc [0xcc]=0, per-child sprite word0 not assigned) -- the documented cockpit-instrument
     * DEVICE subsystem debt (patches 301/302).  The 288x81 values are the loaded instrument-sheet sprite
     * bounding box (data-derived), not asm immediates.  See docs/stage1.md. */
    if (op == 0x24 && getenv("FIST_RETDUMP") && g_fist_after_map) {
        static int nr = 0;
        long wr = getenv("FIST_RETDUMP_N") ? atol(getenv("FIST_RETDUMP_N")) : 1;
        if (++nr >= wr) {
            uint16_t h = *(uint16_t*)(dg+0x156a);
            uint16_t s = *(uint16_t*)(dg+0x1552);
            uint8_t *desc = dg + h;
            uint8_t *src  = dg + s;
            fprintf(stderr,"[retdump] op24 post #%ld  handle word[0x156a]=%04x  desc[0..f]:", wr, h);
            for(int i=0;i<16;i++) fprintf(stderr," %02x", desc[i]);
            fprintf(stderr,"\n[retdump] desc fields word0=%04x [+2]=%04x [+4]=%04x [+6]=%04x [+8]=%04x [+a]=%04x [+c]=%04x [+e]=%04x\n",
                *(uint16_t*)(desc+0),*(uint16_t*)(desc+2),*(uint16_t*)(desc+4),*(uint16_t*)(desc+6),
                *(uint16_t*)(desc+8),*(uint16_t*)(desc+0xa),*(uint16_t*)(desc+0xc),*(uint16_t*)(desc+0xe));
            fprintf(stderr,"[retdump] SRC word[0x1552]=%04x  src[0..1b]:", s);
            for(int i=0;i<0x1c;i++) fprintf(stderr," %02x", src[i]);
            fprintf(stderr,"\n[retdump] src fields [+4]=%04x [+6]=%04x [+8]=%04x [+a]=%04x [+14]=%04x [+16]=%04x\n",
                *(uint16_t*)(src+4),*(uint16_t*)(src+6),*(uint16_t*)(src+8),*(uint16_t*)(src+0xa),
                *(uint16_t*)(src+0x14),*(uint16_t*)(src+0x16));
            fprintf(stderr,"[retdump] vecs [0x68c]=%08x [0xcc]=%08x [0x560]=%08x [0x694]=%08x  child8e91.w0=%04x cont8e84.w0=%04x  d0c(8d0c)=%02x d90(8e90)=%02x\n",
                *(uint32_t*)(dg+0x68c),*(uint32_t*)(dg+0xcc),*(uint32_t*)(dg+0x560),*(uint32_t*)(dg+0x694),
                *(uint16_t*)(dg+0x8e91),*(uint16_t*)(dg+0x8e84),dg[0x8d0c],dg[0x8e90]);
            { uint16_t cands[] = {0x7aa4,0x8d10,0x8d2c,0x8d60,0x8e84,0x8e91,0x8ead,0x8ec9};
              for (unsigned k=0;k<sizeof(cands)/sizeof(cands[0]);k++){
                uint8_t *c = dg + cands[k];
                fprintf(stderr,"[retdump] cand %04x:", cands[k]);
                for(int i=0;i<0x1c;i++) fprintf(stderr," %02x", c[i]);
                fprintf(stderr,"  w=(a-6)=%d h=((8-4)+16)=%d\n",
                    (int16_t)(*(uint16_t*)(c+0xa)-*(uint16_t*)(c+6)),
                    (int16_t)((*(uint16_t*)(c+8)-*(uint16_t*)(c+4))+*(uint16_t*)(c+0x16)));
              } }
            _exit(0);
        }
    }
    /* FIST_MISSFB2C -- capture 0xA0000 after the Nth op-0x2c post.  The FSG-battle / crash-bucket missions
     * (AZER3 etc.) render the windshield through the op-0x2c SECONDARY-viewport path, which the ENGINE
     * itself paints into 0xA0000 (no extender op-0x24 emulation) -- so just snapshot the live framebuffer
     * + current DAC and _exit.  Read-only, default OFF; never reached by the front-end verify flows. */
    if (op == 0x2c && getenv("FIST_MISSFB2C") && getenv("FIST_MISSFB") && g_ext_ready && g_fist_after_map) {
        static int n2c = 0;
        long want2c = getenv("FIST_MISSFB_N") ? atol(getenv("FIST_MISSFB_N")) : 120;
        if (++n2c == want2c) { fist_dump_framebuffer(getenv("FIST_MISSFB")); _exit(0); }
    }
    if (op == 0x24 && getenv("FIST_MISSFB") && g_ext_ready) {
        static int nseen = 0;
        long want = getenv("FIST_MISSFB_N") ? atol(getenv("FIST_MISSFB_N")) : 1;
        if (++nseen == want) {
            uint8_t *xb = g_mem + FIST_EXT_BASE;
            uint32_t tcb_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
            uint8_t *tcb = g_mem + tcb_lin;
            uint32_t save_c93 = *(uint32_t*)(xb+0xc93);
            *(uint32_t*)(xb+0xc93) = (uint32_t)(uintptr_t)tcb;
            if (getenv("FIST_MISSFB_FBIDX")) {
                /* histogram of framebuffer INDICES in the cockpit region (rows 88-200): how many pixels
                 * use index>=80 (the terrain band the palette merge fills)?  Read-only diagnostic. */
                uint8_t *fb = g_mem + 0xA0000; long ge80=0, tot=0; int h[256]={0};
                for (int r=88;r<200;r++) for (int c=0;c<320;c++){ uint8_t p=fb[r*320+c]; tot++; if(p>=80){ge80++; h[p]++;} }
                fprintf(stderr,"[fbidx] cockpit rows88-200: pixels>=80 = %ld/%ld  top idx>=80:",ge80,tot);
                for(int k=0;k<3;k++){int bi=-1,bv=0; for(int i=80;i<256;i++) if(h[i]>bv){bv=h[i];bi=i;} if(bi>=0){fprintf(stderr," idx%d(%dpx)",bi,bv); h[bi]=0;}}
                fprintf(stderr,"\n");
                if (getenv("FIST_MISSFB_FBDUMP")) {
                    FILE*ff=fopen("/tmp/fb_idx.bin","wb"); if(ff){fwrite(fb,1,64000,ff);fclose(ff);}
                    uint8_t *ep2 = g_mem + ((uint32_t)(*(uint16_t*)(dg+0x782))<<4);
                    FILE*fp=fopen("/tmp/pal_merged.bin","wb"); if(fp){fwrite(ep2,1,768,fp);fclose(fp);}
                    fprintf(stderr,"[fbidx] dumped /tmp/fb_idx.bin + /tmp/pal_merged.bin (ge80=%ld)\n",ge80);
                }
            }
            if (getenv("FIST_MISSFB_PROBE")) {
                fprintf(stderr,"[missfb] op24 post #%d  TCB @0x%05x cam[2c/30/34]=%d/%d/%d ang[38/3a/3c/3e]=%u/%d/%d/%u detail[cd/cf]=%u/%u\n",
                    nseen, tcb_lin, *(int32_t*)(tcb+0x2c),*(int32_t*)(tcb+0x30),*(int32_t*)(tcb+0x34),
                    *(uint16_t*)(tcb+0x38),*(int16_t*)(tcb+0x3a),*(int16_t*)(tcb+0x3c),*(uint16_t*)(tcb+0x3e),
                    tcb[0xcd],tcb[0xcf]);
                if (getenv("FIST_ISO_812DUMP")) { extern void m_ext_FUN_0000_944b(void);
                    fprintf(stderr,"[812] 9104=%08x 9108=%08x 90e4=%08x 90b4=%08x  &944b=%08x g_mem+ext+0x9450=%08x tbl9450[0..3]=%08x%08x%08x%08x tbl9650[0..3]=%08x%08x%08x%08x\n",
                        *(uint32_t*)(xb+0x9104),*(uint32_t*)(xb+0x9108),*(uint32_t*)(xb+0x90e4),*(uint32_t*)(xb+0x90b4),
                        (uint32_t)(uintptr_t)&m_ext_FUN_0000_944b,(uint32_t)(uintptr_t)(xb+0x9450),
                        *(uint32_t*)(xb+0x9450),*(uint32_t*)(xb+0x9454),*(uint32_t*)(xb+0x9458),*(uint32_t*)(xb+0x945c),
                        *(uint32_t*)(xb+0x9650),*(uint32_t*)(xb+0x9654),*(uint32_t*)(xb+0x9658),*(uint32_t*)(xb+0x965c)); }
                uint32_t cm=*(uint32_t*)(xb+0x3918); long nz=0; int h[256]={0},nd=0;
                if(cm){uint8_t*t=(uint8_t*)(uintptr_t)cm; for(int i=0;i<65536;i++){if(t[i])nz++; if(!h[t[i]]){h[t[i]]=1;nd++;}}}
                fprintf(stderr,"[missfb] tile3918 nz=%ld/65536 distinct=%d  ray3a24[0..3]=%d/%d/%d/%d ray3e24[0]=%d 90c0=%08x 90c4=%08x\n",
                    nz,nd,*(int32_t*)(xb+0x3a24),*(int32_t*)(xb+0x3a28),*(int32_t*)(xb+0x3a2c),*(int32_t*)(xb+0x3a30),
                    *(int32_t*)(xb+0x3e24),*(uint32_t*)(xb+0x90c0),*(uint32_t*)(xb+0x90c4));
                /* FIST_RAWMTX=<path>: dump the RAW bc9c blend matrix at its true base
                 * (bc90 & 0xffff0000), NOT the +0x4200 tile window at [0x3918].  bc9c writes
                 * M[ch][cl] at base|(ch<<8)|cl, so file offset ch*256+cl == matrix entry --
                 * a clean 256x256 whose diagonal M[ch][ch] encodes the palette directly.
                 * (The old port_bc9c_matrix.bin was read from [0x3918]=base+0x4200, i.e. the
                 * bdc4-upsampled tile overrunning the 64K buffer by 0x4200 -> spurious zero tail.) */
                { const char *rm=getenv("FIST_RAWMTX");
                  if(rm){ uint32_t bc90=*(uint32_t*)(xb+0xbc90);
                    uint8_t*base=(bc90&&bc90<0x100000)?(xb+(bc90&0xffff0000)):(uint8_t*)(uintptr_t)(bc90&0xffff0000);
                    FILE*f=fopen(rm,"wb");
                    if(f){ fwrite(base,1,65536,f); fclose(f);
                      int dd[11]={80,90,100,110,120,140,160,180,200,220,255};
                      fprintf(stderr,"[rawmtx] bc90=%08x base=%p wrote 65536 -> %s  diag:",bc90,(void*)base,rm);
                      for(int i=0;i<11;i++) fprintf(stderr," M[%d]=%d",dd[i],base[dd[i]*256+dd[i]]);
                      fprintf(stderr,"\n"); } } }
                /* FIST_MISSFB_VP: LIVE voxel viewport geometry (set by 8deb from TCB rect) vs the
                 * oracle-forced 90f8=81/90f0=288/90ac=-208.  Also the TCB viewport rect 0x16/18/1a/1c
                 * and the horizon-table 9114 pointer + first bytes. */
                {
                  uint16_t vl=*(uint16_t*)(tcb+0x16),vt=*(uint16_t*)(tcb+0x18),vr=*(uint16_t*)(tcb+0x1a),vb=*(uint16_t*)(tcb+0x1c);
                  uint32_t hz=*(uint32_t*)(xb+0x9114);
                  fprintf(stderr,"[missfb-vp] 90f8(rows)=%u 90f0(width)=%u 90ac=%d 90a8=%08x 90d4=%08x 90d8=%08x 90b8=%08x 90bc=%08x 90ec=%08x\n",
                    *(uint32_t*)(xb+0x90f8),*(uint32_t*)(xb+0x90f0),*(int32_t*)(xb+0x90ac),
                    *(uint32_t*)(xb+0x90a8),*(uint32_t*)(xb+0x90d4),*(uint32_t*)(xb+0x90d8),
                    *(uint32_t*)(xb+0x90b8),*(uint32_t*)(xb+0x90bc),*(uint32_t*)(xb+0x90ec));
                  fprintf(stderr,"[missfb-vp] TCBrect L/T/R/B=%u/%u/%u/%u (w=%d h=%d)  9114ptr=%08x hz[cols 0,10,20,40,60,80]=",vl,vt,vr,vb,vr-vl,vb-vt,hz);
                  /* hz may be an ext-relative offset (<FIST_EXT_BASE) or a host pointer */
                  { uint8_t *hp = (hz && hz < 0x100000) ? (xb+hz) : (uint8_t*)(uintptr_t)hz;
                    if (hp && (uintptr_t)hp>0x10000) { int cc[6]={0,10,20,40,60,80}; for(int i=0;i<6;i++) fprintf(stderr,"%d,",hp[cc[i]]); } }
                  fprintf(stderr,"\n");
                }
                if (getenv("FIST_VEHPROBE")) {
                    uint16_t di = *(uint16_t*)(dg+0x6d34);
                    uint16_t typ = *(uint16_t*)(dg+di);
                    uint16_t bx = (uint16_t)(typ<<1);
                    fprintf(stderr,"[vehprobe] node2d34 di=0x%04x  type[di]=%u  pos[di+4/8/c]=%d/%d/%d  LUT[bx-1a40]=%u  hdg[di+38]=%u ang[di+10]=%u\n",
                        di, typ, *(int32_t*)(dg+(uint16_t)(di+4)), *(int32_t*)(dg+(uint16_t)(di+8)),
                        *(int32_t*)(dg+(uint16_t)(di+0xc)), *(uint16_t*)(dg+(uint16_t)(bx-0x1a40)),
                        *(uint16_t*)(dg+(uint16_t)(di+0x38)), *(uint16_t*)(dg+(uint16_t)(di+0x10)));
                    fprintf(stderr,"[vehprobe] cam globals 9614/9618/961c = %d/%d/%d\n",
                        *(int32_t*)(dg+0x9614), *(int32_t*)(dg+0x9618), *(int32_t*)(dg+0x961c));
                    fprintf(stderr,"[vehprobe] TCB +2c/30/34=%d/%d/%d  +d2/d6/da=%d/%d/%d\n",
                        *(int32_t*)(tcb+0x2c),*(int32_t*)(tcb+0x30),*(int32_t*)(tcb+0x34),
                        *(int32_t*)(tcb+0xd2),*(int32_t*)(tcb+0xd6),*(int32_t*)(tcb+0xda));
                }
                if (getenv("FIST_DIAL55")) {
                    /* DIAL-NEEDLE DIAGNOSTIC (shim-only): the cockpit azimuth needle (patch 311, FUN_1000_75e3)
                     * reads si=word[player+0x55], usi=clamp((abs(si)>>2)&0xfffe,0x18), sprite=word[dg:usi+0x8f24].
                     * NOTE &0xfffe: si in [56..63] all map to usi=14.  Dump the player + full roster with +0x55 AND
                     * the usi mapping (previous session compared raw +0x55, missing that 60->usi14==56->usi14). */
                    uint16_t di   = *(uint16_t*)(dg+0x6d34);   /* camera-focus vehicle (needle reads this) */
                    uint16_t pl   = *(uint16_t*)(dg+0x7ae0);   /* DAT_2000_3ae0 player */
                    #define USI55(off) ({ int _s=*(int16_t*)(dg+(uint16_t)((off)+0x55)); if(_s<0)_s=-_s; \
                                          uint16_t _u=((uint16_t)_s>>2)&0xfffe; if(_u>0x18)_u=0x18; _u; })
                    fprintf(stderr,"[dial55] focus6d34=0x%04x +55=%d usi=%u | player7ae0=0x%04x +55=%d usi=%u | type[focus]=%u\n",
                        di, *(int16_t*)(dg+(uint16_t)(di+0x55)), USI55(di),
                        pl, *(int16_t*)(dg+(uint16_t)(pl+0x55)), USI55(pl), *(uint16_t*)(dg+di));
                    fprintf(stderr,"[dial55] roster slots(off:type:+55:usi:+19flag):");
                    for (int r=0;r<0x10;r++){ uint16_t slot=*(uint16_t*)(dg+0x6d3c+r*2);
                        if(!slot){ fprintf(stderr," [%d]--",r); continue; }
                        fprintf(stderr," [%d]%04x:t%u:v%d:u%u:f%02x", r, slot, *(uint16_t*)(dg+slot),
                            *(int16_t*)(dg+(uint16_t)(slot+0x55)), USI55(slot), *(uint8_t*)(dg+(uint16_t)(slot+0x19))); }
                    fprintf(stderr,"\n");
                    #undef USI55
                    if (getenv("FIST_SPRITEDIR")) { /* sprite directory [0x4f0] at the render frame (working-mission baseline) */
                    unsigned short dseg=*(unsigned short*)(dg+0x4f0);
                    fprintf(stderr,"[spritedir] [0x4f0]=seg 0x%04x records(off:seg):",dseg);
                    unsigned base=(unsigned)dseg<<4;
                    for(int i=0;i<40;i++){ unsigned short off=*(unsigned short*)(g_mem+base+i*4);
                        unsigned short seg=*(unsigned short*)(g_mem+base+i*4+2);
                        if(off||seg) fprintf(stderr," [%d]%04x:%04x",i,off,seg); }
                    fprintf(stderr,"\n");
                }
                if (getenv("FIST_DIAL55_UPD")) { /* is the player in the c0e5 update table (0xdfbc)? + type-method slots */
                        uint16_t pl = *(uint16_t*)(dg+0x6d34);
                        uint16_t ptype = *(uint16_t*)(dg+pl);
                        fprintf(stderr,"[dial55upd] player=0x%04x type=%u  type-method word[DGROUP:(t*2-0x1bac)]: ",pl,ptype);
                        for(int t=0;t<4;t++){ uint16_t o=(uint16_t)(t*2-0x1bac); fprintf(stderr,"t%d=%04x ",t,*(uint16_t*)(dg+o)); }
                        fprintf(stderr,"\n[dial55upd] 0xdfbc update-table (stride4, 182): player-idx=");
                        int idx=-1; int nz=0;
                        for(int i=0;i<182;i++){ uint16_t s=*(uint16_t*)(dg+0xdfbc+i*4); if(s)nz++; if(s==pl)idx=i; }
                        fprintf(stderr,"%d  nonzero-entries=%d  first8=",idx,nz);
                        for(int i=0;i<8;i++) fprintf(stderr,"%04x ",*(uint16_t*)(dg+0xdfbc+i*4));
                        fprintf(stderr,"\n");
                    }
                    if (getenv("FIST_DIAL55_OBJ")) { /* dump the player object (0..0x80) as words for oracle diff */
                        uint16_t off=*(uint16_t*)(dg+0x6d34);
                        fprintf(stderr,"[dial55obj] focus=0x%04x\n",off);
                        for (int b=0;b<0x80;b+=0x10){ fprintf(stderr,"  +%02x:",b);
                            for(int i=0;i<8;i++) fprintf(stderr," %6d", *(int16_t*)(dg+(uint16_t)(off+b+i*2)));
                            fprintf(stderr,"\n"); }
                    }
                }
            }
            if (getenv("FIST_MISSFB_PALCMP")) {
                uint8_t *ep = g_mem + ((uint32_t)(*(uint16_t*)(dg+0x782))<<4);
                uint8_t *mp = xb + 0x5598;
                int same=0; for(int i=0;i<768;i++) if(ep[i]==mp[i]) same++;
                fprintf(stderr,"[palcmp] word0x782 seg=%04x  ext5598 : identical bytes=%d/768\n",*(uint16_t*)(dg+0x782),same);
                fprintf(stderr,"[palcmp] eng[0..7 idx0-2]:"); for(int i=0;i<9;i++) fprintf(stderr," %02x",ep[i]);
                fprintf(stderr,"  ext[0..7]:"); for(int i=0;i<9;i++) fprintf(stderr," %02x",mp[i]); fprintf(stderr,"\n");
                fprintf(stderr,"[palcmp] eng idx0xdc(220)*3:"); for(int i=0;i<9;i++) fprintf(stderr," %02x",ep[220*3+i]);
                fprintf(stderr,"  ext idx0xdc:"); for(int i=0;i<9;i++) fprintf(stderr," %02x",mp[220*3+i]); fprintf(stderr,"\n");
                if (getenv("FIST_MISSFB_PALDUMP")) {
                    FILE*f1=fopen("/tmp/pal_eng782.bin","wb"); if(f1){fwrite(ep,1,768,f1);fclose(f1);}
                    FILE*f2=fopen("/tmp/pal_ext5598.bin","wb"); if(f2){fwrite(mp,1,768,f2);fclose(f2);}
                    fprintf(stderr,"[palcmp] dumped /tmp/pal_eng782.bin + /tmp/pal_ext5598.bin\n");
                }
            }
            if (getenv("FIST_MISSFB_TFOLLOW")) {
                /* Reverse-verify the extender terrain-follow (0x8650): sample the loaded heightmap
                 * (ext ds:0x85bc, a 2048x2048 host buffer) at the camera XY per 0x8650's index math
                 * tile = ((camX<<13)>>(32-detail)) & mask, idx=((tileY&mask)<<detail)+(tileX&mask), then
                 * derive the height-byte -> world-Z scale that yields the oracle settled alt ~24832
                 * (camZ 26880 - eyeLUT 2048).  Sampled at BOTH the LIVE render camera (+2c/+30) and the
                 * ORACLE XY (599178/1115651) so the scale is oracle-anchored, independent of the (wrong)
                 * live Y.  Read-only diagnostic. */
                uint32_t detail = *(uint32_t*)(xb+0x8490);
                uint32_t mask   = *(uint32_t*)(xb+0x849c);
                uint32_t hmbase = *(uint32_t*)(xb+0x85bc);
                uint8_t *hm = (uint8_t*)(uintptr_t)hmbase;
                fprintf(stderr,"[tfollow] detail=%u mask=0x%x hmbase=%08x\n",detail,mask,hmbase);
                struct { const char*nm; int32_t x,y; } P[] = {
                    {"live(+2c/+30)", *(int32_t*)(tcb+0x2c), *(int32_t*)(tcb+0x30)},
                    {"oracle",        599178,               1115651},
                };
                for (int pi=0; hm && pi<2; pi++) {
                    uint32_t tx = ((uint32_t)P[pi].x << 13) >> (32-detail);
                    uint32_t ty = ((uint32_t)(-(int32_t)((uint32_t)P[pi].y<<13))) >> (32-detail);
                    uint32_t idx = (((ty & mask) << detail) + (tx & mask));
                    uint8_t h = hm[idx & 0x3fffff];
                    fprintf(stderr,"[tfollow] %-14s X=%d Y=%d tile(%u,%u) idx=%u h=%u  h<<8=%u h<<9=%u h*192=%u\n",
                        P[pi].nm, P[pi].x, P[pi].y, tx, ty, idx, h, h<<8, h<<9, h*192);
                }
            }
            if (getenv("FIST_MISSFB_RENDER")) {
                /* FIST_MISSFB_TFSETTLE: apply the asm+oracle-VERIFIED extender terrain-follow to the
                 * RENDER camera before 85d0.  Optionally FIST_MISSFB_ORACLEXY forces the oracle XY (pipeline
                 * control), else uses the live render camera.  Z is settled to the heightmap surface:
                 * camZ = (heightmap[((tileY&mask)<<detail)+(tileX&mask)] << 8) + eyeLUT(2048). */
                if (getenv("FIST_MISSFB_TFSETTLE")) {
                    if (getenv("FIST_MISSFB_ORACLEXY")) { *(int32_t*)(tcb+0x2c)=599178; *(int32_t*)(tcb+0x30)=1115651;
                        *(uint16_t*)(tcb+0x38)=19745; *(int16_t*)(tcb+0x3c)=128; *(uint16_t*)(tcb+0x3e)=256; tcb[0xcf]=0; tcb[0xcd]=1; }
                    uint32_t detail=*(uint32_t*)(xb+0x8490), mask=*(uint32_t*)(xb+0x849c);
                    uint8_t *hm=(uint8_t*)(uintptr_t)(*(uint32_t*)(xb+0x85bc));
                    int32_t cx=*(int32_t*)(tcb+0x2c), cy=*(int32_t*)(tcb+0x30);
                    uint32_t tx=((uint32_t)cx<<13)>>(32-detail), ty=((uint32_t)(-(int32_t)((uint32_t)cy<<13)))>>(32-detail);
                    uint8_t h = hm ? hm[(((ty&mask)<<detail)+(tx&mask))&0x3fffff] : 0;
                    if (hm) *(int32_t*)(tcb+0x34) = (h<<8) + 2048;
                    fprintf(stderr,"[tfsettle] cam=%d/%d/%d h=%u settledZ=%u\n",cx,cy,*(int32_t*)(tcb+0x34),h,(h<<8)+2048);
                    { const char *r3=getenv("FIST_MISSFB_RAY3A24"), *r7=getenv("FIST_MISSFB_RAY3E24");
                      if (r3){FILE*f=fopen(r3,"rb"); if(f){fread(xb+0x3a24,1,256*4,f);fclose(f);}}
                      if (r7){FILE*f=fopen(r7,"rb"); if(f){fread(xb+0x3e24,1,256*4,f);fclose(f);}}
                      if (r3||r7){ *(uint32_t*)(xb+0x90c4)=0; fprintf(stderr,"[tfsettle] ray tables injected\n"); } }
                }
                m_ext_FUN_0000_8deb();
                m_ext_FUN_0000_85d0();
                if (!getenv("FIST_MISSFB_LIVEVP")) {   /* force the oracle windshield geometry */
                    *(uint32_t*)(xb+0x90f8)=81; *(uint32_t*)(xb+0x90f0)=288;
                    *(uint32_t*)(xb+0x90ac)=0x140-288;
                    *(uint32_t*)(xb+0x90a8)=(uint32_t)(uintptr_t)(g_mem+0xA0000+0x650);
                }
                m_ext_FUN_0000_8120();
                { int32_t c0=*(int32_t*)(xb+0x90c0),v04=*(int32_t*)(xb+0x9104),v08=*(int32_t*)(xb+0x9108);
                  int32_t esi=(int32_t)(((int64_t)c0*v04)>>32), ebp=(int32_t)(((int64_t)c0*v08)>>32);
                  m_ext_FUN_0000_9200(ebp,esi); }
            }
            /* DAC palette for the dump.  Options (residual #1 -- the mission-palette architecture):
             *   default          : the engine DAC word[DGROUP:0x782] (cockpit correct, terrain 80..255 BLACK)
             *   FIST_MISSFB_EXTPAL: ext+0x5598 = full 532.pal (terrain correct, cockpit becomes NOISE)
             *   FIST_MISSFB_MERGEPAL: the COHERENT single mission DAC -- engine 0..79 (cockpit) + 532.pal
             *                       80..255 (terrain), the data-dictated partial merge.  This is the DAC the
             *                       ORIGINAL uses; the correct measurement palette once terrain renders. */
            { extern void out(int,int);
              uint8_t *eng = g_mem + ((uint32_t)(*(uint16_t*)(dg+0x782))<<4);
              uint8_t *ext = xb + 0x5598;
              static uint8_t merged[768];
              uint8_t *mp;
              if (getenv("FIST_MISSFB_MERGEPAL")) {
                  for (int i=0;i<80*3;i++)  merged[i]=eng[i]&0x3f;   /* cockpit band 0..79  = engine */
                  for (int i=80*3;i<768;i++) merged[i]=ext[i]&0x3f;  /* terrain band 80..255 = 532.pal */
                  mp = merged;
              } else mp = getenv("FIST_MISSFB_EXTPAL") ? ext : eng;
              out(0x3c8,0); for(int pi=0;pi<768;pi++) out(0x3c9, mp[pi] & 0x3f); }
            *(uint32_t*)(xb+0xc93)=save_c93;
            if (getenv("FIST_MISSFB_MEAN")) {
                /* terrain-band (rows 8-88) mean RGB through the merged mission DAC (same merge as the
                 * DAC upload above): the docs' isolation metric.  Oracle render mean = (122,106,71);
                 * default port = (109,78,54).  Also report non-black px + distinct indices. */
                uint8_t *fb = g_mem + 0xA0000;
                uint8_t *eng = g_mem + ((uint32_t)(*(uint16_t*)(dg+0x782))<<4);
                uint8_t *ext = xb + 0x5598;
                static uint8_t pal[768];
                for (int i=0;i<80*3;i++)  pal[i]=(eng[i]&0x3f);
                for (int i=80*3;i<768;i++) pal[i]=(ext[i]&0x3f);
                long sr=0,sg=0,sb=0,nb=0; int hh[256]={0},nd=0;
                for (int r=8;r<88;r++) for (int c=0;c<320;c++){ uint8_t p=fb[r*320+c];
                    if(!hh[p]){hh[p]=1;nd++;} if(p){ nb++;
                        sr+=(pal[p*3]<<2)|(pal[p*3]>>4); sg+=(pal[p*3+1]<<2)|(pal[p*3+1]>>4); sb+=(pal[p*3+2]<<2)|(pal[p*3+2]>>4);} }
                fprintf(stderr,"[missfb-mean] terrain rows8-88 nonblack=%ld/25600 distinct=%d mean RGB=(%ld,%ld,%ld)\n",
                    nb, nd, nb?sr/nb:0, nb?sg/nb:0, nb?sb/nb:0);
            }
            /* FIST_MISSFB_DGDUMP=<path>: write the whole 64 KB DGROUP at the FB capture point.  This
             * is the three-run parity method for a mission FRAME -- the per-tick FIST_SIMHASH cannot
             * reach here, because the in-mission 459a loop never re-enters fist_timer_pump, so [0x452]
             * is frozen and no tick-indexed fingerprint advances.
             * It dumps the whole low 1 MB, not just DGROUP: the first divergence found this way was a
             * glyph-index STRING living in the string segment around linear 0x5d2a2, i.e. outside
             * DGROUP entirely, so a DGROUP-only dump reported "identical" while the frame differed.
             * CAUTION, measured: the op-0x24 post count being equal does NOT make the two targets'
             * snapshots contemporaneous.  On terrain-azer1 the 1st op-0x24 post finds [0x452]=41 on
             * native (SIGALRM time base, and the value does not move with FIST_TICK_HZ) and 314 on
             * wasm; with FIST_COOP_TICK=1 native reads 354, again independent of FIST_TICK_HZ.  So the
             * flow compares two DIFFERENT simulation states and any equality it reports is incidental.
             * Read the [0x452] in both dumps before drawing a conclusion from a diff.  board:0002 */
            { const char *dgp=getenv("FIST_MISSFB_DGDUMP");
              if (dgp) { FILE *f=fopen(dgp,"wb"); if(f){ fwrite(g_mem,1,0x100000,f); fclose(f);
                         fprintf(stderr,"[missfb] low 1 MB dumped -> %s (DGROUP at +0x1c000)\n", dgp); } } }
            { const char *fbp=getenv("FIST_MISSFB"); if(fbp) fist_dump_framebuffer(fbp); }
            _exit(0);
        }
    }
    /* ------------------------------------------------------------------------------------------------
     * FIST_GEOMDUMP -- deterministic mission-viewport geometry probe (default OFF, read-only).
     * The op-0x4c present pump spins in the extender gate WITHOUT ever calling fist_timer_pump, so the
     * tick-anchored FIST_DUMPTICK/FIST_RUNMS watchdogs never fire there.  This gate IS reached each
     * present iteration, so it is the deterministic anchor: (1) log every change of the STRSEG word
     * [DGROUP:0x70] (the base for 4308's model table @STRSEG:0x802); (2) on the FIRST mission-phase op
     * (0x18/0x0c) dump the model/unit table + the viewport geometry descriptors word[0x7ac0] (SRC, the
     * 60d9->mga-0x2604/22bf source) and word[0x7aa4] (DST); (3) sample the geometry + DAT_2000_3aa2
     * (the 60d9 one-shot guard: 0xffff once 60d9 has run) periodically on op 0x4c.  FINDING: geometry
     * stays 0 and 3aa2 stays 0 across the whole cascade -- the copier 60d9 is never reached and the SRC
     * descriptor 0x7ac0 is never populated, INDEPENDENT of the (now-working) model load. */
    if (getenv("FIST_GEOMDUMP")) {
        static uint16_t last70 = 0xabcd;
        uint16_t cur70 = *(uint16_t*)(dg + 0x70);
        if (cur70 != last70) { fprintf(stderr,"[geom] STRSEG word[0x70] %04x -> %04x (at op 0x%02x)\n", last70, cur70, op); last70 = cur70; }
        if (op == 0x18 || op == 0x0c) {
            static int done = 0;
            if (!done) { done = 1;
                uint16_t s = cur70; uint8_t *mt = g_mem + ((uint32_t)s << 4) + 0x802;
                fprintf(stderr, "[geom] FIRST mission op=0x%02x  STRSEG=%04x  model-table @%04x:0x802:\n", op, s, s);
                for (int i=0;i<12;i++){ uint16_t *e = (uint16_t*)(mt + i*8);
                    if (!e[0] && !e[1] && !e[2] && !e[3]) { fprintf(stderr,"[geom]  entry %d ALL-ZERO (end)\n", i); break; }
                    fprintf(stderr,"[geom]  entry %d: flags=%04x [+2]=%04x nameidx=%04x vec=%04x\n", i, e[0],e[1],e[2],e[3]); }
                fprintf(stderr,"[geom] SRC word[0x7ac0]=%04x  DST word[0x7aa4]=%04x  d552=%04x d556a=%04x d548=%02x\n",
                    *(uint16_t*)(dg+0x7ac0), *(uint16_t*)(dg+0x7aa4), *(uint16_t*)(dg+0x1552), *(uint16_t*)(dg+0x156a), dg[0x1548]);
            }
        }
        if (op == 0x4c) {
            static int n4c = 0;
            if (++n4c % 50 == 1)
                fprintf(stderr,"[geom] op4c #%d  3aa2(60d9-ran)=%04x  SRC[0x7ac0]=%04x DST[0x7aa4]=%04x  d552=%04x d548=%02x\n",
                    n4c, *(uint16_t*)(g_mem+0x23aa2), *(uint16_t*)(dg+0x7ac0), *(uint16_t*)(dg+0x7aa4),
                    *(uint16_t*)(dg+0x1552), dg[0x1548]);
        }
    }
    if (op == 0) {                                  /* CREATE TASK */
        uint16_t bytes = *(uint16_t *)(dg + 0xea1a);
        uint16_t paras = (uint16_t)((bytes + 15u) >> 4);
        if (paras == 0) paras = 1;
        if ((uint32_t)g_ext_next_seg + paras > EXT_TASK_POOL_TOP) {
            fprintf(stderr, "[ext] create-task pool EXHAUSTED (need 0x%x paras; next 0x%04x top 0x%04x) "
                            "-- extender task memory too small\n", paras, g_ext_next_seg, EXT_TASK_POOL_TOP);
            return 0;
        }
        uint16_t seg = g_ext_next_seg;
        g_ext_next_seg = (uint16_t)(g_ext_next_seg + paras);
        memset(g_mem + ((uint32_t)seg << 4), 0, (uint32_t)paras << 4);
        fprintf(stderr, "[ext] create-task: %u bytes (0x%x paras) -> task seg 0x%04x (linear 0x%05x)\n",
                bytes, paras, seg, (uint32_t)seg << 4);
        return seg;
    }
    if (op == 0x80) {
        /* op 0x80 = the .CAM in-place DECRYPT.  Faithful C reconstruction of the Doug-Huffman
         * extender kernel FUN_00007762/FUN_0000776e (re_out/fist_image.bin @ image 0x7762), the
         * op-table entry [0xcb3+0x80]=0x7762 dispatched by the PM gate FUN_00000f30
         * (`movzx ebx,bx; mov ebx,[ebx+0xcb3]; call ...`).  ASM-verified stream cipher over 8-byte
         * blocks (keys 0xfade2bad/0xace4dead, per-block LFSR update); validated vs armoredfist/
         * FISTDATA/A.CAM -> section-0 "DESC: TRAINING".  The engine (FIST.DAT bd14->e2a5) sets up
         * esi=(STRSEG<<4)+0x452c (the read buffer) and ecx=len (bytes read, DGROUP:0xf810), writes
         * buf_lin into the current-TCB inbox (TCB+0x3f2 = ebx), then invokes the gate with aa10=0x80.
         * The buffer is passed to the handler in ESI (asm bd14 0x1bd58: esi=(STRSEG<<4)+0x452c), NOT via
         * the TCB inbox (e2a5 writes EBX to the inbox, which bd14 leaves undefined for this op; only ops
         * that carry a command in EBX use it).  So compute buf_lin = (word[DGROUP:0x70]<<4)+0x452c
         * directly (0x452c is the fixed bd14 read buffer; op 0x80 is EXCLUSIVELY the .CAM decrypt --
         * e2a5 is called only from bd14).  len = the engine's ecx = word[DGROUP:0xf810] (bytes read). */
        uint32_t strseg = *(uint16_t *)(dg + 0x70);
        uint32_t buf_lin = (strseg << 4) + 0x452c;      /* esi (+[0x807]=0 identity) */
        uint16_t len = *(uint16_t *)(dg + 0xf810);
        uint32_t blocks = (uint32_t)len >> 3;
        uint8_t *esi = g_mem + buf_lin;                 /* [0x807]=0 identity */
        if (blocks) {
            uint32_t ebp = 0xace4deadu, eax = 0xfade2badu, ebx, edx;
            for (uint32_t i = 0; i < blocks; i++) {
                eax = eax << 1;                                 /* shl eax,1 */
                edx = (eax & 0x80000000u) ? 0xffffffffu : 0u;   /* cdq */
                edx &= 0xbc9abb09u;
                eax = eax + edx + 1u;                           /* stc; adc eax,edx */
                ebp = (ebp >> 1) | (ebp << 31);                 /* ror ebp,1 */
                memcpy(&ebx, esi, 4);
                ebx ^= eax;
                ebx = (ebx & 0xffff0000u) | ((ebx & 0xffu) << 8) | ((ebx >> 8) & 0xffu); /* xchg bh,bl */
                memcpy(esi, &ebx, 4);
                memcpy(&ebx, esi + 4, 4);
                ebx ^= ebp;
                ebx = (ebx >> 16) | (ebx << 16);                /* ror ebx,16 */
                memcpy(esi + 4, &ebx, 4);
                esi += 8;
            }
        }
        if (getenv("FIST_CAM_TRACE"))
            fprintf(stderr, "[ext] op 0x80 .CAM decrypt: buf_lin=0x%05x len=%u blocks=%u -> '%.24s'\n",
                    buf_lin, len, blocks, (char *)(g_mem + buf_lin));
        return 0;
    }
    if (op == 0x18 && g_ext_ready) {
        /* op 0x18 = MAP-LOAD.  op-table @ fist_image.bin:0xcb3[0x18] -> 0x10ca
         *   (`movl $0x10a0, ds:0xd82 ; call 0x89b0`).  0x89b0 (the extender map-load setup) reads the
         * current-TCB [0xc93], probes memory ("4.MEG".."40.MEG") for the terrain LOD, sets the resource
         * dir to "PAL.RES", and opens/loads the map via its own 32-bit-flat FILEMGR (FUN_00005c98/5cc2/
         * 5d50 -> INT 21h AH=4E/3D/3F, serviced by fist_dos.c dos_int_ext with g_fist_ext_int set):
         *   TCB+0x9a = "532.pal" (palette)   [FUN_00006032]
         *   TCB+0x7a = "D32.KLC" (heightmap) [FUN_0000643c]
         *   TCB+0x8a = "C32.KLC" (colormap)  [FUN_0000643c]
         *   TCB+0xaa = "5.SKY"   (sky)       [conditional on DAT_395c]
         * FUN_1000_63b8 staged those names into the ENGINE current-TCB (DAT_2000_aa2c + 0x7a/0x8a/0x9a/
         * 0xaa), so point the extender current-TCB [0xc93] at aa2c (host pointer; [0x807]=0 identity ->
         * TCB-relative name buffers are real host strings that ext_addr() passes straight to open_ci).
         * The gate passes EBX = TCB inbox (word[aa2c+0x3f2]); 0x89b0's param_1/2/3 are handle/flags
         * passthroughs that do NOT reach the FILEMGR name path (the name pointer is read from the TCB),
         * so the file opens are faithful regardless.  Runs ONCE per mission (guarded) to match the single
         * real map load.  Behaviour-neutral for the 19 covered flows (op 0x18 is mission-cascade only). */
        static int map_loaded = 0;
        if (!map_loaded) {
            map_loaded = 1;
            { extern void fist_map_trace(void); fist_map_trace(); }   /* board:0017 FIST_RNGTRACE sync point */
            uint32_t tcb_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
            uint32_t inbox   = *(uint32_t *)(g_mem + tcb_lin + 0x3f2);
            uint8_t *xb = g_mem + FIST_EXT_BASE;
            uint32_t save_c93 = *(uint32_t *)(xb + 0xc93);
            *(uint32_t *)(xb + 0xc93) = (uint32_t)(uintptr_t)(g_mem + tcb_lin);   /* extender cur-TCB */
            *(uint32_t *)(xb + 0xd82) = 0x10a0;                                    /* mirror 0x10ca */
            { const char *a=getenv("FIST_AC64OVR"); if(a) g_mem[tcb_lin+0x54]=(uint8_t)strtoul(a,0,0); }
            fprintf(stderr, "[ext] op 0x18 MAP-LOAD: TCB @0x%05x  '%.13s'/'%.13s'/'%.13s'/'%.13s'  [+0x54]=%02x [+0x59]=%02x\n",
                    tcb_lin, g_mem+tcb_lin+0x7a, g_mem+tcb_lin+0x8a, g_mem+tcb_lin+0x9a, g_mem+tcb_lin+0xaa,
                    g_mem[tcb_lin+0x54], g_mem[tcb_lin+0x59]);
            /* DRIVE THE EXTENDER TASK-SETUP ALLOCATOR (the located bc90 gate).  The original runs
             * FUN_0000_84c0 during the extender exec/task-load (fist_image.bin spawn routines 0dee/... ->
             * 84c0), BEFORE 89b0's map-load.  84c0 allocates, via the seeded bump allocator FUN_0000_36bf/
             * 3772 ([0x90b]/[0x90f]), the persistent palette-reduction working buffers -- the 64 KB
             * color-distance MATRIX at [0xbc90] (0x10000-aligned, so bc9c's low-16 (col<<8|row) index math
             * lands in the 64 KB block), bc94/85c0/85c4/3909/390d/3918 -- and sets the bc98 FREE-CHECKPOINT
             * that 89b0's opening 3322(&bc98) releases to.  This port had never driven 84c0, so [0xbc90]=0
             * -> bc9c wrote through a near-NULL ptr (SIGSEGV).  [0xc93] is still the extender task here
             * (0x90000), so 84c0's TCB+0x488 reset lands on the extender task, not the engine mission TCB.
             * Runs once (map_loaded guard).  84c0 does no INT-21 (pure allocation) -> no ext-mode needed. */
            /* Task-load reinit of the extender bump allocator (FUN_0000_2f7c): the original runs 84c0 at
             * a FRESH task-load (registry empty), so bc90..bc98 register at indices 0..7 with the bc98
             * FREE-CHECKPOINT LAST -> 89b0's opening 3322(&bc98) then frees only per-map blocks ABOVE the
             * checkpoint, keeping the matrix.  This port reaches op 0x18 with a STALE size-0 bc98 already
             * registered at index 0 (a leftover checkpoint from the intro/menu heap), so without the reset
             * 84c0's own bc98 registration DEDUPES away, bc98 stays at index 0, and 3322(&bc98) frees the
             * whole registry (indices 0..7) -> zeroes [0xbc90] -> bc9c NULL-writes (SIGSEGV).  Forcing
             * 2f50=0 makes 84c0's first 36bf call 2f7c (2f50=[0x90b], 2f54=0) -> fresh registry, bc98 at
             * index 7.  Verified: 3661(&bc98)->idx 7, 3322 preserves bc90.  The KDV intro heap is done by
             * op 0x18 (2f54 was already 1, cursor already at base) -> the reset only drops the stale marker. */
            *(uint32_t*)(xb+0x2f50) = 0;
            m_ext_FUN_0000_84c0(inbox);
            /* PRELOAD BLOCK A -- the extender's `rep movsd` memcpy (@lin 0x4708, src fs:[0x1f0]) fills
             * [bc90]=0x10000 with the 64K colour-blend matrix from DSOUNDS.BIN[0xe00:] before the menu;
             * bdc4 (the colormap spatial-upsampler) reads it later.  84c0 has now allocated the [bc90]
             * buffer; populate it with the game's own DSOUNDS.BIN block A (see fist_preload_blockA).  The
             * subsequent build aliases [bc90] -> the tile buffer for bc9c, then restores [bc90]=save_bc90
             * (the block-A buffer we just filled), so block A survives the map-load unclobbered. */
            {
                uint32_t bA = *(uint32_t *)(xb + 0xbc90);
                long bn = fist_preload_blockA((void *)(uintptr_t)bA);
                if (getenv("FIST_BLKADUMP") && bA) {
                    FILE *f = fopen(getenv("FIST_BLKADUMP"), "wb");
                    if (f) { fwrite((void *)(uintptr_t)bA, 1, 0x10000, f); fclose(f);
                        fprintf(stderr, "[blkadump] port block A ([bc90] preload target) 64KB (%ld copied) -> %s\n",
                                bn, getenv("FIST_BLKADUMP")); }
                }
            }
            /* "bc90 REUSED = tile 0x44200 during the build" (docs/oracle_tile3918_producer.md):
             * the ORIGINAL builds the pairwise palette-blend LUT (bc9c) into the SAME 64 KB buffer that
             * ds:0x3918 points at -- ds:0xbc90 aliases the tile during map-load.  The port's 84c0
             * allocated bc90 (082c0000) and 3918 (082f0000) as SEPARATE buffers, so patch-289's fixed
             * bc9c wrote the blend LUT into bc90 while the renderer 9200 (SMC-sampling ds:0x3918) saw an
             * empty tile.  Alias bc90 -> the tile buffer for the build so bc9c fills what 9200 samples.
             * (bc9c/bd0e/bd62 use bc94 for the depth-shade LUTs, a different slot, so bc90 is free to
             * carry the tile ptr through the build.)  Saved/restored around 89b0. */
            uint32_t save_bc90 = *(uint32_t *)(xb + 0xbc90);
            uint32_t save_3918 = *(uint32_t *)(xb + 0x3918);
            uint32_t tile3918  = save_3918;
            /* Experiment FIST_TILEWIN: the ORIGINAL's [3918]=0x44200 is NOT 64KB-aligned; bc9c writes a
             * symmetric matrix into the 64KB-aligned block [bc90]=0x40000, and [3918] windows it at linear
             * offset 0x4200 (=row 66).  Port Route-1 pointers are 64KB-aligned (low16=0) so the window
             * offset is lost.  With FIST_TILEWIN=0x4200: build into the aligned base, then window [3918]. */
            uint32_t win = getenv("FIST_TILEWIN") ? (uint32_t)strtoul(getenv("FIST_TILEWIN"),0,0) : 0;
            /* FIST_NOTILEALIAS (default OFF, DIAGNOSTIC): skip the bc90->tile3918 alias so bdc4 reads the
             * preloaded BLOCK A -> the 254-distinct LIGHT reduce lands at [0x85b8] instead of the 89-collapse
             * (colormap-groundtruth part 2).  Measures the reduce source's distinct-count.  Corrupts the tile
             * build (bc9c writes to blockA), so tile metrics are invalid under this flag -- reduce-only test. */
            if (tile3918 && !getenv("FIST_NOTILEALIAS")) *(uint32_t *)(xb + 0xbc90) = tile3918;   /* bc90 REUSED = tile aligned base */
            /* SKY-SETUP reconstruction (FUN_0000_7660).  89b0's tail builds the 5.SKY source [0x3911] only
             * when [0x395c]!=0, and [0x3958] selects the sky-resample fn.  Both are set by the extender op
             * op-table[0x22] -> 0x10da -> 0x7660, which lives in a Ghidra decompile GAP (0x7490..0x76fd not
             * decompiled) so the port never ran it -> [0x395c] stayed 0 -> no windshield sky.  The original
             * posts this op before op-0x18 map-load (dosbox-fist guest-RAM: original [0x395c]=1 while all
             * four N.MEG find-firsts fail -- so the .MEG probe was NOT the setter).  Reconstructed verbatim
             * from the pinned asm (objdump 0x7660): from the current TCB's +0xcc detail byte, pick the
             * sky-render fn ptr [0x3958] and the sky flag [0x395c].  (oracle AZER1: TCB[+0xcc]=1.) */
            {
                uint8_t *t7660 = (uint8_t*)(uintptr_t)(*(uint32_t*)(xb+0xc93));   /* [0xc93] = current TCB */
                uint8_t cc = t7660[0xcc];
                if (cc == 0) { *(uint32_t*)(xb+0x3958)=0x6877; g_mem[FIST_EXT_BASE+0x395c]=1; }
                else         { *(uint32_t*)(xb+0x3958)=0x689a; g_mem[FIST_EXT_BASE+0x395c]=cc; }
            }
            if (getenv("FIST_FORCE395C"))       /* diagnostic override of the reconstruction above */
                g_mem[FIST_EXT_BASE+0x395c] = (uint8_t)strtoul(getenv("FIST_FORCE395C"),0,0);
            g_fist_ext_int = 1;                    /* extender-mode flat FILEMGR INT 21h */
            if(getenv("FIST_FORCELOD")){ uint8_t*xb=g_mem+FIST_EXT_BASE;
                uint32_t tcb=*(uint32_t*)(xb+0xc93);
                if(tcb){ *(uint8_t*)(g_mem+tcb+0x59)=(uint8_t)strtoul(getenv("FIST_FORCELOD"),0,0);
                    fprintf(stderr,"[forcelod] TCB[+0x59]=%u\n",g_mem[tcb+0x59]); } }
            if(getenv("FIST_LODCHK")){ uint8_t*xb=g_mem+FIST_EXT_BASE;
                fprintf(stderr,"[lodchk PRE ] 8490=%u 8494=%u 395c=%u 395d=%u\n",
                    *(uint32_t*)(xb+0x8490),*(uint32_t*)(xb+0x8494),xb[0x395c],xb[0x395d]);}
            m_ext_FUN_0000_89b0(inbox, inbox, inbox);
            if(getenv("FIST_LODCHK")){ uint8_t*xb=g_mem+FIST_EXT_BASE;
                fprintf(stderr,"[lodchk POST] 8490=%u 8494=%u 395c=%u 395d=%u\n",
                    *(uint32_t*)(xb+0x8490),*(uint32_t*)(xb+0x8494),xb[0x395c],xb[0x395d]);}
            g_fist_ext_int = 0;
            /* FIST_DS3911DUMP (default OFF, READ-ONLY): dump the port's ds:0x3911 SOURCE buffer -- the
             * static map-load buffer the extender's per-frame perspective tile-resample FUN_0000_689a
             * samples (`add esi,[0x3911]` @ ext 0x6933).  89b0's tail (re_out/fist_ext.c ~13268) builds it:
             * open TCB+0xaa asset -> 9ec0 read into [0x3911] -> bdc4 upsample per detail level.  Byte-
             * compared against the oracle ground truth tools/oracle/samples/oracle_lighttile_source_ds3911_static.bin
             * (docs/oracle_lighttile_source.md).  0x937 = the allocation size 36bf reserved. */
            if (getenv("FIST_DS3911DUMP")) {
                uint32_t src  = *(uint32_t *)(xb + 0x3911);
                uint32_t sz   = *(uint32_t *)(xb + 0x937);
                uint8_t *sp   = (src && src < 0x100000) ? (xb + src) : (uint8_t *)(uintptr_t)src;
                fprintf(stderr, "[ds3911] [0x3911]=%08x  [0x937]size=%u (0x%x)  [0x395c]detail=%u  TCB[+0xcc]=%u [+0xd1]=%u [0x3958]=%08x\n",
                        src, sz, sz, (unsigned)g_mem[FIST_EXT_BASE+0x395c],
                        (unsigned)g_mem[tcb_lin+0xcc], (unsigned)g_mem[tcb_lin+0xd1],
                        *(uint32_t*)(g_mem+FIST_EXT_BASE+0x3958));
                if (src) { FILE *f=fopen(getenv("FIST_DS3911DUMP"),"wb");
                    if(f){ long n = sz && sz <= 0x40000 ? sz : 0x40000; fwrite(sp,1,n,f); fclose(f);
                        fprintf(stderr,"[ds3911] port source buffer %ld B -> %s\n", n, getenv("FIST_DS3911DUMP")); } }
                if (getenv("FIST_DS3911EXIT")) { fflush(stderr); _exit(0); }
            }
            /* FIST_BC90DUMP (default OFF): dump the 64 KB buffer bc9c just filled -- the LIVE [bc90]
             * target (during the map-load build [bc90] is aliased to the tile buffer, so this == the
             * port's bc9c blend-matrix output).  Compared byte-for-byte against the ORACLE's live bc9c
             * output (tools/oracle/trace_bc90.sh, phys 0x175200) to settle the bc9c faithfulness verdict. */
            if (getenv("FIST_BC90DUMP")) {
                uint32_t bb = *(uint32_t *)(xb + 0xbc90);
                if (bb) { FILE *f=fopen(getenv("FIST_BC90DUMP"),"wb");
                    if(f){ fwrite((void*)(uintptr_t)bb,1,0x10000,f); fclose(f);
                        fprintf(stderr,"[bc90dump] port bc9c matrix (live [bc90] target) 64KB -> %s\n", getenv("FIST_BC90DUMP")); } }
            }
            *(uint32_t *)(xb + 0xbc90) = save_bc90;
            if (win && tile3918) *(uint32_t *)(xb + 0x3918) = tile3918 + win;  /* window the tile */
            /* FIST_TILEWRAP (default OFF): faithful WRAPPED window of bc9c's 64KB block.
             * The ORIGINAL's [3918] tile ptr windows bc9c's aligned block M at a 16-bit-offset that
             * WRAPS at 64KB (the extender allocator's [bc90]/[3918] geometry across the 0x50000 block
             * boundary).  The port's flat pointers can't 16-bit-wrap, and the non-wrapped `+win` seam
             * above reads out-of-block garbage.  Measurement/repro proof (this iteration): the live
             * bc9c block-base wrapped at 0xf200 == the oracle tile (rows 14..255 = 61952/61952 byte-
             * exact; rows 0..13 = bc06's LOD region, a separate buffer-overlap follow-up).  Materialize
             * the wrapped window into the tile buffer the 9200 sampler reads (in place; M/block-base is
             * consumed only by the tile sampler after this point -- bdc4 reads the separate block A). */
            if (getenv("FIST_TILEWRAP") && tile3918) {
                uint32_t tw = (uint32_t)strtoul(getenv("FIST_TILEWRAP"),0,0);
                uint8_t *M = (uint8_t *)(uintptr_t)tile3918;
                uint8_t *tmp = (uint8_t *)malloc(0x10000);   /* heap: no static/BSS layout footprint */
                if (tmp) { memcpy(tmp, M, 0x10000);
                    for (uint32_t i = 0; i < 0x10000; i++) M[i] = tmp[(i + tw) & 0xffff];
                    free(tmp); }
            }
            *(uint32_t *)(xb + 0xc93) = save_c93;
            fprintf(stderr, "[ext] op 0x18 MAP-LOAD returned; detail[0x8490]=0x%x dim[0x8494]=%u (base=0x0b => 2048)\n",
                    *(uint32_t *)(xb + 0x8490), *(uint32_t *)(xb + 0x8494));
            if (getenv("FIST_MAPPROBE")) fprintf(stderr, "[mapprobe] DAT_8498(cm-off)=0x%x [0x85bc](HM)=0x%x [0x85b8](CM)=0x%x  CM-HM=0x%x\n",
                    *(uint32_t*)(xb+0x8498), *(uint32_t*)(xb+0x85bc), *(uint32_t*)(xb+0x85b8),
                    *(uint32_t*)(xb+0x85b8) - *(uint32_t*)(xb+0x85bc));
            if (getenv("FIST_CMDUMP")) {
                uint32_t cmb = *(uint32_t*)(xb+0x85b8);
                if (cmb) { FILE *f=fopen(getenv("FIST_CMDUMP"),"wb"); if(f){ fwrite((void*)(uintptr_t)cmb,1,0x400000,f); fclose(f);
                    fprintf(stderr,"[cmdump] 85b8 colormap dumped -> %s\n", getenv("FIST_CMDUMP")); } }
            }
            /* FIST_PALDUMP (READ-ONLY, board:0002): dump the bc9c SOURCE palette at ext[0x5598] (256 RGB
             * triples) + report how many of entries 190..255 are populated -- tests whether the port's
             * "colormap rows 190..255 all zero" originates in an incomplete source-palette load (upstream
             * of bc9c) rather than bc9c itself (whose build loop covers full 0..255). */
            if (getenv("FIST_PALDUMP")) {
                uint8_t *sp = xb + 0x5598;
                int hi=0; for(int i=190*3;i<256*3;i++) if(sp[i]) hi++;
                int lo=0; for(int i=80*3;i<190*3;i++) if(sp[i]) lo++;
                fprintf(stderr,"[paldump] ext[0x5598] source palette: entries 80..189 nonzero=%d/%d ; entries 190..255 nonzero=%d/%d ; e190=%d,%d,%d e255=%d,%d,%d\n",
                    lo,110*3,hi,66*3, sp[190*3],sp[190*3+1],sp[190*3+2], sp[255*3],sp[255*3+1],sp[255*3+2]);
                FILE *f=fopen(getenv("FIST_PALDUMP"),"wb"); if(f){ fwrite(sp,1,768,f); fclose(f); }
            }
            /* FIST_CM1MDUMP (board:0002): dump the 1MB region at [0x85bc]+0x100000 whose stats match the
             * live-paging oracle CM (r69.r6980.map_cm.bin: 105 distinct / 1047525 nonzero) -- byte-diff
             * against it to settle whether the port terrain colormap is already bit-identical. */
            if (getenv("FIST_CM1MDUMP")) {
                uint32_t hmb = *(uint32_t*)(xb+0x85bc);
                if (hmb) { FILE *f=fopen(getenv("FIST_CM1MDUMP"),"wb");
                    if(f){ fwrite((void*)(uintptr_t)(hmb+0x100000),1,0x100000,f); fclose(f);
                    fprintf(stderr,"[cm1mdump] [0x85bc]+1M dumped -> %s\n", getenv("FIST_CM1MDUMP")); } }
            }
            /* FIST_HMDUMP (default OFF, READ-ONLY): dump the port's OWN op-0x18-decoded HEIGHTMAP as 6980
             * addresses it -- [0x85bc] first 1MB (the coord layout hm[(v>>22)<<10|(u>>22)]).  Byte-compared
             * to the capture HM (voxel6980_framematched_pass08) to settle whether the port's own contiguous
             * map differs from the flat frame-boundary capture. */
            if (getenv("FIST_HMDUMP")) {
                uint32_t hb = *(uint32_t*)(xb+0x85bc);
                if (hb) { FILE *f=fopen(getenv("FIST_HMDUMP"),"wb"); if(f){ fwrite((void*)(uintptr_t)hb,1,0x100000,f); fclose(f);
                    fprintf(stderr,"[hmdump] port [0x85bc] heightmap first 1MB -> %s\n", getenv("FIST_HMDUMP")); }
                    uint32_t cb=*(uint32_t*)(xb+0x85bc)+0x100000;
                    FILE *g2=fopen("/tmp/port_CM.bin","wb"); if(g2){ fwrite((void*)(uintptr_t)cb,1,0x100000,g2); fclose(g2); }
                    /* FIST_HMDUMP also banks [0x85b8] (the reduce output 6980 SHOULD read) -> /tmp/port_CM858.bin */
                    uint32_t c8=*(uint32_t*)(xb+0x85b8);
                    if(c8){ FILE*g3=fopen("/tmp/port_CM858.bin","wb"); if(g3){ fwrite((void*)(uintptr_t)c8,1,0x100000,g3); fclose(g3);
                        fprintf(stderr,"[hmdump] port [0x85b8] reduce 1MB -> /tmp/port_CM858.bin\n"); } } }
                if (getenv("FIST_HMEXIT")) { fflush(stderr); _exit(0); }
            }
            /* FIST_MTXDUMP: dump the 64 KB blend matrix that bc9c filled + bdc4 read (aliased at 0x3918). */
            if (getenv("FIST_MTXDUMP")) {
                uint32_t mb = save_3918;
                if (mb) { FILE *f=fopen(getenv("FIST_MTXDUMP"),"wb"); if(f){ fwrite((void*)(uintptr_t)mb,1,0x10000,f); fclose(f);
                    fprintf(stderr,"[mtxdump] blend matrix (3918/bc90 build buf) 64KB dumped -> %s\n", getenv("FIST_MTXDUMP")); } }
            }
            /* FIST_BBDUMP: dump the 64KB-aligned BLOCK BASE that bc9c wrote (bc9c masks its dest to
             * `[bc90] & 0xffff0000 | (ch<<8|cl)`, so the flat M[ch][cl] matrix lives at the block base,
             * NOT at the +0x4200-windowed tile pointer that FIST_BC90DUMP/FIST_MTXDUMP read). */
            if (getenv("FIST_BBDUMP")) {
                uint32_t bbase = save_3918 & 0xffff0000u;
                if (bbase) { FILE *f=fopen(getenv("FIST_BBDUMP"),"wb"); if(f){ fwrite((void*)(uintptr_t)bbase,1,0x10000,f); fclose(f);
                    fprintf(stderr,"[bbdump] bc9c block-base (flat M[ch][cl]) 64KB @%#x -> %s\n", bbase, getenv("FIST_BBDUMP")); } }
            }
            /* FIST_AC70PROBE: call the BUILT ac70 with a controlled target to check the nearest-index result. */
            if (getenv("FIST_AC70PROBE")) {
                extern uint32_t m_ext_FUN_0000_ac70(void);
                uint8_t sv60=xb[0xac60], sv64=xb[0xac64];
                uint32_t sv60d=*(uint32_t*)(xb+0xac60), sv64d=*(uint32_t*)(xb+0xac64);
                *(uint32_t*)(xb+0xac64)=80; *(uint32_t*)(xb+0xac60)=255;
                for (int i=80;i<88;i++){
                    xb[0xac68]=xb[0x5260+i*3]; xb[0xac69]=xb[0x5260+i*3+1]; xb[0xac6a]=xb[0x5260+i*3+2];
                    uint32_t r=m_ext_FUN_0000_ac70();
                    fprintf(stderr,"[ac70probe] i=%d pal5260=(%d,%d,%d) 4f60=(%d,%d,%d) ac70=%u  acb6/c0/ca=%d/%d/%d\n",
                        i,xb[0x5260+i*3],xb[0x5260+i*3+1],xb[0x5260+i*3+2],
                        xb[0x4f60+i*3],xb[0x4f60+i*3+1],xb[0x4f60+i*3+2],r&0xff,xb[0xacb6],xb[0xacc0],xb[0xacca]);
                }
                *(uint32_t*)(xb+0xac60)=sv60d; *(uint32_t*)(xb+0xac64)=sv64d;
            }
            if (getenv("FIST_PALDUMP")) {
                FILE *f=fopen(getenv("FIST_PALDUMP"),"wb");
                if(f){ fwrite(xb+0x5598,1,768,f); fwrite(xb+0x4f60,1,768,f); fwrite(xb+0x5260,1,768,f);
                       fwrite(xb+0xa060,1,1024,f); fwrite(xb+0xa460,1,1024,f); fwrite(xb+0xa860,1,1024,f); fclose(f);
                    fprintf(stderr,"[paldump] 5598/4f60/5260/a060/a460/a860 dumped 28a5=%d ac64=%d ac60=%d\n",
                       (int)g_mem[FIST_EXT_BASE+0x28a5],(int)g_mem[FIST_EXT_BASE+0xac64],(int)g_mem[FIST_EXT_BASE+0xac60]); }
            }
            if (getenv("FIST_MAPPROBE")) {
                /* Distinct-count each terrain buffer right after 89b0 (Part-1b collapse verdict). */
                struct { const char *nm; uint32_t off; long n; } B[] = {
                    {"85b8(colormap)",0x85b8, 0x400000}, {"85bc(height)",0x85bc, 0x400000},
                    {"85bc+1M",0x85bc, 0}, {"bc90(blendmtx)",0xbc90, 0x10000},
                    {"3918(tile)",0x3918, 0x10000},
                };
                for (int bi=0; bi<5; bi++) {
                    uint32_t base = *(uint32_t*)(xb+B[bi].off);
                    if (bi==2) base += 0x100000;   /* 85bc+1M */
                    long n = bi==2 ? 0x100000 : B[bi].n;
                    if (!base) { fprintf(stderr,"[mapprobe] %-14s ptr=0 (unallocated)\n", B[bi].nm); continue; }
                    uint8_t *p=(uint8_t*)(uintptr_t)base; int h[256]={0},d=0; long nz=0;
                    for (long i=0;i<n;i++){ if(p[i])nz++; if(!h[p[i]]){h[p[i]]=1;d++;} }
                    fprintf(stderr,"[mapprobe] %-14s ptr=%08x distinct=%d nonzero=%ld/%ld\n",
                            B[bi].nm, base, d, nz, n);
                }
                /* Dump the built 3918 tile and byte-compare vs the oracle reference. */
                uint32_t t3918 = *(uint32_t*)(xb+0x3918);
                if (t3918) {
                    const char *td = getenv("FIST_TILEDUMP");
                    if (td) { FILE *f=fopen(td,"wb"); if(f){ fwrite((void*)(uintptr_t)t3918,1,0x10000,f); fclose(f);
                        fprintf(stderr,"[mapprobe] tile3918 dumped -> %s\n",td); } }
                    const char *tr = getenv("FIST_TILEREF");
                    if (tr) { FILE *f=fopen(tr,"rb"); if(f){ static uint8_t ref[0x10000]; size_t rn=fread(ref,1,0x10000,f); fclose(f);
                        if(rn==0x10000){ uint8_t*t=(uint8_t*)(uintptr_t)t3918; long same=0; int hr[256]={0},ndr=0;
                            for(int i=0;i<0x10000;i++){ if(t[i]==ref[i])same++; if(!hr[ref[i]]){hr[ref[i]]=1;ndr++;} }
                            fprintf(stderr,"[mapprobe] tile3918-vs-ORACLE identical=%ld/65536 (%.1f%%) ref-distinct=%d\n",
                                    same,100.0*same/65536,ndr); } } }
                }
            }
        }
        return 0;
    }
    /* op != 0 = a DISPLAY-LIST / task command (0x04,0x20,0x44,0x64,0x68,0x6c,0x70,0x78,...): the engine
     * has written a command word to the TCB inbox (task+0x3f2 = EBX) and params (task+0x490..0x494), and
     * invokes the extender PM service to CONSUME it.  REVERSED (asm-verified): the command inbox task+0x3f2
     * is written by ~40 poster wrappers (FIST.DAT 0xd94e..0xe37b) but is NEVER READ anywhere in FIST.DAT or
     * either .DVR (full image scan: no read of offset 0x3f2 in any encoding) -- the consumer is the
     * Doug-Huffman extender's 32-bit-PM service, which is NOT present in FIST.DAT and which the oracle
     * cannot reach (the original faults earlier under QEMU).  So there is no engine/driver frame-handler to
     * call here; op 0x78 (present) / 0x64 (frame data) drive the extender's own PM renderer.  e339's tail
     * `ljmp [DGROUP:0x58]` fires only when task[0]!=0 AND op!=0 -- and [0x58]=0xf69:0x314 is the extender
     * ABORT/RESTART handler (`ljmp 0:0xe0`), so task[0] is an ERROR/abort flag, NOT a "started" flag:
     * keeping it 0 (returning without setting it) is the faithful non-error path.  The service returns AX;
     * op 0x78's return gates e584's intro loop (nonzero => animation complete => proceed to menu).  Until
     * the extender PM renderer is acquired (DOSBox-instrumented capture of the op-service, or accept the
     * FMV won't render and model op 0x78 -> complete to reach the reversible main menu), this is a faithful
     * no-op (logged, never silently swallowed) -- it does NOT render, and 0xA0000 stays black for the
     * intro FMV specifically. */
    /* FRAME OPS (0x64 post-frame-data / 0x78 present) -> drive the REAL extender KDV player.  The
     * consumer the earlier notes could not find in FIST.DAT IS this extender code (now linked as the
     * EXT module): on the first frame op OPEN TITLE.KDV (asset name from TCB+0xBA), then DECODE+PRESENT
     * one frame per op -> pixels land in g_mem[0xA0000].  (op 0x78's return gates e584's intro loop;
     * we return 0 = "more frames" -- EOF handling is a follow-on once first light is confirmed.)
     *
     * GATED behind FIST_KDV (default OFF) so the default boot stays crash-free (no regression).  With
     * FIST_KDV=1 execution enters the real player end-to-end and currently stops in the extender's own
     * FILEMGR (FUN_00005cc2/5d50 path resolver) -- the NEXT subsystem: it needs (a) its path-root data
     * (PTR_DAT_00006234 / PTR_s_RESOURCE_RES_0000622c / the DTA at DAT_00000927) initialized, and (b) a
     * 32-bit-FLAT INT-21 file path.  The shared InstallIntFixup reg-file is 16-bit (DS:DX), but the
     * extender is flat: its static path buffers live at module offsets (0x5898 etc.) that fist_dos.c
     * mis-reads as (DS=0x1c00)<<4+DX (engine space), and its heap read buffers are >16-bit host
     * pointers the decompile truncates via `(short)`.  Faithful fix = a 32-bit reg-file kernel
     * re-decompile + an extender INT-21 that addresses g_mem+EDX (flat, [0x807]=0 identity).  See
     * docs/stage1.md.  Until then FIST_KDV is the seam to iterate the player on. */
    /* KDV drive is ON by DEFAULT now (first light is proven) so a normal boot plays the intro then
     * advances to the menu -- matching the original boot->intro->menu flow.  FIST_KDV=0 disables it
     * (escape hatch for A/B'ing the black-FB default).  */
    static int kdv_drive = -1;
    if (kdv_drive < 0) { const char *e = getenv("FIST_KDV"); kdv_drive = (e && e[0] == '0') ? 0 : 1; }
    /* INTRO FRAME-PIN (FIST_KDV_DUMPFRAME=N, default OFF): dump the Nth decoded TITLE.KDV frame for the
     * verify.sh `intro` bit-verify, BYPASSING e584 ENTIRELY.  The display-list SETUP ops
     * (0x20/0x04/0x44/0x68/0x6c/0x70) flow through this gate ONCE, in order, and establish everything the
     * KDV player needs (the asset name "TITLE.KDV" at the intro task +0xBA, the display list) BEFORE e584
     * enters its per-frame PRESENT loop.  op 0x70 is the LAST setup op; by the time it reaches us the setup
     * is complete, so the decoder runs WARM (the "11dd hangs if called standalone BEFORE the setup" hazard
     * only applies when 11dd is driven ahead of these ops -- the backed-out d846564 seam).  We open the
     * stream and stream frames 1..N via a tight 11dd loop (= FUN_0000_6f3e, one KDV chunk per call, no
     * internal wait), dump 0xA0000, and _exit(0) -- WITHOUT ever waiting for an e584-posted op-0x78 present.
     * This is the crux vs the earlier op-0x78-triggered seam: op 0x70 fires BEFORE e584's cooperative-tick
     * per-frame throttle (which crawls ~1 frame/min on the wasm tick and, if fast-forwarded, desyncs the ISR
     * event queue -> e584 aborts the intro), so frame delivery is fully timing-INDEPENDENT.  Pinning by the
     * DECODED-FRAME COUNT off the deterministic KDV stream makes frame N a pure function of consuming N
     * chunks -> native and wasm produce the byte-IDENTICAL frame N (both in <0.3 s).  Frame N is chosen
     * inside the stable end HOLD (fully formed "ARMORED FIST" title, frames ~370..394 identical). */
    if (kdv_drive && op == 0x70 && g_ext_ready && !g_kdv_done) {
        static long df = -2;
        if (df == -2) { const char *e = getenv("FIST_KDV_DUMPFRAME"); df = e ? atol(e) : -1; }
        if (df > 0) {
            g_fist_ext_int = 1;                    /* extender-mode INT 21h (flat FILEMGR) for the player */
            /* OPEN: point the extender current-TCB [0xc93] at a dedicated block and copy the engine-written
             * asset name (intro task +0xBA = "TITLE.KDV") into it (same OPEN as the op-0x78 path below). */
            memcpy(g_ext_kdv_tcb + 0xBA, g_mem + 0x90000 + 0xBA, 16);
            *(uint32_t *)(g_mem + FIST_EXT_BASE + 0xc93) = (uint32_t)(uintptr_t)g_ext_kdv_tcb;
            fprintf(stderr, "[ext] KDV_DUMPFRAME setup-drive: OPEN (asset '%.13s')\n", g_ext_kdv_tcb + 0xBA);
            m_ext_FUN_0000_11cb(0, 0, 0, 0, 0);
            g_kdv_open = 1;
            while (g_kdv_frames < df && !g_ext_eof) {
                m_ext_FUN_0000_11dd(0, 0, 0);      /* decode + blit the next KDV frame */
                if (g_ext_eof) break;
                ++g_kdv_frames;
            }
            fprintf(stderr, "[ext] KDV_DUMPFRAME setup-drive: presented frame %ld (target %ld eof=%d) -- dumping + exiting\n",
                    g_kdv_frames, df, g_ext_eof);
            const char *fb = getenv("FIST_FBDUMP");
            if (fb) fist_dump_framebuffer(fb);
            _exit(0);
        }
    }
    if (kdv_drive && (op == 0x64 || op == 0x78) && g_ext_ready) {
        /* Intro already finished: keep signalling "animation complete" so e584 stays out of its loop
         * (the present op 0x78 return gates e584; nonzero => proceed to the menu). */
        if (g_kdv_done) return (op == 0x78) ? 1 : 0;
        g_fist_ext_int = 1;                    /* extender-mode INT 21h (flat FILEMGR) for the player */
        if (!g_kdv_open) {
            /* Point the extender current-TCB [0xc93] at a DEDICATED control block (not the engine's
             * 16-bit intro task) and copy the engine-written asset name (intro task +0xBA) into it.
             * Faithful: in the original the extender's current-TCB is a separate block from the engine
             * intro task.  This matters because the player's MEMMGR error handler FUN_0000_0f64 does
             * `*[0xc93] = 0xffff` -- with [0xc93] aliased to the engine intro task that would poison
             * the task[0] abort flag e339 checks (spurious ljmp to the extender abort at 0xf9a4). */
            memcpy(g_ext_kdv_tcb + 0xBA, g_mem + 0x90000 + 0xBA, 16);
            *(uint32_t *)(g_mem + FIST_EXT_BASE + 0xc93) = (uint32_t)(uintptr_t)g_ext_kdv_tcb;
            fprintf(stderr, "[ext] KDV OPEN (asset '%.13s' via extender TCB+0xBA)\n", g_ext_kdv_tcb + 0xBA);
            m_ext_FUN_0000_11cb(0, 0, 0, 0, 0);
            g_kdv_open = 1;
        }
        /* FIST_KDV_DUMPFRAME=N (diagnostic, default OFF): pin ONE settled intro frame by FRAME INDEX for
         * the verify.sh `intro` bit-verify.  The engine's e584 intro loop posts op-0x78 (PRESENT) frame by
         * frame; the FIRST op-0x78 below OPENs TITLE.KDV and decodes+blits frame 1 via m_ext_FUN_0000_11dd
         * (= FUN_0000_6f3e, one KDV chunk per call, no internal wait), driven the NORMAL way through e584 so
         * the decoder state is fully warmed.  From frame 1 the op-0x78 branch then DRIVES THE REMAINING
         * PRESENTS DIRECTLY (a tight 11dd loop, no e584 round-trip) to frame N, dumps 0xA0000 (FIST_FBDUMP)
         * and exits.  Pinning by the DECODED-FRAME COUNT off a deterministic stream makes the capture
         * timing-INDEPENDENT -> native and wasm produce the IDENTICAL frame N.
         *   WHY DIRECT-DRIVE (not "let e584 pace all 395"):  on the cooperative wasm tick e584's per-frame
         * INT-8 throttle crawls (~1 frame/min), and any attempt to fast-forward it (shrink b6e0 / bump
         * c452) DESYNCS the ISR-fed event queue from the tick counter so e584 misreads a lingering queue
         * event as a skip-keypress and aborts the intro after ~1 frame (native masks this via SIGALRM
         * running the ISR at rate; wasm cannot).  Driving 11dd directly bypasses the throttle AND e584's
         * abort logic entirely -- the decode is a pure function of the chunk stream, identical on both.
         *   WHY IT IS NOT THE PRIOR HUNG SEAM:  the backed-out d846564 seam called 11dd standalone from the
         * OPEN block BEFORE any e584 present had set up the decode -> its first call blocked.  Here the
         * first present (frame 1) runs through e584 first, so every 11dd we call is warm.
         *   frame N chosen inside the intro's stable end HOLD (fully formed "ARMORED FIST" title, frames
         * ~370..394 identical).  Native reaches frame 385 in ~1.2 s; wasm well within the 120 s budget. */
        static long g_kdv_dumpframe = -2;
        if (g_kdv_dumpframe == -2) { const char *df = getenv("FIST_KDV_DUMPFRAME"); g_kdv_dumpframe = df ? atol(df) : -1; }
        if (op == 0x78) {                      /* present: decode + blit the next frame */
            m_ext_FUN_0000_11dd(0, 0, 0);
            /* patch-084 threads 708b's CF-out (EOF / read-error) here: g_ext_eof=1 => the last chunk
             * was already consumed (offset+8 > filesize), i.e. TITLE.KDV is done.  Faithful intro-end:
             * close the stream and return NONZERO (animation complete) so e584 exits its frame loop and
             * the engine builds the MAIN MENU.  The real game plays the intro ONCE then advances (op
             * 0x78 returns nonzero at end-of-animation, per the reversed e584 contract) -- no loop. */
            if (g_ext_eof) {
                fprintf(stderr, "[ext] KDV EOF detected @frame %ld -- closing stream\n", g_kdv_frames);
                if (!getenv("FIST_KDV_NOCLOSE")) m_ext_FUN_0000_6f17(0, 0, 0, 0, 0, 0);   /* close/free the KDV stream */
                fprintf(stderr, "[ext] KDV close done\n");
                g_kdv_done = 1;
                g_fist_ext_int = 0;
                { const char *ed = getenv("FIST_DUMP_INTROEND"); if (ed) fist_dump_framebuffer(ed); }
                /* DIAGNOSTIC (FIST_MENUBG_TEST): prove the menu-bg render chain end-to-end by injecting
                 * the REAL MAINMENU.MRL bytes into a free segment and pointing the on-screen descriptor
                 * DGROUP:0xf6cc.word0 (linear 0x2b6cc) at it.  NOT a fix -- the faithful path is the
                 * engine opening MAINMENU.MRL itself; this only confirms 0340 paints the real scene. */
                if (getenv("FIST_MENUBG_TEST")) {
                    FILE *mf = fopen("armoredfist/FISTDATA/MAINMENU.MRL", "rb");
                    if (mf) {
                        uint16_t mseg = 0x8000;               /* free: below PSP 0x9800 / TCB 0x90000 */
                        size_t n = fread(g_mem + ((uint32_t)mseg << 4), 1, 0x10000, mf);
                        fclose(mf);
                        *(uint16_t *)(g_mem + 0x2b6cc) = mseg; /* descriptor.word0 = MRL resource segment */
                        fprintf(stderr, "[diag] MAINMENU.MRL (%zu B) -> seg 0x%04x ; descriptor[0xf6cc].word0=0x%04x\n",
                                n, mseg, mseg);
                        if (getenv("FIST_MENUBG_PAL")) {   /* also push the .MRL palette to the DAC (visual confirm) */
                            extern void out(int, int);
                            uint8_t *p = g_mem + ((uint32_t)mseg << 4);
                            out(0x3c8, 0);
                            for (int i = 0; i < 768; i++) out(0x3c9, p[i]);
                            fprintf(stderr, "[diag] uploaded MAINMENU.MRL palette to DAC\n");
                        }
                    }
                }
                fprintf(stderr, "[ext] KDV intro complete: %ld frames decoded -> animation done, "
                                "proceeding to main menu\n", g_kdv_frames);
                g_fbtrap_req = 1;              /* arm the fb write-trap for the menu render */
                return 1;                      /* animation complete */
            }
            ++g_kdv_frames;
            /* FIST_KDV_DUMPFRAME frame-pin: the FB now holds decoded frame g_kdv_frames (frame 1 here --
             * the FIRST present, driven the NORMAL way through e584 so the decoder is fully warmed).  Now
             * drive the REMAINING presents DIRECTLY -- 11dd = FUN_0000_6f3e decodes exactly one KDV chunk
             * per call and returns (no internal wait), so calling it back-to-back streams frames 2..N with
             * NO e584 round-trip.  This SIDESTEPS the two wasm blockers of the e584 path: (a) the per-frame
             * INT-8 throttle crawl (~1 frame/min on the cooperative tick), and (b) e584 aborting the intro
             * after a couple frames when a lingering queue event is misread as a skip-keypress (the desync
             * that broke the b6e0-shrink / c452-bump tries).  It is NOT the prior agent's hung seam: that
             * called 11dd standalone from the OPEN block BEFORE any present set up the decode state -> its
             * first call blocked; here the first present (frame 1) has already run through e584, so 11dd is
             * warm.  Frame-COUNT gated + a deterministic stream => native and wasm dump the IDENTICAL frame
             * N (timing-independent).  Only reached with FIST_KDV_DUMPFRAME set (default OFF). */
            if (g_kdv_dumpframe > 0) {
                while (g_kdv_frames < g_kdv_dumpframe && !g_ext_eof) {
                    m_ext_FUN_0000_11dd(0, 0, 0);   /* decode + present the next KDV frame */
                    if (g_ext_eof) break;
                    ++g_kdv_frames;
                }
                fprintf(stderr, "[ext] KDV_DUMPFRAME: presented frame %ld (target %ld eof=%d) -- dumping + exiting\n",
                        g_kdv_frames, g_kdv_dumpframe, g_ext_eof);
                const char *fb = getenv("FIST_FBDUMP");
                if (fb) fist_dump_framebuffer(fb);
                _exit(0);
            }
            if (getenv("FIST_KDV_TRACE")) {
                uint8_t *eb = g_mem + FIST_EXT_BASE;
                fprintf(stderr, "[ext] KDV frame %ld eof=%d off=%u size=%u fcnt=%u w=%u h=%u\n",
                    g_kdv_frames, g_ext_eof,
                    *(uint32_t*)(eb+0x5570), *(uint32_t*)(eb+0x937), *(uint32_t*)(eb+0x6e84),
                    *(uint32_t*)(eb+0x5578), *(uint32_t*)(eb+0x557c));
            }
        }
        g_fist_ext_int = 0;
        return 0;
    }
    /* ------------------------------------------------------------------------------------------------
     * THE PER-FRAME 3D VOXEL RENDER IS EXTENDER-SIDE (located this iteration; RECON in docs/stage1.md).
     * op 0x0c -> op-table[fist_image.bin:0xcb3 + op] = dword 0x10eb -> trampoline `call 0x78f0`.
     * FUN_0000_78f0 sets fb ptr 90ec = TCB[4]+TCB[2]*16+[0x807], viewport 90f0/90f4/90f8 = TCB[0x1e]/[0x22],
     * then calls 85d0 (camera state from TCB 0x2c/0x30/0x34/0x38/0x3e) + 93c0 (perspective texture-map of
     * colormap DAT_85b8 into 90ec).  Sibling render ops: op 0x10 -> 7940->8fa0 (rotated variant),
     * op 0x08 -> 8df0/3931, op 0x44 -> 7660 (sky).  This is the NovaLogic Voxel Space engine living in the
     * Doug-Huffman extender image, exactly analogous to the KDV FMV player -- the engine (FIST.DAT) only
     * POSTS display objects (op 0x54 roster) + camera/present ops; it does NOT do the 3D projection.
     *
     * FIST_R3D (default OFF -> default boot + the 19 verify flows are byte-untouched): a DIAGNOSTIC seam
     * that drives the located render for op 0x0c.  Replicates 78f0 but (a) points [0xc93] at the mission
     * TCB (ea2e:ea2c), (b) forces 90ec = host &g_mem[0xA0000] (78f0's TCB[2]:[4] real-mode linear is not
     * a host ptr in this model), (c) sources the viewport from TCB+0x1e/+0x22, forcing a bounded 320x200
     * when 0 (the upstream viewport-dimension setup is not yet reconstructed -- a SCAFFOLD to exercise the
     * pipeline).  RESULT (runtime-verified): the pipeline runs CRASH-FREE and fills all 64000 fb bytes,
     * BUT with a single color -- because the colormap at [0x85b8] holds only {0,4} across its whole 4 MB
     * (dwords==4, not terrain texels): the C32.KLC colormap DATA is not yet delivered by the extender
     * map-load (89b0/643c) even though the LOD build completes structurally (detail 0xb / dim 2048).  So
     * this is NOT yet recognizable first pixels -- the true remaining gate is the colormap data path (and
     * the faithful viewport-dimension source), UPSTREAM of this now-validated render.  NOT accepted; kept
     * gated as the next-iteration seam. */
    /* FIST_ENGFB (diagnostic, default OFF): dump the ENGINE-side framebuffer (0xA0000) at the FIRST
     * op-0x0c gate -- captured BEFORE any FIST_R3D extender-render seam runs, so it is purely whatever the
     * engine's own mission render pass (459a->22dd->2322 voxel column engine) wrote.  The RUNMS/DUMPTICK
     * watchdogs cannot fire in-mission (the modal loop never re-enters fist_timer_pump), so this gate is
     * the only in-mission capture point.  Shim-only; does not touch the engine or the 19 verify flows. */
    if (op == 0x0c && g_ext_ready) {
        const char *ef = getenv("FIST_ENGFB");
        if (ef) { static int en = 0; if (en++ == 0) { fist_dump_framebuffer(ef);
            const char *er = getenv("FIST_ENGFB_RAW");
            if (er) { FILE *f = fopen(er, "wb"); if (f) { fwrite(g_mem + 0xA0000, 1, 64000, f); fclose(f); } } } }
    }
    /* FIST_R3D2 (diagnostic): the OPAQUE-LOD voxel terrain render (oracle: FUN_0000_9200 via 82b8/82c0,
     * the DOMINANT fb writer in the ORIGINAL).  Drives 8deb (viewport setup) -> 85d0 (camera) ->
     * 82b8 (8120 projection -> 9200 texel walk -> 82d0 teardown), TCB from the mission ea2e:ea2c.
     * Dumps the colormap[0x3918] + horizon[0x9114] contents to answer whether the DATA is delivered. */
    if (op == 0x0c && g_ext_ready && getenv("FIST_R3D2")) {
        static int r2 = 0;
        uint32_t tcb_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        uint8_t *tcb = g_mem + tcb_lin;
        uint8_t *xb  = g_mem + FIST_EXT_BASE;
        uint32_t save_c93 = *(uint32_t*)(xb+0xc93);
        *(uint32_t*)(xb+0xc93) = (uint32_t)(uintptr_t)tcb;
        if (r2 == 0) fprintf(stderr,"[r3d2] engine-cam 4e1c=%d 4e20=%d 4e24=%d 4e34=%d | d548struct[12/16/1a]=%d/%d/%d | 7b1c(head)=%u 7b18(foc)=%u 4d0e=%d\n",
            *(int32_t*)(g_mem+0x24e1c),*(int32_t*)(g_mem+0x24e20),*(int32_t*)(g_mem+0x24e24),*(int32_t*)(g_mem+0x24e34),
            *(int32_t*)(g_mem+0x1d55a),*(int32_t*)(g_mem+0x1d55e),*(int32_t*)(g_mem+0x1d562),
            *(uint16_t*)(g_mem+0x23b1c),*(uint16_t*)(g_mem+0x23b18),*(int16_t*)(g_mem+0x24d0e));
        if (r2 == 0 && getenv("FIST_R3D2_PROBE")) {
            fprintf(stderr, "[r3d2] TCB @0x%05x rect[16/18/1a/1c]=%d/%d/%d/%d detail[cd]=%u [cf]=%u "
                    "cam[2c/30/34]=%d/%d/%u ang[38/3a/3c/3e]=%u/%d/%d/%u\n",
                    tcb_lin, *(int16_t*)(tcb+0x16),*(int16_t*)(tcb+0x18),*(int16_t*)(tcb+0x1a),*(int16_t*)(tcb+0x1c),
                    tcb[0xcd], tcb[0xcf], *(int32_t*)(tcb+0x2c),*(int32_t*)(tcb+0x30),*(uint32_t*)(tcb+0x34),
                    *(uint16_t*)(tcb+0x38),*(int16_t*)(tcb+0x3a),*(int16_t*)(tcb+0x3c),*(uint16_t*)(tcb+0x3e));
            /* viewport setup (8deb: 90a8 dest, 90f0 colh, 90f8 vieww, 90ac adv, 9114 horizon table) */
            m_ext_FUN_0000_8deb();
            m_ext_FUN_0000_85d0();
            uint32_t cm  = *(uint32_t*)(xb+0x3918);   /* colormap base (host ptr in Route-1 slot) */
            uint32_t hz  = *(uint32_t*)(xb+0x9114);   /* horizon table base (host ptr) */
            uint32_t dst = *(uint32_t*)(xb+0x90a8);   /* dest (host ptr = fb + off) */
            fprintf(stderr, "[r3d2] after setup: 90a8(dst)=%08x 90f0(colh)=%u 90f8(vieww)=%u 90ac(adv)=%u "
                    "9114(hz)=%08x 3918(cm)=%08x 85b8=%08x\n",
                    dst, *(uint32_t*)(xb+0x90f0), *(uint32_t*)(xb+0x90f8), *(uint32_t*)(xb+0x90ac),
                    hz, cm, *(uint32_t*)(xb+0x85b8));
            /* colormap content histogram (host ptr; the sampler indexes only the first 64KB = 256x256) */
            if (cm > 0x1000 && cm < 0xffffffff) {
                uint8_t *cmp = (uint8_t*)(uintptr_t)cm; int h[256]={0}; int nd=0; long nz64=0;
                for (long i=0;i<0x10000;i++){ if(cmp[i])nz64++; if(!h[cmp[i]])nd++; h[cmp[i]]++; }
                long nz4m=0; for(long i=0;i<0x400000;i+=17) if(cmp[i]) nz4m++;
                long firstnz=-1; for(long i=0;i<0x400000;i++) if(cmp[i]){firstnz=i;break;}
                fprintf(stderr, "[r3d2] colormap[0x3918] first64KB nonzero=%ld/65536 distinct=%d ; 4MB-sampled-nonzero=%ld/%d first-nonzero-off=0x%lx (=%ld) 391c(size)=%u 38ed=%u 38f1=%u\n",
                        nz64, nd, nz4m, 0x400000/17, firstnz, firstnz,
                        *(uint32_t*)(xb+0x391c), *(uint32_t*)(xb+0x38ed), *(uint32_t*)(xb+0x38f1));
            }
            if (hz > 0x1000 && hz < 0x100000) {   /* flat module offset, not a host ptr */
                uint8_t *hzp = xb + hz;
                fprintf(stderr, "[r3d2] horizon[0x9114]=+0x%x first20 cols=", hz);
                for(int i=0;i<20;i++) fprintf(stderr,"%u ",hzp[i]);
                fprintf(stderr,"\n");
            }
            /* FIST_R3D2 tile-builder (6980) upstream-state probe: are the loop bounds / ray tables /
             * LUT / map planes populated so the real builder CAN run? */
            {
              uint32_t b85bc=*(uint32_t*)(xb+0x85bc), b3909=*(uint32_t*)(xb+0x3909), b390d=*(uint32_t*)(xb+0x390d);
              fprintf(stderr,"[r3d2] BUILDER-STATE 3901=%u 3905=%u 38fd=%u 3a20=%u 38ed=%u 38f1=%u | 85bc=%08x 3909=%08x 390d=%08x 395d(clr)=%u\n",
                *(uint32_t*)(xb+0x3901), *(uint32_t*)(xb+0x3905), *(uint16_t*)(xb+0x38fd), *(uint32_t*)(xb+0x3a20),
                *(uint32_t*)(xb+0x38ed), *(uint32_t*)(xb+0x38f1), b85bc, b3909, b390d, xb[0x395d]);
              fprintf(stderr,"[r3d2] ray4224:"); for(int i=0;i<6;i++) fprintf(stderr," %d",*(int32_t*)(xb+0x4224+i*4));
              fprintf(stderr," | ray4624:"); for(int i=0;i<6;i++) fprintf(stderr," %d",*(int32_t*)(xb+0x4624+i*4));
              if (b85bc>0x1000 && b85bc<0xffffffff){ uint8_t*mp=(uint8_t*)(uintptr_t)b85bc;
                long hnz=0,cnz=0; for(long i=0;i<0x40000;i++){ if(mp[i])hnz++; if(mp[0x100000+i])cnz++; }
                fprintf(stderr," | mapHEIGHT[85bc]nz/256k=%ld mapCOLOR[+1M]nz/256k=%ld",hnz,cnz); }
              fprintf(stderr,"\n");
            }
        }
        if (getenv("FIST_R3D2_RENDER")) {
            /* Viewport dims: the engine's ddff viewport-configure (writes TCB+0x16..0x1c) is a separate
             * unreconstructed frontier, so the TCB rect is 0 and 8deb yields colh/vieww=0.  Force the
             * ORACLE-captured cockpit-windshield terrain viewport (docs/oracle_terrain_writer.md): 81
             * columns x 288 col-height, dest = fb+0x650, per-column advance 32 -> reproduces the oracle
             * write span [0x0656..0x6b66].  (FIST_R3D2_VW/VH/DST override for experimentation.) */
            m_ext_FUN_0000_8deb();        /* selects horizon table 9114 from TCB[0xcd] detail */
            m_ext_FUN_0000_85d0();        /* camera position/angle from TCB */
            /* FIST_R3D2_BUILD: drive the REAL colormap-tile BUILDER FUN_0000_6980 (op 0x08 -> 3931 ->
             * 6980), which the port never posts.  It voxel-rasterizes the map (height plane at [0x85bc],
             * color plane at +0x100000) through the camera into the 256x256 tile at DAT_3918 that 9200
             * samples.  Reconstructed (build/fist_ext.c) with the patch-286-class base-losses fixed. */
            if (getenv("FIST_R3D2_BUILD")) {
                /* CAMERA OVERRIDE experiment: the intro TCB (0x90000) has a degenerate camera
                 * (height 0, heading 0) -> the voxel walk collapses to a few bands.  Inject a
                 * plausible altitude/heading to test whether the builder produces coherent
                 * multi-colour terrain with a valid camera (proves the builder, isolates the
                 * camera-source frontier). */
                { uint8_t *tcb2 = (uint8_t*)(uintptr_t)*(uint32_t*)(xb+0xc93);
                  const char *cz=getenv("FIST_R3D2_CAMZ"), *ca=getenv("FIST_R3D2_ANG");
                  if (cz) *(int32_t*)(tcb2+0x34) = (int32_t)strtol(cz,0,0);
                  if (ca) *(uint16_t*)(tcb2+0x38) = (uint16_t)strtoul(ca,0,0);
                  /* FULL oracle-camera injection (validation scaffold): the live AZER1 camera
                   * captured from the DOSBox RAM dump = X=609696 Y=1112229 alt=29184 head=19745
                   * roll=128 foc=256 detail=0.  Fields: TCB+0x2c(X) 0x30(Y) 0x34(alt) 0x38(head)
                   * 0x3a(pitch) 0x3c(roll) 0x3e(foc) 0xcf(detail). */
                  const char *cx=getenv("FIST_R3D2_CAMX"), *cy=getenv("FIST_R3D2_CAMY"),
                             *cp=getenv("FIST_R3D2_PITCH"), *crl=getenv("FIST_R3D2_ROLL"),
                             *cf=getenv("FIST_R3D2_FOC"), *cdt=getenv("FIST_R3D2_DETAIL");
                  if (getenv("FIST_R3D2_ORACLE")) {
                      *(int32_t*)(tcb2+0x2c)=609696; *(int32_t*)(tcb2+0x30)=1112229;
                      *(int32_t*)(tcb2+0x34)=29184; *(uint16_t*)(tcb2+0x38)=19745;
                      *(uint16_t*)(tcb2+0x3a)=0; *(uint16_t*)(tcb2+0x3c)=128;
                      *(uint16_t*)(tcb2+0x3e)=256; *(uint8_t*)(tcb2+0xcf)=0;
                  }
                  if (cx) *(int32_t*)(tcb2+0x2c)=(int32_t)strtol(cx,0,0);
                  if (cy) *(int32_t*)(tcb2+0x30)=(int32_t)strtol(cy,0,0);
                  if (cp) *(uint16_t*)(tcb2+0x3a)=(uint16_t)strtoul(cp,0,0);
                  if (crl)*(uint16_t*)(tcb2+0x3c)=(uint16_t)strtoul(crl,0,0);
                  if (cf) *(uint16_t*)(tcb2+0x3e)=(uint16_t)strtoul(cf,0,0);
                  if (cdt)*(uint8_t*)(tcb2+0xcf)=(uint8_t)strtoul(cdt,0,0);
                  m_ext_FUN_0000_85d0(); /* re-read camera after override */ }
                /* RAY-TABLE injection (validation): 0x3a24/0x3e24 are the detail-level base ray-angle
                 * tables built at map/detail-init by a builder the port doesn't yet drive (the image
                 * ships them as placeholder 1s -> 395e divides 0xffffffff/uVar2 with uVar2=0 -> SIGFPE).
                 * They are camera-INDEPENDENT.  Inject the oracle-captured tables to validate the
                 * camera+builder+renderer, isolating the ray-builder as a separate frontier. */
                { const char *r3=getenv("FIST_R3D2_RAY3A24"), *r7=getenv("FIST_R3D2_RAY3E24");
                  if (r3){ FILE*f=fopen(r3,"rb"); if(f){ fread(xb+0x3a24,1,256*4,f); fclose(f);} }
                  if (r7){ FILE*f=fopen(r7,"rb"); if(f){ fread(xb+0x3e24,1,256*4,f); fclose(f);} }
                  /* force 395e to rebuild the 4224/4624 LUT from the injected source */
                  if (r3||r7) *(uint32_t*)(xb+0x90c4) = ~*(uint32_t*)(xb+0x90c0); }
                extern void m_ext_FUN_0000_6980(void);
                m_ext_FUN_0000_6980();
                uint32_t cmb = *(uint32_t*)(xb+0x3918);
                if (r2==0 && cmb>0x1000) { uint8_t*t=(uint8_t*)(uintptr_t)cmb; long nz=0; int h[256]={0},nd=0;
                    for(int i=0;i<65536;i++){ if(t[i])nz++; if(!h[t[i]]){h[t[i]]=1;nd++;} }
                    fprintf(stderr,"[r3d2] AFTER-BUILD tile3918 nz=%ld/65536 distinct=%d (first row: ",nz,nd);
                    for(int i=0;i<16;i++) fprintf(stderr,"%02x ",t[i]); fprintf(stderr,")\n");
                    /* tile texel histogram (which indices) */
                    fprintf(stderr,"[r3d2] tile indices:"); for(int v=0;v<256;v++) if(h[v]) fprintf(stderr," %02x(%d)",v,h[v]); fprintf(stderr,"\n");
                    /* color-plane vs 85b8 distinctness */
                    uint32_t b85bc=*(uint32_t*)(xb+0x85bc), b85b8=*(uint32_t*)(xb+0x85b8);
                    { uint8_t*cp=(uint8_t*)(uintptr_t)(b85bc+0x100000); int hc[256]={0},ndc=0;
                      for(int i=0;i<0x40000;i++) if(!hc[cp[i]]){hc[cp[i]]=1;ndc++;}
                      uint8_t*c8=(uint8_t*)(uintptr_t)b85b8; int h8[256]={0},nd8=0;
                      for(int i=0;i<0x40000;i++) if(!h8[c8[i]]){h8[c8[i]]=1;nd8++;}
                      fprintf(stderr,"[r3d2] colorPlane[85bc+1M] distinct/256k=%d ; 85b8 distinct/256k=%d\n",ndc,nd8); }
                    /* first terrain column: sample tile row 6 (oracle first index expect 0xdc) */
                    fprintf(stderr,"[r3d2] tile col0 depth-run rows: "); for(int r=0;r<20;r++) fprintf(stderr,"%02x ",t[r*256]); fprintf(stderr,"\n");
                    /* compare against a reference tile (oracle 0x44200 dump) if provided */
                    const char *tr=getenv("FIST_R3D2_TILEREF");
                    if (tr) { FILE*rf=fopen(tr,"rb"); if(rf){ static uint8_t ref[0x10000]; size_t n=fread(ref,1,0x10000,rf); fclose(rf);
                        if(n==0x10000){ long same=0; for(int i=0;i<0x10000;i++) if(t[i]==ref[i]) same++;
                            fprintf(stderr,"[r3d2] TILE-vs-ORACLE: identical bytes=%ld/65536 (%.1f%%)  ref[0:8]=",same,100.0*same/65536);
                            for(int i=0;i<8;i++) fprintf(stderr,"%02x ",ref[i]); fprintf(stderr,"\n"); } } } }
            }
            unsigned vw = getenv("FIST_R3D2_VW") ? (unsigned)strtoul(getenv("FIST_R3D2_VW"),0,0) : 81;
            unsigned ch = getenv("FIST_R3D2_VH") ? (unsigned)strtoul(getenv("FIST_R3D2_VH"),0,0) : 288;
            unsigned dst= getenv("FIST_R3D2_DST")? (unsigned)strtoul(getenv("FIST_R3D2_DST"),0,0): 0x650;
            *(uint32_t*)(xb+0x90f8) = vw;                                         /* view width (columns) */
            *(uint32_t*)(xb+0x90f0) = ch;                                         /* column height */
            *(uint32_t*)(xb+0x90ac) = 0x140 - ch;                                /* dest advance */
            *(uint32_t*)(xb+0x90a8) = (uint32_t)(uintptr_t)(g_mem + 0xA0000 + dst);/* dest ptr */
            m_ext_FUN_0000_8120();        /* projection: 90d4/90d8/90b8/90bc from camera */
            /* 9200's params are the per-TEXEL u/v steps 8120 leaves in registers esi/ebp
             * (asm 0x8247 `mov esi,edx`=high(90c0*9104); 0x8251 `mov ebp,edx`=high(90c0*9108)).
             * The __allregs C model returns void from 8120, so recompute them from the stored
             * 90c0/9104/9108 and pass (param_1=ebp, param_2=esi) as 9200 expects. */
            { int32_t c0 = *(int32_t*)(xb+0x90c0), v04 = *(int32_t*)(xb+0x9104), v08 = *(int32_t*)(xb+0x9108);
              int32_t esi = (int32_t)(((int64_t)c0 * v04) >> 32);
              int32_t ebp = (int32_t)(((int64_t)c0 * v08) >> 32);
              m_ext_FUN_0000_9200(ebp, esi); }
            if (r2 == 0) {
                uint8_t *fb = g_mem+0xA0000; long nz=0; int h[256]={0}; int nd=0;
                for(long i=0;i<64000;i++){ if(fb[i]){nz++;} if(!h[fb[i]])nd++; h[fb[i]]++; }
                uint32_t u7=*(uint32_t*)(xb+0x90d4), u5=*(uint32_t*)(xb+0x90d8);
                uint32_t tc0 = ((u5>>0x18)<<8) | (u7>>0x18);
                uint8_t *cmp = (uint8_t*)(uintptr_t)*(uint32_t*)(xb+0x3918);
                fprintf(stderr,"[r3d2] RENDER vw=%u ch=%u -> fb nonzero=%ld/64000 distinct=%d 90d4=%08x 90d8=%08x 90b8=%08x 90bc=%08x tc0=%04x cm[tc0]=%02x\n",
                        vw, ch, nz, nd, u7,u5,*(uint32_t*)(xb+0x90b8),*(uint32_t*)(xb+0x90bc), tc0, cmp[tc0&0xffff]);
                /* histogram of fb rows 5..86 (the rendered voxel viewport = 9200's output only) */
                { int vh[256]={0}; int vnd=0; long vnz=0;
                  for(long r=5;r<86;r++) for(long c=0;c<320;c++){ uint8_t p=fb[r*320+c]; if(p)vnz++; if(!vh[p])vnd++; vh[p]++; }
                  fprintf(stderr,"[r3d2] viewport rows5-86: nonzero=%ld/%d distinct=%d\n", vnz, 81*320, vnd); }
                { extern void out(int,int); uint8_t *mp = xb + 0x5598;
                  out(0x3c8,0); for(int pi=0;pi<768;pi++) out(0x3c9,mp[pi]); }
                const char *dp = getenv("FIST_R3D2_DUMP"); if (dp) fist_dump_framebuffer(dp);
            }
        }
        *(uint32_t*)(xb+0xc93) = save_c93;
        r2++;
        return 0;
    }
    if (op == 0x0c && g_ext_ready && getenv("FIST_R3D")) {
        uint32_t tcb_lin = ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c);
        uint8_t *tcb = g_mem + tcb_lin;
        uint8_t *xb  = g_mem + FIST_EXT_BASE;
        uint32_t save_c93 = *(uint32_t*)(xb+0xc93);
        uint16_t vw = *(uint16_t*)(tcb+0x1e), vh = *(uint16_t*)(tcb+0x22);
        if (vw == 0) vw = 320;  if (vw > 320) vw = 320;
        if (vh == 0) vh = 200;  if (vh > 200) vh = 200;
        *(uint32_t*)(xb+0xc93) = (uint32_t)(uintptr_t)tcb;                 /* mission camera TCB */
        *(uint32_t*)(xb+0x90ec) = (uint32_t)(uintptr_t)(g_mem + 0xA0000); /* VGA framebuffer */
        *(uint32_t*)(xb+0x90f0) = vw;
        *(uint32_t*)(xb+0x90f4) = vw >> 1;
        *(uint32_t*)(xb+0x90f8) = vh;
        m_ext_FUN_0000_85d0();                                            /* camera setup from TCB */
        m_ext_FUN_0000_93c0();                                            /* colormap texture-map -> fb */
        /* MISSION DAC PALETTE UPLOAD.  The colormap-remap LUT (ext 9e60/9ec0) reduces the C32.KLC
         * colormap texels to nearest indices in the MISSION VGA palette at ext+0x5598 -- the raw 6-bit
         * 256x3 palette loaded from 532.pal by the op-0x18 map-load (FUN_0000_6032).  So the framebuffer
         * bytes that 93c0 writes are indices into 5598; the DAC must hold 5598 for them to become visible
         * colours.  (The engine's own retrace-upload buffer word[DGROUP:0x782] carries a grayscale ramp
         * here, NOT the mission palette -- the mission palette lives extender-side.)  Install it to the
         * VGA DAC via the real port path (mirrors the extender KDV uploader 746b, sourced from 5598;
         * 532.pal is already 6-bit so no >>2).  Idempotent, once per rendered frame. */
        { extern void out(int,int); uint8_t *mp = xb + 0x5598;
          out(0x3c8, 0); for (int pi = 0; pi < 768; pi++) out(0x3c9, mp[pi]); }
        *(uint32_t*)(xb+0xc93) = save_c93;
        if (getenv("FIST_R3D_PROBE")) {
            static int rn = 0;
            if (rn++ < 4) {
                uint8_t *fb = g_mem + 0xA0000; long nz = 0; int hist[256] = {0};
                for (long i = 0; i < 64000; i++) { if (fb[i]) nz++; hist[fb[i]]++; }
                int colors = 0; for (int c = 0; c < 256; c++) if (hist[c]) colors++;
                uint8_t *cm = (uint8_t*)(uintptr_t)*(uint32_t*)(xb+0x85b8);
                int cmhist[256] = {0}; for (long i = 0; i < 0x400000; i += 37) cmhist[cm[i]]++;
                int cmcolors = 0; for (int c = 0; c < 256; c++) if (cmhist[c]) cmcolors++;
                fprintf(stderr, "[r3d] op0x0c RENDER vw=%u vh=%u cam2c=%08x cam3e=%04x 85b8=%08x -> fb nonzero=%ld/64000 fb-colors=%d ; colormap distinct-colors(4MB)=%d\n",
                        vw, vh, *(uint32_t*)(tcb+0x2c), *(uint16_t*)(tcb+0x3e), *(uint32_t*)(xb+0x85b8), nz, colors, cmcolors);
                /* FIST_R3D_DUMP: dump the rendered mission framebuffer NOW (the RUNMS watchdog cannot
                 * fire post-render because the mission loop does not re-enter fist_timer_pump).  For the
                 * AE-vs-oracle baseline measurement. */
                { const char *dp = getenv("FIST_R3D_DUMP"); if (dp && rn==1) fist_dump_framebuffer(dp); }
                if (getenv("FIST_R3D_PROBE") && rn==1) {
                    /* VIEWPORT-DIM FRONTIER (frontier 2, docs/stage1.md).  The engine's viewport-configure
                     * FUN_0000_ddff writes TCB+0x1e(width)/+0x22(height) from the rect at word[DGROUP:0x156a]
                     * (=DAT_1000_d56a) fields WORD [+6/+8/+a/+c] (asm 0xde0b.., objdump-verified).  Ghidra
                     * typed d56a int** -> the reads int-scaled + base-lost.  BUT the deeper root is upstream:
                     * this rect's viewport sub-rect fields are NEVER populated (all 0 below), and ddff is a
                     * method-vector target not observed running in-mission -> TCB dims stay 0, so the seam
                     * forces 320x200.  Reported for the next iteration. */
                    uint16_t rp = *(uint16_t*)(g_mem+0x1d56a);
                    uint8_t *rr = g_mem+0x1c000+rp;
                    fprintf(stderr, "[vp] d56a=%04x rect[0,x1,y1,x2,y2]=%d,%d,%d,%d,%d ; TCB+0x1e(w)=%u +0x22(h)=%u (0 => seam forces 320x200)\n",
                        rp, *(int16_t*)(rr+0), *(int16_t*)(rr+6), *(int16_t*)(rr+8),
                        *(int16_t*)(rr+0xa), *(int16_t*)(rr+0xc),
                        *(uint16_t*)(tcb+0x1e), *(uint16_t*)(tcb+0x22));
                }
            }
        }
        return 0;
    }
    { static int extlog = -1; if (extlog < 0) extlog = getenv("FIST_EXTLOG") ? 1 : 0;
      if (extlog)
        fprintf(stderr, "[ext] service op 0x%02x (display-list cmd) inbox=%08x args %04x/%04x/%04x\n", op,
            *(uint32_t *)(g_mem + ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c) + 0x3f2),
            *(uint16_t *)(dg + 0xea1a), *(uint16_t *)(dg + 0xea1c), *(uint16_t *)(dg + 0xea1e)); }
#ifdef __EMSCRIPTEN__
    /* LIVE web play: the mission loop does NOT re-enter fist_timer_pump (see FIST_R3D_DUMP note above),
       so input/frame-post starve in-mission.  op 0x24 is the per-frame cockpit render -- post one frame
       per render from this seam (the palette is the engine ISR's business, patch 575).  board:0001 */
    if (g_web_mode && op == 0x24) {
        extern void fist_web_pump_input(void), fist_web_post_frame(void), fist_web_post_audio(void);
        fist_web_pump_input();
        fist_web_post_frame();
        fist_web_post_audio();
    }
#endif
    return 0;
}

static void setup_bda(void);

static void setup_dos_env(void) {
    /* (b) PSP at FIST_PSP_SEG; env-block segment at PSP:0x2c. */
    uint32_t psp = (uint32_t)FIST_PSP_SEG << 4;
    memset(g_mem + psp, 0, 0x100);
    *(uint16_t *)(g_mem + psp + 0x00) = 0x20cd;           /* INT 20h (standard PSP signature) */
    *(uint16_t *)(g_mem + psp + 0x2c) = FIST_ENV_SEG;     /* environment segment */
    /* (c) DOS environment block.
     * ------------------------------------------------------------------------------------------------
     * EXTENDER CONFIG BLOB (the LOADGAME->FIST.RUN->FIST.DAT handshake).  ORACLE GROUND TRUTH: the
     * engine's FUN_0000_d99b reads word[PSP:0x2c] (the env segment), decodes the FIRST ASCIIZ string
     * from offset 0 as nibble pairs (byte = ((c0-0x30)<<4)|(c1-0x30), big-endian within each 16-bit
     * word) into the config table DGROUP:0xea14.., then swaps (ea16,ea18)/(ea1a,ea1c) and far-CALLs
     * the resulting pointer [ea16].  The blob is NOT set by LOADGAME (the `IBM=`/`ATCODE=` strings in
     * LOADGAME.EXE are LZEXE-0.91 decompressor artifacts, verified by unpacking it; LOADGAME sets NO
     * env var -- it passes hardware config via the command tail and an in-RAM blob).  The nibble blob
     * is synthesized by the Doug-Huffman EXTENDER (FIST.RUN) which rewrites PSP:0x2c before entering
     * the engine.  Captured live from the running original under QEMU (three identical copies in low
     * RAM, e.g. linear 0xe080): the 24-char string below.  It decodes+swaps to:
     *     [ea16] far ptr = 0x0762:0x1179 (linear 0x8799),  args ea1a=0x0f30 ea1c=0 ea1e=0x526,
     *     ea14 = 0x06d9.
     * Disassembling 0x0762:0x1179 in the oracle's own memory shows the extender's real-mode->32-bit-PM
     * callback GATE (push segs; rep movsl the arg block; stack-switch; far-jmp into the PM extender).
     * i.e. d99b's e339 dispatch is an EXTENDER SERVICE CALL, not an engine routine -- in our port the
     * extender role is the shim, so the far-call to linear 0x8799 routes through fist_icall (honest
     * logged trap today; a faithful extender-service handler is the documented next seam).  Seeding
     * the REAL captured blob makes d99b decode the true vector rather than garbage. */
    /* The blob's FIRST word is the PSP of FIST.RUN itself (the LOADGAME child; the extender's own PSP
     * is the one at [DGROUP:0x68]): FUN_0000_fd1b decodes it into DGROUP:0x16c4 and FUN_0000_ff2c reads
     * THAT PSP's command tail, "\r" + 4 hex digits offset + 4 hex digits segment = a far pointer to the
     * hardware/OS detection SCRIPT LOADGAME.EXE leaves in memory (its `-K400,0,1000 -X5000 FIST.RUN`
     * line is LOADGAME's, not the engine's).  Oracle ground truth (DOSBox, scratch/oracle/player.ram.bin):
     * env blob "025402==117900000?300526" -> PSP 0x0254, whose tail is "\r016401D4" -> script at
     * 01D4:0164 (the text below, byte-exact).  The engine parses it against the option tables at
     * STRSEG:0x15c/0x16c (section letter -> option letter -> DGROUP word pair):
     *     >H  C4934,0034 -> [0x242]=0x4934 [0x244]=0x0034  (LOADGAME's CPU speed rating: the MGA driver
     *                       picks its unguarded per-tick DAC method 0be2 for >= 0x31, the engine its
     *                       386 blit variants for > 0x32, SOUNDDVR its fast config for > 0x31)
     *         V0056      -> [0x246]=0x56 (video chipset)      M0805,0400 -> [0x24e]=0x0805 (mouse 8.05)
     *         A0220 I0007 D0001 H0005 T0006  (no H-section entries: skipped by the engine)
     *     >M  X0300,3B40 -> [0x258]=0x3b40 XMS free KB (=> HIGH detail in db3f)   E0400,3B40 -> [0x25a]
     *     >O  D0500,0279 -> [0x260]=0x279 conventional free KB   CADAD,0217 -> [0x264]   HFFFF -> [0x26c]
     *     >Z  end
     * The script is the ORACLE MACHINE's description and is reproduced verbatim so the port runs the
     * same driver/engine variants the reference does.  board:0017 */
    uint32_t env = (uint32_t)FIST_ENV_SEG << 4;
    uint8_t *e = g_mem + env;
    size_t k = 0;
    { char blob[32];                                 /* oracle-captured extender config blob, PSP word ours */
      snprintf(blob, sizeof blob, "%c%c%c%c0762117900000?300526",   /* fd1b: nibble = char - 0x30 */
               0x30 + ((FIST_RUN_PSP_SEG >> 12) & 0xf), 0x30 + ((FIST_RUN_PSP_SEG >> 8) & 0xf),
               0x30 + ((FIST_RUN_PSP_SEG >> 4) & 0xf), 0x30 + (FIST_RUN_PSP_SEG & 0xf));
      for (const char *b = blob; *b; b++) e[k++] = (uint8_t)*b; }
    e[k++] = 0;                 /* end of the config blob string (d99b decodes up to here) */
    {
        static const char script[] =
            ">H\r\nC4934,0034\r\nV0056\r\nM0805,0400\r\nA0220\r\nI0007\r\nD0001\r\nH0005\r\nT0006\r\n"
            ">M\r\nX0300,3B40\r\nE0400,3B40\r\n"
            ">O\r\nD0500,0279\r\nCADAD,0217\r\nHFFFF\r\n"
            ">M\r\nX0300,3B40\r\nE0400,3B40\r\n"
            ">Z\r\n";
        uint32_t rpsp = (uint32_t)FIST_RUN_PSP_SEG << 4, hw = (uint32_t)FIST_HWCFG_SEG << 4;
        char tail[16];
        memset(g_mem + rpsp, 0, 0x100);
        *(uint16_t *)(g_mem + rpsp + 0x00) = 0x20cd;
        *(uint16_t *)(g_mem + rpsp + 0x2c) = FIST_ENV_SEG;
        snprintf(tail, sizeof tail, "\r%04X%04X", 0u, (unsigned)FIST_HWCFG_SEG);   /* off, seg */
        g_mem[rpsp + 0x80] = (uint8_t)strlen(tail);
        memcpy(g_mem + rpsp + 0x81, tail, strlen(tail));
        memcpy(g_mem + hw, script, sizeof script);
    }
    const char *v = "PATH=C:\\";
    while (*v) e[k++] = (uint8_t)*v++;
    e[k++] = 0;                 /* end of PATH= string */
    e[k++] = 0;                 /* empty string => end of environment */
    e[k++] = 1; e[k++] = 0;     /* word: 1 following string (the program path) */
    const char *p = "C:\\FIST.DAT";
    while (*p) e[k++] = (uint8_t)*p++;
    e[k++] = 0;
    fprintf(stderr, "[fist] DOS loader: DGROUP table installed, PSP seg 0x%04x, env seg 0x%04x, "
            "FIST.RUN PSP 0x%04x -> hw script at 0x%04x:0000\n", FIST_PSP_SEG, FIST_ENV_SEG,
            FIST_RUN_PSP_SEG, FIST_HWCFG_SEG);
    setup_bda();
}

/* ---- BIOS Data Area (segment 0040) ---- the engine reads BDA fields directly (no BIOS is present),
 * so the loader role seeds the ones it relies on -- exactly as a real BIOS would have at power-on.
 * FUN_1000_30de (a vsync wait reached early in FUN_1000_2ebe boot, before any INT 10h mode set) reads
 * 0040:0063 = the active 6845 CRTC I/O base port, adds 6 to get Input Status Register 1, and polls the
 * vertical-retrace bit. Color VGA/EGA CRTC base = 0x03D4 -> status reg 0x03DA, which fist_vga.c's in()
 * handler services (toggles the retrace bits). Without this, 0040:0063 is 0 (garbage) and the poll spins
 * on a bogus port forever. Also seed 0040:0049 = current video mode (0x03 = 80x25 text at boot). */
static void setup_bda(void) {
    *(uint16_t *)(g_mem + 0x463) = 0x03D4;   /* 0040:0063  CRTC base I/O port (color) */
    *(uint8_t  *)(g_mem + 0x449) = 0x03;     /* 0040:0049  current BIOS video mode (text) */
    fprintf(stderr, "[fist] BIOS data area seeded (0040:0063=0x03D4 CRTC port)\n");
}

/* ---- FIST_OVL_SELFTEST: exercise the whole multi-module overlay path independently of the engine.
 * Blocker #1 (a DGROUP near-ptr/extraout_DX base-loss) currently corrupts the filename + param block
 * the engine hands INT 21h AH=4B, so the engine cannot reach a good overlay load yet. This gated probe
 * loads the REAL MGAVIDEO.DVR at a heap segment, applies its MZ relocs, registers it, and confirms the
 * dispatcher resolves a runtime call target inside the overlay to a decompiled driver function -- i.e.
 * validates fist_load_overlay + fist_ovl_register (fmap wiring) + fist_icall overlay-range dispatch.
 * Off by default; pure diagnostic (does not run driver code). */
static void ovl_selftest(void) {
    extern struct fist_ovl fist_ovl_tab[]; extern int fist_ovl_n;
    uint16_t load_seg = 0x3400;                        /* a heap segment (as the engine's AH=48 hands out) */
    fprintf(stderr, "[selftest] loading MGAVIDEO.DVR at seg 0x%04x ...\n", load_seg);
    int r = fist_load_overlay("MGAVIDEO.DVR", load_seg, load_seg);
    fprintf(stderr, "[selftest] fist_load_overlay -> %d, overlays registered=%d\n", r, fist_ovl_n);
    if (r == 0 && fist_ovl_n > 0) {
        struct fist_ovl *o = &fist_ovl_tab[fist_ovl_n-1];
        fprintf(stderr, "[selftest]   name='%s' base=0x%05x size=0x%x fmap=%s fmap_n=%u\n",
                o->name, o->base, o->size, o->fmap?"WIRED":"NULL", o->fmap_n);
        /* resolve the driver ENTRY (module offset 0) and a mid-module target through the dispatcher */
        for (uint32_t off = 0; off <= 0x110; off += 0x110) {
            code *fn = fist_icall(o->base + off);
            fprintf(stderr, "[selftest]   fist_icall(base+0x%x) -> %p  (%s)\n", off, (void*)fn,
                    fn ? "resolved" : "NULL");
        }
    }
    fprintf(stderr, "[selftest] done (overlay load + reloc + register + fmap wiring + dispatch resolve).\n");
}

/* ============================================================================================
 * FIST_6980PROVE -- STANDALONE integration-readiness proof for the mission-terrain colour fix.
 * (default OFF; runs BEFORE the engine boot and _exit()s, so it cannot touch the 25 verify flows.)
 *
 * Feeds the port's REAL FUN_0000_6980 (m_ext_FUN_0000_6980) the FRAME-MATCHED render-time inputs
 * captured by FIST_R6980CAP (docs/oracle_colour_gate.md) and byte-compares its tile output to the
 * banked oracle 6980 shadow (== 6980's actual render-time stores, byte-exact ground truth).
 *
 * The 3 real-port fixes this exercises:
 *   (1) COLORMAP -> LIGHT REDUCE:  6980's colour read `mov al,[esi+ecx+0x100000]` must return the
 *       bdc4 LIGHT reduce (max 228), not the port's dark C32 (max 104).  Here we place the captured
 *       LIGHT reduce at [0x85bc]+0x100000 (FIST_6980PROVE=dark places the C32 to show the baseline).
 *   (2) RAMPS-BANK:  3a24/3e24/4224/4624 are paged-boot-filled (all-1 in fist_image.bin) -> banked
 *       here from the capture at ext_base+off.  90c0==90c4 in the capture, so 6980 skips 395e and
 *       consumes 4224/4624 directly (the faithful render-time path).
 *   (3) PROJ-SMC-MODEL:  patches/342 threads [0x3909]'s VALUE (the proj host ptr) + the per-depth
 *       +0x100 SMC base into the two colour-column lookups (Ghidra emitted the static 0x7fffffff
 *       immediate + a byte-deref of LAB_3909 -> wild deref/SIGSEGV).  Same patch rebases 6980's
 *       un-based in-image data refs (0x9454 matrix, 0x4224/0x4624 ramps) to g_mem+fist_ext_base.
 *
 * Bundle layout (tools/oracle/samples/voxel6980_framematched_pass08.bin.gz):
 *   'C69G' + passno(4) + ext[0x9000..0x9200](0x200) + 16 named globals(64) + ramps 4*256(4096)
 *   + proj(0xfa00) + HM(1MB) + LIGHT-reduce(1MB) + dark-C32(1MB) + tile(64K) + shadow(64K)
 * ============================================================================================ */
static void pv_rd(FILE *p, void *dst, size_t n, const char *what) {
    size_t got = fread(dst, 1, n, p);
    if (got != n) { fprintf(stderr, "[6980prove] FATAL short read of %s: %zu/%zu\n", what, got, n); _exit(2); }
}
static void fist_6980_prove(void) {
    const char *mode = getenv("FIST_6980PROVE");
    int use_dark = mode && strcmp(mode, "dark") == 0;
    FILE *p = popen("gzip -dc tools/oracle/samples/voxel6980_framematched_pass08.bin.gz", "r");
    if (!p) { fprintf(stderr, "[6980prove] FATAL: cannot popen the bundle\n"); _exit(2); }
    char magic[4]; pv_rd(p, magic, 4, "magic");
    if (memcmp(magic, "C69G", 4) != 0) { fprintf(stderr, "[6980prove] FATAL: bad magic\n"); _exit(2); }
    uint32_t passno; pv_rd(p, &passno, 4, "passno");
    static uint8_t ext[0x200]; static uint32_t g[16]; static uint32_t ramp[1024];
    static uint8_t proj[0xfa00];
    static uint8_t HM[0x100000], RED[0x100000], CM[0x100000], tile_o[0x10000], shadow[0x10000];
    pv_rd(p, ext, 0x200, "ext");         pv_rd(p, g, 64, "g");        pv_rd(p, ramp, 4096, "ramp");
    pv_rd(p, proj, 0xfa00, "proj");      pv_rd(p, HM, 0x100000, "HM"); pv_rd(p, RED, 0x100000, "RED");
    pv_rd(p, CM, 0x100000, "CM");        pv_rd(p, tile_o, 0x10000, "tile"); pv_rd(p, shadow, 0x10000, "shadow");
    pclose(p);
    /* FIST_6980PROVE_HM=<file> (default OFF): override the capture HM with the port's OWN op-0x18-decoded
     * heightmap (dumped via FIST_HMDUMP) -- isolates whether the port's own contiguous map fixes the
     * over-draw.  FIST_6980PROVE_RED=<file> likewise overrides the LIGHT reduce (colour slot). */
    { const char *hf=getenv("FIST_6980PROVE_HM");
      if (hf) { FILE *h=fopen(hf,"rb"); if(h){ size_t n=fread(HM,1,0x100000,h); fclose(h);
        fprintf(stderr,"[6980prove] overrode HM from %s (%zu B)\n", hf, n); } } }
    { const char *rf=getenv("FIST_6980PROVE_RED");
      if (rf) { FILE *h=fopen(rf,"rb"); if(h){ size_t n=fread(RED,1,0x100000,h); fclose(h);
        fprintf(stderr,"[6980prove] overrode RED reduce from %s (%zu B)\n", rf, n); } } }
    #define GE(off) (*(uint32_t*)(ext + ((off) - 0x9000)))

    uint8_t *xb = g_mem + FIST_EXT_BASE;

    /* (1) colormap buffer: HM at +0, colour source at +0x100000; [0x85bc] = host ptr */
    uint8_t *hmcm = (uint8_t*)malloc(0x200000);
    memcpy(hmcm,            HM,                       0x100000);
    memcpy(hmcm + 0x100000, use_dark ? CM : RED,      0x100000);
    *(uint32_t*)(xb + 0x85bc) = (uint32_t)(uintptr_t)hmcm;
    *(uint32_t*)(xb + 0x85b8) = (uint32_t)(uintptr_t)(hmcm + 0x400000); /* unused by 6980, kept sane */

    /* proj table @ [0x3909], 0x100-aligned (the SMC base masks the low byte) */
    uint8_t *projraw = (uint8_t*)malloc(0x10200);
    uint8_t *projbuf = (uint8_t*)(((uintptr_t)projraw + 0xff) & ~(uintptr_t)0xff);
    memset(projbuf, 0, 0x10000); memcpy(projbuf, proj, 0xfa00);
    *(uint32_t*)(xb + 0x3909) = (uint32_t)(uintptr_t)projbuf;

    /* tile scratch @ [0x3918] (zeroed: 6980 overlays terrain spans, so the compare is direct) */
    uint8_t *tile = (uint8_t*)calloc(1, 0x10000);
    *(uint32_t*)(xb + 0x3918) = (uint32_t)(uintptr_t)tile;
    /* per-depth y-buffer save @ [0x390d] (250*0x100) */
    uint8_t *ybuf = (uint8_t*)calloc(1, 0x10000);
    *(uint32_t*)(xb + 0x390d) = (uint32_t)(uintptr_t)ybuf;

    /* (2) banked ramps at ext_base+off (patch-342 rebases 6980's reads here) */
    memcpy(xb + 0x3a24, &ramp[0],   256*4);
    memcpy(xb + 0x3e24, &ramp[256], 256*4);
    memcpy(xb + 0x4224, &ramp[512], 256*4);
    memcpy(xb + 0x4624, &ramp[768], 256*4);

    /* world camera + scale (6980 recomputes 90fc/9100 from 90e0 + the image matrix) */
    *(uint32_t*)(xb + 0x90d4) = GE(0x90d4); *(uint32_t*)(xb + 0x90d8) = GE(0x90d8);
    *(uint32_t*)(xb + 0x90dc) = GE(0x90dc); *(uint32_t*)(xb + 0x90e0) = GE(0x90e0);
    *(uint32_t*)(xb + 0x90c0) = g[10];      *(uint32_t*)(xb + 0x90c4) = g[15]; /* ==90c0 -> skip 395e */
    /* FIST_6980PROVE_BUILDPROJ (default OFF): force 90c4!=90c0 so 6980 REBUILDS the proj table via 395e
     * from the banked ramps 3a24/3e24 + 90c0 (instead of consuming the injected capture proj).  Tests
     * whether the injected capture proj is a degenerate/frame-skewed artifact. */
    if (getenv("FIST_6980PROVE_BUILDPROJ")) { *(uint32_t*)(xb + 0x90c4) = g[10] ^ 1; fprintf(stderr,"[6980prove] forcing 395e proj rebuild from banked ramps\n"); }

    /* scalars */
    *(uint32_t*)(xb + 0x38ed) = g[4];  *(uint32_t*)(xb + 0x38f1) = g[5];
    *(uint32_t*)(xb + 0x38fd) = g[6];  *(uint32_t*)(xb + 0x3901) = g[7];
    *(uint32_t*)(xb + 0x3905) = g[8];  *(uint32_t*)(xb + 0x3a20) = g[9];

    /* ---- run the REAL port 6980 ---- */
    extern void m_ext_FUN_0000_6980(void);
    m_ext_FUN_0000_6980();

    /* geometry sanity: 6980 recomputes 90fc/9100 from 90e0 + the image matrix */
    fprintf(stderr, "[6980prove] pass %u mode=%s | recomputed 90fc=%08x 9100=%08x  (capture ad301958/9e669af4)\n",
            passno, use_dark ? "dark-C32(baseline)" : "LIGHT-reduce(fix)",
            *(uint32_t*)(xb + 0x90fc), *(uint32_t*)(xb + 0x9100));

    /* byte-compare tile vs the oracle 6980 shadow (both start from 0 -> full-64K compare is exact) */
    long same = 0, tnz = 0, snz = 0, both = 0, mism = 0, over = 0, under = 0;
    for (int i = 0; i < 0x10000; i++) {
        uint8_t t = tile[i], s = shadow[i];
        if (t == s) same++;
        if (t) tnz++;
        if (s) snz++;
        if (t && s) { both++; if (t != s) mism++; }
        if (t && !s) over++;
        if (!t && s) under++;
    }
    fprintf(stderr,
        "[6980prove] tile-vs-shadow: byte-exact %ld/65536 (%.4f%%) | tile_nz=%ld shadow_nz=%ld "
        "both_nz=%ld mism=%ld over=%ld under=%ld\n",
        same, 100.0 * same / 65536.0, tnz, snz, both, mism, over, under);
    /* also: over the shadow's own footprint (rows 0-159 terrain), how many match? */
    long fnz = 0, fmatch = 0;
    for (int i = 0; i < 0x10000; i++) if (shadow[i]) { fnz++; if (tile[i] == shadow[i]) fmatch++; }
    fprintf(stderr, "[6980prove] over-shadow-footprint: %ld/%ld (%.4f%%) exact where 6980 wrote\n",
            fmatch, fnz, fnz ? 100.0 * fmatch / fnz : 0.0);

    /* ---- DECISIVE COLOUR GATE (value-set containment, geometry-independent) ---- */
    /* what colour values did the port's 6980 emit, and are they LIGHT (in the reduce) or dark C32? */
    int seenT[256] = {0}, seenS[256] = {0}, seenR[256] = {0}, seenC[256] = {0};
    long tsum = 0; int tmin = 255, tmax = 0, tdist = 0;
    for (int i = 0; i < 0x10000; i++) if (tile[i]) { int v = tile[i]; if (!seenT[v]) { seenT[v] = 1; tdist++; } tsum += v; if (v < tmin) tmin = v; if (v > tmax) tmax = v; }
    for (int i = 0; i < 0x10000; i++) if (shadow[i]) seenS[shadow[i]] = 1;
    for (int i = 0; i < 0x100000; i++) { seenR[RED[i]] = 1; seenC[CM[i]] = 1; }
    int inR = 0, inC = 0, inS = 0;
    for (int v = 0; v < 256; v++) if (seenT[v]) { if (seenR[v]) inR++; if (seenC[v]) inC++; if (seenS[v]) inS++; }
    fprintf(stderr, "[6980prove] PORT tile value-set: distinct=%d mean=%.1f min=%d max=%d\n",
            tdist, tnz ? (double)tsum / tnz : 0.0, tdist ? tmin : 0, tmax);
    fprintf(stderr, "[6980prove]   contained in LIGHT-reduce(max228): %d/%d | dark-C32(max104): %d/%d | oracle-shadow: %d/%d\n",
            inR, tdist, inC, tdist, inS, tdist);
    fprintf(stderr, "[6980prove]   COLOUR VERDICT: the port's 6980 emitted %s colours\n",
            (tmax > 104) ? "LIGHT (>dark-C32 max 104) == the reduce; the colour fix WORKS through the real 6980"
                         : "DARK (<=104); the colour source is still the dark C32");
    /* ---- compare vs the DENSE render-time tile tile_o (the REAL ground truth; the "shadow" is a
     * SPARSE/incomplete capture = tile_o with ~88% zeroed, so the shadow over-draw metric above is a
     * span-start artifact).  tile_o is fully populated (65536 nz, mean ~199, 66 distinct). ---- */
    { long dsame=0,dboth=0,dmis=0,dunder=0,dtnz=0,donz=0;
      int dR=0; long dsum=0; int dmn=255,dmx=0,dd=0; int seenTd[256]={0};
      for (int i=0;i<0x10000;i++){ uint8_t t=tile[i],o=tile_o[i];
        if(t==o) dsame++; if(t)dtnz++; if(o)donz++;
        if(t&&o){dboth++; if(t!=o)dmis++;} if(!t&&o)dunder++; }
      for (int i=0;i<0x10000;i++) if(tile[i]){int v=tile[i]; if(!seenTd[v]){seenTd[v]=1;dd++;} dsum+=v; if(v<dmn)dmn=v; if(v>dmx)dmx=v;}
      fprintf(stderr,"[6980prove] vs-DENSE-tile_o: byte-exact %ld/65536 (%.2f%%) | port_nz=%ld tile_o_nz=%ld both=%ld mism=%ld under=%ld\n",
        dsame,100.0*dsame/65536.0,dtnz,donz,dboth,dmis,dunder);
      fprintf(stderr,"[6980prove] vs-DENSE-tile_o: port coverage=%.1f%% of tile_o | port distinct=%d mean=%.1f max=%d  tile_o mean~199 max252 distinct66\n",
        donz?100.0*dboth/donz:0.0, dd, dtnz?(double)dsum/dtnz:0.0, dmx);
    }
    const char *dp = getenv("FIST_6980PROVE_DUMP");
    if (dp) { FILE *df = fopen(dp, "wb"); if (df) { fwrite(tile, 1, 0x10000, df); fclose(df); fprintf(stderr, "[6980prove] dumped port tile -> %s\n", dp); } }
    const char *op2 = getenv("FIST_6980PROVE_ODUMP");
    if (op2) { FILE *df = fopen(op2, "wb"); if (df) { fwrite(tile_o, 1, 0x10000, df); fclose(df); fprintf(stderr, "[6980prove] dumped dense tile_o -> %s\n", op2); } }
    #undef GE
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
#ifndef __EMSCRIPTEN__
    if (getenv("FIST_SEGV_BT") || getenv("FIST_FBTRAP") || getenv("FIST_OBJTRAP")) {
        struct sigaction sa; memset(&sa, 0, sizeof sa);
        sa.sa_sigaction = segv_bt; sa.sa_flags = SA_SIGINFO;
        sigaction(SIGSEGV, &sa, NULL);
        if (getenv("FIST_OBJTRAP") || getenv("FIST_WWTRAP")) { extern void objtrap_trap(int,siginfo_t*,void*);
            struct sigaction st; memset(&st,0,sizeof st); st.sa_sigaction=objtrap_trap; st.sa_flags=SA_SIGINFO;
            sigaction(SIGTRAP,&st,NULL); }
    }
#endif
    load_image();
    setup_dos_env();
    ext_module_init();          /* load + register the extender KDV-player module (re_out/fist_ext.c) */
    if (getenv("FIST_6980PROVE")) { fist_6980_prove(); return 0; }
    if (getenv("FIST_OVL_SELFTEST")) { ovl_selftest(); return 0; }
    if (getenv("FIST_NORUN")) {
        fprintf(stderr, "[fist] FIST_NORUN set: link/load OK, not entering engine.\n");
        return 0;
    }
    start_timer();
    int reason = setjmp(g_fist_exit);
    if (reason == 0) {
        fprintf(stderr, "[fist] calling app_entry()\n");
        app_entry(0, 0, 0, 0, 0, 0);
        fprintf(stderr, "[fist] app_entry returned normally\n");
    } else {
        fprintf(stderr, "[fist] engine unwound (reason=%d, exit-code=%d, video-mode=0x%02x)\n",
                reason, g_fist_exit_code, fist_vga_mode());
    }
    const char *fb = getenv("FIST_FBDUMP");
    if (fb) fist_dump_framebuffer(fb);
    if (getenv("FIST_ROWDUMP")) {
        uint16_t *rt = (uint16_t*)(g_mem + 0x1c000 + 0x7c8);
        uint16_t c794 = *(uint16_t*)(g_mem + 0x1c000 + 0x794);
        fprintf(stderr, "[rowdump] c794=%04x rowtab[0]=%u [1]=%u [2]=%u [31]=%u [62]=%u [63]=%u [100]=%u\n",
                c794, rt[0], rt[1], rt[2], rt[31], rt[62], rt[63], rt[100]);
    }
    if (getenv("FIST_DESCRDUMP")) {
        uint16_t w0 = *(uint16_t*)(g_mem + 0x1c000 + 0x9f1c);
        fprintf(stderr, "[descrdump] DGROUP:0x9f1c word0(screen .MS3 seg)=%04x  word[0x9f1e]=%04x\n",
                w0, *(uint16_t*)(g_mem + 0x1c000 + 0x9f1e));
        if (w0) { uint8_t *p = g_mem + ((uint32_t)w0<<4);
            fprintf(stderr, "[descrdump] .MS3 seg bytes: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]); }
    }
    return 0;
}
