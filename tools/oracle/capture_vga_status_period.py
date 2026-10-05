#!/usr/bin/env python3
"""Compare every CPU quantum of a full original VGA period with the shared clock."""
import hashlib,json,math,struct,subprocess,sys
from pathlib import Path

def capture(repo,root,case):
    root.mkdir(parents=True,exist_ok=False)
    sys.path.insert(0,str(repo/'tests'));from test_port_io import tool
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    keys=('framestart','vrstart','vrend','hblkstart','hblkend','htotal','vdend','vtotal')
    status=case['events'][6]['vga_status_device'];doubles=b''.join(bytes.fromhex(status[k]) for k in keys)
    d=struct.unpack('<8d',doubles);first=math.floor(d[0]*30000);last=math.ceil((d[0]+d[7])*30000)
    budgets=(1,20,62,63,64,86,87,88,100,1000,9311);inputs=bytearray()
    for cycle in range(first,last+1):
        tick,index=divmod(cycle,30000);budget=min(budgets[cycle%len(budgets)],30000-index)
        inputs+=doubles+struct.pack('<7I',cycle&1,cycle&255,tick,budget,30000-index-budget,0xfeedbeef,0x1234)
    (root/'input.bin').write_bytes(inputs)
    (root/'clock.c').write_text('''#include "cpu_vga_clock.c"
void status_case(FILE *input,uint32_t result[8]) {
 uint32_t q[5];fist_cpu_require(fread(&status,72,1,input)==1 && fread(q,sizeof q,1,input)==1);
 uint64_t cycle=(uint64_t)q[0]*30000+30000-q[1]-q[2],num=cycle*PIT_HZ_;
 clock_set((FistClock){num/CPU_HZ_,num%CPU_HZ_});g_cpu_time=clock_now();g_cpu_remaining=q[1];
 g_pic_tick=q[0];g_pic_initialized=1;fist_cpu_require(!g_pic_events && !g_pic_service);
 g_cpu_io_removed=((uint64_t)q[4]<<32)|q[3];
 result[0]=in(0x3da);uint64_t tick;unsigned left;observe_core_clock(&tick,&left);
 result[1]=tick;result[2]=fist_clock_cpu_slice(NULL);result[3]=left;
 result[4]=g_cpu_io_removed;result[5]=g_cpu_io_removed>>32;result[6]=status.attr;result[7]=status.pcjr;
}
''')
    (root/'driver.c').write_text('''#define fist_int8_fire unused_int8
#include "sb_clock_fixture.h"
#undef fist_int8_fire
#include "fist_cpu.h"
#include "fist_vga_draw.h"
void fist_int8_fire(void){abort();}
void observe_callback_state(const char *kind){abort();}
extern void status_case(FILE *,uint32_t *);
extern void bind_status_case(void);
int main(int argc,char **argv) {
 FILE *input=fopen(argv[1],"rb"),*output=fopen(argv[2],"wb");fist_cpu_require(input && output);
 FistCpuState cpu={0};fist_clock_bind_cpu(&cpu);bind_status_case();
 while(1) {int value=fgetc(input);if(value==EOF)break;fist_cpu_require(ungetc(value,input)==value);uint32_t q[8];status_case(input,q);fist_cpu_require(fwrite(q,sizeof q,1,output)==1);}
 fist_cpu_require(feof(input) && !ferror(input) && !fclose(input) && !fclose(output));return 0;
}
''')
    # All callbacks are explicit fail-on-reach fixture observers: these cases
    # independently seed status/time and deliberately have no calendar events.
    with (root/'clock.c').open('a') as f:f.write('''
static const uint8_t *status_unreached_line(void *p,uint64_t a,uint64_t b){abort();}
static void status_unreached_emit(void *p,const uint8_t *data){abort();}
static void status_unreached_end(void *p,int aborted){abort();}
void bind_status_case(void) {bind_pit_clock();fist_clock_bind_vga(&drawing,&status,NULL,status_unreached_line,status_unreached_emit,status_unreached_end);}
''')
    common=['-O2','-DNDEBUG','-I'+str(root),'-I'+str(repo/'tests'),'-I'+str(repo/'re_out'),'-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-w']
    sources=[root/'driver.c',root/'clock.c',repo/'tests/cpu_core_exit_pic.c',repo/'re_out/fist_dos.c',repo/'re_out/fist_sb.c']
    original=['g++','-O2','-std=gnu++11',*subprocess.check_output(['sdl-config','--cflags'],text=True).split(),'-I'+str(tree/'include'),'-I'+str(tree),'-ffunction-sections','-fdata-sections',str(repo/'tools/oracle/vga_status_probe.cpp'),str(repo/'tools/oracle/io_delay_probe.cpp'),'-Wl,--gc-sections','-lm','-o',str(root/'original')]
    commands=[('original',original,[str(root/'original')]),('native',['gcc','-m32',*common,*map(str,sources),'-Wl,--gc-sections','-lm','-o',str(root/'native')],[str(root/'native')]),('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc'),*common,*map(str,sources),'-sNODERAWFS=1','-sEXIT_RUNTIME=1','-sALLOW_MEMORY_GROWTH=1','-o',str(root/'wasm.js')],[tool('node','Git/emsdk/node/*/bin/node'),str(root/'wasm.js')])]
    expected=None;results=[]
    for name,build,run in commands:
        p=subprocess.run(build,capture_output=True,timeout=120);(root/(name+'-build.log')).write_bytes(p.stdout+p.stderr);assert p.returncode==0,p.stderr.decode()
        output=root/(name+'.raw');p=subprocess.run([*run,str(root/'input.bin'),str(output)],capture_output=True,timeout=30)
        (root/(name+'-run.log')).write_bytes(p.stdout+p.stderr);assert p.returncode==0,p.stderr.decode()
        data=output.read_bytes();assert len(data)==(last-first+1)*32
        if expected is None:expected=data
        else:assert data==expected,(name,next(i for i,(a,b) in enumerate(zip(data,expected)) if a!=b))
        results.append(dict(target=name,terminal_exit=0,cases=last-first+1,bytes=len(data),sha256=hashlib.sha256(data).hexdigest()));output.unlink()
    paths=[Path(__file__),repo/'tools/oracle/vga_status_probe.cpp',repo/'tools/oracle/io_delay_probe.cpp',repo/'tests/cpu_vga_clock.c',repo/'tests/cpu_pit_clock.c',repo/'tests/cpu_core_exit_pic.c',repo/'re_out/fist_vga.c',repo/'re_out/fist_vga_draw.h',tree/'src/hardware/vga_misc.cpp',tree/'src/hardware/iohandler.cpp',*tree.rglob('*.h'),root/'clock.c',root/'driver.c']
    proof=dict(scope='All428044 CPU quanta of the actual observed VGA period, original vga_read_p3da/I/O delay/PIC_FullIndex versus bound shared owner. Complete response/tick/budget/left/IODelayRemoved/two-flip outputs match on both targets. Independently seeded cases exclude callback/IRQ dispatch and full runtime/frame/audio acceptance.',results=results,input_sha256=hashlib.sha256(inputs).hexdigest(),inputs_sha256={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');(root/'input.bin').unlink()
    return proof
