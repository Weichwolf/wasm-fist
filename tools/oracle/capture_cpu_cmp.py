#!/usr/bin/env python3
"""Compare actual original CMP 3b decoding, lazy flags and every RAM access."""
import argparse,hashlib,itertools,json,resource,struct,subprocess,sys
from pathlib import Path
from cpu_execute_probe import build as original_build
from capture_cpu_byte_instructions import target_build,digest
from capture_cpu_shr_instructions import RECORD_SIZE,parse,operand_mutation

segments=[v for selector in (0x10,0x2082,0x26e,0x2d19,0x4000,0x6000) for v in (selector,selector<<4)]

def cases():
    corpus=[]
    def packet(code,big=0,stack=0,regs=None,lazy=None):
     n=len(corpus)
     if regs is None:regs=[0x8000,0x9000,0xa000,0x4000,0x5000,0x6000,0x7000,0x8000]
     q=[big,stack,(0xffff,0x2f3b,0x1fffe)[n%3],(0x3246,0xfedcba98)[n&1],*regs,(-1 if n&1 else 1)&0xffffffff,(1,2,17,1796)[n%4],n%65 if lazy is None else lazy,len(code)]
     return struct.pack('<28I',*q,*segments)+code
    for big,width,address,stack,segment in itertools.product((0,1),(2,4),(2,4),(0,1),(None,0x26,0x2e,0x36,0x3e,0x64,0x65)):
     default=4 if big else 2
     prefix=(b'\x66' if width!=default else b'')+(b'\x67' if address!=default else b'')+(bytes([segment]) if segment is not None else b'')
     for mode,rm in itertools.product(range(3),range(8)):
      reg=len(corpus)%8;code=prefix+bytes([0x3b,(mode<<6)|(reg<<3)|rm])
      if address==4 and rm==4:code+=b'\x9c'
      if mode==0 and rm==(6 if address==2 else 5):code+=(0x15ba).to_bytes(address,'little')
      elif mode==1:code+=b'\x80'
      elif mode==2:code+=(-0x100).to_bytes(address,'little',signed=True)
      corpus.append(packet(code,big,stack))
     for rm in range(8):
      corpus.append(packet(prefix+bytes([0x3b,0xc0|((7-rm)<<3)|rm]),big,stack,regs=[0x12348000,0x89ab9000,0x7654a000,0xfeed4000,0xcafe5000,0xbeef6000,0xabcd7000,0xdcba8000]))
    for big,width,address,stack,reg,rm in itertools.product((0,1),(2,4),(2,4),(0,1),range(8),range(8)):
     default=4 if big else 2;prefix=(b'\x66' if width!=default else b'')+(b'\x67' if address!=default else b'')
     corpus.append(packet(prefix+bytes([0x3b,0xc0|(reg<<3)|rm]),big,stack,regs=[0x12340000,0x89abffff,0x76548000,0xfeed7fff,0xcafe0001,0xbeeffffe,0xabcd8001,0xdcba7ffe]))
    for big,width,mode,sib in itertools.product((0,1),(2,4),range(3),range(256)):
     default=4 if big else 2;code=(b'\x66' if width!=default else b'')+(b'\x67' if default!=4 else b'')+bytes([0x3b,(mode<<6)|((len(corpus)%8)<<3)|4,sib])
     if mode==0 and (sib&7)==5:code+=struct.pack('<I',0x15ba)
     elif mode==1:code+=b'\x80'
     elif mode==2:code+=struct.pack('<i',-0x100)
     corpus.append(packet(code,big))
    for lazy,width,a,reverse in itertools.product(range(65),(2,4),(0,1,0x7fff,0x8000,0xffff,0x7fffffff,0x80000000,0xffffffff),(0,1)):
     regs=[0x12345678,0x87654321,0xa000,0x4000,0x5000,0x6000,0x7000,0x8000]
     regs[reverse]=a;regs[1-reverse]=1
     corpus.append(packet((b'\x66' if width==4 else b'')+b'\x3b\xc1',regs=regs,lazy=lazy))
    assert len(corpus)==9760,len(corpus)
    return corpus

def negative(repo,root,original_command,commands):
    header=(repo/'re_out/fist_exec.h').read_text();folder=root/'causal';folder.mkdir()
    original=[original_command]
    def packet(code,regs=None):
     if regs is None:regs=[0xfedc8000,0x12340001,0xa000,0x4000,0x5000,0x6000,0x7000,0x8000]
     q=[0,0,0x2f3b,0xfedcba98,*regs,1,17,1,len(code)]
     segments=[v for selector in (0x10,0x2082,0x26e,0x2d19,0x4000,0x6000) for v in (selector,selector<<4)]
     return struct.pack('<28I',*q,*segments)+code
    branch='else if(op==0x3b)fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w));'
    assert header.count(branch)==1
    mutations=[
     ('reversed-operands',header.replace(branch,'else if(op==0x3b)fist_exec_alu(e,7,w,fist_exec_read_op(e,q,w),fist_exec_reg_read(e,i,w));'),packet(b'\x3b\xc1')),
     ('register-writeback',header.replace(branch,'else if(op==0x3b)fist_exec_reg_write(e,i,w,fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w)));'),packet(b'\x3b\xc1')),
     ('eager-result-flags',header.replace(branch,'else if(op==0x3b){fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w));fist_cpu_fill_flags(e->bus->cpu);}'),packet(b'\x3b\xc1')),
     ('word-only-operand',header.replace(branch,branch.replace('7,w,','7,2,').replace('read(e,i,w)','read(e,i,2)').replace('read_op(e,q,w)','read_op(e,q,2)')),packet(b'\x66\x3b\xc1')),
     ('cleared-dirty-lazy-upper',header.replace(branch,'else if(op==0x3b){e->bus->cpu->flags.var1=0;e->bus->cpu->flags.var2=0;e->bus->cpu->flags.res=0;fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w));}'),packet(b'\x3b\xc1')),
     ('bp-uses-ds',operand_mutation(header,'q.seg=2','q.seg=3'),packet(b'\x3b\x46\x00')),
     ('cmp-writes-ram',header.replace(branch,'else if(op==0x3b){uint32_t v=fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w));fist_exec_write_op(e,q,w,v);}'),packet(b'\x3b\x06\xba\x15')),
    ]
    def run(command,data):
     p=subprocess.run(command,input=data,capture_output=True,timeout=30)
     assert p.returncode==0,p.stderr.decode();parse(p.stdout);return p.stdout
    results=[]
    for name,mutant,data in mutations:
     assert mutant!=header,name
     expected=run(original,data);a,sa,ma,fa,ra=parse(expected)
     for target,command in commands:assert run(command,data)==expected,(name,target,'unmodified full positive')
     for target,command in target_build(repo,folder/name,mutant,segment_input=True,group_shift=True,ram_trace=True):
      actual=run(command,data);b,sb,mb,fb,rb=parse(actual)
      assert actual!=expected and sa==sb,(name,target)
      assert (ma==mb)==(name!='cmp-writes-ram'),(name,target,'RAM contract')
      assert fa==fb,(name,target,'code fetch contract')
      fields=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y]
      if name=='register-writeback':assert fields==[0] and ra==rb,(name,target,fields)
      if name=='cleared-dirty-lazy-upper':assert fields==[10,11,12] and ra==rb,(name,target,fields)
      results.append(dict(fault=name,target=target,unmodified_complete_positive_equal=True,differing_CPU_cache_words=fields,all2MiB_RAM_equal=ma==mb,code_fetches_equal=fa==fb,ordered_RAM_accesses_equal=ra==rb,input_sha256=hashlib.sha256(data).hexdigest(),original_sha256=hashlib.sha256(expected).hexdigest(),mutant_sha256=hashlib.sha256(actual).hexdigest()))
     print('PASS CMP distinguishes',name,'on both targets',flush=True)
    return results

def capture(repo,root):
    assert root.is_relative_to(Path('/tmp'))
    root.mkdir(parents=True,exist_ok=False)
    resource.setrlimit(resource.RLIMIT_CORE,(0,resource.getrlimit(resource.RLIMIT_CORE)[1]))
    source=root/'source';source.mkdir()
    original=original_build(repo,source,extra_opcodes=(0x3b,),trace=True,lazy_input=True,segment_input=True,group_shift=True,ram_trace=True)
    commands=target_build(repo,root/'targets',segment_input=True,group_shift=True,ram_trace=True)
    corpus=cases()
    hashes={name:hashlib.sha256() for name in ('original','native','wasm')}
    for start in range(0,len(corpus),16):
     data=b''.join(corpus[start:start+16]);expected=None
     for target,command in [('original',[original]),*commands]:
      p=subprocess.run(command,input=data,capture_output=True,timeout=45)
      assert p.returncode==0,(target,start,p.stderr.decode())
      assert len(p.stdout)==min(16,len(corpus)-start)*RECORD_SIZE,(target,start,'incomplete')
      if expected is None:expected=p.stdout
      else:assert p.stdout==expected,(target,start,next(i for i,(a,b) in enumerate(zip(p.stdout,expected)) if a!=b))
      hashes[target].update(p.stdout)
     if start%512==0:print('PASS complete original CMP',min(start+16,len(corpus)),'/',len(corpus),flush=True)
    negatives=negative(repo,root,original,commands)
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    paths=[Path(__file__),repo/'tools/oracle/cpu_execute_probe.py',repo/'tools/oracle/cpu_execute_probe.cpp',repo/'tools/oracle/capture_cpu_byte_instructions.py',repo/'tools/oracle/capture_cpu_shr_instructions.py',repo/'tools/oracle/rep_probe.cpp',repo/'tests/cpu_execute_controlled.c',root/'targets/controlled.c',*sorted((repo/'re_out').glob('*.h')),*source.glob('*.h'),*tree.rglob('*.h'),tree/'src/cpu/flags.cpp',tree/'src/cpu/cpu.cpp',tree/'src/cpu/modrm.cpp',tree/'src/cpu/core_normal.cpp']
    proof=dict(scope='Actual original CMP CASE_W/D 3b and RMGwEw/RMGdEd/CMPW/CMPD owners. All31 CPU/cache words,three64-bit stack words,all2MiB RAM,every ordered code and physical RAM read match both release targets. Both code/operand/address/stack sizes,all segment overrides,all direct register pairs,all original SIB forms,dirty lazy upper words and65 incoming lazy types are covered. Seven causal faults fail on both targets after full unmodified positives. Protected/paging/provider faults,reaching/runtime/frame/audio acceptance remain separate.',cases=len(corpus),record_size=RECORD_SIZE,outputs_sha256={name:h.hexdigest() for name,h in hashes.items()},corpus_sha256=hashlib.sha256(b''.join(corpus)).hexdigest(),causal_negatives=negatives,inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS all9760 complete original CMP programs/14 causal negatives',flush=True)
    return proof

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--repo',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();capture(a.repo.resolve(),a.output.resolve())
