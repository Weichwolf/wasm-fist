#include "sb_clock_fixture.h"
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
static struct fist_ext_device_registers registers;
static uint32_t packet_pointer, logical_pointer;
static unsigned fetches;
void fist_device_test_charge(unsigned count)
{
    assert(count == 1);
    fist_clock_charge_cpu_instructions(count);
    uint64_t cycle; unsigned remaining = fist_clock_cpu_slice(&cycle);
    uint32_t ebx = registers.ebx;
    if (fetches) { assert(ebx == packet_pointer); ebx = logical_pointer; }
    printf("fetch %llu %u %08x %08x\n", (unsigned long long)cycle,
           remaining, registers.eax, ebx);
    ++fetches;
}
int main(int argc, char **argv)
{
    assert(argc == 7);
    void (*configure)(struct fist_ext_device_registers *) = NULL;
    for (unsigned i = 0; i < fist_ext_fmap_n; ++i) {
        if (i) assert(fist_ext_fmap[i-1].lin < fist_ext_fmap[i].lin);
        if (fist_ext_fmap[i].lin == 0x1280)
            configure = (void (*)(struct fist_ext_device_registers *))fist_ext_fmap[i].fn;
    }
    if (!configure) {
        fputs("missing original 1280 device-configuration producer\n", stderr);
        return 1;
    }
    const unsigned size = 0x100000, packet = 0x90000;
    fist_ext_base = 0x100000;
    uint8_t *module = g_mem + fist_ext_base;
    FILE *input = fopen(argv[1], "rb");
    assert(input && fread(module, 1, size, input) == size &&
           fread(g_mem + packet, 1, 0x498, input) == 0x498);
    assert(fgetc(input) == EOF && !fclose(input));
    packet_pointer = (uint32_t)(uintptr_t)(g_mem + packet);
    logical_pointer = *(uint32_t *)(module + 0xc93);
    *(uint32_t *)(module + 0xc93) = packet_pointer;
    registers.eax = strtoul(argv[4], NULL, 0);
    registers.ebx = strtoul(argv[5], NULL, 0);
    uint64_t current, start = strtoull(argv[3], NULL, 0);
    fist_clock_cpu_slice(&current);
    assert(start >= current && start - current <= UINT32_MAX);
    fist_clock_advance_cpu_cycles((unsigned)(start - current));
    if (strtoul(argv[6], NULL, 0)) {
        /* A word port write must preserve these original adjacent table bytes. */
        module[0x12ce] = 0x5a; module[0x12cf] = 0xc3;
    }
    configure(&registers);
    assert(fetches == 8 && registers.ebx == packet_pointer);
    assert(*(uint32_t *)(module + 0xc93) == packet_pointer);
    *(uint32_t *)(module + 0xc93) = logical_pointer;
    unsigned remaining = fist_clock_cpu_slice(&current);
    printf("return %08x %08x %llu %u\npumps %u\n", registers.eax,
           logical_pointer, (unsigned long long)current, remaining, pumps);
    FILE *output = fopen(argv[2], "wb");
    assert(output && fwrite(module, 1, size, output) == size &&
           fwrite(g_mem + packet, 1, 0x498, output) == 0x498 && !fclose(output));
    return 0;
}
