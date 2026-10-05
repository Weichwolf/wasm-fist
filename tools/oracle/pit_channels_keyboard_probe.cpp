#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/keyboard.cpp"
extern "C" unsigned full_pit_gate_state(void);
extern "C" void full_ppi_write(unsigned value) {write_p61(0x61,value,1);}
extern "C" unsigned full_ppi_read(void) {return read_p61(0x61,1);}
extern "C" unsigned full_ppi_data(void) {return port_61_data;}
void PCSPEAKER_SetType(Bitu type)
{
#ifndef SILENT_SPEAKER_OBSERVER
 printf("{\"kind\":\"speaker-type-request\",\"type\":%u,\"gate2\":%u,\"port61\":%u}\n",(unsigned)type,full_pit_gate_state(),(unsigned)port_61_data);
#endif
}
