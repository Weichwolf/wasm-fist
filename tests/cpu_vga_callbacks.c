/* One original before-IRET seed through the complete reaching device chain. */
#include "fist_pit.h"
#ifdef FIST_VGA_MOFFS_CONTINUE
#define FIST_VGA_SHR_WORD_CONTINUE 1
#endif
#ifdef FIST_VGA_SHR_WORD_CONTINUE
#define FIST_VGA_PUSH_CS_CONTINUE 1
#endif
#define FIST_CORE_EXIT_CONTINUE 1
#define FIST_CORE_EXIT_HOOKS 1
#include "cpu_core_exit.c"
extern void bind_pit_clock(void),observe_pit_devices(const char *,const char *);
extern void observe_drawing(const char *,const char *),bind_drawing(FistVgaMemory *,const char *),finish_drawing(void);
static void core_fixture_start(void) {bind_pit_clock();bind_drawing(&context.vga,output);}
static void core_fixture_stop(void) {finish_drawing();fist_clock_bind_pit(NULL,NULL,NULL,NULL);}
static void core_fixture_observe(const char *kind) {observe_pit_devices(output,kind);observe_drawing(output,kind);}
void observe_callback_state(const char *kind) {state(kind);}
extern int in(int);
extern void out(int,int);
static uint32_t read_byte(void *opaque,unsigned port) {return in(port);}
static void write_byte(void *opaque,unsigned port,unsigned value) {out(port,value);}
static void handler_prefix(void)
{
 char path[1024];snprintf(path,sizeof path,"%s.fetches",output);
 FILE *trace=fopen(path,"w");fist_cpu_require(trace!=NULL);
 FistExec engine={.bus=&context.bus,.in=read_byte,.out=write_byte};
 const char *names[]={"pit-control","pit-low","pit-high"};
 unsigned writes=0,after=0,jcxz=0;
#ifdef FIST_VGA_PUSH_CS_CONTINUE
 unsigned pushed=0;
#ifdef FIST_VGA_SHR_WORD_CONTINUE
 unsigned after_shr=0;
#endif
#ifdef FIST_VGA_MOFFS_CONTINUE
 unsigned after_moffs=0;
#endif
#endif
 for(unsigned count=0;;count++) {
  fist_cpu_require(count<10000);observe_cpu_to(trace,"fetch");fflush(NULL);char kind[64];
  if(after) {snprintf(kind,sizeof kind,"after-%s",names[writes-1]);state(kind);after=0;}
#ifdef FIST_VGA_PUSH_CS_CONTINUE
  if(jcxz==1) {state("after-jcxz");jcxz=2;}
  if(pushed) {
   state("after-push-cs");
#ifdef FIST_VGA_SHR_WORD_CONTINUE
   pushed=0;
#else
   break;
#endif
  }
#ifdef FIST_VGA_SHR_WORD_CONTINUE
  if(after_shr) {
   state("after-shr-word-1");
#ifdef FIST_VGA_MOFFS_CONTINUE
   after_shr=0;
#else
   break;
#endif
  }
#endif
#ifdef FIST_VGA_MOFFS_CONTINUE
  if(after_moffs) {state("after-moffs-byte");break;}
#endif
#else
  if(jcxz) {state("after-jcxz");break;}
#endif
  unsigned op=fist_ram_resident_read(&context.bus,1,cpu.eip,1);
#ifdef FIST_VGA_PUSH_CS_CONTINUE
  if(op==0x0e) {state("before-push-cs");pushed=1;}
#endif
#ifdef FIST_VGA_MOFFS_CONTINUE
  if(cpu.segments[1].value==0x2082 && cpu.eip==0x3b43) {state("before-moffs-byte");after_moffs=1;}
#endif
#ifdef FIST_VGA_SHR_WORD_CONTINUE
  if(cpu.segments[1].value==0x2082 && cpu.eip==0x3b38) {state("before-shr-word-1");after_shr=1;}
#endif
  if(op==0xe6) {
   fist_cpu_require(writes<3);snprintf(kind,sizeof kind,"before-%s",names[writes]);state(kind);writes++;after=1;
  }
  if(op==0xe3) {fist_cpu_require(writes==3);state("before-jcxz");jcxz=1;}
  unsigned checks=fist_exec_fetched(&engine),trap_decoder=0;
  fist_cpu_require(!fist_exec_core_exit(&engine,checks,fist_pic_pending_irqs(),&trap_decoder));
  fist_clock_charge_cpu_instructions(1);
 }
 fist_cpu_require(writes==3 && jcxz && !fclose(trace));
}
