#!/usr/bin/env python3
"""Observe the actual op6c task gate, optionally through reached DOS callbacks."""
from pathlib import Path
import argparse,ast,hashlib,json,os,sys
# Use the same repository-owned CPU/provider/clock observation modules.
sys.path.insert(0,str(Path(__file__).resolve().parent))
from capture_cpu_core_exit import capture,observer as shared_observer
from memory_context import validate_context
from sequence_format import validate,validate_endpoint
from cpu_trace import records
def digest(p):
 with Path(p).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def trace_key(q):
 cycle=q['PIC_Ticks']*q['CPU_CycleMax']+q['CPU_CycleMax']-q['CPU_Cycles']-q['CPU_CycleLeft']
 return cycle,(q['segments'][1]['value'],q['cpu_regs.ip.dword[0]']),tuple(q['registers']),tuple((s['value'],s['base']) for s in q['segments'])

def observer(repo,through_dos=False):
 base=shared_observer(repo);prefix=base[:base.index('class QueueReturned(')]
 script=prefix+('through_dos=%r\n'%through_dos)+'''
import sys
sys.path.insert(0,str(repo/'tools/oracle'))
from resident_image import load,relocate
resident_model=load(repo)
resident=None
dat=(repo/'re_out/fist_dat_image.bin').read_bytes()
ext=(repo/'re_out/fist_image.bin').read_bytes()
gdb.execute('set environment FIST_CPU_TRACE='+str(root.parent/'cpu.trace'))
gdb.execute('set environment FIST_CPU_TRACE_WINDOW=500:536')
watches=[];started=False;find_active=False;find_records=[]
def host_state():
 drive=int(gdb.parse_and_eval('dos.current_drive'));cache='Drives[%d]->dirCache'%drive
 pointer_size=gdb.lookup_type('void').pointer().sizeof
 address=int(gdb.parse_and_eval('&'+cache+'.dirFindFirst[0]'))
 size=int(gdb.parse_and_eval('sizeof('+cache+'.dirFindFirst)'))
 raw=bytes(gdb.selected_inferior().read_memory(address,size))
 return dict(drive=drive,directory=gdb.parse_and_eval('Drives[%d]->curdir'%drive).string(),next_free=int(gdb.parse_and_eval(cache+'.nextFreeFindFirst')),occupied=[bool(int.from_bytes(raw[i:i+pointer_size],'little')) for i in range(0,size,pointer_size)])
def find_record(kind,arguments=None):
 q=state();q.update(kind=kind,pic=pic_state(),calendar=calendar(),HOST=host_state())
 if arguments is not None:q['arguments']=arguments
 label='find-'+kind+'.memory';q['memory_file']=label;q['memory_context']=memory_context(label)
 (root/label).write_bytes(memory())
 with (root/'find-events.jsonl').open('a') as f:f.write(json.dumps(q)+'\\n')
 find_records.append(kind)
 return q
def locate(m,code):
 out=[];at=m.find(code)
 while at!=-1:out.append(at);at=m.find(code,at+1)
 assert out and len(out)<=4,(code.hex(),out)
 return out
def arm_reads(m,ip,image,label):
 base=int(gdb.parse_and_eval('MemBase'))
 size=7 if ip==0xf57 else 32
 for p in locate(m,image[ip:ip+size]):watches.append(CodeRead(base+p,ip,image,label,size))
class CodeRead(gdb.Breakpoint):
 def __init__(self,address,ip,image,label,size):
  self.address=address;self.ip=ip;self.image=image;self.label=label;self.size=size
  super().__init__('*(unsigned char *)(%d)'%address,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True)
 def stop(self):
  global started,resident,find_active
  q=state()
  if q['cpu_regs.ip.dword[0]']!=self.ip:return False
  m=memory();p=physical((q['segments'][1]['base']+self.ip)&0xffffffff,m,q)
  if int(gdb.parse_and_eval('MemBase'))+p!=self.address:return False
  assert m[p:p+self.size]==self.image[self.ip:self.ip+self.size]
  if self.label=='engine-e339':
   dg=physical((q['segments'][3]['base']+0xea10)&0xffffffff,m,q)
   if struct.unpack_from('<H',m,dg)[0]!=0x6c:return False
   started=True
   for w in watches:w.enabled=False
  elif not started:return False
  self.enabled=False
  sb=gdb.parse_and_eval("'sblaster.cpp'::sb")
  scalar_fields=('freq','speaker','midi','time_constant','mode','type',
   'dma.stereo','dma.sign','dma.autoinit','dma.mode','dma.rate','dma.mul','dma.total','dma.left','dma.min','dma.start','dma.bits','dma.remain_size',
   'irq.pending_8bit','irq.pending_16bit',
   'dsp.state','dsp.cmd','dsp.cmd_len','dsp.cmd_in_pos','dsp.in.lastval','dsp.in.pos','dsp.in.used','dsp.out.lastval','dsp.out.pos','dsp.out.used','dsp.test_register','dsp.write_busy',
   'dac.used','dac.last','mixer.index','mixer.mic','mixer.stereo','mixer.enabled','mixer.filtered',
   'adpcm.reference','adpcm.stepsize','adpcm.haveref','hw.base','hw.irq','hw.dma8','hw.dma16','e2.value','e2.count')
  device={key:int(gdb.parse_and_eval("'sblaster.cpp'::sb."+key)) for key in scalar_fields}
  arrays={key:bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval("&'sblaster.cpp'::sb."+key)),int(gdb.parse_and_eval("sizeof('sblaster.cpp'::sb."+key+")")))).hex() for key in
   ('dma.buf','dsp.cmd_in','dsp.in.data','dsp.out.data','dac.data','mixer.dac','mixer.fm','mixer.cda','mixer.master','mixer.lin','mixer.unhandled')}
  save(self.label,dict(opcode_physical=p,opcode_hex=m[p:p+self.size].hex(),SB=device,SB_arrays=arrays,SB_dma_channel_bound=int(sb['dma']['chan'])!=0,**({'HOST':host_state()} if through_dos else {})))
  if self.label=='device-entry':arm_reads(m,0x1348,ext,'before-reset-write')
  if self.label=='before-reset-write':arm_reads(m,0x1349,ext,'after-reset-write')
  if through_dos and self.label=='after-reset-write':arm_reads(m,0x1355,ext,'before-reset-xor')
  if through_dos and self.label=='before-reset-xor':arm_reads(m,0x1357,ext,'after-reset-xor')
  if through_dos and self.label=='after-reset-xor':arm_reads(m,0x1358,ext,'after-reset-off')
  if through_dos and self.label=='after-reset-off':arm_reads(m,0x137c,ext,'before-reset-ack')
  if through_dos and self.label=='before-reset-ack':arm_reads(m,0x137d,ext,'after-reset-ack')
  if through_dos and self.label=='after-reset-ack':arm_reads(m,0x14a1,m[0xf0000:0x100000],'before-dos-callback')
  if through_dos and self.label=='before-dos-callback':arm_reads(m,0x14a5,m[0xf0000:0x100000],'after-dos-callback')
  if through_dos and self.label=='after-dos-callback':
   assert q['cpu.pmode']==0
   stack=physical(q['segments'][2]['base']+(q['registers'][4]&q['cpu.stack.mask']),m,q)
   return_ip,return_cs=struct.unpack_from('<HH',m,stack)
   code=bytearray(return_ip+32)
   for byte in range(32):code[return_ip+byte]=m[physical((return_cs<<4)+return_ip+byte,m,q)]
   arm_reads(m,return_ip,bytes(code),'after-dos-iret')
  if through_dos and self.label=='after-dos-iret':arm_reads(m,0x5df4,ext,'before-find-first-int')
  if through_dos and self.label=='before-find-first-int':
   find_active=True;arm_reads(m,0x14a1,m[0xf0000:0x100000],'before-find-first-callback')
  if through_dos and self.label=='before-find-first-callback':arm_reads(m,0x14a5,m[0xf0000:0x100000],'after-find-first-callback')
  if through_dos and self.label=='after-find-first-callback':arm_reads(m,0x5df6,ext,'find-first-caller')
  if through_dos and self.label=='find-first-caller':find_active=False
  if self.label=='engine-e339':
   arm_reads(m,0xe33c,dat,'after-push-immediate')
  if self.label=='after-push-immediate':
   pointer=physical((q['segments'][3]['base']+0xea16)&0xffffffff,m,q)
   target_ip,target_cs=struct.unpack_from('<HH',m,pointer)
   assert target_ip==0x1179
   resident=bytes(relocate(resident_model,target_cs<<4))
   arm_reads(m,0x1185,resident,'before-push-ss')
  if self.label=='before-push-ss':arm_reads(m,0x1186,resident,'after-push-ss')
  if self.label=='after-push-ss':arm_reads(m,0x118f,resident,'before-sub-word')
  if self.label=='before-sub-word':arm_reads(m,0x1196,resident,'after-sub-word')
  if self.label=='after-sub-word':arm_reads(m,0x11bd,resident,'before-push-full')
  if self.label=='before-push-full':arm_reads(m,0x11c3,resident,'after-push-full')
  if self.label=='after-push-full':arm_reads(m,0x11c9,resident,'before-pop-ss')
  if self.label=='before-pop-ss':arm_reads(m,0x11ca,resident,'after-pop-ss')
  if self.label=='after-pop-ss':arm_reads(m,0x11cd,resident,'after-xchg')
  if self.label=='after-xchg':arm_reads(m,0x11da,resident,'before-shl-dword')
  if self.label=='before-shl-dword':arm_reads(m,0x11de,resident,'after-shl-dword')
  if self.label=='after-shl-dword':
   arm_reads(m,0xf30,ext,'dispatcher-entry')
   arm_reads(m,0x77e2,ext,'device-entry')
   arm_reads(m,0xf57,ext,'dispatcher-return')
   arm_reads(m,0xe34c,dat,'engine-gate-return')
  return False
class FindReturned(gdb.FinishBreakpoint):
 def __init__(self,kind):self.kind=kind;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):find_record(self.kind);return False
class FindEntry(gdb.Breakpoint):
 def __init__(self,symbol,kind):self.kind=kind;super().__init__(symbol,internal=True)
 def stop(self):
  if not find_active:return False
  args=None
  if self.kind=='before-FindFirst':args=dict(search=gdb.parse_and_eval('search').string(),attr=int(gdb.parse_and_eval('attr')),fcb_findfirst=int(gdb.parse_and_eval('fcb_findfirst')))
  if self.kind=='before-SetupSearch':args=dict(drive=int(gdb.parse_and_eval('_sdrive')),attr=int(gdb.parse_and_eval('_sattr')),pattern=gdb.parse_and_eval('pattern').string(),pt=int(gdb.parse_and_eval('this->pt')))
  if self.kind=='before-directory':args=dict(path=gdb.parse_and_eval('path').string(),next_free=int(gdb.parse_and_eval('this->nextFreeFindFirst')))
  if self.kind=='before-SetResult':
   args={k:int(gdb.parse_and_eval('_'+k)) for k in ('size','date','time','attr')}
   args.update(name=gdb.parse_and_eval('_name').string(),name_raw_hex=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('_name')),13)).hex(),pt=int(gdb.parse_and_eval('this->pt')))
  find_record(self.kind,args);FindReturned(self.kind.replace('before-','after-'));return False
if through_dos:
 FindEntry('DOS_21Handler','before-DOS21');FindEntry('DOS_FindFirst','before-FindFirst')
 FindEntry('DOS_DTA::SetupSearch','before-SetupSearch');FindEntry('DOS_Drive_Cache::FindFirst','before-directory');FindEntry('DOS_DTA::SetResult','before-SetResult')
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=500:return False
  m=memory();arm_reads(m,0xe339,dat,'engine-e339');self.enabled=False;return False
Arm('TIMER_AddTick')
end
run
'''
 ast.parse(script.split('python\n',1)[1].rsplit('\nend\nrun',1)[0])
 return script
def verify(repo,root,through_dos=False):
 assert (root/'source.exit').read_text()=='0\n'
 for name,h in json.loads((root/'inputs.json').read_text()).items():assert digest(name)==h,name
 originals=json.loads((root/'originals.json').read_text())
 assert len(originals)==419
 for name,h in originals.items():assert digest(name)==h,name
 folder=root/'source';baseline=root/'baseline'
 for suffix in ('frames','pcm','end'):assert (folder/('sequence.'+suffix)).read_bytes()==(baseline/('sequence.'+suffix)).read_bytes(),suffix
 assert validate_endpoint(folder/'sequence',600)==600
 assert validate(folder/'sequence.frames','F')['records']==39
 assert validate(folder/'sequence.pcm','A')['samples']==27518
 assert 'Python Exception' not in (folder/'dosbox.log').read_text()
 events=[json.loads(s) for s in (folder/'events.jsonl').read_text().splitlines()]
 expected=['engine-e339', 'after-push-immediate', 'before-push-ss', 'after-push-ss', 'before-sub-word', 'after-sub-word', 'before-push-full', 'after-push-full', 'before-pop-ss', 'after-pop-ss', 'after-xchg', 'before-shl-dword', 'after-shl-dword', 'dispatcher-entry', 'device-entry', 'before-reset-write', 'after-reset-write', 'dispatcher-return', 'engine-gate-return']
 if through_dos:expected=['engine-e339', 'after-push-immediate', 'before-push-ss', 'after-push-ss', 'before-sub-word', 'after-sub-word', 'before-push-full', 'after-push-full', 'before-pop-ss', 'after-pop-ss', 'after-xchg', 'before-shl-dword', 'after-shl-dword', 'dispatcher-entry', 'device-entry', 'before-reset-write', 'after-reset-write', 'before-reset-xor', 'after-reset-xor', 'after-reset-off', 'before-reset-ack', 'after-reset-ack', 'before-dos-callback', 'after-dos-callback', 'after-dos-iret', 'before-find-first-int', 'before-find-first-callback', 'after-find-first-callback', 'find-first-caller', 'dispatcher-return', 'engine-gate-return']
 assert [q['kind'] for q in events]==expected
 for q in events:
  validate_context(q['memory_context'],folder,repo)
  assert (folder/q['memory_file']).stat().st_size==16777216
 trace=list(records(root/'cpu.trace'))
 positions=[]
 for q in events:
  matches=[i for i,r in enumerate(trace) if r==trace_key(q)]
  assert len(matches)==1,(q['kind'],matches)
  positions.append(matches[0])
 assert positions==sorted(positions) and len(set(positions))==len(positions)
 assert positions[16]-positions[0]+1==137
 before,after=events[15:17]
 assert before['opcode_hex'].startswith('ee') and (before['registers'][2]&0xffff,before['registers'][0]&0xff)==(0x226,1)
 assert after['cpu_regs.ip.dword[0]']==before['cpu_regs.ip.dword[0]']+1
 assert before['SB']['dsp.state']==2 and after['SB']['dsp.state']==0
 assert {k:v for k,v in before['SB'].items() if k!='dsp.state'}=={k:v for k,v in after['SB'].items() if k!='dsp.state'}
 assert before['SB_arrays']==after['SB_arrays']
 find=[]
 if through_dos:
  find=[json.loads(s) for s in (folder/'find-events.jsonl').read_text().splitlines()]
  assert [q['kind'] for q in find]==['before-DOS21','before-FindFirst','before-SetupSearch','after-SetupSearch','before-directory','after-directory','before-SetResult','after-SetResult','after-FindFirst','after-DOS21']
  for q in find:
   validate_context(q['memory_context'],folder,repo)
   assert (folder/q['memory_file']).stat().st_size==16777216
 proof=dict(find_events=find,scope='Actual first op6c enginee339, protected dispatcher,77e2 and real caller return with full CPU/system/RAM/provider/cache/VGA/PIC/calendar states. Original full600ms39frames/27518mixedPCM unchanged; no native execution or runtime CPU seed.',events=events,trace_records=len(trace),inputs_sha256=json.loads((root/'inputs.json').read_text()),complete_original_acceptance=False)
 (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 print('PASS actual firstop6c full taskgate states',len(events),'trace',len(trace),flush=True)
 return proof

def source_inputs(repo):
 return (Path(__file__),repo/'re_out/fist_dat_image.bin',repo/'re_out/fist_image.bin',repo/'tools/oracle/cpu_trace.py',repo/'tools/oracle/resident_image.py',repo/'tools/oracle/sb_irq_frame_case.json',repo/'third_party/dosbox-build/dosbox-0.74-3/src/hardware/sblaster.cpp',repo/'third_party/dosbox-build/dosbox-0.74-3/src/dos/dos.cpp',repo/'third_party/dosbox-build/dosbox-0.74-3/include/dos_inc.h',*[repo/'third_party/dosbox-build/dosbox-0.74-3'/name for name in ('src/dos/dos_files.cpp','src/dos/dos_classes.cpp','src/dos/drive_local.cpp','src/dos/drive_cache.cpp','include/dos_system.h')])
if __name__=='__main__':
 parser=argparse.ArgumentParser(description='Capture actual original op6c real/protected task gate, its bootstrap instructions,first DSP reset OUT and unchanged full600ms output.')
 parser.add_argument('--repo',type=Path,required=True)
 parser.add_argument('--output',type=Path,required=True)
 parser.add_argument('--verify-only',action='store_true')
 parser.add_argument('--through-dos',action='store_true',help='Continue through reset acknowledgement, AH1a and first AH4e to the protected caller, observing DOS host state.')
 args=parser.parse_args();repo=args.repo.resolve(strict=True);root=args.output.resolve()
 assert root.is_relative_to(Path('/tmp'))
 if args.verify_only:verify(repo,root,args.through_dos)
 else:capture(repo,root,make_observer=lambda repo:observer(repo,args.through_dos),check_capture=lambda repo,root:verify(repo,root,args.through_dos),
  additional_inputs=source_inputs(repo),source_wall_seconds=180)
