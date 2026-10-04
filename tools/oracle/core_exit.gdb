# Read-only first application IRET with a pending IRQ, through its first handler fetch.
set pagination off
set confirm off
python
import ast,gdb,json,os,struct
from pathlib import Path
repo=Path(os.environ['FIST_DETAIL_REPO']);root=Path(os.environ['FIST_DETAIL_OPERANDS_DIR'])
shared=repo/'tools/oracle/file_error.gdb'
program=shared.read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
nodes=[n for n in ast.parse(program).body if isinstance(n,ast.FunctionDef) and n.name in ('state','memory','physical') or isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='fields' for t in n.targets)]
assert len(nodes)==4
exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),str(shared),'exec'),globals())
# CORE_MEMORY_CONTEXT is composed by the capture tool from the existing owner.
fields=fields+('cpu.stack.mask','cpu.stack.notmask','PIC_IRQCheck','PIC_IRQActive','CPU_IODelayRemoved')+SYSTEM_FIELDS
def pic_state():
 irq_fields=('masked','active','inservice','vector')
 controller_fields=('icw_words','icw_index','masked','special','auto_eoi','rotate_on_auto_eoi','single','request_issr','vector_base')
 return dict(irqs=[{k:int(gdb.parse_and_eval("'pic.cpp'::irqs[%d].%s"%(i,k))) for k in irq_fields} for i in range(16)],controllers=[{k:int(gdb.parse_and_eval("'pic.cpp'::pics[%d].%s"%(i,k))) for k in controller_fields} for i in range(2)],special=bool(gdb.parse_and_eval("'pic.cpp'::PIC_Special_Mode")),secondary_pending=int(gdb.parse_and_eval('PIC_IRQOnSecondPicActive')))
def calendar():
 entries=[];entry=gdb.parse_and_eval("'pic.cpp'::pic_queue.next_entry")
 while int(entry):
  assert len(entries)<512
  p=entry.dereference();raw=bytes(gdb.selected_inferior().read_memory(int(p['index'].address),4))
  entries.append(dict(index_bits=int.from_bytes(raw,'little'),value=int(p['value']),handler=str(p['pic_event'])))
  entry=p['next']
 return entries
def save(kind,extra=None):
 q=state();q.update(kind=kind,trap_decoder=bool(gdb.parse_and_eval('cpudecoder==CPU_Core_Normal_Trap_Run')),pic=pic_state(),calendar=calendar())
 if extra:q.update(extra)
 name=kind+'.memory';q['memory_file']=name;q['memory_context']=memory_context(name)
 raw=memory();(root/name).write_bytes(raw)
 if kind=='before-iret':
  ip=q['cpu_regs.ip.dword[0]'];address=physical((q['segments'][1]['base']+ip)&0xffffffff,raw,q)
  q.update(opcode_physical=address,opcode_hex=raw[address:address+16].hex(),core_next_ip=int(gdb.parse_and_eval("'core_normal.cpp'::core.cseip-Segs.phys[1]")))
 with (root/'events.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
 return q
class QueueReturned(gdb.FinishBreakpoint):
 def __init__(self,frame):super().__init__(frame,internal=True)
 def stop(self):save('after-queue');fetch.enabled=True;return False
class HardwareReturned(gdb.FinishBreakpoint):
 def __init__(self):super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):save('after-hardware');return False
class Hardware(gdb.Breakpoint):
 def stop(self):
  assert int(gdb.parse_and_eval('type'))==0
  q=save('before-hardware',{'num':int(gdb.parse_and_eval('num')),'type':0,'oldeip':int(gdb.parse_and_eval('oldeip'))})
  HardwareReturned();parent=gdb.newest_frame().older()
  while parent and parent.name()!='PIC_RunQueue':parent=parent.older()
  assert parent is not None
  QueueReturned(parent);self.enabled=False;return False
hardware=Hardware('CPU_Interrupt');hardware.enabled=False
class Fetch(gdb.Breakpoint):
 def stop(self):save('handler-fetch');self.enabled=False;return False
fetch=Fetch('fist_cpu_trace');fetch.enabled=False
class CoreReturned(gdb.FinishBreakpoint):
 def __init__(self,frame):super().__init__(frame,internal=True)
 def stop(self):save('after-core');hardware.enabled=True;return False
class IretReturned(gdb.FinishBreakpoint):
 def __init__(self,frame):self.core_frame=frame;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  q=save('after-iret')
  assert q['cpu_regs.flags']&0x200 and q['PIC_IRQCheck'] and not q['cpu_regs.flags']&0x100
  CoreReturned(self.core_frame);return False
class Iret(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))<500 or not int(gdb.parse_and_eval('PIC_IRQCheck')):return False
  parent=gdb.newest_frame().older()
  if not parent or parent.name()!='CPU_Core_Normal_Run':return False
  save('before-iret',{'use32':int(gdb.parse_and_eval('use32')),'oldeip':int(gdb.parse_and_eval('oldeip'))})
  IretReturned(parent);self.enabled=False;return False
Iret('CPU_IRET')
end
run
