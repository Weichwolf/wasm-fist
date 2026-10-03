#!/usr/bin/env python3
"""Verify successful original near/far returns and independent WORD inputs."""
import argparse,ast,hashlib,json,re,struct,sys
from pathlib import Path
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def verify(root,repo):
 sys.path.insert(0,str(repo/'tools/oracle'))
 from sequence_format import validate,validate_endpoint
 producers=json.loads((root/'producers.json').read_text())
 for path,sha in producers.items():assert digest(path)==sha,path
 for name in ('baseline','return'):
  assert (root/(name+'.exit')).read_text()=='0\n',name
  prefix=root/name/'sequence';assert validate_endpoint(prefix,600)==600
  assert validate(str(prefix)+'.frames','F')['records']==39
  assert validate(str(prefix)+'.pcm','A')['samples']==27518
 for suffix in ('frames','pcm','end'):assert (root/'baseline'/('sequence.'+suffix)).read_bytes()==(root/'return'/('sequence.'+suffix)).read_bytes(),suffix
 reference=json.loads((repo/'tools/oracle/detail_return_case.json').read_text())
 fetches=[json.loads(row) for row in (root/'return/return-fetches.jsonl').read_text().splitlines()]
 assert len(fetches)==len(reference['handler_to_op68'])==89
 def clock(q):return q['PIC_Ticks']*q['CPU_CycleMax']+q['CPU_CycleMax']-q['CPU_Cycles']-q['CPU_CycleLeft']
 for q,r in zip(fetches,reference['handler_to_op68']):
  assert q['cpu_regs.ip.dword[0]']==r['ip'] and q['segments'][1]['value']==r['cs']
  assert q['registers']==r['registers']
  assert [[s['value'],s['base']] for s in q['segments']]==r['segments']
  assert clock(q)==r['cycle']
 program=(repo/'tools/oracle/file_error.gdb').read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
 nodes=[n for n in ast.parse(program).body if isinstance(n,ast.FunctionDef) and n.name=='physical'];assert len(nodes)==1
 namespace={'struct':struct};exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),'shared paging owner','exec'),namespace);physical=namespace['physical']
 folder=root/'return';states={p.stem:json.loads(p.read_text()) for p in folder.glob('*.json')};assert len(states)==25
 ram={label:(folder/(label+'.memory')).read_bytes() for label in states};assert all(len(m)==16777216 for m in ram.values())
 kernel=(repo/'re_out/fist_image.bin').read_bytes();engine=(repo/'re_out/fist_dat_image.bin').read_bytes()
 kernel_cs=fetches[0]['segments'][1]['value'];engine_cs=fetches[-1]['segments'][1]['value']
 lengths={0x76fc:1,0x10df:1,0xf57:6,0xf5d:1,0xbf04:7,0xe34c:3,0xe34f:1,0xe350:1,0xe351:4,0xe355:4,0xe359:4,0xe35d:2,0xe36a:1,0xe36b:1,0xe36c:1,0xdea5:1,0x6dfd:3,0x6e00:2,0x6e02:3,0xc008:3,0xc00b:1,0x6e05:3,0x6e08:2,0x6e0a:3,0xe2df:2}
 for label,q in states.items():
  ip=q['cpu_regs.ip.dword[0]'];code=physical((q['segments'][1]['base']+ip)&0xffffffff,ram[label],q)
  matching=[f for f in fetches if f['segments'][1]['value']==q['segments'][1]['value'] and f['cpu_regs.ip.dword[0]']==ip]
  assert matching==[q],(label,'complete boundary/fetch state')
  image=kernel if q['segments'][1]['value']==kernel_cs else engine
  assert code==q['fetched_code_physical'] and ram[label][code:code+32].hex()==q['fetched_code_hex']
  assert ram[label][code:code+lengths[ip]]==image[ip:ip+lengths[ip]],(label,'complete fetched instruction')
 def address(label,seg,off):return physical((states[label]['segments'][seg]['base']+off)&0xffffffff,ram[label],states[label])
 def word(label,seg,off):return struct.unpack_from('<H',ram[label],address(label,seg,off))[0]
 def transition(a,b,regs,writes=(),segments=None,flags=None):
  qa,qb=states[a],states[b];assert qb['registers']==regs,(a,b,'GP')
  expected=bytearray(ram[a])
  for addr,value,width in writes:struct.pack_into('<H' if width==2 else '<I',expected,addr,value)
  assert ram[b]==expected,(a,b,'whole RAM')
  assert qb['segments']==(qa['segments'] if segments is None else segments),(a,b,'segments')
  fixed=[key for key in qa if key.startswith(('cpu.','paging.'))]+['PIC_Ticks','CPU_CycleMax','CPU_CycleLeft']
  for key in fixed:assert qa[key]==qb[key],(a,b,key)
  actual_flags={key:value for key,value in qa.items() if key=='cpu_regs.flags' or key.startswith('lflags.')}
  if flags:actual_flags.update(flags)
  for key,value in actual_flags.items():assert qb[key]==value,(a,b,key)
  assert clock(qb)-clock(qa)==1,(a,b,'one instruction')
  checks.append([a,b])
 checks=[]
 def near_return(a,b,width):
  q=states[a];regs=q['registers'].copy();sp=regs[4];value=struct.unpack_from('<H' if width==2 else '<I',ram[a],address(a,2,sp))[0]
  assert value==states[b]['cpu_regs.ip.dword[0]'],(a,'near return frame')
  regs[4]=(sp+width)&0xffffffff;transition(a,b,regs)
 near_return('body-return','handler-return',4);near_return('handler-return','restore-service-stack',4)
 q=states['restore-service-stack'];regs=q['registers'].copy();regs[4]=struct.unpack_from('<I',ram['restore-service-stack'],address('restore-service-stack',3,0xf60))[0]
 transition('restore-service-stack','far-return',regs)
 q=states['far-return'];sp=q['registers'][4];frame=ram['far-return'][address('far-return',2,sp):address('far-return',2,sp)+8];ip,cs=struct.unpack('<II',frame)
 assert ip==states['far-caller-return']['cpu_regs.ip.dword[0]'] and cs&65535==states['far-caller-return']['segments'][1]['value']
 regs=q['registers'].copy();regs[4]+=8;transition('far-return','far-caller-return',regs)
 lazy=(repo/'third_party/dosbox-build/dosbox-0.74-3/src/cpu/lazyflags.h').read_text().split('//Types of Flag changing instructions',1)[1].split('enum {',1)[1].split('};',1)[0]
 types={name:index for index,name in enumerate(re.findall(r'\bt_[A-Za-z0-9_]+\b',lazy))}
 def arithmetic(a,name,v1,v2,res):
  q=states[a];changes={'lflags.type':types[name]}
  for key,value in [('var1',v1),('var2',v2),('res',res)]:changes['lflags.'+key+'.dword[0]']=(q['lflags.'+key+'.dword[0]']&0xffff0000)|(value&65535)
  return changes
 q=states['gate-return'];regs=q['registers'].copy();sp=regs[4];regs[4]=(sp&0xffff0000)|((sp+10)&65535)
 transition('gate-return','save-eax',regs,flags=arithmetic('gate-return','t_ADDw',sp,10,sp+10))
 for a,b,register in [('save-eax','save-esi',0),('save-esi','task-segment-load',6)]:
  q=states[a];regs=q['registers'].copy();regs[4]=(regs[4]&0xffff0000)|((regs[4]-2)&65535)
  transition(a,b,regs,[(address(a,2,regs[4]),regs[register]&65535,2)])
 q=states['task-segment-load'];segments=[s.copy() for s in q['segments']];gs=word('task-segment-load',3,0xea2e);segments[5]={'value':gs,'base':gs<<4}
 transition('task-segment-load','task-offset-load',q['registers'],segments=segments)
 q=states['task-offset-load'];regs=q['registers'].copy();regs[6]=(regs[6]&0xffff0000)|word('task-offset-load',3,0xea2c)
 transition('task-offset-load','task-test',regs)
 q=states['task-test'];status=word('task-test',5,q['registers'][6]&65535);assert status==0
 transition('task-test','normal-branch',q['registers'],flags=arithmetic('task-test','t_CMPw',status,0,status))
 transition('normal-branch','restore-esi',states['normal-branch']['registers'])
 for a,b,register in [('restore-esi','restore-eax',6),('restore-eax','gate-near-return',0)]:
  q=states[a];regs=q['registers'].copy();regs[register]=(regs[register]&0xffff0000)|word(a,2,regs[4]&65535);regs[4]=(regs[4]&0xffff0000)|((regs[4]+2)&65535)
  transition(a,b,regs)
 near_return('gate-near-return','de89-near-return',2);near_return('de89-near-return','first-config-load',2)
 for a,b,offset in [('first-config-load','first-config-shift',0x8b49),('second-config-load','second-config-shift',0x8b4b)]:
  q=states[a];regs=q['registers'].copy();regs[0]=(regs[0]&0xffff0000)|word(a,3,offset);transition(a,b,regs)
 for a,b in [('first-config-shift','first-config-call'),('second-config-shift','second-config-call')]:
  q=states[a];regs=q['registers'].copy();value=regs[0]&65535;regs[0]=(regs[0]&0xffff0000)|(value>>1)
  transition(a,b,regs,flags=arithmetic(a,'t_SHRw',value,1,value>>1))
 for a,b,return_ip in [('first-config-call','config-store',0x6e05),('second-config-call','op68-entry',0x6e0d)]:
  q=states[a];regs=q['registers'].copy();regs[4]=(regs[4]&0xffff0000)|((regs[4]-2)&65535)
  transition(a,b,regs,[(address(a,2,regs[4]),return_ip,2)])
 q=states['config-store'];destination=struct.unpack_from('<H',engine,0xc009)[0]
 transition('config-store','config-return',q['registers'],[(address('config-store',3,destination),q['registers'][0]&65535,2)])
 near_return('config-return','second-config-load',2)
 assert len(checks)==23
 assert all(q['registers'][3]==fetches[0]['registers'][3] for q in fetches)
 proof=dict(scope='Source-only successful detail return: all89 GP/segment/clock fetches agree with the existing original trace. Twenty-five complete raw/lazy flags, control, time and16MiB boundaries verify23 actual instruction transitions: DWORD near RETs, saved service ESP,8-byte far return, WORD engine stack saves/restores and near returns, current task status branch, independent configuration WORD loads/shifts/store/calls. No port/production ABI, IRQ/device-time or complete original acceptance.',
  states=states,memory_sha256={n:digest(folder/(n+'.memory')) for n in states},whole_RAM_transitions=checks,fetches=fetches,far_return_frame=frame.hex(),returned_EAX=fetches[0]['registers'][0],returned_EBX=fetches[0]['registers'][3],config_words=[word('first-config-load',3,0x8b49),word('second-config-load',3,0x8b4b)],op68_input_EAX=fetches[-1]['registers'][0],frames=39,mixed_samples=27518,endpoint_ms=600,capture_sha256={s:digest(root/'baseline'/('sequence.'+s)) for s in ('frames','pcm','end')},producers=producers,verifier_sha256=digest(__file__),complete_original_acceptance=False)
 (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 print('PASS: original89-fetch normal return,25 full state/RAM boundaries,23 instruction/whole RAM transitions; complete39 frames/27518 mixed samples unchanged.')
 return proof
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args();verify(args.output.resolve(strict=True),args.repo.resolve(strict=True))
if __name__=='__main__':main()
