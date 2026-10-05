#!/usr/bin/env python3
"""Observe the actual op6c real/protected task gate and first DSP reset write."""
from pathlib import Path
import argparse,hashlib,json,os,sys
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

def observer(repo):
 base=shared_observer(repo);prefix=base[:base.index('class QueueReturned(')]
 return prefix+'''
import sys
sys.path.insert(0,str(repo/'tools/oracle'))
from resident_image import load,relocate
resident_model=load(repo)
resident=None
dat=(repo/'re_out/fist_dat_image.bin').read_bytes()
ext=(repo/'re_out/fist_image.bin').read_bytes()
gdb.execute('set environment FIST_CPU_TRACE='+str(root.parent/'cpu.trace'))
gdb.execute('set environment FIST_CPU_TRACE_WINDOW=500:536')
watches=[];started=False
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
  global started,resident
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
  save(self.label,dict(opcode_physical=p,opcode_hex=m[p:p+self.size].hex(),SB=device,SB_arrays=arrays,SB_dma_channel_bound=int(sb['dma']['chan'])!=0))
  if self.label=='device-entry':arm_reads(m,0x1348,ext,'before-reset-write')
  if self.label=='before-reset-write':arm_reads(m,0x1349,ext,'after-reset-write')
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
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=500:return False
  m=memory();arm_reads(m,0xe339,dat,'engine-e339');self.enabled=False;return False
Arm('TIMER_AddTick')
end
run
'''
def verify(repo,root):
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
 assert [q['kind'] for q in events]==['engine-e339','after-push-immediate','before-push-ss','after-push-ss','before-sub-word','after-sub-word','before-push-full','after-push-full','before-pop-ss','after-pop-ss','after-xchg','before-shl-dword','after-shl-dword','dispatcher-entry','device-entry','before-reset-write','after-reset-write','dispatcher-return','engine-gate-return']
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
 proof=dict(scope='Actual first op6c enginee339, protected dispatcher,77e2 and real caller return with full CPU/system/RAM/provider/cache/VGA/PIC/calendar states. Original full600ms39frames/27518mixedPCM unchanged; no native execution or runtime CPU seed.',events=events,trace_records=len(trace),inputs_sha256=json.loads((root/'inputs.json').read_text()),complete_original_acceptance=False)
 (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 print('PASS actual firstop6c full taskgate states',len(events),'trace',len(trace),flush=True)
 return proof
if __name__=='__main__':
 parser=argparse.ArgumentParser(description='Capture actual original op6c real/protected task gate, its bootstrap instructions,first DSP reset OUT and unchanged full600ms output.')
 parser.add_argument('--repo',type=Path,required=True)
 parser.add_argument('--output',type=Path,required=True)
 parser.add_argument('--verify-only',action='store_true')
 args=parser.parse_args();repo=args.repo.resolve(strict=True);root=args.output.resolve()
 assert root.is_relative_to(Path('/tmp'))
 if args.verify_only:verify(repo,root)
 else:capture(repo,root,make_observer=observer,check_capture=verify,
  additional_inputs=(Path(__file__),repo/'re_out/fist_dat_image.bin',repo/'re_out/fist_image.bin',repo/'tools/oracle/cpu_trace.py',repo/'tools/oracle/resident_image.py',repo/'tools/oracle/sb_irq_frame_case.json',repo/'third_party/dosbox-build/dosbox-0.74-3/src/hardware/sblaster.cpp'),source_wall_seconds=180)
