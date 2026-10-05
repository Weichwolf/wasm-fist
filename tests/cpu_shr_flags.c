/* Compare the shared CPU producers and boolean flag queries to original source. */
#include "fist_cpu.h"
#include <stdio.h>
static FILE *output;static FistCpuState cpu;
static unsigned flag(unsigned mask) {
 FistCpuFlags *f=&cpu.flags;
 if(f->type==FIST_LAZY_UNKNOWN)return !!(f->flags&mask);
 unsigned bits=fist_cpu_flag_width(f),result=fist_cpu_low(0,f->res,bits),sign=1u<<(bits-1);
 int increment=f->type>=FIST_LAZY_INCB && f->type<=FIST_LAZY_INCD;
 int right=f->type==FIST_LAZY_SHRB || f->type==FIST_LAZY_SHRW || f->type==FIST_LAZY_SHRD;
 fist_cpu_require(increment || right);
 if(mask==4)return !__builtin_parity(result&255);
 if(mask==16)return increment ? (result&15)==0 : !!(f->var2&31);
 if(mask==128)return fist_cpu_sf(&cpu);
 fist_cpu_require(mask==2048);
 return fist_cpu_of(&cpu);
}
static void observe(unsigned value) {
 FistCpuFlags *f=&cpu.flags;
 unsigned q[10]={f->var1,f->var2,f->res,value,!!fist_cpu_cf(&cpu),flag(4),flag(16),!!fist_cpu_zf(&cpu),flag(128),flag(2048)};
 uint64_t wide[4]={f->flags,f->type,f->prev_type,f->oldcf};
 fist_cpu_require(fwrite(q,sizeof q,1,output)==1 && fwrite(wide,sizeof wide,1,output)==1);
}
static void run_case(unsigned bits,unsigned a,unsigned count,unsigned mode) {
 cpu.flags=(FistCpuFlags){.flags=mode?0xffffffff:0x20460202,.type=FIST_LAZY_UNKNOWN,.prev_type=FIST_LAZY_SHLW,.oldcf=1,.var1=0x89abcdef,.var2=0x76543210,.res=0x12345678};
 unsigned value=fist_cpu_shr(&cpu,bits,a,count);observe(value);FistCpuFlags saved=cpu.flags;
 fist_cpu_fill_flags(&cpu);observe(value);cpu.flags=saved;
 value=fist_cpu_incdec(&cpu,bits==8?FIST_LAZY_INCB:bits==16?FIST_LAZY_INCW:FIST_LAZY_INCD,value);
 observe(value);fist_cpu_fill_flags(&cpu);observe(value);
}
#define SHR_REQUIRE fist_cpu_require
#include "cpu_shr_cases.inc"
