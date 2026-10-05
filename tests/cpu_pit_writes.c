/* One actual before-control seed, then real instruction/I/O/PIT/PIC owners. */
#define main unused_core_exit_main
#define state core_state
#include "cpu_core_exit.c"
#undef state
#undef main
#include "fist_pit.h"
extern void bind_pit_clock(void),observe_pit_devices(const char *,const char *);
extern void out(int,int);
static void state(const char *kind)
{core_state(kind);observe_pit_devices(output,kind);}
static void write_byte(void *opaque,unsigned port,unsigned value) {out(port,value);}
int main(int argc,char **argv)
{
    load_core_inputs(argc,argv);bind_pit_clock();
    FistExec engine={.bus=&context.bus,.out=write_byte};
    const char *names[]={"pit-control","pit-low","pit-high"};
    unsigned writes=0,after=0;
    for(unsigned count=0;;count++) {
        fist_cpu_require(count<20);char kind[64];
        if(after) {
            snprintf(kind,sizeof kind,"after-%s",names[writes-1]);state(kind);
            after=0;if(writes==3)break;
        }
        unsigned op=fist_ram_resident_read(&context.bus,1,cpu.eip,1);
        if(op==0xe6) {
            fist_cpu_require(writes<3);
            snprintf(kind,sizeof kind,"before-%s",names[writes]);state(kind);writes++;after=1;
        }
        unsigned checks=fist_exec_fetched(&engine),trap_decoder=0;
        fist_cpu_require(!fist_exec_core_exit(&engine,checks,fist_pic_pending_irqs(),&trap_decoder));
        fist_clock_charge_cpu_instructions(1);
    }
    fist_clock_bind_pit(NULL,NULL,NULL,NULL);fixture_destroy(&context);return 0;
}
