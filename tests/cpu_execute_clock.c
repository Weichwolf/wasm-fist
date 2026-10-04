/* Read-only diagnostics from the real production clock's actual epoch.
 * At the final zero-budget fetch, pic_index is30000 while PIC_Ticks still
 * belongs to the previous tick. CPU_CycleLeft cannot use cycle%30000. */
#include "fist_cpu.h"
#include "fist_vga.c"
void observe_cpu_clock(uint64_t *ticks,unsigned *left) {
 uint64_t cpu=clock_cpu_cycles(clock_now());
 *ticks=g_pic_tick;
 *left=30000u-(unsigned)pic_index(cpu)-g_cpu_remaining;
}

