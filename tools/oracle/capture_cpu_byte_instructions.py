#!/usr/bin/env python3
"""Actual original AND AL/JCXZ instruction and exhaustive byte-flag contracts."""
import argparse,hashlib,itertools,json,struct,subprocess,sys
from pathlib import Path
from cpu_execute_probe import build as original_build

def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def target_build(repo,root,header=None,*,segment_input=False,stack_header=None,group_shift=False,ram_trace=False,controlled_source=None):
    sys.path.insert(0,str(repo/'tests'));from test_port_io import tool
    root.mkdir(parents=True,exist_ok=True)
    (root/'controlled.c').write_text(controlled_source if controlled_source is not None else (repo/'tests/cpu_execute_controlled.c').read_text())
    if header is not None:(root/'fist_exec.h').write_text(header)
    if stack_header is not None:(root/'fist_interrupt.h').write_text(stack_header)
    flags=['-DFIST_EXECUTE_SEGMENT_INPUT'] if segment_input else []
    if group_shift:
        assert segment_input
        flags.append('-DFIST_EXECUTE_GROUP_SHIFT')
    if ram_trace:
        assert group_shift
        flags.append('-DFIST_EXECUTE_RAM_TRACE')
    commands=[]
    for name,compiler,options,output,runner in (
        ('native',['gcc','-m32'],[],root/'native',[]),
        ('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc')],['-sNODERAWFS=1','-sEXIT_RUNTIME=1','-sALLOW_MEMORY_GROWTH=1'],root/'wasm.js',[tool('node','Git/emsdk/node/*/bin/node')])):
        p=subprocess.run([*compiler,'-O2','-DNDEBUG','-DFIST_EXECUTE_FETCH_TRACE','-DFIST_EXECUTE_LAZY_INPUT',*flags,'-I'+str(root),'-I'+str(repo/'re_out'),str(root/'controlled.c'),*options,'-o',str(output)],capture_output=True,text=True,timeout=120)
        (root/(name+'-build.log')).write_text(p.stdout+p.stderr);assert p.returncode==0,p.stderr
        commands.append((name,[*runner,str(output)]))
    return commands

def jcxz_cases(packet):
    cases=[]
    def add(big,width,address,stack,ip,ecx,delta,lazy=0,flags=0x3003,budget=7):
        default=4 if big else 2
        code=(b'\x66' if width!=default else b'')+(b'\x67' if address!=default else b'')+bytes((0xe3,delta&255))
        q=bytearray(packet(code,big=big,stack=stack,ip=ip,flags=flags,ecx=ecx,budget=budget))
        struct.pack_into('<I',q,14*4,lazy);cases.append(bytes(q))
    for big,width,address,stack,ip,ecx,delta in itertools.product((0,1),(2,4),(2,4),(0,1),
            (0xfffc,0xfffd,0xfffe,0xffff,0x10000,0x1fffe,0x2ffff,0x3fffc),
            (0,1,0x10000,0xffff0000,0xffff,0xffffffff),(-128,-2,-1,0,1,127)):
        add(big,width,address,stack,ip,ecx,delta,budget=1 if len(cases)&1 else 7)
    for big,width,address,lazy,flags,ecx in itertools.product((0,1),(2,4),(2,4),
            (0,1,2,3,22,23,24,31,32),(0,0x3003,0xffffffff,0x3246),(0,0x10000,1)):
        add(big,width,address,0,0xffff,ecx,127,lazy=lazy,flags=flags)
    assert len(cases)==5472
    return cases

def and_cases(packet):
    cases=[]
    for context,a,b in itertools.product(range(8),(0,1,0x80,0xff),range(256)):
        big=context&1;stack=(context>>1)&1
        prefix=(b'\x66' if context&2 else b'')+(b'\x67' if context&4 else b'')
        q=bytearray(packet(prefix+bytes((0x24,b)),big=big,stack=stack,ip=(0xfffe,0xffff,0x1fffe,0x1ffff)[context%4],flags=(0x3003,0xffffffff)[context&1],budget=(1,7)[context&1]))
        struct.pack_into('<I',q,4*4,(0x76543200,0xffffff00)[context&1]|a)
        struct.pack_into('<I',q,14*4,(0,3,22,32)[context%4]);cases.append(bytes(q))
    assert len(cases)==8192
    return cases

def negative(repo,root,original,packet):
    header=(repo/'re_out/fist_exec.h').read_text()
    branch='else if(op==0xe3)fist_exec_conditional(e,&ip,width,1,!fist_exec_reg_read(e,1,address));'
    assert header.count(branch)==1
    def seed(code,ip=0xfffe,ecx=0x12340001,lazy=0):
        q=bytearray(packet(code,ip=ip,ecx=ecx));struct.pack_into('<I',q,14*4,lazy);return bytes(q)
    mutations={
        'operand-sized-counter':(header.replace(branch,branch.replace('e,1,address','e,1,width')),seed(b'\x66\xe3\x7f',ecx=0x10000),[8]),
        'cleared-upper-IP':(header.replace('*ip=width==2?(saved&0xffff0000u)|(next&0xffffu):next;','*ip=width==2?next&0xffffu:next;'),seed(b'\xe3\x07',ip=0x1fffe,ecx=0),[8]),
        'eager-flags':(header.replace(branch,'else if(op==0xe3){fist_cpu_fill_flags(e->bus->cpu);'+branch.split(')',1)[1]+'}'),seed(b'\xe3\x07',lazy=1),[9,13]),
        'decremented-counter':(header.replace(branch,'else if(op==0xe3){fist_exec_reg_write(e,1,address,fist_exec_reg_read(e,1,address)-1);'+branch.split(')',1)[1]+'}'),seed(b'\xe3\x07',lazy=32),[1,8]),
        'eager-displacement':(header.replace('if(take)delta=','delta=').replace('uint32_t next=saved+delta+displacement;','if(!take)delta=0;uint32_t next=saved+delta+displacement;'),seed(b'\xe3\x01',ip=0xfff),[]),
    }
    results=[];base=19*4+0x40000;size=base+13*4
    for name,(mutant,inputs,wanted) in mutations.items():
        assert mutant!=header
        p=subprocess.run([original],input=inputs,capture_output=True,timeout=30);assert p.returncode==0 and len(p.stdout)==size
        expected=p.stdout;ow=struct.unpack_from('<19I',expected);otrace=struct.unpack_from('<13I',expected,base)
        for target,command in target_build(repo,root/name,mutant):
            p=subprocess.run(command,input=inputs,capture_output=True,timeout=30);assert p.returncode==0 and len(p.stdout)==size,p.stderr
            aw=struct.unpack_from('<19I',p.stdout);atrace=struct.unpack_from('<13I',p.stdout,base)
            differences=[i for i,(a,b) in enumerate(zip(aw,ow)) if a!=b];assert differences==wanted,(name,target,differences)
            assert p.stdout[19*4:base]==expected[19*4:base]
            if name=='eager-displacement':
                assert aw==ow and otrace[0]==1 and atrace[0]==2
                assert otrace[1:4]==atrace[1:4]==(1,0xfff,1) and atrace[4:7]==(1,0x1000,1)
            if name in ('cleared-upper-IP','eager-flags'):assert atrace==otrace
            results.append(dict(case=name,target=target,differing_CPU_words=differences,all256KiB_RAM_equal=True,original_reads=list(otrace),actual_reads=list(atrace)))
    return results

def capture(repo,root):
    root.mkdir(parents=True,exist_ok=False)
    sys.path.insert(0,str(repo/'tests'))
    from test_cpu_execute import packet
    from cpu_core_exit_fixture import build
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    byte=root/'byte-flags';byte.mkdir()
    reference=byte/'original'
    p=subprocess.run(['g++','-O2','-std=gnu++11','-DFIST_BYTE_AND',*subprocess.check_output(['sdl-config','--cflags'],text=True).split(),'-I'+str(tree/'include'),'-I'+str(tree),'-ffunction-sections','-fdata-sections',str(repo/'tools/oracle/cpu_byte_add_probe.cpp'),'-Wl,--gc-sections','-o',str(reference)],capture_output=True,text=True,timeout=120)
    (byte/'original-build.log').write_text(p.stdout+p.stderr);assert p.returncode==0,p.stderr
    commands=[('original',[str(reference)]),*build(byte,driver=repo/'tests/cpu_byte_add.c',extra_flags=('-DFIST_BYTE_ALU_OPERATION=4',))]
    expected=None;byte_results=[]
    for target,run in commands:
        output=byte/(target+'.raw');p=subprocess.run([*run,str(output)],capture_output=True,timeout=30);assert p.returncode==0,p.stderr
        data=output.read_bytes();assert len(data)==131072*4*10*4
        if expected is None:expected=data
        else:assert data==expected,target
        byte_results.append(dict(target=target,records=524288,bytes=len(data),sha256=hashlib.sha256(data).hexdigest()));output.unlink()
    instruction=root/'instruction';instruction.mkdir()
    original=original_build(repo,instruction,extra_opcodes=(0xe3,),byte_opcodes=(0x24,),trace=True,lazy_input=True)
    silent=root/'silent';silent.mkdir()
    original_silent=original_build(repo,silent,extra_opcodes=(0xe3,),byte_opcodes=(0x24,),lazy_input=True)
    commands=target_build(repo,instruction)
    hashes={};base=19*4+0x40000;record_size=base+13*4
    for name,cases in (('AND-AL',and_cases(packet)),('JCXZ',jcxz_cases(packet))):
        current={n:hashlib.sha256() for n in ('original','original-silent','native','wasm')}
        for start in range(0,len(cases),128):
            batch=cases[start:start+128];data=b''.join(batch)
            p=subprocess.run([original],input=data,capture_output=True,timeout=30);assert p.returncode==0 and len(p.stdout)==len(batch)*record_size,p.stderr
            expected=p.stdout;current['original'].update(expected)
            p=subprocess.run([original_silent],input=data,capture_output=True,timeout=30);assert p.returncode==0 and len(p.stdout)==len(batch)*base,p.stderr
            assert p.stdout==b''.join(expected[i*record_size:i*record_size+base] for i in range(len(batch)));current['original-silent'].update(p.stdout)
            for target,run in commands:
                p=subprocess.run(run,input=data,capture_output=True,timeout=30);assert p.returncode==0,p.stderr
                assert p.stdout==expected,(name,target,start,'Complete CPU/RAM/code-read outputs differ');current[target].update(p.stdout)
        hashes[name]={n:h.hexdigest() for n,h in current.items()}
    results=negative(repo,root/'negative',original,packet)
    paths=[Path(__file__),repo/'tools/oracle/cpu_execute_probe.py',repo/'tools/oracle/cpu_execute_probe.cpp',repo/'tools/oracle/rep_probe.cpp',repo/'tools/oracle/cpu_byte_add_probe.cpp',repo/'tests/cpu_byte_add.c',repo/'tests/cpu_execute_controlled.c',repo/'re_out/fist_exec.h',*tree.rglob('*.h'),tree/'src/cpu/flags.cpp',tree/'src/cpu/cpu.cpp',tree/'src/cpu/core_normal.cpp',*instruction.glob('*.h')]
    proof=dict(scope='Exhaustive131072 original byte AND/FillFlags/INC programs and8192 actual AND AL,Ib plus5472 JCXZ cases: complete19 CPU/budget/lazy words, all256KiB RAM and every code-byte read match both release targets. Silent original observations preserve every CPU/RAM byte. Five JCXZ substitutions distinguish width/wrap/flag/counter/read-page contracts. No runtime/whole handler/frame/audio acceptance.',byte_results=byte_results,instruction_outputs_sha256=hashes,AND_cases=8192,JCXZ_cases=5472,causal_negatives=results,inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS source byte flags/AND AL/JCXZ complete controls and10 causal negatives',flush=True)
    return proof

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--repo',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();capture(a.repo.resolve(),a.output.resolve())
