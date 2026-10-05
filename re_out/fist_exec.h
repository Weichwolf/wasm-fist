#ifndef FIST_EXEC_H
#define FIST_EXEC_H
#include "fist_interrupt.h"
/* Recovered normal-core instruction execution against one CPU/system/RAM owner.
 * The caller owns fetch/budget/PIC retirement and original host callbacks.
 * No guest address, source snapshot, diagnostic file or stop condition here.
 * Contracts: DOSBox core_normal/{prefix_none,prefix_66,prefix_0f,
 * prefix_66_0f,string,support}.h and the shared CPU/control/memory owners.
 * This executes one already charged fetch, including one REP chunk. The
 * caller must handle normal-core exits, trap/PIC dispatch and host callbacks;
 * the return value identifies original retirement checks, rather than inferring
 * a core exit from flags after every opcode. It neither initializes a machine
 * nor supplies missing opcodes. */
typedef struct FistExec FistExec;
struct FistExec {
 FistCpuRam *bus; void *opaque;
 uint32_t (*in)(void *,unsigned);
 void (*out)(void *,unsigned,unsigned);
 unsigned (*budget)(void *);
 void (*credit)(void *);
 void (*charge)(void *,unsigned);
 void (*callback)(FistExec *,unsigned);
 void (*observe)(FistExec *,unsigned,uint32_t,uint32_t);
 /* Observe the explicit instruction fetch, including displacement reads.
  * A data operand using CS is still an ordinary RAM access. */
 void (*code_fetch)(FistExec *,uint32_t,unsigned);
};
enum {FIST_EXEC_INT_BEFORE,FIST_EXEC_INT_AFTER,FIST_EXEC_RET_BEFORE,FIST_EXEC_RET_AFTER,
 FIST_EXEC_CALLBACK_BEFORE,FIST_EXEC_CALLBACK_AFTER};
static inline void fist_exec_observe(FistExec *e,unsigned event,uint32_t a,uint32_t b) {
 if(e->observe)e->observe(e,event,a,b);
}
/* Normal-core decoder choice is machine metadata distinct from EFLAGS.TF.
 * Only original checked instructions can select the trap decoder. A raw PIC
 * pending mask causes a core exit even if priority blocks actual delivery. */
enum {FIST_EXEC_CHECK_PIC=1,FIST_EXEC_CHECK_TRAP=2,FIST_EXEC_RETURN_CORE=4};
static inline int fist_exec_core_exit(FistExec *e,unsigned checks,
                                     unsigned pending,unsigned *trap_decoder) {
 fist_cpu_require(trap_decoder!=NULL && *trap_decoder<=1);
 if((checks&FIST_EXEC_CHECK_TRAP) && (e->bus->cpu->flags.flags&0x100u)) {
  *trap_decoder=1;return 1;
 }
 return (checks&FIST_EXEC_RETURN_CORE) ||
   ((checks&FIST_EXEC_CHECK_PIC) && (e->bus->cpu->flags.flags&FIST_FLAG_IF) && pending);
}
static uint32_t *fist_exec_reg(FistExec *e,unsigned i) {
 switch(i){case 0:return &e->bus->cpu->eax;case 1:return &e->bus->cpu->ecx;case 2:return &e->bus->cpu->edx;
 case 3:return &e->bus->cpu->ebx;case 4:return &e->bus->cpu->esp;case 5:return &e->bus->cpu->ebp;case 6:return &e->bus->cpu->esi;case 7:return &e->bus->cpu->edi;default:abort();}
}
static uint32_t fist_exec_reg_read(FistExec *e,unsigned i,unsigned w) {
 if(w==1 && i>=4)return (*fist_exec_reg(e,i-4)>>8)&255;
 return fist_cpu_low(0,*fist_exec_reg(e,i),8*w);
}
static void fist_exec_reg_write(FistExec *e,unsigned i,unsigned w,uint32_t v) {
 if(w==1 && i>=4){uint32_t *r=fist_exec_reg(e,i-4);*r=(*r&~0xff00u)|((v&255)<<8);}
 else {*fist_exec_reg(e,i)=fist_cpu_low(*fist_exec_reg(e,i),v,8*w);}
}
static uint32_t fist_exec_fetch_code(FistExec *e,uint32_t *ip,unsigned w) {
 if(e->code_fetch)e->code_fetch(e,*ip,w);
 uint32_t v=fist_ram_resident_read(e->bus,1,*ip,w);*ip+=w;return v;
}
typedef struct {unsigned index,seg;uint32_t offset;int direct;} FistExecOperand;
static FistExecOperand fist_exec_operand(FistExec *e,uint32_t *ip,unsigned m,unsigned address,unsigned seg) {
 unsigned mode=m>>6,b=m&7;FistExecOperand q={.index=b,.seg=3,.direct=mode==3};
 if(q.direct)return q;
 if(address==4){
  if(b==4){unsigned sib=fist_exec_fetch_code(e,ip,1),i=(sib>>3)&7;b=sib&7;if(i!=4)q.offset=*fist_exec_reg(e,i)<<(sib>>6);}
  if(mode==0 && b==5)q.offset+=fist_exec_fetch_code(e,ip,4);
  else {q.offset+=*fist_exec_reg(e,b);if(b==4 || b==5)q.seg=2;}
  if(mode==1)q.offset+=(uint32_t)(int32_t)(int8_t)fist_exec_fetch_code(e,ip,1);
  if(mode==2)q.offset+=fist_exec_fetch_code(e,ip,4);
 }else{
  switch(b){case 0:q.offset=e->bus->cpu->ebx+e->bus->cpu->esi;break;case 1:q.offset=e->bus->cpu->ebx+e->bus->cpu->edi;break;
  case 2:q.offset=e->bus->cpu->ebp+e->bus->cpu->esi;q.seg=2;break;case 3:q.offset=e->bus->cpu->ebp+e->bus->cpu->edi;q.seg=2;break;
  case 4:q.offset=e->bus->cpu->esi;break;case 5:q.offset=e->bus->cpu->edi;break;
  case 6:if(mode==0)q.offset=fist_exec_fetch_code(e,ip,2);else {q.offset=e->bus->cpu->ebp;q.seg=2;}break;
  case 7:q.offset=e->bus->cpu->ebx;break;}
  if(mode==1)q.offset+=(int8_t)fist_exec_fetch_code(e,ip,1);
  if(mode==2)q.offset+=fist_exec_fetch_code(e,ip,2);
  q.offset&=0xffff;
 }
 if(seg<6)q.seg=seg;return q;
}
static uint32_t fist_exec_read_op(FistExec *e,FistExecOperand q,unsigned w) {
 return q.direct?fist_exec_reg_read(e,q.index,w):fist_ram_resident_read(e->bus,q.seg,q.offset,w);
}
static void fist_exec_write_op(FistExec *e,FistExecOperand q,unsigned w,uint32_t v) {
 if(q.direct)fist_exec_reg_write(e,q.index,w,v);else fist_ram_resident_write(e->bus,q.seg,q.offset,w,v);
}
static void fist_exec_select_segment(FistExec *e,unsigned s,uint32_t value) {
 fist_cpu_require(s<6 && s!=1);
 value&=0xffff;
 if(!e->bus->cpu->pmode || (e->bus->cpu->flags.flags&FIST_FLAG_VM)){
  e->bus->cpu->segments[s].value=value;e->bus->cpu->segments[s].base=value<<4;
  if(s==2){e->bus->cpu->stack_big=0;e->bus->cpu->stack_mask=0xffff;e->bus->cpu->stack_notmask=0xffff0000;}
  return;
 }
 if(s!=2 && (value&0xfffc)==0){e->bus->cpu->segments[s].value=value;e->bus->cpu->segments[s].base=0;return;}
 FistCpuDescriptor d;fist_cpu_require(fist_cpu_descriptor(e->bus,value,&d));
 unsigned t=fist_descriptor_type(d);
 if(s==2){fist_cpu_require((value&3)==e->bus->cpu->cpl && fist_descriptor_dpl(d)==e->bus->cpu->cpl && fist_descriptor_writable_stack(d));
  e->bus->cpu->stack_big=fist_descriptor_big(d);e->bus->cpu->stack_mask=e->bus->cpu->stack_big?UINT32_MAX:0xffff;e->bus->cpu->stack_notmask=~e->bus->cpu->stack_mask;
 }else{
  fist_cpu_require((t>=0x10 && t<=0x17)||t==0x1a||t==0x1b||t==0x1e||t==0x1f);
  if(t!=0x1e && t!=0x1f)fist_cpu_require((value&3)<=fist_descriptor_dpl(d) && e->bus->cpu->cpl<=fist_descriptor_dpl(d));
 }
 fist_cpu_require(fist_descriptor_present(d));e->bus->cpu->segments[s].value=value;e->bus->cpu->segments[s].base=fist_descriptor_base(d);
}
static void fist_exec_pop_flags(FistExec *e,unsigned width) {
 fist_cpu_require(!e->bus->cpu->pmode || !(e->bus->cpu->flags.flags&FIST_FLAG_VM) ||
                  (e->bus->cpu->flags.flags&FIST_FLAG_IOPL)==FIST_FLAG_IOPL);
 uint32_t mask=FIST_FMASK_ALL;
 if(e->bus->cpu->pmode && e->bus->cpu->cpl>0)mask&=~FIST_FLAG_IOPL;
 if(e->bus->cpu->pmode && !(e->bus->cpu->flags.flags&FIST_FLAG_VM) && ((e->bus->cpu->flags.flags&FIST_FLAG_IOPL)>>12)<e->bus->cpu->cpl)mask&=~FIST_FLAG_IF;
 if(width==2)mask&=0xffff;
 fist_cpu_load_flags(e->bus->cpu,e->bus->system,fist_cpu_pop(e->bus,width),mask);
}
static void fist_exec_jump_far(FistExec *e,uint32_t selector,uint32_t offset,unsigned width) {
 if(!e->bus->cpu->pmode || (e->bus->cpu->flags.flags&FIST_FLAG_VM)){
  fist_cpu_select_real_cs(e->bus->cpu,selector);e->bus->cpu->code_big=0;e->bus->cpu->eip=width==2?offset&0xffff:offset;return;
 }
 FistCpuDescriptor d;fist_cpu_require((selector&0xfffc) && fist_cpu_descriptor(e->bus,selector,&d));
 unsigned t=fist_descriptor_type(d),dpl=fist_descriptor_dpl(d);
 fist_cpu_require(fist_descriptor_code(d) && fist_descriptor_present(d));
 if(t<=0x1b)fist_cpu_require((selector&3)<=e->bus->cpu->cpl && dpl==e->bus->cpu->cpl);
 else fist_cpu_require(dpl<=e->bus->cpu->cpl);
 e->bus->cpu->segments[1].value=(selector&0xfffc)|e->bus->cpu->cpl;e->bus->cpu->segments[1].base=fist_descriptor_base(d);
 e->bus->cpu->code_big=fist_descriptor_big(d);e->bus->cpu->eip=offset;
}
static uint32_t fist_exec_alu(FistExec *e,unsigned operation,unsigned width,uint32_t a,uint32_t b) {
 uint32_t result;unsigned type;
 switch(operation){
 case 0:fist_cpu_require(width==1||width==2||width==4);result=a+b;type=width==1?FIST_LAZY_ADDB:width==2?FIST_LAZY_ADDW:FIST_LAZY_ADDD;break;
 case 1:result=a|b;type=width==1?FIST_LAZY_ORB:width==2?FIST_LAZY_ORW:FIST_LAZY_ORD;break;
 case 4:fist_cpu_require(width==1||width==2);result=a&b;type=width==1?FIST_LAZY_ANDB:FIST_LAZY_ANDW;break;
 case 5:result=a-b;fist_cpu_require(width==1||width==4);type=width==1?FIST_LAZY_SUBB:FIST_LAZY_SUBD;break;
 case 6:result=a^b;type=width==1?FIST_LAZY_XORB:width==2?FIST_LAZY_XORW:FIST_LAZY_XORD;break;
 case 7:result=a-b;type=width==1?FIST_LAZY_CMPB:width==2?FIST_LAZY_CMPW:FIST_LAZY_CMPD;break;
 case 8:fist_cpu_require(width==1||width==2);result=a&b;type=width==1?FIST_LAZY_TESTB:FIST_LAZY_TESTW;break;
 default:abort();
 }fist_cpu_alu(e->bus->cpu,type,8*width,a,b,result);return fist_cpu_low(0,result,8*width);
}
static uint32_t fist_exec_incdec(FistExec *e,unsigned decrement,unsigned width,uint32_t a) {
 unsigned type=decrement?(width==1?FIST_LAZY_DECB:width==2?FIST_LAZY_DECW:FIST_LAZY_DECD):(width==1?FIST_LAZY_INCB:width==2?FIST_LAZY_INCW:FIST_LAZY_INCD);
 return fist_cpu_incdec(e->bus->cpu,type,a);
}
/* JumpCond16_b/w writes reg_ip, preserving its upper word after SAVEIP.
 * Not-taken branches advance that word without fetching the displacement. */
static void fist_exec_conditional(FistExec *e,uint32_t *ip,unsigned width,unsigned displacement,int take) {
 uint32_t saved=*ip;int32_t delta=0;
 if(take)delta=displacement==1?(int8_t)fist_exec_fetch_code(e,ip,1):
   displacement==2?(int16_t)fist_exec_fetch_code(e,ip,2):(int32_t)fist_exec_fetch_code(e,ip,4);
 uint32_t next=saved+delta+displacement;
 *ip=width==2?(saved&0xffff0000u)|(next&0xffffu):next;
}
static unsigned fist_exec_fetched(FistExec *e) {
  uint32_t ip=e->bus->cpu->eip;unsigned width=e->bus->cpu->code_big?4:2,address=width,seg=6,rep=0,op;
  for(;;){op=fist_exec_fetch_code(e,&ip,1);if(op==0x66)width=e->bus->cpu->code_big?2:4;
   else if(op==0x67)address=e->bus->cpu->code_big?2:4;
   else if(op==0x2e)seg=1;else if(op==0x36)seg=2;else if(op==0x3e)seg=3;
   else if(op==0x26)seg=0;else if(op==0x64)seg=4;else if(op==0x65)seg=5;
   else if(op==0xf3||op==0xf2)rep=1;else break;
  }
  if(op==0x90){}
  else if(op==0xfc){e->bus->cpu->flags.flags&=~0x400u;e->bus->system->direction=1;}
  else if(op==0xe6){unsigned port=fist_exec_fetch_code(e,&ip,1);e->out(e->opaque,port,(uint8_t)e->bus->cpu->eax);}
  else if(op==0xee)e->out(e->opaque,(uint16_t)e->bus->cpu->edx,(uint8_t)e->bus->cpu->eax);
  else if(op==0xec)fist_exec_reg_write(e,0,1,e->in(e->opaque,(uint16_t)e->bus->cpu->edx));
  else if(op==0x32||op==0x39||op==0x2b){unsigned m=fist_exec_fetch_code(e,&ip,1),i=(m>>3)&7,w=op==0x32?1:width;FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);
   if(op==0x39)fist_exec_alu(e,7,w,fist_exec_read_op(e,q,w),fist_exec_reg_read(e,i,w));
   else fist_exec_reg_write(e,i,w,fist_exec_alu(e,op==0x32?6:5,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w)));
  }else if(op==0x24)fist_exec_reg_write(e,0,1,fist_exec_alu(e,4,1,fist_exec_reg_read(e,0,1),fist_exec_fetch_code(e,&ip,1)));
  else if(op==0xa8){fist_exec_alu(e,8,1,fist_exec_reg_read(e,0,1),fist_exec_fetch_code(e,&ip,1));}
  else if(op==0x03){unsigned m=fist_exec_fetch_code(e,&ip,1);FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);unsigned i=(m>>3)&7;fist_exec_reg_write(e,i,width,fist_exec_alu(e,0,width,fist_exec_reg_read(e,i,width),fist_exec_read_op(e,q,width)));}
  else if(op==0xc4){unsigned m=fist_exec_fetch_code(e,&ip,1);FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);fist_cpu_require(!q.direct);FistExecOperand sel=q;sel.offset+=width;fist_exec_select_segment(e,0,fist_exec_read_op(e,sel,2));fist_exec_reg_write(e,(m>>3)&7,width,fist_exec_read_op(e,q,width));}
  else if(op==0xe3)fist_exec_conditional(e,&ip,width,1,!fist_exec_reg_read(e,1,address));
  else if(op==0xe1||op==0xe2){fist_exec_reg_write(e,1,address,fist_exec_reg_read(e,1,address)-1);fist_exec_conditional(e,&ip,width,1,fist_exec_reg_read(e,1,address) && (op==0xe2 || fist_cpu_zf(e->bus->cpu)));}
  else if(op==0xe9){int32_t d=width==4?(int32_t)fist_exec_fetch_code(e,&ip,4):(int16_t)fist_exec_fetch_code(e,&ip,2);ip=width==2?(uint16_t)(ip+d):ip+d;}
  else if(op>=0xb0 && op<=0xb7)fist_exec_reg_write(e,op&7,1,fist_exec_fetch_code(e,&ip,1));
  else if(op>=0x50 && op<=0x57)fist_cpu_push(e->bus,width,fist_exec_reg_read(e,op&7,width));
  else if(op==0x06||op==0x0e||op==0x1e)fist_cpu_push(e->bus,width,e->bus->cpu->segments[op==0x06?0:op==0x0e?1:3].value);
  else if(op==0x9c){fist_cpu_require(!e->bus->cpu->pmode || !(e->bus->cpu->flags.flags&FIST_FLAG_VM) || (e->bus->cpu->flags.flags&FIST_FLAG_IOPL)==FIST_FLAG_IOPL);fist_cpu_fill_flags(e->bus->cpu);fist_cpu_push(e->bus,width,e->bus->cpu->flags.flags&(width==4?0xfcffffu:0xffffu));}
  else if(op==0x88){unsigned m=fist_exec_fetch_code(e,&ip,1);FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);fist_exec_write_op(e,q,1,fist_exec_reg_read(e,(m>>3)&7,1));}
  else if(op==0xc6){unsigned m=fist_exec_fetch_code(e,&ip,1);FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);fist_cpu_require(((m>>3)&7)==0);fist_exec_write_op(e,q,1,fist_exec_fetch_code(e,&ip,1));}
  else if(op==0x0a||op==0x0b){unsigned m=fist_exec_fetch_code(e,&ip,1),w=op==0x0a?1:width;FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);unsigned i=(m>>3)&7;fist_exec_reg_write(e,i,w,fist_exec_alu(e,1,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w)));}
  else if(op==0xe8){uint32_t d=fist_exec_fetch_code(e,&ip,width);fist_cpu_push(e->bus,width,ip);e->bus->cpu->eip=width==2?(uint16_t)(ip+d):ip+d;return 0;}
  else if(op==0xa3){unsigned offset=fist_exec_fetch_code(e,&ip,address);fist_ram_resident_write(e->bus,seg<6?seg:3,offset,width,fist_exec_reg_read(e,0,width));}
  else if(op==0xac){uint32_t offset=address==4?e->bus->cpu->esi:e->bus->cpu->esi&0xffff;fist_exec_reg_write(e,0,1,fist_ram_resident_read(e->bus,seg<6?seg:3,offset,1));fist_exec_reg_write(e,6,address,offset+e->bus->system->direction);}
  else if(op==0xc0){unsigned m=fist_exec_fetch_code(e,&ip,1);FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);unsigned count=fist_exec_fetch_code(e,&ip,1)&31;fist_cpu_require(((m>>3)&7)==4);uint32_t a=fist_exec_read_op(e,q,1);
   if(count)fist_exec_write_op(e,q,1,fist_cpu_shl(e->bus->cpu,8,a,count));
  }
  else if(op>=0x58 && op<=0x5f)fist_exec_reg_write(e,op&7,width,fist_cpu_pop(e->bus,width));
  else if(op==0x07 || op==0x1f){unsigned which=op==0x07?0:3;
   uint32_t value=fist_ram_resident_read(e->bus,2,e->bus->cpu->esp&e->bus->cpu->stack_mask,2);fist_exec_select_segment(e,which,value);
   e->bus->cpu->esp=fist_cpu_stack_advance(e->bus->cpu,e->bus->cpu->esp,width);
  }else if(op==0x9d){fist_exec_pop_flags(e,width);e->bus->cpu->eip=ip;return FIST_EXEC_CHECK_TRAP|FIST_EXEC_CHECK_PIC;}
  else if(op==0x9e){fist_cpu_fill_flags(e->bus->cpu);fist_cpu_load_flags(e->bus->cpu,e->bus->system,fist_exec_reg_read(e,4,1),FIST_FMASK_NORMAL&255);}
  else if(op==0xfa)fist_cpu_set_if(e->bus->cpu,0);
  else if(op==0xfb){fist_cpu_set_if(e->bus->cpu,1);e->bus->cpu->eip=ip;return FIST_EXEC_CHECK_PIC;}
  else if(op==0x0f){
   unsigned second=fist_exec_fetch_code(e,&ip,1);
   if(second==0xa0||second==0xa8)fist_cpu_push(e->bus,width,e->bus->cpu->segments[second==0xa0?4:5].value);
   else if(second==0xb6||second==0xb7){unsigned m=fist_exec_fetch_code(e,&ip,1);FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);fist_exec_reg_write(e,(m>>3)&7,width,fist_exec_read_op(e,q,second==0xb6?1:2));}
   else if(second==0xa1||second==0xa9){unsigned which=second==0xa1?4:5;
    uint32_t v=fist_ram_resident_read(e->bus,2,e->bus->cpu->esp&e->bus->cpu->stack_mask,2);fist_exec_select_segment(e,which,v);
    e->bus->cpu->esp=fist_cpu_stack_advance(e->bus->cpu,e->bus->cpu->esp,width);
   }else if(second==0x22){unsigned m=fist_exec_fetch_code(e,&ip,1);fist_cpu_require((m>>6)==3);unsigned cr=(m>>3)&7;
    if(cr==0)fist_ram_set_cr0_normal(e->bus,*fist_exec_reg(e,m&7));
    else {fist_cpu_require(cr==3);fist_ram_set_cr3(e->bus,*fist_exec_reg(e,m&7));}
   }else if(second==0x00||second==0x01){unsigned m=fist_exec_fetch_code(e,&ip,1);FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);unsigned which=(m>>3)&7;
    if(second==0x00){fist_cpu_require(which==3 && fist_cpu_ltr(e->bus,fist_exec_read_op(e,q,2))==0);}
    else {fist_cpu_require(!q.direct && (which==2||which==3));uint32_t limit=fist_exec_read_op(e,q,2);q.offset+=2;
     uint32_t base=fist_exec_read_op(e,q,4);if(width==2)base&=0xffffff;
     if(which==2)fist_cpu_lgdt(e->bus->system,limit,base);else fist_cpu_lidt(e->bus->system,limit,base);
    }
   }else if(second==0x93){unsigned m=fist_exec_fetch_code(e,&ip,1);FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);fist_exec_write_op(e,q,1,!fist_cpu_cf(e->bus->cpu));}
   else if(second==0x84||second==0x85||second==0x86){int take=second==0x84?fist_cpu_zf(e->bus->cpu):second==0x85?!fist_cpu_zf(e->bus->cpu):fist_cpu_cf(e->bus->cpu)||fist_cpu_zf(e->bus->cpu);fist_exec_conditional(e,&ip,width,width,take);}
   else abort();
  }else if(op==0x8f){
   uint32_t value=fist_cpu_pop(e->bus,width);unsigned m=fist_exec_fetch_code(e,&ip,1);FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);
   fist_cpu_require(((m>>3)&7)==0);fist_exec_write_op(e,q,width,value);
  }else if(op==0xff){unsigned m=fist_exec_fetch_code(e,&ip,1),which=(m>>3)&7;FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);uint32_t v=fist_exec_read_op(e,q,width);
   if(which==0||which==1)fist_exec_write_op(e,q,width,fist_exec_incdec(e,which,width,v));
   else if(which==6)fist_cpu_push(e->bus,width,v);
   else if(which==2){fist_cpu_push(e->bus,width,ip);e->bus->cpu->eip=v;return 0;}
   else if(which==3){fist_cpu_require(!q.direct && !e->bus->cpu->pmode);q.offset+=width;unsigned cs=fist_exec_read_op(e,q,2);
    fist_cpu_fill_flags(e->bus->cpu);fist_cpu_push(e->bus,width,e->bus->cpu->segments[1].value);fist_cpu_push(e->bus,width,ip);
    fist_exec_jump_far(e,cs,v,width);return FIST_EXEC_CHECK_TRAP;
   }else {fist_cpu_require(which==4);e->bus->cpu->eip=v;return 0;}
  }else if(op==0x3c){uint32_t a=fist_exec_reg_read(e,0,1),b=fist_exec_fetch_code(e,&ip,1);fist_cpu_alu(e->bus->cpu,FIST_LAZY_CMPB,8,a,b,a-b);}
  else if(op==0x76||op==0x77||op==0x74||op==0x75||op==0x72||op==0x73){
   int take=op==0x76?fist_cpu_cf(e->bus->cpu)||fist_cpu_zf(e->bus->cpu):op==0x77?!fist_cpu_cf(e->bus->cpu)&&!fist_cpu_zf(e->bus->cpu):op==0x74?fist_cpu_zf(e->bus->cpu):op==0x75?!fist_cpu_zf(e->bus->cpu):op==0x72?fist_cpu_cf(e->bus->cpu):!fist_cpu_cf(e->bus->cpu);
   fist_exec_conditional(e,&ip,width,1,take);
  }
  else if(op==0xeb){int8_t d=fist_exec_fetch_code(e,&ip,1);ip=width==2?(uint16_t)(ip+d):ip+d;}
  else if(op==0xd0||op==0xd1){unsigned m=fist_exec_fetch_code(e,&ip,1),which=(m>>3)&7,bytes=op==0xd0?1:width;FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);
   fist_cpu_require(which==5 || (op==0xd1 && which==4 && width==2));uint32_t v=fist_exec_read_op(e,q,bytes);
   fist_exec_write_op(e,q,bytes,which==5?fist_cpu_shr(e->bus->cpu,bytes*8,v,1):fist_cpu_shl(e->bus->cpu,16,v,1));
  }else if(op==0x8b||op==0x8d||op==0x8e||op==0x89||op==0x8a||op==0x86||op==0x33||op==0x09||op==0x2a){unsigned m=fist_exec_fetch_code(e,&ip,1);FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);
   unsigned index=(m>>3)&7;
   if(op==0x8e){fist_exec_select_segment(e,index,fist_exec_read_op(e,q,2));
    if(index==2)e->credit(e->opaque);
   }else if(op==0x89)fist_exec_write_op(e,q,width,fist_exec_reg_read(e,index,width));
   else if(op==0x8a)fist_exec_reg_write(e,index,1,fist_exec_read_op(e,q,1));
   else if(op==0x86){uint32_t v=fist_exec_read_op(e,q,1);fist_exec_write_op(e,q,1,fist_exec_reg_read(e,index,1));fist_exec_reg_write(e,index,1,v);}
   else if(op==0x33)fist_exec_reg_write(e,index,width,fist_exec_alu(e,6,width,fist_exec_reg_read(e,index,width),fist_exec_read_op(e,q,width)));
   else if(op==0x09)fist_exec_write_op(e,q,width,fist_exec_alu(e,1,width,fist_exec_read_op(e,q,width),fist_exec_reg_read(e,index,width)));
   else if(op==0x2a)fist_exec_reg_write(e,index,1,fist_exec_alu(e,5,1,fist_exec_reg_read(e,index,1),fist_exec_read_op(e,q,1)));
   else {fist_cpu_require(op!=0x8d || !q.direct);fist_exec_reg_write(e,index,width,op==0x8b?fist_exec_read_op(e,q,width):q.offset);}
  }else if(op==0xc3){e->bus->cpu->eip=fist_cpu_pop(e->bus,width);return 0;}
  else if(op==0xcb){fist_cpu_fill_flags(e->bus->cpu);fist_exec_observe(e,FIST_EXEC_RET_BEFORE,width==4,0);
   fist_cpu_far_ret(e->bus,width==4,0);fist_exec_observe(e,FIST_EXEC_RET_AFTER,width==4,0);return 0;}
  else if(op==0xcf){fist_cpu_iret(e->bus,width==4);return FIST_EXEC_CHECK_TRAP|FIST_EXEC_CHECK_PIC;}
  else if(op==0xcd){unsigned num=fist_exec_fetch_code(e,&ip,1);
   fist_exec_observe(e,FIST_EXEC_INT_BEFORE,num,ip);
   fist_cpu_sw_interrupt(e->bus,num,ip);fist_exec_observe(e,FIST_EXEC_INT_AFTER,num,ip);return 0;}
  else if(op==0xea){uint32_t offset=fist_exec_fetch_code(e,&ip,width),selector=fist_exec_fetch_code(e,&ip,2);fist_cpu_fill_flags(e->bus->cpu);fist_exec_jump_far(e,selector,offset,width);return FIST_EXEC_CHECK_TRAP;}
  else if(op==0xa0||op==0xa1){unsigned offset=fist_exec_fetch_code(e,&ip,address),bytes=op==0xa0?1:width;fist_exec_reg_write(e,0,bytes,fist_ram_resident_read(e->bus,seg<6?seg:3,offset,bytes));}
  else if(op==0x25){fist_exec_reg_write(e,0,width,fist_exec_alu(e,4,width,fist_exec_reg_read(e,0,width),fist_exec_fetch_code(e,&ip,width)));}
  else if(op>=0x40 && op<=0x4f){fist_exec_reg_write(e,op&7,width,fist_exec_incdec(e,op>=0x48,width,fist_exec_reg_read(e,op&7,width)));}
  else if(op==0x80||op==0x81||op==0x83||op==0xf6||op==0xf7||op==0xc7){unsigned m=fist_exec_fetch_code(e,&ip,1),w=(op==0x80||op==0xf6)?1:width;FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);
   unsigned which=(m>>3)&7;uint32_t b=op==0x83?fist_cpu_low(0,(uint32_t)(int32_t)(int8_t)fist_exec_fetch_code(e,&ip,1),w*8):fist_exec_fetch_code(e,&ip,w);
   if(op==0xc7){fist_cpu_require(which==0);fist_exec_write_op(e,q,w,b);}
   else {unsigned operation=(op==0xf6||op==0xf7)?8:which;
    if(operation==8)fist_cpu_require(which==0);uint32_t v=fist_exec_alu(e,operation,w,fist_exec_read_op(e,q,w),b);
    if(operation!=7 && operation!=8)fist_exec_write_op(e,q,w,v);
   }
  }else if(op==0xfe){unsigned m=fist_exec_fetch_code(e,&ip,1),which=(m>>3)&7;
   if(which==7){fist_cpu_require(m==0x38);unsigned callback=fist_exec_fetch_code(e,&ip,2);
    fist_cpu_fill_flags(e->bus->cpu);e->bus->cpu->eip=ip;fist_exec_observe(e,FIST_EXEC_CALLBACK_BEFORE,callback,0);
    fist_cpu_require(e->callback!=NULL);e->callback(e,callback);
    fist_exec_observe(e,FIST_EXEC_CALLBACK_AFTER,callback,0);return FIST_EXEC_RETURN_CORE;
   }else{fist_cpu_require(which<=1);FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);fist_exec_write_op(e,q,1,fist_exec_incdec(e,which,1,fist_exec_read_op(e,q,1)));}
  }
  else if(op>=0xb8 && op<=0xbf)fist_exec_reg_write(e,op&7,width,fist_exec_fetch_code(e,&ip,width));
  else if(op==0xa4||op==0xa5){
   unsigned w=op==0xa4?1:width;
   unsigned count=rep?fist_exec_reg_read(e,1,address):1, take=count,cost=0;
   if(rep){e->credit(e->opaque);unsigned budget=e->budget(e->opaque);
    take=count<budget?count:budget;
    cost=count>budget?budget:count<=1&&budget<=1?1:count;
   }
   for(unsigned i=0;i<take;i++){
    unsigned si=fist_exec_reg_read(e,6,address),di=fist_exec_reg_read(e,7,address);
    uint32_t v=fist_ram_resident_read(e->bus,seg<6?seg:3,si,w);
    fist_ram_resident_write(e->bus,0,di,w,v);
    fist_exec_reg_write(e,6,address,si+e->bus->system->direction*w);fist_exec_reg_write(e,7,address,di+e->bus->system->direction*w);
   }
   if(rep){fist_exec_reg_write(e,1,address,count-take);if(count>take)ip=e->bus->cpu->eip;
    e->charge(e->opaque,cost);
   }
  }else abort();
  e->bus->cpu->eip=ip;return 0;
}
#endif
