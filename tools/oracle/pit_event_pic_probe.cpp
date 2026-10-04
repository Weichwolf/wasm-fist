/* Reuse the actual PIC, I/O, CPU globals and failure paths of the slice oracle. */
#define main fist_pic_slice_probe_main
#include "pic_slice_probe.cpp"
#undef main

extern "C" void source_pit_init(void);
extern "C" PIC_EventHandler source_pit_handler(void);
extern "C" void source_pit_write(unsigned,unsigned);
extern "C" unsigned source_pit_read(void);
extern "C" void source_pit_dump(void);

static void snapshot(const char *kind,unsigned label)
{
    printf("{\"kind\":\"%s\",\"label\":%u,\"tick\":%u,\"index_nd\":%d,"
           "\"cycles\":%d,\"left\":%d,\"irq0\":[%u,%u,%u,%u],"
           "\"IRQCheck\":%u,\"service\":%u,\"pit0\":",kind,label,
           (unsigned)PIC_Ticks,(int)PIC_TickIndexND(),(int)CPU_Cycles,(int)CPU_CycleLeft,
           (unsigned)irqs[0].active,(unsigned)irqs[0].masked,
           (unsigned)irqs[0].inservice,(unsigned)irqs[0].vector,
           (unsigned)PIC_IRQCheck,(unsigned)InEventService);
    source_pit_dump();
    printf(",\"queue\":["); unsigned count=0;
    for (PICEntry *entry=pic_queue.next_entry;entry;entry=entry->next) {
        assert(entry->pic_event==source_pit_handler());
        float deadline=entry->index*CPU_CycleMax; unsigned index,cycles;
        memcpy(&index,&entry->index,sizeof index); memcpy(&cycles,&deadline,sizeof cycles);
        printf("%s[%u,%u,%u]",count++ ? "," : "",index,cycles,(unsigned)entry->value);
        assert(count<=PIC_QUEUESIZE);
    }
    unsigned free_count=0;
    for (PICEntry *entry=pic_queue.free_entry;entry;entry=entry->next) assert(++free_count<=PIC_QUEUESIZE);
    printf("],\"free_entries\":%u}\n",free_count);
    assert(count+free_count+(InEventService ? 1u : 0u)==PIC_QUEUESIZE);
}

/* Only the trace build redirects timer.cpp's external IRQ call. This executes
 * the real PIC producer first; baseline/trace states are compared in full. */
void source_timer_activate_irq(Bitu irq)
{
    PIC_ActivateIRQ(irq);
    assert(irq==0);
    snapshot("irq0-activate",0);
}

static void fetch(unsigned count)
{
    while (count--) {
        while (CPU_Cycles--<=0) while (!PIC_RunQueue()) TIMER_AddTick();
    }
}

int main(int argc,char **argv)
{
    if (argc!=2 && argc!=3) return 2;
    PIC_8259A controller(NULL);
    assert(cpu_regs.flags==0); /* Guest IRQ construction is outside this producer proof. */
    source_pit_init();
    if (argc==2 && !strcmp(argv[1],"budget")) {
        for (unsigned i=0;i<368;++i) {
            while (PIC_Ticks<i) TIMER_AddTick();
            CPU_Cycles=0;CPU_CycleLeft=1;assert(PIC_RunQueue());
            for (unsigned step=0;step<2;++step) {
                fetch(1); snapshot("budget",2*i+step);
            }
        }
        return 0;
    }
    if (argc!=3 || strcmp(argv[1],"lifecycle")) return 2;
    FILE *script=fopen(argv[2],"r"); if (!script) return 2;
    snapshot("state",0);
    char op; unsigned a,b,label=0; int fields;
    while ((fields=fscanf(script," %c %x %x",&op,&a,&b))==3) {
        switch (op) {
        case 'F': assert(!b); fetch(a); break;
        case 'O': source_io_write_delay();source_pit_write(a,b);break;
        case 'R': assert(a==0x40 && !b);source_io_read_delay();
                  printf("{\"kind\":\"read\",\"label\":%u,\"value\":%u}\n",label+1,source_pit_read());break;
        default:abort();
        }
        snapshot("state",++label);
    }
    assert(fields==EOF && feof(script) && !ferror(script));fclose(script);
    return 0;
}
