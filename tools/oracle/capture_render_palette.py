#!/usr/bin/env python3
"""Compare literal original renderer palette functions and complete host calls."""
import argparse,hashlib,json,re
from pathlib import Path
from palette_control_fixture import build,compare,digest,run


def fixtures(repo,root):
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    source=(tree/'src/gui/render.cpp').read_text()
    check=re.search(r'static void Check_Palette\(void\) \{.*?\n\}',source,re.S)[0]
    set_pal=re.search(r'void RENDER_SetPal\(.*?\n\}',source,re.S)[0]
    reset=source.split('/* Reset the palette change detection',1)[1].split('//Finish this frame',1)[0].split('*/',1)[1]
    original='''#include <stdint.h>
    #include <string.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include "dosbox.h"
    #include "render.h"
    #include "video.h"
    #include "render_scalers.h"
    Render_t render;
    static uint32_t rgb_provider(uint8_t,uint8_t,uint8_t);
    static void palette_provider(uint64_t,uint64_t,const void *);
    Bitu GFX_GetRGB(Bit8u r,Bit8u g,Bit8u b){return rgb_provider(r,g,b);}
    void GFX_SetPalette(Bitu first,Bitu count,GFX_PalEntry *rgb){palette_provider(first,count,rgb);}
    '''+check+'\n'+set_pal+'\nstatic void palette_reset(void){'+reset+'}\n'+'''
    #define P render.pal
    static void consume(unsigned mode){render.scale.outMode=(decltype(render.scale.outMode))mode;Check_Palette();}
    static void set_pal(unsigned i,unsigned r,unsigned g,unsigned b){RENDER_SetPal(i,r,g,b);}
    #include "cases.inc"
    '''
    portable='''#include <stdio.h>
    #include <stdlib.h>
    #include "fist_render_palette.h"
    static FistRenderPalette palette;
    static uint32_t rgb_provider(uint8_t,uint8_t,uint8_t);
    static void palette_provider(uint64_t,uint64_t,const void *);
    static uint32_t get_rgb(void *p,uint8_t r,uint8_t g,uint8_t b){return rgb_provider(r,g,b);}
    static void set_palette(void *p,uint64_t first,uint64_t count,const void *rgb){palette_provider(first,count,rgb);}
    static void palette_reset(void){fist_render_palette_reset(&palette);}
    static void consume(unsigned mode){fist_render_check_palette(&palette,mode,NULL,get_rgb,set_palette);}
    static void set_pal(unsigned i,unsigned r,unsigned g,unsigned b){fist_render_set_pal(&palette,i,r,g,b);}
    #define P palette
    #include "cases.inc"
    '''
    common=r'''
    #include <stdint.h>
    #include <string.h>
    static FILE *output;static unsigned records,host_variant,rgb_count,palette_count;
    static uint32_t calls[256][4];static uint64_t palette_first,palette_entries;static unsigned char palette_rgb[1024];
    static uint32_t rgb_provider(uint8_t r,uint8_t g,uint8_t b) {
     if(rgb_count>=256)abort();uint32_t result=((uint32_t)r<<16)|((uint32_t)g<<8)|b;
     if(host_variant)result^=0xabcde123;
     uint32_t *q=calls[rgb_count++];q[0]=r;q[1]=g;q[2]=b;q[3]=result;return result;
    }
    static void palette_provider(uint64_t first,uint64_t count,const void *rgb) {
     if(palette_count++ || first>255 || count>256 || first+count>256)abort();
     palette_first=first;palette_entries=count;memcpy(palette_rgb,rgb,count*4);
    }
    static void clear_calls(void){rgb_count=palette_count=0;memset(calls,0,sizeof calls);palette_first=palette_entries=0;memset(palette_rgb,0,sizeof palette_rgb);}
    static void init(unsigned variant,unsigned changed,uint64_t first,uint64_t last){
     memset(&P,0x9b,sizeof P);P.changed=changed;P.first=first;P.last=last;host_variant=variant;
     for(unsigned i=0;i<256;i++){P.rgb[i].red=i*13+variant*91;P.rgb[i].green=i*29+variant*47;P.rgb[i].blue=i*41+variant*11;P.rgb[i].unused=0xd5;P.modified[i]=i*7+variant;}
     clear_calls();
    }
    static void record(void){
    #define EMIT(v) do {if(fwrite(&(v),sizeof(v),1,output)!=1)abort();}while(0)
     EMIT(P.rgb);EMIT(P.lut);EMIT(P.modified);uint8_t changed=P.changed;EMIT(changed);uint64_t first=P.first,last=P.last;EMIT(first);EMIT(last);
     uint32_t count=rgb_count,pc=palette_count;EMIT(count);if(fwrite(calls,count*16,1,output)!=1 && count)abort();EMIT(pc);EMIT(palette_first);EMIT(palette_entries);if(fwrite(palette_rgb,palette_entries*4,1,output)!=1 && palette_entries)abort();records++;
    #undef EMIT
    }
    int main(int argc,char **argv){
     if(argc!=2)abort();output=fopen(argv[1],"wb");if(!output)abort();
     unsigned modes[]={0,1,2,3,17};
     for(unsigned m=0;m<5;m++)for(unsigned changed=0;changed<2;changed++)for(unsigned variant=0;variant<2;variant++)for(unsigned first=0;first<=256;first++) {
      unsigned lasts[]={0,255,first?first-1:0,first<256?first:255,first<255?first+1:0,127};
      for(unsigned j=0;j<6;j++){init(variant,changed,first,lasts[j]);consume(modes[m]);record();clear_calls();consume(modes[m]);record();}
     }
     /* Every RGB component/byte and entry, retaining the complete dirty palette, LUT and metadata. */
     for(unsigned entry=0;entry<256;entry++)for(unsigned lane=0;lane<3;lane++)for(unsigned value=0;value<256;value++){
      init(entry&1,1,UINT64_C(0x100000100),0);unsigned rgb[]={P.rgb[entry].red,P.rgb[entry].green,P.rgb[entry].blue};rgb[lane]=value;set_pal(entry,rgb[0],rgb[1],rgb[2]);record();
     }
     for(unsigned changed=0;changed<2;changed++)for(unsigned variant=0;variant<2;variant++)for(unsigned first=0;first<=256;first++){init(variant,changed,first,first&255);palette_reset();record();}
     if(fclose(output))abort();fprintf(stderr,"records=%u\n",records);return 0;
    }
    '''
    (root/'original.cpp').write_text(original);(root/'portable.c').write_text(portable);(root/'cases.inc').write_text(common)


def causal(repo,root):
    work=root/'causal';work.mkdir()
    common=(root/'cases.inc').read_text().split('int main(',1)[0]+'''int main(int argc,char **argv){
     if(argc!=2)abort();output=fopen(argv[1],"wb");if(!output)abort();
     init(0,1,256,0);consume(3);record();
     init(0,0,128,128);consume(2);record();
     init(1,1,77,129);palette_reset();record();
     init(0,1,UINT64_C(0x100000100),0);set_pal(63,23,41,59);record();
     if(fclose(output))abort();return 0;}
    '''
    (work/'cases.inc').write_text(common);(work/'original.cpp').write_bytes((root/'original.cpp').read_bytes())
    command=build(repo,work,original=True)[0][1]
    run(command,work/'original.raw');expected=(work/'original.raw').read_bytes();results=[]
    header=(repo/'re_out/fist_render_palette.h').read_text()
    mutants={
     'preserve-dirty-modified':header.replace('if(p->changed){memset(p->modified,0,sizeof p->modified);p->changed=0;}','if(p->changed){p->changed=0;}'),
     'widen-16bit-LUT':header.replace('p->lut.b16[i]','p->lut.b32[i]'),
     'reset-clears-RGB':header.replace('p->first=0;p->last=255;', 'memset(p->rgb,0,sizeof p->rgb);p->first=0;p->last=255;'),
     'omit-last-interval':header.replace('if(p->last<entry)p->last=entry;',''),
    }
    for fault,text in {'positive':header,**mutants}.items():
     folder=work/fault;folder.mkdir();(folder/'fist_render_palette.h').write_text(text)
     (folder/'portable.c').write_bytes((root/'portable.c').read_bytes());(folder/'cases.inc').write_text(common)
     for target,command in build(repo,folder):
      raw=folder/(target+'.raw');run(command,raw);actual=raw.read_bytes();assert len(actual)==len(expected)
      if fault=='positive':assert actual==expected;raw.unlink();continue
      assert actual!=expected;first=next(i for i,(a,b) in enumerate(zip(expected,actual)) if a!=b)
      results.append(dict(fault=fault,target=target,terminal_exit=0,first_difference=first,original_byte=expected[first],mutant_byte=actual[first],all_preceding_bytes_equal=True,original_sha256=hashlib.sha256(expected).hexdigest(),mutant_sha256=hashlib.sha256(actual).hexdigest()));raw.unlink()
    (work/'original.raw').unlink()
    proof=dict(scope='Four deliberate renderer faults fail both targets after complete source positives.',results=results)
    (work/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    return results


def capture(repo,root):
    assert root.is_relative_to(Path('/tmp'))
    root.mkdir(parents=True,exist_ok=False);fixtures(repo,root)
    results=compare(repo,root,259316,expected_log='records=259316')
    negatives=causal(repo,root)
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    paths=[Path(__file__),repo/'tools/oracle/palette_control_fixture.py',
           repo/'re_out/fist_render_palette.h',root/'original.cpp',root/'portable.c',root/'cases.inc',
           tree/'src/gui/render.cpp',tree/'src/gui/render_scalers.h',tree/'config.h',
           *sorted((tree/'include').glob('*.h'))]
    proof=dict(scope='259316 literal original RENDER_SetPal,Reset palette fields and Check_Palette programs. Complete RGB/unused,LUT,modified,changed,64-bit bounds and ordered host calls match both release targets. Every scaler mode including default,dirty state,interval edge,consecutive consumers,component/value/entry and reset is covered. Actual frame/scaler/caller transport and complete frame/audio parity remain separate.',
               results=results,causal_negatives=negatives,
               inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    return proof


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();capture(args.repo.resolve(),args.output.resolve())
