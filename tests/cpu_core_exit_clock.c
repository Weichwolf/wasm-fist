/* Matched original observation inputs, for a bounded core/PIC/frame regression.
 * Runtime machine initialization is not supplied by this fixture. */
#include "fist_cpu.h"
#include "fist_vga.c"
#include <stdio.h>
static void unsupported_calendar_event(unsigned value) {abort();}
void prepare_core_queue(void)
{
    /* Controlled source initialization calls PIC_RunQueue before entering the
     * normal core. No failed fetch or MOV/POP SS has occurred at this boundary. */
    cpu_slice_start();
}
void restore_core_clock(FILE *input)
{
    uint32_t q[4];fist_cpu_require(fread(q,sizeof q,1,input)==1);
    uint64_t cycle=(uint64_t)q[0]*30000u+30000u-q[1]-q[2];
    uint64_t numerator=cycle*PIT_HZ_;
    clock_set((FistClock){numerator/CPU_HZ_,numerator%CPU_HZ_});
    pic_tick_sync(cycle);fist_cpu_require(g_pic_tick==q[0]);
    g_cpu_remaining=q[1];g_cpu_time=clock_now();
    FistPicEntry **tail=&g_pic_events;
    fist_cpu_require(q[3]<PIC_QUEUE_SIZE);
    for(unsigned i=0;i<q[3];i++) {
        uint32_t row[2];fist_cpu_require(fread(row,sizeof row,1,input)==1 && g_pic_free);
        FistPicEntry *entry=g_pic_free;g_pic_free=entry->next;
        memcpy(&entry->index,&row[0],4);entry->value=row[1];
        /* Original calendar deadlines are complete matched inputs. The source
         * chain reaches no device event; encountering one fails, never skips it. */
        fist_cpu_require(pic_event_cycles(entry->index)>pic_index(cycle));
        entry->handler=unsupported_calendar_event;entry->next=NULL;
        *tail=entry;tail=&entry->next;
    }
}
void observe_core_clock(uint64_t *ticks,unsigned *left)
{
    uint64_t cycle=clock_cpu_cycles(clock_now());
    *ticks=g_pic_tick;*left=30000u-(unsigned)pic_index(cycle)-g_cpu_remaining;
}
