#include "ghidra_compat.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t g_mem[FIST_MEM_SIZE];
extern uint32_t fist_ext_base;
static jmp_buf device_entry;
static unsigned entered;
/* Stop at the captured device-initializer boundary. Its body is not part of
 * this producer regression. Every other indirect target fails if reached. */
code *fist_icall(uint32_t address)
{
    if (address != fist_ext_base + 0x138d)
        fprintf(stderr, "unexpected indirect target %08x\n", address);
    assert(address == fist_ext_base + 0x138d);
    ++entered;
    longjmp(device_entry, 1);
}
void halt_baddata(void) { abort(); }
void fist_timer_pump(void) { abort(); }
void fist_int8_fire(void) { abort(); }
void fist_set_int8_handler(uint32_t p) { abort(); }
void fist_input_set_mouse_handler(uint32_t p, unsigned m) { abort(); }
void fist_input_mouse_state(unsigned *x, unsigned *y, unsigned *b) { abort(); }
void fist_input_mouse_setpos(unsigned x, unsigned y) { abort(); }
int fist_opl_owns(int p) { return 0; }
int fist_opl_in(int p) { abort(); }
void fist_opl_out(int p, int v) { abort(); }
int fist_sb_owns(int p) { return 0; }
int fist_sb_in(int p) { abort(); }
void fist_sb_out(int p, int v) { abort(); }
int fist_ovl_register(const char *n, uint32_t b, uint32_t s) { abort(); }
jmp_buf g_fist_exit;
volatile int g_fist_exit_code;

extern unsigned m_ext_FUN_0000_23ec(unsigned short, unsigned);
extern void m_ext_FUN_0000_76fd(unsigned, unsigned);
int main(int argc, char **argv)
{
    assert(argc == 4 || argc == 5);
    const int effects = argc == 5;
    const unsigned expected_entries = effects ? strtoul(argv[4], NULL, 0) : 1;
    const unsigned size = 0x100000, dma = 0x2de0;
    const unsigned pointers[] = {0x092f, 0x15d7, 0x15db, 0x15df,
        0x15fb, 0x15ff, 0x1603, 0x23dc, 0x23e0, 0x2716};
    fist_ext_base = 0x100000;
    uint8_t *module = g_mem + fist_ext_base;
    FILE *input = fopen(argv[1], "rb");
    assert(input && fread(module, 1, size, input) == size &&
           fread(g_mem + dma, 1, 0x800, input) == 0x800);
    assert(fgetc(input) == EOF && !fclose(input));
    for (unsigned i = 0; i < sizeof pointers / sizeof *pointers; ++i) {
        uint32_t *field = (uint32_t *)(module + pointers[i]), offset = *field;
        uint32_t linear = 0x10000000u + offset;
        uint8_t *pointer;
        if (linear >= 0x10000000u && linear - 0x10000000u < size)
            pointer = module + offset;
        else { assert(linear < FIST_MEM_SIZE); pointer = g_mem + linear; }
        *field = (uint32_t)(uintptr_t)pointer;
    }
    if (!setjmp(device_entry)) {
        if (effects) {
            /* The old-mode callback is unreachable in these original active=0
             * cases. Poison its unused C argument; this is no GP/ABI claim. */
            m_ext_FUN_0000_76fd(strtoul(argv[3], NULL, 0), 0x89ab7654u);
        } else {
            m_ext_FUN_0000_23ec(strtoul(argv[3], NULL, 0), 0);
        }
        assert(expected_entries == 0); /* Required device entry must not return. */
    }
    assert(entered == expected_entries);
    for (unsigned i = 0; i < sizeof pointers / sizeof *pointers; ++i) {
        uint32_t *field = (uint32_t *)(module + pointers[i]);
        uint32_t relative = *field - (uint32_t)(uintptr_t)g_mem;
        assert(relative < FIST_MEM_SIZE);
        *field = relative >= fist_ext_base && relative - fist_ext_base < size ?
            relative - fist_ext_base : relative - 0x10000000u;
    }
    FILE *output = fopen(argv[2], "wb");
    assert(output && fwrite(module, 1, size, output) == size &&
           fwrite(g_mem + dma, 1, 0x800, output) == 0x800 && !fclose(output));
    return 0;
}
