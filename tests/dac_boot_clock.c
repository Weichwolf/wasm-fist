/* Observe production initialization; this fixture contains no palette recipe. */
#include "fist_cpu.h"
#include "fist_vga.c"
#include "dac_state_fixture.h"
void write_boot_palette(FILE *output)
{
 fist_text_clock_init();
 write_dac_state_packet(output,g_dac_context,g_dac_render_context,g_dac_mode);
}
