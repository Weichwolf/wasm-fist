/* Exhaustive byte ADD and INC carry retention through the shared actual owners. */
#include "sb_clock_fixture.h"
#include "fist_exec.h"
#include <stdio.h>
#ifndef FIST_BYTE_ALU_OPERATION
#define FIST_BYTE_ALU_OPERATION 0
#endif
static FistCpuState cpu;
static FILE *output;
static void observe(unsigned value) {
 uint32_t q[10]={cpu.flags.flags,cpu.flags.var1,cpu.flags.var2,cpu.flags.res,cpu.flags.type,cpu.flags.prev_type,cpu.flags.oldcf,value,fist_cpu_cf(&cpu),fist_cpu_zf(&cpu)};
 fist_cpu_require(fwrite(q,sizeof q,1,output)==1);
}
int main(int argc,char **argv) {
 fist_cpu_require(argc==2);output=fopen(argv[1],"wb");fist_cpu_require(output!=NULL);
 FistCpuRam bus={.cpu=&cpu};FistExec engine={.bus=&bus};
 for(unsigned a=0;a<256;a++)for(unsigned b=0;b<256;b++)for(unsigned mode=0;mode<2;mode++) {
  cpu.flags=(FistCpuFlags){mode?0xffffffff:0x20460202,0x89abcdef,0x76543210,0x12345678,FIST_LAZY_CMPW,FIST_LAZY_SHLW,1};
  unsigned value=fist_exec_alu(&engine,FIST_BYTE_ALU_OPERATION,1,a,b);observe(value);
  FistCpuFlags saved=cpu.flags;fist_cpu_fill_flags(&cpu);observe(value);
  cpu.flags=saved;value=fist_cpu_incdec(&cpu,FIST_LAZY_INCB,value);observe(value);fist_cpu_fill_flags(&cpu);observe(value);
 }
 fist_cpu_require(!ferror(output) && !fclose(output));return 0;
}
