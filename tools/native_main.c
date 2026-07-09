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
    if (g_deadline_ms > 0) {
        struct timeval tv; gettimeofday(&tv, 0);
        long now = tv.tv_sec*1000L + tv.tv_usec/1000L;
        if (now >= g_deadline_ms) {
            const char *fb = getenv("FIST_FBDUMP");
            fprintf(stderr, "[fist] FIST_RUNMS watchdog: dumping frame + exiting (video-mode=0x%02x)\n",
                    fist_vga_mode());
            if (fb) fist_dump_framebuffer(fb);
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
            if (getenv("FIST_PALDUMP")) {
                uint16_t pseg = *(uint16_t*)(g_mem+0x1c782);
                uint8_t *pb = g_mem + ((uint32_t)pseg<<4);
                fprintf(stderr,"[paldump] 0x782 seg=%04x buf[0..23]:", pseg);
                for (int i=0;i<24;i++) fprintf(stderr," %02x", pb[i]);
                fprintf(stderr,"\n[paldump] buf[36..47](idx12-15):");
                for (int i=36;i<48;i++) fprintf(stderr," %02x", pb[i]);
                fprintf(stderr,"  fade786=%02x\n", g_mem[0x1c786]);
            }
            _exit(0);
        }
    }
    /* Async vertical-retrace IRQ (palette upload): fires on the timer heartbeat regardless of whether
     * the engine's current loop polls in(0x3da) -- mirrors the original's async retrace ISR (FUN_0000_0b1f)
     * that uploads word[DGROUP:0x782] to the DAC ~70Hz.  Without this the DAC stays black in loops (e.g.
     * the main-menu idle) that never reach an explicit in(0x3da). */
    { extern void fist_vga_service_retrace(void); fist_vga_service_retrace(); }
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

int fist_extender_gate(void) {
    uint8_t *dg = g_mem + DGROUP_LIN;
    uint16_t op = *(uint16_t *)(dg + 0xea10);
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
