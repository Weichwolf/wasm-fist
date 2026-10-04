/* Explicit controlled CPU/budget/physical inputs for original-core composition
 * cases. Continuous source tests separately exercise the real shared clock. */
#include "fist_exec.h"
#include <stdio.h>
static unsigned remaining;
static unsigned budget(void *p) {return remaining;}
static void credit(void *p) {remaining++;}
static void charge(void *p,unsigned count) {fist_cpu_require(count<=remaining);remaining-=count;}
int main(void) {
 uint32_t q[16];size_t count;static uint8_t memory[0x40000];
 static FistCpuState cpu;static FistCpuSystem sys;static FistCpuRam bus;
 FistPhysicalHandler ram={.kind=FIST_PHYSICAL_RAM,.flags=3};
 const FistPhysicalHandler *providers[sizeof memory/4096];
 uint32_t firstmb[FIST_RAM_FIRSTMB];
 for(unsigned i=0;i<sizeof providers/sizeof *providers;i++)providers[i]=&ram;
 for(unsigned i=0;i<FIST_RAM_FIRSTMB;i++)firstmb[i]=i;
 while((count=fread(q,sizeof *q,16,stdin))) {
  fist_cpu_require(count==16 && q[15]>0 && q[15]<=32 && q[2]+q[15]<=sizeof memory);
  for(unsigned i=0;i<sizeof memory;i++)memory[i]=(i*37+(i>>8)+11)&255;
  fist_cpu_require(fread(memory+q[2],1,q[15],stdin)==q[15]);
  memset(&cpu,0,sizeof cpu);memset(&sys,0,sizeof sys);
  memcpy(&cpu,q+4,8*sizeof *q);cpu.eip=q[2];cpu.flags.flags=q[3];
  cpu.flags=(FistCpuFlags){.flags=q[3],.type=FIST_LAZY_UNKNOWN,.prev_type=FIST_LAZY_CMPD,.oldcf=1,
   .var1=0x12345678,.var2=0x87654321,.res=0xabcdef01};
  cpu.code_big=q[0];cpu.stack_big=q[1];cpu.stack_mask=q[1]?UINT32_MAX:0xffff;
  cpu.stack_notmask=~cpu.stack_mask;sys.direction=(int32_t)q[12];remaining=q[13]-1;
  fist_ram_create(&bus,&cpu,&sys,memory,sizeof memory,FIST_ARCH_MIXED);
  fist_ram_restore_provider(&bus,providers,sizeof providers/sizeof *providers,firstmb,1,2);
  FistExec engine={.bus=&bus,.budget=budget,.credit=credit,.charge=charge};
  fist_exec_fetched(&engine);
  uint32_t out[19];memcpy(out,&cpu,9*sizeof *out);memcpy(out+9,&cpu.flags,sizeof cpu.flags);
  out[16]=remaining;out[17]=(uint32_t)sys.direction;out[18]=cpu.code_big;
  fist_cpu_require(fwrite(out,sizeof out,1,stdout)==1 && fwrite(memory,sizeof memory,1,stdout)==1);
  free(bus.tlb);
 }
 fist_cpu_require(!ferror(stdin));return 0;
}
