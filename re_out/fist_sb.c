/* re_out/fist_sb.c -- Armored Fist platform shim: Sound Blaster DSP + 8237 DMA + IRQ -> PCM.
 *
 * The 1994 engine plays digital audio (menu music, in-mission SFX/voice) by programming the Sound
 * Blaster DSP directly (port base 0x2x0), setting up an 8237 DMA transfer of PCM out of a fixed
 * conventional-memory buffer, and refilling on the SB completion IRQ.  This shim is the runtime target
 * of the Ghidra `in`/`out` port intrinsics for that hardware -- the DOS analog of DD2's DirectSound
 * shim.  It models the DSP command FSM + the 8237 channel state, and on a "start DMA output" command it
 * decodes the PCM the engine placed in g_mem and appends it (converted to signed-16 LE) to a PCM ring,
 * optionally written to a WAV sink (native) / dumped for node.  Auto-init streaming closes the loop via
 * the completion IRQ: fist_sb_pump() (driven from the timer pump) raises the SB IRQ at block boundaries
 * so the engine's ISR runs and refills -- the same cooperative-tick discipline the PIT ISR uses.
 *
 * STATUS (iteration 1): the SOUND driver (SOUNDDVR.DVR) is not yet wired on the port -- its init vector
 * (+0x2 -> body @0x78) is not in the fmap, so the engine's `call far load_seg:0x2` no-ops and NO sound
 * port is touched yet.  This shim is therefore the READY platform layer: correct per the SB16 protocol,
 * env-gated (default OFF -> zero effect on the 26 video flows), and unit-verified with a synthetic DMA
 * block (tools/oracle/sb_selftest).  It produces real engine PCM the moment SOUNDDVR is wired
 * (SeedDriverVecs re-decompile + the DSP/DMA base-loss reconstruction; see docs/audio.md).
 */
#include "ghidra_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================= enable / config ================= */
static int g_en = -1;
int fist_sb_enabled(void)
{
    if (g_en < 0) g_en = getenv("FIST_SB") ? 1 : 0;
    return g_en;
}
static int g_tr = -1;
static int sbtrace(void){ if(g_tr<0) g_tr = getenv("FIST_SB_TRACE")?1:0; return g_tr; }

/* ---- sound-source REGISTER threading (be0e -> c510 -> 01ec -> 0af4) ----
 * The engine registers a sound descriptor by an INDIRECT far call through DGROUP:0x510 (== the driver's
 * 01ec->0af4 register method).  The __allregs indirect-vector dispatch drops the register args, so the
 * engine call site (FUN_0000_be0e / be67) publishes the exact AX/BX/ES it set up here, and the driver
 * methods 01ec/0af4 read them (the documented per-patch indirect-method-vector threading + the unaff_ES
 * segment-reg class ApplyConv cannot thread).  See docs/audio.md and patches 349/350. */
unsigned short g_snd_reg_ax;   /* AX: be0e = word[DGROUP:0x9f2e+id]; be67 = 0xffff (flush) */
unsigned short g_snd_reg_bx;   /* BX: descriptor OFFSET (be0e sets 0) */
unsigned short g_snd_reg_es;   /* ES: descriptor SEGMENT = word[DGROUP:0x9f1c] */

/* ---- SOUND.CFG music-device LETTER threading (bdcc -> c508 -> 014e) ----
 * The engine dispatches the sound-device CONFIG method by an INDIRECT far call through DGROUP:0x508
 * (== the driver 014e device-config method) with the music-device letter in AL (asm 0xbdd6: mov al,
 * [DGROUP:0x248]; lcall *0x508).  The __allregs indirect-vector dispatch drops AL, so the engine call
 * site (FUN_0000_bdcc) publishes byte[DGROUP:0x248] here and 014e reads it.  For SOUND.CFG "0132710000"
 * the engine parse leaves 'C' (0x43) at DGROUP:0x248 -> 014e code 3 = the OPL/AdLib device (0x388).
 * See patch 352 and docs/audio.md §13. */
unsigned short g_snd_cfg_letter;

/* ---- device REGISTRATION: FUN_1000_1917 (DGROUP:0xd4) + FUN_1000_107a (DGROUP:0xf4) ----
 * These two engine service functions are UNRECOVERED by Ghidra (no C body); the driver init
 * (FUN_0000_0078, asm 0x78) calls them via the far vectors DGROUP:0xd4/0xf4 to register the sound
 * device.  Iteration 8 recovers them as LOADER-SHIM helpers (approved -- 107a is an INT-8/IRQ chain
 * register the shim owns; 1917 is an owner-tagged MEMMGR search over the pools patch 345 builds).
 * They are wired into fist_icall (linear 0x11917 / 0x1107a) so the driver init's lcall lands here
 * once the si=0xec far section installs DGROUP:0xd4/0xf4 (patch 355).  Gated: only reached under
 * FIST_SB (the whole sound-config dispatch is FIST_SB-installed).  Asm-verified vs fist_dat_image.bin
 * (1917 @0x11917 / search 0x11345 / 107a @0x1107a / INT-chain insert 0x14ce7). */
extern unsigned char g_mem[];
extern uint32_t fist_snd_base;            /* SOUNDDVR load base (linear); seg = base>>4 */

/* The driver timer-ISR (SOUNDDVR cs:0x3d6) that 107a chains into INT 8 -- recorded here for the
 * cooperative per-tick drive (the shim owns the PIT).  seg = SOUNDDVR load seg, off = 0x3d6. */
unsigned short g_snd_isr_seg;             /* 0 until 107a registers it */
unsigned short g_snd_isr_off = 0x3d6;
unsigned short g_snd_workobj_seg;         /* the (0x4fa,DGROUP)-owned device work object, if found */

/* FUN_1000_1917 (asm 0x11917): search pools 0x16d4/0x16f6/0x1718 for the block owned by
 * (bx=0x4fa, cx=DGROUP=0x1c00); if found, ds=block + format via 0x10cce and return its seg; else
 * CF=1 (fail).  The owner search 0x1135f: head = word[DGROUP:pool+0xa]; walk next = word[blk:0xc];
 * match word[blk:0x6]==bx && word[blk:0x8]==cx. */
static unsigned short snd_pool_search(unsigned short pool_off, unsigned short bx, unsigned short cx)
{
    unsigned short blk = *(unsigned short *)(g_mem + 0x1c000u + pool_off + 0xa);
    while (blk) {
        unsigned long base = (unsigned long)blk << 4;
        if (*(unsigned short *)(g_mem + base + 6) == bx &&
            *(unsigned short *)(g_mem + base + 8) == cx)
            return blk;                          /* found */
        blk = *(unsigned short *)(g_mem + base + 0xc);
    }
    return 0;                                    /* not found */
}

void fist_snd_1917(void)
{
    /* asm constants at the call site (FUN_0000_0078 asm 0xae..0xb9): ax=0x413 sub 0, bx=0x4fa, cx=ds */
    unsigned short bx = 0x4fa, cx = 0x1c00;
    unsigned short blk = snd_pool_search(0x16d4, bx, cx);
    if (!blk) blk = snd_pool_search(0x16f6, bx, cx);
    if (!blk) blk = snd_pool_search(0x1718, bx, cx);
    /* NB the caller (FUN_0000_0078) discards 1917's return/CF, so we do not touch the shared frame
     * scheduler CF (g_fist_cf) here -- record the found work object for the sequencer only. */
    if (!blk) { g_snd_workobj_seg = 0; if (sbtrace()) fprintf(stderr,"[snd] 1917: no (0x4fa,DGROUP) work object in pools (fail)\n"); return; }
    g_snd_workobj_seg = blk;
    if (sbtrace()) fprintf(stderr,"[snd] 1917: device work object found @ seg 0x%04x (pools)\n", blk);
    /* format (0x10cce) is the block reformat; the found block is already MEMMGR-live -> a bit-exact
     * reformat is deferred (no menu consumer reads it before the sequencer streams). */
}

/* FUN_1000_107a (asm 0x1107a): register the driver IRQ/timer handler.  0x14ce7 inserts SOUNDDVR
 * cs:0x3d6 into the INT-8 handler chain (a standard far-jmp chain-splice at es:bx = snd_seg:0x3d6).
 * The shim owns the PIT/INT-8 layer, so "register" = record the driver ISR entry for the cooperative
 * per-tick drive (fist_snd_isr_tick). */
void fist_snd_107a(void)
{
    g_snd_isr_seg = (unsigned short)(fist_snd_base >> 4);
    g_snd_isr_off = 0x3d6;
    if (sbtrace()) fprintf(stderr,"[snd] 107a: sound timer-ISR registered @ %04x:%04x\n", g_snd_isr_seg, g_snd_isr_off);
}

/* SB DSP base port (default 0x220; the ports 0x2x0..0x2xF window).  The engine derives the base from
 * SOUND.CFG; we accept the whole 0x220-0x22F window and additionally 0x210-0x260 so any configured base
 * is trapped.  (The exact SOUND.CFG->base decode is documented in docs/audio.md but is NOT load-bearing
 * here: we watch whatever base the driver writes.) */
#define SB_BASE_DEFAULT 0x220
static int g_base = SB_BASE_DEFAULT;

/* ================= PCM ring + WAV sink ================= */
/* Canonical internal format: signed 16-bit LE, mono, at the DSP-programmed sample rate.  (The engine's
 * menu music is mono 8-bit unsigned via SB DMA; we up-convert to s16 for a stable compare format.) */
#define RING_CAP (4u*1024u*1024u)     /* 4M samples ~ 95 s @ 44.1k -- ample for a menu-music capture */
static short  *g_ring;
static unsigned g_ring_n;             /* samples written */
static int    g_rate = 11025;         /* last DSP-programmed output rate (Hz) */

static FILE  *g_wav;                  /* WAV sink (FIST_AUDIO_WAV or default) */
static long   g_wav_data_off;         /* byte offset of the data chunk length field */
static unsigned g_wav_written;        /* PCM bytes written to the WAV */

static void wav_wr32(FILE*f,unsigned v){ fputc(v&0xff,f);fputc((v>>8)&0xff,f);fputc((v>>16)&0xff,f);fputc((v>>24)&0xff,f); }
static void wav_wr16(FILE*f,unsigned v){ fputc(v&0xff,f);fputc((v>>8)&0xff,f); }

static void wav_open(void)
{
    if (g_wav) return;
    const char *p = getenv("FIST_AUDIO_WAV");
    if (!p) p = "/tmp/fist_audio.wav";
    g_wav = fopen(p, "wb");
    if (!g_wav) { fprintf(stderr,"[sb] cannot open WAV '%s'\n", p); return; }
    /* header with placeholder sizes; patched in fist_sb_flush() */
    fwrite("RIFF",1,4,g_wav); wav_wr32(g_wav,0);            /* riff size */
    fwrite("WAVE",1,4,g_wav);
    fwrite("fmt ",1,4,g_wav); wav_wr32(g_wav,16);
    wav_wr16(g_wav,1);                                       /* PCM */
    wav_wr16(g_wav,1);                                       /* mono */
    wav_wr32(g_wav,(unsigned)g_rate);
    wav_wr32(g_wav,(unsigned)g_rate*2);                      /* byte rate */
    wav_wr16(g_wav,2);                                       /* block align */
    wav_wr16(g_wav,16);                                      /* bits */
    fwrite("data",1,4,g_wav); g_wav_data_off = ftell(g_wav); wav_wr32(g_wav,0);
    if (sbtrace()) fprintf(stderr,"[sb] WAV -> %s (rate=%d)\n", p, g_rate);
}

/* Append one signed-16 mono sample to the ring + WAV. */
static void emit(short s)
{
    if (!g_ring) g_ring = (short*)malloc(RING_CAP*sizeof(short));
    if (g_ring && g_ring_n < RING_CAP) g_ring[g_ring_n++] = s;
    if (!g_wav) wav_open();
    if (g_wav) { wav_wr16(g_wav,(unsigned short)s); g_wav_written += 2; }
}

/* ================= 8237 DMA channel state ================= */
/* We track the two channels the SB uses: channel 1 (8-bit) and channel 5 (16-bit). */
typedef struct { unsigned addr, count, page; int ff; int autoinit; } dmach_t;
static dmach_t g_dma1, g_dma5;        /* g_dma1 = ch1 (8-bit), g_dma5 = ch5 (16-bit) */

/* 8237 controller #1 (ch0-3): ports 0x00-0x0F.  ff = high/low byte flip-flop, per-controller. */
static void dma1_out(int port, int val)
{
    switch (port) {
    case 0x00: case 0x02:            /* ch0/1 base addr (we only use ch1) */
        if (port==0x02){ g_dma1.addr = g_dma1.ff ? ((g_dma1.addr&0x00ff)|(val<<8)) : ((g_dma1.addr&0xff00)|val); g_dma1.ff^=1; }
        break;
    case 0x03:                       /* ch1 count */
        g_dma1.count = g_dma1.ff ? ((g_dma1.count&0x00ff)|(val<<8)) : ((g_dma1.count&0xff00)|val); g_dma1.ff^=1; break;
    case 0x0b:                       /* mode: bit4 = auto-init */
        if ((val&3)==1) g_dma1.autoinit = (val>>4)&1; break;
    case 0x0c: g_dma1.ff = 0; break; /* clear flip-flop */
    case 0x0a: break;                /* single mask */
    }
}
/* 8237 controller #2 (ch4-7, 16-bit): ports 0xC0-0xDF; page for ch5 = 0x8B. */
static void dma5_out(int port, int val)
{
    switch (port) {
    case 0xc4: g_dma5.addr = g_dma5.ff ? ((g_dma5.addr&0x00ff)|(val<<8)) : ((g_dma5.addr&0xff00)|val); g_dma5.ff^=1; break;
    case 0xc6: g_dma5.count = g_dma5.ff ? ((g_dma5.count&0x00ff)|(val<<8)) : ((g_dma5.count&0xff00)|val); g_dma5.ff^=1; break;
    case 0xd6: if ((val&3)==1) g_dma5.autoinit = (val>>4)&1; break;
    case 0xd8: g_dma5.ff = 0; break;
    case 0xd4: break;
    }
}

/* ================= SB DSP command FSM ================= */
static int  g_dsp_args_left;          /* bytes still expected for the current command */
static int  g_dsp_cmd;                /* current command byte */
static unsigned char g_dsp_arg[4];
static int  g_dsp_argi;
static int  g_reset_step;             /* DSP reset write sequence */
static int  g_read_val = -1;          /* pending DSP data-read byte (0xAA after reset, version, ...) */
static int  g_irq_pending;            /* SB completion IRQ latched (acked by reading base+0xE/0xF) */
static int  g_speaker;                /* DSP speaker enable (D1/D3) */
static int  g_block16;                /* current transfer is 16-bit */
static int  g_playing;

static void (*g_irq_cb)(void);        /* engine SB-ISR invoker (set by native_main); may be NULL */
void fist_sb_set_irq_cb(void (*cb)(void)) { g_irq_cb = cb; }

/* Decode+emit the PCM block the engine placed at the DMA-programmed address. len = transfer bytes. */
static void run_dma_block(void)
{
    dmach_t *dc = g_block16 ? &g_dma5 : &g_dma1;
    unsigned phys, bytes;
    if (g_block16) { phys = (dc->page<<16) | (dc->addr<<1); bytes = ((dc->count+1)&0xffff)*2; }
    else           { phys = (dc->page<<16) |  dc->addr;      bytes =  (dc->count+1)&0xffff;    }
    if (sbtrace())
        fprintf(stderr,"[sb] DMA %s play: phys=0x%05x bytes=%u rate=%d ai=%d\n",
                g_block16?"16":"8", phys, bytes, g_rate, dc->autoinit);
    if (phys + (g_block16?bytes:bytes) > FIST_MEM_SIZE) { fprintf(stderr,"[sb] DMA phys OOB 0x%x\n",phys); return; }
    if (g_speaker) {
        if (g_block16) {
            for (unsigned i=0;i+1<bytes;i+=2){ short s = (short)(g_mem[phys+i] | (g_mem[phys+i+1]<<8)); emit(s); }
        } else {
            for (unsigned i=0;i<bytes;i++){ short s = (short)((int)(g_mem[phys+i]-128) << 8); emit(s); } /* u8 -> s16 */
        }
    }
    g_playing = dc->autoinit;         /* auto-init keeps running until DSP 0xDA */
    g_irq_pending = 1;                /* completion IRQ */
}

/* DSP command byte (base+0xC) + its argument bytes. */
static void dsp_command(int val)
{
    if (g_dsp_args_left > 0) {
        g_dsp_arg[g_dsp_argi++] = (unsigned char)val;
        if (--g_dsp_args_left > 0) return;
        /* command complete: dispatch */
        switch (g_dsp_cmd) {
        case 0x40: g_rate = 1000000 / (256 - g_dsp_arg[0]); break;           /* time constant */
        case 0x41: g_rate = (g_dsp_arg[0]<<8) | g_dsp_arg[1]; break;         /* SB16 output rate */
        case 0x42: g_rate = (g_dsp_arg[0]<<8) | g_dsp_arg[1]; break;         /* SB16 input rate (ignore) */
        case 0x48: g_dma1.count = (g_dsp_arg[0]) | (g_dsp_arg[1]<<8); break; /* set 8-bit block size */
        case 0x14: case 0x15: case 0x91:                                     /* 8-bit single-cycle out */
            g_dma1.count = (g_dsp_arg[0]) | (g_dsp_arg[1]<<8); g_block16=0; g_dma1.autoinit=0; run_dma_block(); break;
        case 0xb0: case 0xb2: case 0xb4: case 0xb6:                          /* SB16 16-bit out: mode,lenlo,lenhi */
            g_block16=1; g_dma5.count = (g_dsp_arg[1]) | (g_dsp_arg[2]<<8); g_dma5.autoinit=(g_dsp_cmd&0x04)?0:1; run_dma_block(); break;
        case 0xc0: case 0xc2: case 0xc4: case 0xc6:                          /* SB16 8-bit out: mode,lenlo,lenhi */
            g_block16=0; g_dma1.count = (g_dsp_arg[1]) | (g_dsp_arg[2]<<8); g_dma1.autoinit=(g_dsp_cmd&0x04)?0:1; run_dma_block(); break;
        }
        return;
    }
    g_dsp_cmd = val; g_dsp_argi = 0;
    switch (val) {
    case 0x40: g_dsp_args_left = 1; break;
    case 0x41: case 0x42: case 0x48: case 0x14: case 0x15: case 0x91: g_dsp_args_left = 2; break;
    case 0xb0: case 0xb2: case 0xb4: case 0xb6:
    case 0xc0: case 0xc2: case 0xc4: case 0xc6: g_dsp_args_left = 3; break;
    case 0x1c: case 0x90:                                                    /* 8-bit auto-init out (block from 0x48) */
        g_block16=0; g_dma1.autoinit=1; run_dma_block(); break;
    case 0xd1: g_speaker = 1; break;                                         /* speaker on */
    case 0xd3: g_speaker = 0; break;                                         /* speaker off */
    case 0xd0: g_playing = 0; break;                                         /* pause 8-bit DMA */
    case 0xd4: g_playing = 1; break;                                         /* resume 8-bit DMA */
    case 0xda: g_playing = 0; break;                                         /* exit 8-bit auto-init */
    case 0xd9: g_playing = 0; break;                                         /* exit 16-bit auto-init */
    case 0xe1: g_read_val = 4; break;                                        /* DSP version major (SB16=4); minor next read */
    default: break;
    }
}

/* ================= port dispatch ================= */
int fist_sb_owns(int port)
{
    if (!fist_sb_enabled()) return 0;
    port &= 0xffff;
    if (port >= g_base && port <= g_base+0xf) return 1;                      /* SB DSP window */
    if (port <= 0x0f) return 1;                                             /* 8237 controller #1 */
    if (port == 0x83 || port == 0x8b) return 1;                            /* DMA page (ch1 / ch5) */
    if (port >= 0xc0 && port <= 0xdf) return 1;                            /* 8237 controller #2 */
    return 0;
}

void fist_sb_out(int port, int val)
{
    port &= 0xffff; val &= 0xff;
    if (port >= g_base && port <= g_base+0xf) {
        switch (port - g_base) {
        case 0x6:                                                           /* DSP reset */
            if (val==1) g_reset_step=1;
            else if (val==0 && g_reset_step==1){ g_reset_step=0; g_read_val=0xAA; g_dsp_args_left=0; g_playing=0; }
            break;
        case 0xc: dsp_command(val); break;                                  /* DSP write command/data */
        }
        return;
    }
    if (port==0x83){ g_dma1.page=val; return; }
    if (port==0x8b){ g_dma5.page=val; return; }
    if (port<=0x0f) dma1_out(port,val);
    else if (port>=0xc0 && port<=0xdf) dma5_out(port,val);
}

int fist_sb_in(int port)
{
    port &= 0xffff;
    if (port >= g_base && port <= g_base+0xf) {
        switch (port - g_base) {
        case 0xe:                                                           /* read-buffer status: bit7 = data avail */
            g_irq_pending = 0;                                              /* reading DSP status/ack clears the 8-bit IRQ */
            return (g_read_val>=0) ? 0x80 : 0x00;
        case 0xf: g_irq_pending = 0; return 0x80;                          /* 16-bit IRQ ack */
        case 0xa: {                                                         /* DSP data read */
            int v = (g_read_val>=0)? g_read_val : 0xff;
            g_read_val = (g_dsp_cmd==0xe1 && v==4) ? 0x05 : -1;            /* version minor after major */
            return v; }
        case 0xc: return 0x00;                                             /* write-buffer status: bit7=0 -> ready */
        }
        return 0xff;
    }
    return 0xff;
}

/* Called from the cooperative timer pump: while an auto-init stream is running, re-raise the completion
 * IRQ so the engine's SB ISR refills the next block.  No-op until SOUNDDVR is wired + an ISR is set. */
void fist_sb_pump(void)
{
    if (!fist_sb_enabled() || !g_playing) return;
    if (g_irq_pending && g_irq_cb) { g_irq_pending = 0; g_irq_cb(); }
}

/* Finalize the WAV (patch the RIFF/data sizes) + report. */
void fist_sb_flush(void)
{
    if (!g_wav) return;
    long end = ftell(g_wav);
    fseek(g_wav, 4, SEEK_SET);            wav_wr32(g_wav, (unsigned)(end-8));
    fseek(g_wav, g_wav_data_off, SEEK_SET); wav_wr32(g_wav, g_wav_written);
    fseek(g_wav, end, SEEK_SET);
    fclose(g_wav); g_wav = NULL;
    fprintf(stderr,"[sb] WAV finalized: %u PCM bytes (%u samples @ %d Hz, %.2fs)\n",
            g_wav_written, g_ring_n, g_rate, g_rate? (double)g_ring_n/g_rate : 0.0);
}

/* Test/inspection accessors (used by the standalone selftest). */
unsigned fist_sb_ring_count(void){ return g_ring_n; }
int      fist_sb_rate(void){ return g_rate; }
