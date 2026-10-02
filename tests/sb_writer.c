#include "sb_clock_fixture.h"
#include <string.h>
extern uint32_t m_ext_FUN_0000_132f(uint32_t, uint16_t);
extern void fist_clock_advance_cpu_cycles(unsigned);
extern unsigned fist_clock_cpu_slice(uint64_t *);
static int trace;
int fist_writer_test_in(int port)
{
    uint64_t before, after; fist_clock_cpu_slice(&before);
    int value = in(port); unsigned remaining = fist_clock_cpu_slice(&after);
    if (trace) printf("io r %x %x %llu %llu %u\n", port, value,
                      (unsigned long long)before, (unsigned long long)after, remaining);
    return value;
}
void fist_writer_test_out(int port, int value)
{
    uint64_t before, after; fist_clock_cpu_slice(&before);
    out(port, value); unsigned remaining = fist_clock_cpu_slice(&after);
    if (trace) printf("io w %x %x %llu %llu %u\n", port, value & 255,
                      (unsigned long long)before, (unsigned long long)after, remaining);
}
int main(int argc, char **argv)
{
    assert(argc == 3);
    setenv("FIST_SB", "1", 1);
    /* Match the independently selected initial original device status counter.
     * Initialization consumes no guest instruction time inside the fixture. */
    unsigned busy = strtoul(argv[1], NULL, 10);
    for (unsigned i = 0; i < busy; ++i) fist_sb_in(0x22c);
    FILE *script = fopen(argv[2], "r"); assert(script);
    unsigned long long start; unsigned eax, port, count = 0; int fields; char op;
    while ((fields = fscanf(script, " %c", &op)) == 1) {
        if (op == 'v') {
            assert(fscanf(script, " %x %x", &port, &eax) == 2);
            fist_sb_out(port, eax); /* matched initial DSP command state */
            continue;
        }
        assert(op == 'p' && fscanf(script, " %llu %x %x", &start, &eax, &port) == 3);
        uint64_t current; fist_clock_cpu_slice(&current);
        assert(start >= current && start-current <= UINT32_MAX);
        fist_clock_advance_cpu_cycles((unsigned)(start-current));
        trace = 1;
        uint32_t result = m_ext_FUN_0000_132f(eax, port);
        trace = 0;
        unsigned remaining = fist_clock_cpu_slice(&current);
        printf("return %08x %llu %u\n", result, (unsigned long long)current, remaining);
        ++count;
    }
    assert(fields == EOF && !ferror(script) && !fclose(script) && count);
    printf("pumps %u\n", pumps);
    fist_sb_flush();
    return 0;
}
