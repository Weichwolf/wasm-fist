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
extern int  fist_vga_mode(void);

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
static void segv_bt(int sig, siginfo_t *si, void *uc) {
    ucontext_t *u = (ucontext_t *)uc;
    unsigned long eip = (unsigned long)u->uc_mcontext.gregs[14 /*REG_EIP*/];
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

/* ====================== TIMER-DRIVEN EXECUTION MODEL ======================
 * The engine installs its OWN INT-8 (PIT) ISR (DOS set-vector 0x08 -> FUN_1000_30f8 @ linear 0x130f8)
 * and busy-waits on the tick counters that ISR maintains (DGROUP:0x452 in FUN_1000_3346, DGROUP:0x18ca
 * in FUN_1000_3352, ...). On the original the PIT IRQ fires ~18.2x/s (or the reprogrammed rate) and
 * asynchronously preempts those spins to run the ISR. Our host is single-threaded and MUST NOT run
 * engine C from inside a signal handler (unsafe re-entrancy), so we drive the ISR COOPERATIVELY:
 *   - a periodic SIGALRM only bumps the BIOS tick (0040:006C) and raises a pending-tick flag
 *     (async-signal-safe: no engine code, no libc);
 *   - fist_timer_pump() drains pending ticks at safe points (the engine's spin-waits and the port-I/O
 *     shims) by invoking the engine's installed INT-8 ISR via the indirect-call dispatcher, guarded
 *     against re-entry. So DGROUP:0x452 advances and the spins clear, without corrupting engine state.
 * RATE KNOB (determinism): FIST_TICK_HZ sets the SIGALRM frequency (default 200 Hz). Each SIGALRM =
 * one pending ISR invocation (capped). For Stage-1 unblocking any correct-enough rate works; the knob
 * is the seam a later deterministic (instruction-counted) tick source will replace. */
#define BIOS_TICK_LIN 0x46C
static volatile sig_atomic_t g_tick_pending;     /* raised by the timer source, drained by the pump */
#define TICK_PENDING_CAP 8
/* One tick of the host time base: bump the BIOS 18.2 Hz counter (0040:006C) and queue one INT-8 ISR
 * invocation. Async-signal-safe (native SIGALRM handler) AND callable synchronously (wasm pump). */
static void tick_advance(void){
    (*(volatile uint32_t*)(g_mem+BIOS_TICK_LIN))++;
    if (g_tick_pending < TICK_PENDING_CAP) g_tick_pending++;
}
#ifndef __EMSCRIPTEN__
static void tick_handler(int sig){ (void)sig; tick_advance(); }
static void start_timer(void){
    struct sigaction sa = {0}; sa.sa_handler = tick_handler; sa.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sa, 0);
    long hz = 200; const char *e = getenv("FIST_TICK_HZ"); if (e){ long v = atol(e); if (v>0 && v<=100000) hz = v; }
    long usec = 1000000L / hz; if (usec < 1) usec = 1;
    struct itimerval it; it.it_interval.tv_sec=0; it.it_interval.tv_usec=usec;
    it.it_value = it.it_interval; setitimer(ITIMER_REAL, &it, 0);
    fprintf(stderr, "[fist] host timer started (%ld Hz SIGALRM; INT-8 ISR pumped cooperatively)\n", hz);
}
#else
/* wasm has no SIGALRM/setitimer. The tick is driven COOPERATIVELY: each fist_timer_pump() advances the
 * time base by one tick (fist_wasm_tick, called at the top of the pump). fist_timer_pump is invoked from
 * the engine's spin-waits and port-I/O shims, so ticks accrue in lockstep with engine progress -- the
 * same deterministic seam FIST_TICK_HZ selects on native, here at "one tick per safe point". The rendered
 * frame does not depend on the tick RATE (the menu is static once painted), so this is native-parity. */
static void start_timer(void){
    fprintf(stderr, "[fist] wasm cooperative time base (one INT-8 tick per pump; no SIGALRM)\n");
}
void fist_wasm_tick(void){ tick_advance(); }
#endif

/* ---- the engine's installed INT-8 (PIT) ISR entry, captured at DOS set-vector 0x08 ---- */
static uint32_t g_int8_lin;          /* linear (SEG<<4)+OFF of the installed handler */
static int      g_int8_set;          /* becomes 1 once the engine installs its ISR */
static int      g_in_isr;            /* re-entry guard: never run the ISR from inside itself */
static long     g_isr_runs;          /* diagnostic count */

void fist_set_int8_handler(uint32_t linear){
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

void fist_timer_pump(void){
#ifdef __EMSCRIPTEN__
    { extern void fist_wasm_tick(void); fist_wasm_tick(); }   /* cooperative time base (no SIGALRM) */
#else
    /* Debug seam: FIST_COOP_TICK=1 drives the INT-8 time base COOPERATIVELY on native too (one tick
     * per pump, exactly like wasm) so the engine can be traced under gdb without gdb having to process
     * thousands of SIGALRM/sec. The rendered frame does not depend on the tick RATE (the menu is a fixed
     * point once painted), so a coop-tick native run reaches the same deterministic menu/sub-screen frame
     * as the SIGALRM-timed run -- diagnostics only; the shipped run keeps the SIGALRM timer. */
    { static int coop = -1; if (coop < 0) coop = getenv("FIST_COOP_TICK") ? 1 : 0;
      if (coop) tick_advance(); }
#endif
    { extern void fbtrap_arm_hook(void); fbtrap_arm_hook(); }
    /* Dev watchdog: FIST_RUNMS=<ms> dumps the framebuffer (FIST_FBDUMP) and exits after a wall-clock
     * deadline -- lets a first-light frame be captured while the engine is in its (non-returning) main
     * loop.  Runs in main context (safe for the PPM writer). */
    static long g_deadline_ms = -2;
    if (g_deadline_ms == -2) {
        const char *r = getenv("FIST_RUNMS");
        g_deadline_ms = r ? atol(r) : -1;
        if (g_deadline_ms > 0) {
            struct timeval tv; gettimeofday(&tv, 0);
            g_deadline_ms += tv.tv_sec*1000L + tv.tv_usec/1000L;
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
    if (g_dumptick == -2) { const char *d = getenv("FIST_DUMPTICK"); g_dumptick = d ? atol(d) : -1; }
    if (g_dumptick > 0) {
        uint16_t cur = *(uint16_t*)(g_mem + 0x1c452);
        if ((long)cur >= g_dumptick) fist_dump_and_exit("FIST_DUMPTICK");
    }
    if (g_deadline_ms > 0) {
        struct timeval tv; gettimeofday(&tv, 0);
        long now = tv.tv_sec*1000L + tv.tv_usec/1000L;
        if (now >= g_deadline_ms) fist_dump_and_exit("FIST_RUNMS watchdog");
    }
    /* Async vertical-retrace IRQ (palette upload): fires on the timer heartbeat regardless of whether
     * the engine's current loop polls in(0x3da) -- mirrors the original's async retrace ISR (FUN_0000_0b1f)
     * that uploads word[DGROUP:0x782] to the DAC ~70Hz.  Without this the DAC stays black in loops (e.g.
     * the main-menu idle) that never reach an explicit in(0x3da). */
    { extern void fist_vga_service_retrace(void); fist_vga_service_retrace(); }
    /* EXTENDER frame-ready handshake (we ARE the Doug-Huffman extender).  On the mission cockpit path
     * FUN_1000_a84c sets d549(0x1549)=0x1c + FUN_1000_795c sets d548(0x1548)=1 ("cockpit view, waiting
     * for the next frame").  The extender's 32-bit-PM flight model -- which is NOT in FIST.DAT -- signals
     * "frame data ready" by OR-ing bit7 into that handshake byte (via the TCB+8 pointer d99b installed),
     * i.e. d548: 1 -> 0x81.  That is exactly the gate FUN_1000_795c tests to run a20d(camera) + df0e ->
     * aa10=0x24 -> e339 -> the extender 9200 windshield voxel render.  Emulate the always-ready extender
     * here.  GATED on d549==0x1c (the cockpit view, only ever set on the in-mission 459a path) so it is
     * strictly behaviour-neutral for the 19 menu/pre-mission verify flows. */
    if (g_mem[0x1c000 + 0x1549] == 0x1c && g_mem[0x1c000 + 0x1548] == 1)
        g_mem[0x1c000 + 0x1548] = 0x81;
    if (!g_int8_set || g_in_isr) return;
    { extern void fist_queue_check(const char*); fist_queue_check("pre-isr"); }
    int budget = 4;
    while (g_tick_pending > 0 && budget-- > 0){
        g_tick_pending--;                         /* benign race w/ SIGALRM: at worst drops a tick */
        code *fn = fist_icall(g_int8_lin);
        if (!fn) break;
        g_in_isr = 1;
        ((int(*)(int,int,int,int,int,int,int,int,int,int))fn)(0,0,0,0,0,0,0,0,0,0);
        g_in_isr = 0;
        g_isr_runs++;
        { extern void fist_queue_check(const char*); fist_queue_check("post-isr"); }
    }
    fist_input_pump();
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
 *   FIST_MOUSE="t:x:y:b; t:x:y:b; ..."   t = pump-after-ready trigger; x,y = PIXEL pos (0..319,0..199);
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
static long g_ready_pumps = 0; /* pumps since menu ready */
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
    g_ready_pumps++;
    while (g_mstep_i < g_mstep_n && g_ready_pumps >= g_mstep[g_mstep_i].t){
        unsigned nx=g_mstep[g_mstep_i].x, ny=g_mstep[g_mstep_i].y, nb=g_mstep[g_mstep_i].b;
        unsigned vx = nx*2, vy = ny;           /* mode-13h virtual coords: x doubled, y 1:1 */
        fprintf(stderr, "[input] step %d @pump%ld -> move (%u,%u) btn %u->%u\n",
                g_mstep_i, g_ready_pumps, nx, ny, g_last_btn, nb);
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
 * FUN_0000_f842 semantics (a leading seg word, then (off,val) pairs -> DGROUP[off]=val:seg) vs
 * FUN_0000_f7ef near semantics (no seg word, (off,val) pairs -> DGROUP[off]=val, addend 0 in base-0).
 * Returns the number of (off,val) entries applied. */
int fist_apply_reloc_section(uint16_t si, int is_far) {
    #define RW(a) (*(uint16_t*)(g_mem + (a)))
    uint32_t p = RELOC_TAB_LIN + si;
    uint16_t seg = 0;
    if (is_far) { seg = RW(p); p += 2; }
    int n = 0;
    for (;;) {
        uint16_t off = RW(p);
        if (off == 0) break;
        RW(DGROUP_LIN + off) = RW(p + 2);
        if (is_far) RW(DGROUP_LIN + off + 2) = seg;
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
    /* Video chipset id: DGROUP:0x246 (DAT_1000_c246). FUN_0000_134e reads it as the detected video
     * chipset and, when == 0x56, sets the driver drive-letter byte to 'M' (0x4d) -> "..\MGAVIDEO.DVR"
     * (patch 021 pokes it into the template at DGROUP:0x743). It is ONLY EVER READ in the engine -- a
     * hardware-detection subsystem (VESA/chipset probe) that our port does not run populates it. The
     * value the original detect produces on this title's supported Matrox (MGA) path is 0x56; model that
     * detect result here in the loader role (honest: the field the not-run probe would have written).
     * Without it DAT_1000_c246 stays 0 -> drive letter 0 -> the video filename truncates to "..\". */
    g_mem[DGROUP_LIN + 0x246] = 0x56;
    /* System-requirements memory fields, read by FUN_0000_cb45 (the "enough memory to run" gate,
     * called from cae6 just before the env-config decoder d99b).  cb45 requires:
     *   DGROUP:0x260 (conventional free KB) >= 0x1e5 (485)     else "NOT ENOUGH CONVENTIONAL MEMORY"
     *   DGROUP:0x258 (XMS free KB)          >= 0x9c4 (2500)    else "NOT ENOUGH XMS MEMORY"
     *   DGROUP:0x26c (free disk space)      >= 0x32  (50)      else "NOT ENOUGH FREE HARD DRIVE SPACE"
     * and DGROUP:0x258 additionally auto-selects the detail level in FUN_0000_db3f (<5000 => LOW,
     * 5000..11000 => MEDIUM, >11000 => HIGH).  None of these three fields has ANY writer in the engine
     * decompile (grep + a full store-opcode scan of the image: no store to 0x258/0x260/0x26c) -- they
     * are populated from LOADGAME's hardware/OS detection blob (unpacked LOADGAME format, per-line
     * ASCII-hex `X<xmsver>,<xms_free_KB>` / `D<dosver>,<conv_free>` etc.), which our port does not run.
     * Seed them in the loader role (as with the video chipset DAT_1000_c246 above) using the values a
     * real reference machine yields -- the 32MB oracle machine: XMS free ~0x7B40 KB (=> HIGH detail),
     * conventional free 0x279 KB (from the captured `D0500,0279` line), plenty of disk.  Without them
     * cb45 takes its error path (far-calls the "not enough memory" reporter, which our shim traps to 0
     * -> execution wrongly continues).  TODO: wire the LOADGAME hw-blob parse and demote to a patch. */
    *(uint16_t *)(g_mem + DGROUP_LIN + 0x258) = 0x7b40;   /* XMS free KB  (>11000 => HIGH detail) */
    *(uint16_t *)(g_mem + DGROUP_LIN + 0x260) = 0x0279;   /* conventional free KB */
    *(uint16_t *)(g_mem + DGROUP_LIN + 0x26c) = 0x0064;   /* free disk space */
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

/* PATCH 292: the mission-object-roster iterator FUN_0000_c33c returns its updated SI register (always
 * 0xffff on both the found and exhausted exits -- asm 0xc381/0xc386 `mov si,0xffff`) here; its caller
 * FUN_0000_378e stores it into word[DGROUP:0x4b9e] (asm 0x37a7 `mov [0x4b9e],si`) so the NEXT iterator
 * call sees si!=0 and skips the re-INIT (word[0x4b9e]==0 is the once-per-frame INIT trigger). */
unsigned short g_fist_iter_si;

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
    *(uint32_t *)(xb + 0x90b) = (uint32_t)(uintptr_t)(g_mem + FIST_EXT_HEAP);   /* heap base */
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
int fist_extender_gate(void) {
    uint8_t *dg = g_mem + DGROUP_LIN;
    uint16_t op = *(uint16_t *)(dg + 0xea10);
    if (op == 0x18) g_fist_after_map = 1;
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
            fprintf(stderr,"[ophist] total=%ld op40(ad1e)=%ld op24(9200)=%ld op0c=%ld op18=%ld op54=%ld op60=%ld op44=%ld | tile3918 nz=%ld dist=%d\n",
                total, ophist[0x40], ophist[0x24], ophist[0x0c], ophist[0x18], ophist[0x54], ophist[0x60], ophist[0x44], nz, dist);
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
                uint32_t cm=*(uint32_t*)(xb+0x3918); long nz=0; int h[256]={0},nd=0;
                if(cm){uint8_t*t=(uint8_t*)(uintptr_t)cm; for(int i=0;i<65536;i++){if(t[i])nz++; if(!h[t[i]]){h[t[i]]=1;nd++;}}}
                fprintf(stderr,"[missfb] tile3918 nz=%ld/65536 distinct=%d  ray3a24[0..3]=%d/%d/%d/%d ray3e24[0]=%d 90c0=%08x 90c4=%08x\n",
                    nz,nd,*(int32_t*)(xb+0x3a24),*(int32_t*)(xb+0x3a28),*(int32_t*)(xb+0x3a2c),*(int32_t*)(xb+0x3a30),
                    *(int32_t*)(xb+0x3e24),*(uint32_t*)(xb+0x90c0),*(uint32_t*)(xb+0x90c4));
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
            if (tile3918) *(uint32_t *)(xb + 0xbc90) = tile3918;   /* bc90 REUSED = tile aligned base */
            g_fist_ext_int = 1;                    /* extender-mode flat FILEMGR INT 21h */
            m_ext_FUN_0000_89b0(inbox, inbox, inbox);
            g_fist_ext_int = 0;
            *(uint32_t *)(xb + 0xbc90) = save_bc90;
            if (win && tile3918) *(uint32_t *)(xb + 0x3918) = tile3918 + win;  /* window the tile */
            *(uint32_t *)(xb + 0xc93) = save_c93;
            fprintf(stderr, "[ext] op 0x18 MAP-LOAD returned; detail[0x8490]=0x%x dim[0x8494]=%u (base=0x0b => 2048)\n",
                    *(uint32_t *)(xb + 0x8490), *(uint32_t *)(xb + 0x8494));
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
    fprintf(stderr, "[ext] service op 0x%02x (display-list cmd) inbox=%08x args %04x/%04x/%04x\n", op,
            *(uint32_t *)(g_mem + ((uint32_t)(*(uint16_t*)(dg+0xea2e))<<4) + *(uint16_t*)(dg+0xea2c) + 0x3f2),
            *(uint16_t *)(dg + 0xea1a), *(uint16_t *)(dg + 0xea1c), *(uint16_t *)(dg + 0xea1e));
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
    uint32_t env = (uint32_t)FIST_ENV_SEG << 4;
    uint8_t *e = g_mem + env;
    size_t k = 0;
    const char *blob = "06=90762117900000?300526";   /* oracle-captured extender config blob */
    while (*blob) e[k++] = (uint8_t)*blob++;
    e[k++] = 0;                 /* end of the config blob string (d99b decodes up to here) */
    const char *v = "PATH=C:\\";
    while (*v) e[k++] = (uint8_t)*v++;
    e[k++] = 0;                 /* end of PATH= string */
    e[k++] = 0;                 /* empty string => end of environment */
    e[k++] = 1; e[k++] = 0;     /* word: 1 following string (the program path) */
    const char *p = "C:\\FIST.DAT";
    while (*p) e[k++] = (uint8_t)*p++;
    e[k++] = 0;
    fprintf(stderr, "[fist] DOS loader: DGROUP table installed, PSP seg 0x%04x, env seg 0x%04x\n",
            FIST_PSP_SEG, FIST_ENV_SEG);
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

int main(int argc, char **argv) {
    (void)argc; (void)argv;
#ifndef __EMSCRIPTEN__
    if (getenv("FIST_SEGV_BT") || getenv("FIST_FBTRAP")) {
        struct sigaction sa; memset(&sa, 0, sizeof sa);
        sa.sa_sigaction = segv_bt; sa.sa_flags = SA_SIGINFO;
        sigaction(SIGSEGV, &sa, NULL);
    }
#endif
    load_image();
    setup_dos_env();
    ext_module_init();          /* load + register the extender KDV-player module (re_out/fist_ext.c) */
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
