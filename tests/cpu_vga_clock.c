/* Reuse the one complete PIT/CPU/PIC fixture transport. Drawing effects use
 * the actual bound device and common clock; observations do not supply them. */
#define FIST_CPU_VGA_CLOCK 1
#include "fist_vga_memory.h"
#include "cpu_pit_clock.c"
#ifdef FIST_CPU_DAC_CLOCK
#include "dac_state_fixture.h"
static FistDac machine_dac;
static FistRenderPalette machine_pal;
#endif
static FistVgaDraw drawing;
static FistVgaMemory *drawing_memory;
static FILE *draw_requests;
static uint32_t drawing_base_kind,drawing_base_offset;
extern void observe_callback_state(const char *);
static void actual_draw_part(unsigned value)
{observe_callback_state("before-draw-part");fist_clock_vga_draw_part(value);observe_callback_state("after-draw-part");}
static void actual_vert_interrupt(unsigned value)
{observe_callback_state("before-vert-interrupt");fist_clock_vga_vert_interrupt(value);observe_callback_state("after-vert-interrupt");}
static void actual_panning(unsigned value)
{observe_callback_state("before-panning");fist_clock_vga_panning(value);observe_callback_state("after-panning");}
static void actual_display_start(unsigned value)
{observe_callback_state("before-display-start");fist_clock_vga_display_start(value);observe_callback_state("after-display-start");}
static void restore_drawing(FILE *input)
{
 fist_cpu_require(fread(&drawing,224,1,input)==1 && fread(&drawing.parts_delay,8,1,input)==1 && fread(&drawing.real_start,72,1,input)==1);
 drawing_base_kind=read_word(input);drawing_base_offset=read_word(input);
 g_pic_service=read_word(input);uint32_t lag=read_word(input);memcpy(&g_pic_service_lag,&lag,4);
#ifdef FIST_CPU_DAC_CLOCK
 fist_cpu_require(read_dac_state(input,&machine_dac,&machine_pal)==drawing.vga_mode);
#endif
}
static const uint8_t *drawing_line(void *context,uint64_t address,uint64_t line)
{
 uint8_t *base=(drawing_base_kind==1?drawing_memory->fastmem:drawing_memory->linear)+drawing_base_offset;
 uint32_t bytes=drawing_base_kind==1?drawing_memory->fastmem_size:drawing_memory->linear_size;
 fist_cpu_require(drawing_base_kind<=1 && (uint64_t)drawing_base_offset+drawing.linear_mask+1+drawing.line_length<=bytes);
 return fist_vga_linear_line(&drawing,base,address);
}
static void drawing_emit(void *context,const uint8_t *data)
{
 uint64_t q[]={1,drawing.address,drawing.address_line,drawing.line_length};
 fist_cpu_require(fwrite(q,sizeof q,1,draw_requests)==1 && fwrite(data,drawing.line_length,1,draw_requests)==1);
}
static void drawing_end(void *context,int abort_update)
{
 uint32_t q[]={2,(unsigned)abort_update};uint64_t cycle=clock_cpu_cycles(clock_now());
 fist_cpu_require(fwrite(q,sizeof q,1,draw_requests)==1 && fwrite(&cycle,8,1,draw_requests)==1);
}

void bind_drawing(FistVgaMemory *memory,const char *prefix)
{
 drawing_memory=memory;char path[1024];snprintf(path,sizeof path,"%s.draw-requests",prefix);
 draw_requests=fopen(path,"wb");fist_cpu_require(draw_requests!=NULL);
 fist_clock_bind_vga(&drawing,&status,NULL,drawing_line,drawing_emit,drawing_end);
#ifdef FIST_CPU_DAC_CLOCK
 fist_clock_bind_dac(&machine_dac,&machine_pal);
#endif
}
void finish_drawing(void) {
#ifdef FIST_CPU_DAC_CLOCK
 fist_clock_bind_dac(NULL,NULL);
#endif
 fist_clock_bind_vga(NULL,NULL,NULL,NULL,NULL,NULL);fist_cpu_require(!fclose(draw_requests));
}
void observe_drawing(const char *prefix,const char *kind)
{
 FILE *f=part(prefix,kind,"drawing");
 fist_cpu_require(fwrite(&drawing,224,1,f)==1 && fwrite(&drawing.parts_delay,8,1,f)==1 && fwrite(&drawing.real_start,72,1,f)==1);
 uint32_t base[]={drawing_base_kind,drawing_base_offset};
 fist_cpu_require(fwrite(base,8,1,f)==1 && !fclose(f));
 f=part(prefix,kind,"service");uint32_t q[2]={(unsigned)g_pic_service,0};memcpy(q+1,&g_pic_service_lag,4);
 fist_cpu_require(fwrite(q,8,1,f)==1 && !fclose(f));
}
#ifdef FIST_CPU_DAC_CLOCK
void write_dac_state(FILE *output) {
 write_dac_state_packet(output,&machine_dac,&machine_pal,drawing.vga_mode);
}
void observe_dac_state(const char *prefix,const char *kind) {
 FILE *f=part(prefix,kind,"dac-state");write_dac_state(f);fist_cpu_require(!fclose(f));
}
#endif
