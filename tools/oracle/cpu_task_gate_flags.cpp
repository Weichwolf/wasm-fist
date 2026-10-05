/* Use the original instruction macros, flag queries and materialization unchanged. */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "flags.cpp"
#include "instructions.h"
#include <assert.h>
#include <stdio.h>
CPU_Regs cpu_regs;
static FILE *output;
#define ProbeLoad(x) (x)
#define ProbeSave(x,v) (x)=(v)
static void observe(unsigned value) {
 unsigned q[10]={lflags.var1.dword[0],lflags.var2.dword[0],lflags.res.dword[0],value,!!get_CF(),!!get_PF(),!!get_AF(),!!get_ZF(),!!get_SF(),!!get_OF()};
 uint64_t wide[4]={reg_flags,lflags.type,lflags.prev_type,lflags.oldcf};
 assert(fwrite(q,sizeof q,1,output)==1 && fwrite(wide,sizeof wide,1,output)==1);
}
static void start(unsigned mode,Bitu type) {
 reg_flags=mode?0xffffffff:0x20460202;lflags.type=type;lflags.prev_type=t_SHLw;lflags.oldcf=1;
 lflags.var1.dword[0]=0x89abcdef;lflags.var2.dword[0]=0x76543210;lflags.res.dword[0]=0x12345678;
}
static void finish(unsigned value,unsigned bits) {
 observe(value);LazyFlags saved=lflags;Bitu flags=reg_flags;FillFlags();observe(value);
 lflags=saved;reg_flags=flags;
 do {switch(bits) {case 8:INCB(value,ProbeLoad,ProbeSave);break;case 16:INCW(value,ProbeLoad,ProbeSave);break;case 32:INCD(value,ProbeLoad,ProbeSave);break;default:abort();}}while(0);
 observe(value);FillFlags();observe(value);
}
static void run_sub(unsigned a,unsigned b,unsigned mode) {
 start(mode,t_CMPw);Bit16u value=a;
 do {SUBW(value,b,ProbeLoad,ProbeSave);}while(0);finish(value,16);
}
static void run_shl(unsigned bits,unsigned a,unsigned count,unsigned mode) {
 start(mode,t_UNKNOWN);unsigned value=a;
 do {switch(bits) {case 8:SHLB(value,count,ProbeLoad,ProbeSave);break;case 16:SHLW(value,count,ProbeLoad,ProbeSave);break;case 32:SHLD(value,count,ProbeLoad,ProbeSave);break;default:abort();}}while(0);finish(value,bits);
}
#define CHECK assert
#include "cpu_task_gate_flags_cases.inc"
