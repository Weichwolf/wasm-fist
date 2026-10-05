/* Exercise the shared CPU owners, including carry-preserving INC after each operation. */
#include "fist_cpu.h"
#include "sb_clock_fixture.h"
#include "fist_exec.h"
#include <stdio.h>
#include <string.h>
static FILE *output;static FistCpuState cpu;
static void observe(unsigned value) {
 FistCpuFlags saved=cpu.flags;
 fist_cpu_fill_flags(&cpu);unsigned pf=!!(cpu.flags.flags&4),af=!!(cpu.flags.flags&16);cpu.flags=saved;
 unsigned q[10]={cpu.flags.var1,cpu.flags.var2,cpu.flags.res,value,!!fist_cpu_cf(&cpu),pf,af,!!fist_cpu_zf(&cpu),!!fist_cpu_sf(&cpu),!!fist_cpu_of(&cpu)};
 uint64_t wide[4]={cpu.flags.flags,cpu.flags.type,cpu.flags.prev_type,cpu.flags.oldcf};
 fist_cpu_require(fwrite(q,sizeof q,1,output)==1 && fwrite(wide,sizeof wide,1,output)==1);
}
static void start(unsigned mode,unsigned type) {
 cpu.flags=(FistCpuFlags){.flags=mode?0xffffffff:0x20460202,.type=type,.prev_type=FIST_LAZY_SHLW,.oldcf=1,.var1=0x89abcdef,.var2=0x76543210,.res=0x12345678};
}
static void finish(unsigned value,unsigned bits) {
 observe(value);FistCpuFlags saved=cpu.flags;
 fist_cpu_fill_flags(&cpu);observe(value);cpu.flags=saved;
 value=fist_cpu_incdec(&cpu,bits==8?FIST_LAZY_INCB:bits==16?FIST_LAZY_INCW:FIST_LAZY_INCD,value);
 observe(value);fist_cpu_fill_flags(&cpu);observe(value);
}
static void run_sub(unsigned a,unsigned b,unsigned mode) {
 start(mode,FIST_LAZY_CMPW);
 FistCpuRam bus={.cpu=&cpu};FistExec engine={.bus=&bus};finish(fist_exec_alu(&engine,5,2,a,b),16);
}
static void run_shl(unsigned bits,unsigned a,unsigned count,unsigned mode) {
 start(mode,FIST_LAZY_UNKNOWN);finish(fist_cpu_shl(&cpu,bits,a,count),bits);
}
#define CHECK fist_cpu_require
#include "cpu_task_gate_flags_cases.inc"
