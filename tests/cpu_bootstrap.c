#include "memory_context_fixture.h"
/* Replay reached original bootstrap bytes through the shared CPU/memory owner. */
#include "fist_interrupt.h"
#include <stdio.h>
static FistCpuState cpu;
static FistCpuSystem sys;
static uint8_t *ram;
static FistMemoryFixture context;
#define bus context.bus
static uint32_t cursor;
static unsigned instructions;
static uint32_t clock_words[4];
static const char *output_path;
static uint32_t fetch(unsigned width)
{
    uint32_t value=fist_ram_resident_read(&bus,1,cursor,width);
    cursor+=width;
    return value;
}
static void report(const char *label)
{
    uint32_t q[58];memcpy(q,&cpu,sizeof cpu);memcpy(q+37,&sys,sizeof sys);
    printf("%s",label);
    for(unsigned i=0;i<58;++i)printf(" %08x",q[i]);
    printf(" clock %u %u %u %u\n",clock_words[0],clock_words[1],clock_words[2],clock_words[3]);
}
static void descriptor_table(unsigned which,unsigned segment,uint32_t offset)
{
    uint32_t limit=fist_ram_resident_read(&bus,segment,offset,2);
    uint32_t base=fist_ram_resident_read(&bus,segment,offset+2,4)&0xffffff;
    if(which==2)fist_cpu_lgdt(&sys,limit,base);
    else {fist_cpu_require(which==3);fist_cpu_lidt(&sys,limit,base);}
    report(which==2 ? "LGDT" : "LIDT");
}
int main(int argc,char **argv)
{
    if(argc!=4)return 2;
    FILE *f=fopen(argv[1],"rb");if(!f)return 3;
    if(fread(&cpu,sizeof cpu,1,f)!=1 || fread(&sys,sizeof sys,1,f)!=1 || fread(clock_words,sizeof clock_words,1,f)!=1 )return 4;
    FILE *input=f;
    ram=malloc(0x1000000);if(!ram)return 5;
    f=fopen(argv[2],"rb");if(!f || fread(ram,0x1000000,1,f)!=1 || fgetc(f)!=EOF)return 6;fclose(f);
    fixture_restore(&context,input,&cpu,&sys,ram,0x1000000);
    if(fgetc(input)!=EOF || fclose(input))return 4;
    cursor=cpu.eip;output_path=argv[3];
    for(;;) {
        if(instructions)clock_words[0]--;
        cpu.eip=cursor;instructions++;
        if(instructions>1) {
            char label[64],path[1024];
            snprintf(label,sizeof label,"fetch-%u",instructions-1);report(label);
            snprintf(path,sizeof path,"%s-fetch-%u.memory",output_path,instructions-1);
            f=fopen(path,"wb");if(!f || fwrite(ram,0x1000000,1,f)!=1 || fclose(f))return 8;
            snprintf(path,sizeof path,"%s-fetch-%u.context",output_path,instructions-1);
            f=fopen(path,"wb");if(!f)return 9;fixture_cache(&context,f);if(fclose(f))return 9;
        }
        fist_cpu_require(cpu.code_big==0 && !(cpu.flags.flags&FIST_FLAG_VM));
        unsigned operand32=0,segment=3,opcode=fetch(1);
        while(opcode==0x36 || opcode==0x2e || opcode==0x66) {
            if(opcode==0x36)segment=2;
            else if(opcode==0x2e)segment=1;
            else operand32=!operand32;
            opcode=fetch(1);
        }
        if(opcode==0x0f) {
            unsigned second=fetch(1),rm=fetch(1),which=(rm>>3)&7;
            if(second==1 || second==0) {
                fist_cpu_require(!operand32 && (rm&0xc7)==6);
                uint32_t offset=fetch(2);
                if(second==1)descriptor_table(which,segment,offset);
                else {
                    fist_cpu_require(which==3 && cpu.pmode && cpu.cpl==0);
                    uint32_t selector=fist_ram_resident_read(&bus,segment,offset,2);
                    int result=fist_cpu_ltr(&bus,selector);
                    report("LTR");printf("return %d instructions %u\n",result,instructions);
                    f=fopen(argv[3],"wb");if(!f || fwrite(ram,0x1000000,1,f)!=1 || fclose(f))return 7;
                    char path[1024];snprintf(path,sizeof path,"%s.context",output_path);
                    f=fopen(path,"wb");if(!f)return 9;fixture_cache(&context,f);if(fclose(f))return 9;
                    fixture_destroy(&context);free(ram);return result;
                }
            } else {
                fist_cpu_require(second==0x22 && rm>=0xc0 && (which==0 || which==3));
                uint32_t registers[8];memcpy(registers,&cpu,sizeof registers);
                uint32_t value=registers[rm&7];
                if(which==3)fist_ram_set_cr3(&bus,value);
                else fist_ram_set_cr0_normal(&bus,value);
            }
        } else if(opcode==0xa1) {
            uint32_t offset=fetch(2);
            fist_cpu_require(operand32);
            cpu.eax=fist_ram_resident_read(&bus,segment,offset,4);
        } else if(opcode==0xea) {
            fist_cpu_require(!operand32 && cpu.pmode && cpu.cpl==0);
            uint32_t offset=fetch(2),selector=fetch(2);FistCpuDescriptor d;
            fist_cpu_require(fist_cpu_descriptor(&bus,selector,&d));
            fist_cpu_require(fist_descriptor_present(d) && fist_descriptor_nonconforming_code(d) && fist_descriptor_dpl(d)==cpu.cpl);
            cpu.segments[1].value=(selector&0xfffc)|cpu.cpl;
            cpu.segments[1].base=fist_descriptor_base(d);cpu.code_big=fist_descriptor_big(d);
            cursor=offset;cpu.eip=offset;
        } else if(opcode==0x8e) {
            unsigned rm=fetch(1);fist_cpu_require(!operand32 && ((rm>>3)&7)==2 && (rm&0xc7)==6);
            uint32_t offset=fetch(2),selector=fist_ram_resident_read(&bus,segment,offset,2);
            FistCpuDescriptor d;fist_cpu_require(fist_cpu_descriptor(&bus,selector,&d));
            fist_cpu_require(cpu.pmode && (selector&3)==cpu.cpl && fist_descriptor_dpl(d)==cpu.cpl && fist_descriptor_present(d) && fist_descriptor_writable_stack(d));
            fist_cpu_select_stack(&cpu,selector,cpu.esp,d);
            /* core_normal MOV SS: CPU_Cycles++ keeps the following instruction in this slice. */
            clock_words[0]++;
        } else if(opcode==0x80) {
            unsigned rm=fetch(1);fist_cpu_require(((rm>>3)&7)==4 && (rm&0xc7)==6);
            uint32_t offset=fetch(2),b=fetch(1),a=fist_ram_resident_read(&bus,segment,offset,1),value=a&b;
            fist_cpu_alu(&cpu,FIST_LAZY_ANDB,8,a,b,value);
            fist_ram_resident_write(&bus,segment,offset,1,value);
        } else abort();
    }
}
