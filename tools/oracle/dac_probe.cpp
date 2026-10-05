#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include "dosbox.h"
#include "render.h"
#include "vga.h"
VGA_Type vga;Render_t render;MachineType machine;SVGACards svgaCard;
static void registered(unsigned,unsigned,unsigned,unsigned,unsigned);
static void event(unsigned,unsigned,unsigned,unsigned);
// Diagnostic environment knobs are disabled in this device-only source fixture.
#define getenv(...) ((char *)0)
#include "@DOSBOX_DAC@"
#undef getenv
@RENDER_SET_PAL@
static unsigned read_port(unsigned port) {
 switch(port) {case 0x3c6:return read_p3c6(port,1);case 0x3c7:return read_p3c7(port,1);case 0x3c8:return read_p3c8(port,1);case 0x3c9:return read_p3c9(port,1);default:abort();}
}
static void write_port(unsigned port,unsigned value) {
 switch(port) {case 0x3c6:write_p3c6(port,value,1);break;case 0x3c7:write_p3c7(port,value,1);break;case 0x3c8:write_p3c8(port,value,1);break;case 0x3c9:write_p3c9(port,value,1);break;default:abort();}
}
void IO_RegisterWriteHandler(Bitu port,IO_WriteHandler *handler,Bitu mask,Bitu range) {
 IO_WriteHandler *wanted[]={write_p3c6,write_p3c7,write_p3c8,write_p3c9};
 assert(port>=0x3c6 && port<=0x3c9 && handler==wanted[port-0x3c6]);registered(1,port,mask,range,port-0x3c6);
}
void IO_RegisterReadHandler(Bitu port,IO_ReadHandler *handler,Bitu mask,Bitu range) {
 IO_ReadHandler *wanted[]={read_p3c6,read_p3c7,read_p3c8,read_p3c9};
 assert(port>=0x3c6 && port<=0x3c9 && handler==wanted[port-0x3c6]);registered(0,port,mask,range,port-0x3c6);
}
#define MACHINE machine
#define CARD svgaCard
static void set_machine(unsigned value) {machine=(MachineType)value;}
static void set_card(unsigned value) {svgaCard=(SVGACards)value;}
#define SET_MACHINE(m) set_machine(m)
#define SET_CARD(c) set_card(c)
static void combine(unsigned attr,unsigned pal) {VGA_DAC_CombineColor(attr,pal);}
static void set_entry(unsigned entry,unsigned red,unsigned green,unsigned blue) {VGA_DAC_SetEntry(entry,red,green,blue);}
static void setup(void) {VGA_SetupDAC();}
#define D vga.dac
#define P render.pal
#define MODE vga.mode
#define SET_MODE(v) (vga.mode=(VGAModes)(v))
#define REQUIRE assert
#include "cases.inc"
