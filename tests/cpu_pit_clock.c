/* Fixture restoration/serialization; every timer effect uses the real owner. */
#include "fist_cpu.h"
#include "fist_pit.h"
#include "fist_vga.c"
static FistPit timer;
static struct {double frame,vrstart,vrend,hstart,hend,htotal,vdend,vtotal;uint32_t attr,pcjr;} status;
static void unsupported_panning(unsigned value) {abort();}
static void unsupported_vertical(unsigned value) {abort();}
static FistPicEvent events[]={unsupported_panning,unsupported_vertical,fist_clock_pit_event};
static uint32_t read_word(FILE *input)
{uint32_t value;fist_cpu_require(fread(&value,4,1,input)==1);return value;}
static void restore_counter(FILE *input,FistPitCounter *p)
{
    p->cntr=read_word(input);uint32_t delay=read_word(input);memcpy(&p->delay,&delay,4);
    fist_cpu_require(fread(&p->start,8,1,input)==1);
    p->read_latch=read_word(input);p->write_latch=read_word(input);
    p->mode=read_word(input);p->latch_mode=read_word(input);p->read_state=read_word(input);p->write_state=read_word(input);
    p->bcd=read_word(input);p->go_read_latch=read_word(input);p->new_mode=read_word(input);
    p->counterstatus_set=read_word(input);p->counting=read_word(input);p->update_count=read_word(input);
}
void restore_core_clock(FILE *input)
{
    uint32_t tick=read_word(input),budget=read_word(input),left=read_word(input),count=read_word(input);
    uint64_t cycle=(uint64_t)tick*30000u+30000u-budget-left,numerator=cycle*PIT_HZ_;
    clock_set((FistClock){numerator/CPU_HZ_,numerator%CPU_HZ_});pic_tick_sync(cycle);
    fist_cpu_require(g_pic_tick==tick && count<PIC_QUEUE_SIZE);
    g_cpu_remaining=budget;g_cpu_time=clock_now();FistPicEntry **tail=&g_pic_events;
    for(unsigned i=0;i<count;i++) {
        uint32_t bits=read_word(input),value=read_word(input),identity=read_word(input);
        fist_cpu_require(identity<3 && g_pic_free);
        FistPicEntry *entry=g_pic_free;g_pic_free=entry->next;
        memcpy(&entry->index,&bits,4);entry->value=value;entry->handler=events[identity];entry->next=NULL;
        fist_cpu_require(pic_event_cycles(entry->index)>pic_index(cycle));
        *tail=entry;tail=&entry->next;
    }
    for(unsigned i=0;i<3;i++)restore_counter(input,&timer.counters[i]);
    timer.gate2=read_word(input);timer.status=read_word(input);timer.status_locked=read_word(input);
    fist_cpu_require(fread(&status,72,1,input)==1 && fread(&g_cpu_io_removed,8,1,input)==1);
}
static void unreached_speaker(void *context,unsigned count,unsigned mode) {abort();}
static void unreached_speaker_type(void *context,unsigned type) {abort();}
void bind_pit_clock(void) {fist_clock_bind_pit(&timer,NULL,unreached_speaker,unreached_speaker_type);}
void observe_core_clock(uint64_t *tick,unsigned *left)
{uint64_t cycle=clock_cpu_cycles(clock_now());*tick=g_pic_tick;*left=30000u-(unsigned)pic_index(cycle)-g_cpu_remaining;}
static void observe_counter(FILE *output,FistPitCounter *p)
{
    uint32_t first[2]={p->cntr,0};memcpy(&first[1],&p->delay,4);
    uint32_t rest[]={p->read_latch,p->write_latch,p->mode,p->latch_mode,p->read_state,p->write_state,
        p->bcd,p->go_read_latch,p->new_mode,p->counterstatus_set,p->counting,p->update_count};
    fist_cpu_require(fwrite(first,8,1,output)==1 && fwrite(&p->start,8,1,output)==1 && fwrite(rest,48,1,output)==1);
}
static FILE *part(const char *prefix,const char *kind,const char *suffix)
{char path[1024];snprintf(path,sizeof path,"%s-%s.%s",prefix,kind,suffix);FILE *f=fopen(path,"wb");fist_cpu_require(f!=NULL);return f;}
void observe_pit_devices(const char *prefix,const char *kind)
{
    FILE *f=part(prefix,kind,"pit");
    for(unsigned i=0;i<3;i++)observe_counter(f,&timer.counters[i]);
    uint32_t global[]={timer.gate2,timer.status,timer.status_locked};
    fist_cpu_require(fwrite(global,12,1,f)==1 && !fclose(f));
    f=part(prefix,kind,"status");fist_cpu_require(fwrite(&status,72,1,f)==1 && !fclose(f));
    f=part(prefix,kind,"io");uint64_t removed=fist_clock_io_delay_removed();
    fist_cpu_require(fwrite(&removed,8,1,f)==1 && !fclose(f));
    f=part(prefix,kind,"calendar");uint32_t count=0;
    for(FistPicEntry *e=g_pic_events;e;e=e->next)count++;
    fist_cpu_require(count<=512 && fwrite(&count,4,1,f)==1);
    for(FistPicEntry *e=g_pic_events;e;e=e->next) {
        uint32_t row[3]={0,e->value,3};memcpy(&row[0],&e->index,4);
        for(unsigned i=0;i<3;i++)if(e->handler==events[i])row[2]=i;
        fist_cpu_require(row[2]<3 && fwrite(row,12,1,f)==1);
    }
    fist_cpu_require(!fclose(f));
}
