#ifndef FIST_RENDER_PALETTE_H
#define FIST_RENDER_PALETTE_H
#include <stdint.h>
#include <string.h>
/* Original render.pal and RENDER_SetPal/Reset/Check_Palette field owners. */
typedef struct {
 struct {uint8_t red,green,blue,unused;} rgb[256];
 union {uint16_t b16[256];uint32_t b32[256];} lut;
 uint8_t modified[256],changed;
 uint64_t first,last;
} FistRenderPalette;
static void fist_render_set_pal(void *context,uint8_t entry,uint8_t red,uint8_t green,uint8_t blue)
{
 FistRenderPalette *p=context;
 p->rgb[entry].red=red;p->rgb[entry].green=green;p->rgb[entry].blue=blue;
 if(p->first>entry)p->first=entry;if(p->last<entry)p->last=entry;
}
static void fist_render_palette_reset(FistRenderPalette *p)
{
 p->first=0;p->last=255;p->changed=0;memset(p->modified,0,sizeof p->modified);
}
typedef uint32_t (*FistRenderGetRgb)(void *,uint8_t,uint8_t,uint8_t);
typedef void (*FistRenderSetPalette)(void *,uint64_t,uint64_t,const void *);
static void fist_render_check_palette(FistRenderPalette *p,unsigned out_mode,void *context,
 FistRenderGetRgb get_rgb,FistRenderSetPalette set_palette)
{
 if(p->changed){memset(p->modified,0,sizeof p->modified);p->changed=0;}
 if(p->first>p->last)return;
 if(out_mode==0)set_palette(context,p->first,p->last-p->first+1,p->rgb+p->first);
 else if(out_mode==1 || out_mode==2) {
  for(uint64_t i=p->first;i<=p->last;i++) {
   uint16_t v=get_rgb(context,p->rgb[i].red,p->rgb[i].green,p->rgb[i].blue);
   if(v!=p->lut.b16[i]){p->changed=1;p->modified[i]=1;p->lut.b16[i]=v;}
  }
 }else {
  for(uint64_t i=p->first;i<=p->last;i++) {
   uint32_t v=get_rgb(context,p->rgb[i].red,p->rgb[i].green,p->rgb[i].blue);
   if(v!=p->lut.b32[i]){p->changed=1;p->modified[i]=1;p->lut.b32[i]=v;}
  }
 }
 p->first=256;p->last=0;
}
#endif
