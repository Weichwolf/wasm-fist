#include "fist_interrupt.h"
#include <stdio.h>
_Static_assert(sizeof(FistCpuState)==37*4,"CPU words");
_Static_assert(sizeof(FistCpuSystem)==21*4,"system words");
static FistCpuState cpu;
static FistCpuSystem sys;
int main(int argc,char **argv)
{
    if(argc!=5)return 2;
    FILE *f=fopen(argv[1],"rb");if(!f)return 3;
    uint32_t op[3];
    if(fread(op,sizeof op,1,f)!=1 || fread(&cpu,sizeof cpu,1,f)!=1 || fread(&sys,sizeof sys,1,f)!=1 || fgetc(f)!=EOF)return 4;
    fclose(f);
    uint8_t *ram=malloc(0x1000000);if(!ram)return 5;
    f=fopen(argv[2],"rb");if(!f || fread(ram,0x1000000,1,f)!=1 || fgetc(f)!=EOF)return 6;
    fclose(f);
    int result=0;
    if(op[0]==0)fist_cpu_hw_interrupt(&cpu,&sys,ram,0x1000000,op[1]);
    else if(op[0]==1)fist_cpu_iret(&cpu,&sys,ram,0x1000000,op[1]);
    else if(op[0]==2)fist_cpu_lgdt(&sys,op[1],op[2]);
    else if(op[0]==3)fist_cpu_lidt(&sys,op[1],op[2]);
    else if(op[0]==5)result=fist_cpu_ltr(&cpu,&sys,ram,0x1000000,op[1]);
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
    fclose(expected);free(ram);puts("complete-RAM 16777216");
    if(op[0]==5)printf("return %d\n",result);
    return 0;
}
