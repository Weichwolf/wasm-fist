#define FIST_TEST_EXT_BASE
#include "sb_clock_fixture.h"
#include "pic_controller_setup.h"
#include <string.h>

#ifdef FIST_PARENT_NO_PIC_CLOCK
/* Parent producer has no queue handoff for newly exposed PIC state. These
 * reaching port-only cases install no unmasked request and never use it. */
void fist_clock_pic_requeue(void) { abort(); }
#endif

int main(int argc, char **argv)
{
    assert(argc>=3 && argc<=5);
    if (!strncmp(argv[argc-1],"wav:",4)) {
        setenv("FIST_AUDIO_WAV",argv[argc-1]+4,1);
        --argc;
    }
    assert(argc==3 || argc==4);
    setenv("FIST_SB","1",1);
    for (unsigned i=0;i<0x10000;++i) g_mem[i]=(unsigned char)((i*29)^(i>>8));
    if (argc==4) {
        assert(!strcmp(argv[3],"sb-irq"));
        pic_controller_setup_sb_irq(fist_pic_write,fist_pic_take_irq);
    }
    uint64_t current,start=strtoull(argv[1],NULL,10);
    extern unsigned fist_clock_cpu_slice(uint64_t *);
    extern void fist_clock_advance_cpu_cycles(unsigned);
    fist_clock_cpu_slice(&current);
    assert(start>=current && start-current<=UINT32_MAX);
    fist_clock_advance_cpu_cycles((unsigned)(start-current));
    FILE *script=fopen(argv[2],"r"); assert(script);
    char op; unsigned port,value,count=0; int fields;
    static unsigned char data[65536];
    while ((fields=fscanf(script," %c %x %x",&op,&port,&value))==3) {
        fist_clock_charge_cpu_instructions(1);
        uint64_t fetched; fist_clock_cpu_slice(&fetched);
        if (op=='r') {
            unsigned actual=in(port);
            printf("read %x %x\n",port,actual);
        } else if (op=='w') out(port,value);
        else if (op=='h') fist_pic_activate_irq(port);
        else if (op=='l') fist_pic_deactivate_irq(port);
        else if (op=='m') fist_pic_set_irq_mask(port,value!=0);
        else if (op=='j') {
            unsigned vector=0; int selected=fist_pic_take_irq(port,value!=0,&vector);
            printf("irq %d %d\n",selected,selected==-1 ? -1 : (int)vector);
        } else if (op=='g') {
            assert(value<=sizeof data); unsigned n=fist_sb_read_pcm8(value,data);
            printf("data %u ",n); for (unsigned i=0;i<n;++i) printf("%02x",data[i]); puts("");
        } else if (op=='s') {
            printf("state %u %u %d\n",fist_sb_dma_left(),fist_sb_ring_count(),fist_sb_rate());
        } else assert(op=='n');
        uint64_t after; unsigned remaining=fist_clock_cpu_slice(&after);
        printf("clock %llu %llu %u\n",(unsigned long long)fetched,(unsigned long long)after,remaining);
        ++count;
    }
    assert(fields==EOF && !ferror(script) && !fclose(script) && count);
    printf("pumps %u\n",pumps);
    fist_sb_flush();
}
