#include "ghidra_compat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <unistd.h>
uint8_t g_mem[FIST_MEM_SIZE];
extern uint32_t fist_ext_base;
unsigned char g_ext_find_cf;
jmp_buf g_fist_exit;
volatile int g_fist_exit_code;
/* No IRQ vector is installed in this isolated file-service observation.
 * Endpoints outside that contract fail if unexpectedly reached. */
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
static unsigned commands[64],count,size;
static int opening_failure;
static const char *output;
static unsigned char tcb[0x1000];
void file_cf_int_dispatch(void) {
 unsigned command=*(uint16_t *)(g_mem+0xf0000)>>8;
 assert(count<64);commands[count++]=command;
 fist_int_dispatch(); /* The real DOS/kernel/clock owner supplies every I/O result. */
 if (opening_failure && command==0x4e && !*(uint16_t *)(g_mem+0xf0012)) {
  char path[1024];assert(snprintf(path,sizeof path,"%s/FISTDATA/HIGH.DTL",getenv("FIST_DATADIR"))<sizeof path);
  assert(unlink(path)==0); /* Same controlled file absence as original6065. */
 }
}
void file_cf_error_entry(unsigned reason) {
 uint8_t *module=g_mem+fist_ext_base;
 printf("error reason %u size %u cf %u task %u\n",reason,*(uint32_t *)(module+0x937),g_ext_find_cf,*(uint16_t *)tcb);
 printf("dos");for(unsigned i=0;i<count;i++)printf(" %02x",commands[i]);printf("\n");
 FILE *out=fopen(output,"wb");assert(out && fwrite(g_mem+0x200000-16,1,size+32,out)==size+32 && !fclose(out));
 char task_output[1024];assert(snprintf(task_output,sizeof task_output,"%s.task",output)<sizeof task_output);
 out=fopen(task_output,"wb");assert(out && fwrite(tcb,1,sizeof tcb,out)==sizeof tcb && !fclose(out));
 /* The test adapter selects original at-0f64 or post-store at-f57.
  * No I/O/CF/store result is supplied. Nonlocal exit, caller continuation
  * and timing remain outside this observation. */
 exit(0);
}
int main(int argc,char **argv) {
 assert(argc==8);fist_ext_base=0x100000;
 uint8_t *module=g_mem+fist_ext_base,*destination=g_mem+0x200000;
 FILE *image=fopen(argv[1],"rb");assert(image);
 size_t n=fread(module,1,0x10000,image);assert(n && !ferror(image) && feof(image) && !fclose(image));
 static const char root[1]={0};
 *(uint32_t *)(module+0x927)=(uint32_t)strtoul(argv[5],0,0);
 *(uint32_t *)(module+0xc93)=(uint32_t)(uintptr_t)tcb;
 *(uint32_t *)(module+0x6234)=(uint32_t)(uintptr_t)root;
 *(uint32_t *)(module+0x6238)=0;*(uint32_t *)(module+0x622c)=0;
 *(uint32_t *)(module+0x937)=(uint32_t)strtoul(argv[7],0,0);
 strcpy((char *)(module+0x85a4),argv[2]);
 size=strtoul(argv[3],0,0);assert(size+32<0x100000);output=argv[4];opening_failure=!strcmp(argv[6],"open");
 /* Original5ce5 reads TCB+496 as an alternate drive. Keep real inputs zero;
  * guard only the two adjacent bytes against an over-wide status store. */
 memset(tcb,0,sizeof tcb);memset(tcb+2,0xa5,2);
 memset(destination-16,0xa5,size+32);g_fist_ext_int=1;
 m_ext_FUN_0000_6032(opening_failure?(int)(uintptr_t)destination:0,0,0,0,(uint32_t)(uintptr_t)(module+0x85a4),0);
 fputs("FAIL: file query returned instead of reaching the original error boundary\n",stderr);
 return 1;
}
