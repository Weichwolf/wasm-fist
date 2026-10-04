/* The original timer owns every counter/control write and callback. The trace
 * build forwards IRQ activation to the original PIC before observing it. */
#include <assert.h>
#ifdef FIST_TIMER_TRACE
#define PIC_ActivateIRQ source_timer_activate_irq
#endif
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/timer.cpp"
#ifdef FIST_TIMER_TRACE
#undef PIC_ActivateIRQ
#endif

extern "C" void source_pit_init(void) { new TIMER(NULL); }
extern "C" PIC_EventHandler source_pit_handler(void) { return PIT0_Event; }
extern "C" void source_pit_write(unsigned port, unsigned value)
{
    assert((port==0x40 || port==0x43) && value<=255);
    if (port==0x40) write_latch(port,value,1);
    else write_p43(port,value,1);
}
extern "C" unsigned source_pit_read(void) { return read_latch(0x40,1); }
extern "C" void source_pit_dump(void)
{
    const PIT_Block &p=pit[0];
    unsigned delay; unsigned long long start;
    static_assert(sizeof delay==sizeof p.delay,"float observer width");
    static_assert(sizeof start==sizeof p.start,"double observer width");
    memcpy(&delay,&p.delay,sizeof delay); memcpy(&start,&p.start,sizeof start);
    printf("[%u,%u,%llu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u]",
        (unsigned)p.cntr,delay,start,(unsigned)p.read_latch,(unsigned)p.write_latch,
        (unsigned)p.mode,(unsigned)p.latch_mode,(unsigned)p.read_state,
        (unsigned)p.write_state,(unsigned)p.bcd,(unsigned)p.go_read_latch,
        (unsigned)p.new_mode,(unsigned)p.counterstatus_set,(unsigned)p.counting,
        (unsigned)p.update_count,(unsigned)counter_output(0));
}

/* Unreached PC-speaker/guest-memory paths fail; this fixture covers PIT0 only.
 * FIST_PITTRACE is cleared by its runner so guest trace reads are not reached. */
void PCSPEAKER_SetCounter(Bitu,Bitu) { abort(); }
Bit16u mem_readw(PhysPt) { abort(); }
