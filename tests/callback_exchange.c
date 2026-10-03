#include "ghidra_compat.h"
#include <assert.h>

uint8_t g_mem[16*1024*1024];
unsigned char g_fist_cf;
extern undefined4 FUN_1000_46b6(undefined2, undefined2);
extern void FUN_1000_3446(void);

int main(int argc, char **argv)
{
    assert(argc == 7);
    FILE *file = fopen(argv[2], "rb");
    assert(file && fread(g_mem, 1, sizeof g_mem, file) == sizeof g_mem);
    assert(fgetc(file) == EOF && !fclose(file));
    uint16_t offset = strtoul(argv[4], 0, 0), segment = strtoul(argv[5], 0, 0);
    g_fist_cf = strtoul(argv[6], 0, 0);
    if (!strcmp(argv[1], "exchange")) {
        uint32_t old = FUN_1000_46b6(offset, segment);
        printf("old %08x cf %u\n", old, g_fist_cf);
    } else {
        assert(!strcmp(argv[1], "caller"));
        FUN_1000_3446();
        printf("caller cf %u\n", g_fist_cf);
    }
    file = fopen(argv[3], "wb");
    assert(file && fwrite(g_mem, 1, sizeof g_mem, file) == sizeof g_mem && !fclose(file));
    return 0;
}
