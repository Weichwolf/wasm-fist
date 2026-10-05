/* Complete bound drawing/PIC observations; the production owners supply effects. */
#define fist_int8_fire unused_int8
#include "sb_clock_fixture.h"
#undef fist_int8_fire
#include "fist_cpu.h"
#include "fist_vga.c"
void fist_int8_fire(void){abort();}
static FistVgaDraw initial_drawing,replacement,*v=&initial_drawing;static FistVgaStatus status;
static uint8_t ram[0x40000],requests[0x40000];static unsigned request_size;
static void request(const void *p,unsigned n){fist_cpu_require(request_size+n<=sizeof requests);memcpy(requests+request_size,p,n);request_size+=n;}
static const uint8_t *line(void *p,uint64_t a,uint64_t b){return fist_vga_linear_line(v,ram,a);}
static void emit(void *p,const uint8_t *data){uint64_t q[]={1,v->address,v->address_line,v->line_length};request(q,sizeof q);request(data,v->line_length);}
static void end(void *p,int aborted){uint64_t q[]={2,aborted};request(q,sizeof q);}
static void word(uint32_t q){fist_cpu_require(fwrite(&q,4,1,stdout)==1);}
static void first_part(unsigned),sentinel(unsigned);
static void snapshot(unsigned label){
 fist_cpu_require(fwrite(v,sizeof *v,1,stdout)==1);uint64_t cycle=clock_cpu_cycles(clock_now());unsigned index=pic_index(cycle);
 word(label);word(g_pic_tick);word(index);word(g_cpu_remaining);word(30000-index-g_cpu_remaining);word(g_pic_service);
 unsigned n=0;for(FistPicEntry *e=g_pic_events;e;e=e->next)n++;word(n);
 for(FistPicEntry *e=g_pic_events;e;e=e->next){uint32_t bits;memcpy(&bits,&e->index,4);word(bits);word(e->value);unsigned id=e->handler==fist_clock_vga_draw_part?0:e->handler==first_part?1:e->handler==fist_clock_vga_vert_interrupt?2:e->handler==fist_clock_vga_display_start?3:e->handler==sentinel?4:5;fist_cpu_require(id<5);word(id);}
 fist_cpu_require(fwrite(ram,sizeof ram,1,stdout)==1);word(request_size);fist_cpu_require(!request_size || fwrite(requests,request_size,1,stdout)==1);request_size=0;
}
static void observed_add(void *p,float delay,uint64_t value){vga_host_add(p,delay,value);snapshot(1);}
static void first_part(unsigned value){snapshot(2);fist_clock_vga_draw_part(value);snapshot(3);}
static void sentinel(unsigned value){uint64_t q[]={5,value};request(q,sizeof q);}
int main(){uint64_t q[40];fist_cpu_require(fread(q,sizeof q,1,stdin)==1 && fread(ram,sizeof ram,1,stdin)==1);memcpy(v,q,sizeof *v);
 FistCpuState cpu={0};fist_clock_bind_cpu(&cpu);fist_clock_bind_vga(v,&status,NULL,line,emit,end);g_vga_draw_host.add=observed_add;
 uint32_t bits=q[39];float delay;memcpy(&delay,&bits,4);
 fist_clock_add_event(first_part,delay,50);
 fist_clock_add_event(fist_clock_vga_vert_interrupt,20.0f,0);
 fist_clock_add_event(fist_clock_vga_display_start,21.0f,0);
 fist_clock_add_event(sentinel,4.0f,123);
 fist_clock_cpu_core_exit();snapshot(0);
 fist_clock_charge_cpu_instructions(400);snapshot(5);
 if(q[38]) {
  if(q[38]==1){replacement=*v;replacement.address+=1000;fist_clock_bind_vga(&replacement,&status,NULL,line,emit,end);v=&replacement;g_vga_draw_host.add=observed_add;}
  else {fist_cpu_require(q[38]==2);fist_clock_bind_vga(NULL,NULL,NULL,NULL,NULL,NULL);}
  snapshot(6);
 }
 fist_clock_charge_cpu_instructions(400000);snapshot(4);return 0;
}
