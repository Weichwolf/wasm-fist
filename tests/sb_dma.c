#include "fist_sb.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifndef ORIGINAL_SB
uint8_t g_mem[0x1000000];
uint32_t fist_ext_base;
extern unsigned short m_ext_FUN_0000_2810(void);
/* Record the actual translated producer's byte OUTs, then let the real device
 * owner consume them. This endpoint checks device state, not I/O clock costs. */
void out(int port, int value)
{
    port &= 0xffff; value &= 0xff;
    fprintf(stderr, "out %x %x\n", port, value);
    fist_sb_out(port, value);
}
int in(int port) { return fist_sb_in(port); }
#else
extern uint8_t *g_mem;
void original_sb_init(void);
#endif

static unsigned irqs;
static void irq(void) { ++irqs; }

int main(void)
{
    setenv("FIST_SB", "1", 1);
#ifdef ORIGINAL_SB
    original_sb_init();
#endif
    /* Exercise changing DMA bytes, including both halves and page boundaries. */
    for (unsigned i=0; i<0x1000000; ++i)
        g_mem[i]=(uint8_t)((i*29)^(i>>8));
    fist_sb_set_irq_cb(irq);
    static unsigned char data[65536];
    char op;
    unsigned a,b;
    while (scanf(" %c", &op)==1) {
#ifndef ORIGINAL_SB
        if (op=='i') {
            char path[512];
            if (scanf("%x %x %511s", &a, &b, path)!=3) return 2;
            fist_ext_base=0x100000;
            uint8_t *module=g_mem+fist_ext_base;
            FILE *image=fopen(path,"rb");
            assert(image);
            size_t size=fread(module,1,0x10000,image);
            assert(size && feof(image) && !ferror(image) && !fclose(image));
            memcpy(module+0x12c8,&a,4); memcpy(module+0x23e8,&b,4);
            static uint8_t before[0x10000]; memcpy(before,module,sizeof before);
            fprintf(stderr,"return %x\n",m_ext_FUN_0000_2810());
            assert(!memcmp(before,module,sizeof before));
        } else
#endif
        if (op=='w') {
            if (scanf("%x %x", &a, &b)!=2) return 2;
            fist_sb_out(a,b);
        } else if (op=='r') {
            if (scanf("%x", &a)!=1) return 2;
            printf("read %x %x\n",a,fist_sb_in(a));
        } else if (op=='d') {
            if (scanf("%u", &a)!=1 || a>sizeof(data)) return 2;
            unsigned n=fist_sb_read_pcm8(a,data);
            printf("data %u ",n);
            for (unsigned i=0; i<n; ++i) printf("%02x",data[i]);
            puts("");
        } else if (op=='p') {
            fist_sb_pump();
        } else if (op=='s') {
            printf("state %u %u %u %d\n",fist_sb_dma_left(),
                   fist_sb_ring_count(),irqs,fist_sb_rate());
        } else return 2;
    }
    fist_sb_flush();
    return 0;
}
