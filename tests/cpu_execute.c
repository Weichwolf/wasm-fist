/* Continuous actual-byte startup regression. Captured state is test input only.
 * Original host DOS services and observation/stopping remain test adapters. */
#include "fist_interrupt.h"
#include "fist_dos_cpu.h"
#define fist_int8_fire fist_cpu_execute_unbound_int8
#include "sb_clock_fixture.h"
#undef fist_int8_fire
static unsigned replay_running;
void fist_int8_fire(void) { fist_cpu_require(!replay_running); }
extern unsigned fist_clock_cpu_slice(uint64_t *);
extern void observe_cpu_clock(uint64_t *,unsigned *);
extern void fist_clock_credit_cpu_fetch(void);
extern void fist_clock_advance_cpu_cycles(unsigned);
#include "memory_context_fixture.h"
#include "source_layout.h"
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
static FistCpuState cpu;
static FistCpuSystem sys;
static FistMemoryFixture context;
static unsigned remaining;
static const char *output;
static int find_active;
static FistDosFindHost host;
#define host_next_free host.next_free
#define host_drive host.drive
#define host_occupied host.occupied
#define host_name_prior host.name_prior
#define host_directory host.directory
#include "fist_exec.h"
#include "fist_dos_cpu.h"
static FistExec engine;
static uint32_t reg_read(unsigned i,unsigned width) { return fist_exec_reg_read(&engine,i,width); }
static void reg_write(unsigned i,unsigned width,uint32_t value) { fist_exec_reg_write(&engine,i,width,value); }
static void state(const char *kind) {
 uint32_t q[58];memcpy(q,&cpu,sizeof cpu);memcpy(q+37,&sys,sizeof sys);
 uint64_t cycle;remaining=fist_clock_cpu_slice(&cycle);
 uint64_t ticks;unsigned left;observe_cpu_clock(&ticks,&left);
 printf("%s %llu %llu %u %u",kind,(unsigned long long)cycle,(unsigned long long)ticks,remaining,left);for(unsigned i=0;i<58;i++)printf(" %08x",q[i]);puts("");fflush(stdout);
}
static void snapshot(const char *label) {
 char name[1024];snprintf(name,sizeof name,"%s-%s.memory",output,label);FILE *f=fopen(name,"wb");
 fist_cpu_require(f && fwrite(context.bus.ram,16777216,1,f)==1 && !fclose(f));
 snprintf(name,sizeof name,"%s-%s.context",output,label);f=fopen(name,"wb");
 fist_cpu_require(f!=NULL);fixture_cache(&context,f);fist_cpu_require(!fclose(f));
}

static void dos21_initial(void) {fist_dos_cpu_set_dta(&context.bus);}

static void write_host_state(const char *label) {
 char path[1024];snprintf(path,sizeof path,"%s-%s.host",output,label);
 FILE *file=fopen(path,"wb");fist_cpu_require(file!=NULL);
 uint32_t values[2]={host_drive,host_next_free};
 fist_cpu_require(fwrite(values,sizeof values,1,file)==1 && fwrite(host_occupied,1,sizeof host_occupied,file)==sizeof host_occupied);
 fist_cpu_require(fclose(file)==0);
}
static void find_state(const char *label) {
 char full[64];snprintf(full,sizeof full,"find-%s",label);state(full);snapshot(full);
 if(!strcmp(label,"before-directory") || !strcmp(label,"after-directory") || !strcmp(label,"before-SetResult"))write_host_state(full);
}
static void find_observer(void *opaque,const char *kind) {find_state(kind);}
static void dos21(void) {
 if(!find_active){dos21_initial();return;}
 fist_dos_cpu_find_first(&context.bus,&host);
}

static void observed_snapshot(void) {
 uint32_t cs=cpu.segments[1].value,ip=cpu.eip;unsigned keep=0;
 if(cs==8)keep=ip==0x1abb||ip==0x1abf||ip==0x142f||ip==0x1432||ip==0x1558||ip==0x155d||ip==0x1563||ip==0x1569;
 if(cs==0x2dd)keep=ip==0x1437||ip==0x143a||ip==0x143d||ip==0x143f||ip==0x1548||ip==0x154b||ip==0x1550||ip==0x1553;
 if(keep && !find_active){char label[64];snprintf(label,sizeof label,"fetch-%04x-%04x",cs,ip);snapshot(label);}
}

static void startup_snapshot(void) {
 if(cpu.segments[1].value!=0x2b || find_active)return;
 switch(cpu.eip){case 0x77e2:case 0x77e9:case 0x1280:case 0x12ab:case 0x77ee:case 0x23c4:case 0x133a:case 0x77ff:case 0x7809:case 0x3322:case 0x780e:case 0x6032:case 0x603f:case 0x5cc2:case 0x5cdd:{
  char label[32];snprintf(label,sizeof label,"prefix-%04x",cpu.eip);snapshot(label);break;
 }}
}
static unsigned observed_app_int;
static unsigned producer_budget(void *opaque) { return fist_clock_cpu_slice(NULL); }
static void producer_credit(void *opaque) { fist_clock_credit_cpu_fetch(); }
static void producer_charge(void *opaque,unsigned count) { fist_clock_charge_cpu_instructions(count); }
static uint32_t producer_in(void *opaque,unsigned port) { return in(port); }
static void producer_out(void *opaque,unsigned port,unsigned value) { out(port,value); }
static void producer_callback(FistExec *e,unsigned number) { fist_cpu_require(number==SOURCE_DOS_CALLBACK);dos21(); }
static void producer_observe(FistExec *e,unsigned event,uint32_t a,uint32_t b) {
 switch(event) {
 case FIST_EXEC_INT_BEFORE:
  observed_app_int=cpu.segments[1].value==0x2b;
  if(observed_app_int){if(cpu.eip==0x5df4){find_active=1;find_state("before-software");}
   else {fist_cpu_require(cpu.eip==0x5cdd);state("before-software");}}break;
 case FIST_EXEC_INT_AFTER:
  if(observed_app_int){if(find_active)find_state("after-software");else{state("after-software");snapshot("after-software");}}break;
 case FIST_EXEC_RET_BEFORE:
  if(find_active)find_state("before-far-ret");else{state("before-software-ret");snapshot("before-software-ret");}break;
 case FIST_EXEC_RET_AFTER:
  if(find_active)find_state("after-far-ret");else{state("after-software-ret");snapshot("after-software-ret");}break;
 case FIST_EXEC_CALLBACK_BEFORE:
  if(find_active)find_state("before-DOS21");else{state("before-DOS21");snapshot("before-DOS21");}break;
 case FIST_EXEC_CALLBACK_AFTER:
  if(find_active)find_state("after-DOS21");else{state("after-DOS21");snapshot("after-DOS21");}break;
 default:abort();
 }
}
static void execute(void) {
 unsigned fetches=0;
 engine=(FistExec){.bus=&context.bus,.in=producer_in,.out=producer_out,.budget=producer_budget,
  .credit=producer_credit,.charge=producer_charge,.callback=producer_callback,.observe=producer_observe};
 for(;;){
  fist_clock_charge_cpu_instructions(1);state("fetch");fetches++;
  startup_snapshot();observed_snapshot();
  if(cpu.eip==0x5df4 && cpu.segments[1].value==0x2b)snapshot("before-next-DOS");
  if(find_active && cpu.segments[1].value==0x2b){snapshot("find-caller");break;}
  fist_exec_fetched(&engine);
 }
 printf("fetches %u\n",fetches);
}
int main(int argc,char **argv){
 fist_cpu_require(argc==5);fist_cpu_require(setenv("FIST_SB","1",1)==0);host_directory=argv[4];host.observe=find_observer;FILE *f=fopen(argv[1],"rb");fist_cpu_require(f!=NULL);
 fist_cpu_require(fread(&cpu,sizeof cpu,1,f)==1 && fread(&sys,sizeof sys,1,f)==1);
 uint8_t *ram=g_mem;FILE *memory=fopen(argv[2],"rb");
 fist_cpu_require(memory && fread(ram,16777216,1,memory)==1 && fgetc(memory)==EOF && !fclose(memory));
 fixture_restore(&context,f,&cpu,&sys,ram,16777216);remaining=fixture_word(f);
 uint64_t start=fixture_word(f),now;fist_clock_cpu_slice(&now);fist_cpu_require(start>=now && start-now<=UINT32_MAX);
 fist_clock_advance_cpu_cycles((unsigned)(start-now));fist_clock_cpu_ss_instruction();fist_clock_bind_cpu(&cpu);replay_running=1;
 host_next_free=fixture_word(f);fist_cpu_require(host_next_free<2048);
 fist_cpu_require(fread(host_occupied,1,sizeof host_occupied,f)==sizeof host_occupied && fread(host_name_prior,1,sizeof host_name_prior,f)==sizeof host_name_prior);
 host_drive=fixture_word(f);fist_cpu_require(host_drive<26);
 fist_cpu_require(fgetc(f)==EOF && !fclose(f));output=argv[3];
 write_host_state("initial-host");
 /* Actual same-capture first INT21 state; every following CPU/RAM mutation
  * comes from execution. Never reseed at API, paging or return boundaries. */
 /* The actual outer caller enters here before its first fetch; INTs are decoded. */
 execute();fixture_destroy(&context);return 0;
}
