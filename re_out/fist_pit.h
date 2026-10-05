#ifndef FIST_PIT_H
#define FIST_PIT_H
#include <stdint.h>
#include <math.h>
#include "fist_cpu.h"
/* timer.cpp PIT_Block and its counter/control/gate producers. The machine owns
 * this state and the host callbacks; no startup snapshot or clock is supplied. */
typedef struct {
 uint32_t cntr; float delay; double start;
 uint16_t read_latch,write_latch;
 uint8_t mode,latch_mode,read_state,write_state;
 uint8_t bcd,go_read_latch,new_mode,counterstatus_set,counting,update_count;
} FistPitCounter;
typedef struct {
 FistPitCounter counters[3];
 uint8_t gate2,status,status_locked;
} FistPit;
typedef struct {
 void *context;
 double (*time)(void *);
 void (*add_event)(void *,float);
 void (*remove_events)(void *);
 void (*irq)(void *,unsigned,int);
 void (*minimum_budget)(void *,unsigned);
 void (*speaker)(void *,unsigned,unsigned);
} FistPitHost;
#define FIST_PIT_HZ 1193182u
static inline float fist_pit_delay(unsigned count)
{
 /* Both source divisions are binary32; x87 otherwise retains extra precision. */
 volatile float frequency=(float)FIST_PIT_HZ/(float)count;
 volatile float delay=1000.0f/frequency;
 return delay;
}
static inline int fist_pit_output(FistPit *pit,const FistPitHost *host,unsigned counter)
{
 fist_cpu_require(counter<3);FistPitCounter *p=&pit->counters[counter];
 double index=host->time(host->context)-p->start;
 switch(p->mode) {
 case 0:return !p->new_mode && index>p->delay;
 case 2:if(p->new_mode)return 1;return fmod(index,(double)p->delay)>0;
 case 3:if(p->new_mode)return 1;return fmod(index,(double)p->delay)*2<p->delay;
 case 4:return 1;
 default:return 1; /* Original reports unsupported modes and returns high. */
 }
}
static inline uint16_t fist_pit_bin_to_bcd(uint16_t v)
{return v%10+(((v/10)%10)<<4)+(((v/100)%10)<<8)+(((v/1000)%10)<<12);}
static inline uint16_t fist_pit_bcd_to_bin(uint16_t v)
{return (v&15)+((v>>4)&15)*10+((v>>8)&15)*100+((v>>12)&15)*1000;}
static inline void fist_pit_counter_latch(FistPit *pit,const FistPitHost *host,unsigned counter)
{
 fist_cpu_require(counter<3);FistPitCounter *p=&pit->counters[counter];
 p->go_read_latch=0;
 if(counter==2 && !pit->gate2 && p->mode!=1)return;
 double index=host->time(host->context)-p->start;
 switch(p->mode) {
 case 4:case 0:
  if(index>p->delay) {
   index-=p->delay;
   if(p->bcd) {
    index=fmod(index,(1000.0/FIST_PIT_HZ)*10000.0);
    p->read_latch=(uint16_t)(9999-index*(FIST_PIT_HZ/1000.0));
   } else {
    index=fmod(index,(1000.0/FIST_PIT_HZ)*65536.0);
    p->read_latch=(uint16_t)(65535-index*(FIST_PIT_HZ/1000.0));
   }
  } else p->read_latch=(uint16_t)(p->cntr-index*(FIST_PIT_HZ/1000.0));
  break;
 case 1:
  if(p->counting)p->read_latch=index>p->delay?65535:(uint16_t)(p->cntr-index*(FIST_PIT_HZ/1000.0));
  break;
 case 2:
  index=fmod(index,(double)p->delay);
  p->read_latch=(uint16_t)(p->cntr-(index/p->delay)*p->cntr);
  break;
 case 3:
  index=fmod(index,(double)p->delay)*2;
  if(index>p->delay)index-=p->delay;
  p->read_latch=(uint16_t)(p->cntr-(index/p->delay)*p->cntr);
  p->read_latch&=0xfffe;
  break;
 default:p->read_latch=65535;break;
 }
}
static inline void fist_pit_status_latch(FistPit *pit,const FistPitHost *host,unsigned counter)
{
 fist_cpu_require(counter<3);if(pit->status_locked)return;
 FistPitCounter *p=&pit->counters[counter];
 pit->status=p->bcd|((p->mode&7)<<1);
 pit->status|=p->read_state==0||p->read_state==3?0x30:p->read_state==1?0x10:p->read_state==2?0x20:0;
 if(fist_pit_output(pit,host,counter))pit->status|=0x80;
 if(p->new_mode)pit->status|=0x40;
 p->counterstatus_set=1;pit->status_locked=1;
}
static inline void fist_pit_control(FistPit *pit,const FistPitHost *host,unsigned value)
{
 fist_cpu_require(value<=255);unsigned counter=(value>>6)&3;
 if(counter==3) {
  if(!(value&0x20))for(unsigned i=0;i<3;i++)if(value&(2u<<i))fist_pit_counter_latch(pit,host,i);
  if(!(value&0x10))for(unsigned i=0;i<3;i++)if(value&(2u<<i)) {fist_pit_status_latch(pit,host,i);break;}
  return;
 }
 if(!(value&0x30)) {fist_pit_counter_latch(pit,host,counter);return;}
 FistPitCounter *p=&pit->counters[counter];p->bcd=(value&1)!=0;
 if(p->bcd && p->cntr>=9999)p->cntr=9999;
 if(p->counterstatus_set) {p->counterstatus_set=0;pit->status_locked=0;}
 p->update_count=0;p->counting=0;
 p->read_state=p->write_state=(value>>4)&3;
 unsigned mode=(value>>1)&7;if(mode>5)mode-=4;
 if(!p->mode)p->mode=mode;
 if(counter==0) {
  host->remove_events(host->context);
  if(!fist_pit_output(pit,host,0) && mode) {host->irq(host->context,0,1);host->minimum_budget(host->context,25);}
  if(!mode)host->irq(host->context,0,0);
 }
 p->new_mode=1;p->mode=mode;
}
static inline void fist_pit_counter_write(FistPit *pit,const FistPitHost *host,unsigned counter,unsigned value)
{
 fist_cpu_require(counter<3 && value<=255);FistPitCounter *p=&pit->counters[counter];
 if(p->bcd)p->write_latch=fist_pit_bin_to_bcd(p->write_latch);
 switch(p->write_state) {
 case 0:p->write_latch|=(value&255)<<8;p->write_state=3;break;
 case 3:p->write_latch=value&255;p->write_state=0;break;
 case 1:p->write_latch=value&255;break;
 case 2:p->write_latch=(value&255)<<8;break;
 }
 if(p->bcd)p->write_latch=fist_pit_bcd_to_bin(p->write_latch);
 if(p->write_state==0)return;
 p->cntr=p->write_latch?p->write_latch:p->bcd?9999:65536;
 if(counter==0 && !p->new_mode && p->mode==2) {p->update_count=1;return;}
 p->start=host->time(host->context);p->delay=fist_pit_delay(p->cntr);
 if(counter==0 && (p->new_mode || p->mode==0)) {
  if(p->mode==0)host->remove_events(host->context);
  host->add_event(host->context,p->delay);
 } else if(counter==2)host->speaker(host->context,p->cntr,p->mode);
 p->new_mode=0;
}
static inline unsigned fist_pit_counter_read(FistPit *pit,const FistPitHost *host,unsigned counter)
{
 fist_cpu_require(counter<3);FistPitCounter *p=&pit->counters[counter];
 if(p->counterstatus_set) {p->counterstatus_set=0;pit->status_locked=0;return pit->status;}
 if(p->go_read_latch)fist_pit_counter_latch(pit,host,counter);
 if(p->bcd)p->read_latch=fist_pit_bin_to_bcd(p->read_latch);
 unsigned result=0;
 switch(p->read_state) {
 case 0:result=p->read_latch>>8;p->read_state=3;p->go_read_latch=1;break;
 case 3:result=p->read_latch&255;p->read_state=0;break;
 case 1:result=p->read_latch&255;p->go_read_latch=1;break;
 case 2:result=p->read_latch>>8;p->go_read_latch=1;break;
 default:abort();
 }
 if(p->bcd)p->read_latch=fist_pit_bcd_to_bin(p->read_latch);
 return result;
}
static inline void fist_pit_gate2(FistPit *pit,const FistPitHost *host,unsigned enabled)
{
 fist_cpu_require(enabled<=1);if(pit->gate2==enabled)return;
 FistPitCounter *p=&pit->counters[2];
 switch(p->mode) {
 case 0:
  if(enabled)p->start=host->time(host->context);
  else {fist_pit_counter_latch(pit,host,2);p->cntr=p->read_latch;}
  break;
 case 1:if(enabled) {p->counting=1;p->start=host->time(host->context);}break;
 case 2:case 3:
  if(enabled)p->start=host->time(host->context);else fist_pit_counter_latch(pit,host,2);
  break;
 }
 pit->gate2=enabled;
}
static inline void fist_pit_event(FistPit *pit,const FistPitHost *host)
{
 host->irq(host->context,0,1);FistPitCounter *p=&pit->counters[0];
 if(p->mode) {
  p->start+=p->delay;
  if(p->update_count) {p->delay=fist_pit_delay(p->cntr);p->update_count=0;}
  host->add_event(host->context,p->delay);
 }
}
static inline void fist_pit_initialize(FistPit *pit,const FistPitHost *host)
{
 /* TIMER construction assigns these fields, retaining other existing state.
  * A cold machine supplies its initial zero state, rather than this producer
  * clearing a repeated construction's retained latches/start/mode metadata. */
 FistPitCounter *p=pit->counters;
 p[0].cntr=65536;p[0].write_state=p[0].read_state=3;
 p[0].read_latch=p[0].write_latch=0;p[0].mode=3;p[0].bcd=0;
 p[0].go_read_latch=1;p[0].counterstatus_set=0;p[0].update_count=0;
 p[1].bcd=0;p[1].write_state=3;p[1].read_state=1;p[1].go_read_latch=1;
 p[1].cntr=18;p[1].mode=2;p[1].counterstatus_set=0;
 p[2].read_latch=1320;p[2].write_state=p[2].read_state=3;p[2].mode=3;p[2].bcd=0;
 p[2].cntr=1320;p[2].go_read_latch=1;p[2].counterstatus_set=0;p[2].counting=0;
 for(unsigned i=0;i<3;i++)p[i].delay=fist_pit_delay(p[i].cntr);
 pit->status_locked=0;pit->gate2=0;host->add_event(host->context,p[0].delay);
}
/* Bind the machine's actual timer/speaker owner to the existing CPU/PIC clock.
 * Binding never initializes counters or installs a captured startup state.
 * Detachment removes timer callbacks and keeps the actual PIC calendar owner;
 * it does not reactivate the translated outer loop's synthetic PIT clock. */
void fist_clock_bind_pit(FistPit *pit,void *speaker_context,
                         void (*speaker)(void *,unsigned,unsigned),
                         void (*speaker_type)(void *,unsigned));
const FistPitHost *fist_clock_pit_host(void);
double fist_clock_full_index(void);
uint64_t fist_clock_io_delay_removed(void);
#endif
