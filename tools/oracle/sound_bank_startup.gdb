set pagination off
set confirm off
python
# Reuse the original memory-manager state/RAM/paging snapshot owner.
from pathlib import Path
import os
shared_probe=Path(os.environ['FIST_MEMMGR_SOURCE_REPO'])/'tools/oracle/memmgr_startup.gdb'
program=shared_probe.read_text().split('\npython\n',1)[1].split('watches=[];allocations=0;allocator=None',1)[0]
exec(compile(program,str(shared_probe),'exec'),globals())
shared_snapshot=snapshot
ext_cs=None

def snapshot(label):
 q,m=shared_snapshot(label)
 engine=physical((q['segments'][3]['base']+q['memmgr_slots']['0xca1'])&0xffffffff,m,q)
 off,seg=struct.unpack_from('<HH',m,engine+0xea2c)
 q.update(engine_dgroup_physical=engine,engine_task={'segment':seg,'offset':off,
          'physical':physical((seg<<4)+off,m,q)})
 (root/(label+'.json')).write_text(json.dumps(q,indent=2)+'\n')
 return q,m
watches=[]
chain={0x77e2:('device-start-entry',0x77ee),0x77ee:('after-device-configuration',0x780e),0x780e:('after-checkpoint-free',0x781a),0x781a:('after-size-query',0x36bf),0x36bf:('allocation-entry',0x7832),0x7832:('allocation-return',0x7841),0x7841:('after-bank-read',0x7858),0x7858:('after-checkpoint',0x7869),0x7869:('device-start-return',0x6e95),0x6e95:('kdv-entry',None)}
def watch(entry,q,m):
 base=int(gdb.parse_and_eval('MemBase'));code=q['segments'][1]['base']
 p=CodeRead(base+physical((code+entry)&0xffffffff,m,q),entry);watches.append(p)
class CodeRead(gdb.Breakpoint):
 def __init__(self,addr,entry):
  self.entry=entry;self.address=addr
  super().__init__('*(unsigned char *)(%d)'%addr,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True)
 def stop(self):
  global ext_cs
  q=state()
  if q['cpu_regs.ip.dword[0]']!=self.entry:return False
  if ext_cs is None:
   # CS writes can precede descriptor updates. Derive the role at the actual fetch.
   m=memory();guest=physical((q['segments'][1]['base']+self.entry)&0xffffffff,m,q)
   if int(gdb.parse_and_eval('MemBase'))+guest!=self.address:return False
   ext_cs=q['segments'][1]['value']
  elif q['segments'][1]['value']!=ext_cs:return False
  if self.entry==0x36bf and q['registers'][2]!=0x85b0:return False
  self.enabled=False;label,next_entry=chain[self.entry];q,m=snapshot(label)
  if next_entry is not None:watch(next_entry,q,m)
  return False
class CSWrite(gdb.Breakpoint):
 def stop(self):
  q=state()
  global ext_cs
  m=memory();image=(repo/'re_out/fist_image.bin').read_bytes()
  addr=physical((q['segments'][1]['base']+0x84c0)&0xffffffff,m,q)
  if m[addr:addr+32]!=image[0x84c0:0x84e0]:return False
  code=physical((q['segments'][1]['base']+0x77e2)&0xffffffff,m,q)
  if m[code:code+32]!=image[0x77e2:0x7802]:return False
  self.enabled=False;watch(0x77e2,q,m);return False
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=10:return False
  CSWrite('Segs.val[1]',type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_WRITE,internal=True);self.enabled=False;return False
Arm('TIMER_AddTick')
end
run
