/* Controlled original status/I/O producer; no event dispatch is claimed. */
#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/vga_misc.cpp"
#include <assert.h>
VGA_Type vga;
Segments Segs;
CPU_Regs cpu_regs;
Bit32s CPU_Cycles,CPU_CycleLeft,CPU_CycleMax=30000;
Bit64s CPU_IODelayRemoved;
Bitu PIC_Ticks;
extern void source_io_read_delay(void);
int main(int argc,char **argv) {
 FILE *input=fopen(argv[1],"rb"),*output=fopen(argv[2],"wb");assert(input && output);
 double d[8];unsigned q[7];size_t count;
 while((count=fread(d,1,sizeof d,input))==sizeof d) {
  assert(fread(q,sizeof q,1,input)==1);
  vga.draw.delay.framestart=d[0];vga.draw.delay.vrstart=d[1];vga.draw.delay.vrend=d[2];
  vga.draw.delay.hblkstart=d[3];vga.draw.delay.hblkend=d[4];vga.draw.delay.htotal=d[5];
  vga.draw.delay.vdend=d[6];vga.draw.delay.vtotal=d[7];
  vga.internal.attrindex=q[0];vga.tandy.pcjr_flipflop=q[1];PIC_Ticks=q[2];CPU_Cycles=q[3];CPU_CycleLeft=q[4];
  CPU_IODelayRemoved=((Bit64s)q[6]<<32)|q[5];source_io_read_delay();
  unsigned result[8]={(unsigned)vga_read_p3da(0x3da,1),(unsigned)PIC_Ticks,(unsigned)CPU_Cycles,(unsigned)CPU_CycleLeft,(unsigned)CPU_IODelayRemoved,(unsigned)(CPU_IODelayRemoved>>32),(unsigned)vga.internal.attrindex,(unsigned)vga.tandy.pcjr_flipflop};
  assert(fwrite(result,sizeof result,1,output)==1);
 }
 assert(!count && feof(input) && !ferror(input) && !fclose(input) && !fclose(output));return 0;
}
