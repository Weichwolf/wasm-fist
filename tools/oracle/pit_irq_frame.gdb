# Read-only first IRQ0 frame at each distinct actual IVT/IDT destination.
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
fields=fields+('cpu.stack.mask','cpu.stack.notmask','cpu.idt.table_base','cpu.idt.table_limit','cpu.gdt.table_base','cpu.gdt.table_limit','cpu.mpl','cpu.trap_skip','PIC_IRQActive','PIC_IRQCheck','CPU_IODelayRemoved','cpu.gdt.ldt_base','cpu.gdt.ldt_limit','cpu.gdt.ldt_value','cpu_tss.base','cpu_tss.limit','cpu_tss.selector','cpu_tss.is386','cpu_tss.valid','CPU_flag_id_toggle','cpu.direction','lastint')
selected={};active=[];events=0;serial=0;seen=set();sparse=bytearray(16777216)
def save(kind,index,full=False,extra=None):
 global serial
 serial+=1
 q=state();q.update(kind=kind,index=index,serial=serial)
 q['irq0']=[int(gdb.parse_and_eval("'pic.cpp'::irqs[0].%s"%field)) for field in ('active','masked','inservice','vector')]
 if extra:q.update(extra)
 if full:
  name='%s-%d-%d.memory'%(kind,index,serial);q['memory_file']=name;(root/name).write_bytes(memory())
 with (root/'irq-events.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
 return q
class Returned(gdb.FinishBreakpoint):
 def __init__(self,index,kind):
  self.index=index;self.kind=kind;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  q=save(self.kind,self.index,True)
  if self.kind=='after-iret':
   first=selected[self.index]
   if q['cpu_regs.ip.dword[0]']==first['cpu_regs.ip.dword[0]'] and q['segments'][1]==first['segments'][1] and q['registers'][4]==first['registers'][4]:
    active.remove(self.index)
    if not active:trace.enabled=False
  return False
class Hardware(gdb.Breakpoint):
 def stop(self):
  global events
  if int(gdb.parse_and_eval('type'))!=0:return False
  q=state();num=int(gdb.parse_and_eval('num'));old=int(gdb.parse_and_eval('oldeip'));events+=1
  save('hardware',events,False,{'num':num,'type':0,'oldeip':old})
  if num==int(gdb.parse_and_eval("'pic.cpp'::irqs[0].vector")):
   m=memory();linear=q['cpu.idt.table_base']+num*(8 if q['cpu.pmode'] else 4)
   where=physical(linear,m,q);destination=m[where:where+(8 if q['cpu.pmode'] else 4)].hex()
   key=(q['cpu.pmode'],destination)
   if key not in seen:
    seen.add(key)
    q=save('before-hardware',events,True,{'num':num,'type':0,'oldeip':old,'vector_hex':destination,'vector_physical':where})
    selected[events]=q;active.append(events);Returned(events,'after-hardware');trace.enabled=True
  return False
class Iret(gdb.Breakpoint):
 def stop(self):
  if not active:return False
  index=active[-1];save('before-iret',index,True,{'use32':int(gdb.parse_and_eval('use32')),'oldeip':int(gdb.parse_and_eval('oldeip'))})
  Returned(index,'after-iret');return False
class Fetch(gdb.Breakpoint):
 def stop(self):
  if not active:return False
  q=state();base=int(gdb.parse_and_eval('MemBase'));linear=(q['segments'][1]['base']+q['cpu_regs.ip.dword[0]'])&0xffffffff
  inferior=gdb.selected_inferior()
  if q['paging.enabled']:
   directory=(q['paging.cr3']&0xfffff000)+4*(linear>>22)
   sparse[directory:directory+4]=bytes(inferior.read_memory(base+directory,4));pde=struct.unpack_from('<I',sparse,directory)[0]
   if not pde&128:
    table=(pde&0xfffff000)+4*((linear>>12)&1023)
    sparse[table:table+4]=bytes(inferior.read_memory(base+table,4))
  address=physical(linear,sparse,q);q.update(kind='fetch',index=active[-1],fetched_code_physical=address,fetched_code_hex=bytes(inferior.read_memory(base+address,32)).hex())
  with (root/'irq-fetches.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
  return False
# SHELL_Init copies the entire stack CommandTail, including its uninitialized
# suffix. Record the actual input; never normalize it or change guest memory.
class TailReturned(gdb.FinishBreakpoint):
 def __init__(self,q):
  self.q=q;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  q=self.q;base=int(gdb.parse_and_eval('MemBase'))
  q['after_hex']=bytes(gdb.selected_inferior().read_memory(base+q['physical'],q['size'])).hex()
  (root/'shell-tail-copy.json').write_text(json.dumps(q)+'\n')
  return False
class TailCopy(gdb.Breakpoint):
 def stop(self):
  caller=gdb.newest_frame().older()
  if not caller or caller.name()!='SHELL_Init':return False
  address=int(gdb.parse_and_eval('pt'));psp=int(caller.read_var('psp_seg'))
  if address!=psp*16+128:return False
  size=int(gdb.lookup_type('CommandTail').sizeof)
  assert int(gdb.parse_and_eval('size'))+1==size
  raw=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('data')),size))
  q=dict(physical=address,psp_segment=psp,size=size,count=raw[0],
         initialized_bytes=raw[0]+2,defined_prefix_hex=raw[:raw[0]+2].hex(),
         source_hex=raw.hex(),caller=caller.name(),source_line=caller.find_sal().line)
  TailReturned(q);self.enabled=False;return False
TailCopy('MEM_BlockWrite')
trace=Fetch('fist_cpu_trace');trace.enabled=False
Hardware('CPU_Interrupt');Iret('CPU_IRET')
def exited(event):
 (root/'irq-completion.json').write_text(json.dumps(dict(exit_code=event.exit_code,hardware_events=events,selected=list(selected),active=active,serial=serial,fetches=sum(1 for line in (root/'irq-fetches.jsonl').open())))+'\n')
gdb.events.exited.connect(exited)
end
run
