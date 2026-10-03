/* Compile the actual original DMA and DSP producers. The mixer test endpoint
 * records PCM8 input bytes; it does not claim resampling or final-mixer coverage. */
#define test dma_module
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/dma.cpp"
#undef test
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/sblaster.cpp"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

HostPt MemBase;
PagingBlock paging;
volatile int fist_memread_armed=0;
extern "C" void fist_memread(Bit8u *) { abort(); }
volatile int fist_mem_armed=0;
extern "C" void fist_memrec(Bit8u *, Bit8u) { abort(); }
Bit8u MixTemp[MIXER_BUFSIZE];
extern "C" { unsigned char *g_mem; }
static unsigned consumed, produced;
static unsigned char *destination;
static bool irq_delivered;
static void (*irq_cb)(void);
static MixerChannel channel;
#ifdef FIST_SB_EVENT_CLOCK
static FILE *sample_sink;
#endif

void DEBUG_ShowMsg(const char *format, ...) {}
#ifndef FIST_SB_EVENT_CLOCK
void E_Exit(const char *format, ...) { abort(); }
void GFX_ShowMsg(const char *format, ...) { abort(); }
IO_ReadHandleObject::~IO_ReadHandleObject() {}
IO_WriteHandleObject::~IO_WriteHandleObject() {}
void PIC_ActivateIRQ(Bitu irq) { if (irq!=7) abort(); irq_delivered=false; }
void PIC_DeActivateIRQ(Bitu irq) { if (irq!=7) abort(); }
void PIC_RemoveEvents(PIC_EventHandler handler) {}
void PIC_AddEvent(PIC_EventHandler handler, float delay, Bitu val) {
    /* These cases reach no scheduled PIC event. Expanding into the clock
     * contract must fail here until a real event endpoint is provided. */
    abort();
}
#endif
void MixerChannel::FillUp(void) {}
void MixerChannel::SetFreq(Bitu freq) { freq_add=freq; }
/* Retained by the original port switch, outside these DMA/IRQ cases. */
void MixerChannel::Enable(bool) { abort(); }
void MixerChannel::SetVolume(float, float) { abort(); }
MixerChannel *MIXER_FindChannel(const char *) { abort(); }
void MIDI_RawOutByte(Bit8u) { abort(); }
void MixerChannel::AddSamples_m8(Bitu len, const Bit8u *data) {
    produced+=len;
    if (destination) { memcpy(destination+consumed,data,len); consumed+=len; }
#ifdef FIST_SB_EVENT_CLOCK
    if (sample_sink) {
        for (Bitu i=0; i<len; ++i) {
            unsigned sample=(Bit16u)((data[i]-128)*256);
            assert(fputc(sample&255,sample_sink)!=EOF);
            assert(fputc(sample>>8,sample_sink)!=EOF);
        }
        assert(!fflush(sample_sink));
    }
#endif
}
#define UNREACHED(name, type) \
    void MixerChannel::name(Bitu, const type *) { abort(); }
UNREACHED(AddSamples_m8s, Bit8s)
UNREACHED(AddSamples_s8, Bit8u)
UNREACHED(AddSamples_s8s, Bit8s)
UNREACHED(AddSamples_m16, Bit16s)
UNREACHED(AddSamples_m16u, Bit16u)
UNREACHED(AddSamples_s16, Bit16s)
UNREACHED(AddSamples_s16u, Bit16u)

extern "C" void original_sb_init(void) {
    g_mem=MemBase=(Bit8u *)calloc(0x1000000,1);
    for (unsigned i=0; i<LINK_START; ++i) paging.firstmb[i]=i;
    UpdateEMSMapping();
    DmaControllers[0]=new DmaController(0);
    DmaControllers[1]=new DmaController(1);
    sb.type=SBT_16;
    sb.hw.base=0x220;
    sb.hw.dma8=1;
    sb.hw.irq=7;
    sb.freq=22050;
    sb.chan=&channel;
#ifdef FIST_SB_EVENT_CLOCK
    sb.dsp.state=DSP_S_NORMAL;
    const char *samples=getenv("FIST_SB_PROBE_PCM");
    if (samples) { sample_sink=fopen(samples,"wb"); assert(sample_sink); }
#endif
}

extern "C" void fist_sb_out(int port, int val) {
#ifdef FIST_SB_EVENT_CLOCK
    /* Event cases execute the actual command dispatcher, including pause,
     * restart and reset cancellation, rather than the demand-only adapter. */
    if (port>=0x220 && port<=0x22f) { write_sb(port,val,1); return; }
#endif
    if (port==0x226) { write_sb(port,val,1); return; }
    if (port==0x224) { write_sb(port,val,1); return; }
    if (port!=0x22c) { DMA_Write_Port(port,val,1); return; }
    /* Feed only the captured PCM8 commands. The actual DSP DMA preparation,
     * callback, byte consumption, terminal count and IRQ producers run above. */
    static int command=-1, args=0;
    if (args) {
        sb.dsp.in.data[sb.dsp.in.pos++]=val;
        if (--args) return;
        if (command==0x40) sb.freq=1000000/(256-sb.dsp.in.data[0]);
        else if (command==0x48)
            sb.dma.total=1+sb.dsp.in.data[0]+(sb.dsp.in.data[1]<<8);
        else DSP_PrepareDMA_Old(DSP_DMA_8,false,false);
        return;
    }
    command=val;
    sb.dsp.in.pos=0;
    switch (val) {
    case 0x40: args=1; break;
    case 0x48: case 0x14: case 0x15: case 0x91: args=2; break;
    case 0x1c: case 0x90: DSP_PrepareDMA_Old(DSP_DMA_8,true,false); break;
    case 0xd1: sb.speaker=true; break;
    case 0xd3: sb.speaker=false; break;
    case 0xd0: sb.mode=MODE_DMA_PAUSE; break;
    case 0xd4: sb.mode=sb.dma.chan->masked ? MODE_DMA_MASKED : MODE_DMA; break;
    case 0xda: sb.dma.autoinit=false; break;
    case 0xe1: write_sb(port,val,1); break; /* actual version-byte producer */
    default: abort();
    }
}
extern "C" int fist_sb_in(int port) {
    if (port>=0x220 && port<=0x22f) return read_sb(port,1);
    return DMA_Read_Port(port,1)&0xff;
}
extern "C" unsigned fist_sb_read_pcm8(unsigned want, unsigned char *data) {
    if (sb.mode!=MODE_DMA) return 0;
    if (want>sb.dma.left) want=sb.dma.left; /* SBLASTER_CallBack */
    destination=data;
    consumed=0;
    GenerateDMASound(want);
    destination=NULL;
    return consumed;
}
extern "C" unsigned fist_sb_dma_left(void) { return sb.dma.left; }
extern "C" unsigned fist_sb_ring_count(void) { return produced; }
extern "C" int fist_sb_rate(void) { return sb.freq; }
extern "C" void fist_sb_flush(void) {}
extern "C" void fist_sb_set_irq_cb(void (*cb)(void)) { irq_cb=cb; }
extern "C" void fist_sb_pump(void) {
    if (sb.irq.pending_8bit && !irq_delivered && irq_cb) {
        irq_delivered=true;
        irq_cb();
    }
}
