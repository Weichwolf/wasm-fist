#include "ghidra_compat.h"
#include <assert.h>

uint8_t g_mem[FIST_MEM_SIZE];
void fist_timer_pump(void) { extern void fist_clock_advance(unsigned); fist_clock_advance(1); }
void fist_int8_fire(void) {}
int fist_opl_owns(int port) { (void)port; return 0; }
int fist_opl_in(int port) { (void)port; return 0; }
void fist_opl_out(int port, int value) { (void)port; (void)value; }
int fist_sb_owns(int port) { (void)port; return 0; }
int fist_sb_in(int port) { (void)port; return 0; }
void fist_sb_out(int port, int value) { (void)port; (void)value; }
extern unsigned long long fist_clock_now(void);

int main(void)
{
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
