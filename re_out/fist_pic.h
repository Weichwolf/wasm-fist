#ifndef FIST_PIC_H
#define FIST_PIC_H
/* Shared clock-owned DOSBox PIC callbacks. Delays are float milliseconds;
 * callbacks re-arm relative to their scheduled index, even when serviced late. */
typedef void (*FistPicEvent)(unsigned value);
void fist_clock_add_event(FistPicEvent handler, float delay, unsigned value);
void fist_clock_remove_events(FistPicEvent handler);
void fist_clock_remove_specific_events(FistPicEvent handler, unsigned value);
#endif
