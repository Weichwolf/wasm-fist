#!/usr/bin/env python3
"""Compare actual original PIC re-arms with the bound shared VGA device."""
import argparse,hashlib,json,re,struct,subprocess,sys
from pathlib import Path
from capture_vga_draw_programs import original_program

def original_source(repo):
    original,*_=original_program(repo)
    head=original[:original.index('int main()')]
    head=head.replace('    MachineType machine;','')
    head=re.sub(r'    static void PIC_ActivateIRQ\(Bitu irq\).*?\n','',head)
    head=re.sub(r'    static void PIC_AddEvent\(.*?\n    }\n','',head,flags=re.S)
    restore=original[original.index('vga.draw.resizing=q[0];'):original.index('     switch(q[38])')]
    observe=original[original.index('uint64_t out[38]='):original.index('     assert(fwrite(out')]
    start=head.index('    static Bit8u * VGA_Draw_Linear_Line')
    head=head[:start]+'''static void snapshot(unsigned);
static void observed_add(PIC_EventHandler h,float delay,Bitu value) {PIC_AddEvent(h,delay,value);snapshot(1);}
#define PIC_AddEvent observed_add
'''+head[start:]
    return '''#define main unused_pic_main
#include "pic_slice_probe.cpp"
#undef main
'''+head+'''
#undef PIC_AddEvent
static void first_part(Bitu),sentinel(Bitu);
static void word(uint32_t q) {assert(fwrite(&q,4,1,stdout)==1);}
static void snapshot(unsigned label) {
'''+observe+'''
 assert(fwrite(out,sizeof out,1,stdout)==1);word(label);word(PIC_Ticks);word(PIC_TickIndexND());word(CPU_Cycles);word(CPU_CycleLeft);word(InEventService);
 unsigned n=0;for(PICEntry *e=pic_queue.next_entry;e;e=e->next)n++;word(n);
 for(PICEntry *e=pic_queue.next_entry;e;e=e->next) {
  uint32_t bits;memcpy(&bits,&e->index,4);word(bits);word(e->value);
  unsigned id=e->pic_event==VGA_DrawPart?0:e->pic_event==first_part?1:e->pic_event==VGA_VertInterrupt?2:e->pic_event==VGA_DisplayStartLatch?3:e->pic_event==sentinel?4:5;assert(id<5);word(id);
 }
 assert(fwrite(ram,sizeof ram,1,stdout)==1);word(request_size);assert(!request_size || fwrite(requests,request_size,1,stdout)==1);request_size=0;
}
static void first_part(Bitu value) {snapshot(2);VGA_DrawPart(value);snapshot(3);}
static void sentinel(Bitu value) {uint64_t q[]={5,value};request(q,sizeof q);}
static void fetch(unsigned count) {while(count--)while(CPU_Cycles--<=0)while(!PIC_RunQueue())TIMER_AddTick();}
int main() {uint64_t q[40];assert(fread(q,sizeof q,1,stdin)==1 && fread(ram,sizeof ram,1,stdin)==1);initialize_queue();
'''+restore+'''
 uint32_t bits=q[39];float delay;memcpy(&delay,&bits,4);
 PIC_AddEvent(first_part,delay,50);PIC_AddEvent(VGA_VertInterrupt,20.0f,0);PIC_AddEvent(VGA_DisplayStartLatch,21.0f,0);PIC_AddEvent(sentinel,4.0f,123);
 assert(PIC_RunQueue());snapshot(0);fetch(400);snapshot(5);
 if(q[38]) {
  PIC_RemoveEvents(VGA_DrawPart);PIC_RemoveEvents(VGA_VertInterrupt);PIC_RemoveEvents(VGA_DisplayStartLatch);
  if(q[38]==1)vga.draw.address+=1000;else assert(q[38]==2);snapshot(6);
 }
 fetch(400000);snapshot(4);return 0;
}
'''

def rows(data):
    result=[];offset=0
    while offset<len(data):
        state=struct.unpack_from('<38Q',data,offset);offset+=304
        budget=struct.unpack_from('<7I',data,offset);offset+=28
        calendar=struct.unpack_from('<'+str(budget[6]*3)+'I',data,offset);offset+=budget[6]*12
        memory=data[offset:offset+0x40000];offset+=0x40000
        n=struct.unpack_from('<I',data,offset)[0];offset+=4
        requests=data[offset:offset+n];offset+=n
        assert len(memory)==0x40000 and len(requests)==n
        result.append((state,budget,calendar,memory,requests))
    assert offset==len(data);return result

def capture(repo,root):
    assert root.is_relative_to(Path('/tmp'));root.mkdir(parents=True,exist_ok=False)
    sys.path.insert(0,str(repo/'tests'));from test_port_io import tool
    (root/'original.cpp').write_text(original_source(repo))
    (root/'portable.c').write_bytes((repo/'tests/vga_pic_rearm.c').read_bytes())
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    flags=['-O2','-ffunction-sections','-fdata-sections','-Wl,--gc-sections']
    def build(name,args):
        p=subprocess.run(args,capture_output=True,timeout=120);(root/(name+'-build.log')).write_bytes(p.stdout+p.stderr);assert p.returncode==0,p.stderr.decode()
    build('original',['g++','-std=gnu++11',*flags,*subprocess.check_output(['sdl-config','--cflags'],text=True).split(),'-I'+str(repo/'tools/oracle'),'-I'+str(tree/'include'),'-I'+str(tree),str(root/'original.cpp'),'-lm','-o',str(root/'original')])
    base=[0,320,200,80,48000,5,2,65535,320,320,1,0,200,0,0,2,512,4,50,4,4,8,0,1,1,0,0,0]
    base+=list(struct.unpack('<Q',struct.pack('<d',3.177755)))+[0,0,2,5,5,3,65,142,262144]
    ram=bytes((i*37+(i>>8)+(i>>16)*73+11)&255 for i in range(0x40000))
    cases={}
    for mode in range(3):
        # Lifecycle removal reaches a pending DrawPart and both pending latches.
        for delay in ((0.01,0.99999,1.5) if mode==0 else (0.01,)):
            bits=struct.unpack('<I',struct.pack('<f',delay))[0]
            cases[f'{mode}-{delay}']=struct.pack('<40Q',*base,mode,bits)+ram
    expected={};summaries=[];negatives=[]
    compilers=[('native',['gcc','-m32'],[],[]),('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc')],['-sNODERAWFS=1','-sEXIT_RUNTIME=1','-sALLOW_MEMORY_GROWTH=1'],[tool('node','Git/emsdk/node/*/bin/node')])]
    def run(command,data):
        p=subprocess.run(command,input=data,capture_output=True,timeout=30);assert p.returncode==0,p.stderr.decode();assert p.stdout;return p.stdout
    for case,data in cases.items():
        output=run([str(root/'original')],data);expected[case]=output
        observed=rows(output)
        if case.startswith('0-'):
            rearmed=[q for q in observed if q[1][0]==1]
            assert len(rearmed)==3 and all(q[1][3]==0 and q[1][5]==1 for q in rearmed)
            assert observed[-1][0][19]==0 and observed[-1][0][14]==200,case
            assert observed[-1][2][2::3]==(2,3),case
        else:
            removed=next(q for q in observed if q[1][0]==6)
            assert removed[2][2::3]==(4,),removed[2]
            assert observed[-1][2]==() and observed[-1][4]==struct.pack('<2Q',5,123)
        summaries.append(dict(case=case,rows=len(observed),bytes=len(output),sha256=hashlib.sha256(output).hexdigest(),input_sha256=hashlib.sha256(data).hexdigest()))
    for variant in ('current','old-service-rearm','missing-owned-removal'):
        folder=root/variant;folder.mkdir()
        text=(repo/'re_out/fist_vga.c').read_text()
        if variant=='old-service-rearm':
            old='if (!g_pic_service && !clock_equal(g_cpu_time, clock_now())) cpu_slice_start();';assert text.count(old)==1
            text=text.replace(old,'if (!clock_equal(g_cpu_time, clock_now())) cpu_slice_start();')
        if variant=='missing-owned-removal':
            for handler in ('draw_part','vert_interrupt','display_start'):
                old=f'fist_clock_remove_events(fist_clock_vga_{handler});';assert text.count(old)==1;text=text.replace(old,'/* deliberate omitted owner removal */')
        # The quoted endpoint header resolves relative to the real shim directory.
        text=text.replace('#include "../tools/oracle/fist_sequence_endpoint.h"','#include "fist_sequence_endpoint.h"');(folder/'fist_vga.c').write_text(text)
        for target,compiler,extra,runner in compilers:
            out=folder/(target+'.js' if target=='wasm' else target)
            sources=[root/'portable.c',repo/'re_out/fist_pic.c',repo/'re_out/fist_dos.c',repo/'re_out/fist_sb.c']
            build(variant+'-'+target,[*compiler,*flags,'-DNDEBUG','-I'+str(folder),'-I'+str(repo/'tests'),'-I'+str(repo/'re_out'),'-I'+str(repo/'tools/oracle'),*map(str,sources),*extra,'-lm','-o',str(out)])
            chosen=cases if variant=='current' else {'0-0.01':cases['0-0.01']} if variant=='old-service-rearm' else {k:v for k,v in cases.items() if not k.startswith('0-')}
            for case,data in chosen.items():
                p=subprocess.run([*runner,str(out)],input=data,capture_output=True,timeout=30)
                if variant=='current':assert p.returncode==0 and p.stdout==expected[case],(target,case,p.stderr.decode())
                else:
                    assert p.returncode!=0 or p.stdout!=expected[case],(variant,target,case)
                    item=dict(fault=variant,target=target,case=case,terminal_exit=p.returncode)
                    if p.returncode==0:
                        a,b=rows(expected[case]),rows(p.stdout);first=next(i for i,(x,y) in enumerate(zip(a,b)) if x!=y)
                        item['original_rows']=len(a);item['actual_rows']=len(b)
                        item['first_row']=first;item['original_budget']=a[first][1];item['actual_budget']=b[first][1]
                        if variant=='old-service-rearm':
                            assert len(a)==len(b) and first==2 and a[first][1][3]==0 and b[first][1][3]>0
                            assert all(x[0]==y[0] and x[2:]==y[2:] for x,y in zip(a,b))
                    negatives.append(item)
    paths=[Path(__file__),repo/'tools/oracle/capture_vga_draw_programs.py',repo/'tools/oracle/pic_slice_probe.cpp',repo/'tests/vga_pic_rearm.c',repo/'re_out/fist_vga.c',repo/'re_out/fist_vga_draw.h',repo/'re_out/fist_pic.c',tree/'src/hardware/vga_draw.cpp',tree/'src/hardware/pic.cpp',*tree.rglob('*.h'),root/'original.cpp',root/'portable.c']
    proof=dict(scope='Actual original PIC_RunQueue/PIC_AddEvent and verbatim VGA_DrawPart versus bound shared VGA/PIC. Five complete programs, all38 drawing words, CPU tick/index/budget/left/service, ordered event indices/values/identities, all256KiB RAM and every complete line/end request. Three actual re-arms per drawing program; replace/detach remove the three owned callbacks through original PIC_RemoveEvents and preserve an unrelated reaching event. Renderer bodies, bootstrap, complete runtime video/audio acceptance remain open.',programs=summaries,causal_negatives=negatives,inputs_sha256={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS five complete bound VGA/PIC programs and six causal negatives',flush=True);return proof

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--repo',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();capture(a.repo.resolve(),a.output.resolve())
