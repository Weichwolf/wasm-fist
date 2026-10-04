/* Restore/observe the actual one PIC owner, without a shadow controller. */
#include "fist_cpu.h"
#include "fist_pic.c"
static unsigned word(FILE *input)
{
    uint32_t value;fist_cpu_require(fread(&value,4,1,input)==1);return value;
}
void restore_core_pic(FILE *input)
{
    initialize();
    for(unsigned i=0;i<16;i++) {
        irqs[i].masked=word(input);irqs[i].active=word(input);
        irqs[i].inservice=word(input);irqs[i].vector=word(input);
    }
    for(unsigned i=0;i<2;i++) {
        pics[i].icw_words=word(input);pics[i].icw_index=word(input);
        pics[i].special=word(input);pics[i].auto_eoi=word(input);
        pics[i].rotate_auto_eoi=word(input);pics[i].single=word(input);
        pics[i].read_service=word(input);
    }
    pending=0;for(unsigned i=0;i<16;i++)check_line(i);
    fist_cpu_require(pending==word(input));active_irq=word(input);
    fist_cpu_require(active_irq==255 || (active_irq<16 && irqs[active_irq].inservice));
}
void observe_core_pic(FILE *output)
{
    uint32_t q[80];unsigned n=0;
    for(unsigned i=0;i<16;i++) {
        q[n++]=irqs[i].masked;q[n++]=irqs[i].active;
        q[n++]=irqs[i].inservice;q[n++]=irqs[i].vector;
    }
    for(unsigned i=0;i<2;i++) {
        q[n++]=pics[i].icw_words;q[n++]=pics[i].icw_index;
        q[n++]=pics[i].special;q[n++]=pics[i].auto_eoi;
        q[n++]=pics[i].rotate_auto_eoi;q[n++]=pics[i].single;
        q[n++]=pics[i].read_service;
    }
    q[n++]=pending;q[n++]=active_irq;fist_cpu_require(n==80);
    fist_cpu_require(fwrite(q,sizeof q,1,output)==1);
}
unsigned observe_core_active(void) {initialize();return active_irq;}
