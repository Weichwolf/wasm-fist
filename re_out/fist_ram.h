#ifndef FIST_RAM_H
#define FIST_RAM_H
#include "fist_cpu.h"
/* Original MIXED/386FAST fully-linked paths with explicit physical providers.
 * Provider maps and first-MB state are required inputs, never inferred from
 * buffer size. Device callbacks keep original width and linear-address inputs.
 * Other architectures and guest faults still require their actual owners. */
enum { FIST_RAM_TLB_SIZE=1024*1024, FIST_RAM_LINKS=128*1024/4,
       FIST_RAM_FIRSTMB=(1024+64)/4, FIST_ARCH_MIXED=0xff,
       FIST_ARCH_386FAST=0x35 };
enum { FIST_PHYSICAL_RAM=1, FIST_PHYSICAL_ROM=2, FIST_PHYSICAL_DEVICE=3,
       FIST_PHYSICAL_READABLE=1, FIST_PHYSICAL_WRITEABLE=2, FIST_PHYSICAL_HASROM=4 };
typedef struct {
    unsigned kind,flags;
    void *context;
    uint8_t *(*host_read)(void *,uint32_t);
    uint8_t *(*host_write)(void *,uint32_t);
    uint32_t (*read)(void *,uint32_t,unsigned);
    void (*write)(void *,uint32_t,unsigned,uint32_t);
} FistPhysicalHandler;
typedef struct {
    uint32_t physical;
    unsigned readable,writeable,mapped;
    const FistPhysicalHandler *handler;
    uint8_t *read,*write;
} FistRamPage;
typedef struct {
    FistCpuState *cpu;
    FistCpuSystem *system;
    uint8_t *ram;
    size_t size;
    unsigned architecture;
    FistRamPage *tlb;
    uint32_t links[FIST_RAM_LINKS],used;
    uint32_t firstmb[FIST_RAM_FIRSTMB];
    const FistPhysicalHandler *const *providers;
    uint32_t provider_pages;
    unsigned a20_enabled,a20_controlport;
} FistCpuRam;
static inline void fist_ram_restore_provider(FistCpuRam *bus,
        const FistPhysicalHandler *const *providers,uint32_t pages,
        const uint32_t firstmb[FIST_RAM_FIRSTMB],unsigned a20_enabled,unsigned controlport)
{
    fist_cpu_require(providers!=NULL && pages==bus->size/4096 && controlport<256);
    bus->providers=providers;bus->provider_pages=pages;
    memcpy(bus->firstmb,firstmb,sizeof bus->firstmb);
    bus->a20_enabled=a20_enabled;bus->a20_controlport=controlport;
}
static inline uint32_t fist_ram_physical_dword(FistCpuRam *bus,uint32_t address)
{
    fist_cpu_require(bus->size>=4 && address<=bus->size-4);
    return (uint32_t)bus->ram[address] | ((uint32_t)bus->ram[address+1]<<8) |
           ((uint32_t)bus->ram[address+2]<<16) | ((uint32_t)bus->ram[address+3]<<24);
}
static inline void fist_ram_physical_write_dword(FistCpuRam *bus,uint32_t address,uint32_t value)
{
    fist_cpu_require(bus->size>=4 && address<=bus->size-4);
    for(unsigned i=0;i<4;i++)bus->ram[address+i]=(uint8_t)(value>>(8*i));
}
static inline void fist_ram_clear_tlb(FistCpuRam *bus)
{
    for(unsigned i=0;i<bus->used;i++) {
        FistRamPage *page=&bus->tlb[bus->links[i]];
        page->readable=0;page->writeable=0;
        page->mapped=0;page->handler=NULL;page->read=NULL;page->write=NULL;
    }
    bus->used=0;
}
static inline void fist_ram_create(FistCpuRam *bus,FistCpuState *cpu,FistCpuSystem *system,
        uint8_t *ram,size_t size,unsigned architecture)
{
    memset(bus,0,sizeof *bus);
    bus->cpu=cpu;bus->system=system;bus->ram=ram;bus->size=size;bus->architecture=architecture;
    bus->tlb=calloc(FIST_RAM_TLB_SIZE,sizeof *bus->tlb);fist_cpu_require(bus->tlb!=NULL);
}
static inline void fist_ram_bind_page(FistCpuRam *bus,uint32_t linear,uint32_t physical,
        const FistPhysicalHandler *handler)
{
    fist_cpu_require(linear<FIST_RAM_TLB_SIZE && physical<FIST_RAM_TLB_SIZE);
    FistRamPage *page=&bus->tlb[linear];
    fist_cpu_require(physical<bus->size/4096);
    fist_cpu_require(handler!=NULL);
    page->physical=physical;page->handler=handler;page->read=NULL;page->write=NULL;
    if(handler->kind==FIST_PHYSICAL_RAM || handler->kind==FIST_PHYSICAL_ROM) {
        unsigned flags=FIST_PHYSICAL_READABLE | (handler->kind==FIST_PHYSICAL_RAM ?
            FIST_PHYSICAL_WRITEABLE : FIST_PHYSICAL_HASROM);
        fist_cpu_require(handler->flags==flags);
        page->read=bus->ram+((size_t)physical<<12);
        if(handler->kind==FIST_PHYSICAL_RAM)page->write=page->read;
    } else {
        fist_cpu_require(handler->kind==FIST_PHYSICAL_DEVICE);
        if(handler->flags&FIST_PHYSICAL_READABLE) {
            fist_cpu_require(handler->host_read!=NULL);
            page->read=handler->host_read(handler->context,physical);
            fist_cpu_require(page->read!=NULL);
        }
        if(handler->flags&FIST_PHYSICAL_WRITEABLE) {
            fist_cpu_require(handler->host_write!=NULL);
            page->write=handler->host_write(handler->context,physical);
            fist_cpu_require(page->write!=NULL);
        }
    }
    page->readable=page->read!=NULL;page->writeable=page->write!=NULL;page->mapped=1;
}
static inline void fist_ram_link_page(FistCpuRam *bus,uint32_t linear,uint32_t physical)
{
    fist_cpu_require(bus->providers!=NULL && physical<bus->provider_pages);
    if(bus->used>=FIST_RAM_LINKS)fist_ram_clear_tlb(bus);
    fist_ram_bind_page(bus,linear,physical,bus->providers[physical]);
    bus->links[bus->used++]=linear;
}
static inline unsigned fist_ram_init_page(FistCpuRam *bus,uint32_t linear,int writing)
{
    uint32_t page=linear>>12,physical;
    if(bus->cpu->paging_enabled) {
        fist_cpu_require(bus->architecture==FIST_ARCH_MIXED || bus->architecture==FIST_ARCH_386FAST);
        uint32_t directory=(bus->cpu->cr3&0xfffff000u)+4*(page>>10);
        uint32_t table=fist_ram_physical_dword(bus,directory);
        fist_cpu_require(table&1);
        uint32_t entry_address=(table&0xfffff000u)+4*(page&1023u);
        uint32_t entry=fist_ram_physical_dword(bus,entry_address);
        fist_cpu_require(entry&1);
        unsigned user=(bus->cpu->cpl&bus->system->mpl)==3;
        /* Original MIXED/386 user check combines both US bits with AND. */
        fist_cpu_require(!user || (table&4) || (entry&4));
        fist_cpu_require(!writing || !user || ((table&2) && (entry&2)));
        if(!(table&0x20))fist_ram_physical_write_dword(bus,directory,table|0x20);
        /* Original priv_check==0 marks even reads dirty before linking handlers. */
        if((entry&0x60)!=0x60)fist_ram_physical_write_dword(bus,entry_address,entry|0x60);
        physical=entry>>12;
    } else physical=page<FIST_RAM_FIRSTMB ? bus->firstmb[page] : page;
    fist_ram_link_page(bus,page,physical);
    return 0;
}
static inline uint32_t fist_ram_address(FistCpuRam *bus,uint32_t linear,int writing)
{
    FistRamPage *page=&bus->tlb[linear>>12];
    if(!page->mapped)fist_ram_init_page(bus,linear,writing);
    fist_cpu_require(writing ? page->writeable : page->readable);
    uint32_t physical=(page->physical<<12)|(linear&4095u);
    fist_cpu_require(physical<bus->size);
    return physical;
}
static inline uint32_t fist_ram_read(FistCpuRam *bus,uint32_t linear,unsigned width)
{
    fist_cpu_require(width==1 || width==2 || width==4);
    if((linear&4095u)+width>4096u) {
        uint32_t value=0;
        for(unsigned i=0;i<width;i++)value|=fist_ram_read(bus,linear+i,1)<<(8*i);
        return value;
    }
    FistRamPage *page=&bus->tlb[linear>>12];
    if(!page->mapped)fist_ram_init_page(bus,linear,0);
    if(page->read) {
        uint32_t value=0;const uint8_t *p=page->read+(linear&4095u);
        for(unsigned i=0;i<width;i++)value|=(uint32_t)p[i]<<(8*i);
        return value;
    }
    fist_cpu_require(page->handler!=NULL && page->handler->read!=NULL);
    uint32_t value=page->handler->read(page->handler->context,linear,width);
    return width==1 ? (uint8_t)value : width==2 ? (uint16_t)value : value;
}
static inline void fist_ram_write(FistCpuRam *bus,uint32_t linear,unsigned width,uint32_t value)
{
    fist_cpu_require(width==1 || width==2 || width==4);
    if((linear&4095u)+width>4096u) {
        for(unsigned i=0;i<width;i++)fist_ram_write(bus,linear+i,1,(uint8_t)(value>>(8*i)));
        return;
    }
    FistRamPage *page=&bus->tlb[linear>>12];
    if(!page->mapped)fist_ram_init_page(bus,linear,1);
    if(page->write) {
        uint8_t *p=page->write+(linear&4095u);
        for(unsigned i=0;i<width;i++)p[i]=(uint8_t)(value>>(8*i));
        return;
    }
    fist_cpu_require(page->handler!=NULL);
    if(page->handler->kind==FIST_PHYSICAL_ROM)return;
    fist_cpu_require(page->handler->write!=NULL);
    page->handler->write(page->handler->context,linear,width,
                        width==1 ? (uint8_t)value : width==2 ? (uint16_t)value : value);
}
static inline uint32_t fist_ram_resident_read(FistCpuRam *bus,unsigned segment,uint32_t offset,unsigned width)
{
    fist_cpu_require(segment<6);
    return fist_ram_read(bus,bus->cpu->segments[segment].base+offset,width);
}
static inline void fist_ram_resident_write(FistCpuRam *bus,unsigned segment,uint32_t offset,unsigned width,uint32_t value)
{
    fist_cpu_require(segment<6 && (width==1 || width==2 || width==4));
    uint32_t linear=bus->cpu->segments[segment].base+offset;
    fist_ram_write(bus,linear,width,value);
}
static inline void fist_ram_map_page(FistCpuRam *bus,uint32_t linear,uint32_t physical)
{
    fist_cpu_require(linear<FIST_RAM_TLB_SIZE && physical<FIST_RAM_TLB_SIZE);
    if(linear<FIST_RAM_FIRSTMB) {
        bus->firstmb[linear]=physical;
        FistRamPage *page=&bus->tlb[linear];
        page->readable=0;page->writeable=0;page->mapped=0;
        page->handler=NULL;page->read=NULL;page->write=NULL;
    } else fist_ram_link_page(bus,linear,physical);
}
static inline void fist_ram_a20_enable(FistCpuRam *bus,unsigned enabled)
{
    uint32_t physical=enabled ? 256 : 0;
    for(unsigned i=0;i<16;i++)fist_ram_map_page(bus,256+i,physical+i);
    bus->a20_enabled=enabled!=0;
}
static inline void fist_ram_set_cr3(FistCpuRam *bus,uint32_t value)
{
    bus->cpu->cr3=value;
    if(bus->cpu->paging_enabled)fist_ram_clear_tlb(bus);
}
static inline void fist_ram_set_cr0_normal(FistCpuRam *bus,uint32_t value)
{
    if(bus->cpu->cr0==value)return;
    bus->cpu->cr0=value;bus->cpu->pmode=(value&1)!=0;
    unsigned enabled=bus->cpu->pmode && (value&0x80000000u)!=0;
    if(enabled!=bus->cpu->paging_enabled) {
        bus->cpu->paging_enabled=enabled;
        if(enabled)fist_ram_set_cr3(bus,bus->cpu->cr3);
        fist_ram_clear_tlb(bus);
    }
}
static inline void fist_ram_near_ret(FistCpuRam *bus)
{
    FistCpuState *cpu=bus->cpu;
    fist_cpu_require(cpu->code_big);
    cpu->eip=fist_ram_resident_read(bus,2,cpu->esp&cpu->stack_mask,4);
    cpu->esp=(cpu->esp&cpu->stack_notmask)|((cpu->esp+4)&cpu->stack_mask);
}
static inline void fist_ram_opcode_fetch(FistCpuRam *bus,uint32_t ip)
{
    /* core_normal observes the fetch boundary before its opcode Fetchb.
     * Translated producers must perform that actual CS read too; otherwise a
     * cold code page never enters the same shared cache as operand accesses. */
    fist_cpu_fetch(bus->cpu,ip);
    (void)fist_ram_resident_read(bus,1,ip,1);
}
#endif
