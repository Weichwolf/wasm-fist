#include "ghidra_compat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t g_mem[FIST_MEM_SIZE];
void fist_timer_pump(void) { extern void fist_clock_advance(unsigned); fist_clock_advance(1); }
static unsigned g_irqs;
static uint32_t g_loaded_size;
static unsigned g_registered;
int fist_ovl_register(const char *name, uint32_t base, uint32_t size)
{
    (void)name; (void)base;
    g_loaded_size = size;
    ++g_registered;
    return 0;
}
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
    if (argc == 4 && !strcmp(argv[1], "mz-start")) {
        extern void fist_text_init(void);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        fist_text_clock_init();
        uint32_t loaded;
        assert(!fist_load_mz(argv[2], 0, 0, &loaded));
        for (unsigned stage = 0; stage < 3; ++stage) {
            if (stage == 1) fist_text_init();
            if (stage == 2) fist_clock_charge_cpu_instructions(2);
            uint64_t cycle;
            unsigned remaining = fist_clock_cpu_slice(&cycle);
            printf("%llu %u\n", (unsigned long long)cycle, remaining);
        }
        FILE *output = fopen(argv[3], "wb");
        assert(output && fwrite(g_mem, 1, loaded, output) == loaded && !fclose(output));
        return 0;
    }
    if (argc == 7 && !strcmp(argv[1], "mz-overlay")) {
        extern void fist_text_init(void);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        fist_text_init();
        memset(g_mem, 0xa5, sizeof g_mem);
        unsigned segment = strtoul(argv[3], NULL, 0), relocation = strtoul(argv[4], NULL, 0);
        unsigned length = strtoul(argv[5], NULL, 0);
        assert(length <= FIST_MEM_SIZE);
        int result = fist_load_overlay(argv[2], segment, relocation);
        uint64_t cycle;
        unsigned remaining = fist_clock_cpu_slice(&cycle);
        FILE *output = fopen(argv[6], "wb");
        assert(output && fwrite(g_mem, 1, length, output) == length && !fclose(output));
        printf("%d %llu %u %u %u\n", result, (unsigned long long)cycle, remaining,
               g_loaded_size, g_registered);
        return 0;
    }
    if (argc == 2 && !strcmp(argv[1], "start-cpu")) {
        extern void fist_text_init(void);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        fist_text_init();
        uint64_t cycle;
        unsigned remaining = fist_clock_cpu_slice(&cycle);
        printf("%llu %u\n", (unsigned long long)cycle, remaining);
        return 0;
    }
    if ((argc == 4 && !strcmp(argv[1], "pic-slice")) ||
        (argc == 5 && (!strcmp(argv[1], "pic-retire") || !strcmp(argv[1], "pic-file-read") ||
                       !strcmp(argv[1], "pic-masked-read")))) {
        extern void fist_text_init(void), fist_clock_charge_cpu_instructions(unsigned);
        extern void fist_clock_advance_cpu_cycles(unsigned);
        extern void fist_vga_set_mode(int);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        unsigned tick = strtoul(argv[2], NULL, 10), index = strtoul(argv[3], NULL, 10);
        int file_read = strcmp(argv[1], "pic-slice") && strcmp(argv[1], "pic-retire");
        assert(tick >= (file_read ? 20u : 76u) && tick <= 10000 && index < 30000);
        fist_text_init();
        if (!file_read) fist_vga_set_mode(0x13);
        uint64_t current;
        fist_clock_cpu_slice(&current);
        uint64_t target = (uint64_t)tick * 30000u + index;
        assert(target >= current && target - current <= UINT32_MAX);
        fist_clock_advance_cpu_cycles((unsigned)(target - current));
        if (file_read) {
            if (!strcmp(argv[1], "pic-masked-read")) out(0x21, 0xfc);
            unsigned count = strtoul(argv[4], NULL, 10), mask = 0;
            while (count--) {
                mask = in(0x21);
                if (mask & 4) { mask &= 0xfb; out(0x21, mask); }
            }
            uint64_t cycle;
            unsigned slice = fist_clock_cpu_slice(&cycle);
            printf("%llu %u %u\n", (unsigned long long)cycle, slice, mask);
            return 0;
        }
        if (argc == 5) {
            const char *input = argv[4];
            do {
                char *tail;
                unsigned long count = strtoul(input, &tail, 10);
                assert(*input && tail != input && count && count <= UINT32_MAX && (!*tail || *tail == ','));
                fist_clock_charge_cpu_instructions((unsigned)count);
                if (!*tail) break;
                input = tail + 1;
            } while (1);
        }
        uint64_t cycle;
        unsigned slice = fist_clock_cpu_slice(&cycle);
        printf("%llu %u\n", (unsigned long long)cycle, slice);
        return 0;
    }
    if (argc == 5 && (!strcmp(argv[1], "pit") || !strcmp(argv[1], "pit-cpu") || !strcmp(argv[1], "pit-cpu-base"))) {
        extern void fist_clock_advance(unsigned), fist_clock_charge_cpu_instructions(unsigned);
        unsigned mode = strtoul(argv[2], NULL, 10), period = strtoul(argv[3], NULL, 10);
        unsigned elapsed = strtoul(argv[4], NULL, 10);
        assert((mode == 2 || mode == 3) && period && period <= 65536 && elapsed);
        if (!strcmp(argv[1], "pit-cpu-base")) fist_clock_charge_cpu_instructions(123);
        out(0x43, 0x30 | (mode << 1));
        out(0x40, period & 0xff);
        out(0x40, period >> 8);
        if (!strcmp(argv[1], "pit")) fist_clock_advance(elapsed - 1);
        else fist_clock_charge_cpu_instructions(elapsed);
        out(0x43, 0);
        unsigned low = in(0x40), high = in(0x40);
        printf("%u\n", low | (high << 8));
        return 0;
    }
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
            out(0x43, 0xb4);   /* channel 2, low/high bytes, mode 2 */
            out(0x42, 50000 & 0xff);
            out(0x42, 50000 >> 8);
            out(0x43, 0x80);
            int before_low = in(0x42), before_high = in(0x42);
            out(ports[i], mode ^ 3);
            out(0x43, 0x80);
            int after_low = in(0x42), after_high = in(0x42);
            if (mode & 1) printf("%u %d %u %u\n", ports[i], mode,
                                before_low | (before_high << 8), after_low | (after_high << 8));
            assert((in(0x61) & 3) == mode);
        }
    }
    return 0;
}
