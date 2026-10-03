#define FIST_TEST_EXT_BASE
#include "sb_clock_fixture.h"
#include <string.h>
void halt_baddata(void) { abort(); }
/* All original BACKLAND variants miss. A successful loader dispatch is outside
 * this diagnostic fixture and must fail if unexpectedly reached. */
code *fist_icall_near(uint16_t seg,uint16_t off) { abort(); }
extern void FUN_1000_26fc(unsigned short,int,unsigned short,unsigned short);
static unsigned calls;
void fist_resource_test_int_dispatch(void)
{
    uint16_t *r=(uint16_t *)(g_mem+0xf0000);
    assert(r[0]==0x4300 && r[10]==0x21);
    printf("probe %u %04x %04x %04x %04x %04x %s\n",calls++,
           r[5],r[6],r[1],r[2],r[3],g_mem+((uint32_t)r[7]<<4)+r[3]);
    fist_int_dispatch();
    assert(r[0]==2 && r[9]==1);
}
int main(int argc,char **argv)
{
    assert(argc==5);
    FILE *input=fopen(argv[1],"rb");assert(input);
    assert(fread(g_mem,1,sizeof g_mem,input)==sizeof g_mem && !ferror(input));
    assert(fgetc(input)==EOF && !fclose(input));
    uint16_t bp=(uint16_t)strtoul(argv[3],0,0);
    assert(!setenv("FIST_DATADIR",argv[4],1));
    FUN_1000_26fc(0x1c00,0x6b7e,0x6b78,bp);
    printf("calls %u\n",calls);
    FILE *output=fopen(argv[2],"wb");assert(output);
    assert(fwrite(g_mem,1,sizeof g_mem,output)==sizeof g_mem && !fclose(output));
}
