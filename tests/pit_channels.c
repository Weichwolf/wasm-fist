
#define fist_int8_fire unused_int8
#include "sb_clock_fixture.h"
#undef fist_int8_fire
void fist_int8_fire(void) {abort();}
extern void begin_lifecycle(void),lifecycle_fetch(unsigned),lifecycle_out(unsigned,unsigned),lifecycle_read(unsigned,unsigned),lifecycle_gate(unsigned),lifecycle_init_again(void),lifecycle_snapshot(const char *,unsigned),lifecycle_budget(void),lifecycle_detach(void);
static void endpoint_observation(void) {lifecycle_snapshot("state",0xffffffffu);}
int main(int argc,char **argv)
{
 /* The isolated WASM fixture does not inherit the host process environment.
  * Transfer matched endpoint settings before any emulated instruction work. */
 if(argc==4) {setenv("FIST_SEQUENCE_END_MS",argv[2],1);setenv("FIST_SEQUENCE",argv[3],1);}
 if(getenv("FIST_SEQUENCE_END_MS"))atexit(endpoint_observation);
 if(argc!=2 && argc!=4)return 2;begin_lifecycle();
 if(!strcmp(argv[1],"budget")) {lifecycle_budget();return 0;}
 FILE *f=fopen(argv[1],"r");if(!f)return 2;lifecycle_snapshot("state",0);
 char op;unsigned a,b,label=0;int fields;
 while((fields=fscanf(f," %c %x %x",&op,&a,&b))==3) {
  switch(op) {
  case 'F':if(b)return 2;lifecycle_fetch(a);break;
  case 'O':lifecycle_out(a,b);break;
  case 'R':if(b)return 2;lifecycle_read(label+1,a);break;
  case 'G':if(b)return 2;lifecycle_gate(a);break;
  case 'I':if(a||b)return 2;lifecycle_init_again();break;
  case 'D':if(a||b)return 2;lifecycle_detach();break;
  default:return 2;
  }
  lifecycle_snapshot("state",++label);
 }
 return fields==EOF && feof(f) && !ferror(f) && !fclose(f)?0:2;
}
