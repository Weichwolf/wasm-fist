#include "ghidra_compat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
uint8_t g_mem[FIST_MEM_SIZE];
extern uint32_t fist_ext_base;
unsigned char g_ext_find_cf;
jmp_buf g_fist_exit;
volatile int g_fist_exit_code;
/* No IRQ vector is installed in this isolated file-service fixture. The real
 * clock/DOS owners run; this regression does not assert interrupt or timing parity.
 * Every endpoint outside that fixture fails if unexpectedly reached. */
void fist_timer_pump(void) { abort(); }
void fist_int8_fire(void) {}
void fist_set_int8_handler(uint32_t p) { abort(); }
void fist_input_set_mouse_handler(uint32_t p,unsigned m) { abort(); }
void fist_input_mouse_state(unsigned *x,unsigned *y,unsigned *b) { abort(); }
void fist_input_mouse_setpos(unsigned x,unsigned y) { abort(); }
int fist_opl_owns(int p) { return 0; }
int fist_opl_in(int p) { abort(); }
void fist_opl_out(int p,int v) { abort(); }
int fist_sb_owns(int p) { return 0; }
int fist_sb_in(int p) { abort(); }
void fist_sb_out(int p,int v) { abort(); }
int fist_ovl_register(const char *n,uint32_t b,uint32_t s) { abort(); }
void halt_baddata(void) { abort(); }
code *fist_icall(uint32_t a) { abort(); }
extern unsigned m_ext_FUN_0000_6032(int,unsigned short,unsigned short,unsigned short,unsigned,unsigned short);
extern int g_fist_ext_int;
int main(int argc,char **argv) {
 assert(argc==5 || argc==6);
 fist_ext_base=0x100000;
 uint8_t *module=g_mem+fist_ext_base,*destination=g_mem+0x200000;
 FILE *image=fopen(argv[1],"rb");assert(image);
 size_t n=fread(module,1,0x10000,image);assert(n && !ferror(image) && feof(image) && !fclose(image));
 static const char root[1]={0};static unsigned char dta[128],tcb[0x1000];
 *(uint32_t *)(module+0x927)=argc==6 ? (uint32_t)strtoul(argv[5],0,0) : (uint32_t)(uintptr_t)dta;
 *(uint32_t *)(module+0xc93)=(uint32_t)(uintptr_t)tcb;
 *(uint32_t *)(module+0x6234)=(uint32_t)(uintptr_t)root;
 *(uint32_t *)(module+0x6238)=0;*(uint32_t *)(module+0x622c)=0;
 strcpy((char *)(module+0x85a4),argv[2]);
 unsigned size=strtoul(argv[3],0,0);assert(size+32<0x100000);
 memset(destination-16,0xa5,size+32);
 g_fist_ext_int=1;
 unsigned query=m_ext_FUN_0000_6032(0,0,0,0,(uint32_t)(uintptr_t)(module+0x85a4),0);
 printf("query %u size %u error %u\n",query,*(uint32_t *)(module+0x937),*(uint16_t *)tcb);
 for (unsigned i=0;i<size+32;i++) assert((destination-16)[i]==0xa5);
 unsigned loaded=m_ext_FUN_0000_6032((int)(uintptr_t)destination,0,0,0,(uint32_t)(uintptr_t)(module+0x85a4),0);
 printf("load %u size %u error %u handle %u\n",loaded,*(uint32_t *)(module+0x937),*(uint16_t *)tcb,*(uint16_t *)(module+0x5cc0));
 FILE *out=fopen(argv[4],"wb");assert(out && fwrite(destination-16,1,size+32,out)==size+32 && !fclose(out));
}
