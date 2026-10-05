/* Execute DOSBox's DoString verbatim. Only the MOVS cases are reached; unrelated
 * string-I/O/comparison routes abort so missing probe bindings cannot pass. */
#include "cpu.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
CPUBlock cpu;
#ifdef FIST_EXECUTE_SEGMENT_INPUT
static unsigned char memory[0x200000];
#else
static unsigned char memory[0x40000];
#endif
static unsigned load(unsigned address, unsigned width) {
    assert(address + width <= sizeof memory);
    unsigned value = 0;
    memcpy(&value, memory + address, width);
    return value;
}
static void save(unsigned address, unsigned value, unsigned width) {
    assert(address + width <= sizeof memory);
    memcpy(memory + address, &value, width);
}
static void unused_route(...) { abort(); }
#define LoadMb(a) load(a,1)
#define LoadMw(a) load(a,2)
#define LoadMd(a) load(a,4)
#define SaveMb(a,v) save(a,v,1)
#define SaveMw(a,v) save(a,v,2)
#define SaveMd(a,v) save(a,v,4)
#define IO_ReadB(...) (unused_route(),0u)
#define IO_ReadW(...) (unused_route(),0u)
#define IO_ReadD(...) (unused_route(),0u)
#define IO_WriteB(...) unused_route()
#define IO_WriteW(...) unused_route()
#define IO_WriteD(...) unused_route()
#define CMPB(...) unused_route()
#define CMPW(...) unused_route()
#define CMPD(...) unused_route()
#define LOG(...) unused_route
#define PREFIX_ADDR 1
#define PREFIX_REP 2
#define TEST_PREFIX_REP (core.prefixes & PREFIX_REP)
#define SegBase(s) SegPhys(s)
#define BaseDS core.base_ds
#define LOADIP core.cseip = SegBase(cs) + reg_eip
static const Bit32u AddrMaskTable[2] = {0xffff,0xffffffff};
static struct { unsigned prefixes; PhysPt base_ds,cseip; bool rep_zero; } core;
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/core_normal/string.h"

void source_rep_setup(unsigned count, unsigned width, int direction, int displacement) {
    assert(width == 1 || width == 2 || width == 4);
    for (unsigned i=0;i<sizeof memory;++i) memory[i] = (i*37+(i>>8)+11)&255;
    core.prefixes = PREFIX_ADDR | PREFIX_REP;
    core.base_ds = Segs.phys[es] = 0;
    cpu.direction = direction;
    reg_ecx = count;
    reg_esi = 0x8000 + (direction < 0 && count ? (count-1)*width : 0);
    reg_edi = reg_esi + displacement;
}
bool source_rep_step(unsigned width) {
    DoString(width == 4 ? R_MOVSD : width == 2 ? R_MOVSW : R_MOVSB);
    return reg_ecx != 0;
}
void source_rep_dump(const char *path) {
    FILE *file = fopen(path,"wb");
    assert(file && fwrite(memory,1,sizeof memory,file)==sizeof memory && !fclose(file));
}
