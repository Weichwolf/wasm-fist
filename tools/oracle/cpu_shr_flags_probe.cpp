/* Original shift/INC/GetFlags/FillFlags bodies retain complete dirty lazy state. */
#include <stdlib.h>
#include <stdint.h>
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/flags.cpp"
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/instructions.h"
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
static void run_case(unsigned bits,unsigned a,unsigned count,unsigned mode) {
 reg_flags=mode?0xffffffff:0x20460202;lflags.type=t_UNKNOWN;lflags.prev_type=t_SHLw;lflags.oldcf=1;
 lflags.var1.dword[0]=0x89abcdef;lflags.var2.dword[0]=0x76543210;lflags.res.dword[0]=0x12345678;
 unsigned value=a;
 do {switch(bits) {case 8:SHRB(value,count,ProbeLoad,ProbeSave);break;case 16:SHRW(value,count,ProbeLoad,ProbeSave);break;case 32:SHRD(value,count,ProbeLoad,ProbeSave);break;default:abort();}}while(0);
 observe(value);LazyFlags saved=lflags;Bitu flags=reg_flags;FillFlags();observe(value);
 lflags=saved;reg_flags=flags;
 do {switch(bits) {case 8:INCB(value,ProbeLoad,ProbeSave);break;case 16:INCW(value,ProbeLoad,ProbeSave);break;case 32:INCD(value,ProbeLoad,ProbeSave);break;default:abort();}}while(0);
 observe(value);FillFlags();observe(value);
}
#define SHR_REQUIRE assert
#include "../../tests/cpu_shr_cases.inc"
