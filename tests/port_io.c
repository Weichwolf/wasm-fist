#include "ghidra_compat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t g_mem[FIST_MEM_SIZE];
void fist_timer_pump(void) { extern void fist_clock_advance(unsigned); fist_clock_advance(1); }
static unsigned g_irqs;
void fist_int8_fire(void) { ++g_irqs; }
int fist_opl_owns(int port) { (void)port; return 0; }
int fist_opl_in(int port) { (void)port; return 0; }
void fist_opl_out(int port, int value) { (void)port; (void)value; }
int fist_sb_owns(int port) { (void)port; return 0; }
int fist_sb_in(int port) { (void)port; return 0; }
void fist_sb_out(int port, int value) { (void)port; (void)value; }
extern unsigned long long fist_clock_now(void);
static unsigned long long g_end_clock;
static void check_endpoint(void)
{
    assert(fist_clock_now() == g_end_clock);
    assert(g_irqs == (g_end_clock - 1) / 0x10000);
}

int main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "sequence")) {
        extern void fist_text_init(void), fist_clock_advance(unsigned);
        g_end_clock = (strtoull(getenv("FIST_SEQUENCE_END_MS"), NULL, 10) * 1193182u + 999) / 1000;
        atexit(check_endpoint);
        fist_text_init();
        fist_clock_advance(4 * 1193182u);
        return 42;
    }
    const int ports[] = {0x3c0, 0x3c1, 0x3c2, 0x3c3, 0x3c4, 0x3c5, 0x3ce, 0x3cf,
                         0x3d4, 0x3d5, 0x3d8, 0x3d9, 0x20, 0xa0, 0x21, 0xa1, 0x64};
    for (unsigned i = 0; i < sizeof ports / sizeof *ports; ++i) {
        for (int mode = 0; mode < 4; ++mode) {
            out(0x61, mode);
            assert((in(0x61) & 3) == mode);
            out(0x43, 0x90);
            out(0x42, 200);
            int before = in(0x42);
            unsigned long long time = fist_clock_now();
            out(ports[i], mode ^ 3);
            int after = in(0x42);
            if (mode & 1) assert(after == before - (int)(fist_clock_now() - time));
            assert((in(0x61) & 3) == mode);
        }
    }
    return 0;
}
