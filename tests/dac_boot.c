/* Exercise the actual initial clock/palette constructor with fixture host hooks. */
#define fist_int8_fire unused_int8
#include "sb_clock_fixture.h"
#undef fist_int8_fire
#include "fist_cpu.h"
void fist_int8_fire(void) {abort();}
extern void write_boot_palette(FILE *);
int main(int argc,char **argv)
{
 fist_cpu_require(argc==2);
 FILE *output=fopen(argv[1],"wb");fist_cpu_require(output!=NULL);
 write_boot_palette(output);fist_cpu_require(!fclose(output));return 0;
}
