/* Execute the actual original instruction and lazy-flag owners. */
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/flags.cpp"
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/instructions.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
CPU_Regs cpu_regs;
#define ProbeLoad(x) (x)
#define ProbeSave(x,y) (x)=(y)
int main(void) {
 unsigned raw,type,previous,oldcf,var1,var2,result,a,b;char operation[3];int fields;
 while ((fields=scanf(" %2s %x %x %x %x %x %x %x %x %x",operation,&raw,&type,&previous,&oldcf,&var1,&var2,&result,&a,&b))==10) {
  cpu_regs.flags=raw;lflags.type=type;lflags.prev_type=previous;lflags.oldcf=oldcf;
  lflags.var1.dword[0]=var1;lflags.var2.dword[0]=var2;lflags.res.dword[0]=result;
  if (!strcmp(operation,"XW")) { XORW(a,b,ProbeLoad,ProbeSave); }
  else if (!strcmp(operation,"OW")) { ORW(a,b,ProbeLoad,ProbeSave); }
  else if (!strcmp(operation,"HB")) { ANDB(a,b,ProbeLoad,ProbeSave); }
  else if (!strcmp(operation,"HW")) { ANDW(a,b,ProbeLoad,ProbeSave); }
  else if (!strcmp(operation,"SB")) { SUBB(a,b,ProbeLoad,ProbeSave); }
  else if (!strcmp(operation,"IB")) { INCB(a,ProbeLoad,ProbeSave); }
  else if (!strcmp(operation,"IW")) { INCW(a,ProbeLoad,ProbeSave); }
  else if (!strcmp(operation,"DB")) { DECB(a,ProbeLoad,ProbeSave); }
  else if (!strcmp(operation,"DW")) { DECW(a,ProbeLoad,ProbeSave); }
  else if (!strcmp(operation,"TW")) { TESTW(a,b,ProbeLoad,ProbeSave); }
  else if (!strcmp(operation,"LB") || !strcmp(operation,"LW")) {
   /* The normal-core group decoder masks the count before invoking these macros. */
   b&=31;
   switch(operation[1]) {
    case 'B': { SHLB(a,b,ProbeLoad,ProbeSave);break; }
    case 'W': { SHLW(a,b,ProbeLoad,ProbeSave);break; }
   }
  }
  else { assert(!operation[1]);switch(operation[0]) {
   case 'N':break;
   case 'X': { XORD(a,b,ProbeLoad,ProbeSave);break; }
   case 'O': { ORD(a,b,ProbeLoad,ProbeSave);break; }
   case 'C': { CMPD(a,b,ProbeLoad,ProbeSave);break; }
   case 'A': { ADDD(a,b,ProbeLoad,ProbeSave);break; }
   case 'S': { SUBD(a,b,ProbeLoad,ProbeSave);break; }
   case 'I': { INCD(a,ProbeLoad,ProbeSave);break; }
   case 'D': { DECD(a,ProbeLoad,ProbeSave);break; }
   default:assert(false);
  }}
  printf("%08x %x %x %x %08x %08x %08x %08x %u %u\n",cpu_regs.flags,(unsigned)lflags.type,(unsigned)lflags.prev_type,(unsigned)lflags.oldcf,lflags.var1.dword[0],lflags.var2.dword[0],lflags.res.dword[0],a,!!get_CF(),!!get_ZF());
 }
 assert(fields==EOF && !ferror(stdin));
}
