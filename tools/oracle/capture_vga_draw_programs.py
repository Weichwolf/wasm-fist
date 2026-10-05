#!/usr/bin/env python3
"""Verbatim original VGA drawing contracts and complete causal regressions."""
from pathlib import Path
import argparse,hashlib,itertools,json,struct,subprocess,sys

def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def negative(repo,root):
    sys.path.insert(0,str(repo/'tests'))
    from test_port_io import tool
    proof=json.loads((root/'proof.json').read_text())
    for p,h in proof['inputs_sha256'].items():assert digest(p)==h,p
    header=(root/'fist_vga_draw.h').read_text()
    base=[0,320,200,80,48000,5,2,65535,320,320,1,0,200,0,150,2,512,4,50,1,4,8,0,1,1,0,0,0]
    base+=list(struct.unpack('<Q',struct.pack('<d',3.177755)))+[0,0,2,5,5,3,65,142,262144]
    ram=bytes((i*37+(i>>8)+(i>>16)*73+11)&255 for i in range(0x40000))
    def case(op,count=0,changes=()):
     q=base.copy()
     for index,value in changes:q[index]=value
     return struct.pack('<40Q',*q,op,count)+ram
    mutations=[
     ('narrowed-address', 'v->address+=v->address_add;', 'v->address=(uint32_t)(v->address+v->address_add);',case(0,1,[(4,0xffffffff)])),
     ('missing-line-advance','v->address_line++;','/* deliberate missing original address-line advance */',case(0,2)),
     ('missing-wrap-copy','memcpy(base+v->linear_mask+1,base,v->line_length);','(void)base;',case(4,0,[(4,65500),(9,128)])),
     ('missing-display-mask','v->display_start&(uint32_t)(v->vmemwrap-1)','v->display_start',case(2,0,[(30,0x10000fff0),(37,65536)])),
     ('text-panning','if(v->vga_mode!=9)v->address+=v->panning;','v->address+=v->panning;',case(3,0,[(34,9),(35,0)])),
     ('wrong-irq-machine','if(v->machine==4)h->irq','if(v->machine==5)h->irq',case(1,0,[(33,4),(36,0x10)])),
     ('wrong-last-part-lines','v->parts_left!=1?v->parts_lines:v->lines_total-v->lines_done','v->parts_lines',case(0,1,[(19,2)])),
    ]
    def parse(b):
     assert len(b)>=304+len(ram)+4
     state=struct.unpack('<38Q',b[:304]);memory=b[304:304+len(ram)]
     n=struct.unpack('<I',b[304+len(ram):308+len(ram)])[0];requests=b[308+len(ram):]
     assert len(requests)==n
     return state,memory,requests
    results=[]
    for name,old,new,input_ in mutations:
     assert header.count(old)==1,(name,old)
     folder=root/('negative-'+name);folder.mkdir(exist_ok=True)
     (folder/'fist_vga_draw.h').write_text(header.replace(old,new));(folder/'portable.c').write_bytes((root/'portable.c').read_bytes())
     p=subprocess.run([str(root/'original')],input=input_,capture_output=True,timeout=30);assert p.returncode==0,p.stderr
     expected=p.stdout;es,em,er=parse(expected)
     for target,compiler,options,output,runner in (
      ('native',['gcc','-m32'],[],folder/'native',[]),
      ('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc')],['-sNODERAWFS=1','-sEXIT_RUNTIME=1','-sALLOW_MEMORY_GROWTH=1'],folder/'wasm.js',[tool('node','Git/emsdk/node/*/bin/node')])):
      p=subprocess.run([*compiler,'-O2','-DNDEBUG','-I'+str(folder),str(folder/'portable.c'),*options,'-o',str(output)],capture_output=True,text=True,timeout=120)
      (folder/(target+'-build.log')).write_text(p.stdout+p.stderr);assert p.returncode==0,p.stderr
      p=subprocess.run([*runner,str(output)],input=input_,capture_output=True,timeout=30);assert p.returncode==0,p.stderr
      actual=p.stdout;assert actual!=expected,(name,target)
      a,m,r=parse(actual);fields=[i for i,(x,y) in enumerate(zip(es,a)) if x!=y]
      offsets=[i for i,(x,y) in enumerate(zip(em,m)) if x!=y]
      if name=='narrowed-address':assert fields==[4] and not offsets and r==er
      if name=='missing-display-mask':assert fields==[29] and not offsets and r==er
      if name=='wrong-irq-machine':assert es==a and em==m and len(er)==16 and not r
      if name=='wrong-last-part-lines':assert es==a and em==m and len(er)==len(r)==4*8+320+3*8 and r!=er
      results.append(dict(case=name,target=target,terminal_exit=0,input_sha256=hashlib.sha256(input_).hexdigest(),original_sha256=hashlib.sha256(expected).hexdigest(),actual_sha256=hashlib.sha256(actual).hexdigest(),differing_state_words=fields,first_memory_difference=offsets[0] if offsets else None,original_requests_bytes=len(er),actual_requests_bytes=len(r),requests_equal=er==r,mutant_header_sha256=digest(folder/'fist_vga_draw.h')))
     print('PASS distinguish',name,'on both release targets',flush=True)
    result=dict(scope='Seven deliberate original VGA callback faults distinguished by complete state/RAM/line/end/rearm/IRQ outputs on both targets. Positive source receipt is immutable. No renderer/full-runtime acceptance.',positive_proof_sha256=digest(root/'proof.json'),script_sha256=digest(Path(__file__)),results=results,complete_original_acceptance=False)
    (root/'negative-proof.json').write_text(json.dumps(result,indent=2)+'\n')
    return result

def original_program(repo):
    """One owner for verbatim original VGA bodies and complete state transport."""
    source=repo/'third_party/dosbox-build/dosbox-0.74-3/src/hardware/vga_draw.cpp'
    tree=source.parents[2]
    header=repo/'re_out/fist_vga_draw.h'
    fields=('resizing','width','height','blocks','address','panning','bytes_skip','linear_mask','address_add','line_length','address_line_total','address_line','lines_total','vblank_skip','lines_done','lines_scaled','split_line','parts_total','parts_lines','parts_left','byte_panning_shift','bpp','double_scan','doublewidth','doubleheight','blinking','mode','vret_triggered')
    configs=('real_start','display_start','bytes_skip','pel_panning')
    extras=('machine','vga_mode','attr_mode_control','vertical_retrace_end','vmemwrap')
    def function(name,next_name):
     s=source.read_text();a=s.index(name);a=s.rfind('\n',0,a)+1;b=s.index(next_name,a);b=s.rfind('\n',0,b)+1
     return s[a:b]
    original='''#include <stdint.h>
    #include "dosbox.h"
    #include "vga.h"
    #include <assert.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    static_assert(sizeof(Bitu)==8,"Match original64-bit host Bitu");
    VGA_Type vga;
    MachineType machine;
    static unsigned char ram[0x40000];
    static unsigned char requests[0x40000];
    static unsigned request_size;
    static void request(const void *p,unsigned n) {assert(request_size+n<=sizeof requests);memcpy(requests+request_size,p,n);request_size+=n;}
    static void emit(const void *p) {
     uint64_t q[]={1,vga.draw.address,vga.draw.address_line,vga.draw.line_length};request(q,sizeof q);request(p,vga.draw.line_length);
    }
    static void (*RENDER_DrawLine)(const void *)=emit;
    static void RENDER_EndUpdate(bool abort_update) {uint64_t q[]={2,abort_update};request(q,sizeof q);}
    static void PIC_ActivateIRQ(Bitu irq) {uint64_t q[]={4,irq};request(q,sizeof q);}
    static void VGA_DrawPart(Bitu);
    static void PIC_AddEvent(void (*handler)(Bitu),float delay,Bitu value) {
     assert(handler==VGA_DrawPart);uint32_t bits;memcpy(&bits,&delay,4);uint64_t q[]={3,bits,value};request(q,sizeof q);
    }
    '''
    original+=function('static Bit8u * VGA_Draw_Linear_Line(','static Bit8u * VGA_Draw_Xlat16_Linear_Line(')
    original+='static Bit8u *(*VGA_DrawLine)(Bitu,Bitu)=VGA_Draw_Linear_Line;\n'
    original+=function('static void VGA_ProcessSplit()','static void VGA_DrawSingleLine(')
    original+=function('static void VGA_DrawPart(Bitu lines)','void VGA_SetBlinking(')
    original+=function('static void VGA_VertInterrupt(','static void VGA_Other_VertInterrupt(')
    original+=function('static void VGA_DisplayStartLatch(','static void VGA_PanningLatch(')
    restore='\n'.join('vga.draw.'+k+'=q['+str(i)+'];' for i,k in enumerate(fields))
    restore+='\nmemcpy(&vga.draw.delay.parts,q+28,8);\n'
    restore+='\n'.join('vga.config.'+k+'=q['+str(29+i)+'];' for i,k in enumerate(configs))
    restore+='\nmachine=(MachineType)q[33];vga.mode=(VGAModes)q[34];vga.attr.mode_control=q[35];vga.crtc.vertical_retrace_end=q[36];vga.vmemwrap=q[37];vga.draw.linear_base=ram;\n'
    # Enum fields require exact declared type, no guessed replacement definitions.
    restore=restore.replace('vga.draw.mode=q[26];','vga.draw.mode=(Drawmode)q[26];')
    observe='uint64_t out[38]={'+','.join('vga.draw.'+k for k in fields)+',0,'+','.join('vga.config.'+k for k in configs)+',machine,vga.mode,vga.attr.mode_control,vga.crtc.vertical_retrace_end,vga.vmemwrap};memcpy(out+28,&vga.draw.delay.parts,8);'
    original+='''int main() {
     uint64_t q[40];
     while(fread(q,sizeof q,1,stdin)==1) {
     assert(fread(ram,sizeof ram,1,stdin)==1);memset(&vga,0,sizeof vga);request_size=0;
    '''+restore+'''
     switch(q[38]) {
     case 0:VGA_DrawPart(q[39]);break;case 1:VGA_VertInterrupt(0);break;
     case 2:VGA_DisplayStartLatch(0);break;case 3:VGA_ProcessSplit();break;
     case 4:emit(VGA_Draw_Linear_Line(vga.draw.address,vga.draw.address_line));break;
     default:abort();
     }
    '''+observe+'''
     assert(fwrite(out,sizeof out,1,stdout)==1 && fwrite(ram,sizeof ram,1,stdout)==1 && fwrite(&request_size,4,1,stdout)==1 && (!request_size || fwrite(requests,request_size,1,stdout)==1));
     }return 0;
    }
    '''
    return original,fields,configs,extras

def capture(repo,root):
    root.mkdir(parents=True,exist_ok=False)
    sys.path.insert(0,str(repo/'tests'))
    from test_port_io import tool
    source=repo/'third_party/dosbox-build/dosbox-0.74-3/src/hardware/vga_draw.cpp'
    tree=source.parents[2]
    header=repo/'re_out/fist_vga_draw.h'
    def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
    original,fields,configs,extras=original_program(repo)
    (root/'original.cpp').write_text(original)
    portable='''#include "fist_vga_draw.h"
    #include <stdio.h>
    #include <stdlib.h>
    static FistVgaDraw v;
    static unsigned char ram[0x40000],requests[0x40000];
    static unsigned request_size;
    static void request(const void *p,unsigned n) {if(request_size+n>sizeof requests)abort();memcpy(requests+request_size,p,n);request_size+=n;}
    static const uint8_t *line(void *ctx,uint64_t address,uint64_t scanline) {return fist_vga_linear_line(&v,ram,address);}
    static void emit(void *ctx,const uint8_t *p) {uint64_t q[]={1,v.address,v.address_line,v.line_length};request(q,sizeof q);request(p,v.line_length);}
    static void end(void *ctx,int abort_update) {uint64_t q[]={2,abort_update};request(q,sizeof q);}
    static void add(void *ctx,float delay,uint64_t value) {uint32_t bits;memcpy(&bits,&delay,4);uint64_t q[]={3,bits,value};request(q,sizeof q);}
    static void irq(void *ctx,unsigned irq) {uint64_t q[]={4,irq};request(q,sizeof q);}
    static const FistVgaDrawHost host={NULL,line,emit,end,add,irq};
    int main() {
     uint64_t q[40];
     while(fread(q,sizeof q,1,stdin)==1) {
     if(fread(ram,sizeof ram,1,stdin)!=1)abort();memcpy(&v,q,sizeof v);request_size=0;
     switch(q[38]) {
     case 0:fist_vga_draw_part(&v,&host,q[39]);break;case 1:fist_vga_vert_interrupt(&v,&host);break;
     case 2:fist_vga_display_start_latch(&v);break;case 3:fist_vga_process_split(&v);break;
     case 4:emit(NULL,line(NULL,v.address,v.address_line));break;default:abort();
     }
     if(fwrite(&v,sizeof v,1,stdout)!=1 || fwrite(ram,sizeof ram,1,stdout)!=1 || fwrite(&request_size,4,1,stdout)!=1 || (request_size && fwrite(requests,request_size,1,stdout)!=1))abort();
     }return 0;
    }
    '''
    (root/'portable.c').write_text(portable);(root/'fist_vga_draw.h').write_bytes(header.read_bytes())
    def build(name,args):
     p=subprocess.run(args,capture_output=True,text=True,timeout=120);(root/(name+'-build.log')).write_text(p.stdout+p.stderr);assert p.returncode==0,p.stderr
    flags=['-O2','-std=gnu++11',*subprocess.check_output(['sdl-config','--cflags'],text=True).split(),'-I'+str(tree/'include'),'-I'+str(tree)]
    build('original',['g++',*flags,str(root/'original.cpp'),'-o',str(root/'original')])
    runs=[('original',[str(root/'original')])]
    for name,compiler,options,output,runner in (
     ('native',['gcc','-m32'],[],root/'native',[]),
     ('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc')],['-sNODERAWFS=1','-sEXIT_RUNTIME=1','-sALLOW_MEMORY_GROWTH=1'],root/'wasm.js',[tool('node','Git/emsdk/node/*/bin/node')])):
     build(name,[*compiler,'-O2','-DNDEBUG','-I'+str(root),str(root/'portable.c'),*options,'-o',str(output)])
     runs.append((name,[*runner,str(output)]))
    base=[0,320,200,80,48000,5,2,65535,320,320,1,0,200,0,150,2,512,4,50,1,4,8,0,1,1,0,0,0]
    base+=list(struct.unpack('<Q',struct.pack('<d',3.177755)) )+[0,0,2,5,5,3,65,142,262144]
    assert len(base)==38
    cases=[]
    def add(op,count=0,**kwargs):
     q=base.copy()
     for k,value in kwargs.items():q[29+configs.index(k[4:]) if k.startswith('cfg_') else fields.index(k) if k in fields else 29+configs.index(k) if k in configs else 33+extras.index(k)]=value
     cases.append(q+[op,count])
    for lines,parts,total,start,split,mode,machine,attr in itertools.product((0,1,3,50),(0,1,2,3),(1,4),(0,0xffffffff,0x100000000),(151,153,512),(3,9),(4,5),(0,32)):
     add(0,lines,parts_left=parts,address_line_total=total,address=start,split_line=split,vga_mode=mode,machine=machine,attr_mode_control=attr)
    for triggered,retrace,machine in itertools.product((0,1),range(256),(4,5)):
     add(1,vret_triggered=triggered,vertical_retrace_end=retrace,machine=machine)
    for display,wrap,skip in itertools.product((0,1,0xffff,0xffffffff,0x100000000,0xffffffffffffffff),(0,1,65536,262144),(0,1,255)):
     add(2,display_start=display,vmemwrap=wrap,cfg_bytes_skip=skip)
    for address,mask,length in itertools.product((0,1,65500,65535,0xffffffff,0x100000000,0xffffffffffffffff),(255,65535),(1,128,256)):
     add(4,address=address,linear_mask=mask,line_length=length)
    for mode,machine,attr,skip,panning in itertools.product((3,9),(4,5),(0,32),(0,255),(0,255)):
     add(3,vga_mode=mode,machine=machine,attr_mode_control=attr,bytes_skip=skip,panning=panning)
    ram=bytes((i*37+(i>>8)+(i>>16)*73+11)&255 for i in range(0x40000))
    hashes={name:hashlib.sha256() for name,_ in runs}
    for start in range(0,len(cases),64):
     data=b''.join(struct.pack('<40Q',*q)+ram for q in cases[start:start+64]);expected=None
     for name,args in runs:
      p=subprocess.run(args,input=data,capture_output=True,timeout=30);assert p.returncode==0,(name,start,p.stderr[:1000]);assert p.stdout
      if expected is None:expected=p.stdout
      else:assert p.stdout==expected,(name,start,'Complete state/RAM/request output differs')
      hashes[name].update(p.stdout)
    
    paths=[Path(__file__),*tree.rglob('*.h'),source,tree/'include/vga.h',tree/'include/dosbox.h',tree/'config.h',header,*[root/p for p in ('original.cpp','portable.c','fist_vga_draw.h','original','native','wasm.js','wasm.wasm')]]
    proof=dict(scope='Controlled verbatim original VGA_DrawPart/ProcessSplit/VertInterrupt/DisplayStartLatch/Draw_Linear_Line on original64-bit Bitu host versus explicit64-bit shared arithmetic on both32-bit release targets. Full38 canonical64-bit state words including double delay bits, all256KiB RAM and every complete line/end/rearm/IRQ request match. No renderer/scaler body, runtime bootstrap or complete frame/audio acceptance.',cases=len(cases),outputs_sha256={n:h.hexdigest() for n,h in hashes.items()},inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    proof['causal_negatives']=negative(repo,root)
    return proof

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--repo',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();capture(a.repo.resolve(),a.output.resolve())
