#ifndef FIST_VGA_MEMORY_H
#define FIST_VGA_MEMORY_H
#include "fist_ram.h"
/* VGA_Map_Handler and VGA_ChainedVGA_Handler from original vga_memory.cpp.
 * Buffers, aperture and banks are explicit captured inputs. Other VGA handlers
 * and actual port register transitions require their corresponding owners. */
typedef struct {
    FistCpuRam *bus;
    uint8_t *linear,*fastmem;
    size_t linear_size,fastmem_size;
    uint32_t wrap,base,mask,read_bank,write_bank;
} FistVgaMemory;
static inline uint32_t fist_vga_memory_load(const uint8_t *data,unsigned width)
{
    uint32_t value=0;
    for(unsigned i=0;i<width;i++)value|=(uint32_t)data[i]<<(8*i);
    return value;
}
static inline void fist_vga_memory_store(uint8_t *data,unsigned width,uint32_t value)
{
    for(unsigned i=0;i<width;i++)data[i]=(uint8_t)(value>>(8*i));
}
static inline uint32_t fist_vga_memory_address(FistVgaMemory *v,uint32_t linear,unsigned writing)
{
    FistRamPage *page=&v->bus->tlb[linear>>12];
    fist_cpu_require(page->mapped);
    uint32_t physical=(page->physical<<12)|(linear&4095u);
    return ((physical&v->mask)+(writing ? v->write_bank : v->read_bank))&(v->wrap-1);
}
static inline uint32_t fist_vga_chained_read(void *context,uint32_t linear,unsigned width)
{
    FistVgaMemory *v=context;
    uint32_t address=fist_vga_memory_address(v,linear,0);
    if(address&(width-1)) {
        uint32_t value=0;
        for(unsigned i=0;i<width;i++) {
            uint32_t offset=(((address+i)&~3u)<<2)|((address+i)&3u);
            fist_cpu_require(offset<v->linear_size);
            value|=(uint32_t)v->linear[offset]<<(8*i);
        }
        return value;
    }
    uint32_t offset=((address&~3u)<<2)|(address&3u);
    fist_cpu_require(v->linear_size>=width && offset<=v->linear_size-width);
    return fist_vga_memory_load(v->linear+offset,width);
}
static inline void fist_vga_chained_write(void *context,uint32_t linear,unsigned width,uint32_t value)
{
    FistVgaMemory *v=context;
    uint32_t address=fist_vga_memory_address(v,linear,1);
    if(address&(width-1)) {
        for(unsigned i=0;i<width;i++) {
            uint32_t offset=(((address+i)&~3u)<<2)|((address+i)&3u);
            fist_cpu_require(offset<v->linear_size);
            v->linear[offset]=(uint8_t)(value>>(8*i));
        }
    } else {
        uint32_t offset=((address&~3u)<<2)|(address&3u);
        fist_cpu_require(v->linear_size>=width && offset<=v->linear_size-width);
        fist_vga_memory_store(v->linear+offset,width,value);
    }
    fist_cpu_require(v->fastmem_size>=width && address<=v->fastmem_size-width);
    fist_vga_memory_store(v->fastmem+address,width,value);
    /* Original writeCache<Size> replicates the whole width when start<320. */
    if(address<320) {
        fist_cpu_require(v->fastmem_size>=width && address+65536<=v->fastmem_size-width);
        fist_vga_memory_store(v->fastmem+address+65536,width,value);
    }
}
static inline uint8_t *fist_vga_map_read(void *context,uint32_t physical_page)
{
    FistVgaMemory *v=context;
    uint32_t offset=(v->read_bank+(physical_page-v->base)*4096u)&(v->wrap-1);
    fist_cpu_require(v->linear_size>=4096 && offset<=v->linear_size-4096);
    return v->linear+offset;
}
static inline uint8_t *fist_vga_map_write(void *context,uint32_t physical_page)
{
    FistVgaMemory *v=context;
    uint32_t offset=(v->write_bank+(physical_page-v->base)*4096u)&(v->wrap-1);
    fist_cpu_require(v->linear_size>=4096 && offset<=v->linear_size-4096);
    return v->linear+offset;
}
#endif
