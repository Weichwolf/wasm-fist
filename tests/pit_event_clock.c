/* Observe shared-clock budgets from complete captured CPU contexts. IRQ delivery
 * and a full original caller replay require their separate integration proof. */
#define fist_int8_fire unused_fixture_int8_fire
#include "sb_clock_fixture.h"
#undef fist_int8_fire
#include "fist_cpu.h"
#include <string.h>
extern unsigned fist_clock_cpu_slice(uint64_t *);
extern void fist_clock_advance_cpu_cycles(unsigned);
static FistCpuState cpu;
static unsigned legacy_int8_calls;
_Static_assert(sizeof(FistCpuState)==37*sizeof(uint32_t),"full CPU input record");
/* Record every legacy timer delivery attempt. This fixture observes budgets;
 * it does not implement or accept the missing guest IRQ/IF/IRET transport. */
void fist_int8_fire(void) { ++legacy_int8_calls; }

int main(void)
{
    uint32_t words[37]; size_t count; unsigned cases=0;
    while ((count=fread(words,1,sizeof words,stdin))==sizeof words) {
        uint64_t at; fist_clock_cpu_slice(&at);
        uint64_t start=(at/30000u+1)*30000u-1;
        assert(start>=at && start-at<=UINT32_MAX);
        fist_clock_advance_cpu_cycles((unsigned)(start-at));
        memcpy(&cpu,words,sizeof cpu);
        FistCpuState *previous=fist_clock_bind_cpu(&cpu);
        for (unsigned step=0;step<2;++step) {
            fist_clock_charge_cpu_instructions(1);
            unsigned remaining=fist_clock_cpu_slice(&at);
            printf("%u %u %llu %u\n",cases,step,(unsigned long long)at,remaining);
        }
        fist_clock_bind_cpu(previous); ++cases;
    }
    assert(!count && feof(stdin) && !ferror(stdin));
    printf("cases %u pumps %u legacy-int8 %u\n",cases,pumps,legacy_int8_calls);
}
