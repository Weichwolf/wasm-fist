/* re_out/fist_opl.c -- Armored Fist platform shim: OPL2/OPL3 FM synthesis (port 0x388/0x389) -> PCM.
 *
 * Iteration 7 (2026-07-18).  The menu background music is OPL FM synthesis (oracle A/B proven -- docs/
 * audio.md 12), NOT SB-DMA digital.  The SOUNDDVR device 3 (AdLib/OPL, SOUND.CFG letter 'C') programs the
 * OPL chip via the writer FUN_0000_0f21 = `out 0x388,reg; out 0x389,val`.  This shim traps those ports,
 * feeds the register stream to the DOSBox-matched DBOPL core (fist_opl_dbopl.cpp + opl/dbopl.cpp, the
 * verbatim DOSBox 0.74-3 DBOPL emulator the oracle uses by default), synthesizes the FM voices into a
 * signed-16 PCM ring + WAV sink, and closes the loop for a bit-exact menu-music compare vs
 * ref/audio_menu_oracle.wav.
 *
 * Env-gated: default OFF (FIST_OPL unset) -> fist_opl_owns() returns 0 for every port -> zero effect on
 * the 26 video flows.  FIST_SB (the SB-DMA shim) also enables OPL trapping so a single FIST_SB=1 exercises
 * the whole sound path.  This is the OPL analog of fist_sb.c (which stays valid for the in-mission digital
 * SFX/voice device at driver 0x2820).
 *
 * STATUS (iteration 7): the SOUNDDVR device-3 OPL SELECT + INIT chain (patches 352-354) now reaches real
 * out(0x388) writes on the menu (the AdLib init: reg 0x01=0x20, 0x08, 0xBD=0xC0, per-channel 0x40/0xB0).
 * The CONTINUOUS note stream (the menu music sequencer) is gated on the op=6 play trigger (516f->50e6,
 * DGROUP:0x4fc bit7) which is a SEPARATE, still-unlocated start path -- until it drives note-ons, the OPL
 * output is the (near-silent) init state.  The generation cadence (samples per PIT tick) is a calibration
 * step for when the sequencer plays.  fist_opl_dbopl.cpp is unit-verified (a synthetic AdLib note ->
 * peak 5091, 99.9% non-silent).
 */
#include "ghidra_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- DBOPL bridge (fist_opl_dbopl.cpp, extern "C") ---- */
void     fist_dbopl_reset(unsigned rate);
void     fist_dbopl_write(unsigned reg, unsigned char val);
unsigned fist_dbopl_generate(unsigned nsamp, int *out);
int      fist_dbopl_is_opl3(void);

/* ================= enable / config ================= */
static int g_en = -1;
int fist_opl_enabled(void)
{
    if (g_en < 0) g_en = (getenv("FIST_OPL") || getenv("FIST_SB")) ? 1 : 0;
    return g_en;
}
static int g_tr = -1;
static int opltrace(void){ if(g_tr<0) g_tr = getenv("FIST_OPL_TRACE")?1:0; return g_tr; }

/* OPL sample rate: DOSBox default mixer rate = 44100 (the oracle WAV rate). */
static int  g_rate = 44100;
static int  g_rate_set = 0;

/* Generation cadence: the FM synth is advanced in REAL EMULATED TIME, one PIT period per engine INT-8
 * tick (fist_opl_tick, called from the ISR drain).  samples/tick = rate * pit_div / PIT_HZ.  This keeps
 * the WAV bounded + roughly real-time.  Precise phase-pinning vs the oracle (sub-tick alignment of the
 * register writes to the sample clock) is the final bit-exact step once the op=6 sequencer drives notes. */
#define PIT_HZ 1193182.0
static double g_samp_acc = 0.0;
int fist_vga_pit0_div(void);

/* ================= register write latch ================= */
static int g_latch;          /* value last written to 0x388 (the AdLib register index) */
static unsigned g_a0_writes, g_b0_writes, g_keyon_writes;  /* diag: fnum-lo / block+fnum-hi / key-on */
static unsigned g_writes;    /* total 0x389 register writes seen */

/* ================= PCM ring + WAV sink (mono s16) ================= */
#define RING_CAP (8u*1024u*1024u)
static short   *g_ring;
static unsigned g_ring_n;
static int      g_gen32[1024];      /* DBOPL Bit32s scratch */

static FILE  *g_wav;
static long   g_wav_data_off;
static unsigned g_wav_written;

static void wav_wr32(FILE*f,unsigned v){ fputc(v&0xff,f);fputc((v>>8)&0xff,f);fputc((v>>16)&0xff,f);fputc((v>>24)&0xff,f); }
static void wav_wr16(FILE*f,unsigned v){ fputc(v&0xff,f);fputc((v>>8)&0xff,f); }

static void wav_open(void)
{
    if (g_wav) return;
    const char *p = getenv("FIST_AUDIO_WAV");
    if (!p) p = "/tmp/fist_audio.wav";
    g_wav = fopen(p, "wb");
    if (!g_wav) { fprintf(stderr,"[opl] cannot open WAV '%s'\n", p); return; }
    fwrite("RIFF",1,4,g_wav); wav_wr32(g_wav,0);
    fwrite("WAVE",1,4,g_wav);
    fwrite("fmt ",1,4,g_wav); wav_wr32(g_wav,16);
    wav_wr16(g_wav,1);                        /* PCM */
    wav_wr16(g_wav,1);                        /* mono */
    wav_wr32(g_wav,(unsigned)g_rate);
    wav_wr32(g_wav,(unsigned)g_rate*2);
    wav_wr16(g_wav,2);
    wav_wr16(g_wav,16);
    fwrite("data",1,4,g_wav); g_wav_data_off = ftell(g_wav); wav_wr32(g_wav,0);
    if (opltrace()) fprintf(stderr,"[opl] WAV -> %s (rate=%d)\n", p, g_rate);
}

/* DBOPL Bit32s -> clamped s16.  (GenerateBlock2 output is already ~s16-scaled; clamp for safety.) */
static short clamp16(int v){ if(v>32767)v=32767; else if(v<-32768)v=-32768; return (short)v; }

static void emit_block(unsigned n)
{
    if (n == 0) return;
    if (n > 1024) n = 1024;
    unsigned got = fist_dbopl_generate(n, g_gen32);
    if (!g_ring) g_ring = (short*)malloc(RING_CAP*sizeof(short));
    if (!g_wav) wav_open();
    for (unsigned i = 0; i < got; i++) {
        short s = clamp16(g_gen32[i]);
        if (g_ring && g_ring_n < RING_CAP) g_ring[g_ring_n++] = s;
        if (g_wav) { wav_wr16(g_wav,(unsigned short)s); g_wav_written += 2; }
    }
}

static void ensure_rate(void)
{
    if (g_rate_set) return;
    const char *e = getenv("FIST_OPL_RATE"); if (e) g_rate = atoi(e);
    if (g_rate < 8000) g_rate = 44100;
    fist_dbopl_reset((unsigned)g_rate);
    g_rate_set = 1;
}

/* ================= port dispatch ================= */
int fist_opl_owns(int port)
{
    if (!fist_opl_enabled()) return 0;
    port &= 0xffff;
    return (port == 0x388 || port == 0x389);
}

void fist_opl_out(int port, int val)
{
    port &= 0xffff; val &= 0xff;
    ensure_rate();
    if (port == 0x388) { g_latch = val; return; }         /* register index latch */
    if (port == 0x389) {                                   /* data -> DBOPL WriteReg */
        fist_dbopl_write((unsigned)g_latch, (unsigned char)val);
        g_writes++;
        unsigned r = g_latch & 0xff;
        if (r >= 0xa0 && r <= 0xa8) g_a0_writes++;          /* fnum-low */
        if (r >= 0xb0 && r <= 0xb8) { g_b0_writes++; if (val & 0x20) g_keyon_writes++; }
        if (opltrace() && g_writes <= 200)
            fprintf(stderr,"[opl] reg[0x%02x] = 0x%02x\n", r, val);
    }
}

/* AdLib status read (port 0x388): the detect FUN_0000_0f54 writes timer regs then reads the status.
 * DBOPL's timer status isn't exposed without the Handler; return the classic "OPL present" pattern so
 * the (already config-selected) device-3 detect passes.  Bit7|bit6 set = both timers expired. */
int fist_opl_in(int port)
{
    port &= 0xffff;
    if (port == 0x388) return 0x00;   /* timer status; device 3 is already selected -> detect not gating */
    return 0xff;
}

/* Called once per engine INT-8 (PIT) tick from the ISR drain: advance the OPL synth by one real PIT
 * period's worth of samples (rate * pit_div / PIT_HZ), accumulator for the fractional remainder. */
void fist_opl_tick(void)
{
    if (!fist_opl_enabled() || !g_rate_set) return;
    int div = fist_vga_pit0_div();
    g_samp_acc += (double)g_rate * (double)div / PIT_HZ;
    unsigned n = (unsigned)g_samp_acc;
    if (n > 65536) n = 65536;           /* guard a pathological divisor */
    if (n) { g_samp_acc -= n; emit_block(n); }
}

/* fist_opl_pump: retained for the port-I/O pump path but generation is driven per-tick (fist_opl_tick),
 * so this is a no-op (the FM synth must not advance faster than emulated time). */
void fist_opl_pump(void) { }

void fist_opl_flush(void)
{
    if (!g_wav) return;
    long end = ftell(g_wav);
    fseek(g_wav, 4, SEEK_SET);              wav_wr32(g_wav, (unsigned)(end-8));
    fseek(g_wav, g_wav_data_off, SEEK_SET); wav_wr32(g_wav, g_wav_written);
    fseek(g_wav, end, SEEK_SET);
    fclose(g_wav); g_wav = NULL;
    fprintf(stderr,"[opl] WAV finalized: %u samples @ %d Hz (%.2fs); %u OPL reg writes; opl3=%d\n",
            g_ring_n, g_rate, g_rate? (double)g_ring_n/g_rate : 0.0, g_writes, fist_dbopl_is_opl3());
    fprintf(stderr,"[opl] fnum writes: A0-A8=%u  B0-B8=%u  key-on=%u\n",
            g_a0_writes, g_b0_writes, g_keyon_writes);
}

unsigned fist_opl_writes(void){ return g_writes; }
unsigned fist_opl_ring_count(void){ return g_ring_n; }
