#!/usr/bin/env python3
"""Compare original CMPB 3a decoding, lazy flags and every RAM access."""
import argparse,hashlib,itertools,json,resource,struct,subprocess
from pathlib import Path
from cpu_execute_probe import build as original_build
from capture_cpu_byte_instructions import target_build,digest
from capture_cpu_cmp import cases as word_cases,segments
from capture_cpu_shr_instructions import RECORD_SIZE,parse,operand_mutation

def cases():
 corpus=[]
 for data in word_cases():
  code=bytearray(data[112:]);index=0
  while code[index] in (0x66,0x67,0x26,0x2e,0x36,0x3e,0x64,0x65):index+=1
  assert code[index]==0x3b;code[index]=0x3a;corpus.append(data[:112]+code)
 for lazy,value,reverse,prefix in itertools.product(range(65),(0,1,0x7f,0x80,0xff,0x100,0xdeadbe80,0x123456ff),(0,1),(b'',b'\x66')):
  regs=[0x12345678,0x87654321,0xa000,0x4000,0x5000,0x6000,0x7000,0x8000];regs[reverse]=value;regs[1-reverse]=1
  code=prefix+b'\x3a\xc1';q=[0,0,0x2f55,0xfedcba98,*regs,1,17,lazy,len(code)]
  corpus.append(struct.pack('<28I',*q,*segments)+code)
 assert len(corpus)==11840
 return corpus

def packet(code,regs=None):
 if regs is None:regs=[0xfedcba80,0x12340001,0xa000,0x4000,0x5000,0x6000,0x7000,0x8000]
 return struct.pack('<28I',0,0,0x2f55,0xfedcba98,*regs,1,17,1,len(code),*segments)+code

def negative(repo,work,original,commands,header):
 branch='else if(op==0x3b||op==0x3a)fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w));';assert header.count(branch)==1
 width='w=(op==0x32||op==0x3a)?1:width';assert header.count(width)==1
 mutations=[
  ('reversed-operands',header.replace(branch,'else if(op==0x3b||op==0x3a)fist_exec_alu(e,7,w,fist_exec_read_op(e,q,w),fist_exec_reg_read(e,i,w));'),packet(b'\x3a\xc1')),
  ('register-writeback',header.replace(branch,'else if(op==0x3b||op==0x3a)fist_exec_reg_write(e,i,w,fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w)));'),packet(b'\x3a\xc1')),
  ('eager-flags',header.replace(branch,'else if(op==0x3b||op==0x3a){fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w));fist_cpu_fill_flags(e->bus->cpu);}'),packet(b'\x3a\xc1')),
  ('widened-byte',header.replace(width,'w=op==0x32?1:width'),packet(b'\x66\x3a\xc1')),
  ('cleared-lazy-upper',header.replace(branch,'else if(op==0x3b||op==0x3a){e->bus->cpu->flags.var1=0;e->bus->cpu->flags.var2=0;e->bus->cpu->flags.res=0;fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w));}'),packet(b'\x3a\xc1')),
  ('bp-uses-ds',operand_mutation(header,'q.seg=2','q.seg=3'),packet(b'\x3a\x46\x00')),
  ('writes-ram',header.replace(branch,'else if(op==0x3b||op==0x3a){uint32_t v=fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w));fist_exec_write_op(e,q,w,v);}'),packet(b'\x3a\x06\xb4\x15')),
  ('wrong-high-byte-alias',header.replace('if(w==1 && i>=4)return (*fist_exec_reg(e,i-4)>>8)&255;','if(w==1 && i>=4)return *fist_exec_reg(e,i)&255;'),packet(b'\x3a\xc4')),
 ]
 def run(command,data):
  p=subprocess.run(command,input=data,capture_output=True,timeout=30);assert p.returncode==0,p.stderr.decode();parse(p.stdout);return p.stdout
 results=[]
 for name,mutant,data in mutations:
  assert mutant!=header,name;expected=run([original],data);a,sa,ma,fa,ra=parse(expected)
  for target,command in commands:assert run(command,data)==expected,(name,target,'full unmodified positive')
  for target,command in target_build(repo,work/'causal'/name,mutant,segment_input=True,group_shift=True,ram_trace=True):
   actual=run(command,data);b,sb,mb,fb,rb=parse(actual);assert actual!=expected and sa==sb,(name,target)
   assert (ma==mb)==(name!='writes-ram'),(name,target,'full RAM')
   assert fa==fb,(name,target,'ordered code fetches')
   fields=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y]
   if name=='register-writeback':assert fields==[0] and ra==rb,(name,target,fields)
   if name=='cleared-lazy-upper':assert fields==[10,11,12] and ra==rb,(name,target,fields)
   results.append(dict(fault=name,target=target,unmodified_complete_positive_equal=True,differing_CPU_cache_words=fields,all2MiB_RAM_equal=ma==mb,ordered_code_fetches_equal=fa==fb,ordered_RAM_accesses_equal=ra==rb,input_sha256=hashlib.sha256(data).hexdigest(),original_sha256=hashlib.sha256(expected).hexdigest(),mutant_sha256=hashlib.sha256(actual).hexdigest()))
  print('PASS CMPB distinguishes',name,'on both targets',flush=True)
 return results

def capture(repo,work):
 assert work.is_relative_to(Path("/tmp"))
 resource.setrlimit(resource.RLIMIT_CORE,(0,resource.getrlimit(resource.RLIMIT_CORE)[1]));work.mkdir(parents=True,exist_ok=False);source=work/'source';source.mkdir()
 header=(repo/'re_out/fist_exec.h').read_text()
 original=original_build(repo,source,byte_opcodes=(0x3a,),trace=True,lazy_input=True,segment_input=True,group_shift=True,ram_trace=True)
 commands=target_build(repo,work/'targets',header,segment_input=True,group_shift=True,ram_trace=True)
 corpus=cases();hashes={name:hashlib.sha256() for name in ('original','native','wasm')};batches=32
 for start in range(0,len(corpus),batches):
  data=b''.join(corpus[start:start+batches]);expected=None
  for target,command in [('original',[original]),*commands]:
   p=subprocess.run(command,input=data,capture_output=True,timeout=45);assert p.returncode==0,(target,start,p.stderr.decode())
   assert len(p.stdout)==min(batches,len(corpus)-start)*RECORD_SIZE,(target,start,'incomplete')
   if expected is None:expected=p.stdout
   else:assert p.stdout==expected,(target,start,next(i for i,(a,b) in enumerate(zip(p.stdout,expected)) if a!=b))
   hashes[target].update(p.stdout)
  if start%1024==0:print('PASS complete original CMPB',min(start+batches,len(corpus)),'/',len(corpus),flush=True)
 negatives=negative(repo,work,original,commands,header)
 tree=repo/'third_party/dosbox-build/dosbox-0.74-3';paths=[Path(__file__),repo/'re_out/fist_exec.h',repo/'tools/oracle/capture_cpu_cmp.py',repo/'tools/oracle/cpu_execute_probe.py',repo/'tools/oracle/cpu_execute_probe.cpp',repo/'tools/oracle/capture_cpu_byte_instructions.py',repo/'tools/oracle/capture_cpu_shr_instructions.py',repo/'tools/oracle/rep_probe.cpp',repo/'tests/cpu_execute_controlled.c',work/'targets/controlled.c',*sorted((repo/'re_out').glob('*.h')),*source.glob('*.h'),*tree.rglob('*.h'),tree/'src/cpu/flags.cpp',tree/'src/cpu/cpu.cpp',tree/'src/cpu/modrm.cpp',tree/'src/cpu/core_normal.cpp']
 proof=dict(scope='Verbatim original CASE_B3a/RMGbEb/CMPB controlled program proof. All31 CPU/cache words,three64-bit stack words,all2MiB RAM,every ordered code and physical RAM access equal both release targets. Code/address/stack sizes,operand prefix ignored for byte width,segment overrides,all direct byte register pairs/high aliases and SIB forms,dirty lazy upper words/65 incoming types and signed BYTE boundaries covered. Eight causes rejected on both targets. Protected/page-fault/full startup/device/frame/audio acceptance remains separate.',cases=len(corpus),record_size=RECORD_SIZE,outputs_sha256={name:h.hexdigest() for name,h in hashes.items()},corpus_sha256=hashlib.sha256(b''.join(corpus)).hexdigest(),causal_negatives=negatives,inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
 (work/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS all11840 original CMPB programs/16 causal negatives',flush=True)
 return proof

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--repo',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
 a=p.parse_args();capture(a.repo.resolve(),a.output.resolve())
