#!/usr/bin/env python3
"""Complete original SHR/INC/lazy-state and boolean-query comparisons."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys

PROGRAMS=148672
RECORDS=PROGRAMS*4
RECORD_BYTES=72

def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def target_build(repo,root,header=None):
    sys.path.insert(0,str(repo/'tests'));from test_port_io import tool
    root.mkdir(parents=True,exist_ok=True)
    for name in ('cpu_shr_flags.c','cpu_shr_cases.inc'):(root/name).write_bytes((repo/'tests'/name).read_bytes())
    if header is not None:(root/'fist_cpu.h').write_text(header)
    commands=[]
    for target,compiler,options,out,runner in (
            ('native',['gcc','-m32'],['-Wl,--gc-sections'],root/'native',[]),
            ('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc')],
             ['-sNODERAWFS=1','-sEXIT_RUNTIME=1','-sALLOW_MEMORY_GROWTH=1'],root/'wasm.js',[tool('node','Git/emsdk/node/*/bin/node')])):
        result=subprocess.run([*compiler,'-O2','-DNDEBUG','-ffunction-sections','-fdata-sections',
            '-I'+str(root),'-I'+str(repo/'re_out'),str(root/'cpu_shr_flags.c'),*options,'-o',str(out)],capture_output=True,text=True,timeout=120)
        (root/(target+'-build.log')).write_text(result.stdout+result.stderr);assert result.returncode==0,result.stderr
        commands.append((target,[*runner,str(out)]))
    return commands

def output(command,path):
    result=subprocess.run([*command,str(path)],capture_output=True,text=True,timeout=60)
    assert result.returncode==0,result.stderr
    data=path.read_bytes();assert len(data)==RECORDS*RECORD_BYTES
    return data

def negative(repo,root,expected):
    header=(repo/'re_out/fist_cpu.h').read_text()
    begin=header.index('static inline uint32_t fist_cpu_shr(');end=header.index('static inline int fist_cpu_overflow(',begin)
    shift=header[begin:end]
    mutations=[
        ('getter-includes-sign',header.replace('materialize ? a>=sign : a>sign','materialize ? a>=sign : a>=sign'),[9]),
        ('fill-excludes-sign',header.replace('materialize ? a>=sign : a>sign','materialize ? a>sign : a>sign'),[9,10]),
        ('lost-upper-var1-res',header[:begin]+shift.replace('fist_cpu_low(f->var1,a,bits)','a').replace('fist_cpu_low(f->res,a>>count,bits)','a>>count')+header[end:],[0,2]),
        ('lost-upper-count',header[:begin]+shift.replace('fist_cpu_low(f->var2,count,8)','count')+header[end:],[1]),
        ('cf-uses-next-bit',header.replace('>>(count-1)) & 1','>>count) & 1'),[4]),
    ]
    results=[]
    for name,mutant,wanted in mutations:
        assert mutant!=header,name
        for target,run in target_build(repo,root/name,mutant):
            path=root/name/(target+'.raw');actual=output(run,path);assert actual!=expected,(name,target)
            first=next(i for i,(a,b) in enumerate(zip(actual,expected)) if a!=b);record=first//RECORD_BYTES
            a=struct.unpack_from('<10I4Q',expected,record*RECORD_BYTES);b=struct.unpack_from('<10I4Q',actual,record*RECORD_BYTES)
            fields=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y];assert fields==wanted,(name,target,fields)
            results.append(dict(fault=name,target=target,terminal_exit=0,complete_records=RECORDS,
                first_differing_record=record,differing_logical_fields=fields,original_record=list(a),mutant_record=list(b),sha256=hashlib.sha256(actual).hexdigest()))
            path.unlink()
        print('PASS distinguish original SHR flags',name,'on both targets',flush=True)
    return results

def capture(repo,root):
    assert root.is_relative_to(Path('/tmp'));root.mkdir(parents=True,exist_ok=False)
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    original=root/'original'
    result=subprocess.run(['g++','-O2','-std=gnu++11','-ffunction-sections','-fdata-sections',
        *subprocess.check_output(['sdl-config','--cflags'],text=True).split(),'-I'+str(tree/'include'),'-I'+str(tree),
        str(repo/'tools/oracle/cpu_shr_flags_probe.cpp'),'-Wl,--gc-sections','-o',str(original)],capture_output=True,text=True,timeout=120)
    (root/'original-build.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stderr
    results=[];expected=None
    for target,run in [('original',[str(original)]),*target_build(repo,root)]:
        path=root/(target+'.raw');data=output(run,path)
        if expected is None:expected=data
        else:assert data==expected,(target,'Complete raw/lazy/operand/query records differ')
        results.append(dict(target=target,programs=PROGRAMS,records=RECORDS,bytes=len(data),sha256=hashlib.sha256(data).hexdigest()))
        path.unlink()
    causal=negative(repo,root/'negative',expected)
    paths=[Path(__file__),repo/'tools/oracle/cpu_shr_flags_probe.cpp',repo/'tests/cpu_shr_flags.c',
        repo/'tests/cpu_shr_cases.inc',repo/'re_out/fist_cpu.h',*tree.rglob('*.h'),tree/'src/cpu/flags.cpp']
    proof=dict(scope='Verbatim original SHRB/W/D, INC and get_CF/PF/AF/ZF/SF/OF/FillFlags bodies compared to shared CPU producers and boolean OF/SF queries. All byte values/counts0..31, every word value at reached count1, other word/DWORD edge values/counts, dirty upper lazy fields, zero-count preservation and carry-preserving INC are covered. Every original-width64-bit raw flags/type/prev/oldcf value is emitted. Five causal faults distinguish getter/materialization/sign/count/upper-state/CF causes on both optimized release targets. Encoded instructions, protected/fault/whole-runtime and complete frame/audio acceptance remain separate.',results=results,causal_negatives=causal,inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS original148672 SHR programs/594688 complete records and10 causal negatives',flush=True)
    return proof

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();capture(args.repo.resolve(),args.output.resolve())
