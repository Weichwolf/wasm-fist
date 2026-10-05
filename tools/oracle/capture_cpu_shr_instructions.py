#!/usr/bin/env python3
"""Compare actual original D0/D1 SHR decoding, flags and ordered RAM accesses."""
import argparse,hashlib,itertools,json,struct,subprocess,sys
from pathlib import Path
from cpu_execute_probe import build as original_build
from capture_cpu_byte_instructions import target_build,digest

RECORD_SIZE=31*4+3*8+0x200000+49*4+65*4

def cases():
    cases=[]
    segments=[v for selector in (0x10,0x2082,0x26e,0x2d19,0x4000,0x6000) for v in (selector,selector<<4)]
    def add(code,big,stack,ip=0x3b38,direct=False):
     ordinal=len(cases)
     regs=[0x12348000,0x89ab9000,0x7654a000,0xfeed4000,0xcafe5000,0xbeef6000,0xabcd7000,0xdcba8000] if direct else [0x8000,0x9000,0xa000,0x4000,0x5000,0x6000,0x7000,0x8000]
     q=[big,stack,ip,(0x202,0xfedcba98)[ordinal&1],*regs,1 if ordinal&1 else 0xffffffff,(1,2,17,1796)[ordinal%4],ordinal%65,len(code)]
     cases.append(struct.pack('<28I',*q,*segments)+code)
    for big,width,address,stack,segment in itertools.product((0,1),(1,2,4),(2,4),(0,1),(None,0x26,0x2e,0x36,0x3e,0x64,0x65)):
     default=4 if big else 2
     prefix=(b'\x66' if width!=1 and width!=default else b'')+(b'\x67' if address!=default else b'')+(bytes([segment]) if segment is not None else b'')
     opcode=0xd0 if width==1 else 0xd1
     for mode in range(3):
      for rm in range(8):
       code=prefix+bytes([opcode,(mode<<6)|(5<<3)|rm])
       if address==4 and rm==4:code+=b'\x9c' # ESP base + EBX index *4, original SIB table.
       if mode==0 and rm==(6 if address==2 else 5):code+=(0x450).to_bytes(address,'little')
       elif mode==1:code+=b'\x80'
       elif mode==2:code+=(-0x100).to_bytes(address,'little',signed=True)
       add(code,big,stack,ip=0xffff if len(cases)&1 else 0x3b38)
       if width==1:add(b'\x66'+code,big,stack)
     for rm in range(8):
      add(prefix+bytes([opcode,0xc0|(5<<3)|rm]),big,stack,direct=True)
      if width==1:add(b'\x66'+prefix+bytes([opcode,0xc0|(5<<3)|rm]),big,stack,direct=True)
    # All original SIB base/index/scale combinations in each displacement mode.
    for mode,sib in itertools.product(range(3),range(256)):
     code=bytes([0xd1,(mode<<6)|(5<<3)|4,sib])
     if mode==0 and (sib&7)==5:code+=struct.pack('<I',0x450)
     elif mode==1:code+=b'\x80'
     elif mode==2:code+=struct.pack('<i',-0x100)
     add(code,1,0)
    assert len(cases)==7936
    return cases

def packet(code,*,regs=None,stack=0):
 if regs is None:regs=[0xfedcba98,0x9000,0xa000,0x4000,0x5000,0x6000,0x7000,0x8000]
 q=[0,stack,0x3b38,0xfedcba98,*regs,1,1796,1,len(code)]
 segments=[v for selector in (0x10,0x2082,0x26e,0x2d19,0x4000,0x6000) for v in (selector,selector<<4)]
 return struct.pack('<28I',*q,*segments)+code
def operand_mutation(text,old,new):
 begin=text.index('static FistExecOperand fist_exec_operand(');end=text.index('static uint32_t fist_exec_read_op(',begin)
 body=text[begin:end];assert old in body
 return text[:begin]+body.replace(old,new)+text[end:]
def parse(b):
 assert len(b)==RECORD_SIZE
 return struct.unpack_from('<31I',b),struct.unpack_from('<3Q',b,124),b[148:148+0x200000],struct.unpack_from('<49I',b,148+0x200000),struct.unpack_from('<65I',b,148+0x200000+49*4)

def negative(repo,root,original,commands):
    header=(repo/'re_out/fist_exec.h').read_text();portable=(repo/'tests/cpu_execute_controlled.c').read_text()
    word='bytes=op==0xd0?1:width';assert header.count(word)==1
    registers=[0xfedcba98,0x9000,0xa000,0xffff,0x15000,0x6000,0x8001,0x8000]
    mutations=[
     ('widened-byte-operand',header.replace(word,'bytes=width'),portable,packet(b'\x66\xd0\xe8'),[0,10,12,13],False),
     ('word-only-d1',header.replace(word,'bytes=op==0xd0?1:2'),portable,packet(b'\x66\xd1\xe8'),[0,10,12,13],False),
     ('bp-uses-ds',operand_mutation(header,'q.seg=2','q.seg=3'),portable,packet(b'\xd1\x6e\x00'),None,True),
     ('ignored-cs-override',operand_mutation(header,'if(seg<6)q.seg=seg','if(seg<6 && seg!=1)q.seg=seg'),portable,packet(b'\x2e\xd1\x2e\x50\x04'),None,True),
     ('lost-word-ea-wrap',operand_mutation(header,'q.offset&=0xffff;',''),portable,packet(b'\xd1\x28',regs=registers),None,True),
     ('esp-sib-uses-ds',operand_mutation(header,'if(b==4 || b==5)q.seg=2;','if(b==5)q.seg=2;'),portable,packet(b'\x67\xd1\x2c\x24'),None,True),
     ('stack-sized-address',operand_mutation(header,'if(seg<6)q.seg=seg;return q;','if(q.seg==2)q.offset&=e->bus->cpu->stack_mask;if(seg<6)q.seg=seg;return q;'),portable,packet(b'\x67\xd1\x2c\x24',regs=registers),None,True),
    ]
    # Route CS data through the wrong code observer while keeping all RAM observations.
    observer=portable.replace('  engine.code_fetch=observe_code_fetch;','  engine.code_fetch=NULL;')
    anchor=' uint32_t value=fist_ram_resident_read(bus,segment,offset,width);'
    assert observer.count(anchor)==1
    observer=observer.replace(anchor,' if(segment==1)observe_fetch(segment,offset,width);\n'+anchor)
    mutations.append(('cs-data-is-code',header,observer,packet(b'\x2e\xd1\x2e\x50\x04'),[],False))
    results=[]
    def run(command,data):
        p=subprocess.run(command,input=data,capture_output=True,timeout=30)
        assert p.returncode==0,p.stderr.decode();parse(p.stdout);return p.stdout
    for name,h,p,data,wanted,memory_difference in mutations:
        assert h!=header or p!=portable,name
        expected=run([original],data);a,sa,ma,fa,ra=parse(expected)
        for target,command in commands:assert run(command,data)==expected,(name,target,'unmodified positive')
        for target,command in target_build(repo,root/name,h,segment_input=True,group_shift=True,ram_trace=True,controlled_source=p):
            actual=run(command,data);b,sb,mb,fb,rb=parse(actual);assert actual!=expected,(name,target)
            fields=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y]
            offsets=[i for i,(x,y) in enumerate(zip(ma,mb)) if x!=y]
            assert sa==sb,(name,target)
            if wanted is not None:assert fields==wanted,(name,target,fields)
            assert bool(offsets)==memory_difference,(name,target,len(offsets))
            if name=='cs-data-is-code':assert fa!=fb and ra==rb
            else:
                assert fa==fb,(name,target)
                assert (ra!=rb)==memory_difference,(name,target)
            results.append(dict(fault=name,target=target,terminal_exit=0,unmodified_positive_source_bytes_equal=True,
                differing_CPU_cache_words=fields,memory_differences=len(offsets),first_memory_difference=offsets[0] if offsets else None,
                code_fetches_equal=fa==fb,all_RAM_accesses_equal=ra==rb,original_code_fetches=list(fa),mutant_code_fetches=list(fb),
                original_RAM_accesses=list(ra),mutant_RAM_accesses=list(rb),input_sha256=hashlib.sha256(data).hexdigest(),
                original_sha256=hashlib.sha256(expected).hexdigest(),mutant_sha256=hashlib.sha256(actual).hexdigest()))
        print('PASS distinguish encoded SHR',name,'on both targets',flush=True)
    return results

def capture(repo,root):
    assert root.is_relative_to(Path('/tmp'));root.mkdir(parents=True,exist_ok=False)
    source=root/'source';source.mkdir()
    original=original_build(repo,source,trace=True,lazy_input=True,segment_input=True,group_shift=True,ram_trace=True)
    commands=target_build(repo,root/'targets',segment_input=True,group_shift=True,ram_trace=True)
    runs=[('original',[original]),*commands]
    corpus=cases();hashes={name:hashlib.sha256() for name,_ in runs}
    for start in range(0,len(corpus),16):
        batch=corpus[start:start+16];data=b''.join(batch);expected=None
        for target,run in runs:
            p=subprocess.run(run,input=data,capture_output=True,timeout=45)
            assert p.returncode==0,(target,start,p.stderr.decode())
            assert len(p.stdout)==len(batch)*RECORD_SIZE,(target,start,'incomplete output')
            if expected is None:expected=p.stdout
            else:assert p.stdout==expected,(target,start,next(i for i,(a,b) in enumerate(zip(p.stdout,expected)) if a!=b))
            hashes[target].update(p.stdout)
        if start%512==0:print('Matched encoded SHR',min(start+16,len(corpus)),'/',len(corpus),flush=True)
    negatives=negative(repo,root/'negative',original,commands)
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    paths=[Path(__file__),repo/'tools/oracle/capture_cpu_byte_instructions.py',repo/'tools/oracle/cpu_execute_probe.py',
        repo/'tools/oracle/cpu_execute_probe.cpp',repo/'tools/oracle/rep_probe.cpp',repo/'tests/cpu_execute_controlled.c',
        *sorted((repo/'re_out').glob('*.h')),*tree.rglob('*.h'),
        tree/'src/cpu/flags.cpp',tree/'src/cpu/cpu.cpp',tree/'src/cpu/modrm.cpp',tree/'src/cpu/core_normal.cpp',*source.glob('*.h')]
    proof=dict(scope='Actual original D0/D1 CASE_B/W/D and full GRP2/EA/ModRM source owners versus shared execution with matched cached segments. Complete31 CPU/cache words, full64-bit stack metadata, all2MiB RAM, every actual code-fetch read including word/DWORD displacement and every ordered physical guest RAM read/write/value compare. CS data remains a RAM read and does not become a code fetch. Both code/address/stack sizes, byte/WORD/DWORD operands, every segment override/direct register and all original SIB combinations are covered. Eight causal faults fail on both release targets after their unmodified positives match the source. Protected/paging/provider faults, whole runtime and complete frame/audio acceptance remain separate.',
        cases=len(corpus),record_size=RECORD_SIZE,outputs_sha256={name:h.hexdigest() for name,h in hashes.items()},
        corpus_sha256=hashlib.sha256(b''.join(corpus)).hexdigest(),causal_negatives=negatives,
        inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS original7936 encoded SHR programs and16 causal negatives on both release targets',flush=True)
    return proof

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--repo',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();capture(a.repo.resolve(),a.output.resolve())
