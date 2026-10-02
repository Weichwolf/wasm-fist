#ifndef FIST_SB_CLOCK_FIXTURE_H
#define FIST_SB_CLOCK_FIXTURE_H
#include "ghidra_compat.h"
#include "fist_sb.h"
#include "fist_pic.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
uint8_t g_mem[FIST_MEM_SIZE];
#ifdef FIST_TEST_EXT_BASE
uint32_t fist_ext_base;
#endif
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

#endif
