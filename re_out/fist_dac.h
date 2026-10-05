#ifndef FIST_DAC_H
#define FIST_DAC_H
#include <stdint.h>
typedef struct {uint8_t red,green,blue;} FistDacRgb;
typedef struct {
 uint8_t bits,pel_mask,pel_index,state,write_index,read_index;
 uint64_t first_changed;
 uint8_t combine[16];FistDacRgb rgb[256];uint16_t xlat16[256];
} FistDac;
typedef void (*FistDacSetPal)(void *,uint8_t,uint8_t,uint8_t,uint8_t);
/* VGA_DAC_SendColor/UpdateColor and ports3c6..3c9 from the local original.
 * Modes3/5 are original enum M_VGA/M_LIN8. Renderer state belongs to set_pal. */
static void fist_dac_send(FistDac *d,void *opaque,FistDacSetPal set_pal,unsigned index,unsigned src) {
 unsigned r=d->rgb[src].red,g=d->rgb[src].green,b=d->rgb[src].blue;
 d->xlat16[index]=((b>>1)&31)|((g&63)<<5)|(((r>>1)&31)<<11);
 set_pal(opaque,index,(r<<2)|(r>>4),(g<<2)|(g>>4),(b<<2)|(b>>4));
}
static void fist_dac_update(FistDac *d,void *opaque,FistDacSetPal set_pal,unsigned index) {
 fist_dac_send(d,opaque,set_pal,index,index&d->pel_mask);
}
static void fist_dac_write(FistDac *d,unsigned mode,void *opaque,FistDacSetPal set_pal,unsigned port,unsigned value) {
 switch(port) {
 case 0x3c6:
  if(d->pel_mask!=value) {d->pel_mask=value;for(unsigned i=0;i<256;i++)fist_dac_update(d,opaque,set_pal,i);}break;
 case 0x3c7:d->read_index=value;d->pel_index=0;d->state=0;d->write_index=value+1;break;
 case 0x3c8:d->write_index=value;d->pel_index=0;d->state=1;break;
 case 0x3c9:
  value&=0x3f;
  switch(d->pel_index) {
  case 0:d->rgb[d->write_index].red=value;d->pel_index=1;break;
  case 1:d->rgb[d->write_index].green=value;d->pel_index=2;break;
  case 2:
   d->rgb[d->write_index].blue=value;
   if(mode==3||mode==5) {
    fist_dac_update(d,opaque,set_pal,d->write_index);
    if(d->pel_mask!=0xff) {
     unsigned index=d->write_index;
     if((index&d->pel_mask)==index)for(unsigned i=index+1;i<256;i++)
      if((i&d->pel_mask)==index)fist_dac_update(d,opaque,set_pal,i);
    }
   }else for(unsigned i=0;i<16;i++)if(d->combine[i]==d->write_index)fist_dac_send(d,opaque,set_pal,i,d->write_index);
   d->write_index++;d->pel_index=0;break;
  /* Release-build diagnostic path makes no state change. */
  default:break;
  }
  break;
 }
}
static unsigned fist_dac_read(FistDac *d,unsigned port) {
 switch(port) {
 case 0x3c6:return d->pel_mask;
 case 0x3c7:return d->state==0?3:0;
 case 0x3c8:return d->write_index;
 case 0x3c9:
  switch(d->pel_index) {
  case 0:d->pel_index=1;return d->rgb[d->read_index].red;
  case 1:d->pel_index=2;return d->rgb[d->read_index].green;
  case 2:{unsigned value=d->rgb[d->read_index].blue;d->read_index++;d->pel_index=0;return value;}
  default:return 0;
  }
 }
 return 0;
}
/* Original VGA_DAC_CombineColor; the caller owns machine/SVGA metadata. */
static void fist_dac_combine(FistDac *d,unsigned mode,int is_vga,int svga_none,void *opaque,FistDacSetPal set_pal,uint8_t attr,uint8_t pal) {
 d->combine[attr]=pal;
 if(mode==5)return;
 if(mode==3 && (!is_vga || !svga_none))return;
 fist_dac_send(d,opaque,set_pal,attr,pal);
}
/* Original non-VGA SetEntry deliberately sends attribute i from source i. */
static void fist_dac_set_entry(FistDac *d,void *opaque,FistDacSetPal set_pal,unsigned entry,uint8_t red,uint8_t green,uint8_t blue) {
 d->rgb[entry].red=red;d->rgb[entry].green=green;d->rgb[entry].blue=blue;
 for(unsigned i=0;i<16;i++)if(d->combine[i]==entry)fist_dac_send(d,opaque,set_pal,i,i);
}
typedef void (*FistDacRegister)(void *,unsigned,unsigned,unsigned,unsigned);
/* Original setup initializes only these fields; it retains palette/mapping/renderer state. */
static void fist_dac_setup(FistDac *d,int is_vga,int is_ega,void *opaque,FistDacRegister reg) {
 d->first_changed=256;d->bits=6;d->pel_mask=255;d->pel_index=0;d->state=0;d->read_index=0;d->write_index=0;
 if(is_vga)for(unsigned port=0x3c6;port<=0x3c9;port++) {reg(opaque,1,port,1,1);reg(opaque,0,port,1,1);}
 else if(is_ega)for(unsigned i=0;i<64;i++) {
  d->rgb[i].red=((i&4)?0x2a:0)+((i&32)?0x15:0);
  d->rgb[i].green=((i&2)?0x2a:0)+((i&16)?0x15:0);
  d->rgb[i].blue=((i&1)?0x2a:0)+((i&8)?0x15:0);
 }
}
#endif
