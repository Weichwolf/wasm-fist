/* Actual original control bodies, flag macros and PIC. Controlled real-mode
 * vector-selection endpoint; the real application capture supplies IRQ frames. */
#include "original_pic_context.h"
#include "original_flags.cpp"
#include "instructions.h"
CPUBlock cpu;
Bitu CPU_flag_id_toggle=0;
static unsigned char memory[262144];
Bit16u mem_readw(PhysPt a){assert(a+2<=sizeof memory);Bit16u v;memcpy(&v,memory+a,2);return v;}
Bit32u mem_readd(PhysPt a){assert(a+4<=sizeof memory);Bit32u v;memcpy(&v,memory+a,4);return v;}
static struct {PhysPt cseip;} core;
#define SegBase(s) SegPhys(s)
#include "original_ip.h"
#include "original_cpu_control.h"
#define CPU_TRAP_CHECK 1
#define RUNEXCEPTION() {abort();}
#define ProbeLoad(x) (x)
#define ProbeSave(x,v) (x)=(v)
static unsigned selected=0xffffffffu,event_count;
static void marker(Bitu){event_count++;}
static void observe_irq(Bitu vector){assert(selected==0xffffffffu);selected=vector;}
static unsigned fetch(void){assert(core.cseip<sizeof memory);return memory[core.cseip++];}
static void run_core(void){
 while(CPU_Cycles-->0){
  LOADIP;unsigned op=fetch(),width=cpu.code.big?4:2;
  if(op==0x66){width=cpu.code.big?2:4;op=fetch();}
  if((op==0x9d || op==0xcf) && width==4)op+=0x100;
  switch(op){
  case 0x40:if(width==4){INCD(reg_eax,ProbeLoad,ProbeSave);}else{INCW(reg_ax,ProbeLoad,ProbeSave);}break;
#include "original_cases.h"
  default:abort();
  }SAVEIP;
 }FillFlags();return;
 decode_end:SAVEIP;FillFlags();return;
}
static void snapshot(const char *kind){
 printf("%s %llu %u %d %d %u %u %u %u %u %d",kind,(unsigned long long)(PIC_Ticks*30000u+PIC_TickIndexND()),(unsigned)PIC_Ticks,CPU_Cycles,CPU_CycleLeft,(unsigned)PIC_IRQCheck,(unsigned)PIC_IRQActive,selected,event_count,cpudecoder==CPU_Core_Normal_Trap_Run,cpu.direction);
 for(unsigned i=0;i<8;i++)printf(" %08x",reg_32(i));
 printf(" %08x %08x %08x %08x %08x %08x %08x %08x %u %u\n",reg_eip,reg_flags,lflags.var1.dword[0],lflags.var2.dword[0],lflags.res.dword[0],(unsigned)lflags.type,(unsigned)lflags.prev_type,(unsigned)lflags.oldcf,cpu.code.big,cpu.stack.big);
}
int main(int argc,char **argv){
 assert(argc==8);unsigned op=strtoul(argv[1],0,0),big=atoi(argv[2]),flip=atoi(argv[3]),pending=atoi(argv[4]),flags=strtoul(argv[5],0,0),budget=atoi(argv[6]),stackbig=atoi(argv[7]);
 PIC_8259A controller(NULL);initialize_queue();PIC_SetIRQMask(7,false);
 if(pending==2){reg_flags=0x200;PIC_ActivateIRQ(1);PIC_runIRQs();assert(selected==9);selected=0xffffffffu;}
 if(pending)PIC_ActivateIRQ(7);
 cpu.pmode=false;cpu.cpl=0;cpu.code.big=big;cpu.stack.big=stackbig;cpu.stack.mask=stackbig?0xffffffff:0xffff;cpu.stack.notmask=~cpu.stack.mask;cpu.direction=1;
 Segs.val[cs]=0x2000;Segs.phys[cs]=0x20000;reg_eip=0x2000;reg_eax=0x7fffffff;reg_esp=0x8000;
 reg_flags=0x3003;lflags.type=t_UNKNOWN;lflags.prev_type=t_CMPd;lflags.oldcf=1;lflags.var1.dword[0]=0xdeadbeef;lflags.var2.dword[0]=0x12345678;lflags.res.dword[0]=0x87654321;
 memset(memory,0x40,sizeof memory);unsigned a=0x22001;if(flip)memory[a++]=0x66;memory[a]=op;memcpy(memory+0x8000,&flags,4);if(op==0xcf){unsigned width=(big!=flip)?4:2;unsigned frame[]={0x2400,0x2000,flags};for(unsigned i=0;i<3;i++)memcpy(memory+0x8000+i*width,&frame[i],width);}
 PIC_AddEvent(marker,(float)budget/30000.0f);assert(PIC_RunQueue());snapshot("start");
 run_core();assert(PIC_RunQueue());snapshot("retire");
 fwrite(memory+0x8000,1,16,stdout);return 0;
}
