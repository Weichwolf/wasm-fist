/* Armored Fist Sound Blaster platform shim.
 * PCM8 DMA consumption follows DOSBox 0.74-3 dma.cpp/sblaster.cpp: the mixer
 * requests bytes; DSP completion and DMA terminal count have separate owners.
 * The PCM ring/WAV is a device diagnostic, not the final stereo mixer. Device
 * initialization, shared mixer demand/timing and the legacy PCM16 path remain
 * under board/0003; no complete SB16 or original-output parity is claimed.
 */
#include "ghidra_compat.h"
#include "fist_sb.h"
#include "fist_pic.h"
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
 * The IRQ/timer-ISR register the driver init (FUN_0000_0078, asm 0x78) calls through DGROUP:0xf4 to
 * register the sound device is UNRECOVERED by Ghidra (no C body); the shim owns the INT-8 layer, so
 * it is a loader-shim helper wired into fist_icall (linear 0x1107a).  Its sibling DGROUP:0xd4 (0x11917)
 * is the MEMMGR resize -- FUN_1000_1917, patch 586 (a stand-in here used to search the pools for
 * the 0x4fa work object and resize nothing).  Gated: only reached under FIST_SB.  Asm-verified vs
 * fist_dat_image.bin (107a @0x1107a / INT-chain insert 0x14ce7). */
extern unsigned char g_mem[];
extern uint32_t fist_snd_base;            /* SOUNDDVR load base (linear); seg = base>>4 */

/* The driver timer-ISR (SOUNDDVR cs:0x3d6) that 107a chains into INT 8 -- recorded here for the
 * cooperative per-tick drive (the shim owns the PIT).  seg = SOUNDDVR load seg, off = 0x3d6. */
unsigned short g_snd_isr_seg;             /* 0 until 107a registers it */
unsigned short g_snd_isr_off = 0x3d6;

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

/* Drive the driver TIMER-ISR sequencer (SOUNDDVR body @ cs:0x3dd = fist_snd_base+0x3dd) once per engine
 * INT-8 (PIT) tick.  We ARE the extender/PIT layer (107a chained the driver ISR into INT 8), so the
 * faithful equivalent of the hardware timer firing is to invoke the driver ISR body each tick.  The ISR
 * self-gates on the arm word DGROUP:0x23e==2 and the note-descriptor table DGROUP:0x4fe (both engine/
 * device-object state) so this is behaviour-neutral until the sound system is armed.  Entry = 0x3dd (the
 * body); 0x3d6 is the ljmp-to-old-handler chain slot, not the entry. */
unsigned g_snd_arm_bumps;                 /* diagnostic: how many arm-producer bumps we issued */
void fist_snd_isr_tick(void)
{
    extern int fist_opl_enabled(void);
    if (!g_snd_isr_seg || !fist_opl_enabled()) return;

    /* One-time faithful voice-slot init (the driver device-config 0x252 does exactly this at 0x272:
     * `mov al,0xff; mov bx,6; cs:0x60(bx)=al` -> cs:0x60..0x66 = 0xff = "voice free").  That config
     * path is unreached in the port, so the ISR's 7-voice scan would otherwise read uninitialised 0 as
     * "note 0 active" and dispatch spurious releases.  With the slots free the scan is a safe no-op
     * until a real note-on populates a slot. */
    static int voices_init = 0;
    if (!voices_init) {
        unsigned long dsb = fist_snd_base;              /* driver load base (linear); cs = base>>4 */
        for (int i = 0; i <= 6; i++) g_mem[dsb + 0x60 + i] = 0xff;
        voices_init = 1;
    }

    /* Drive the ARM-GATE PRODUCER (FUN_1000_1042, patch 357) so the arm word DGROUP:0x23e reaches 2 --
     * the consumer (the ISR) requires exactly 2 to run its scan and drops it to 1 only when a voice is
     * active.  We keep it pinned at 2 each tick (the sound-service dispatcher's cadence, which the port
     * never reaches; we own the PIT).  Bump only while < 2 so it cannot overshoot past 2 and stall. */
    {
        code *arm = fist_icall(0x11042u);
        int guard = 4;
        while (arm && g_mem[0x1c23e] < 2 && guard-- > 0) {
            ((void(*)(void))arm)();
            g_snd_arm_bumps++;
        }
    }

    code *fn = fist_icall(fist_snd_base + 0x3ddu);
    if (fn) ((int(*)(int,int,int,int,int,int,int,int,int,int))fn)(0,0,0,0,0,0,0,0,0,0);

    /* NB the device-3 VOICE/ENVELOPE + MIDI-sequencer advance (FUN_0000_0a28 -> 0c39, patches 358/359)
     * is NO LONGER driven here (once-per-engine-INT8-tick made the tempo ~16x too fast -- iter 15 §21).
     * It is now driven at the driver MUSIC-TIMER rate, locked to the OPL sample clock, from
     * fist_opl_tick() -> fist_snd_seq_advance() below.  See docs/audio.md §22. */

    /* diagnostic snapshot of the sequencer state (FIST_SND_DIAG): max arm reached, any voice active,
     * note-table offset populated?  Answers "is the sequencer FED?" without gdb. */
    {
        extern unsigned g_snd_seq_maxarm, g_snd_seq_active, g_snd_seq_isrruns;
        g_snd_seq_isrruns++;
        unsigned char arm = g_mem[0x1c23e];
        if (arm > g_snd_seq_maxarm) g_snd_seq_maxarm = arm;
        unsigned long dsb = fist_snd_base;
        for (int i = 0; i <= 6; i++) if (g_mem[dsb + 0x60 + i] != 0xff) g_snd_seq_active++;
    }
}
unsigned g_snd_seq_maxarm, g_snd_seq_active, g_snd_seq_isrruns;

/* ---- MUSIC-TIMER cadence (iteration 16) ----
 * The MIDI sequencer + per-voice envelope advance FUN_0000_0a28 (-> 0c39 fetch/decode + per-voice
 * OPL A0/B0 fnum reprogram) does ONE music-tick per call.  The SOUNDDVR timer ISR runs at 7231.4 Hz
 * (PIT ch0 divisor 0xa5=165, asm 0x6f6/0x714 in fist_snd_image.bin) and the music advance is a fixed
 * sub-division of it; the exact per-call divider is not isolable from the SOUNDDVR image alone (the
 * 0a28 invoker cs:0x1d2 via [cs:0x5c2] is dead-in-image -- installed/chained externally at runtime),
 * so the effective MUSIC_HZ is selected among the asm-grounded ISR sub-divisions 7231.4/k by best
 * xcorr vs the DOSBox OPL oracle (ref/audio_menu_oracle.wav).  Driven from fist_opl_tick(), locked to
 * the OPL sample clock (advance once per opl_rate/MUSIC_HZ generated samples) so the tempo is
 * independent of the engine-tick / fast-forward rate.  Behaviour-neutral until device 3 is selected
 * (FIST_SB) + the song is playing ([ds:0xe]!=0). */
unsigned g_snd_seq_advances;
void fist_snd_seq_advance(void)
{
    extern int fist_opl_enabled(void);
    if (!g_snd_isr_seg || !fist_opl_enabled()) return;
    /* board:0003: the MIDI sequencer (0a28 -> 0c39) is ATOMIC per tick.  Its note dispatch writes OPL
       registers (0f21 -> out()), and out() pumps the cooperative PIT tick (fist_timer_pump), which loops
       back here -- a nested advance would run 0c39 recursively and corrupt the SHARED driver delay counter
       [ds:0x14] (decw 0->0xffff = a 65535-tick stall) and cursor, so the menu music emitted ~4 note-ons in
       190s instead of the original's continuous melody.  On real hardware OPL I/O never re-drives the
       sequencer; guard against re-entry to restore that invariant. */
    static int in_seq = 0;
    if (in_seq) return;
    in_seq = 1;
    code *adv = fist_icall(fist_snd_base + 0xa28u);
    if (adv) { ((int(*)(int,int,int,int))adv)(0,0,0,0); g_snd_seq_advances++; }
    in_seq = 0;
}

void fist_snd_diag(void)
{
    if (!getenv("FIST_SND_DIAG")) return;
    fprintf(stderr, "[snd-diag] arm-bumps=%u isr-runs=%u max-arm=%u voice-active-hits=%u "
            "note-table[0x4fe]=0x%04x seq-advances=%u\n",
            g_snd_arm_bumps, g_snd_seq_isrruns, g_snd_seq_maxarm, g_snd_seq_active,
            *(unsigned short*)(g_mem + 0x1c4fe), g_snd_seq_advances);
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
static int    g_rate = 22050;         /* last DSP-programmed output rate (Hz) */

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
typedef struct {
    unsigned addr, count, page, curraddr, currcnt;
    int autoinit, masked, tcount, request;
} dmach_t;
static dmach_t g_dma[8];
static int g_dma_ff[2], g_dma_initialized;
#define g_dma1 g_dma[1]
#define g_dma5 g_dma[5]
static void dma_mask(unsigned channel, int masked);

static void dma_init(void)
{
    if (g_dma_initialized) return;
    for (unsigned i=0; i<8; ++i) g_dma[i].masked=1;
    g_dma_initialized=1;
}

/* The low/high flip-flop belongs to the controller, including unrelated channels. */
static void dma_out(unsigned controller, unsigned reg, unsigned val)
{
    if (reg<8) {
        dmach_t *dc=&g_dma[controller*4+(reg>>1)];
        unsigned *base=(reg&1) ? &dc->count : &dc->addr;
        unsigned *current=(reg&1) ? &dc->currcnt : &dc->curraddr;
        unsigned shift=g_dma_ff[controller] ? 8 : 0;
        *base=(*base & (0xffffu ^ (0xffu<<shift))) | (val<<shift);
        *current=(*current & (0xffffu ^ (0xffu<<shift))) | (val<<shift);
        g_dma_ff[controller]^=1;
        return;
    }
    switch (reg) {
    case 0xa: dma_mask(controller*4+(val&3), !!(val&4)); break;
    case 0xb: g_dma[controller*4+(val&3)].autoinit=!!(val&0x10); break;
    case 0xc: g_dma_ff[controller]=0; break;
    case 0xd:
        for (unsigned i=0; i<4; ++i) {
            dma_mask(controller*4+i,1);
            g_dma[controller*4+i].tcount=0;
        }
        g_dma_ff[controller]=0;
        break;
    case 0xe: case 0xf:
        for (unsigned i=0; i<4; ++i)
            dma_mask(controller*4+i, reg==0xf ? !!(val&(1u<<i)) : 0);
        break;
    }
}

static int dma_in(unsigned controller, unsigned reg)
{
    if (reg<8) {
        dmach_t *dc=&g_dma[controller*4+(reg>>1)];
        unsigned value=(reg&1) ? dc->currcnt : dc->curraddr;
        unsigned shift=g_dma_ff[controller] ? 8 : 0;
        g_dma_ff[controller]^=1;
        return (value>>shift)&0xff;
    }
    if (reg==8) {
        int status=0;
        for (unsigned i=0; i<4; ++i) {
            dmach_t *dc=&g_dma[controller*4+i];
            if (dc->tcount) status|=1u<<i;
            if (dc->request) status|=1u<<(i+4);
            dc->tcount=0;
        }
        return status;
    }
    return 0xff;
}

/* ================= SB DSP command FSM ================= */
static int  g_dsp_args_left;          /* bytes still expected for the current command */
static int  g_dsp_cmd;                /* current command byte */
static unsigned char g_dsp_arg[4];
static int  g_dsp_argi;
enum { DSP_RESET, DSP_RESET_WAIT, DSP_NORMAL };
static int g_dsp_state = DSP_NORMAL;
static unsigned g_dsp_write_busy;
static unsigned char g_last_read;
static int  g_read_val = -1;          /* pending DSP data-read byte (0xAA after reset, version, ...) */
static int  g_irq_pending;            /* completion bits: 1 = PCM8, 2 = PCM16 */
static unsigned g_hw_irq = 7;        /* matched DOSBox/default device configuration */
static unsigned char g_mixer_index;
static int  g_speaker;                /* DSP speaker enable (D1/D3) */
static int  g_block16;                /* current transfer is 16-bit */
static int  g_playing;
static unsigned g_dsp_total, g_dsp_left;
static int g_dsp_autoinit, g_paused, g_irq_delivered, g_dma_active;
static unsigned char g_pcm8_scratch[65536];

static void (*g_irq_cb)(void);        /* legacy completion callback; protected PIC/CPU delivery is open */
void fist_sb_set_irq_cb(void (*cb)(void)) { g_irq_cb = cb; }

static void raise_irq(unsigned bit)
{
    /* Original SB_RaiseIRQ coalesces each width before activating its PIC line. */
    if (g_irq_pending & bit) return;
    g_irq_pending |= bit;
    g_irq_delivered = 0;
    fist_pic_activate_irq(g_hw_irq);
}

static unsigned read_pcm8(unsigned want, unsigned char *data);

static void dma_mask(unsigned channel, int masked)
{
    dmach_t *dc=&g_dma[channel];
    dc->masked=masked;
    /* DOSBox's DMA_MASKED callback drains its 3-ms minimum before stopping. */
    if (channel==1 && !g_block16 && g_playing && g_dma_active && masked) {
        read_pcm8((unsigned)g_rate*3/1000, g_pcm8_scratch);
        g_dma_active=0;
    } else if (channel==1 && !g_block16 && !masked && g_playing && !g_paused) {
        g_dma_active=1;
    }
}

static void start_pcm8(int autoinit)
{
    g_block16=0;
    g_dsp_autoinit=autoinit;
    g_dsp_left=g_dsp_total;
    g_playing=1;
    g_paused=0;
    g_dma_active=!g_dma1.masked;
    g_irq_pending=g_irq_delivered=0;
    g_dma1.request=1;
}

/* DOSBox DmaChannel::Read increments current registers and reloads the DMA ring
 * independently of the DSP block. The reached PCM8 callback clips to DSP left. */
static unsigned read_pcm8(unsigned want, unsigned char *data)
{
    dmach_t *dc=&g_dma1;
    unsigned read=0;
    if (g_dsp_autoinit) {
        if (want>=g_dsp_left) want=g_dsp_left;
    } else if (g_dsp_left<=(unsigned)g_rate*3/1000) want=g_dsp_left;
    dc->curraddr&=0xffff;
    while (want) {
        unsigned left=dc->currcnt+1;
        /* The original DMA reader accesses want bytes before handling terminal
         * count, even when only left bytes are returned in single-cycle mode. */
        if (dc->curraddr+want>0x10000) {
            fprintf(stderr,"[sb] DMA segbound wrapping (read)\n");
            abort();
        }
        memcpy(data+read, g_mem+(dc->page<<16)+dc->curraddr, want);
        if (want<left) {
            dc->curraddr+=want;
            dc->currcnt-=want;
            read+=want;
            break;
        }
        read+=left;
        want-=left;
        dc->tcount=1;
        if (dc->autoinit) {
            dc->curraddr=dc->addr;
            dc->currcnt=dc->count;
        } else {
            dc->curraddr+=left;
            dc->currcnt=0xffff;
            dc->masked=1;
            break;
        }
    }
    for (unsigned i=0; i<read; ++i) emit((short)(((int)data[i]-128)*256));
    g_dsp_left-=read;
    if (!g_dsp_left) {
        if (g_dsp_autoinit) g_dsp_left=g_dsp_total;
        else g_playing=0;
        raise_irq(1);
    }
    return read;
}

unsigned fist_sb_read_pcm8(unsigned want, unsigned char *data)
{
    dma_init();
    if (!fist_sb_enabled() || !g_playing || g_block16 || g_paused || !g_dma_active)
        return 0;
    if (want>g_dsp_left) want=g_dsp_left; /* SBLASTER_CallBack */
    return read_pcm8(want,data);
}
unsigned fist_sb_dma_left(void) { return g_dsp_left; }

/* Decode+emit the PCM block the engine placed at the DMA-programmed address. len = transfer bytes. */
static void run_dma_block(void)
{
    dmach_t *dc = g_block16 ? &g_dma5 : &g_dma1;
    unsigned phys, bytes;
    if (g_block16) { phys = (dc->page<<16) | (dc->addr<<1); bytes = g_dsp_total*2; }
    else           { phys = (dc->page<<16) |  dc->addr;      bytes = g_dsp_total;    }
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
    g_playing = g_dsp_autoinit;         /* auto-init keeps running until DSP 0xDA */
    raise_irq(g_block16 ? 2 : 1); /* acknowledgement width is independent */
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
        case 0x48: g_dsp_total = 1+g_dsp_arg[0]+(g_dsp_arg[1]<<8); break; /* set 8-bit block size */
        case 0x14: case 0x15: case 0x91:                                     /* 8-bit single-cycle out */
            g_dsp_total=1+g_dsp_arg[0]+(g_dsp_arg[1]<<8); start_pcm8(0); break;
        case 0xb0: case 0xb2: case 0xb4: case 0xb6:                          /* SB16 16-bit out: mode,lenlo,lenhi */
            g_block16=1; g_dsp_total=1+g_dsp_arg[1]+(g_dsp_arg[2]<<8); g_dsp_autoinit=!!(g_dsp_cmd&0x04); run_dma_block(); break;
        case 0xc0: case 0xc2: case 0xc4: case 0xc6:                          /* SB16 8-bit out: mode,lenlo,lenhi */
            g_block16=0; g_dsp_total=1+g_dsp_arg[1]+(g_dsp_arg[2]<<8); g_dsp_autoinit=!!(g_dsp_cmd&0x04); run_dma_block(); break;
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
        start_pcm8(1); break;
    case 0xd1: g_speaker = 1; break;                                         /* speaker on */
    case 0xd3: g_speaker = 0; break;                                         /* speaker off */
    case 0xd0: g_paused = 1; g_dma_active=0; break;                                         /* pause 8-bit DMA */
    case 0xd4: g_paused = 0; g_dma_active=g_playing && !g_dma1.masked; break;                                         /* resume 8-bit DMA */
    case 0xda: g_dsp_autoinit = 0; break;                                         /* exit 8-bit auto-init */
    case 0xd9: g_playing = 0; break;                                         /* exit 16-bit auto-init */
    case 0xe1: g_read_val = 4; break;                                        /* DSP version major (SB16=4); minor next read */
    default: break;
    }
}

static void dsp_finish_reset(unsigned value)
{
    (void)value;
    g_read_val = 0xaa;
    g_dsp_state = DSP_NORMAL;
}
static void dsp_reset(int value)
{
    /* Original DSP_DoReset tests bit zero, not literal byte values. */
    if ((value & 1) && g_dsp_state != DSP_RESET) {
        fist_pic_deactivate_irq(g_hw_irq);
        fist_clock_remove_events(dsp_finish_reset);
        g_read_val = -1;
        g_dsp_args_left = g_dsp_argi = 0;
        g_dsp_write_busy = 0;
        g_playing = g_dma_active = 0;
        g_dsp_total = g_dsp_left = 0;
        g_dsp_autoinit = g_paused = g_block16 = 0;
        g_irq_pending = g_irq_delivered = 0;
        g_dma1.request = 0;
        g_rate = 22050;
        g_dsp_state = DSP_RESET;
    } else if (!(value & 1) && g_dsp_state == DSP_RESET) {
        g_dsp_state = DSP_RESET_WAIT;
        fist_clock_remove_events(dsp_finish_reset);
        fist_clock_add_event(dsp_finish_reset, 20.0f / 1000.0f, 0);
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
    dma_init();
    port &= 0xffff; val &= 0xff;
    if (port >= g_base && port <= g_base+0xf) {
        switch (port - g_base) {
        case 0x4: g_mixer_index=(unsigned char)val; break;
        case 0x6:                                                           /* DSP reset */
            dsp_reset(val);
            break;
        case 0xc: dsp_command(val); break;                                  /* DSP write command/data */
        }
        return;
    }
    if (port==0x83){ g_dma1.page=val; return; }
    if (port==0x8b){ g_dma5.page=val; return; }
    if (port<=0x0f) dma_out(0,port,val);
    else if (port>=0xc0 && port<=0xdf) dma_out(1,(port-0xc0)>>1,val);
}

int fist_sb_in(int port)
{
    dma_init();
    port &= 0xffff;
    if (port >= g_base && port <= g_base+0xf) {
        switch (port - g_base) {
        case 0x4: return g_mixer_index;
        case 0x5:
            if (g_mixer_index==0x82) return g_irq_pending;
            return 0xff; /* other mixer-register contracts remain open */
        case 0xe:                                                           /* read-buffer status: bit7 = data avail */
            if (g_irq_pending & 1) fist_pic_deactivate_irq(g_hw_irq);
            g_irq_pending &= ~1;                                           /* only the 8-bit completion */
            return (g_read_val>=0) ? 0xff : 0x7f;
        case 0xf: g_irq_pending &= ~2; return 0xff;                       /* only the 16-bit completion */
        case 0xa: {                                                         /* DSP data read */
            int v = (g_read_val>=0)? g_read_val : g_last_read;
            g_last_read = (unsigned char)v;
            g_read_val = (g_dsp_cmd==0xe1 && v==4) ? 0x05 : -1;            /* version minor after major */
            return v; }
        case 0xc:
            if (g_dsp_state != DSP_NORMAL) return 0xff;
            return (++g_dsp_write_busy & 8) ? 0xff : 0x7f;                                             /* write-buffer status: bit7=0 -> ready */
        }
        return 0xff;
    }
    if (port==0x83) return g_dma1.page;
    if (port==0x8b) return g_dma5.page;
    if (port<=0xf) return dma_in(0,port);
    if (port>=0xc0 && port<=0xdf) return dma_in(1,(port-0xc0)>>1);
    return 0xff;
}

/* Deliver a latched completion once. Only the DSP ACK clears the latch; a
 * single-cycle transfer also delivers its IRQ after playback has stopped. */
void fist_sb_pump(void)
{
    if (!fist_sb_enabled()) return;
    if (g_irq_pending && !g_irq_delivered && g_irq_cb) {
        g_irq_delivered=1;
        g_irq_cb();
    }
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
