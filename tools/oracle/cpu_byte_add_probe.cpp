/* Verbatim original ADDB/INCB and lazy-flag owners, including all dirty upper bits. */
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/flags.cpp"
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/instructions.h"
#include <assert.h>
#include <stdio.h>
CPU_Regs cpu_regs;
static FILE *output;
#define ProbeLoad(x) (x)
#define ProbeSave(x,v) (x)=(v)
static void observe(unsigned value) {
 unsigned q[10]={reg_flags,(unsigned)lflags.var1.dword[0],(unsigned)lflags.var2.dword[0],(unsigned)lflags.res.dword[0],(unsigned)lflags.type,(unsigned)lflags.prev_type,(unsigned)lflags.oldcf,value,!!get_CF(),!!get_ZF()};
 assert(fwrite(q,sizeof q,1,output)==1);
}
int main(int argc,char **argv) {
 assert(argc==2);output=fopen(argv[1],"wb");assert(output);
 for(unsigned a=0;a<256;a++)for(unsigned b=0;b<256;b++)for(unsigned mode=0;mode<2;mode++) {
  reg_flags=mode?0xffffffff:0x20460202;lflags.type=t_CMPw;lflags.prev_type=t_SHLw;lflags.oldcf=1;
  lflags.var1.dword[0]=0x89abcdef;lflags.var2.dword[0]=0x76543210;lflags.res.dword[0]=0x12345678;
  Bit8u value=a;
#ifdef FIST_BYTE_AND
  ANDB(value,b,ProbeLoad,ProbeSave);
#else
  ADDB(value,b,ProbeLoad,ProbeSave);
#endif
  observe(value);
  LazyFlags saved=lflags;unsigned flags=reg_flags;FillFlags();observe(value);
  lflags=saved;reg_flags=flags;INCB(value,ProbeLoad,ProbeSave);observe(value);FillFlags();observe(value);
 }
 assert(!ferror(output) && !fclose(output));return 0;
}
