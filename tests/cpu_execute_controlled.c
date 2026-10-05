/* Explicit controlled CPU/budget/physical inputs for original-core composition
 * cases. Continuous source tests separately exercise the real shared clock. */
#ifdef FIST_EXECUTE_FETCH_TRACE
#include "fist_interrupt.h"
static unsigned fetch_count,fetch_reads[13];
static uint32_t observed_read(FistCpuRam *bus,unsigned segment,uint32_t offset,unsigned width) {
 if(segment==1) {
  fist_cpu_require(fetch_count<4);unsigned *r=fetch_reads+1+3*fetch_count++;
  r[0]=segment;r[1]=offset;r[2]=width;fetch_reads[0]=fetch_count;
 }
 return fist_ram_resident_read(bus,segment,offset,width);
}
#define fist_ram_resident_read observed_read
#endif
#include "fist_exec.h"
#ifdef FIST_EXECUTE_FETCH_TRACE
#undef fist_ram_resident_read
#endif
#include <stdio.h>
static unsigned remaining;
static unsigned budget(void *p) {return remaining;}
static void credit(void *p) {remaining++;}
static void charge(void *p,unsigned count) {fist_cpu_require(count<=remaining);remaining-=count;}
int main(void) {
 uint32_t q[16];size_t count;
#ifdef FIST_EXECUTE_SEGMENT_INPUT
 static uint8_t memory[0x200000];
#else
 static uint8_t memory[0x40000];
#endif
 static FistCpuState cpu;static FistCpuSystem sys;static FistCpuRam bus;
 FistPhysicalHandler ram={.kind=FIST_PHYSICAL_RAM,.flags=3};
 const FistPhysicalHandler *providers[sizeof memory/4096];
 uint32_t firstmb[FIST_RAM_FIRSTMB];
 for(unsigned i=0;i<sizeof providers/sizeof *providers;i++)providers[i]=&ram;
 for(unsigned i=0;i<FIST_RAM_FIRSTMB;i++)firstmb[i]=i;
 while((count=fread(q,sizeof *q,16,stdin))) {
  fist_cpu_require(count==16 && q[15]>0 && q[15]<=32 && q[2]+q[15]<=sizeof memory);
  for(unsigned i=0;i<sizeof memory;i++)memory[i]=(i*37+(i>>8)+11)&255;
#ifdef FIST_EXECUTE_SEGMENT_INPUT
  uint32_t segments[12];fist_cpu_require(fread(segments,sizeof segments,1,stdin)==1);
  for(unsigned i=0;i<6;i++)fist_cpu_require(segments[2*i]<=0xffff);
  fist_cpu_require((uint64_t)segments[3]+q[2]+q[15]<=sizeof memory);
  fist_cpu_require(fread(memory+segments[3]+q[2],1,q[15],stdin)==q[15]);
#else
  fist_cpu_require(fread(memory+q[2],1,q[15],stdin)==q[15]);
#endif
  memset(&cpu,0,sizeof cpu);memset(&sys,0,sizeof sys);
  memcpy(&cpu,q+4,8*sizeof *q);cpu.eip=q[2];cpu.flags.flags=q[3];
#ifdef FIST_EXECUTE_SEGMENT_INPUT
  memcpy(cpu.segments,segments,sizeof segments);
#endif
  cpu.flags=(FistCpuFlags){.flags=q[3],.type=FIST_LAZY_UNKNOWN,.prev_type=FIST_LAZY_CMPD,.oldcf=1,
   .var1=0x12345678,.var2=0x87654321,.res=0xabcdef01};
#ifdef FIST_EXECUTE_LAZY_INPUT
  cpu.flags.type=q[14];
#endif
  cpu.code_big=q[0];cpu.stack_big=q[1];cpu.stack_mask=q[1]?UINT32_MAX:0xffff;
  cpu.stack_notmask=~cpu.stack_mask;sys.direction=(int32_t)q[12];remaining=q[13]-1;
  fist_ram_create(&bus,&cpu,&sys,memory,sizeof memory,FIST_ARCH_MIXED);
  fist_ram_restore_provider(&bus,providers,sizeof providers/sizeof *providers,firstmb,1,2);
  FistExec engine={.bus=&bus,.budget=budget,.credit=credit,.charge=charge};
#ifdef FIST_EXECUTE_FETCH_TRACE
  fetch_count=0;memset(fetch_reads,0,sizeof fetch_reads);
#endif
  fist_exec_fetched(&engine);
  uint32_t out[19];memcpy(out,&cpu,9*sizeof *out);memcpy(out+9,&cpu.flags,sizeof cpu.flags);
  out[16]=remaining;out[17]=(uint32_t)sys.direction;out[18]=cpu.code_big;
  fist_cpu_require(fwrite(out,sizeof out,1,stdout)==1);
#ifdef FIST_EXECUTE_SEGMENT_INPUT
  uint64_t stack[3]={cpu.stack_big,cpu.stack_mask,cpu.stack_notmask};
  fist_cpu_require(fwrite(cpu.segments,sizeof cpu.segments,1,stdout)==1 && fwrite(stack,sizeof stack,1,stdout)==1);
#endif
  fist_cpu_require(fwrite(memory,sizeof memory,1,stdout)==1);
#ifdef FIST_EXECUTE_FETCH_TRACE
  fist_cpu_require(fwrite(fetch_reads,sizeof fetch_reads,1,stdout)==1);
#endif
  free(bus.tlb);
 }
 fist_cpu_require(!ferror(stdin));return 0;
}
