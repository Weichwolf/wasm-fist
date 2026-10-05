/* Original CodeRead observes the physical instruction read after its page was linked. */
#include "fist_cpu.h"
#include "fist_dos_cpu.h"
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
static FistDosFindHost host;
static unsigned through_dos,dos_callback_number;
static void host_state(const char *kind) {
 char path[1024];snprintf(path,sizeof path,"%s-%s.host",output,kind);FILE *f=fopen(path,"wb");
 uint32_t words[2]={host.drive,host.next_free};fist_cpu_require(f!=NULL);
 fist_cpu_require(fwrite(words,sizeof words,1,f)==1 && fwrite(host.occupied,1,sizeof host.occupied,f)==sizeof host.occupied && !fclose(f));
}
static void find_state(void *opaque,const char *kind) {
 char label[64];snprintf(label,sizeof label,"find-%s",kind);state(label);host_state(label);
}
static void dos_callback(FistExec *e,unsigned number) {
 fist_cpu_require(number==dos_callback_number);
#ifdef FIST_TASK_GATE_DTA_ONLY
 fist_dos_cpu_set_dta(e->bus);
#else
 if(((cpu.eax>>8)&255)==0x1a)fist_dos_cpu_set_dta(e->bus);
 else {fist_cpu_require(((cpu.eax>>8)&255)==0x4e);find_state(NULL,"before-DOS21");fist_dos_cpu_find_first(e->bus,&host);find_state(NULL,"after-DOS21");}
#endif
}
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
  state(boundary[next].label);if(through_dos)host_state(boundary[next].label);next++;
  if(next==total)longjmp(captured,1);
 }
 #ifdef FIST_TASK_GATE_BEFORE_FETCH
 value=fist_ram_resident_read(bus,segment,offset,width);
 #endif
 return value;
}
int main(int argc,char **argv) {
 fist_cpu_require(argc==5 || argc==7);through_dos=argc==7;
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
 if(through_dos) {
 FILE *hf=fopen(argv[5],"rb");uint32_t h[3];fist_cpu_require(hf && fread(h,sizeof h,1,hf)==1);
 host.drive=h[0];host.next_free=h[1];dos_callback_number=h[2];fist_cpu_require(host.drive<26 && host.next_free<2048);
 fist_cpu_require(fread(host.occupied,1,sizeof host.occupied,hf)==sizeof host.occupied && fread(host.name_prior,1,sizeof host.name_prior,hf)==sizeof host.name_prior && fgetc(hf)==EOF && !fclose(hf));
 host.directory=argv[6];host.observe=find_state;host_state("initial-host");
 }
 if(setjmp(captured)){fixture_destroy(&context);return 0;}
 FistExec engine={.bus=&context.bus,.in=read_byte,.out=TASK_WRITE,.budget=budget,.credit=credit,.charge=charge,.callback=through_dos?dos_callback:NULL};
 for(unsigned count=0;count<(through_dos?50000:200);count++) {
  observe_cpu("fetch");
  unsigned checks=fist_exec_fetched(&engine),trap=0;
  int service=fist_exec_core_exit(&engine,checks,fist_pic_pending_irqs(),&trap);
  if(!through_dos)fist_cpu_require(!service);
  if(service) {
   fist_cpu_require(!trap);fist_clock_cpu_core_exit();unsigned vector;
   (void)fist_pic_dispatch_irq(cpu.flags.flags,trap,&vector,deliver,NULL);
  }
  fist_clock_charge_cpu_instructions(1);
 }
 abort();
}
