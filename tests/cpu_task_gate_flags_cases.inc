/* Identical explicit inputs for the original and portable flag producers. */
int main(int argc,char **argv) {
 CHECK(argc==3 || argc==4);output=fopen(argv[1],"wb");CHECK(output!=NULL);
 unsigned edge=argc==4,cases=0;
 if(!strcmp(argv[2],"sub-word")) {
  unsigned values[]={0,1,15,16,0x17f,0x180,0x181,0x7fff,0x8000,0x8001,0xfffe,0xffff};
  for(unsigned mode=0;mode<2;mode++) {
   if(!edge)for(unsigned a=0;a<65536;a++){run_sub(a,0x180,mode);cases++;}
   for(unsigned i=0;i<sizeof values/sizeof *values;i++)for(unsigned j=0;j<sizeof values/sizeof *values;j++) {
    run_sub(values[i],values[j],mode);cases++;
   }
  }
  CHECK(cases==(edge?288:131360));
 }else {
  CHECK(!strcmp(argv[2],"shl"));
  for(unsigned mode=0;mode<2;mode++) {
   for(unsigned a=0;a<256;a+=edge?255:1)for(unsigned count=0;count<32;count++){run_shl(8,a,count,mode);cases++;}
   for(unsigned a=0;a<65536;a+=edge?65535:1){run_shl(16,a,1,mode);cases++;}
   for(unsigned a=0;a<65536;a+=edge?65535:1)for(unsigned sign=0;sign<2;sign++){run_shl(32,a|(sign?0x80000000u:0),4,mode);cases++;}
   unsigned words[]={0,1,2,0x7fff,0x8000,0x8001,0xffff};
   for(unsigned i=0;i<sizeof words/sizeof *words;i++)for(unsigned count=0;count<32;count++){run_shl(16,words[i],count,mode);cases++;}
   unsigned dwords[]={0,1,2,0x7fff,0x8000,0x8001,0xffff,0x10000,0x7fffffff,0x80000000,0x80000001,0xffffffff};
   for(unsigned i=0;i<sizeof dwords/sizeof *dwords;i++)for(unsigned count=0;count<32;count++){run_shl(32,dwords[i],count,mode);cases++;}
  }
  CHECK(cases==(edge?1356:410816));
 }
 CHECK(!ferror(output) && !fclose(output));return 0;
}
