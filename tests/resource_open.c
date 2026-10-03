#define FIST_TEST_EXT_BASE
#include "sb_clock_fixture.h"
#include <string.h>
void halt_baddata(void) { abort(); }
/* All original BACKLAND variants miss. A successful loader dispatch is outside
 * this diagnostic fixture and must fail if unexpectedly reached. */
code *fist_icall_near(uint16_t seg,uint16_t off) { abort(); }
extern void FUN_1000_26fc(unsigned short,int,unsigned short,unsigned short);
extern void FUN_0000_222f(void);
extern void FUN_0000_f842(int,unsigned short,unsigned short *,unsigned short);
extern unsigned int FUN_1000_223c(unsigned short,unsigned short,unsigned short,
                                 unsigned short,unsigned short,unsigned short);
unsigned short g_fist_1345_bp;
static unsigned calls;
static const char *before_path;
static void save(const char *path)
{
    FILE *output=fopen(path,"wb");assert(output);
    assert(fwrite(g_mem,1,sizeof g_mem,output)==sizeof g_mem && !fclose(output));
}
/* This recovery branch is outside the recorded early CRT input. Fail if the
 * real generated F7C3 reaches it instead of silently substituting behavior. */
unsigned int FUN_1000_0d2e(unsigned short ax,int cx,unsigned short bp) { abort(); }
code *fist_icall_far(uint32_t pointer)
{
    if (pointer==0x0f69306c) return (code *)FUN_1000_26fc;
    if (pointer==0x0f6901b2) return (code *)FUN_0000_f842;
    abort();
}
void fist_resource_test_int_dispatch(void)
{
    uint16_t *r=(uint16_t *)(g_mem+0xf0000);
    assert(r[10]==0x21);
    uint16_t incoming=r[0];
    if (incoming==0x4300) {
        printf("probe %u %04x %04x %04x %04x %04x %s\n",calls++,
               r[5],r[6],r[1],r[2],r[3],g_mem+((uint32_t)r[7]<<4)+r[3]);
    } else assert(before_path);
    fist_int_dispatch();
    if (incoming==0x4300) assert(r[0]==2 && r[9]==1);
    /* Snapshot after the last preceding DOS call: the ensuing full-memory
     * interval includes the actual emitted CRT install and its RET helpers.
     * Earlier legacy interrupt/ES stores are retained in the input snapshot. */
    if (incoming==0x2524) save(before_path);
}
int main(int argc,char **argv)
{
    assert(argc==5 || argc==6);
    FILE *input=fopen(argv[1],"rb");assert(input);
    assert(fread(g_mem,1,sizeof g_mem,input)==sizeof g_mem && !ferror(input));
    assert(fgetc(input)==EOF && !fclose(input));
    uint16_t bp=(uint16_t)strtoul(argv[3],0,0);
    assert(!setenv("FIST_DATADIR",argv[4],1));
    g_fist_1345_bp=bp;  /* actual source fixture input, published by production1345 */
    if (argc==6 && !strcmp(argv[5],"crt")) {
        char path[4096];assert(snprintf(path,sizeof path,"%s.before",argv[2])<(int)sizeof path);
        before_path=path;
        FUN_1000_223c(0x1c00,0,0,0x91c,4,0x400);
    } else if (argc==6 && !strcmp(argv[5],"caller")) FUN_0000_222f();
    else { assert(argc==5);FUN_1000_26fc(0x1c00,0x6b7e,0x6b78,bp); }
    printf("calls %u\n",calls);
    save(argv[2]);
}
