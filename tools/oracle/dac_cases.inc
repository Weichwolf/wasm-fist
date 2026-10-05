#include <stdio.h>
#include <string.h>
static FILE *output;
static unsigned event_count;
static uint32_t events[256][4];
static unsigned programs,registration_count;
static uint32_t registrations[8][5];
static void registered(unsigned direction,unsigned port,unsigned mask,unsigned range,unsigned handler) {
 REQUIRE(registration_count<8);uint32_t *q=registrations[registration_count++];q[0]=direction;q[1]=port;q[2]=mask;q[3]=range;q[4]=handler;
}
static void event(unsigned index,unsigned red,unsigned green,unsigned blue) {
 REQUIRE(event_count<256);
 uint32_t *q=events[event_count++];q[0]=index;q[1]=red;q[2]=green;q[3]=blue;
}
static void reset(unsigned mode,unsigned index,unsigned lane,unsigned mask,unsigned state,unsigned variant) {
 memset(&D,0,sizeof D);memset(&P,0,sizeof P);
 SET_MACHINE(5);SET_CARD(0);registration_count=0;memset(registrations,0,sizeof registrations);
 SET_MODE(mode);D.bits=variant&1?8:6;D.pel_mask=mask;D.pel_index=lane;D.state=state;
 D.write_index=index;D.read_index=255-index;D.first_changed=UINT64_C(0x123456789abcdef0);
 for(unsigned i=0;i<16;i++)D.combine[i]=variant&1?index:(index+i*17)&255;
 for(unsigned i=0;i<256;i++) {
  D.rgb[i].red=(i*37+variant*91)&255;D.rgb[i].green=(i*17+variant*47)&255;D.rgb[i].blue=(i*61+variant*11)&255;
  D.xlat16[i]=0xab00^i;
  P.rgb[i].red=i*13;P.rgb[i].green=i*29;P.rgb[i].blue=i*41;P.rgb[i].unused=0xd5;
  P.modified[i]=i*7;
 }
 memset(&P.lut,0x9b,sizeof P.lut);P.changed=true;P.first=UINT64_C(0x100000100);P.last=0;
 event_count=0;memset(events,0,sizeof events);
}
static void record(unsigned value) {
 unsigned char data[8192],*p=data;
#define APPEND(v) do {memcpy(p,&(v),sizeof(v));p+=sizeof(v);} while(0)
 uint32_t result=value;APPEND(result);
 APPEND(D.bits);APPEND(D.pel_mask);APPEND(D.pel_index);APPEND(D.state);APPEND(D.write_index);APPEND(D.read_index);
 uint64_t first_changed=D.first_changed;APPEND(first_changed);
 APPEND(D.combine);APPEND(D.rgb);APPEND(D.xlat16);
 uint32_t mode=MODE;APPEND(mode);
 APPEND(P.rgb);APPEND(P.lut);APPEND(P.modified);
 uint8_t changed=P.changed;APPEND(changed);
 uint64_t first=P.first,last=P.last;APPEND(first);APPEND(last);
 uint32_t machine_value=MACHINE,card_value=CARD;APPEND(machine_value);APPEND(card_value);APPEND(registration_count);APPEND(registrations);
 uint32_t count=event_count;APPEND(count);
 memcpy(p,events,event_count*16);p+=event_count*16;
 REQUIRE((unsigned)(p-data)<=sizeof data && fwrite(data,p-data,1,output)==1);
#undef APPEND
 programs++;
}
static void one(unsigned mode,unsigned index,unsigned lane,unsigned mask,unsigned state,unsigned variant,unsigned port,unsigned write,unsigned value) {
 reset(mode,index,lane,mask,state,variant);
 unsigned result=0;if(write)write_port(port,value);else result=read_port(port);record(result);
}
int main(int argc,char **argv) {
 REQUIRE(argc==2);output=fopen(argv[1],"wb");REQUIRE(output!=NULL);
 /* Every source RGB component/value/write index, including bit6/7 truncation. */
 for(unsigned i=0;i<256;i++)for(unsigned lane=0;lane<3;lane++)for(unsigned value=0;value<256;value++)
  one(3,i,lane,255,1,i&1,0x3c9,1,value);
 /* Every write index/mask alias predicate in both indexed modes. */
 for(unsigned mode=3;mode<=5;mode+=2)for(unsigned mask=0;mask<256;mask++)for(unsigned i=0;i<256;i++)
  one(mode,i,2,mask,1,i&1,0x3c9,1,0xa5);
 /* Every old/new mask: changed masks refresh all256 ordered colors. */
 for(unsigned old=0;old<256;old++)for(unsigned value=0;value<256;value++)one(3,17,0,old,1,1,0x3c6,1,value);
 /* All original mode enums and every attribute alias, including duplicates. */
 for(unsigned mode=0;mode<18;mode++)for(unsigned i=0;i<256;i++)for(unsigned variant=0;variant<2;variant++)
  one(mode,i,2,255,1,variant,0x3c9,1,0x7f);
 /* Reads/controls cover every index and valid/diagnostic component/state. */
 unsigned lanes[]={0,1,2,3,255},states[]={0,1,2,255};
 for(unsigned i=0;i<256;i++)for(unsigned lane=0;lane<5;lane++)for(unsigned state=0;state<4;state++)for(unsigned port=0x3c6;port<=0x3c9;port++)
  one(3,i,lanes[lane],0x7f,states[state],i&1,port,0,0);
 for(unsigned i=0;i<256;i++)for(unsigned port=0x3c7;port<=0x3c8;port++)one(3,253,255,0x7f,255,1,port,1,i);
 /* Read and write operations share the component cursor across a sequence. */
 for(unsigned i=0;i<256;i++) {
  reset(3,i,0,255,1,i&1);
  write_port(0x3c7,i);record(0);
  record(read_port(0x3c9));write_port(0x3c9,0x91);record(0);record(read_port(0x3c9));
  write_port(0x3c9,0xe3);record(0);write_port(0x3c8,i);record(0);
  write_port(0x3c9,0x84);record(0);record(read_port(0x3c9));write_port(0x3c9,0xc7);record(0);
 }
 /* All mode/machine/SVGA branch combinations and every attribute. */
 unsigned pals[]={0,1,15,127,255};
 for(unsigned mode=0;mode<18;mode++)for(unsigned machine=0;machine<6;machine++)for(unsigned card=0;card<5;card++)for(unsigned attr=0;attr<16;attr++)for(unsigned p=0;p<5;p++) {
  reset(mode,71,2,127,1,p&1);SET_MACHINE(machine);SET_CARD(card);combine(attr,pals[p]);record(0);
 }
 for(unsigned attr=0;attr<16;attr++)for(unsigned pal=0;pal<256;pal++) {reset(3,71,2,127,1,pal&1);combine(attr,pal);record(0);}
 /* Every entry/component/byte, retaining the other two dirty source components. */
 for(unsigned entry=0;entry<256;entry++)for(unsigned lane=0;lane<3;lane++)for(unsigned value=0;value<256;value++) {
  reset(3,entry,2,127,1,entry&1);unsigned rgb[]={D.rgb[entry].red,D.rgb[entry].green,D.rgb[entry].blue};rgb[lane]=value;
  set_entry(entry,rgb[0],rgb[1],rgb[2]);record(0);
 }
 for(unsigned entry=0;entry<256;entry++)for(unsigned variant=0;variant<2;variant++) {
  reset(3,entry,2,127,1,variant);set_entry(entry,255,128,64);record(0);
 }
 for(unsigned mode=0;mode<18;mode++)for(unsigned machine=0;machine<6;machine++)for(unsigned card=0;card<5;card++)for(unsigned variant=0;variant<2;variant++) {
  reset(mode,253,255,127,255,variant);SET_MACHINE(machine);SET_CARD(card);setup();record(0);
 }
 REQUIRE(programs==671224);
 REQUIRE(!ferror(output) && !fclose(output));return 0;
}
