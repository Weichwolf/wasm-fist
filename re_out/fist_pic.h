#ifndef FIST_PIC_H
#define FIST_PIC_H
/* Shared clock-owned DOSBox PIC callbacks. Delays are float milliseconds;
 * callbacks re-arm relative to their scheduled index, even when serviced late. */
typedef void (*FistPicEvent)(unsigned value);
void fist_clock_add_event(FistPicEvent handler, float delay, unsigned value);
void fist_clock_remove_events(FistPicEvent handler);
void fist_clock_remove_specific_events(FistPicEvent handler, unsigned value);
/* VGA-machine 8259 controller state. Taking an eligible IRQ returns its line
 * and vector; the CPU owner must construct the interrupt frame before executing
 * the handler. This API does not supply a protected-mode CPU/vector/IRET owner. */
int fist_pic_read(unsigned port);
void fist_pic_write(unsigned port, unsigned value);
void fist_pic_activate_irq(unsigned irq);
void fist_pic_deactivate_irq(unsigned irq);
void fist_pic_set_irq_mask(unsigned irq, int masked);
int fist_pic_take_irq(unsigned flags, int trap_decoder, unsigned *vector);
#endif
