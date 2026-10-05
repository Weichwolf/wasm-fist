/* Extend the one IRET/PIC/frame fixture through the actual first IRQ0 prefix. */
#define FIST_CORE_EXIT_CONTINUE 1
#include "fist_cpu.h"
#include "cpu_core_exit.c"
static void handler_prefix(void)
{
    FistExec engine={.bus=&context.bus};
    for(unsigned count=0;;count++) {
        observe_cpu("fetch");
        unsigned op=fist_ram_resident_read(&context.bus,1,cpu.eip,1);
        /* First I/O is the source-observed boundary, before its body. */
        if(op==0xec) {state("first-io");return;}
        fist_cpu_require(count<100);
        unsigned checks=fist_exec_fetched(&engine),trap_decoder=0;
        fist_cpu_require(!fist_exec_core_exit(&engine,checks,fist_pic_pending_irqs(),&trap_decoder));
        fist_clock_charge_cpu_instructions(1);
    }
}
