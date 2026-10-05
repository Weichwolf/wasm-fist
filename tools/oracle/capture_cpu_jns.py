#!/usr/bin/env python3
"""Compare actual original JNS and every original release sign-query branch."""
import argparse,hashlib,itertools,json,re,resource,struct,subprocess,sys
from pathlib import Path
from cpu_execute_probe import build as original_build
from capture_cpu_byte_instructions import target_build,digest


def contract(repo):
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    lazy=(tree/'src/cpu/lazyflags.h').read_text()
    flags=(tree/'src/cpu/flags.cpp').read_text()
    body=flags[flags.index('Bit32u get_SF(void) {'):flags.index('Bit32u get_OF(void) {')]
    names=re.findall(r't_\w+',lazy[lazy.index('//Types of Flag changing instructions'):])
    assert len(names)==len(set(names))==66 and names[0]=='t_UNKNOWN' and names[-1]=='t_LASTFLAG'
    widths={}
    for tags,width in re.findall(r'((?:\s*case t_\w+:)+)\s*return\s*\(lf_res([bwd])&0x[0-9a-fA-F]+\);',body):
        widths.update({name:dict(b=8,w=16,d=32)[width] for name in re.findall(r't_\w+',tags)})
    assert len(widths)==49 and 'return GETFLAG(SF);' in body and body.count('return false;')==2
    assert 'LOG(LOG_CPU,LOG_ERROR)' in body
    config=(tree/'config.h').read_text()
    assert '/* #undef C_DEBUG */' in config and not re.search(r'^\s*#define\s+C_DEBUG\b',config,re.M)
    logging=(tree/'include/logging.h').read_text()
    release=logging[logging.index('#else  //C_DEBUG'):]
    operators=re.findall(r'void operator\(\)\([^\n]*\)\s*\{([^}]*)\}',release)
    assert operators and len(operators)==release.count('void operator()') and all(not body.strip() for body in operators)
    return dict(enum={name:i for i,name in enumerate(names)},result_width=widths,
                recognized_sign_query_tags=list(range(len(names))),original_get_SF=body,
                original_release_logging=release)


def sign_queries(repo,root,contract):
    from test_port_io import tool
    work=root/'sign';work.mkdir()
    recognized=contract['recognized_sign_query_tags'];assert len(recognized)==66
    widths=[contract['result_width'].get(name,0) for name in contract['enum']]
    # Inputs are shared; expectations come from the actual source getter.
    common='''#include <stdio.h>
    static FILE *output;
    int main(int argc,char **argv) {
     REQUIRE(argc==2);output=fopen(argv[1],"wb");REQUIRE(output!=NULL);
     unsigned tags[]={'''+','.join(map(str,recognized))+'''},widths[]={'''+','.join(map(str,widths))+'''};
     unsigned edges[]={0,1,2,0x7fff,0x8000,0x8001,0xffff,0x10000,0x7fffffff,0x80000000,0x80000001,0xffffffff};
     unsigned records=0;
     for(unsigned mode=0;mode<2;mode++)for(unsigned t=0;t<sizeof tags/sizeof *tags;t++) {
      unsigned tag=tags[t],width=widths[tag];
      if(width==8 || width==16) {
       for(unsigned value=0;value<(1u<<width);value++) {run_case(tag,(width==8?0xcdef1200u:0xcdef0000u)|value,mode);records++;}
      }else for(unsigned i=0;i<sizeof edges/sizeof *edges;i++) {run_case(tag,edges[i],mode);records++;}
     }
     REQUIRE(records==2236720);REQUIRE(!ferror(output) && !fclose(output));return 0;
    }
    '''
    (work/'cases.inc').write_text(common)
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    original='''#include <assert.h>
    #include <stdint.h>
    #include "'''+str(tree/'src/cpu/flags.cpp')+'''"
    #include <stdio.h>
    CPU_Regs cpu_regs;
    static void run_case(unsigned tag,unsigned result,unsigned mode) {
     reg_flags=mode?0xffffffff:0x2002;lflags.type=(decltype(lflags.type))tag;
     lflags.var1.dword[0]=0xf1ca8080;lflags.var2.dword[0]=0x89abcdef;lflags.res.dword[0]=result;lflags.prev_type=t_SHLw;lflags.oldcf=1;
     unsigned q[]={lflags.var1.dword[0],lflags.var2.dword[0],lflags.res.dword[0],!!get_SF()};
     uint64_t wide[]={reg_flags,lflags.type,lflags.prev_type,lflags.oldcf};
     assert(fwrite(q,sizeof q,1,output)==1 && fwrite(wide,sizeof wide,1,output)==1);
    }
    #define REQUIRE assert
    #include "cases.inc"
    '''
    # Declare the stream before the body; shared main must not declare it twice.
    common=common.replace('static FILE *output;','');(work/'cases.inc').write_text(common)
    original=original.replace('CPU_Regs cpu_regs;','CPU_Regs cpu_regs;\nstatic FILE *output;')
    portable='''#include "fist_cpu.h"
    #include <stdio.h>
    static FILE *output;static FistCpuState cpu;
    static void run_case(unsigned tag,unsigned result,unsigned mode) {
     cpu.flags=(FistCpuFlags){.flags=mode?0xffffffff:0x2002,.type=tag,.var1=0xf1ca8080,.var2=0x89abcdef,.res=result,.prev_type=FIST_LAZY_SHLW,.oldcf=1};
     unsigned sign=fist_cpu_sf(&cpu);
     unsigned q[]={cpu.flags.var1,cpu.flags.var2,cpu.flags.res,sign};
     uint64_t wide[]={cpu.flags.flags,cpu.flags.type,cpu.flags.prev_type,cpu.flags.oldcf};
     fist_cpu_require(fwrite(q,sizeof q,1,output)==1 && fwrite(wide,sizeof wide,1,output)==1);
    }
    #define REQUIRE fist_cpu_require
    #include "cases.inc"
    '''
    (work/'original.cpp').write_text(original);(work/'portable.c').write_text(portable)
    p=subprocess.run(['g++','-O2','-std=gnu++11',*subprocess.check_output(['sdl-config','--cflags'],text=True).split(),'-I'+str(tree/'include'),'-I'+str(tree),'-ffunction-sections','-fdata-sections',str(work/'original.cpp'),'-Wl,--gc-sections','-o',str(work/'original')],capture_output=True,text=True,timeout=120)
    (work/'original-build.log').write_text(p.stdout+p.stderr);assert p.returncode==0,p.stderr
    native,wasm=work/'native',work/'wasm.js'
    commands=[('original',[str(work/'original')])]
    for target,compiler,extra,out,runner in (
     ('native',['gcc','-m32'],['-Wl,--gc-sections'],native,[]),
     ('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc')],['-sNODERAWFS=1','-sEXIT_RUNTIME=1','-sALLOW_MEMORY_GROWTH=1'],wasm,[tool('node','Git/emsdk/node/*/bin/node')])):
     p=subprocess.run([*compiler,'-O2','-DNDEBUG','-I'+str(repo/'re_out'),str(work/'portable.c'),*extra,'-o',str(out)],capture_output=True,text=True,timeout=120)
     (work/(target+'-build.log')).write_text(p.stdout+p.stderr);assert p.returncode==0,p.stderr
     commands.append((target,[*runner,str(out)]))
    results=[];expected=None
    for target,command in commands:
     path=work/(target+'.raw');p=subprocess.run([*command,str(path)],capture_output=True,text=True,timeout=90);assert p.returncode==0,p.stderr
     actual=path.read_bytes();assert len(actual)==2236720*48
     if expected is None:expected=actual
     else:assert actual==expected,(target,'boolean sign or full unchanged state differs')
     results.append(dict(target=target,records=2236720,bytes=len(actual),sha256=hashlib.sha256(actual).hexdigest()));path.unlink()
    proof=dict(scope='All66 original enum tags: every byte/word result, dirty upper result bytes, DWORD edges and both raw SF states compare2236720 complete query/unchanged var1/var2/res/raw flags/type/prev/oldcf records. Original UNKNOWN/raw,49 result-width branches,DIV/MUL false and13 diagnostic defaults plus LASTFLAG false are covered.',results=results)
    (work/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS original66 sign-query tags/2236720 complete records on both targets',flush=True)
    return proof


def programs(repo,root,contract,opcode=0x79):
    assert opcode in (0x78,0x79)
    label="JS" if opcode==0x78 else "JNS"
    from test_cpu_execute import packet
    work=root/'programs';work.mkdir()
    source=work/'source';source.mkdir();original=original_build(repo,source,extra_opcodes=(opcode,),trace=True,lazy_input=True)
    header=(repo/'re_out/fist_exec.h').read_text();cpu=(repo/'re_out/fist_cpu.h').read_text()
    controlled='#include "fist_cpu.h"\n'+(repo/'tests/cpu_execute_controlled.c').read_text()
    def compile_(folder,exec_header,cpu_header=None):
     folder.mkdir(parents=True,exist_ok=True);(folder/'fist_cpu.h').write_text(cpu if cpu_header is None else cpu_header)
     return target_build(repo,folder,exec_header,controlled_source=controlled)
    commands=compile_(work/'targets',header)
    recognized=contract['recognized_sign_query_tags'];assert len(recognized)==66
    shapes=((0xffff,-128),(0xfffe,127),(0x1fffe,127),(0x1ffff,-1),(0xfff,-128),(0x3fffc,-1),(0x1fffc,1),(0x1fffd,0))
    def make(code,big=0,stack=0,ip=0xfffe,tag=4,flags=0x3286):
     q=bytearray(packet(code,big=big,stack=stack,ip=ip,flags=flags));struct.pack_into('<I',q,14*4,tag);return bytes(q)
    cases=[]
    for big,width,address,stack,tag,(ip,delta) in itertools.product((0,1),(2,4),(2,4),(0,1),recognized,shapes):
     default=4 if big else 2;prefix=(b'\x66' if width!=default else b'')+(b'\x67' if address!=default else b'')
     cases.append(make(prefix+bytes((opcode,delta&255)),big,stack,ip,tag,(0x3206,0x3286)[len(cases)&1]))
    assert len(cases)==8448
    size=19*4+0x40000+13*4;hashes={name:hashlib.sha256() for name,_ in [('original',[original]),*commands]}
    for start in range(0,len(cases),128):
     batch=cases[start:start+128];data=b''.join(batch);expected=None
     for target,command in [('original',[original]),*commands]:
      p=subprocess.run(command,input=data,capture_output=True,timeout=45);assert p.returncode==0,(target,start,p.stderr.decode());assert len(p.stdout)==len(batch)*size
      if expected is None:expected=p.stdout
      else:assert p.stdout==expected,(target,start,next(i for i,(a,b) in enumerate(zip(p.stdout,expected)) if a!=b))
      hashes[target].update(p.stdout)
     if start%1024==0:print('Matched actual original',label,min(start+128,len(cases)),'/',len(cases),flush=True)
    branch='op==%#x?%sfist_cpu_sf(e->bus->cpu):'%(opcode,'!' if opcode==0x79 else '');assert header.count(branch)==1
    start=header.index('else if(op==0x76||');marker=header[start:header.index('{',start)+1];assert header.count(marker)==1
    mutations=[
     ('raw-sf',header.replace(branch,'op==%#x?%s(e->bus->cpu->flags.flags&0x80u):'%(opcode,'!' if opcode==0x79 else '!!')),[8]),
     ('operand-sized-sf',header.replace(branch,'op==%#x?%s(e->bus->cpu->flags.res&(width==2?0x8000u:0x80000000u)):'%(opcode,'!' if opcode==0x79 else '!!')),[8]),
     ('materialized-flags',header.replace(marker,marker+'if(op==%#x)fist_cpu_fill_flags(e->bus->cpu);'%opcode),[9,13]),
    ]
    results=[];data=make(bytes((opcode,0x7f)))
    def run(command):
     p=subprocess.run(command,input=data,capture_output=True,timeout=30);assert p.returncode==0,p.stderr.decode();assert len(p.stdout)==size;return p.stdout
    expected=run([original]);a=struct.unpack_from('<19I',expected);trace=struct.unpack_from('<13I',expected,size-52)
    for name,h,wanted in mutations:
     assert h!=header
     for target,command in commands:assert run(command)==expected,(name,target,'unmodified positive')
     for target,command in compile_(work/name,h):
      actual=run(command);b=struct.unpack_from('<19I',actual);other=struct.unpack_from('<13I',actual,size-52)
      fields=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y];assert fields==wanted,(name,target,fields)
      assert actual[76:size-52]==expected[76:size-52]
      if name=='materialized-flags':assert other==trace
      else:assert trace[0]==1+(opcode==0x79) and other[0]==1+(opcode==0x78)
      results.append(dict(fault=name,target=target,differing_CPU_words=fields,all256KiB_RAM_equal=True,original_code_reads=list(trace),mutant_code_reads=list(other)))
     print('PASS distinguish original',label,name,'on both targets',flush=True)
    # Omitting the original release diagnostic fallback must fail even with unchanged raw SF.
    data=make(bytes((opcode,0x7f)),tag=contract['enum']['t_ROLb'])
    expected=run([original])
    old='return width?!!(f->res & (1u<<(width-1))):0;'
    assert cpu.count(old)==1
    missing=cpu.replace(old,'fist_cpu_require(width);'+old)
    for target,command in commands:assert run(command)==expected
    for target,command in compile_(work/'missing-default',header,missing):
     p=subprocess.run(command,input=data,capture_output=True,timeout=30)
     assert p.returncode!=0 and p.stdout!=expected
     (work/(target+'-missing-default.log')).write_bytes(p.stderr)
     results.append(dict(fault='missing-release-default',target=target,terminal_exit=p.returncode,original_complete_bytes=len(expected),mutant_bytes=len(p.stdout),unmodified_positive_source_bytes_equal=True))
    proof=dict(scope='Actual CASE_W/D JNS/TFLG_NS programs match19 CPU/budget/raw/lazy words,all256KiB RAM and13 ordered code-read words. All65 tags plus LASTFLAG,both code/operand/address/stack modes,displacements and16-bit IP-high preservation/wrap are covered. Raw SF,operand-sized SF,eager flags and omitted release fallback faults fail both targets after their unmodified positives match.',cases=len(cases),record_size=size,outputs_sha256={name:h.hexdigest() for name,h in hashes.items()},causal_negatives=results)
    if opcode==0x78:proof['scope']=proof['scope'].replace('JNS/TFLG_NS','JS/TFLG_S')
    (work/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS original8448',label,'programs/8 causal results',flush=True)
    return proof


def capture(repo,root):
    assert root.is_relative_to(Path('/tmp'))
    root.mkdir(parents=True,exist_ok=False)
    sys.path.insert(0,str(repo/'tests'))
    limit=resource.getrlimit(resource.RLIMIT_CORE)
    resource.setrlimit(resource.RLIMIT_CORE,(0,limit[1]))
    try:
        recovered=contract(repo)
        (root/'source-contract.json').write_text(json.dumps(recovered,indent=2)+'\n')
        sign=sign_queries(repo,root,recovered)
        coded=programs(repo,root,recovered)
        tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
        paths=[Path(__file__),repo/'tools/oracle/capture_cpu_byte_instructions.py',
            repo/'tools/oracle/cpu_execute_probe.py',repo/'tools/oracle/cpu_execute_probe.cpp',
            repo/'tools/oracle/rep_probe.cpp',repo/'tests/cpu_execute_controlled.c',repo/'tests/test_cpu_execute.py',
            *sorted((repo/'re_out').glob('*.h')),*tree.rglob('*.h'),
            *[tree/p for p in ('config.h','src/cpu/flags.cpp','src/cpu/cpu.cpp','src/cpu/core_normal.cpp','src/cpu/modrm.cpp')],
            *sorted((root/'programs/source').glob('*.h'))]
        proof=dict(scope='Original-backed JNS and complete release get_SF contracts. Reaching first IRQ0 transport, actual startup/remaining handler/renderer/mixer and complete runtime frame/audio acceptance remain separate.',
            source_contract=recovered,sign_queries=sign,programs=coded,
            inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
        (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
        return proof
    finally:
        resource.setrlimit(resource.RLIMIT_CORE,limit)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();capture(args.repo.resolve(),args.output.resolve())
