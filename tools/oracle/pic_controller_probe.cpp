/* Actual original 8259/PIC/DSP/DMA producers. CPU interrupt-frame construction
 * is excluded: the boundary records the vector selected by original PIC. */
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/pic.cpp"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

Bit32s CPU_Cycles = 0, CPU_CycleLeft = 30000, CPU_CycleMax = 30000;
Bit64s CPU_IODelayRemoved = 0;
MachineType machine = MCH_VGA;
Segments Segs;
CPU_Regs cpu_regs;
CPU_Decoder *cpudecoder;
static int delivered = -1;
Bits CPU_Core_Normal_Trap_Run(void) { abort(); }
void CPU_Interrupt(Bitu vector, Bitu, Bitu) { assert(delivered == -1); delivered = vector; }
void E_Exit(const char *, ...) { abort(); }
void GFX_ShowMsg(const char *, ...) { abort(); }
void source_io_read_delay(void);
void source_io_write_delay(void);
extern "C" {
void original_sb_init(void);
void fist_sb_out(int,int);
int fist_sb_in(int);
unsigned fist_sb_read_pcm8(unsigned,unsigned char *);
unsigned fist_sb_dma_left(void);
unsigned fist_sb_ring_count(void);
int fist_sb_rate(void);
extern unsigned char *g_mem;
}

static void original_write_pic(unsigned port,unsigned value)
{
    if (port&1) write_data(port,value,1); else write_command(port,value,1);
}
static int original_take_irq(unsigned flags,int trap,unsigned *vector)
{
    bool before[16]; for (unsigned i=0;i<16;++i) before[i]=irqs[i].active;
    cpu_regs.flags=flags; cpudecoder=trap ? CPU_Core_Normal_Trap_Run : NULL;
    delivered=-1; PIC_runIRQs();
    int selected=-1;
    if (delivered!=-1) for (unsigned i=0;i<16;++i)
        if (before[i] && !irqs[i].active) { assert(selected==-1); selected=i; }
    assert((selected==-1)==(delivered==-1));
    if (selected!=-1) *vector=delivered;
    // Explicit diagnostic gate, without executing a guest interrupt frame.
    cpu_regs.flags=0; cpudecoder=NULL;
    return selected;
}
#include "../../tests/pic_controller_setup.h"

int main(int argc, char **argv)
{
    assert(argc == 3 || argc == 4);
    setvbuf(stdout,NULL,_IONBF,0);
    PIC_8259A controller(NULL);
    original_sb_init();
    for (unsigned i = 0; i < 0x10000; ++i) g_mem[i] = (unsigned char)((i*29)^(i>>8));
    if (argc==4) {
        assert(!strcmp(argv[3],"sb-irq"));
        pic_controller_setup_sb_irq(original_write_pic,original_take_irq);
    }
    unsigned long long start = strtoull(argv[1],NULL,10);
    PIC_Ticks = start/30000u; CPU_CycleLeft = 30000-start%30000u;
    assert(PIC_RunQueue());
    FILE *script = fopen(argv[2],"r"); assert(script);
    char op; unsigned port,value,count=0; int fields;
    static unsigned char data[65536];
    while ((fields=fscanf(script," %c %x %x",&op,&port,&value))==3) {
        while (CPU_Cycles-- <= 0) while (!PIC_RunQueue()) TIMER_AddTick();
        unsigned long long fetched = PIC_Ticks*30000u+PIC_TickIndexND();
        if (op=='r') {
            source_io_read_delay();
            unsigned actual = port==0x20 || port==0xa0 ? read_command(port,1) :
                              port==0x21 || port==0xa1 ? read_data(port,1) : fist_sb_in(port);
            assert(actual==value); printf("read %x %x\n",port,actual);
        } else if (op=='w') {
            source_io_write_delay();
            if (port==0x20 || port==0xa0) write_command(port,value,1);
            else if (port==0x21 || port==0xa1) write_data(port,value,1);
            else fist_sb_out(port,value);
        } else if (op=='h') PIC_ActivateIRQ(port);
        else if (op=='l') PIC_DeActivateIRQ(port);
        else if (op=='m') PIC_SetIRQMask(port,value!=0);
        else if (op=='j') {
            unsigned vector=0; int selected=original_take_irq(port,value!=0,&vector);
            printf("irq %d %d\n",selected,selected==-1 ? -1 : (int)vector);
        } else if (op=='g') {
            assert(value<=sizeof data); unsigned n=fist_sb_read_pcm8(value,data);
            printf("data %u ",n); for (unsigned i=0;i<n;++i) printf("%02x",data[i]); puts("");
        } else if (op=='s') {
            printf("state %u %u %d\n",fist_sb_dma_left(),fist_sb_ring_count(),fist_sb_rate());
        } else assert(op=='n');
        printf("clock %llu %llu %d\n",fetched,
               (unsigned long long)(PIC_Ticks*30000u+PIC_TickIndexND()),CPU_Cycles);
        ++count;
    }
    assert(fields==EOF && !ferror(script) && !fclose(script) && count);
}
