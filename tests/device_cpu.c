#define fist_icall fist_reset_unexpected_icall
#include "sb_clock_fixture.h"
#undef fist_icall
#include "memory_context_fixture.h"
#include <string.h>
extern uint32_t fist_ext_base;
extern const struct fist_fent fist_ext_fmap[];
extern const unsigned fist_ext_fmap_n;
unsigned char g_ext_find_cf;
uint32_t g_fist_ext_edx_out, g_ext_edx;
int g_ext_eof;
uint16_t g_fist_op50_cx;
uint32_t g_fist_op50_edx, g_fist_op50_esi, g_fist_op50_edi;
void halt_baddata(void) { abort(); }
code *fist_icall_far(uint32_t address) { abort(); }
extern unsigned fist_clock_cpu_slice(uint64_t *);
extern void fist_clock_advance_cpu_cycles(unsigned);
static FistCpuState cpu;
static FistCpuSystem sys;
static FistMemoryFixture context;
static unsigned fetches;
code *fist_icall(uint32_t address)
{
    assert(address >= fist_ext_base);
    for (unsigned i=0; i<fist_ext_fmap_n; ++i)
        if (fist_ext_fmap[i].lin==address-fist_ext_base)
            return (code *)fist_ext_fmap[i].fn;
    abort();
}
_Static_assert(sizeof(FistCpuState) == 37*sizeof(uint32_t), "portable CPU record");
void fist_cpu_test_charge(unsigned count)
{
    assert(count == 1);
    fist_clock_charge_cpu_instructions(count);
    uint64_t cycle; unsigned remaining = fist_clock_cpu_slice(&cycle);
    uint32_t words[58]; memcpy(words, &cpu, sizeof cpu); memcpy(words+37,&sys,sizeof sys);
    printf("fetch %llu %u %u", (unsigned long long)cycle, remaining,
           30000u-(unsigned)(cycle%30000u)-remaining);
    for (unsigned i=0; i<58; ++i) printf(" %08x", words[i]);
    putchar('\n');
    ++fetches;
    fflush(stdout);
}
int main(int argc, char **argv)
{
    if (argc==2 && !strcmp(argv[1],"flags-full")) {
        uint32_t words[37]; size_t count;
        while ((count=fread(words,1,sizeof words,stdin))==sizeof words) {
            memcpy(&cpu,words,sizeof cpu);
            unsigned cf=fist_cpu_cf(&cpu),zf=fist_cpu_zf(&cpu);
            fist_cpu_fill_flags(&cpu);
            memcpy(words,&cpu,sizeof words);
            printf("%u %u",cf,zf);
            for (unsigned i=0;i<37;++i) printf(" %08x",words[i]);
            putchar('\n');
        }
        assert(!count && feof(stdin) && !ferror(stdin));
        return 0;
    }
    if (argc==2 && !strcmp(argv[1],"instructions")) {
        FistCpuFlags *f=&cpu.flags; int fields; unsigned a,b; char op;
        while ((fields=scanf(" %c %x %x %x %x %x %x %x %x %x",&op,&f->flags,
            &f->type,&f->prev_type,&f->oldcf,&f->var1,&f->var2,&f->res,&a,&b))==10) {
            uint32_t result;
            switch (op) {
            case 'N':break;
            case 'X':result=a^b;fist_cpu_alu(&cpu,FIST_LAZY_XORD,32,a,b,result);a=result;break;
            case 'O':result=a|b;fist_cpu_alu(&cpu,FIST_LAZY_ORD,32,a,b,result);a=result;break;
            case 'C':result=a-b;fist_cpu_alu(&cpu,FIST_LAZY_CMPD,32,a,b,result);break;
            case 'A':result=a+b;fist_cpu_alu(&cpu,FIST_LAZY_ADDD,32,a,b,result);a=result;break;
            case 'S':result=a-b;fist_cpu_alu(&cpu,FIST_LAZY_SUBD,32,a,b,result);a=result;break;
            case 'I':a=fist_cpu_incdec(&cpu,FIST_LAZY_INCD,a);break;
            case 'D':a=fist_cpu_incdec(&cpu,FIST_LAZY_DECD,a);break;
            default:abort();
            }
            printf("%08x %x %x %x %08x %08x %08x %08x %u %u\n",f->flags,
                   f->type,f->prev_type,f->oldcf,f->var1,f->var2,f->res,a,
                   fist_cpu_cf(&cpu),fist_cpu_zf(&cpu));
        }
        assert(fields==EOF && !ferror(stdin));
        return 0;
    }
    if (argc==2 && !strcmp(argv[1],"flags")) {
        FistCpuFlags *f=&cpu.flags; int fields;
        while ((fields=scanf("%x %x %x %x %x %x %x",&f->flags,&f->type,&f->prev_type,
                              &f->oldcf,&f->var1,&f->var2,&f->res))==7) {
            unsigned cf=fist_cpu_cf(&cpu),zf=fist_cpu_zf(&cpu);
            fist_cpu_fill_flags(&cpu);
            printf("%u %u %08x %x %x %x %08x %08x %08x\n",cf,zf,f->flags,
                   f->type,f->prev_type,f->oldcf,f->var1,f->var2,f->res);
        }
        assert(fields==EOF && !ferror(stdin));
        return 0;
    }
    assert(argc == 6);
    unsigned entry = strtoul(argv[4], NULL, 0);
    void (*run)(FistCpuRam *) = NULL;
    for (unsigned i=0; i<fist_ext_fmap_n; ++i) {
        if (i) assert(fist_ext_fmap[i-1].lin < fist_ext_fmap[i].lin);
        if (fist_ext_fmap[i].lin == entry) run=(void (*)(FistCpuRam *))fist_ext_fmap[i].fn;
    }
    if (!run) {
        fprintf(stderr, "missing original CPU-context device entry %04x\n", entry);
        return 1;
    }
    setenv("FIST_SB", "1", 1);
    uint64_t current, start=strtoull(argv[3], NULL, 0);
    fist_clock_cpu_slice(&current);
    assert(start >= current && start-current <= UINT32_MAX);
    fist_clock_advance_cpu_cycles((unsigned)(start-current));
    FILE *input=fopen(argv[1], "rb");
    assert(input && fread(&cpu, 1, sizeof cpu, input)==sizeof cpu &&
           fread(&sys,1,sizeof sys,input)==sizeof sys &&
           fread(g_mem, 1, FIST_MEM_SIZE, input)==FIST_MEM_SIZE);
    fixture_restore(&context,input,&cpu,&sys,g_mem,FIST_MEM_SIZE);
    assert(fgetc(input)==EOF && !fclose(input));
    fist_ext_base=0x100000;
    assert(cpu.eip==entry);
    FistCpuState *previous=fist_clock_bind_cpu(&cpu);
    run(&context.bus);
    /* Observe the next actual caller fetch, like the original capture boundary. */
    fist_cpu_test_charge(1);
    fist_clock_bind_cpu(previous);
    printf("pumps %u fetches %u\n", pumps, fetches);
    FILE *output=fopen(argv[2], "wb");
    assert(output && fwrite(g_mem, 1, FIST_MEM_SIZE, output)==FIST_MEM_SIZE && !fclose(output));
    output=fopen(argv[5],"wb");assert(output);
    fixture_cache(&context,output);assert(!fclose(output));
    fixture_destroy(&context);
    return 0;
}
