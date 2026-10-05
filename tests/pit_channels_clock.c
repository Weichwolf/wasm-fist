
#include "fist_cpu.h"
#include "fist_vga.c"

static FistPit full_pit;
#define host (*fist_clock_pit_host())
static void pit_speaker(void *context,unsigned count,unsigned mode)
{printf("{\"kind\":\"speaker-request\",\"count\":%u,\"mode\":%u}\n",count,mode);}
static void pit_speaker_type(void *context,unsigned type)
{printf("{\"kind\":\"speaker-type-request\",\"type\":%u,\"gate2\":%u,\"port61\":%u}\n",type,full_pit.gate2,g_port61);}

/* The source controlled producer has rawFLAGS0; IRQ requests are observed,
 * and guest hardware-frame/IF/startup transport is outside this fixture. */
static FistCpuState controlled_cpu;

void begin_lifecycle(void)
{fist_clock_bind_cpu(&controlled_cpu);pic_tick_sync(0);g_cpu_remaining=0;g_cpu_time=clock_now();fist_clock_bind_pit(&full_pit,NULL,pit_speaker,pit_speaker_type);fist_pit_initialize(&full_pit,&host);}
void lifecycle_fetch(unsigned count) {fist_clock_charge_cpu_instructions(count);}

void lifecycle_out(unsigned port,unsigned value)
{
 out(port,value);
}
void lifecycle_read(unsigned label,unsigned port)
{
 printf("{\"kind\":\"read\",\"label\":%u,\"port\":%u,\"value\":%u}\n",label,port,(unsigned)in(port));
}
void lifecycle_gate(unsigned enabled) {fist_pit_gate2(&full_pit,&host,enabled);}
void lifecycle_init_again(void) {fist_pit_initialize(&full_pit,&host);}
void lifecycle_detach(void) {fist_clock_bind_pit(NULL,NULL,NULL,NULL);}
extern void lifecycle_irqs(uint32_t *);
void lifecycle_snapshot(const char *kind,unsigned label)
{
 uint64_t cycle=clock_cpu_cycles(clock_now());uint32_t irqs[66];lifecycle_irqs(irqs);
 if(g_pit_context)fist_cpu_require((unsigned)fist_vga_pit0_div()==full_pit.counters[0].cntr);
 printf("{\"kind\":\"state\",\"label\":%u,\"tick\":%u,\"index_nd\":%d,\"cycles\":%u,\"left\":%u,\"IRQCheck\":%u,\"IRQActive\":%u,\"service\":%u,\"io_removed\":%llu,\"irqs\":[",label,(unsigned)g_pic_tick,(int)pic_index(cycle),g_cpu_remaining,30000u-(unsigned)pic_index(cycle)-g_cpu_remaining,irqs[64],irqs[65],g_pic_service,(unsigned long long)fist_clock_io_delay_removed());
 for(unsigned i=0;i<16;i++)printf("%s[%u,%u,%u,%u]",i?",":"",irqs[4*i],irqs[4*i+1],irqs[4*i+2],irqs[4*i+3]);
 printf("],\"counters\":[");
 for(unsigned i=0;i<3;i++) {
  FistPitCounter *p=&full_pit.counters[i];uint32_t delay;uint64_t start;memcpy(&delay,&p->delay,4);memcpy(&start,&p->start,8);
  printf("%s[%u,%u,%llu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u]",i?",":"",p->cntr,delay,(unsigned long long)start,p->read_latch,p->write_latch,p->mode,p->latch_mode,p->read_state,p->write_state,p->bcd,p->go_read_latch,p->new_mode,p->counterstatus_set,p->counting,p->update_count,fist_pit_output(&full_pit,&host,i));
 }
 printf("],\"gate2\":%u,\"status\":%u,\"status_locked\":%u,\"port61\":%u,\"queue\":[",full_pit.gate2,full_pit.status,full_pit.status_locked,g_port61);
 unsigned count=0,free_count=0;
 for(FistPicEntry *e=g_pic_events;e;e=e->next) {
  fist_cpu_require(e->handler==fist_clock_pit_event);uint32_t bits,deadline;memcpy(&bits,&e->index,4);
  volatile float cycles=e->index*30000.0f;memcpy(&deadline,(const void *)&cycles,4);
  printf("%s[%u,%u,%u]",count++?",":"",bits,deadline,e->value);
 }
 for(FistPicEntry *e=g_pic_free;e;e=e->next)fist_cpu_require(++free_count<=512);
 fist_cpu_require(count+free_count+(g_pic_service?1u:0u)==512);
 printf("],\"free_entries\":%u}\n",free_count);fflush(stdout);
}
void lifecycle_budget(void)
{
    for(unsigned i=0;i<368;i++) {
        /* Original controlled budget command explicitly advances TIMER ticks,
         * resets CPU_Cycles0/CycleLeft1 and calls PIC_RunQueue at each row. */
        if(g_pic_tick<i)pic_tick_sync((uint64_t)i*30000u);
        uint64_t cycle=(uint64_t)i*30000u+29999u,numerator=cycle*PIT_HZ_;
        clock_set((FistClock){numerator/CPU_HZ_,numerator%CPU_HZ_});
        g_cpu_remaining=0;g_cpu_time=clock_now();cpu_slice_start();
        for(unsigned step=0;step<2;step++) {
            lifecycle_fetch(1);lifecycle_snapshot("state",2*i+step);
        }
    }
}
