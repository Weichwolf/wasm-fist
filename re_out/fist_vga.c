/* re_out/fist_vga.c -- Armored Fist platform shim: VGA + port I/O.
 *
 * Runtime target of the Ghidra `in`/`out` port intrinsics. Implements the pieces the engine's boot
 * path touches: VGA DAC palette (ports 0x3C8/0x3C9), CRTC/Sequencer/GC/Attr (accepted), input-status
 * retrace poll (0x3DA, toggles so busy-waits terminate), PIT (0x40-0x43), keyboard (0x60/0x64), PIC
 * (0x20/0xA0). The framebuffer is linear 0xA0000 inside g_mem (VGA mode 13h). fist_dump_framebuffer()
 * writes it + the palette to a PPM so the rendered frame is observable.
 */
#include "ghidra_compat.h"
#include "fist_pic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../tools/oracle/fist_sequence_endpoint.h"
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
#include "fist_sb.h"

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

/* Preserve CPU phase below a PIT count; I/O/pump and fallback retrace costs remain approximate. */
#define PIT_HZ_       1193182u
#define CPU_HZ_       30000000u
#define FRAME_COUNTS  17025u
/* SetupDrawing retains vtotal when the mode-switch difference is below 0.0001 ms. */
#define VGA_CLOCK_    (28322000u / 9u)
#define VGA_MODE13_CLOCK_ (25175000u / 8u)
#define VGA_FRAME_NUM (100ull * 449u * PIT_HZ_)
#define FRAME_LINES   449.0           /* vtotal */
#define VRETRACE_LINE 412             /* vrstart (vdispend + 12); the pulse lasts to line 414 */
#define VDISPEND_LINE 400             /* status bit 0 (blanking) from here to the end of the frame */
static unsigned long long g_clock;            /* PIT counts since power-on */
static unsigned g_clock_fraction;             /* fractional count, denominator 30000000 */
typedef struct { unsigned long long counts; unsigned fraction; } FistClock;
static FistClock g_cpu_time = {UINT64_MAX, 0};
static unsigned g_cpu_remaining;
static FistClock clock_now(void){ return (FistClock){g_clock, g_clock_fraction}; }
static void clock_set(FistClock time){ g_clock = time.counts; g_clock_fraction = time.fraction; }
static int clock_equal(FistClock a, FistClock b){ return a.counts == b.counts && a.fraction == b.fraction; }
static FistClock clock_after_cpu(unsigned count){
    uint64_t numerator = (uint64_t)g_clock_fraction + (uint64_t)count * PIT_HZ_;
    return (FistClock){g_clock + numerator / CPU_HZ_, (unsigned)(numerator % CPU_HZ_)};
}
static int clock_before(FistClock a, FistClock b){
    return a.counts < b.counts || (a.counts == b.counts && a.fraction < b.fraction);
}
static unsigned long long g_sequence_next;
static unsigned long long g_sequence_vertical_num;
static unsigned long long g_sequence_event_num;
static unsigned long long g_sequence_event_cycles;
static unsigned long long g_sequence_pic_tick;
static unsigned long long g_sequence_pic_part_tick;
static unsigned long long g_sequence_pic_previous_tick;
static float g_sequence_pic_previous_lag;
static int g_sequence_pic_previous_ready;
static int g_sequence_pic_previous_mode;
static unsigned long long g_text_vertical_num;
static unsigned long long g_text_pic_tick;
static float g_text_pic_lag;
static unsigned long long g_sequence_resize_ready;
static int g_text_phase_set;
static int g_sequence_mode = -1;
static int g_sequence_blank;
static unsigned g_text_frame_count;
static unsigned char g_sequence_pixels[640 * 400];
static unsigned g_sequence_part;
static int g_sequence_dispatch;
static float g_sequence_pic_vertical_lag;
static float g_sequence_pic_part_lag;
static int g_sequence_pic_ready;

#define VGA_TEXT_PART_MS   ((float)(100.0 * 100 * 1000 / VGA_CLOCK_))
#define VGA_MODE13_PART_MS ((float)(100.0 * 100 * 1000 / VGA_MODE13_CLOCK_))
#define VGA_VERTICAL_MS    ((float)(100.0 * 449 * 1000 / VGA_CLOCK_))

/* DOSBox queues float residuals and exposes its float PIC tick in the capture callback. */
static void fist_sequence_pic_step(float delay)
{
    float next = delay + g_sequence_pic_part_lag;
    unsigned whole = (unsigned)next;
    g_sequence_pic_part_tick += whole;
    g_sequence_pic_part_lag = next - whole;
}

static void fist_sequence_pic_advance_vertical(void)
{
    g_sequence_pic_previous_tick = g_sequence_pic_tick;
    g_sequence_pic_previous_lag = g_sequence_pic_vertical_lag;
    g_sequence_pic_previous_ready = g_sequence_pic_ready;
    g_sequence_pic_previous_mode = g_sequence_mode;
    float next = VGA_VERTICAL_MS + g_sequence_pic_vertical_lag;
    unsigned whole = (unsigned)next;
    g_sequence_pic_tick += whole;
    g_sequence_pic_vertical_lag = next - whole;
}

static void fist_sequence_pic_part(unsigned part)
{
    if (part == 1) {
        g_sequence_pic_part_tick = g_sequence_pic_tick;
        g_sequence_pic_part_lag = g_sequence_pic_vertical_lag;
    }
    fist_sequence_pic_step(g_sequence_mode == 0x13 ? VGA_MODE13_PART_MS : VGA_TEXT_PART_MS);
    float cycles = g_sequence_pic_part_lag * 30000.0f;
    unsigned long long fractional = (unsigned long long)cycles;
    if ((float)fractional < cycles) ++fractional;
    g_sequence_event_cycles = g_sequence_pic_part_tick * 30000u + fractional;
}

static int g_text_clock_initialized;
void fist_text_clock_init(void)
{
    if (g_text_clock_initialized) return;
    const char *prefix = getenv("FIST_TEXT_STATE");
    if (prefix) {
        char path[1024];
        FILE *f;
        unsigned long long start_ns, vertical_ns;
        if (snprintf(path, sizeof path, "%s.vga", prefix) >= (int)sizeof path) abort();
        f = fopen(path, "rb");
        if (!f || fscanf(f, "FISTVGA1\n%llu %llu\n%llu %a", &start_ns, &vertical_ns,
                         &g_text_pic_tick, &g_text_pic_lag) != 4 ||
            fgetc(f) != '\n' || fgetc(f) != EOF || fclose(f) ||
            g_text_pic_tick > UINT32_MAX || !(g_text_pic_lag >= 0 && g_text_pic_lag < 1) ||
            vertical_ns > start_ns ||
            start_ns > (UINT64_MAX - 500000000ull) / PIT_HZ_) abort();
        /* The fixture rounds a fixed-core CPU timestamp to nanoseconds. */
        uint64_t start_cpu = (start_ns / 100) * 3 + ((start_ns % 100) * 3 + 50) / 100;
        uint64_t start_num = start_cpu * PIT_HZ_;
        clock_set((FistClock){start_num / CPU_HZ_, start_num % CPU_HZ_});
        unsigned long long vertical_num = vertical_ns * PIT_HZ_;
        unsigned long long whole = vertical_num / 1000000000ull;
        unsigned long long frac = vertical_num % 1000000000ull;
        if (whole > (UINT64_MAX - VGA_CLOCK_) / VGA_CLOCK_) abort();
        g_text_vertical_num = whole * VGA_CLOCK_ +
            (frac * VGA_CLOCK_ + 500000000ull) / 1000000000ull;
        g_text_phase_set = 1;
    }
    g_vmode = 3;
    memcpy(g_pal, fist_text_dac, sizeof g_pal);
    fist_sequence_mode_set();
    g_text_clock_initialized = 1;
}

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
    g_mem[0x460] = 7;
    g_mem[0x461] = 6;
    g_mem[0x485] = 16;
    g_mem[0x487] = 0x60;
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
    fist_text_clock_init();
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
    unsigned col = g_mem[0x450], row = g_mem[0x451];
    unsigned start = g_mem[0x461], end = g_mem[0x460];
    int cursor = (g_text_frame_count & 8) && col < 80 && row < 25 && g_mem[0x485] &&
        (start & 0x60) != 0x20;
    if (cursor && !(g_mem[0x487] & 9) && !((start | end) & 0xe0)) {
        unsigned height = g_mem[0x485] - 1;
        if (end < start) {
            if (end) { start = end; end = height; }
        } else if ((start | end) >= height || end != height - 1 || start != height) {
            if (end > 3) {
                if (start + 2 < end) {
                    if (start > 2) start = (height + 1) / 2;
                    end = height;
                } else {
                    start = start - end + height;
                    end = height;
                    if (height > 12) { --start; --end; }
                }
            }
        }
    }
    for (unsigned y = part * 100; y < (part + 1) * 100; ++y) {
        unsigned char *dst = g_sequence_pixels + y * 640;
        for (unsigned xcell = 0; xcell < 80; ++xcell) {
            unsigned cell = 2 * ((y / 16) * 80 + xcell);
            unsigned ch = g_text_cells[cell], attr = g_text_cells[cell + 1];
            unsigned glyph = fist_text_font[ch * 16 + y % 16];
            for (unsigned x = 0; x < 8; ++x)
                dst[xcell * 8 + x] = glyph & (0x80u >> x) ? attr & 15 : (attr >> 4) & 7;
        }
        if (cursor && y / 16 == row && y % 16 >= start && y % 16 <= end)
            memset(dst + col * 8, g_text_cells[2 * (row * 80 + col) + 1] & 15, 8);
    }
}

/* DOSBox VGA_DrawPart samples four 50-row bands before RENDER_EndUpdate. */
static unsigned long long fist_sequence_part_clock(unsigned part)
{
    if (g_sequence_pic_ready) fist_sequence_pic_part(part);
    g_sequence_event_num = g_sequence_vertical_num +
        VGA_FRAME_NUM * VDISPEND_LINE * part / (449u * 4u);
    const unsigned long long denom = (unsigned long long)PIT_HZ_ * VGA_CLOCK_;
    unsigned long long pit = g_sequence_event_num / VGA_CLOCK_;
    unsigned long long pit_fraction = g_sequence_event_num % VGA_CLOCK_;
    unsigned long long seconds = pit / PIT_HZ_, remainder = pit % PIT_HZ_;
    unsigned long long scaled = remainder * CPU_HZ_;
    unsigned long long fractional = (scaled % PIT_HZ_) * VGA_CLOCK_ + pit_fraction * CPU_HZ_;
    unsigned long long scanout_cycles = seconds * CPU_HZ_ + scaled / PIT_HZ_ +
        (fractional + denom - 1) / denom;
    if (!g_sequence_pic_ready) g_sequence_event_cycles = scanout_cycles;
    return (scanout_cycles / CPU_HZ_) * PIT_HZ_ +
        ((scanout_cycles % CPU_HZ_) * PIT_HZ_ + CPU_HZ_ - 1) / CPU_HZ_;
}

static void fist_sequence_mode_set(void)
{
    if (g_vmode != 0x13 && g_vmode != 3) return;
    g_cpu_time.counts = UINT64_MAX;
    if (g_vmode == 0x13 && g_sequence_mode == 3 && g_sequence_next) {
        g_sequence_resize_ready = g_clock + (50ull * PIT_HZ_ + 500) / 1000;
        g_sequence_blank = 1;
        return;
    }
    g_sequence_next = 0;
    g_sequence_resize_ready = 0;
    g_sequence_mode = g_vmode;
    g_sequence_blank = 0;
    g_sequence_pic_previous_ready = 0;
    unsigned long long ready = g_clock;
    g_sequence_vertical_num = g_vmode == 3 && g_text_phase_set ? g_text_vertical_num :
        (ready / FRAME_COUNTS + 1) * FRAME_COUNTS * (unsigned long long)VGA_CLOCK_;
    if (g_vmode == 3 && g_text_phase_set) {
        g_sequence_pic_tick = g_text_pic_tick;
        g_sequence_pic_vertical_lag = g_text_pic_lag;
        g_sequence_pic_ready = 1;
    }
    g_sequence_part = 0;
    g_text_frame_count = 1;
    g_sequence_next = fist_sequence_part_clock(1);
    if (g_vmode == 3 && g_text_phase_set && g_sequence_next <= g_clock) abort();
}

void fist_sequence_present(void)
{
    if (!g_sequence_dispatch || !getenv("FIST_SEQUENCE") ||
        (g_vmode != 0x13 && g_vmode != 3)) return;
    unsigned char palette[256][4];
    for (unsigned i = 0; i < 256; ++i)
        for (unsigned lane = 0; lane < 3; ++lane)
            palette[i][lane] = (unsigned char)((g_pal[i][lane] << 2) | (g_pal[i][lane] >> 4));
    unsigned width = g_sequence_mode == 3 ? 640 : FB_W, height = g_sequence_mode == 3 ? 400 : FB_H;
    unsigned long long time = (g_sequence_event_cycles + 15u) / 30u;
    if (g_sequence_pic_ready) {
        unsigned long long milliseconds = g_sequence_event_cycles / 30000u;
        float fraction = (float)(g_sequence_event_cycles % 30000u) / 30000.0f;
        time = (unsigned long long)(((double)milliseconds + (double)fraction) * 1000.0 + 0.5);
    }
    fist_sequence_frame_us(time, width, height, width, g_sequence_pixels, &palette[0][0]);
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
static unsigned char  g_pit_mode[3] = {3,0,0}, g_pit_rw[3];  /* BIOS channel 0 starts in mode 3 */
static unsigned char  g_pit_wsub[3], g_pit_rsub[3];
static unsigned short g_pit_wlatch[3];
static FistClock g_pit_base[3];
static int            g_pit_latched[3]; static unsigned short g_pit_latch[3];
static unsigned char g_port61;
int fist_vga_pit0_div(void){ return g_pit_reload[0] ? g_pit_reload[0] : 0x10000; }
unsigned long long fist_clock_now(void){ return g_clock; }
unsigned fist_clock_frame_counts(void){ return FRAME_COUNTS; }
static unsigned pit_period(int ch){ return g_pit_reload[ch] ? g_pit_reload[ch] : 0x10000u; }
static unsigned pit_count(int ch){
    unsigned p = pit_period(ch);
    double elapsed = (double)(g_clock - g_pit_base[ch].counts) +
        ((double)g_clock_fraction - g_pit_base[ch].fraction) / CPU_HZ_;
    if (g_pit_mode[ch] == 2 || g_pit_mode[ch] == 3) {
        float frequency = (float)PIT_HZ_ / (float)p;
        float delay = 1000.0f / frequency;
        double index = fmod(elapsed * 1000.0 / PIT_HZ_, delay);
        if (g_pit_mode[ch] == 3) {
            index *= 2;
            if (index > delay) index -= delay;
        }
        unsigned count = (unsigned)(p - (index / delay) * p);
        return count & (g_pit_mode[ch] == 3 ? 0xfffe : 0xffff);
    }
    unsigned e = (unsigned long long)elapsed % p;
    return (p - e) & 0xffff;
}
static FistClock pit_next_wrap(void){
    unsigned p = pit_period(0);
    unsigned long long e = g_clock - g_pit_base[0].counts;
    if (g_clock_fraction < g_pit_base[0].fraction) --e;
    return (FistClock){g_pit_base[0].counts + (e / p + 1) * p, g_pit_base[0].fraction};
}
unsigned long long fist_pit0_next_wrap(void){ FistClock next = pit_next_wrap();
    return next.counts + (next.fraction != 0); }
static uint64_t clock_cpu_cycles(FistClock time)
{
    return (time.counts / PIT_HZ_) * CPU_HZ_ +
        ((time.counts % PIT_HZ_) * CPU_HZ_ + time.fraction) / PIT_HZ_;
}
static void pic_slice_limit(unsigned *slice, uint64_t tick, unsigned index,
                            uint64_t event_tick, float event_lag)
{
    if (event_tick < tick) return;
    float deadline = ((float)(event_tick - tick) + event_lag) * 30000.0f;
    if (deadline <= index) return;
    unsigned cycles = (unsigned)(deadline - index);
    if (!cycles) cycles = 1;
    if (cycles < *slice) *slice = cycles;
}
static void pic_slice_vga(unsigned *slice, uint64_t tick, unsigned index,
                          uint64_t origin_tick, float origin_lag, int mode, int parts)
{
    unsigned clock = mode == 0x13 ? VGA_MODE13_CLOCK_ : VGA_CLOCK_;
    const float delays[] = {0, VGA_VERTICAL_MS,
        (float)(100.0 * VRETRACE_LINE * 1000 / clock),
        (float)(100.0 * (VRETRACE_LINE + 2) * 1000 / clock),
        (float)(100.0 * VDISPEND_LINE * 1000 / clock + 0.005)};
    for (unsigned i = 0; i < sizeof delays / sizeof *delays; ++i) {
        float next = origin_lag + delays[i];
        unsigned whole = (unsigned)next;
        pic_slice_limit(slice, tick, index, origin_tick + whole, next - whole);
    }
    if (parts) {
        for (unsigned part = 0; part < 4; ++part) {
            float next = origin_lag + (mode == 0x13 ? VGA_MODE13_PART_MS : VGA_TEXT_PART_MS);
            unsigned whole = (unsigned)next;
            origin_tick += whole;
            origin_lag = next - whole;
            pic_slice_limit(slice, tick, index, origin_tick, origin_lag);
        }
    }
}
/* DOSBox pic.cpp: 512 float-index entries, stable equal-time insertion and
 * callback-relative re-arming. The active CPU budget is the shared owner above;
 * queue callbacks run at PIC dispatch, after instruction/REP effects finish. */
#define PIC_QUEUE_SIZE 512u
typedef struct FistPicEntry {
    float index;
    FistPicEvent handler;
    unsigned value;
    struct FistPicEntry *next;
} FistPicEntry;
static FistPicEntry g_pic_entries[PIC_QUEUE_SIZE];
static FistPicEntry *g_pic_free, *g_pic_events;
static uint64_t g_pic_tick;
static int g_pic_initialized, g_pic_service;
static float g_pic_service_lag;
static void cpu_slice_start(void);
static FistCpuState *g_cpu_context;
FistCpuState *fist_clock_bind_cpu(FistCpuState *cpu)
{
    FistCpuState *previous=g_cpu_context;
    g_cpu_context=cpu;
    return previous;
}
static void pic_tick_sync(uint64_t cpu)
{
    if (!g_pic_initialized) {
        for (unsigned i = 0; i + 1 < PIC_QUEUE_SIZE; ++i)
            g_pic_entries[i].next = &g_pic_entries[i + 1];
        g_pic_free = g_pic_entries;
        g_pic_tick = cpu / 30000u;
        g_pic_initialized = 1;
    }
    /* TIMER_AddTick subtracts one float per tick, not a rounded total. DOS
     * callback credits can move virtual time back without reversing PIC_Ticks. */
    while (g_pic_tick < cpu / 30000u) {
        for (FistPicEntry *entry = g_pic_events; entry; entry = entry->next)
            entry->index -= 1.0f;
        ++g_pic_tick;
    }
}
static int64_t pic_index(uint64_t cpu) { return (int64_t)cpu - (int64_t)(g_pic_tick * 30000u); }
static float pic_event_cycles(float index)
{
    /* Original PIC_RunQueue multiplies float index by CPU_CycleMax before
     * subtracting its integer tick index. Round that product to binary32:
     * optimized x87 may otherwise retain an extended-precision result and
     * truncate the reached DSP-reset budget from 598 to 597 cycles. */
    volatile float cycles = index * 30000.0f;
    return cycles;
}
static void pic_service_events(uint64_t cpu)
{
    if (g_pic_service) return;
    int64_t index = pic_index(cpu);
    g_pic_service = 1;
    while (g_pic_events) {
        /* Store the original float product before comparing it. Native x87
         * excess precision must not postpone entries at the rounded deadline. */
        float deadline = pic_event_cycles(g_pic_events->index);
        if (deadline > index) break;
        FistPicEntry *entry = g_pic_events;
        g_pic_events = entry->next;
        g_pic_service_lag = entry->index;
        entry->handler(entry->value);
        entry->next = g_pic_free;
        g_pic_free = entry;
    }
    g_pic_service = 0;
}
void fist_clock_add_event(FistPicEvent handler, float delay, unsigned value)
{
    if (!clock_equal(g_cpu_time, clock_now())) cpu_slice_start();
    if (!g_pic_free) { fprintf(stderr, "[pic] Event queue full\n"); return; }
    FistPicEntry *entry = g_pic_free;
    g_pic_free = entry->next;
    uint64_t cpu = clock_cpu_cycles(clock_now());
    /* PIC_TickIndex returns binary32 before PIC_AddEvent adds its delay. */
    volatile float index = (float)pic_index(cpu) / 30000.0f;
    entry->index = delay + (g_pic_service ? g_pic_service_lag : index);
    entry->handler = handler;
    entry->value = value;
    FistPicEntry **place = &g_pic_events;
    while (*place && (*place)->index <= entry->index) place = &(*place)->next;
    entry->next = *place;
    *place = entry;
    /* AddEntry may return the remaining budget to CPU_CycleLeft. The failed
     * normal-core decrement is charged on the next fetch, never in this call. */
    /* AddEntry converts the binary32 difference to PIC_MakeCycles' double. */
    volatile float delta = g_pic_events->index - index;
    int cycles = (int)(30000.0 * (double)delta);
    if (cycles < (int)g_cpu_remaining) g_cpu_remaining = 0;
}
static void pic_remove(FistPicEvent handler, int specific, unsigned value)
{
    FistPicEntry **place = &g_pic_events;
    while (*place) {
        FistPicEntry *entry = *place;
        if (entry->handler == handler && (!specific || entry->value == value)) {
            *place = entry->next;
            entry->next = g_pic_free;
            g_pic_free = entry;
        } else place = &entry->next;
    }
}
void fist_clock_remove_events(FistPicEvent handler) { pic_remove(handler, 0, 0); }
void fist_clock_remove_specific_events(FistPicEvent handler, unsigned value) { pic_remove(handler, 1, value); }

static unsigned cpu_next_slice(uint64_t cpu)
{
    uint64_t tick = cpu / 30000u;
    unsigned index = cpu % 30000u, slice = 30000u - index;
    uint64_t pit = clock_cpu_cycles(pit_next_wrap());
    if (pit > cpu && pit - cpu < slice) slice = (unsigned)(pit - cpu);
    if (g_sequence_pic_ready) {
        pic_slice_vga(&slice, tick, index, g_sequence_pic_tick, g_sequence_pic_vertical_lag, g_sequence_mode, 1);
        if (g_sequence_pic_previous_ready)
            pic_slice_vga(&slice, tick, index, g_sequence_pic_previous_tick, g_sequence_pic_previous_lag,
                          g_sequence_pic_previous_mode, 0);
    }
    else if (g_sequence_next) {
        uint64_t draw = clock_cpu_cycles((FistClock){g_sequence_next, 0});
        if (draw > cpu && draw - cpu < slice) slice = (unsigned)(draw - cpu);
    }
    if (g_sequence_resize_ready) {
        uint64_t resize = clock_cpu_cycles((FistClock){g_sequence_resize_ready, 0});
        if (resize > cpu && resize - cpu < slice) slice = (unsigned)(resize - cpu);
    }
    if (g_pic_events) {
        float deadline = pic_event_cycles(g_pic_events->index);
        volatile float distance = deadline - pic_index(cpu);
        if (distance > 0) {
            unsigned cycles = (unsigned)distance;
            if (!cycles) cycles = 1;
            if (cycles < slice) slice = cycles;
        }
    }
    return slice;
}
unsigned fist_clock_cpu_slice(uint64_t *cycle)
{
    uint64_t cpu = clock_cpu_cycles(clock_now());
    if (cycle) *cycle = cpu;
    return clock_equal(g_cpu_time, clock_now()) ? g_cpu_remaining : cpu_next_slice(cpu);
}
static void cpu_slice_start(void)
{
    uint64_t cpu = clock_cpu_cycles(clock_now());
    pic_tick_sync(cpu);
    pic_service_events(cpu);
    g_cpu_remaining = cpu_next_slice(clock_cpu_cycles(clock_now()));
    g_cpu_time = clock_now();
}
void fist_clock_pic_requeue(void)
{
    /* write_data/PIC_SetIRQMask/OCW3 return the active budget to CycleLeft.
     * The next failed normal-core fetch owns the decrement and dispatch. */
    if (!clock_equal(g_cpu_time, clock_now())) cpu_slice_start();
    g_cpu_remaining = 0;
}
/* Step the clock to `target`, firing the channel-0 interrupt at every wrap on the way (the ISR may
 * re-program the channel, which restarts the count from that instant, as on the 8253). */
int g_int8_replay;   /* board:0017 FIST_FRAME_SCHEDULE: the INT-8s come from the schedule, not the clock */
int g_int8_force;    /* ... except from an explicit spin-wait pump (a fade, a delay): those need the interrupt */
static void clock_advance_to(FistClock target){
    extern void fist_int8_fire(void);
    uint64_t end_ms = fist_sequence_end_ms();
    uint64_t end_clock = end_ms ? (end_ms * PIT_HZ_ + 999) / 1000 : 0;
    int complete = end_clock && !clock_before(target, (FistClock){end_clock, 0});
    if (complete) target = (FistClock){end_clock - 1, 0};
    while (clock_before(clock_now(), target)) {
        FistClock w = pit_next_wrap();
        FistClock resize = {g_sequence_resize_ready, 0}, next = {g_sequence_next, 0};
        if (g_sequence_resize_ready && !clock_before(target, resize) &&
            !clock_before(w, resize) && (!g_sequence_next || !clock_before(next, resize))) {
            clock_set(resize);
            g_sequence_resize_ready = 0;
            g_sequence_blank = 0;
            g_sequence_part = 0;
            g_sequence_vertical_num += VGA_FRAME_NUM;
            fist_sequence_pic_advance_vertical();
            g_sequence_mode = 0x13;
            g_sequence_next = fist_sequence_part_clock(1);
        }
        else if (g_sequence_next && !clock_before(target, next) && !clock_before(w, next)) {
            int same_tick = !clock_before(next, w) && !clock_before(w, next);
            clock_set(next);
            if (g_sequence_blank) memset(g_sequence_pixels + g_sequence_part * 64000u, 0, 64000u);
            else if (g_sequence_mode == 3) fist_text_scan_part(g_sequence_part);
            else memcpy(g_sequence_pixels + g_sequence_part * FB_SZ / 4,
                        g_mem + VGA_FB + g_sequence_part * FB_SZ / 4, FB_SZ / 4);
            if (++g_sequence_part == 4) {
                g_sequence_dispatch = 1;
                fist_sequence_present();
                g_sequence_dispatch = 0;
                g_sequence_part = 0;
                g_sequence_vertical_num += VGA_FRAME_NUM;
                fist_sequence_pic_advance_vertical();
                if (g_vmode == 3) ++g_text_frame_count;
            }
            g_sequence_next = fist_sequence_part_clock(g_sequence_part + 1);
            if (same_tick && (!g_int8_replay || g_int8_force)) fist_int8_fire();
        }
        else if (!clock_before(target, w)) { clock_set(w); if (!g_int8_replay || g_int8_force) fist_int8_fire(); }
        else clock_set(target);
    }
    if (complete) {
        clock_set((FistClock){end_clock, 0});
        fist_sequence_endpoint_complete();
        exit(0);
    }
}
void fist_clock_advance_to(unsigned long long target){ clock_advance_to((FistClock){target, 0}); }
void fist_clock_advance(unsigned n){ clock_advance_to((FistClock){g_clock + n, g_clock_fraction}); }
void fist_clock_advance_cpu_cycles(unsigned count)
{
    clock_advance_to(clock_after_cpu(count));
}
static void cpu_prepare_instruction(void)
{
    if (!clock_equal(g_cpu_time, clock_now())) cpu_slice_start();
    if (!g_cpu_remaining) {
        /* The failed normal-loop decrement survives PIC dispatch; TIMER_AddTick resets it. */
        if (clock_cpu_cycles(clock_now()) % 30000u) fist_clock_advance_cpu_cycles(1);
        /* CPU_Core_Normal_Run materializes lazy flags after its failed
         * decrement and before PIC_RunQueue resumes the next slice. */
        if (g_cpu_context) fist_cpu_fill_flags(g_cpu_context);
        cpu_slice_start();
    }
}
void fist_clock_cpu_ss_instruction(void)
{
    /* Normal core MOV/POP SS refunds its fetch and forces the following instruction. */
    cpu_prepare_instruction();
}
static void cpu_credit_cycles(unsigned count)
{
    uint64_t credit = (uint64_t)count * PIT_HZ_;
    FistClock time = clock_now();
    if (credit > time.fraction) {
        --time.counts;
        time.fraction += CPU_HZ_;
    }
    time.fraction -= (unsigned)credit;
    clock_set(time);
    g_cpu_time = time;
    g_cpu_remaining += count;
}
void fist_clock_credit_cpu_fetch(void)
{
    /* Original CPU_Cycles++: do not dispatch PIC, rewind PIC_Ticks, or
     * materialize flags. The caller has charged this instruction's fetch. */
    cpu_credit_cycles(1);
}
void fist_clock_rep_movs(uint8_t *dst, const uint8_t *src, unsigned width, uint32_t count, int direction)
{
    /* DOSBox DoString: fetch is refunded, a chunk reserves the available CPU budget,
     * and all chunk writes complete before PIC resumes. Zero/one count at budget one
     * consumes one cycle. Forward overlap therefore cannot be replaced by memmove. */
    do {
        cpu_prepare_instruction();
        unsigned take = count < g_cpu_remaining ? count : g_cpu_remaining;
        unsigned cost = count <= 1 && g_cpu_remaining <= 1 ? 1 : take;
        for (unsigned i = 0; i < take; ++i) {
            uint32_t value = 0;
            memcpy(&value, src, width);
            memcpy(dst, &value, width);
            src += direction * (int)width;
            dst += direction * (int)width;
        }
        fist_clock_charge_cpu_instructions(cost);
        count -= take;
    } while (count);
}
void fist_clock_charge_cpu_instructions(unsigned count)
{
    if (!count) return;
    while (count) {
        cpu_prepare_instruction();
        unsigned take = count < g_cpu_remaining ? count : g_cpu_remaining;
        g_cpu_remaining -= take;
        FistClock target = clock_after_cpu(take);
        clock_advance_to(target);
        count -= take;
        if (clock_equal(clock_now(), target)) g_cpu_time = clock_now();
        else if (!clock_equal(g_cpu_time, clock_now())) cpu_slice_start();
    }
}
void fist_clock_charge_dos_transfer(uint16_t value)
{
    /* DOSBox dos.cpp modify_cycles: the five-cycle floor can credit a callback's
     * active budget. It does not dispatch PIC or roll TIMER_AddTick forward. */
    if (!clock_equal(g_cpu_time, clock_now())) cpu_slice_start();
    unsigned cost = 4u * value;
    unsigned remaining = cost + 5u < g_cpu_remaining ? g_cpu_remaining - cost : 5u;
    if (remaining <= g_cpu_remaining) {
        fist_clock_charge_cpu_instructions(g_cpu_remaining - remaining);
    } else {
        cpu_credit_cycles(remaining - g_cpu_remaining);
    }
}
static void cpu_io_delay(int write)
{
    /* DOSBox iohandler.cpp: callback I/O subtracts budget, without retiring an instruction. */
    unsigned delay = 30000u / (write ? (unsigned)(1024 / 0.75) : 1024u);
    if (fist_clock_cpu_slice(NULL) >= 3u * delay) fist_clock_charge_cpu_instructions(delay);
}
void fist_clock_wait_bios_ticks(unsigned count)
{
    const uint16_t *tick = (const uint16_t *)(g_mem + 0x46c);
    while (count--) {
        uint16_t previous = *tick;
        do { clock_advance_to(pit_next_wrap()); } while (*tick == previous);
    }
}
static int vga_status(unsigned long long c){   /* port 0x3da at clock c: bit3 vsync, bit0 vertical blanking */
    double line;
    if (g_text_phase_set && c * VGA_CLOCK_ >= g_text_vertical_num) {
        unsigned long long phase = (c * VGA_CLOCK_ - g_text_vertical_num) % VGA_FRAME_NUM;
        line = (double)phase * 449.0 / VGA_FRAME_NUM;
    } else {
        line = (double)(unsigned)(c % FRAME_COUNTS) * (FRAME_LINES / FRAME_COUNTS);
    }
    int r = 0;
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
    port &= 0xffff;
    int sb = fist_sb_owns(port);
    if (port == 0x20 || port == 0x21 || port == 0xa0 || port == 0xa1 || sb) cpu_io_delay(0);
    else fist_timer_pump();   /* remaining port costs are owned by board:0026 */
    if (fist_opl_owns(port)) return fist_opl_in(port);  /* OPL FM 0x388 status (FIST_OPL/FIST_SB) */
    if (sb) return fist_sb_in(port);   /* SB DSP + 8237 DMA window (FIST_SB, default off) */
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
    case 0x61: g_port61 ^= 0x30; return g_port61;
    case 0x64: return 0x00;     /* keyboard status: no data available */
    case 0x201: return 0xf0;    /* joystick: no buttons pressed, timers low */
    case 0x20: case 0x21: case 0xa0: case 0xa1: return fist_pic_read(port);
    default:
        if (traceon()) fprintf(stderr, "[port] in  0x%03x -> 0\n", port);
        return 0;
    }
}

void out(int port, int val)
{
    port &= 0xffff; val &= 0xff;
    int sb = fist_sb_owns(port);
    if (port == 0x20 || port == 0x21 || port == 0xa0 || port == 0xa1 || sb) cpu_io_delay(1);
    else fist_timer_pump();   /* remaining port costs are owned by board:0026 */
    if (fist_opl_owns(port)) { fist_opl_out(port, val); return; }  /* OPL FM 0x388/0x389 (FIST_OPL/FIST_SB) */
    if (sb) { fist_sb_out(port, val); return; }   /* SB DSP + 8237 DMA (FIST_SB, default off) */
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
            g_pit_reload[ch] = g_pit_wlatch[ch]; g_pit_base[ch] = clock_now();
            if (ch == 2 && getenv("FIST_SPEAKER_TRACE"))
                fprintf(stderr, "FIST_SPEAKER counter %.9f count=%u mode=%u type=%u\n",
                        (double)g_clock * 1000.0 / PIT_HZ_, pit_period(2), g_pit_mode[2], g_port61 & 3);
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
        return;
    case 0x20: case 0x21: case 0xa0: case 0xa1: fist_pic_write(port, val); return;
    case 0x61:
        if (((g_port61 ^ val) & 3) && getenv("FIST_SPEAKER_TRACE"))
            fprintf(stderr, "FIST_SPEAKER type %.9f type=%u previous=%u\n",
                    (double)g_clock * 1000.0 / PIT_HZ_, val & 3, g_port61 & 3);
        if ((g_port61 ^ val) & 1 && (val & 1)) g_pit_base[2] = clock_now();
        g_port61 = (unsigned char)val;
        return;
    case 0x64:   /* kbd cmd */
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
