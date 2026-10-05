/* Controlled original normal-core execution. Branch bodies/support are read
 * verbatim from the local source; MOVS calls its original DoString owner. */
#include <stdint.h>
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/flags.cpp"
CPU_Regs cpu_regs;
Segments Segs;
Bit32s CPU_Cycles;
#include "rep_probe.cpp"
#define mem_writew(a,v) save(a,v,2)
#define mem_writed(a,v) save(a,v,4)
#ifdef FIST_EXECUTE_FETCH_TRACE
static unsigned fetch_count,fetch_reads[13];
static void observe_fetch(unsigned segment,unsigned offset,unsigned width) {
 assert(fetch_count<4);unsigned *r=fetch_reads+1+3*fetch_count++;
 r[0]=segment;r[1]=offset-SegPhys(cs);r[2]=width;fetch_reads[0]=fetch_count;
}
static Bit8u Fetchb(void) {observe_fetch(1,core.cseip,1);return load(core.cseip++,1);}
#else
static Bit8u Fetchb(void) {return load(core.cseip++,1);}
#endif
static Bit16u Fetchw(void) { Bit16u v=load(core.cseip,2);core.cseip+=2;return v; }
static Bit32u Fetchd(void) { Bit32u v=load(core.cseip,4);core.cseip+=4;return v; }
#undef LOADIP
#include "original_branch_support.h"
#define Push_16 CPU_Push16
#define Push_32 CPU_Push32
#define TEST_PREFIX_ADDR (core.prefixes&PREFIX_ADDR)
int main(void) {
 unsigned q[16];size_t count;
 while((count=fread(q,sizeof *q,16,stdin))) {
  assert(count==16 && q[15]>0 && q[15]<=32 && q[2]+q[15]<=sizeof memory);
  for(unsigned i=0;i<sizeof memory;i++)memory[i]=(i*37+(i>>8)+11)&255;
#ifdef FIST_EXECUTE_SEGMENT_INPUT
  unsigned segments[12];assert(fread(segments,sizeof segments,1,stdin)==1);
  for(unsigned i=0;i<6;i++)assert(segments[2*i]<=0xffff);
  assert((uint64_t)segments[3]+q[2]+q[15]<=sizeof memory);
  assert(fread(memory+segments[3]+q[2],1,q[15],stdin)==q[15]);
#else
  assert(fread(memory+q[2],1,q[15],stdin)==q[15]);
#endif
  memset(&cpu_regs,0,sizeof cpu_regs);memset(&Segs,0,sizeof Segs);
#ifdef FIST_EXECUTE_SEGMENT_INPUT
  for(unsigned i=0;i<6;i++) {Segs.val[i]=segments[2*i];Segs.phys[i]=segments[2*i+1];}
#endif
  for(unsigned i=0;i<8;i++)reg_32(i)=q[4+i];
  reg_eip=q[2];reg_flags=q[3];
#ifdef FIST_EXECUTE_LAZY_INPUT
  lflags.type=(decltype(lflags.type))q[14];
#else
  lflags.type=t_UNKNOWN;
#endif
  lflags.prev_type=t_CMPd;lflags.oldcf=1;lflags.var1.dword[0]=0x12345678;
  lflags.var2.dword[0]=0x87654321;lflags.res.dword[0]=0xabcdef01;
  cpu.code.big=q[0];cpu.stack.big=q[1];cpu.stack.mask=q[1]?0xffffffff:0xffff;
  /* cpu.cpp's real stack constructor uses Bitu values 0/0xffff0000,
   * rather than the 64-bit complement of a 32-bit mask on this host. */
  cpu.stack.notmask=q[1]?0:0xffff0000;cpu.direction=(int)q[12];
  core.base_ds=0;core.prefixes=q[0]?PREFIX_ADDR:0;
#ifdef FIST_EXECUTE_FETCH_TRACE
  fetch_count=0;memset(fetch_reads,0,sizeof fetch_reads);
#endif
  CPU_Cycles=q[13]-1;LOADIP;
  unsigned width=q[0]?4:2,op;
  for(;;) {
   op=Fetchb();
   if(op==0x66)width=q[0]?2:4;
   else if(op==0x67)core.prefixes=(core.prefixes&~PREFIX_ADDR)|(q[0]?0:PREFIX_ADDR);
   else if(op==0xf2||op==0xf3)core.prefixes|=PREFIX_REP;
   else break;
  }
  unsigned once=1;
  if(op==0x0f)op=0x100|Fetchb();
  while(once--) {
   if(op==0xa4||op==0xa5) {DoString(op==0xa4?R_MOVSB:width==2?R_MOVSW:R_MOVSD);SAVEIP;continue;}
   if(width==2) {switch(op) {
#include "original_cases_2.h"
    default:abort();
   }}else {switch(op) {
#include "original_cases_4.h"
    default:abort();
   }}
   SAVEIP;
  }
  unsigned out[19];for(unsigned i=0;i<8;i++)out[i]=reg_32(i);
  out[8]=reg_eip;out[9]=reg_flags;out[10]=lflags.var1.dword[0];out[11]=lflags.var2.dword[0];out[12]=lflags.res.dword[0];
  out[13]=lflags.type;out[14]=lflags.prev_type;out[15]=lflags.oldcf;out[16]=(unsigned)CPU_Cycles;
  out[17]=(unsigned)cpu.direction;out[18]=q[0];
  assert(fwrite(out,sizeof out,1,stdout)==1);
#ifdef FIST_EXECUTE_SEGMENT_INPUT
  unsigned segment_out[12];for(unsigned i=0;i<6;i++) {segment_out[2*i]=Segs.val[i];segment_out[2*i+1]=Segs.phys[i];}
  uint64_t stack[3]={(uint64_t)cpu.stack.big,cpu.stack.mask,cpu.stack.notmask};
  assert(fwrite(segment_out,sizeof segment_out,1,stdout)==1 && fwrite(stack,sizeof stack,1,stdout)==1);
#endif
  assert(fwrite(memory,sizeof memory,1,stdout)==1);
#ifdef FIST_EXECUTE_FETCH_TRACE
  assert(fwrite(fetch_reads,sizeof fetch_reads,1,stdout)==1);
#endif
 }
 assert(!ferror(stdin));
}
