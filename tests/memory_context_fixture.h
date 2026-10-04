#ifndef FIST_MEMORY_CONTEXT_FIXTURE_H
#define FIST_MEMORY_CONTEXT_FIXTURE_H
#include "fist_vga_memory.h"
#include <stdio.h>
/* Portable source fixture transport. Every mapping and cached entry is input;
 * no flat-memory, identity-first-MB or zero-system fallback is supplied. */
typedef struct {
    FistCpuRam bus;
    FistVgaMemory vga;
    FistPhysicalHandler handlers[5];
    const FistPhysicalHandler **providers;
} FistMemoryFixture;
static inline uint32_t fixture_word(FILE *f)
{
    uint32_t value;fist_cpu_require(fread(&value,sizeof value,1,f)==1);return value;
}
static inline void fixture_restore(FistMemoryFixture *f,FILE *input,
        FistCpuState *cpu,FistCpuSystem *sys,uint8_t *ram,size_t size)
{
    uint32_t h[14];fist_cpu_require(fread(h,sizeof h,1,input)==1);
    fist_cpu_require(h[1]==1 && h[2]==0 && h[3]==size/4096 && h[12]<=FIST_RAM_LINKS && h[13]<=h[12]);
    fist_ram_create(&f->bus,cpu,sys,ram,size,h[0]);
    f->vga=(FistVgaMemory){.bus=&f->bus,.linear_size=h[6],.fastmem_size=2u*h[6],
        .wrap=h[7],.base=h[10],.mask=h[11],.read_bank=h[8],.write_bank=h[9]};
    f->vga.linear=malloc(f->vga.linear_size);f->vga.fastmem=malloc(f->vga.fastmem_size);
    fist_cpu_require(f->vga.linear && f->vga.fastmem);
    f->handlers[1]=(FistPhysicalHandler){.kind=FIST_PHYSICAL_RAM,.flags=3};
    f->handlers[2]=(FistPhysicalHandler){.kind=FIST_PHYSICAL_ROM,.flags=5};
    f->handlers[3]=(FistPhysicalHandler){.kind=FIST_PHYSICAL_DEVICE,.flags=19,.context=&f->vga,
        .host_read=fist_vga_map_read,.host_write=fist_vga_map_write};
    f->handlers[4]=(FistPhysicalHandler){.kind=FIST_PHYSICAL_DEVICE,.flags=16,.context=&f->vga,
        .read=fist_vga_chained_read,.write=fist_vga_chained_write};
    uint32_t firstmb[FIST_RAM_FIRSTMB];fist_cpu_require(fread(firstmb,sizeof firstmb,1,input)==1);
    f->providers=malloc(h[3]*sizeof *f->providers);fist_cpu_require(f->providers!=NULL);
    for(unsigned i=0;i<h[3];i++) {
        uint32_t kind=fixture_word(input);fist_cpu_require(kind>0 && kind<5);
        f->providers[i]=&f->handlers[kind];
    }
    fist_ram_restore_provider(&f->bus,f->providers,h[3],firstmb,h[4],h[5]);
    fist_cpu_require(fread(f->bus.links,sizeof *f->bus.links,h[12],input)==h[12]);
    f->bus.used=h[12];
    for(unsigned i=0;i<h[13];i++) {
        uint32_t q[5];fist_cpu_require(fread(q,sizeof q,1,input)==1 && q[0]<FIST_RAM_TLB_SIZE && q[2]<5);
        FistRamPage *page=&f->bus.tlb[q[0]];
        if(q[2])fist_ram_bind_page(&f->bus,q[0],q[1],&f->handlers[q[2]]);
        else page->physical=q[1];
        fist_cpu_require(page->readable==q[3] && page->writeable==q[4]);
    }
    fist_cpu_require(fread(f->vga.linear,f->vga.linear_size,1,input)==1 &&
                     fread(f->vga.fastmem,f->vga.fastmem_size,1,input)==1);
}
static inline void fixture_cache(FistMemoryFixture *f,FILE *output)
{
    FistCpuRam *bus=&f->bus;
    FistVgaMemory *v=&f->vga;
    uint32_t header[10]={bus->architecture,bus->provider_pages,bus->a20_enabled,bus->a20_controlport,
        v->linear_size,v->wrap,v->read_bank,v->write_bank,v->base,v->mask};
    fist_cpu_require(fwrite(header,sizeof header,1,output)==1 &&
                     fwrite(bus->firstmb,sizeof bus->firstmb,1,output)==1);
    for(unsigned i=0;i<bus->provider_pages;i++) {
        unsigned kind;
        for(kind=1;kind<5;kind++)if(bus->providers[i]==&f->handlers[kind])break;
        fist_cpu_require(kind<5);
        uint32_t q[2]={kind,bus->providers[i]->flags};
        fist_cpu_require(fwrite(q,sizeof q,1,output)==1);
    }
    fist_cpu_require(fwrite(&bus->used,sizeof bus->used,1,output)==1);
    fist_cpu_require(fwrite(bus->links,sizeof *bus->links,bus->used,output)==bus->used);
    for(unsigned i=0;i<bus->used;i++) {
        uint32_t linear=bus->links[i];FistRamPage *page=&bus->tlb[linear];unsigned kind=0;
        if(page->handler)for(kind=1;kind<5;kind++)if(page->handler==&f->handlers[kind])break;
        fist_cpu_require(kind<5);
        uint32_t q[5]={linear,page->physical,kind,page->readable,page->writeable};
        fist_cpu_require(fwrite(q,sizeof q,1,output)==1);
    }
    fist_cpu_require(fwrite(f->vga.linear,f->vga.linear_size,1,output)==1 &&
                     fwrite(f->vga.fastmem,f->vga.fastmem_size,1,output)==1);
}
static inline void fixture_destroy(FistMemoryFixture *f)
{
    free(f->bus.tlb);free(f->providers);free(f->vga.linear);free(f->vga.fastmem);
}
#endif
