/* Complete hardware/renderer palette wire contract, independent of C padding. */
#ifndef FIST_DAC_STATE_FIXTURE_H
#define FIST_DAC_STATE_FIXTURE_H
#include "fist_dac.h"
#include "fist_render_palette.h"
#include <stdio.h>
#define DAC_STATE_READ(v) fist_cpu_require(fread(&(v),sizeof(v),1,input)==1)
static unsigned read_dac_state(FILE *input,FistDac *d,FistRenderPalette *p)
{
 fist_cpu_require(fread(&d->bits,6,1,input)==1);
 DAC_STATE_READ(d->first_changed);DAC_STATE_READ(d->combine);
 DAC_STATE_READ(d->rgb);DAC_STATE_READ(d->xlat16);
 uint32_t mode;DAC_STATE_READ(mode);
 DAC_STATE_READ(p->rgb);DAC_STATE_READ(p->lut);DAC_STATE_READ(p->modified);
 DAC_STATE_READ(p->changed);DAC_STATE_READ(p->first);DAC_STATE_READ(p->last);
 return mode;
}
#undef DAC_STATE_READ
#define DAC_STATE_WRITE(v) fist_cpu_require(fwrite(&(v),sizeof(v),1,output)==1)
static void write_dac_state_packet(FILE *output,const FistDac *d,const FistRenderPalette *p,uint32_t mode)
{
 fist_cpu_require(fwrite(&d->bits,6,1,output)==1);
 DAC_STATE_WRITE(d->first_changed);DAC_STATE_WRITE(d->combine);
 DAC_STATE_WRITE(d->rgb);DAC_STATE_WRITE(d->xlat16);DAC_STATE_WRITE(mode);
 DAC_STATE_WRITE(p->rgb);DAC_STATE_WRITE(p->lut);DAC_STATE_WRITE(p->modified);
 DAC_STATE_WRITE(p->changed);DAC_STATE_WRITE(p->first);DAC_STATE_WRITE(p->last);
}
#undef DAC_STATE_WRITE
#endif
