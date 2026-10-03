set pagination off
set confirm off
python
# Reuse original error/continuation observers and their state/RAM/paging owner.
from pathlib import Path
import os
shared_probe=Path(os.environ['FIST_DETAIL_REPO'])/'tools/oracle/file_error_main.gdb'
program=shared_probe.read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
exec(compile(program,str(shared_probe),'exec'),globals())
dispatcher_cs=None
reference_selector=json.loads((repo/'tools/oracle/file_error_case.json').read_text())['states']['at-6032']['segments'][1]['value']
service_chain={0xf30:('service-entry',0xf36),
 0xf36:('service-stack-saved',0xf39),0xf39:('service-index',0xf3f),
 0xf3f:('service-target',0xf45),0xf45:('service-task-read',0xf4b),
 0xf4b:('service-inbox-read',0xf51),0xf51:('service-handler-call',0x10da),
 0x10da:('service-handler-entry',0x7660),0x7660:('service-body-entry',None)}
class ServiceCodeRead(gdb.Breakpoint):
 def __init__(self,address,entry):
  self.entry=entry;self.address=address
  super().__init__('*(unsigned char *)(%d)'%address,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True)
 def stop(self):
  global dispatcher_cs
  q=state()
  if q['cpu_regs.ip.dword[0]']!=self.entry:return False
  if dispatcher_cs is None:
   m=memory();a=physical((q['segments'][1]['base']+self.entry)&0xffffffff,m,q)
   if int(gdb.parse_and_eval('MemBase'))+a!=self.address:return False
   dispatcher_cs=q['segments'][1]['value']
  elif q['segments'][1]['value']!=dispatcher_cs:return False
  if self.entry==0xf30:
   with (root/'dispatch-selectors.jsonl').open('a') as out:out.write(json.dumps({'selector':q['registers'][3],'PIC_Ticks':q['PIC_Ticks']})+'\n')
   if (q['registers'][3]&65535)!=0x44:return False
  self.enabled=False;label,next_entry=service_chain[self.entry];snapshot(label)
  q=state();m=memory();base=int(gdb.parse_and_eval('MemBase'));code=q['segments'][1]['base']
  if next_entry is not None:watches.append(ServiceCodeRead(base+physical((code+next_entry)&0xffffffff,m,q),next_entry))
  return False
class DispatcherCSWrite(gdb.Breakpoint):
 def stop(self):
  q=state()
  if q['segments'][1]['value']!=reference_selector:return False
  m=memory();image=(repo/'re_out/fist_image.bin').read_bytes()
  address=physical((q['segments'][1]['base']+0x84c0)&0xffffffff,m,q)
  if m[address:address+32]!=image[0x84c0:0x84e0]:return False
  (root/'dispatcher-arm.json').write_text(json.dumps(q,indent=2)+'\n')
  self.enabled=False;base=int(gdb.parse_and_eval('MemBase'))
  watches.append(ServiceCodeRead(base+physical((q['segments'][1]['base']+0xf30)&0xffffffff,m,q),0xf30))
  return False
class DispatcherArm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=10:return False
  DispatcherCSWrite('Segs.val[1]',type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_WRITE,internal=True)
  self.enabled=False;return False
if observe:DispatcherArm('TIMER_AddTick')
end
run
