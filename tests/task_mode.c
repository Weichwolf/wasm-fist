#include "ghidra_compat.h"
#include <assert.h>
uint8_t g_mem[16*1024*1024];
extern uint8_t FUN_1000_23ba(void), FUN_1000_23bf(uint8_t);
extern void observed_task_calls(void);
static unsigned calls;
code *fist_icall_far(uint32_t address)
{
    ++calls;
    if (address == 0x0f692d2a) return (code *)FUN_1000_23ba;
    if (address == 0x0f692d2f) return (code *)FUN_1000_23bf;
    abort();
}
int main(int argc, char **argv)
{
    assert(argc == 5);
    FILE *f = fopen(argv[2], "rb"); assert(f);
    assert(fread(g_mem, 1, sizeof g_mem, f) == sizeof g_mem);
    assert(fgetc(f) == EOF && !fclose(f));
    unsigned mode_address = strtoul(argv[4], 0, 0);
    assert(mode_address < sizeof g_mem);
    if (!strcmp(argv[1], "calls")) {
        observed_task_calls();
        printf("calls %u mode %02x\n", calls, g_mem[mode_address]);
    } else {
        assert(!strcmp(argv[1], "exchange"));
        uint8_t value = FUN_1000_23bf(0xa5);
        printf("return %02x mode %02x\n", value, g_mem[mode_address]);
    }
    f = fopen(argv[3], "wb"); assert(f);
    assert(fwrite(g_mem, 1, sizeof g_mem, f) == sizeof g_mem && !fclose(f));
    return 0;
}
