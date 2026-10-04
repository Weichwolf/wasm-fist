#include "fist_interrupt.h"
#include "memory_context_fixture.h"
#include <stdio.h>
_Static_assert(sizeof(FistCpuState)==37*4,"CPU words");
_Static_assert(sizeof(FistCpuSystem)==21*4,"system words");
static FistCpuState cpu;
static FistCpuSystem sys;
static FistMemoryFixture context;
static uint32_t observed_read(void *opaque,uint32_t address,unsigned width)
{
    uint32_t value=fist_vga_chained_read(opaque,address,width);
    printf("read %08x %u %08x\n",address,width,value);
    return value;
}
static void observed_write(void *opaque,uint32_t address,unsigned width,uint32_t value)
{
    printf("write %08x %u %08x\n",address,width,value);
    fist_vga_chained_write(opaque,address,width,value);
}
int main(int argc,char **argv)
{
    if(argc!=6)return 2;
    FILE *f=fopen(argv[1],"rb");if(!f)return 3;
    uint32_t op[3];
    if(fread(op,sizeof op,1,f)!=1 || fread(&cpu,sizeof cpu,1,f)!=1 || fread(&sys,sizeof sys,1,f)!=1)return 4;
    uint8_t *ram=malloc(0x1000000);if(!ram)return 5;
    FILE *memory=fopen(argv[2],"rb");if(!memory || fread(ram,0x1000000,1,memory)!=1 || fgetc(memory)!=EOF)return 6;
    fclose(memory);
    fixture_restore(&context,f,&cpu,&sys,ram,0x1000000);
    if(fgetc(f)!=EOF || fclose(f))return 4;
    int result=0;
    if(op[0]==0)fist_cpu_hw_interrupt(&context.bus,op[1]);
    else if(op[0]==1)fist_cpu_iret(&context.bus,op[1]);
    else if(op[0]==7)fist_cpu_sw_interrupt(&context.bus,op[1],op[2]);
    else if(op[0]==8)fist_cpu_far_ret(&context.bus,op[1],op[2]);
    else if(op[0]==2)fist_cpu_lgdt(&sys,op[1],op[2]);
    else if(op[0]==3)fist_cpu_lidt(&sys,op[1],op[2]);
    else if(op[0]==5 || op[0]==6) {
        if(op[0]==6) {
            context.handlers[4].read=observed_read;
            context.handlers[4].write=observed_write;
        }
        result=fist_cpu_ltr(&context.bus,op[1]);
    }
    else return 7;
    uint32_t words[58];memcpy(words,&cpu,sizeof cpu);memcpy(words+37,&sys,sizeof sys);
    for(unsigned i=0;i<58;++i)printf(i ? " %08x" : "%08x",words[i]);
    puts("");
    f=fopen(argv[3],"wb");if(!f || fwrite(ram,0x1000000,1,f)!=1 || fclose(f))return 8;
    FILE *expected=fopen(argv[4],"rb");if(!expected)return 9;
    uint8_t buffer[4096];size_t at=0,count;
    while((count=fread(buffer,1,sizeof buffer,expected))) {
        if(at+count>0x1000000 || memcmp(buffer,ram+at,count))return 10;
        at+=count;
    }
    if(ferror(expected) || at!=0x1000000)return 11;
    fclose(expected);
    f=fopen(argv[5],"wb");if(!f)return 12;
    fixture_cache(&context,f);if(fclose(f))return 13;
    fixture_destroy(&context);free(ram);puts("complete-RAM 16777216");
    if(op[0]==5 || op[0]==6)printf("return %d\n",result);
    return 0;
}
