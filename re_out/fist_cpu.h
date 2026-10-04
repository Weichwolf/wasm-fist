#ifndef FIST_CPU_H
#define FIST_CPU_H
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Explicit original CPU context for recovered instruction producers. Register
 * and lazy-flag words retain all bits, including untouched upper byte/word data.
 * Legacy C-only callers still require recovered context transport. */
typedef struct {
    uint32_t flags, var1, var2, res, type, prev_type, oldcf;
} FistCpuFlags;
typedef struct {
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi, eip;
    struct { uint32_t value, base; } segments[6];
    FistCpuFlags flags;
    uint32_t code_big, stack_big, stack_mask, stack_notmask;
    uint32_t pmode, cpl, cr0, cr3, paging_enabled;
} FistCpuState;

/* Original lazyflags.h enum values for the operations recovered here. */
enum {
    FIST_LAZY_UNKNOWN=0, FIST_LAZY_ADDW=2, FIST_LAZY_XORB=19,
    FIST_LAZY_XORD=21, FIST_LAZY_CMPB=22, FIST_LAZY_CMPW=23,
    FIST_LAZY_TESTB=31
};

static inline uint32_t fist_cpu_low(uint32_t old, uint32_t value, unsigned bits)
{
    assert(bits==8 || bits==16 || bits==32);
    uint32_t mask=bits==32 ? UINT32_MAX : (1u<<bits)-1;
    return (old & ~mask) | (value & mask);
}
static inline void fist_cpu_alu(FistCpuState *cpu, unsigned type, unsigned bits,
                                uint32_t a, uint32_t b, uint32_t result)
{
    FistCpuFlags *f=&cpu->flags;
    f->var1=fist_cpu_low(f->var1,a,bits);
    f->var2=fist_cpu_low(f->var2,b,bits);
    f->res=fist_cpu_low(f->res,result,bits);
    f->type=type;
}
static inline unsigned fist_cpu_flag_width(const FistCpuFlags *f)
{
    switch (f->type) {
    case FIST_LAZY_CMPB: case FIST_LAZY_XORB: case FIST_LAZY_TESTB: return 8;
    case FIST_LAZY_ADDW: case FIST_LAZY_CMPW: return 16;
    case FIST_LAZY_XORD: return 32;
    default: abort();
    }
}
static inline int fist_cpu_zf(const FistCpuState *cpu)
{
    const FistCpuFlags *f=&cpu->flags;
    if (f->type==FIST_LAZY_UNKNOWN) return !!(f->flags & 0x40);
    return fist_cpu_low(0,f->res,fist_cpu_flag_width(f))==0;
}
static inline int fist_cpu_cf(const FistCpuState *cpu)
{
    const FistCpuFlags *f=&cpu->flags;
    switch (f->type) {
    case FIST_LAZY_UNKNOWN: return !!(f->flags & 1);
    case FIST_LAZY_CMPB: return (uint8_t)f->var1 < (uint8_t)f->var2;
    case FIST_LAZY_CMPW: return (uint16_t)f->var1 < (uint16_t)f->var2;
    case FIST_LAZY_ADDW: return (uint16_t)f->res < (uint16_t)f->var1;
    case FIST_LAZY_XORB: case FIST_LAZY_XORD: case FIST_LAZY_TESTB: return 0;
    default: abort();
    }
}
static inline void fist_cpu_fill_flags(FistCpuState *cpu)
{
    FistCpuFlags *f=&cpu->flags;
    if (f->type==FIST_LAZY_UNKNOWN) return;
    unsigned bits=fist_cpu_flag_width(f);
    uint32_t a=fist_cpu_low(0,f->var1,bits), b=fist_cpu_low(0,f->var2,bits);
    uint32_t result=fist_cpu_low(0,f->res,bits), sign=1u<<(bits-1);
    int arithmetic=f->type==FIST_LAZY_ADDW || f->type==FIST_LAZY_CMPB || f->type==FIST_LAZY_CMPW;
    int overflow=arithmetic && ((f->type==FIST_LAZY_ADDW ? (a^b^sign) : (a^b)) & (result^a) & sign);
    uint32_t flags=(fist_cpu_cf(cpu) ? 1u : 0u) |
        (!__builtin_parity(result & 255) ? 4u : 0u) |
        (arithmetic && ((a^b^result) & 0x10) ? 0x10u : 0u) |
        (!result ? 0x40u : 0u) | (result & sign ? 0x80u : 0u) |
        (overflow ? 0x800u : 0u);
    /* FillFlags updates exactly CF/PF/AF/ZF/SF/OF and clears the lazy tag;
     * var1/var2/res, oldcf, prev_type and every other raw flag survive. */
    f->flags=(f->flags & ~0x8d5u) | flags;
    f->type=FIST_LAZY_UNKNOWN;
}
static inline void fist_cpu_set_if(FistCpuState *cpu, int enabled)
{
    unsigned iopl=(cpu->flags.flags>>12)&3;
    assert(!cpu->pmode || (!(cpu->flags.flags & 0x20000) ? iopl>=cpu->cpl : iopl==3));
    cpu->flags.flags=(cpu->flags.flags & ~0x200u) | (enabled ? 0x200u : 0u);
}
static inline void fist_cpu_near_ret(FistCpuState *cpu, const uint8_t *memory)
{
    assert(cpu->code_big);
    uint32_t address=cpu->segments[2].base+(cpu->esp & cpu->stack_mask);
    memcpy(&cpu->eip,memory+address,4);
    cpu->esp=(cpu->esp & cpu->stack_notmask) | ((cpu->esp+4) & cpu->stack_mask);
}

/* The shared clock owns core exits. Binding transfers the caller's actual
 * context; it never initializes register/flag values or supplies guest work. */
FistCpuState *fist_clock_bind_cpu(FistCpuState *cpu);
void fist_clock_charge_cpu_instructions(unsigned count);
static inline void fist_cpu_fetch(FistCpuState *cpu, uint32_t ip)
{
    cpu->eip=ip;
    fist_clock_charge_cpu_instructions(1);
}

#endif
