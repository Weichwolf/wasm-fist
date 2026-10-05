/* Observe the same controller owner used by the reaching CPU fixture. */
#include "cpu_core_exit_pic.c"
void lifecycle_irqs(uint32_t *q)
{initialize();for(unsigned i=0;i<16;i++) {q[4*i]=irqs[i].active;q[4*i+1]=irqs[i].masked;q[4*i+2]=irqs[i].inservice;q[4*i+3]=irqs[i].vector;}q[64]=pending;q[65]=active_irq;}
