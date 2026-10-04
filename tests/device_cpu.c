#define fist_icall fist_reset_unexpected_icall
#include "sb_clock_fixture.h"
#undef fist_icall
#include "fist_cpu.h"
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
    uint32_t words[37]; memcpy(words, &cpu, sizeof words);
    printf("fetch %llu %u %u", (unsigned long long)cycle, remaining,
           30000u-(unsigned)(cycle%30000u)-remaining);
    for (unsigned i=0; i<37; ++i) printf(" %08x", words[i]);
    putchar('\n');
    ++fetches;
    fflush(stdout);
}
int main(int argc, char **argv)
{
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
    assert(argc == 5);
    unsigned entry = strtoul(argv[4], NULL, 0);
    void (*run)(FistCpuState *) = NULL;
    for (unsigned i=0; i<fist_ext_fmap_n; ++i) {
        if (i) assert(fist_ext_fmap[i-1].lin < fist_ext_fmap[i].lin);
        if (fist_ext_fmap[i].lin == entry) run=(void (*)(FistCpuState *))fist_ext_fmap[i].fn;
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
           fread(g_mem, 1, FIST_MEM_SIZE, input)==FIST_MEM_SIZE &&
           fgetc(input)==EOF && !fclose(input));
    fist_ext_base=0x100000;
    assert(cpu.eip==entry);
    FistCpuState *previous=fist_clock_bind_cpu(&cpu);
    run(&cpu);
    /* Observe the next actual caller fetch, like the original capture boundary. */
    fist_cpu_test_charge(1);
    fist_clock_bind_cpu(previous);
    printf("pumps %u fetches %u\n", pumps, fetches);
    FILE *output=fopen(argv[2], "wb");
    assert(output && fwrite(g_mem, 1, FIST_MEM_SIZE, output)==FIST_MEM_SIZE && !fclose(output));
    return 0;
}
