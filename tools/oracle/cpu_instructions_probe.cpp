/* Execute the actual original instruction and lazy-flag owners. */
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/flags.cpp"
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/instructions.h"
#include <assert.h>
#include <stdio.h>
CPU_Regs cpu_regs;
#define ProbeLoad(x) (x)
#define ProbeSave(x,y) (x)=(y)
int main(void) {
 unsigned raw,type,previous,oldcf,var1,var2,result,a,b;char operation;int fields;
 while ((fields=scanf(" %c %x %x %x %x %x %x %x %x %x",&operation,&raw,&type,&previous,&oldcf,&var1,&var2,&result,&a,&b))==10) {
  cpu_regs.flags=raw;lflags.type=type;lflags.prev_type=previous;lflags.oldcf=oldcf;
  lflags.var1.dword[0]=var1;lflags.var2.dword[0]=var2;lflags.res.dword[0]=result;
  switch(operation) {
   case 'N':break;
   case 'X': { XORD(a,b,ProbeLoad,ProbeSave);break; }
   case 'O': { ORD(a,b,ProbeLoad,ProbeSave);break; }
   case 'C': { CMPD(a,b,ProbeLoad,ProbeSave);break; }
   case 'A': { ADDD(a,b,ProbeLoad,ProbeSave);break; }
   case 'S': { SUBD(a,b,ProbeLoad,ProbeSave);break; }
   case 'I': { INCD(a,ProbeLoad,ProbeSave);break; }
   case 'D': { DECD(a,ProbeLoad,ProbeSave);break; }
   default:assert(false);
  }
  printf("%08x %x %x %x %08x %08x %08x %08x %u %u\n",cpu_regs.flags,(unsigned)lflags.type,(unsigned)lflags.prev_type,(unsigned)lflags.oldcf,lflags.var1.dword[0],lflags.var2.dword[0],lflags.res.dword[0],a,!!get_CF(),!!get_ZF());
 }
 assert(fields==EOF && !ferror(stdin));
}
