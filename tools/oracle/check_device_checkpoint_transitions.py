#!/usr/bin/env python3
"""Check all original checkpoint instructions against actual original ALU/flag producers."""
from pathlib import Path
import copy,json,re,struct,subprocess,sys
from verify_device_start_prefix import digest
from verify_device_checkpoint import verify,physical_owner
REGISTERS=('eax','ecx','edx','ebx','esp','ebp','esi','edi')
FLAGS=('cpu_regs.flags','lflags.type','lflags.prev_type','lflags.oldcf','lflags.var1.dword[0]','lflags.var2.dword[0]','lflags.res.dword[0]')

def check(root,repo):
 verify(root,repo)
 physical=physical_owner(repo)
 rows=[json.loads(l) for l in (root/'source/prefix-fetches.jsonl').read_text().splitlines()]
 assert len(rows)==368
 memory=bytearray((root/'source/7809.memory').read_bytes())
 image=(repo/'re_out/fist_image.bin').read_bytes()
 instructions={}
 for q in rows[241:-1]:
  ip=q['cpu_regs.ip.dword[0]']
  if ip in instructions:continue
  dump=subprocess.check_output(['objdump','-D','-b','binary','-m','i386','-M','intel',f'--start-address={ip}',f'--stop-address={ip+16}',str(repo/'re_out/fist_image.bin')],text=True)
  line=next(s for s in dump.splitlines() if re.match(r'\s*%x:'%ip,s))
  m=re.fullmatch(r'\s*([0-9a-f]+):\s*((?:[0-9a-f]{2}\s+)+)\s*([a-z]+)\s*(.*?)\s*',line)
  assert m,line
  code=bytes.fromhex(m[2]);assert image[ip:ip+len(code)]==code
  instructions[ip]=(m[3],m[4],len(code))
 expected=copy.deepcopy(rows[241]);operations=[];writes=[]
 def address(segment,offset):return physical((expected['segments'][segment]['base']+offset)&0xffffffff,memory,expected)
 def read(segment,offset):return struct.unpack_from('<I',memory,address(segment,offset))[0]
 def write(segment,offset,value):
  p=address(segment,offset);struct.pack_into('<I',memory,p,value&0xffffffff)
  writes.append(dict(ip=expected['cpu_regs.ip.dword[0]'],segment=segment,offset=offset,physical=p,value=value&0xffffffff))
 def location(operand):
  operand=operand.removeprefix('DWORD PTR ')
  if operand.startswith('ds:'):return (3,int(operand[3:],16))
  assert operand.startswith('[') and operand.endswith(']'),operand
  terms=operand[1:-1].split('+');segment=2 if any(t.split('*')[0] in ('esp','ebp') for t in terms) else 3
  offset=0
  for term in terms:
   if term.startswith('0x'):offset+=int(term,16)
   else:
    reg,*scale=term.split('*');assert reg in REGISTERS,operand
    offset+=expected['registers'][REGISTERS.index(reg)]*(int(scale[0]) if scale else 1)
  return segment,offset&0xffffffff
 def get(operand):
  if operand in REGISTERS:return expected['registers'][REGISTERS.index(operand)]
  if operand.startswith('0x'):return int(operand,16)
  return read(*location(operand))
 def put(operand,value):
  if operand in REGISTERS:expected['registers'][REGISTERS.index(operand)]=value&0xffffffff
  else:write(*location(operand),value)
 def push(value):
  r=expected['registers'];r[4]=(r[4]&expected['cpu.stack.notmask'])|((r[4]-4)&expected['cpu.stack.mask']);write(2,r[4]&expected['cpu.stack.mask'],value)
 def pop():
  r=expected['registers'];value=read(2,r[4]&expected['cpu.stack.mask']);r[4]=(r[4]&expected['cpu.stack.notmask'])|((r[4]+4)&expected['cpu.stack.mask']);return value
 def original(operation,a=0,b=0):
  before=[expected[k] for k in FLAGS]
  text=operation+' '+' '.join('%x'%v for v in [*before,a,b])+'\n'
  p=subprocess.run([str(root/'instructions-probe')],input=text,capture_output=True,text=True,check=True)
  out=[int(v,16) for v in p.stdout.split()];assert len(out)==10
  for k,v in zip(FLAGS,out[:7]):expected[k]=v
  operations.append(dict(ip=expected['cpu_regs.ip.dword[0]'],operation=operation,input_flags=before,a=a,b=b,output=out))
  return out[7:]
 for i,q in enumerate(rows[241:-1],241):
  assert q==expected,(i,'before',q,expected)
  ip=expected['cpu_regs.ip.dword[0]'];op,operands,width=instructions[ip];args=operands.split(',') if operands else []
  next_ip=ip+width
  if op=='mov':put(args[0],get(args[1]))
  elif op=='call':push(next_ip);next_ip=get(args[0])
  elif op=='ret':next_ip=pop()
  elif op=='push':push(get(args[0]))
  elif op=='pop':put(args[0],pop())
  elif op=='cld':expected['cpu_regs.flags']&=~0x400
  elif op=='loop':
   put('ecx',get('ecx')-1)
   if get('ecx'):next_ip=get(args[0])
  elif op in ('xor','or','cmp','add','sub','inc','dec'):
   operation={'xor':'X','or':'O','cmp':'C','add':'A','sub':'S','inc':'I','dec':'D'}[op]
   value,cf,zf=original(operation,get(args[0]),get(args[1]) if len(args)==2 else 0)
   if op!='cmp':put(args[0],value)
  elif op in ('je','jne','jbe'):
   value,cf,zf=original('N')
   if {'je':bool(zf),'jne':not zf,'jbe':bool(cf or zf)}[op]:next_ip=get(args[0])
  else:raise AssertionError((ip,op,operands))
  expected['CPU_Cycles']-=1;expected['cpu_regs.ip.dword[0]']=next_ip
  p=address(1,next_ip);expected['fetched_code_physical']=p;expected['fetched_code_hex']=memory[p:p+32].hex()
  assert expected==rows[i+1],(i,hex(ip),op,{k:(expected[k],rows[i+1][k]) for k in expected if expected[k]!=rows[i+1][k]})
  label='%04x'%next_ip;boundary=root/'source'/(label+'.memory')
  if boundary.exists():assert memory==boundary.read_bytes(),label
 assert memory==(root/'source/780e.memory').read_bytes()
 proof=dict(scope='Original-source transition validation: every one of126 original checkpoint-free transitions has exact full CPU/code/time/address fields and complete RAM. Original instructions.h/flags.cpp, not port flag formulas, calculate every ALU/condition. No port implementation acceptance.',transitions=126,unique_instruction_addresses=len(instructions),instructions={hex(ip):dict(mnemonic=v[0],operands=v[1],width=v[2]) for ip,v in instructions.items()},original_ALU_and_condition_calls=operations,guest_stores=writes,producers={str(p):digest(p) for p in (repo/'tools/oracle/cpu_instructions_probe.cpp',root/'instructions-probe',repo/'tools/oracle/verify_device_checkpoint.py',Path(__file__),repo/'third_party/dosbox-build/dosbox-0.74-3/src/cpu/flags.cpp',repo/'third_party/dosbox-build/dosbox-0.74-3/src/cpu/instructions.h',Path('/usr/bin/objdump'))},complete_original_acceptance=False)
 (root/'transitions-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 print('PASS:all126 complete CPU transitions, original ALU/condition owner and every guest store match')
 return proof

if __name__=='__main__':
 import argparse
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--repo',type=Path,required=True)
 parser.add_argument('--source',type=Path,required=True)
 args=parser.parse_args();check(args.source.resolve(strict=True),args.repo.resolve(strict=True))
