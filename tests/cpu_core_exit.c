/* Continuous reaching original IRET/core/PIC/frame regression; one seed only. */
#define fist_int8_fire unused_int8
#include "sb_clock_fixture.h"
#undef fist_int8_fire
void fist_int8_fire(void) {abort();}
#include "fist_exec.h"
#include "memory_context_fixture.h"
extern void restore_core_clock(FILE *),restore_core_pic(FILE *),observe_core_pic(FILE *);
extern void observe_core_clock(uint64_t *,unsigned *);
extern unsigned fist_clock_cpu_slice(uint64_t *);
#ifdef FIST_CORE_EXIT_HOOKS
static void core_fixture_start(void),core_fixture_stop(void),core_fixture_observe(const char *);
#endif
static FistCpuState cpu;
static FistCpuSystem sys;
static FistMemoryFixture context;
static const char *output;
static void observe_cpu_to(FILE *stream,const char *kind)
{
    uint32_t q[58];memcpy(q,&cpu,sizeof cpu);memcpy(q+37,&sys,sizeof sys);
    uint64_t time,tick;unsigned budget=fist_clock_cpu_slice(&time),left;
    observe_core_clock(&tick,&left);
    fprintf(stream,"%s %llu %llu %u %u",kind,(unsigned long long)time,(unsigned long long)tick,budget,left);
    for(unsigned i=0;i<58;i++)fprintf(stream," %08x",q[i]);fputc('\n',stream);fflush(stream);
}
static void observe_cpu(const char *kind) {observe_cpu_to(stdout,kind);}
static void state(const char *kind)
{
    observe_cpu(kind);
    char path[1024];snprintf(path,sizeof path,"%s-%s.memory",output,kind);FILE *f=fopen(path,"wb");
    fist_cpu_require(f && fwrite(g_mem,16777216,1,f)==1 && !fclose(f));
    snprintf(path,sizeof path,"%s-%s.context",output,kind);f=fopen(path,"wb");
    fist_cpu_require(f!=NULL);fixture_cache(&context,f);fist_cpu_require(!fclose(f));
    snprintf(path,sizeof path,"%s-%s.pic",output,kind);f=fopen(path,"wb");
    fist_cpu_require(f!=NULL);observe_core_pic(f);fist_cpu_require(!fclose(f));
#ifdef FIST_CORE_EXIT_HOOKS
    core_fixture_observe(kind);
#endif
}
static void deliver(void *opaque,unsigned vector)
{
    state("before-hardware");fist_cpu_hw_interrupt(&context.bus,vector);state("after-hardware");
}
#ifdef FIST_CORE_EXIT_CONTINUE
static void handler_prefix(void);
#endif
static void load_core_inputs(int argc,char **argv)
{
    fist_cpu_require(argc==4);FILE *f=fopen(argv[1],"rb");fist_cpu_require(f!=NULL);
    fist_cpu_require(fread(&cpu,sizeof cpu,1,f)==1 && fread(&sys,sizeof sys,1,f)==1);
    FILE *memory=fopen(argv[2],"rb");
    fist_cpu_require(memory && fread(g_mem,16777216,1,memory)==1 && fgetc(memory)==EOF && !fclose(memory));
    fixture_restore(&context,f,&cpu,&sys,g_mem,16777216);restore_core_pic(f);restore_core_clock(f);
    fist_cpu_require(fgetc(f)==EOF && !fclose(f));fist_clock_bind_cpu(&cpu);output=argv[3];
}
int main(int argc,char **argv)
{
    load_core_inputs(argc,argv);
#ifdef FIST_CORE_EXIT_HOOKS
    core_fixture_start();
#endif
    FistExec engine={.bus=&context.bus};state("before-iret");
    unsigned checks=fist_exec_fetched(&engine);state("after-iret");unsigned trap_decoder=0;
    fist_cpu_require(fist_exec_core_exit(&engine,checks,fist_pic_pending_irqs(),&trap_decoder) && !trap_decoder);
    fist_clock_cpu_core_exit();state("after-core");unsigned vector;
    fist_cpu_require(fist_pic_dispatch_irq(cpu.flags.flags,trap_decoder,&vector,deliver,NULL)==0 && vector==8);
    state("after-queue");fist_clock_charge_cpu_instructions(1);state("handler-fetch");
#ifdef FIST_CORE_EXIT_CONTINUE
    handler_prefix();
#endif
#ifdef FIST_CORE_EXIT_HOOKS
    core_fixture_stop();
#endif
    fixture_destroy(&context);return 0;
}
