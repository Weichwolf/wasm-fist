#ifndef FIST_INTERRUPT_H
#define FIST_INTERRUPT_H
#include "fist_ram.h"

/* Recovered original cpu.cpp CPU_LGDT/LIDT/LTR, CPU_HW_Interrupt and CPU_IRET
 * operations. Caller owns and transports the complete CPU/system context.
 * Task/V86/fault delivery and other physical device handlers require their original paths;
 * unsupported execution terminates rather than supplying successful guest work.
 * See board/0026 and tools/oracle/pit_irq_frame_case.json for reached coverage. */


enum {
    FIST_FLAG_IF=0x200u, FIST_FLAG_TF=0x100u, FIST_FLAG_DF=0x400u,
    FIST_FLAG_IOPL=0x3000u, FIST_FLAG_NT=0x4000u, FIST_FLAG_VM=0x20000u,
    FIST_FMASK_NORMAL=0x40fd5u, FIST_FMASK_ALL=0x47fd5u
};
typedef struct {
    uint32_t low, high;
} FistCpuDescriptor;
static inline unsigned fist_descriptor_type(FistCpuDescriptor d) { return (d.high>>8)&31; }
static inline unsigned fist_descriptor_dpl(FistCpuDescriptor d) { return (d.high>>13)&3; }
static inline unsigned fist_descriptor_present(FistCpuDescriptor d) { return (d.high>>15)&1; }
static inline unsigned fist_descriptor_big(FistCpuDescriptor d) { return (d.high>>22)&1; }
static inline uint32_t fist_descriptor_base(FistCpuDescriptor d)
{
    return (d.low>>16) | ((d.high&255)<<16) | (d.high&0xff000000u);
}
static inline FistCpuDescriptor fist_cpu_load_descriptor(FistCpuRam *bus, uint32_t address)
{
    FistCpuSystem *sys=bus->system;
    sys->mpl=0;
    FistCpuDescriptor d={fist_ram_read(bus,address,4),
                         fist_ram_read(bus,address+4,4)};
    sys->mpl=3;
    return d;
}
static inline uint32_t fist_descriptor_limit(FistCpuDescriptor d)
{
    uint32_t limit=(d.low&0xffffu) | (d.high&0xf0000u);
    return (d.high&0x800000u) ? (limit<<12)|0xfffu : limit;
}
static inline void fist_cpu_lgdt(FistCpuSystem *sys,uint32_t limit,uint32_t base)
{
    sys->gdt_limit=limit;sys->gdt_base=base;
}
static inline void fist_cpu_lidt(FistCpuSystem *sys,uint32_t limit,uint32_t base)
{
    sys->idt_limit=limit;sys->idt_base=base;
}
static inline int fist_cpu_prepare_exception(FistCpuSystem *sys,unsigned num,uint32_t error)
{
    sys->exception_which=num;sys->exception_error=error;
    return 1;
}
static inline int fist_cpu_descriptor(FistCpuRam *bus, uint32_t selector, FistCpuDescriptor *out)
{
    FistCpuSystem *sys=bus->system;
    uint32_t index=selector&~7u;
    uint32_t base=(selector&4) ? sys->ldt_base : sys->gdt_base;
    uint32_t limit=(selector&4) ? sys->ldt_limit : sys->gdt_limit;
    if(index>=limit) return 0;
    *out=fist_cpu_load_descriptor(bus,base+index);
    return 1;
}
static inline int fist_cpu_ltr(FistCpuRam *bus,uint32_t selector)
{
    FistCpuSystem *sys=bus->system;
    if(!(selector&0xfffcu)) {
        sys->tss_valid=0;sys->tss_selector=0;sys->tss_base=0;
        sys->tss_limit=0;sys->tss_is386=1;
        return 0;
    }
    FistCpuDescriptor d;
    if((selector&4) || !fist_cpu_descriptor(bus,selector,&d))
        return fist_cpu_prepare_exception(sys,13,selector);
    unsigned type=fist_descriptor_type(d);
    if(type!=1 && type!=9)return fist_cpu_prepare_exception(sys,13,selector);
    if(!fist_descriptor_present(d))return fist_cpu_prepare_exception(sys,11,selector);
    /* TaskStateSegment::SetSelector reloads its cached descriptor. */
    sys->tss_valid=0;
    fist_cpu_require(fist_cpu_descriptor(bus,selector,&d));
    fist_cpu_require(fist_descriptor_present(d));
    sys->tss_selector=selector;sys->tss_valid=1;
    sys->tss_base=fist_descriptor_base(d);sys->tss_limit=fist_descriptor_limit(d);
    sys->tss_is386=fist_descriptor_type(d)&8;
    d.high|=0x200u;
    sys->tss_desc_low=d.low;sys->tss_desc_high=d.high;
    uint32_t address=sys->gdt_base+(selector&~7u);
    sys->mpl=0;
    /* TaskStateSegment::SaveSelector -> GDTDescriptorTable::SetDescriptor ->
     * Descriptor::Save performs two mem_writed calls, including on devices. */
    fist_ram_write(bus,address,4,d.low);
    fist_ram_write(bus,address+4,4,d.high);
    sys->mpl=3;
    return 0;
}
static inline uint32_t fist_cpu_stack_advance(const FistCpuState *cpu, uint32_t esp, int delta)
{
    return (esp & cpu->stack_notmask) | ((esp+delta) & cpu->stack_mask);
}
static inline void fist_cpu_push(FistCpuRam *bus,
                                 unsigned width, uint32_t value)
{
    FistCpuState *cpu=bus->cpu;
    uint32_t next=fist_cpu_stack_advance(cpu,cpu->esp,-(int)width);
    fist_ram_resident_write(bus,2,next&cpu->stack_mask,width,value);
    cpu->esp=next;
}
static inline uint32_t fist_cpu_stack_read(FistCpuRam *bus,unsigned width,uint32_t *esp)
{
    FistCpuState *cpu=bus->cpu;
    uint32_t value=fist_ram_resident_read(bus,2,*esp&cpu->stack_mask,width);
    *esp=fist_cpu_stack_advance(cpu,*esp,width);
    return value;
}
static inline uint32_t fist_cpu_pop(FistCpuRam *bus,unsigned width)
{
    FistCpuState *cpu=bus->cpu;
    return fist_cpu_stack_read(bus,width,&cpu->esp);
}
static inline void fist_cpu_select_real_cs(FistCpuState *cpu,uint32_t cs)
{
    cpu->segments[1].value=(uint16_t)cs;
    cpu->segments[1].base=(uint16_t)cs<<4;
}
static inline void fist_cpu_select_stack(FistCpuState *cpu, uint32_t selector,
        uint32_t esp, FistCpuDescriptor d)
{
    cpu->segments[2].value=selector;
    cpu->segments[2].base=fist_descriptor_base(d);
    cpu->stack_big=fist_descriptor_big(d);
    cpu->stack_mask=cpu->stack_big ? UINT32_MAX : 0xffffu;
    cpu->stack_notmask=~cpu->stack_mask;
    cpu->esp=cpu->stack_big ? esp : fist_cpu_low(cpu->esp,esp,16);
}
static inline void fist_cpu_load_flags(FistCpuState *cpu,FistCpuSystem *sys,
                                      uint32_t flags,uint32_t mask)
{
    mask|=sys->flag_id_toggle;
    cpu->flags.flags=(cpu->flags.flags & ~mask) | (flags & mask) | 2;
    sys->direction=1-(int)((cpu->flags.flags & FIST_FLAG_DF)>>9);
    /* CPU_SetFlags plus DestroyConditionFlags: no operand or oldcf changes. */
    cpu->flags.type=FIST_LAZY_UNKNOWN;
}
static inline int fist_descriptor_nonconforming_code(FistCpuDescriptor d)
{
    unsigned t=fist_descriptor_type(d);
    return t>=0x18 && t<=0x1b;
}
static inline int fist_descriptor_code(FistCpuDescriptor d)
{
    unsigned t=fist_descriptor_type(d);
    return t>=0x18 && t<=0x1f;
}
static inline int fist_descriptor_writable_stack(FistCpuDescriptor d)
{
    unsigned t=fist_descriptor_type(d);
    return t==0x12 || t==0x13 || t==0x16 || t==0x17;
}
static inline void fist_cpu_hw_interrupt(FistCpuRam *bus,uint32_t num)
{
    FistCpuState *cpu=bus->cpu;
    FistCpuSystem *sys=bus->system;
    sys->lastint=(uint8_t)num;
    fist_cpu_fill_flags(cpu);
    uint32_t oldeip=cpu->eip;
    if(!cpu->pmode) {
        fist_cpu_push(bus,2,cpu->flags.flags);
        fist_cpu_push(bus,2,cpu->segments[1].value);
        fist_cpu_push(bus,2,oldeip);
        cpu->flags.flags&=~(FIST_FLAG_IF|FIST_FLAG_TF);
        cpu->eip=fist_ram_read(bus,sys->idt_base+num*4,2);
        fist_cpu_select_real_cs(cpu,fist_ram_read(bus,sys->idt_base+num*4+2,2));
        cpu->code_big=0;
        return;
    }
    fist_cpu_require(!(cpu->flags.flags&FIST_FLAG_VM));
    fist_cpu_require(num*8<sys->idt_limit);
    FistCpuDescriptor gate=fist_cpu_load_descriptor(bus,sys->idt_base+num*8);
    unsigned type=fist_descriptor_type(gate);
    fist_cpu_require(type==6 || type==7 || type==14 || type==15);
    fist_cpu_require(fist_descriptor_present(gate));
    uint32_t selector=gate.low>>16;
    FistCpuDescriptor cs;
    fist_cpu_require((selector&0xfffc)!=0);
    fist_cpu_require(fist_cpu_descriptor(bus,selector,&cs));
    unsigned dpl=fist_descriptor_dpl(cs);
    fist_cpu_require(dpl<=cpu->cpl && fist_descriptor_code(cs) && fist_descriptor_present(cs));
    unsigned width=(type&8) ? 4 : 2;
    if(fist_descriptor_nonconforming_code(cs)) {
        if(dpl<cpu->cpl) {
            uint32_t old_ss=cpu->segments[2].value,old_esp=cpu->esp;
            fist_cpu_require(sys->tss_valid);
            uint32_t tss=sys->tss_base+(sys->tss_is386 ? 4+dpl*8 : 2+dpl*4);
            sys->mpl=0;
            uint32_t esp=fist_ram_read(bus,tss,sys->tss_is386 ? 4 : 2);
            uint32_t ss=fist_ram_read(bus,tss+(sys->tss_is386 ? 4 : 2),2);
            sys->mpl=3;
            FistCpuDescriptor sd;
            fist_cpu_require((ss&0xfffc)!=0);
            fist_cpu_require(fist_cpu_descriptor(bus,ss,&sd));
            fist_cpu_require((ss&3)==dpl && fist_descriptor_dpl(sd)==dpl);
            fist_cpu_require(fist_descriptor_writable_stack(sd) && fist_descriptor_present(sd));
            fist_cpu_select_stack(cpu,ss,esp,sd);
            cpu->cpl=dpl;
            fist_cpu_push(bus,width,old_ss);
            fist_cpu_push(bus,width,old_esp);
        } else fist_cpu_require(dpl==cpu->cpl);
    }
    fist_cpu_push(bus,width,cpu->flags.flags);
    fist_cpu_push(bus,width,cpu->segments[1].value);
    fist_cpu_push(bus,width,oldeip);
    cpu->segments[1].value=(selector&0xfffc) | cpu->cpl;
    cpu->segments[1].base=fist_descriptor_base(cs);
    cpu->code_big=fist_descriptor_big(cs);
    cpu->eip=(gate.low&0xffff) | (gate.high&0xffff0000u);
    if(!(type&1))cpu->flags.flags&=~FIST_FLAG_IF;
    cpu->flags.flags&=~(FIST_FLAG_TF|FIST_FLAG_NT|FIST_FLAG_VM);
}
static inline void fist_cpu_check_segments(FistCpuRam *bus)
{
    FistCpuState *cpu=bus->cpu;
    const unsigned segments[]={0,3,4,5};
    for(unsigned i=0;i<4;++i) {
        unsigned s=segments[i];FistCpuDescriptor d;
        int invalid=!fist_cpu_descriptor(bus,cpu->segments[s].value,&d);
        if(!invalid) {
            unsigned type=fist_descriptor_type(d);
            if(type>=0x10 && type<=0x1b)invalid=cpu->cpl>fist_descriptor_dpl(d);
        }
        if(invalid) {cpu->segments[s].value=0;cpu->segments[s].base=0;}
    }
}
static inline void fist_cpu_iret(FistCpuRam *bus,unsigned use32)
{
    FistCpuState *cpu=bus->cpu;
    FistCpuSystem *sys=bus->system;
    unsigned width=use32 ? 4 : 2;
    if(!cpu->pmode) {
        cpu->eip=fist_cpu_pop(bus,width);
        fist_cpu_select_real_cs(cpu,fist_cpu_pop(bus,width));
        fist_cpu_load_flags(cpu,sys,fist_cpu_pop(bus,width),
                           FIST_FMASK_ALL & (use32 ? UINT32_MAX : 0xffff));
        cpu->code_big=0;
        return;
    }
    fist_cpu_require(!(cpu->flags.flags&(FIST_FLAG_VM|FIST_FLAG_NT)));
    uint32_t esp=cpu->esp;
    uint32_t ip=fist_cpu_stack_read(bus,width,&esp);
    uint32_t cs=fist_cpu_stack_read(bus,width,&esp)&0xffff;
    uint32_t flags=fist_cpu_stack_read(bus,width,&esp);
    if(!use32)flags|=cpu->flags.flags&0xffff0000u;
    fist_cpu_require(!(flags&FIST_FLAG_VM));
    uint32_t rpl=cs&3;
    FistCpuDescriptor cd;
    fist_cpu_require((cs&0xfffc)!=0 && rpl>=cpu->cpl);
    fist_cpu_require(fist_cpu_descriptor(bus,cs,&cd));
    fist_cpu_require(fist_descriptor_code(cd) && fist_descriptor_present(cd));
    if(fist_descriptor_nonconforming_code(cd))fist_cpu_require(rpl==fist_descriptor_dpl(cd));
    else fist_cpu_require(fist_descriptor_dpl(cd)<=rpl);
    uint32_t mask=cpu->cpl ? FIST_FMASK_NORMAL|FIST_FLAG_NT : FIST_FMASK_ALL;
    if(((cpu->flags.flags&FIST_FLAG_IOPL)>>12)<cpu->cpl)mask&=~FIST_FLAG_IF;
    if(rpl==cpu->cpl) {
        cpu->esp=esp;
        cpu->segments[1].value=cs;cpu->segments[1].base=fist_descriptor_base(cd);
        cpu->code_big=fist_descriptor_big(cd);cpu->eip=ip;
        fist_cpu_load_flags(cpu,sys,flags,mask);
    } else {
        uint32_t new_esp=fist_cpu_stack_read(bus,width,&esp);
        uint32_t ss=fist_cpu_stack_read(bus,width,&esp)&0xffff;
        FistCpuDescriptor sd;
        fist_cpu_require((ss&0xfffc)!=0 && (ss&3)==rpl);
        fist_cpu_require(fist_cpu_descriptor(bus,ss,&sd));
        fist_cpu_require(fist_descriptor_dpl(sd)==rpl && fist_descriptor_writable_stack(sd) && fist_descriptor_present(sd));
        cpu->segments[1].value=cs;cpu->segments[1].base=fist_descriptor_base(cd);
        cpu->code_big=fist_descriptor_big(cd);
        fist_cpu_load_flags(cpu,sys,flags,mask);
        cpu->cpl=rpl;cpu->eip=ip;
        fist_cpu_select_stack(cpu,ss,new_esp,sd);
        fist_cpu_check_segments(bus);
    }
}
#endif
