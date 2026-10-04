/* Explicit controlled real-mode input for original normal-core/PIC cases.
 * IRQ vector selection is the endpoint; the complete original frame regression
 * independently covers the reached application delivery. */
#define fist_int8_fire unused_int8
#include "sb_clock_fixture.h"
#undef fist_int8_fire
void fist_int8_fire(void){abort();}
#include "fist_exec.h"
extern unsigned fist_clock_cpu_slice(uint64_t *);
extern void fist_clock_cpu_ss_instruction(void);
extern void observe_core_clock(uint64_t *,unsigned *);
extern unsigned observe_core_active(void);

static FistCpuState cpu;
static FistCpuSystem sys;
static FistCpuRam bus;
static unsigned selected=0xffffffffu,event_count,trap_decoder;
static void marker(unsigned value){event_count++;}
static void snapshot(const char *kind){
 uint64_t time,tick;unsigned cycles=fist_clock_cpu_slice(&time),left;observe_core_clock(&tick,&left);
 printf("%s %llu %u %u %u %u %u %u %u %u %d",kind,(unsigned long long)time,(unsigned)tick,cycles,left,fist_pic_pending_irqs(),observe_core_active(),selected,event_count,trap_decoder,sys.direction);
 uint32_t regs[9];memcpy(regs,&cpu,sizeof regs);for(unsigned i=0;i<9;i++)printf(" %08x",regs[i]);
 printf(" %08x %08x %08x %08x %08x %08x %08x %u %u\n",cpu.flags.flags,cpu.flags.var1,cpu.flags.var2,cpu.flags.res,cpu.flags.type,cpu.flags.prev_type,cpu.flags.oldcf,cpu.code_big,cpu.stack_big);
}
int main(int argc,char **argv){
 fist_cpu_require(argc==8);unsigned op=strtoul(argv[1],0,0),big=atoi(argv[2]),flip=atoi(argv[3]),pending=atoi(argv[4]),flags=strtoul(argv[5],0,0),budget=atoi(argv[6]),stackbig=atoi(argv[7]);
 fist_pic_set_irq_mask(7,0);
 if(pending==2){unsigned vector;fist_pic_activate_irq(1);fist_cpu_require(fist_pic_take_irq(0x200,0,&vector)==1 && vector==9);}
 if(pending)fist_pic_activate_irq(7);
 fist_clock_cpu_ss_instruction();fist_clock_add_event(marker,(float)budget/30000.0f,0);fist_clock_cpu_ss_instruction();
 cpu=(FistCpuState){.eax=0x7fffffff,.esp=0x8000,.eip=0x2000,.code_big=big,.stack_big=stackbig,.stack_mask=stackbig?0xffffffff:0xffff,.stack_notmask=stackbig?0:0xffff0000,.flags={.flags=0x3003,.type=FIST_LAZY_UNKNOWN,.prev_type=FIST_LAZY_CMPD,.oldcf=1,.var1=0xdeadbeef,.var2=0x12345678,.res=0x87654321}};
 cpu.segments[1].value=0x2000;cpu.segments[1].base=0x20000;sys.direction=1;
 memset(g_mem,0x40,262144);unsigned a=0x22001;if(flip)g_mem[a++]=0x66;g_mem[a]=op;memcpy(g_mem+0x8000,&flags,4);if(op==0xcf){unsigned width=(big!=flip)?4:2;unsigned frame[]={0x2400,0x2000,flags};for(unsigned i=0;i<3;i++)memcpy(g_mem+0x8000+i*width,&frame[i],width);}
 static const FistPhysicalHandler ram={.kind=FIST_PHYSICAL_RAM,.flags=3};static const FistPhysicalHandler *providers[4096];for(unsigned i=0;i<4096;i++)providers[i]=&ram;
 uint32_t firstmb[FIST_RAM_FIRSTMB];for(unsigned i=0;i<FIST_RAM_FIRSTMB;i++)firstmb[i]=i;
 fist_ram_create(&bus,&cpu,&sys,g_mem,sizeof g_mem,FIST_ARCH_MIXED);fist_ram_restore_provider(&bus,providers,4096,firstmb,1,2);fist_clock_bind_cpu(&cpu);
 FistExec engine={.bus=&bus};snapshot("start");unsigned early=0;
 while(fist_clock_cpu_slice(NULL)){
  fist_clock_charge_cpu_instructions(1);unsigned reason=fist_exec_fetched(&engine);
  if(fist_exec_core_exit(&engine,reason,fist_pic_pending_irqs(),&trap_decoder)){early=1;break;}
 }
 if(early)fist_clock_cpu_core_exit();else fist_clock_cpu_ss_instruction();
 unsigned vector;int irq=fist_pic_take_irq(cpu.flags.flags,trap_decoder,&vector);if(irq>=0)selected=vector;
 snapshot("retire");fwrite(g_mem+0x8000,1,16,stdout);free(bus.tlb);return 0;
}
