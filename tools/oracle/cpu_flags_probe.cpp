/* Exercise the actual original lazy-flag owner; no CPU/device scheduler is copied. */
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/flags.cpp"
#include <assert.h>
#include <stdio.h>
CPU_Regs cpu_regs;
int main(void)
{
    unsigned flags,type,previous,oldcf,a,b,result; int fields;
    while ((fields=scanf("%x %x %x %x %x %x %x",&flags,&type,&previous,&oldcf,&a,&b,&result))==7) {
        assert(type==t_UNKNOWN || type==t_ADDw || type==t_XORb || type==t_XORd ||
               type==t_CMPb || type==t_CMPw || type==t_TESTb || type==t_ADDd ||
               type==t_ORb || type==t_ORd || type==t_SUBd || type==t_CMPd ||
               type==t_INCd || type==t_DECd);
        cpu_regs.flags=flags; lflags.type=type; lflags.prev_type=previous; lflags.oldcf=oldcf;
        lflags.var1.dword[0]=a; lflags.var2.dword[0]=b; lflags.res.dword[0]=result;
        unsigned cf=!!get_CF(),zf=!!get_ZF();
        FillFlags();
        printf("%u %u %08x %x %x %x %08x %08x %08x\n",cf,zf,cpu_regs.flags,
               (unsigned)lflags.type,(unsigned)lflags.prev_type,(unsigned)lflags.oldcf,
               lflags.var1.dword[0],lflags.var2.dword[0],lflags.res.dword[0]);
    }
    assert(fields==EOF && !ferror(stdin));
}
