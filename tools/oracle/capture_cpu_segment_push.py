#!/usr/bin/env python3
"""Compare complete original segment-push instructions and reaching fault variants."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import re
import struct
import subprocess
from cpu_execute_probe import build as original_build
from capture_cpu_byte_instructions import target_build

RAM_BYTES=0x200000
STATE_BYTES=31*4+3*8
RECORD_BYTES=STATE_BYTES+RAM_BYTES+13*4

def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def packet(code,*,big=0,stack=0,ip=0x3abb,sp=0x80,lazy=1,flags=0xfedcba98,
           selectors=(0x10,0x2082,0x26e,0x19f5,0,0),direction=1,budget=1796):
    q=[big,stack,ip,flags,0x12345678,0x89abcdef,0x76543210,0xfeedbeef,
       sp,0xfedcba98,0x13579bdf,0x2468ace0,direction&0xffffffff,budget,lazy,len(code)]
    segments=[value for selector in selectors for value in (selector,selector<<4)]
    return struct.pack('<28I',*q,*segments)+code

def cases(tree):
    text=(tree/'src/cpu/lazyflags.h').read_text()
    tags=re.findall(r'\bt_[A-Za-z0-9_]+\b',text[text.index('t_UNKNOWN=0'):text.index('t_LASTFLAG')])
    assert len(tags)==len(set(tags))
    selectors=((0x10,0x2082,0x26e,0x19f5,0,0),(0xffff,0xffff,0xffff,0xfffe,0xfffd,0xfffc),
               (0,1,2,3,4,5),(0x4000,0x8000,0x7fff,0x1234,0x5678,0xabcd))
    programs=[]
    for op,big,operand_override,address_override,stack,sel,flags in itertools.product(
            (0x06,0x0e,0x1e),(0,1),(0,1),(0,1),(0,1),selectors,(0x202,0xfedcba98)):
        sps=(0,1,2,3,0xfffc,0xfffd,0xfffe,0xffff,0xcafe0000,0xfeedffff) if not stack else (4,5,0xffff,0x10000,0x1ffff,0x100000)
        for sp in sps:
            ordinal=len(programs)
            code=(b'\x66' if operand_override else b'')+(b'\x67' if address_override else b'')+bytes([op])
            programs.append(packet(code,big=big,stack=stack,ip=(0x3abb,0xfffd,0xfffe,0xffff)[ordinal%4],
                sp=sp,lazy=ordinal%len(tags),flags=flags,selectors=sel,direction=1 if ordinal&1 else -1,
                budget=(1,2,17,1796)[ordinal%4]))
    assert len(programs)==3072
    return programs,len(tags)

def parse(data):
    assert len(data)==RECORD_BYTES
    return (struct.unpack_from('<31I',data),struct.unpack_from('<3Q',data,124),
            data[STATE_BYTES:STATE_BYTES+RAM_BYTES],data[STATE_BYTES+RAM_BYTES:])

def run(command,data):
    result=subprocess.run(command,input=data,capture_output=True,timeout=45)
    assert result.returncode==0,(command,result.stderr.decode())
    return result.stdout

def negative(repo,root,original):
    header=(repo/'re_out/fist_exec.h').read_text()
    stack_header=(repo/'re_out/fist_interrupt.h').read_text()
    route='else if(op==0x06||op==0x0e||op==0x16||op==0x1e)fist_cpu_push(e->bus,width,e->bus->cpu->segments[op==0x06?0:op==0x0e?1:op==0x16?2:3].value);'
    assert header.count(route)==1
    mutations=[
        ('wrong-cs-selector',header.replace('op==0x0e?1:op==0x16?2:3','op==0x0e?3:op==0x16?2:3'),stack_header,packet(b'\x0e')),
        ('fixed16-operand',header.replace(route,route.replace('fist_cpu_push(e->bus,width,','fist_cpu_push(e->bus,op==0x0e?2:width,')),stack_header,packet(b'\x66\x0e')),
        ('address-controls-stack',header.replace(route,'else if(op==0x06||op==0x0e||op==0x16||op==0x1e) {if(op==0x0e)e->bus->cpu->stack_mask=address==4?UINT32_MAX:0xffff;fist_cpu_push(e->bus,width,e->bus->cpu->segments[op==0x06?0:op==0x0e?1:op==0x16?2:3].value);}'),stack_header,packet(b'\x67\x0e',sp=0x10080)),
        ('word-only-dword-write',header,stack_header.replace('fist_ram_resident_write(bus,2,next&cpu->stack_mask,width,value);','fist_ram_resident_write(bus,2,next&cpu->stack_mask,width==4?2:width,value);'),packet(b'\x66\x0e')),
        ('materialized-flags',header.replace(route,route.replace('fist_cpu_push(e->bus,','{fist_cpu_fill_flags(e->bus->cpu);fist_cpu_push(e->bus,')+'}'),stack_header,packet(b'\x0e')),
        ('lost-upper-esp',header,stack_header.replace('cpu->esp=next;','cpu->esp=next&cpu->stack_mask;'),packet(b'\x0e',sp=0xcafe0080)),
    ]
    results=[]
    for name,h,s,data in mutations:
        assert h!=header or s!=stack_header,name
        expected=run([original],data);a,sa,ma,fa=parse(expected)
        for target,command in target_build(repo,root/name,h,segment_input=True,stack_header=s):
            actual=run(command,data);b,sb,mb,fb=parse(actual);assert actual!=expected,(name,target)
            fields=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y]
            metadata=[i for i,(x,y) in enumerate(zip(sa,sb)) if x!=y]
            offsets=[i for i,(x,y) in enumerate(zip(ma,mb)) if x!=y]
            assert fa==fb,(name,target)
            if name=='wrong-cs-selector':assert not fields and not metadata and offsets
            elif name=='fixed16-operand':assert fields==[4] and not metadata and offsets
            elif name=='address-controls-stack':assert metadata==[1] and offsets
            elif name=='word-only-dword-write':
                assert not fields and not metadata and offsets and set(offsets)<=set(range(0x26e0+0x7c+2,0x26e0+0x7c+4))
            elif name=='materialized-flags':assert 13 in fields and set(fields)<=set((9,13)) and not metadata and not offsets
            elif name=='lost-upper-esp':assert fields==[4] and not metadata and not offsets
            results.append(dict(fault=name,target=target,terminal_exit=0,differing_CPU_segment_words=fields,
                differing_full64_stack_words=metadata,memory_differences=len(offsets),
                first_memory_difference=offsets[0] if offsets else None,fetch_trace_equal=True,
                original_sha256=hashlib.sha256(expected).hexdigest(),mutant_sha256=hashlib.sha256(actual).hexdigest(),
                input_sha256=hashlib.sha256(data).hexdigest()))
        print('PASS distinguish',name,'on both targets',flush=True)
    return results

def capture(repo,root):
    assert root.is_relative_to(Path('/tmp'));root.mkdir(parents=True,exist_ok=False)
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    programs,lazy_tags=cases(tree)
    original=original_build(repo,root,extra_opcodes=(0x06,0x0e,0x1e),trace=True,lazy_input=True,segment_input=True)
    silent=root/'silent';silent.mkdir()
    original_silent=original_build(repo,silent,extra_opcodes=(0x06,0x0e,0x1e),lazy_input=True,segment_input=True)
    commands=[('original',[original]),*target_build(repo,root,segment_input=True)]
    hashes={name:hashlib.sha256() for name,_ in commands};hashes['original-silent']=hashlib.sha256()
    inputs=hashlib.sha256()
    for start in range(0,len(programs),16):
        batch=programs[start:start+16];data=b''.join(batch);inputs.update(data);expected=None
        for target,command in commands:
            output=run(command,data);assert len(output)==len(batch)*RECORD_BYTES
            if expected is None:expected=output
            else:assert output==expected,(target,start,'Complete CPU/segments/stack/RAM/fetch output differs')
            hashes[target].update(output)
        output=run([original_silent],data)
        base=STATE_BYTES+RAM_BYTES
        assert len(output)==len(batch)*base
        assert output==b''.join(expected[i*RECORD_BYTES:i*RECORD_BYTES+base] for i in range(len(batch)))
        hashes['original-silent'].update(output)
        if start%512==0:print('Matched segment PUSH programs',min(start+16,len(programs)),'/',len(programs),flush=True)
    results=negative(repo,root/'negative',original)
    paths=[Path(__file__),repo/'tools/oracle/cpu_execute_probe.py',repo/'tools/oracle/cpu_execute_probe.cpp',
        repo/'tools/oracle/rep_probe.cpp',repo/'tools/oracle/capture_cpu_byte_instructions.py',
        repo/'tests/cpu_execute_controlled.c',repo/'re_out/fist_exec.h',repo/'re_out/fist_interrupt.h',
        repo/'re_out/fist_cpu.h',*tree.rglob('*.h'),tree/'src/cpu/flags.cpp',tree/'src/cpu/cpu.cpp',
        tree/'src/cpu/core_normal.cpp',*root.glob('*.h'),*silent.glob('*.h')]
    proof=dict(scope='Verbatim original CASE_W/D PUSH ES/CS/DS and CPU_Push16/32 versus the shared port stack owner. '
        'All3072 matched programs compare19 CPU/budget/lazy/direction/code words,12 cached segment words, '
        'three full original-width64-bit stack metadata words, all2MiB RAM and13 fetch words. '
        'Original stack initialization follows cpu.cpp; silent original observations retain all CPU/cache/stack/RAM bytes. '
        'Six causal faults are distinguished on both release targets. PUSH SS, protected segment faults, '
        'actual startup transport, whole handler and complete runtime frame/audio acceptance remain open.',
        cases=len(programs),valid_source_lazy_tags=lazy_tags,record_size=RECORD_BYTES,
        outputs_sha256={name:h.hexdigest() for name,h in hashes.items()},input_sha256=inputs.hexdigest(),
        causal_negatives=results,inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS complete original3072 segment PUSH programs and12 causal negatives',flush=True)
    return proof

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();capture(args.repo.resolve(),args.output.resolve())
