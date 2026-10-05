/* Original CodeRead observes the physical instruction read after its page was linked. */
#include "fist_cpu.h"
#include "fist_interrupt.h"
#include <setjmp.h>
static uint32_t task_code_read(FistCpuRam *,unsigned,uint32_t,unsigned);
#define fist_ram_resident_read task_code_read
#define main core_main
#include "cpu_core_exit.c"
#undef main
#undef fist_ram_resident_read
extern void fist_clock_credit_cpu_fetch(void);
extern int in(int);extern void out(int,int);
static uint32_t read_byte(void *p,unsigned port){return in(port);}
static void write_byte(void *p,unsigned port,unsigned value){out(port,value);}
#ifdef FIST_TASK_GATE_UNBOUND_IO
#define TASK_WRITE NULL
#else
#define TASK_WRITE write_byte
#endif
static unsigned budget(void *p){return fist_clock_cpu_slice(NULL);}
static void credit(void *p){fist_clock_credit_cpu_fetch();}
static void charge(void *p,unsigned n){fist_clock_charge_cpu_instructions(n);}
static struct {unsigned cs,ip;char label[48];} boundary[32];
static unsigned total,next;
static jmp_buf captured;
static uint32_t task_code_read(FistCpuRam *bus,unsigned segment,uint32_t offset,unsigned width) {
 uint32_t value;
#ifndef FIST_TASK_GATE_BEFORE_FETCH
 value=fist_ram_resident_read(bus,segment,offset,width);
#endif
 if(segment==1 && offset==cpu.eip && width==1 && next<total &&
    cpu.segments[1].value==boundary[next].cs && cpu.eip==boundary[next].ip) {
  state(boundary[next++].label);
  if(next==total)longjmp(captured,1);
 }
 #ifdef FIST_TASK_GATE_BEFORE_FETCH
 value=fist_ram_resident_read(bus,segment,offset,width);
 #endif
 return value;
}
int main(int argc,char **argv) {
 fist_cpu_require(argc==5);
 FILE *f=fopen(argv[4],"r");fist_cpu_require(f!=NULL);
 for(;;) {
  fist_cpu_require(total<sizeof boundary/sizeof *boundary);
  int fields=fscanf(f,"%x %x %47s",&boundary[total].cs,&boundary[total].ip,boundary[total].label);
  if(fields==EOF)break;
  fist_cpu_require(fields==3);total++;
 }
 fist_cpu_require(total>0 && !ferror(f) && !fclose(f));
 setenv("FIST_SB","1",1);
 load_core_inputs(4,argv);
 if(setjmp(captured)){fixture_destroy(&context);return 0;}
 FistExec engine={.bus=&context.bus,.in=read_byte,.out=TASK_WRITE,.budget=budget,.credit=credit,.charge=charge};
 for(unsigned count=0;count<200;count++) {
  observe_cpu("fetch");
  unsigned checks=fist_exec_fetched(&engine),trap=0;
  fist_cpu_require(!fist_exec_core_exit(&engine,checks,fist_pic_pending_irqs(),&trap));
  fist_clock_charge_cpu_instructions(1);
 }
 abort();
}
