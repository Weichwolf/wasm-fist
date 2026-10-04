#ifndef FIST_CPU_H
#define FIST_CPU_H
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Unsupported CPU/RAM paths must fail in release builds as well. */
static inline void fist_cpu_require(int condition)
{
    if (!condition) abort();
}

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

typedef struct {
    uint32_t idt_base, idt_limit, gdt_base, gdt_limit;
    uint32_t ldt_base, ldt_limit, ldt_value;
    uint32_t tss_base, tss_limit, tss_selector, tss_is386, tss_valid;
    uint32_t mpl, trap_skip, flag_id_toggle;
    int32_t direction;
    uint32_t lastint;
    uint32_t tss_desc_low, tss_desc_high, exception_which, exception_error;
} FistCpuSystem;

/* Original lazyflags.h enum values for the operations recovered here. */
enum {
    FIST_LAZY_UNKNOWN=0, FIST_LAZY_ADDW=2, FIST_LAZY_ADDD=3,
    FIST_LAZY_ORB=4, FIST_LAZY_ORW=5, FIST_LAZY_ORD=6,
    FIST_LAZY_ANDB=13, FIST_LAZY_ANDW=14,
    FIST_LAZY_SUBB=16, FIST_LAZY_SUBD=18,
    FIST_LAZY_XORB=19, FIST_LAZY_XORW=20,
    FIST_LAZY_XORD=21, FIST_LAZY_CMPB=22, FIST_LAZY_CMPW=23,
    FIST_LAZY_CMPD=24, FIST_LAZY_INCB=25, FIST_LAZY_INCW=26, FIST_LAZY_INCD=27,
    FIST_LAZY_DECB=28, FIST_LAZY_DECW=29, FIST_LAZY_DECD=30,
    FIST_LAZY_TESTB=31, FIST_LAZY_TESTW=32, FIST_LAZY_SHLB=34, FIST_LAZY_SHLW=35
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
    case FIST_LAZY_ANDB: case FIST_LAZY_CMPB: case FIST_LAZY_XORB:
    case FIST_LAZY_TESTB: case FIST_LAZY_ORB: case FIST_LAZY_SUBB:
    case FIST_LAZY_INCB: case FIST_LAZY_DECB: case FIST_LAZY_SHLB: return 8;
    case FIST_LAZY_ADDW: case FIST_LAZY_CMPW: case FIST_LAZY_XORW:
    case FIST_LAZY_ORW: case FIST_LAZY_ANDW: case FIST_LAZY_TESTW:
    case FIST_LAZY_INCW: case FIST_LAZY_DECW: case FIST_LAZY_SHLW: return 16;
    case FIST_LAZY_XORD: case FIST_LAZY_ADDD: case FIST_LAZY_ORD:
    case FIST_LAZY_SUBD: case FIST_LAZY_CMPD: case FIST_LAZY_INCD: case FIST_LAZY_DECD: return 32;
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
    case FIST_LAZY_SUBB: case FIST_LAZY_CMPB: return (uint8_t)f->var1 < (uint8_t)f->var2;
    case FIST_LAZY_CMPW: return (uint16_t)f->var1 < (uint16_t)f->var2;
    case FIST_LAZY_ADDW: return (uint16_t)f->res < (uint16_t)f->var1;
    case FIST_LAZY_ADDD: return f->res < f->var1;
    case FIST_LAZY_SUBD: case FIST_LAZY_CMPD: return f->var1 < f->var2;
    case FIST_LAZY_INCB: case FIST_LAZY_INCW: case FIST_LAZY_INCD:
    case FIST_LAZY_DECB: case FIST_LAZY_DECW: case FIST_LAZY_DECD: return !!(f->flags & 1);
    case FIST_LAZY_SHLB: case FIST_LAZY_SHLW: {
        unsigned bits=fist_cpu_flag_width(f), count=(uint8_t)f->var2;
        return count>bits ? 0 : (fist_cpu_low(0,f->var1,bits)>>(bits-count)) & 1;
    }
    case FIST_LAZY_XORB: case FIST_LAZY_XORD: case FIST_LAZY_TESTB:
    case FIST_LAZY_XORW: case FIST_LAZY_TESTW: case FIST_LAZY_ANDW:
    case FIST_LAZY_ANDB: case FIST_LAZY_ORB: case FIST_LAZY_ORW: case FIST_LAZY_ORD: return 0;
    default: abort();
    }
}
/* Original INC/DEC LoadCF modifies the raw CF bit before replacing the lazy
 * tag. Byte/word writes preserve upper var1/res bits; var2, oldcf and prev_type
 * remain exactly as received. The returned value has the instruction width. */
static inline uint32_t fist_cpu_incdec(FistCpuState *cpu, unsigned type, uint32_t a)
{
    fist_cpu_require((type>=FIST_LAZY_INCB && type<=FIST_LAZY_INCD) ||
                     (type>=FIST_LAZY_DECB && type<=FIST_LAZY_DECD));
    FistCpuFlags *f=&cpu->flags;
    FistCpuFlags operation={.type=type};
    unsigned bits=fist_cpu_flag_width(&operation);
    unsigned cf=fist_cpu_cf(cpu);
    f->flags=(f->flags & ~1u) | cf;
    f->var1=fist_cpu_low(f->var1,a,bits);
    f->res=fist_cpu_low(f->res,type<=FIST_LAZY_INCD ? a+1u : a-1u,bits);
    f->type=type;
    return fist_cpu_low(0,f->res,bits);
}
/* The original SHLB/SHLW macros receive the already decoded five-bit count.
 * Even a word shift writes only the byte in var2; a zero count changes nothing. */
static inline uint32_t fist_cpu_shl(FistCpuState *cpu, unsigned bits, uint32_t a, unsigned count)
{
    fist_cpu_require((bits==8 || bits==16) && count<=31);
    if (!count) return fist_cpu_low(0,a,bits);
    FistCpuFlags *f=&cpu->flags;
    a=fist_cpu_low(0,a,bits);
    f->var1=fist_cpu_low(f->var1,a,bits);
    f->var2=fist_cpu_low(f->var2,count,8);
    f->res=fist_cpu_low(f->res,a<<count,bits);
    f->type=bits==8 ? FIST_LAZY_SHLB : FIST_LAZY_SHLW;
    return fist_cpu_low(0,f->res,bits);
}
static inline void fist_cpu_fill_flags(FistCpuState *cpu)
{
    FistCpuFlags *f=&cpu->flags;
    if (f->type==FIST_LAZY_UNKNOWN) return;
    unsigned bits=fist_cpu_flag_width(f);
    uint32_t a=fist_cpu_low(0,f->var1,bits), b=fist_cpu_low(0,f->var2,bits);
    uint32_t result=fist_cpu_low(0,f->res,bits), sign=1u<<(bits-1);
    int adding=f->type==FIST_LAZY_ADDW || f->type==FIST_LAZY_ADDD;
    int arithmetic=adding || f->type==FIST_LAZY_SUBB || f->type==FIST_LAZY_CMPB || f->type==FIST_LAZY_CMPW ||
                   f->type==FIST_LAZY_SUBD || f->type==FIST_LAZY_CMPD;
    int increment=f->type>=FIST_LAZY_INCB && f->type<=FIST_LAZY_INCD;
    int decrement=f->type>=FIST_LAZY_DECB && f->type<=FIST_LAZY_DECD;
    int shift=f->type==FIST_LAZY_SHLB || f->type==FIST_LAZY_SHLW;
    int overflow=increment ? result==sign : decrement ? result==sign-1 :
        shift ? ((result^a) & sign) :
        arithmetic && ((adding ? (a^b^sign) : (a^b)) & (result^a) & sign);
    int auxiliary=increment ? !(result & 15u) : decrement ? (result & 15u)==15u :
        shift ? (f->var2 & 31u) :
        arithmetic && ((a^b^result) & 0x10);
    uint32_t flags=(fist_cpu_cf(cpu) ? 1u : 0u) |
        (!__builtin_parity(result & 255) ? 4u : 0u) |
        (auxiliary ? 0x10u : 0u) |
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
    fist_cpu_require(!cpu->pmode || (!(cpu->flags.flags & 0x20000) ? iopl>=cpu->cpl : iopl==3));
    cpu->flags.flags=(cpu->flags.flags & ~0x200u) | (enabled ? 0x200u : 0u);
}
/* The shared clock owns core exits. Binding transfers the caller's actual
 * context; it never initializes register/flag values or supplies guest work. */
FistCpuState *fist_clock_bind_cpu(FistCpuState *cpu);
void fist_clock_charge_cpu_instructions(unsigned count);
/* Original decode_end/early return: materialize flags and resume PIC at the
 * same CPU time, without the failed normal-loop fetch decrement. */
void fist_clock_cpu_core_exit(void);
/* MOV/POP SS and REP return the already charged normal-core fetch to
 * CPU_Cycles. The translated C-only SS entry still owns no charged fetch. */
void fist_clock_credit_cpu_fetch(void);
static inline void fist_cpu_fetch(FistCpuState *cpu, uint32_t ip)
{
    cpu->eip=ip;
    fist_clock_charge_cpu_instructions(1);
}

#endif
