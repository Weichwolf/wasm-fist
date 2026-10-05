#include <assert.h>
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/timer.cpp"
static TIMER *module;
extern "C" void full_pit_init(void) {module=new TIMER(NULL);}
extern "C" void full_pit_destroy(void) {assert(module);delete module;module=NULL;}
extern "C" PIC_EventHandler full_pit_handler(void) {return PIT0_Event;}
extern "C" void full_pit_write(unsigned port,unsigned value)
{
 assert((port==0x40 || port==0x42 || port==0x43) && value<=255);
 if(port==0x43)write_p43(port,value,1);else write_latch(port,value,1);
}
extern "C" unsigned full_pit_read(unsigned port) {assert(port>=0x40 && port<=0x42);return read_latch(port,1);}
extern "C" void full_pit_gate(unsigned enabled) {assert(enabled<=1);TIMER_SetGate2(enabled!=0);}
extern "C" unsigned full_pit_gate_state(void) {return gate2;}
extern "C" void full_pit_dump(void)
{
 printf("[");
 for(unsigned i=0;i<3;i++) {
  const PIT_Block &p=pit[i];unsigned delay;unsigned long long start;
  memcpy(&delay,&p.delay,4);memcpy(&start,&p.start,8);
  printf("%s[%u,%u,%llu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u]",i?",":"",(unsigned)p.cntr,delay,start,(unsigned)p.read_latch,(unsigned)p.write_latch,(unsigned)p.mode,(unsigned)p.latch_mode,(unsigned)p.read_state,(unsigned)p.write_state,(unsigned)p.bcd,(unsigned)p.go_read_latch,(unsigned)p.new_mode,(unsigned)p.counterstatus_set,(unsigned)p.counting,(unsigned)p.update_count,(unsigned)counter_output(i));
 }
 printf("],\"gate2\":%u,\"status\":%u,\"status_locked\":%u",(unsigned)gate2,(unsigned)latched_timerstatus,(unsigned)latched_timerstatus_locked);
}
void PCSPEAKER_SetCounter(Bitu count,Bitu mode)
{
#ifndef SILENT_SPEAKER_OBSERVER
 printf("{\"kind\":\"speaker-request\",\"count\":%u,\"mode\":%u}\n",(unsigned)count,(unsigned)mode);
#endif
}
Bit16u mem_readw(PhysPt) {abort();}
