/* Shared explicit inputs for original and portable shift/flag programs. */

int main(int argc,char **argv) {
 SHR_REQUIRE(argc==2);output=fopen(argv[1],"wb");SHR_REQUIRE(output!=NULL);
 unsigned cases=0;
 for(unsigned mode=0;mode<2;mode++) {
  for(unsigned a=0;a<256;a++)for(unsigned count=0;count<32;count++) {run_case(8,a,count,mode);cases++;}
  for(unsigned a=0;a<65536;a++) {run_case(16,a,1,mode);cases++;}
  unsigned words[]={0,1,2,0x7fff,0x8000,0x8001,0xffff};
  for(unsigned i=0;i<sizeof words/sizeof *words;i++)for(unsigned count=0;count<32;count++) {run_case(16,words[i],count,mode);cases++;}
  unsigned dwords[]={0,1,2,0x7fff,0x8000,0x8001,0xffff,0x10000,0x7fffffff,0x80000000,0x80000001,0xffffffff};
  for(unsigned i=0;i<sizeof dwords/sizeof *dwords;i++)for(unsigned count=0;count<32;count++) {run_case(32,dwords[i],count,mode);cases++;}
 }
 SHR_REQUIRE(cases==148672);SHR_REQUIRE(!ferror(output) && !fclose(output));return 0;
}
