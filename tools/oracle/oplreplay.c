/* oplreplay -- synthesize a CLEAN oracle reference WAV from a captured OPL
 * register+timestamp stream, through the SAME DBOPL core the port uses.
 *
 * The committed ref/audio_menu_oracle.wav is a 12.7 s STEREO clip with menu-click
 * SFX and no clean loop (docs/audio.md §23) -> useless for a bit-exact xcorr.  This
 * tool takes the trace_opl.sh capture ($FISTOPLLOG, lines "... t=<ms> ... val=<vv>
 * DATA reg=<rr> ...") and replays it through DBOPL at the captured PIC-ms timing,
 * giving a full-length mono 44.1 kHz reference that is synthesis- and format-matched
 * to the port's FIST_AUDIO_WAV -> a meaningful port-vs-oracle envelope xcorr.
 *
 *   build: g++ -O2 -c re_out/fist_opl_dbopl.cpp -o /tmp/dbopl.o -Ire_out
 *          g++ -O2 -c re_out/opl/dbopl.cpp     -o /tmp/dboplcore.o -Ire_out/opl
 *          gcc -O2 tools/oracle/oplreplay.c /tmp/dbopl.o /tmp/dboplcore.o -o oplreplay -lstdc++ -lm
 *   usage: oplreplay <opl-capture-log> <out.wav> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void fist_dbopl_reset(unsigned rate);
void fist_dbopl_write(unsigned reg, unsigned char val);
unsigned fist_dbopl_generate(unsigned nsamp, int *out);
#define SR 44100
static int buf[4096];
static void wr32(FILE*f,unsigned v){for(int i=0;i<4;i++)fputc((v>>(8*i))&0xff,f);}
static void wr16(FILE*f,unsigned v){fputc(v&0xff,f);fputc((v>>8)&0xff,f);}
int main(int argc,char**argv){
  FILE*in=fopen(argv[1],"r"); FILE*out=fopen(argv[2],"wb");
  fist_dbopl_reset(SR);
  fwrite("RIFF",1,4,out);wr32(out,0);fwrite("WAVE",1,4,out);fwrite("fmt ",1,4,out);wr32(out,16);
  wr16(out,1);wr16(out,1);wr32(out,SR);wr32(out,SR*2);wr16(out,2);wr16(out,16);
  fwrite("data",1,4,out);long doff=ftell(out);wr32(out,0);
  char line[4096]; long t0=-1; long tprev=0; unsigned written=0; double acc=0;
  while(fgets(line,sizeof line,in)){
    char*p=strstr(line,"t="); if(!p)continue; long t=atol(p+2);
    char*pv=strstr(line,"val="); char*pr=strstr(line,"reg=");
    /* only DATA lines have "DATA reg=" */
    char*pd=strstr(line,"DATA reg="); if(!pd)continue;
    unsigned reg=strtoul(pd+9,0,16); unsigned val=strtoul(pv+4,0,16);
    if(t0<0){t0=t;tprev=t;}
    /* generate samples for (t-tprev) ms */
    double ns=(double)(t-tprev)*SR/1000.0; acc+=ns; unsigned n=(unsigned)acc; acc-=n; tprev=t;
    while(n){unsigned c=n>4096?4096:n; unsigned got=fist_dbopl_generate(c,buf);
      for(unsigned i=0;i<got;i++){int s=buf[i];if(s>32767)s=32767;if(s<-32768)s=-32768;wr16(out,(unsigned short)s);written+=2;} n-=c;}
    fist_dbopl_write(reg,(unsigned char)val);
  }
  long end=ftell(out);fseek(out,4,SEEK_SET);wr32(out,end-8);fseek(out,doff,SEEK_SET);wr32(out,written);
  fclose(out);fclose(in);
  fprintf(stderr,"replayed -> %s (%u samples, %.1fs)\n",argv[2],written/2,(written/2.0)/SR);
  return 0;
}
