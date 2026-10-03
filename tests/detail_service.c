#include "ghidra_compat.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t g_mem[FIST_MEM_SIZE];
jmp_buf g_fist_exit;
volatile int g_fist_exit_code;
unsigned char g_ext_find_cf;
unsigned short g_fist_ext_ecx, g_fist_ext_edx, g_fist_ext_edi;
extern uint32_t fist_ext_base;
extern int g_fist_ext_int;
int detail_service_gate(void);

/* This observation runs the actual op44 branch and real FILEMGR/DOS/clock.
 * Unexpected endpoints outside that contract fail immediately. */
void fist_timer_pump(void) { abort(); }
void fist_int8_fire(void) {}
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
void halt_baddata(void) { abort(); }
code *fist_icall(uint32_t a) { abort(); }

static unsigned commands[64], count;
void detail_service_int_dispatch(void) {
    assert(count < 64);
    commands[count++] = *(uint16_t *)(g_mem+0xf0000) >> 8;
    fist_int_dispatch();
}

static void detail_service_prepare(const char *image_name, unsigned sky, unsigned detail) {
    fist_ext_base = 0x100000;
    uint8_t *module = g_mem+fist_ext_base, *task = g_mem+0x90000;
    FILE *image = fopen(image_name, "rb");
    assert(image);
    size_t bytes = fread(module, 1, 0x10000, image);
    assert(bytes && !ferror(image) && feof(image) && !fclose(image));
    static const char empty[1] = {0};
    *(uint32_t *)(module+0xc93) = (uint32_t)(uintptr_t)task;
    *(uint32_t *)(module+0x927) = 0x80b;
    *(uint32_t *)(module+0x6234) = (uint32_t)(uintptr_t)empty;
    *(uint32_t *)(module+0x6238) = 0;
    *(uint32_t *)(module+0x622c) = 0;
    *(uint16_t *)(g_mem+0x2aa2c) = 0;
    *(uint16_t *)(g_mem+0x2aa2e) = 0x9000;
    *(uint16_t *)(g_mem+0x2aa10) = 0x44;
    task[0xcc] = sky;
    task[0xd1] = detail;
    /* A prior alternate-sky selection must be reset on the next default op44. */
    *(uint32_t *)(module+0x3958) = 0x689a;
    *(uint32_t *)(module+0x937) = 0x89ab7654;
    *(uint32_t *)(g_mem+0xf0024) = 0x89ab7654;
    memset(module+0x3a20-4, 0xa5, 2052+8);
}

int main(int argc, char **argv) {
    assert(argc == 5);
    detail_service_prepare(argv[1], strtoul(argv[2], 0, 0), strtoul(argv[3], 0, 0));
    uint8_t *module = g_mem+fist_ext_base, *task = g_mem+0x90000;
    int result = detail_service_gate();
    printf("result %u size %u ebx %u sky %u mode %u task %u flat %u\n",
           (unsigned)result, *(uint32_t *)(module+0x937),
           *(uint32_t *)(g_mem+0xf0024), *(uint32_t *)(module+0x3958),
           module[0x395c], *(uint16_t *)task, g_fist_ext_int);
    printf("dos");
    for (unsigned i=0; i<count; ++i) printf(" %02x", commands[i]);
    printf("\n");
    FILE *output = fopen(argv[4], "wb");
    assert(output && fwrite(module+0x3a20-4, 1, 2052+8, output) == 2052+8 && !fclose(output));
}
