#include "fist_dac.h"
#include "fist_render_palette.h"
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
static FistDac device;static unsigned model_mode,model_machine,model_card;
static void registered(unsigned,unsigned,unsigned,unsigned,unsigned);
static FistRenderPalette pal;
static void event(unsigned,unsigned,unsigned,unsigned);
static void set_pal(void *opaque,uint8_t entry,uint8_t red,uint8_t green,uint8_t blue) {
 fist_render_set_pal(&pal,entry,red,green,blue);
 event(entry,red,green,blue);
}
static unsigned read_port(unsigned port) {return fist_dac_read(&device,port);}
static void write_port(unsigned port,unsigned value) {fist_dac_write(&device,model_mode,NULL,set_pal,port,value);}
static void register_port(void *opaque,unsigned direction,unsigned port,unsigned mask,unsigned range) {registered(direction,port,mask,range,port-0x3c6);}
#define MACHINE model_machine
#define CARD model_card
#define SET_MACHINE(m) (model_machine=(m))
#define SET_CARD(c) (model_card=(c))
static void combine(unsigned attr,unsigned pal) {fist_dac_combine(&device,model_mode,model_machine==5,model_card==0,NULL,set_pal,attr,pal);}
static void set_entry(unsigned entry,unsigned red,unsigned green,unsigned blue) {fist_dac_set_entry(&device,NULL,set_pal,entry,red,green,blue);}
static void setup(void) {fist_dac_setup(&device,model_machine==5,model_machine==4,NULL,register_port);}
#define D device
#define P pal
#define MODE model_mode
#define SET_MODE(v) (model_mode=(v))
// Release tests retain explicit behavioral checks.
#define REQUIRE(x) do {if(!(x))abort();} while(0)
#include "cases.inc"
