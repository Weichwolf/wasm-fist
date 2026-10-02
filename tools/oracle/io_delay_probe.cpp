/* Keep the original I/O source in its own translation unit (PIC has private names in common). */
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/iohandler.cpp"

void source_io_read_delay(void) { IO_USEC_read_delay(); }
void source_io_write_delay(void) { IO_USEC_write_delay(); }
