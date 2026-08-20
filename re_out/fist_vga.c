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

/* DAC state machine (ports 0x3C8 write-index, 0x3C7 read-index, 0x3C9 data) */
static int g_dac_widx, g_dac_wsub;    /* write index + sub-component (0=R,1=G,2=B) */
static int g_dac_ridx, g_dac_rsub;

static int g_trace = -1;
static int traceon(void){ if(g_trace<0){ extern char*getenv(const char*); g_trace=getenv("FIST_TRACE_TRAPS")?1:0;} return g_trace; }

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
    if (traceon()) fprintf(stderr, "[vga] set video mode 0x%02x (%s)\n", g_vmode,
        g_vmode==0x13 ? "320x200x256 linear" : "other");
    /* mode set clears the display in real VGA; zero the aperture the engine will draw into. */
    if (g_vmode==0x13) memset(g_mem+VGA_FB, 0, FB_SZ);
}
int  fist_vga_mode(void){ return g_vmode; }

/* ================= port I/O ================= */
static int g_3da_toggle;
static unsigned short g_pit[3] = {0xffff,0xffff,0xffff};
/* PIT channel-0 reload divisor (INT-8 rate = 1193182/div): the OPL shim uses it to advance the FM synth
 * by the real per-tick sample count.  0 divisor == 65536. */
int fist_vga_pit0_div(void){ return g_pit[0] ? g_pit[0] : 0x10000; }
static int g_pit_sub[3];

/* Service the MGAVIDEO palette-upload semaphore DGROUP:0x786 bit0 == the driver's vertical-retrace ISR
 * FUN_0000_0b1f (asm 0xb25: `shr BYTE ds:0x786,1; jae ...`): when bit0 is set, upload the 768-byte 6-bit
 * palette buffer at word[DGROUP:0x782] to the DAC (out 0x3c8,0; 768x out 0x3c9).  On the original this is
 * an ASYNC IRQ that fires ~70Hz regardless of the main loop; our port has no retrace IRQ, so it is driven
 * from BOTH the in(0x3da) retrace poll the engine busy-waits on AND the cooperative INT-8 timer pump
 * (fist_timer_pump) -- the pump is the "time passes / IRQs fire" heartbeat, so decoupling the palette
 * upload from an explicit in(0x3da) poll mirrors the async retrace IRQ faithfully.  Idempotent. */
void fist_vga_service_retrace(void)
{
    if (g_mem[0x1c786] & 1) {
        g_mem[0x1c786] >>= 1;
        unsigned pseg = *(unsigned short *)(g_mem + 0x1c782);
        const unsigned char *pb = g_mem + ((unsigned)pseg << 4);
        for (int i = 0; i < 256; i++) {
            g_pal[i][0] = pb[i*3+0] & 0x3f;
            g_pal[i][1] = pb[i*3+1] & 0x3f;
            g_pal[i][2] = pb[i*3+2] & 0x3f;
        }
    }
    /* The real driver retrace ISR (MGAVIDEO 0x0b1f) uploads the present palette, THEN calls the
     * registered per-frame palette-fade step at word[DGROUP:0x5e8] (which advances the fade ramp,
     * rewrites the present buffer, and re-arms 0x786 for the next upload).  This shim stands in for
     * that ISR, so we invoke the fade step here.  Gate on the exact marker the SETTINGS fade setup
     * (MGAVIDEO 0x0874) writes -- offset word 0x5e8 == 0x946 -- so no other screen's vtable slot is
     * ever called; 0x0946 clears it to 0 when the fade completes (present == loaded-1, the settled
     * DOSBox palette).  0x0946 does no port I/O, so this is not re-entrant into in()/out(). */
    if (*(unsigned short *)(g_mem + 0x1c5e8) == 0x946) {
        extern code *fist_icall_far(uint32_t);
        ((void (*)(void))fist_icall_far(*(uint32_t *)(g_mem + 0x1c5e8)))();
    }
}

int in(int port)
{
    fist_timer_pump();   /* cooperative PIT-ISR tick: the engine polls ports in its wait/render loops */
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
    case 0x3da: case 0x3ba: /* input status 1: toggle retrace bits so vsync polls exit */
        g_3da_toggle ^= 0x09;
        fist_vga_service_retrace();   /* vblank boundary: upload the palette if 0340 requested it */
        return g_3da_toggle;
    case 0x40: case 0x41: case 0x42: { /* PIT counter read (latch low/high alternating) */
        int ch=port-0x40; int v = g_pit_sub[ch]? (g_pit[ch]>>8):(g_pit[ch]&0xff);
        g_pit_sub[ch]^=1; g_pit[ch]--; return v & 0xff; }
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
    fist_timer_pump();   /* cooperative PIT-ISR tick (see in()) */
    port &= 0xffff; val &= 0xff;
    if (fist_opl_owns(port)) { fist_opl_out(port, val); return; }  /* OPL FM 0x388/0x389 (FIST_OPL/FIST_SB) */
    if (fist_sb_owns(port)) { fist_sb_out(port, val); return; }   /* SB DSP + 8237 DMA (FIST_SB, default off) */
    switch (port) {
    case 0x3c8: g_dac_widx = val; g_dac_wsub = 0; return;     /* set DAC write index */
    case 0x3c7: g_dac_ridx = val; g_dac_rsub = 0; return;     /* set DAC read index */
    case 0x3c9: /* DAC data write: R,G,B (6-bit) */
        g_pal[g_dac_widx & 0xff][g_dac_wsub] = (unsigned char)(val & 0x3f);
        if (++g_dac_wsub==3){ g_dac_wsub=0; g_dac_widx=(g_dac_widx+1)&0xff; }
        return;
    case 0x40: case 0x41: case 0x42: { /* PIT counter load */
        int ch=port-0x40;
        if(!g_pit_sub[ch]) g_pit[ch]=(g_pit[ch]&0xff00)|val; else g_pit[ch]=(g_pit[ch]&0x00ff)|(val<<8);
        g_pit_sub[ch]^=1; return; }
    case 0x43: return;   /* PIT control */
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
