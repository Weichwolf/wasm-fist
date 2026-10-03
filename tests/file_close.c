#include "sb_clock_fixture.h"
#include <string.h>
int fist_sb_owns(int p) { return 0; }
int fist_sb_in(int p) { abort(); }
void fist_sb_out(int p,int v) { abort(); }
uint32_t fist_ext_base;
#define REG(o) (*(uint16_t *)(g_mem+0xf0000+(o)))
extern undefined2 FUN_0000_fefb(undefined2,undefined2,undefined2,undefined2,undefined2,undefined2,undefined2,undefined2,undefined2);
extern undefined2 FUN_1000_50c8(undefined2,undefined2,undefined2,undefined2,undefined2,undefined2,undefined2,undefined2);
static unsigned calls, opened, closed, read_cf, read_ax;
static int fail_read;
void fist_close_test_dispatch(void)
{
    assert(REG(0x14)==0x21);
    unsigned command=REG(0)>>8, handle=REG(2);
    assert(command==0x3d || command==0x3f || command==0x3e || command==0x42);
    ++calls;
    if(command==0x3f && fail_read) REG(2)=0xffff;
    fist_int_dispatch();
    if(command==0x3f && fail_read) REG(2)=handle;
    if(command==0x3d && !REG(0x12)) opened=REG(0);
    if(command==0x3e) closed=handle;
    if(command==0x3f) {read_cf=REG(0x12);read_ax=REG(0);}
}
static unsigned open_sample(void)
{
    REG(0)=0x3d00;REG(6)=0x740;REG(0xe)=0x1c00;REG(0x14)=0x21;
    fist_int_dispatch();assert(!REG(0x12));return REG(0);
}
int main(int argc,char **argv)
{
    assert(argc==6);
    memset(g_mem,0xa5,sizeof g_mem);
    *(uint16_t *)(g_mem+0x1c070)=0x3000;
    strcpy((char *)(g_mem+0x30089),argv[2]);
    strcpy((char *)(g_mem+0x1c740),argv[2]);
    unsigned peers=strtoul(argv[4],0,0), unrelated=strtoul(argv[5],0,0);
    unsigned peer[16];assert(peers<16);
    for(unsigned i=0;i<peers;++i) peer[i]=open_sample();
    fail_read=!strcmp(argv[1],"read-error");
    undefined2 result;
    if(!strcmp(argv[1],"size")) result=FUN_1000_50c8(0,0x740,0x4fa,0,0,0,unrelated,0);
    else result=FUN_0000_fefb(0,0x168,0,0,0x89,0,0,0,unrelated);
    unsigned returned_cf=REG(0x12),returned_dx=REG(6),returned_cx=REG(4);
    FILE *out=fopen(argv[3],"wb");assert(out);
    assert(fwrite(g_mem+0x1c740,1,65,out)==65 && !fclose(out));
    unsigned alive=0;
    for(unsigned i=0;i<peers;++i) {
        REG(0)=0x4200;REG(2)=peer[i];REG(4)=0;REG(6)=0;REG(0x14)=0x21;
        fist_int_dispatch();alive+=!REG(0x12);
    }
    /* Real DOS allocation must observe the closed handle, including all peers. */
    strcpy((char *)(g_mem+0x1c740),"sample.bin");
    unsigned next=open_sample();
    printf("result %u cf %u dx %u cx %u calls %u opened %u closed %u read-ax %u read-cf %u next %u peers-alive %u\n",
           result,returned_cf,returned_dx,returned_cx,calls,opened,closed,read_ax,read_cf,next,alive);
}
