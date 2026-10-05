#ifndef FIST_VGA_DRAW_H
#define FIST_VGA_DRAW_H
#include <stdint.h>
#include <string.h>
#include <math.h>
/* Status and drawing belong to the actual VGA device. Binding supplies state
 * owned by the machine, never a captured or synthetic initialization. */
typedef struct {
 double frame,vrstart,vrend,hstart,hend,htotal,vdend,vtotal;
 uint32_t attr,pcjr;
} FistVgaStatus;
static inline unsigned fist_vga_read_status(FistVgaStatus *v,double time)
{
 unsigned result=0;double frame=time-v->frame;
 v->attr=0;v->pcjr=0;
 if(frame>=v->vrstart && frame<=v->vrend)result|=8;
 if(frame>=v->vdend)result|=1;
 else {double line=fmod(frame,v->htotal);if(line>=v->hstart && line<=v->hend)result|=1;}
 return result;
}
/* Source: vga_draw.cpp VGA_ProcessSplit/DrawPart/VertInterrupt/DisplayStartLatch.
 * State initialization and host renderer bodies remain independent contracts. */
typedef struct {
 uint64_t resizing,width,height,blocks,address,panning,bytes_skip,linear_mask,
  address_add,line_length,address_line_total,address_line,lines_total,vblank_skip,
  lines_done,lines_scaled,split_line,parts_total,parts_lines,parts_left,
  byte_panning_shift,bpp,double_scan,doublewidth,doubleheight,blinking,mode,vret_triggered;
 double parts_delay;
 uint64_t real_start,display_start,config_bytes_skip,pel_panning,
  machine,vga_mode,attr_mode_control,vertical_retrace_end,vmemwrap;
} FistVgaDraw;
typedef struct {
 void *context;
 const uint8_t *(*line)(void *,uint64_t,uint64_t);
 void (*emit)(void *,const uint8_t *);
 void (*end)(void *,int);
 void (*add)(void *,float,uint64_t);
 void (*irq)(void *,unsigned);
} FistVgaDrawHost;
static inline const uint8_t *fist_vga_linear_line(FistVgaDraw *v,uint8_t *base,uint64_t address)
{
 uint64_t offset=address&v->linear_mask;
 if(v->linear_mask-offset<v->line_length)
  memcpy(base+v->linear_mask+1,base,v->line_length);
 return base+offset;
}
static inline void fist_vga_process_split(FistVgaDraw *v)
{
 /* DOSBox MachineType MCH_EGA=4, VGAModes M_TEXT=9. */
 if((v->attr_mode_control&0x20) || v->machine==4)v->address=0;
 else {v->address=v->byte_panning_shift*v->bytes_skip;if(v->vga_mode!=9)v->address+=v->panning;}
 v->address_line=0;
}
static inline void fist_vga_draw_part(FistVgaDraw *v,const FistVgaDrawHost *h,uint64_t lines)
{
 while(lines--) {
  const uint8_t *data=h->line(h->context,v->address,v->address_line);
  h->emit(h->context,data);
  v->address_line++;
  if(v->address_line>=v->address_line_total){v->address_line=0;v->address+=v->address_add;}
  v->lines_done++;
  if(v->split_line==v->lines_done)fist_vga_process_split(v);
 }
 if(--v->parts_left) {
  volatile float delay=(float)v->parts_delay;
  h->add(h->context,delay,v->parts_left!=1?v->parts_lines:v->lines_total-v->lines_done);
 } else h->end(h->context,0);
}
static inline void fist_vga_vert_interrupt(FistVgaDraw *v,const FistVgaDrawHost *h)
{
 if(!v->vret_triggered && (v->vertical_retrace_end&0x30)==0x10) {
  v->vret_triggered=1;if(v->machine==4)h->irq(h->context,9);
 }
}
static inline void fist_vga_display_start_latch(FistVgaDraw *v)
{v->real_start=v->display_start&(uint32_t)(v->vmemwrap-1);v->bytes_skip=v->config_bytes_skip;}
/* Device line production and host rendering are distinct original contracts.
 * Drawing callbacks use the same CPU/PIC calendar as PIT. Detachment removes
 * their owned events and retains the actual calendar. Constructor, vertical
 * frame setup and renderer/scaler bodies remain with the machine owner. */
void fist_clock_bind_vga(FistVgaDraw *,FistVgaStatus *,void *,
                         const uint8_t *(*)(void *,uint64_t,uint64_t),
                         void (*)(void *,const uint8_t *),void (*)(void *,int));
void fist_clock_vga_draw_part(unsigned value);
void fist_clock_vga_vert_interrupt(unsigned value);
void fist_clock_vga_display_start(unsigned value);
#endif
