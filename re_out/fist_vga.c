/* re_out/fist_vga.c -- Armored Fist platform shim: VGA + port I/O.
 *
 * Runtime target of the Ghidra `in`/`out` port intrinsics. Implements the pieces the engine's boot
 * path touches: VGA DAC palette (ports 0x3C8/0x3C9), CRTC/Sequencer/GC/Attr (accepted), input-status
 * retrace poll (0x3DA, toggles so busy-waits terminate), PIT (0x40-0x43), keyboard (0x60/0x64), PIC
 * (0x20/0xA0). The framebuffer is linear 0xA0000 inside g_mem (VGA mode 13h). fist_dump_framebuffer()
 * writes it + the palette to a PPM so the rendered frame is observable.
 */
#include "ghidra_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../tools/oracle/fist_sequence_capture.h"
#include "fist_vga_bios_palette.h"
#include "fist_vga_text_font.h"
#include "fist_vga_text_palette.h"

#define VGA_FB   0xA0000u
#define FB_W 320
#define FB_H 200
#define FB_SZ (FB_W*FB_H)

static unsigned char g_pal[256][3];   /* DAC palette, 6-bit components (0..63) */
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
EMSCRIPTEN_KEEPALIVE unsigned char *fist_web_palette(void){ return &g_pal[0][0]; } /* 256*3 6-bit RGB */
/* Force the present MGAVIDEO palette buffer (word[DGROUP:0x782]) into g_pal, exactly like FIST_PALNOW
 * before a native dump.  In-mission the DAC-upload retrace-poll may not have run at the instant the web
 * frame is posted, leaving g_pal stale/black though the render + palette buffer are valid.  board:0001 */
EMSCRIPTEN_KEEPALIVE void fist_web_force_palette(void){
    unsigned pseg = *(unsigned short *)(g_mem + 0x1c782);
    if (!pseg) return;
    const unsigned char *pb = g_mem + ((unsigned)pseg << 4);
    for (int i = 0; i < 256; i++){ g_pal[i][0]=pb[i*3+0]&0x3f; g_pal[i][1]=pb[i*3+1]&0x3f; g_pal[i][2]=pb[i*3+2]&0x3f; }
}
#endif
static int g_vmode = -1;              /* last video mode set via INT 10h / this shim */
#define TEXT_SZ (80u * 25u * 2u)
static unsigned char *const g_text_cells = g_mem + 0xb8000u;

/* DAC state machine (ports 0x3C8 write-index, 0x3C7 read-index, 0x3C9 data) */
static int g_dac_widx, g_dac_wsub;    /* write index + sub-component (0=R,1=G,2=B) */
static int g_dac_ridx, g_dac_rsub;

static int g_trace = -1;
static int traceon(void){ if(g_trace<0){ extern char*getenv(const char*); g_trace=getenv("FIST_TRACE_TRAPS")?1:0;} return g_trace; }
static void fist_sequence_mode_set(void);

/* Sound Blaster shim (fist_sb.c): SB port window trapping.  Default OFF (FIST_SB unset) -> fist_sb_owns
 * returns 0 for every port -> the switch below runs exactly as before -> zero effect on the video flows. */
int  fist_sb_owns(int port);
int  fist_sb_in(int port);
void fist_sb_out(int port, int val);

/* OPL FM shim (fist_opl.c): port 0x388/0x389 trapping.  Default OFF (FIST_OPL/FIST_SB unset) ->
 * fist_opl_owns returns 0 -> zero effect on the video flows. */
int  fist_opl_owns(int port);
int  fist_opl_in(int port);
void fist_opl_out(int port, int val);

void fist_vga_set_mode(int mode)
{
    g_vmode = mode & 0xff;
    if (getenv("FIST_VGA_TRACE")) {
        extern unsigned long long fist_clock_now(void);
        fprintf(stderr, "[vga] mode %02x t=%.6f\n", g_vmode,
                (double)fist_clock_now() * 1000.0 / 1193182.0);
        fprintf(stderr, "[vga] vectors 426=%08x 53c=%08x d8b4=%u\n",
                *(uint32_t *)(g_mem + 0x1c426), *(uint32_t *)(g_mem + 0x1c53c),
                g_mem[0x1d8b4]);
    }
    if (traceon()) fprintf(stderr, "[vga] set video mode 0x%02x (%s)\n", g_vmode,
        g_vmode==0x13 ? "320x200x256 linear" : "other");
    /* mode set clears the display in real VGA; zero the aperture the engine will draw into. */
    if (g_vmode==0x13) {
        memcpy(g_pal, fist_mode13_dac, sizeof g_pal);
        memset(g_mem+VGA_FB, 0, FB_SZ);
        if (getenv("FIST_VGA_TRACE"))
            fprintf(stderr, "[vga] mode13 DAC17=%u,%u,%u\n",
                    g_pal[17][0], g_pal[17][1], g_pal[17][2]);
    }
    fist_sequence_mode_set();
}
int  fist_vga_mode(void){ return g_vmode; }

/* ================= port I/O ================= */

/* ---- The machine clock: the PIT and the VGA retrace on one virtual time line (board:0026) ----
 *
 * The engine makes the PIT interrupt its vertical-retrace interrupt: 2fd3 measures one retrace period in
 * PIT counts (30de polls 0x3da between two retrace edges while the counter free-runs) into [0x44c], 3064
 * scales the 60 Hz frame tick [0x452] from it (d8b8 = [0x44c] << 16 / 0x4dae), and the ISR 30f8 waits
 * for the retrace and re-programs the PIT on every interrupt.  So time is ONE thing here: a count of PIT
 * clocks (1193182 Hz).  Every port access and every cooperative pump costs one count (an infinitely fast
 * CPU whose I/O takes 0.84 us -- DOSBox at cycles=max is that machine too); the PIT channel 0 counter and
 * the VGA status derive from the count; the INT-8 fires when the channel-0 counter wraps.
 *
 * The oracle machine (DOSBox, mode 13h): htotal 100 chars at 25.175/8 MHz, vtotal 449 lines -> 70.086 Hz,
 * 17024.6 counts per frame; the retrace pulse is lines 412..414 (int10_modes.cpp: vrstart = vdispend+12,
 * vrend = vrstart+2), 76 counts.  2fd3 measures 0x427f = 17023 there (every RAM capture agrees), so the
 * model's frame is FRAME_COUNTS long such that the same measurement reads 17023 here: the two 30de returns
 * are exact retrace-start edges, the reload (3 outs) and the latch (1 out) sit between them, and the
 * count read is 65536 - (P - 2) -> P = 17025.
 *
 * Each status poll consumes one count. The timeout at 30f8 depends on the number of reads, even when
 * their returned status is unchanged. fist_clock_advance() fires every channel-0 wrap in order. */
#define PIT_HZ_       1193182u
#define FRAME_COUNTS  17025u          /* one mode-13h frame, see above */
/* DOSBox retains the loader's 9-dot VGA clock through the mode-13h switch. */
#define VGA_CLOCK_    (28322000u / 9u)
#define VGA_FRAME_NUM (100ull * 449u * PIT_HZ_)
#define FRAME_LINES   449.0           /* vtotal */
#define VRETRACE_LINE 412             /* vrstart (vdispend + 12); the pulse lasts to line 414 */
#define VDISPEND_LINE 400             /* status bit 0 (blanking) from here to the end of the frame */
static unsigned long long g_clock;            /* PIT counts since power-on */
static unsigned long long g_sequence_next;
static unsigned long long g_sequence_vertical_num;
static unsigned long long g_sequence_event_num;
static unsigned char g_sequence_pixels[640 * 400];
static unsigned g_sequence_part;
static int g_sequence_dispatch;

void fist_text_init(void)
{
    for (unsigned i = 0; i < 80 * 25; ++i) {
        g_text_cells[2 * i] = ' ';
        g_text_cells[2 * i + 1] = 7;
    }
    g_mem[0x449] = 3;
    g_mem[0x44a] = 80;
    g_mem[0x44b] = 0;
    g_mem[0x450] = g_mem[0x451] = g_mem[0x462] = 0;
    const char *prefix = getenv("FIST_TEXT_STATE");
    if (prefix) {
        char path[1024];
        if (snprintf(path, sizeof path, "%s.text", prefix) >= (int)sizeof path) abort();
        FILE *f = fopen(path, "rb");
        if (!f || fread(g_text_cells, 1, TEXT_SZ, f) != TEXT_SZ ||
            fgetc(f) != EOF || fclose(f)) abort();
        if (snprintf(path, sizeof path, "%s.bda", prefix) >= (int)sizeof path) abort();
        unsigned char bda[256];
        f = fopen(path, "rb");
        if (!f || fread(bda, 1, sizeof bda, f) != sizeof bda || fgetc(f) != EOF || fclose(f)) abort();
        if (bda[0x49] != 3 || bda[0x4a] != 80 || bda[0x4b] || bda[0x62] ||
            bda[0x51] >= 25 || bda[0x50] >= 80) abort();
        memcpy(g_mem + 0x400, bda, sizeof bda);
    }
    g_vmode = 3;
    memcpy(g_pal, fist_text_dac, sizeof g_pal);
    fist_sequence_mode_set();
}

void fist_text_write(unsigned ch)
{
    if (g_vmode != 3) return;
    unsigned row = g_mem[0x451], col = g_mem[0x450];
    switch (ch & 0xff) {
    case 7: break;
    case 8: if (col) --col; break;
    case '\r': col = 0; break;
    case '\n': ++row; break;
    default:
        g_text_cells[2 * (row * 80 + col)] = ch;
        if (++col == 80) { col = 0; ++row; }
        break;
    }
    if (row == 25) {
        memmove(g_text_cells, g_text_cells + 160, TEXT_SZ - 160);
        for (unsigned i = TEXT_SZ - 160; i < TEXT_SZ; i += 2) {
            g_text_cells[i] = ' ';
            g_text_cells[i + 1] = 7;
        }
        --row;
    }
    g_mem[0x450] = col;
    g_mem[0x451] = row;
}

static void fist_text_scan_part(unsigned part)
{
    for (unsigned y = part * 100; y < (part + 1) * 100; ++y) {
        unsigned char *dst = g_sequence_pixels + y * 640;
        for (unsigned col = 0; col < 80; ++col) {
            unsigned cell = 2 * ((y / 16) * 80 + col);
            unsigned ch = g_text_cells[cell], attr = g_text_cells[cell + 1];
            unsigned glyph = fist_text_font[ch * 16 + y % 16];
            for (unsigned x = 0; x < 8; ++x)
                dst[col * 8 + x] = glyph & (0x80u >> x) ? attr & 15 : (attr >> 4) & 7;
        }
    }
}

/* DOSBox VGA_DrawPart samples four 50-row bands before RENDER_EndUpdate. */
static unsigned long long fist_sequence_part_clock(unsigned part)
{
    g_sequence_event_num = g_sequence_vertical_num +
        VGA_FRAME_NUM * VDISPEND_LINE * part / (449u * 4u);
    return (g_sequence_event_num + VGA_CLOCK_ - 1) / VGA_CLOCK_;
}

static void fist_sequence_mode_set(void)
{
    g_sequence_next = 0;
    if ((g_vmode != 0x13 && g_vmode != 3) || !getenv("FIST_SEQUENCE")) return;
    unsigned long long ready = g_clock + (g_vmode == 0x13 ? (50ull * PIT_HZ_ + 500) / 1000 : 0);
    g_sequence_vertical_num = (ready / FRAME_COUNTS + 1) * FRAME_COUNTS * (unsigned long long)VGA_CLOCK_;
    g_sequence_part = 0;
    g_sequence_next = fist_sequence_part_clock(1);
}

void fist_sequence_present(void)
{
    if (!g_sequence_dispatch || !getenv("FIST_SEQUENCE") ||
        (g_vmode != 0x13 && g_vmode != 3)) return;
    unsigned char palette[256][4];
    for (unsigned i = 0; i < 256; ++i)
        for (unsigned lane = 0; lane < 3; ++lane)
            palette[i][lane] = (unsigned char)((g_pal[i][lane] << 2) | (g_pal[i][lane] >> 4));
    unsigned width = g_vmode == 3 ? 640 : FB_W, height = g_vmode == 3 ? 400 : FB_H;
    fist_sequence_frame((double)g_sequence_event_num * 1000.0 / (PIT_HZ_ * (double)VGA_CLOCK_), width, height, width,
                        g_sequence_pixels, &palette[0][0]);
}
void fist_sequence_finish(void){ fist_sequence_close(); }
void fist_kdv_instruction_count(uint64_t count)
{
    const char *path = getenv("FIST_KDV_INSTRLOG");
    if (!path) return;
    static FILE *log;
    static unsigned frame;
    if (!log) {
        log = fopen(path, "w");
        if (!log) abort();
    }
    fprintf(log, "%u %llu\n", ++frame, (unsigned long long)count);
    fflush(log);
}
static unsigned short g_pit_reload[3] = {0,0,0};   /* 0 == 65536 */
static unsigned char  g_pit_mode[3], g_pit_rw[3];  /* control word: mode, access (1 lo,2 hi,3 lo/hi) */
static unsigned char  g_pit_wsub[3], g_pit_rsub[3];
static unsigned short g_pit_wlatch[3];
static unsigned long long g_pit_base[3];      /* clock at which the current count started */
static int            g_pit_latched[3]; static unsigned short g_pit_latch[3];
int fist_vga_pit0_div(void){ return g_pit_reload[0] ? g_pit_reload[0] : 0x10000; }
unsigned long long fist_clock_now(void){ return g_clock; }
unsigned fist_clock_frame_counts(void){ return FRAME_COUNTS; }
static unsigned pit_period(int ch){ return g_pit_reload[ch] ? g_pit_reload[ch] : 0x10000u; }
static unsigned pit_count(int ch){            /* the channel's current count (modes 2/3: reload - elapsed) */
    unsigned p = pit_period(ch); unsigned long long e = (g_clock - g_pit_base[ch]) % p;
    return (unsigned)(p - e) & 0xffff; }
unsigned long long fist_pit0_next_wrap(void){ unsigned p = pit_period(0);
    unsigned long long e = g_clock - g_pit_base[0]; return g_pit_base[0] + (e / p + 1) * p; }
/* Step the clock to `target`, firing the channel-0 interrupt at every wrap on the way (the ISR may
 * re-program the channel, which restarts the count from that instant, as on the 8253). */
int g_int8_replay;   /* board:0017 FIST_FRAME_SCHEDULE: the INT-8s come from the schedule, not the clock */
int g_int8_force;    /* ... except from an explicit spin-wait pump (a fade, a delay): those need the interrupt */
void fist_clock_advance_to(unsigned long long target){
    extern void fist_int8_fire(void);
    while (g_clock < target) {
        unsigned long long w = fist_pit0_next_wrap();
        if (g_sequence_next && g_sequence_next <= target && g_sequence_next <= w) {
            int same_tick = g_sequence_next == w;
            g_clock = g_sequence_next;
            if (g_vmode == 3) fist_text_scan_part(g_sequence_part);
            else memcpy(g_sequence_pixels + g_sequence_part * FB_SZ / 4,
                        g_mem + VGA_FB + g_sequence_part * FB_SZ / 4, FB_SZ / 4);
            if (++g_sequence_part == 4) {
                g_sequence_dispatch = 1;
                fist_sequence_present();
                g_sequence_dispatch = 0;
                g_sequence_part = 0;
                g_sequence_vertical_num += VGA_FRAME_NUM;
            }
            g_sequence_next = fist_sequence_part_clock(g_sequence_part + 1);
            if (same_tick && (!g_int8_replay || g_int8_force)) fist_int8_fire();
        }
        else if (w <= target) { g_clock = w; if (!g_int8_replay || g_int8_force) fist_int8_fire(); }
        else g_clock = target;
    }
}
void fist_clock_advance(unsigned n){ fist_clock_advance_to(g_clock + n); }
static int vga_status(unsigned long long c){   /* port 0x3da at clock c: bit3 vsync, bit0 vertical blanking */
    double line = (double)(unsigned)(c % FRAME_COUNTS) * (FRAME_LINES / FRAME_COUNTS); int r = 0;
    if (line >= VRETRACE_LINE && line <= VRETRACE_LINE + 2) r |= 8;
    if (line >= VDISPEND_LINE) r |= 1;                   /* (the per-line horizontal blank is not modelled:
                                                            nothing in the engine or the drivers reads bit 0) */
    return r;
}
/* The MGAVIDEO palette upload and the per-tick DAC service are the ENGINE's: its INT-8 handler
 * (FUN_1000_31c3) far-calls the driver method at DGROUP:0x5e4 once per tick -- 0be2 on a machine
 * LOADGAME rates >= 0x31 (patch 575: the rating arrives through the LOADGAME hand-off script, see
 * native_main.c setup_dos_env) -- which uploads word[0x782] to the DAC when bit0 of [0x786] is set,
 * runs the DAC animation list and the fade stepper at [0x5e8].  The shim used to stand in for that
 * ISR here (an upload per pump and per in(0x3da) poll, no animation list); with the driver's own
 * method live the stand-in is gone, and in(0x3da) is only the retrace-status toggle. */

int in(int port)
{
    fist_timer_pump();   /* one PIT count of machine time, and the INT-8 it may bring (board:0026) */
    port &= 0xffff;
    if (fist_opl_owns(port)) return fist_opl_in(port);  /* OPL FM 0x388 status (FIST_OPL/FIST_SB) */
    if (fist_sb_owns(port)) return fist_sb_in(port);   /* SB DSP + 8237 DMA window (FIST_SB, default off) */
    switch (port) {
    case 0x3c7: return 0;
    case 0x3c8: return g_dac_widx & 0xff;
    case 0x3c9: { /* DAC data read */
        int v = g_pal[g_dac_ridx & 0xff][g_dac_rsub];
        if (++g_dac_rsub==3){ g_dac_rsub=0; g_dac_ridx=(g_dac_ridx+1)&0xff; }
        return v & 0x3f; }
    case 0x3da: case 0x3ba: return vga_status(g_clock);
    case 0x40: case 0x41: case 0x42: { /* PIT counter read: the latched value, else the live count */
        int ch=port-0x40; unsigned v = g_pit_latched[ch] ? g_pit_latch[ch] : pit_count(ch);
        int b;
        if (g_pit_rw[ch] == 1) b = v & 0xff;
        else if (g_pit_rw[ch] == 2) b = (v >> 8) & 0xff;
        else { b = g_pit_rsub[ch] ? (v >> 8) & 0xff : v & 0xff; g_pit_rsub[ch] ^= 1; }
        if (g_pit_rw[ch] != 3 || !g_pit_rsub[ch]) g_pit_latched[ch] = 0;   /* both bytes read: unlatch */
        return b; }
    case 0x60: return 0;        /* keyboard data: no scan code */
    case 0x61: return 0x20;     /* PPI port B (refresh toggle bit) */
    case 0x64: return 0x00;     /* keyboard status: no data available */
    case 0x201: return 0xf0;    /* joystick: no buttons pressed, timers low */
    case 0x20: case 0xa0: return 0;   /* PIC */
    default:
        if (traceon()) fprintf(stderr, "[port] in  0x%03x -> 0\n", port);
        return 0;
    }
}

void out(int port, int val)
{
    fist_timer_pump();   /* one PIT count of machine time (see in()) */
    port &= 0xffff; val &= 0xff;
    if (fist_opl_owns(port)) { fist_opl_out(port, val); return; }  /* OPL FM 0x388/0x389 (FIST_OPL/FIST_SB) */
    if (fist_sb_owns(port)) { fist_sb_out(port, val); return; }   /* SB DSP + 8237 DMA (FIST_SB, default off) */
    switch (port) {
    case 0x3c8:
        if (getenv("FIST_VGA_TRACE") && g_vmode == 0x13 && val == 0)
#ifndef __EMSCRIPTEN__
            fprintf(stderr, "[vga] DAC reset t=%.6f c452=%u caller=%p\n",
                (double)g_clock * 1000.0 / PIT_HZ_, *(uint16_t *)(g_mem + 0x1c452),
                __builtin_return_address(0));
#else
            fprintf(stderr, "[vga] DAC reset t=%.6f c452=%u\n",
                (double)g_clock * 1000.0 / PIT_HZ_, *(uint16_t *)(g_mem + 0x1c452));
#endif
        g_dac_widx = val; g_dac_wsub = 0; return;     /* set DAC write index */
    case 0x3c7: g_dac_ridx = val; g_dac_rsub = 0; return;     /* set DAC read index */
    case 0x3c9: /* DAC data write: R,G,B (6-bit) */
        g_pal[g_dac_widx & 0xff][g_dac_wsub] = (unsigned char)(val & 0x3f);
        if (++g_dac_wsub==3){ g_dac_wsub=0; g_dac_widx=(g_dac_widx+1)&0xff; }
        return;
    case 0x40: case 0x41: case 0x42: { /* PIT counter load: the new period starts when the write completes */
        int ch=port-0x40; int done = 0;
        if (g_pit_rw[ch] == 1) { g_pit_wlatch[ch] = (unsigned short)val; done = 1; }
        else if (g_pit_rw[ch] == 2) { g_pit_wlatch[ch] = (unsigned short)(val << 8); done = 1; }
        else if (!g_pit_wsub[ch]) { g_pit_wlatch[ch] = (unsigned short)((g_pit_wlatch[ch] & 0xff00) | val); g_pit_wsub[ch] = 1; }
        else { g_pit_wlatch[ch] = (unsigned short)((g_pit_wlatch[ch] & 0x00ff) | (val << 8)); g_pit_wsub[ch] = 0; done = 1; }
        if (done) {
            g_pit_reload[ch] = g_pit_wlatch[ch]; g_pit_base[ch] = g_clock;
            if (ch == 0 && getenv("FIST_VGA_TRACE") && g_clock < 200u * (PIT_HZ_ / 1000u))
#ifndef __EMSCRIPTEN__
                fprintf(stderr, "[vga] PIT0 reload=%u t=%.6f caller=%p\n", pit_period(0),
                        (double)g_clock * 1000.0 / PIT_HZ_, __builtin_return_address(0));
#else
                fprintf(stderr, "[vga] PIT0 reload=%u t=%.6f\n", pit_period(0),
                        (double)g_clock * 1000.0 / PIT_HZ_);
#endif
        }
        return; }
    case 0x43: {         /* PIT control word: channel, access mode, counting mode; access 0 = latch */
        int ch = (val >> 6) & 3, rw = (val >> 4) & 3;
        if (ch == 3) return;
        if (rw == 0) { g_pit_latch[ch] = (unsigned short)pit_count(ch); g_pit_latched[ch] = 1; g_pit_rsub[ch] = 0; return; }
        g_pit_rw[ch] = (unsigned char)rw; g_pit_mode[ch] = (unsigned char)((val >> 1) & 7);
        g_pit_wsub[ch] = 0; g_pit_rsub[ch] = 0; g_pit_latched[ch] = 0;
        return; }
    case 0x3c0: case 0x3c1: /* attribute controller */
    case 0x3c2: case 0x3c3: /* misc output / feature */
    case 0x3c4: case 0x3c5: /* sequencer (map mask etc.) */
    case 0x3ce: case 0x3cf: /* graphics controller */
    case 0x3d4: case 0x3d5: /* CRTC */
    case 0x3d8: case 0x3d9: /* mode/color select */
    case 0x20: case 0xa0:   /* PIC EOI */
    case 0x21: case 0xa1:   /* PIC mask */
    case 0x61: case 0x64:   /* PPI / kbd cmd */
        return;
    default:
        if (traceon()) fprintf(stderr, "[port] out 0x%03x, 0x%02x\n", port, val);
        return;
    }
}

/* ================= framebuffer dump (FIST_FBDUMP=path) ================= */
/* Writes the 320x200 mode-13h aperture at 0xA0000, mapped through the DAC palette, as a binary PPM.
 * Returns the number of non-zero pixels (a cheap "did the engine draw anything" signal). */
long fist_dump_framebuffer(const char *path)
{
    /* FIST_PALNOW: force the MGAVIDEO palette buffer (word[DGROUP:0x782]) into the DAC before dumping,
       to observe a frame whose render completed but whose retrace-poll palette upload has not yet fired
       (e.g. a crash later in the same frame).  Diagnostic only. */
    if (getenv("FIST_PALNOW")) {
        unsigned pseg = *(unsigned short *)(g_mem + 0x1c782);
        const unsigned char *pb = g_mem + ((unsigned)pseg << 4);
        for (int i = 0; i < 256; i++) {
            g_pal[i][0] = pb[i*3+0] & 0x3f;
            g_pal[i][1] = pb[i*3+1] & 0x3f;
            g_pal[i][2] = pb[i*3+2] & 0x3f;
        }
    }
    FILE *f = fopen(path, "wb");
    if(!f){ fprintf(stderr,"[fb] cannot write %s\n", path); return -1; }
    fprintf(f, "P6\n%d %d\n255\n", FB_W, FB_H);
    const unsigned char *fb = g_mem + VGA_FB;
    long nonzero = 0;
    unsigned char row[FB_W*3];
    for (int y=0; y<FB_H; ++y){
        for (int x=0; x<FB_W; ++x){
            unsigned char idx = fb[y*FB_W + x];
            if (idx) nonzero++;
            /* 6-bit DAC -> 8-bit via VGA hardware bit-replication (v<<2)|(v>>4) -- the same expansion
             * DOSBox/real VGA use for a screenshot (e.g. 0x33 -> 0xCF=207), so the dump is pixel-comparable
             * to the DOSBox reference.  (v*255/63 truncation was off-by-one on most non-saturated values.) */
            row[x*3+0] = (unsigned char)((g_pal[idx][0]<<2)|(g_pal[idx][0]>>4));
            row[x*3+1] = (unsigned char)((g_pal[idx][1]<<2)|(g_pal[idx][1]>>4));
            row[x*3+2] = (unsigned char)((g_pal[idx][2]<<2)|(g_pal[idx][2]>>4));
        }
        fwrite(row,1,sizeof row,f);
    }
    fclose(f);
    /* palette-usage + distinct-index histogram for a quick console signal */
    int distinct=0; { char seen[256]={0}; for(int i=0;i<FB_SZ;i++){ unsigned char v=fb[i]; if(!seen[v]){seen[v]=1;distinct++;} } }
    fprintf(stderr, "[fb] dumped %s : mode=0x%02x nonzero=%ld/%d distinct-indices=%d\n",
            path, g_vmode, nonzero, FB_SZ, distinct);
    return nonzero;
}
