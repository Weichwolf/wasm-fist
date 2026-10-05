#define main unused_pic_main
#include "pic_slice_probe.cpp"
#undef main
extern "C" void full_pit_init(void),full_pit_write(unsigned,unsigned),full_pit_gate(unsigned),full_pit_dump(void);
extern "C" void full_pit_destroy(void);
extern "C" unsigned full_pit_read(unsigned);
extern "C" PIC_EventHandler full_pit_handler(void);
extern "C" void full_ppi_write(unsigned);
extern "C" unsigned full_ppi_read(void),full_ppi_data(void);
static void snapshot(unsigned label)
{
 printf("{\"kind\":\"state\",\"label\":%u,\"tick\":%u,\"index_nd\":%d,\"cycles\":%d,\"left\":%d,\"IRQCheck\":%u,\"IRQActive\":%u,\"service\":%u,\"io_removed\":%lld,\"irqs\":[",label,(unsigned)PIC_Ticks,(int)PIC_TickIndexND(),(int)CPU_Cycles,(int)CPU_CycleLeft,(unsigned)PIC_IRQCheck,(unsigned)PIC_IRQActive,(unsigned)InEventService,(long long)CPU_IODelayRemoved);
 for(unsigned i=0;i<16;i++)printf("%s[%u,%u,%u,%u]",i?",":"",(unsigned)irqs[i].active,(unsigned)irqs[i].masked,(unsigned)irqs[i].inservice,(unsigned)irqs[i].vector);
 printf("],\"counters\":");full_pit_dump();printf(",\"port61\":%u,\"queue\":[",full_ppi_data());unsigned n=0,f=0;
 for(PICEntry *e=pic_queue.next_entry;e;e=e->next) {
  assert(e->pic_event==full_pit_handler());unsigned index,deadline;float cycles=e->index*CPU_CycleMax;
  memcpy(&index,&e->index,4);memcpy(&deadline,&cycles,4);
  printf("%s[%u,%u,%u]",n++?",":"",index,deadline,(unsigned)e->value);
 }
 for(PICEntry *e=pic_queue.free_entry;e;e=e->next)assert(++f<=512);
 assert(n+f+(InEventService?1:0)==512);printf("],\"free_entries\":%u}\n",f);fflush(stdout);
}
static void fetch(unsigned count)
{while(count--)while(CPU_Cycles--<=0)while(!PIC_RunQueue())TIMER_AddTick();}
static int run_program(int argc,char **argv)
{
 if(argc!=2)return 2;
 PIC_8259A controller(NULL);full_pit_init();
 if(!strcmp(argv[1],"budget")) {
  for(unsigned i=0;i<368;i++) {
   while(PIC_Ticks<i)TIMER_AddTick();CPU_Cycles=0;CPU_CycleLeft=1;assert(PIC_RunQueue());
   for(unsigned step=0;step<2;step++) {fetch(1);snapshot(2*i+step);}
  }
  return 0;
 }
 FILE *f=fopen(argv[1],"r");if(!f)return 2;snapshot(0);
 char op;unsigned a,b,label=0;int fields;
 while((fields=fscanf(f," %c %x %x",&op,&a,&b))==3) {
  switch(op) {
  case 'F':assert(!b);fetch(a);break;
  case 'O':source_io_write_delay();if(a==0x61)full_ppi_write(b);else full_pit_write(a,b);break;
  case 'R':assert(!b);source_io_read_delay();printf("{\"kind\":\"read\",\"label\":%u,\"port\":%u,\"value\":%u}\n",label+1,a,a==0x61?full_ppi_read():full_pit_read(a));break;
  case 'G':assert(!b);full_pit_gate(a);break;
  case 'I':assert(!a && !b);full_pit_init();break;
  case 'D':assert(!a && !b);full_pit_destroy();break;
  default:abort();
  }
  snapshot(++label);
 }
 assert(fields==EOF && feof(f) && !ferror(f) && !fclose(f));return 0;
}
int main(int argc,char **argv)
{
 try {return run_program(argc,argv);}
 catch(int endpoint) {assert(endpoint==1);snapshot(0xffffffffu);return 0;}
}
