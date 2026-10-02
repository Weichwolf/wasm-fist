#include "ghidra_compat.h"
#include "fist_sb.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

uint8_t g_mem[FIST_MEM_SIZE];
uint32_t fist_ext_base;
jmp_buf g_fist_exit;
volatile int g_fist_exit_code;
static unsigned pumps;
/* Record the parent's incorrect cooperative route. The clock budget and SB
 * device belong to the real production owners; no IRQ vector is installed. */
void fist_timer_pump(void) { extern void fist_clock_advance(unsigned); ++pumps; fist_clock_advance(1); }
void fist_int8_fire(void) {}
void fist_set_int8_handler(uint32_t p) { abort(); }
void fist_input_set_mouse_handler(uint32_t p, unsigned m) { abort(); }
void fist_input_mouse_state(unsigned *x, unsigned *y, unsigned *b) { abort(); }
void fist_input_mouse_setpos(unsigned x, unsigned y) { abort(); }
int fist_opl_owns(int p) { return 0; }
int fist_opl_in(int p) { abort(); }
void fist_opl_out(int p, int v) { abort(); }
int fist_ovl_register(const char *n, uint32_t b, uint32_t s) { abort(); }
code *fist_icall(uint32_t a) { abort(); }

int main(int argc, char **argv)
{
    assert(argc == 3);
    setenv("FIST_SB", "1", 1);
    extern void fist_clock_advance_cpu_cycles(unsigned);
    extern unsigned fist_clock_cpu_slice(uint64_t *);
    uint64_t current, start = strtoull(argv[1], NULL, 10);
    fist_clock_cpu_slice(&current);
    assert(start >= current && start-current <= UINT32_MAX);
    fist_clock_advance_cpu_cycles((unsigned)(start-current));
    FILE *script = fopen(argv[2], "r"); assert(script);
    char op; unsigned port, value, count = 0; int fields;
    while ((fields = fscanf(script, " %c %x %x", &op, &port, &value)) == 3) {
        fist_clock_charge_cpu_instructions(1); /* original normal-core fetch */
        uint64_t fetched; fist_clock_cpu_slice(&fetched);
        if (op == 'r') assert((unsigned)in(port) == value);
        else if (op == 'w') out(port, value);
        else assert(op == 'n');
        uint64_t after; unsigned remaining = fist_clock_cpu_slice(&after);
        printf("%llu %llu %u\n", (unsigned long long)fetched,
               (unsigned long long)after, remaining);
        ++count;
    }
    assert(fields == EOF && !ferror(script) && !fclose(script) && count);
    printf("pumps %u\n", pumps);
    fist_sb_flush();
    return 0;
}
