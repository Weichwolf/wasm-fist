# Read-only first IRQ0 at each distinct IVT/IDT destination and reached system loads.
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
fields=fields+('cpu.stack.mask','cpu.stack.notmask','cpu.idt.table_base','cpu.idt.table_limit','cpu.gdt.table_base','cpu.gdt.table_limit','cpu.mpl','cpu.trap_skip','PIC_IRQActive','PIC_IRQCheck','CPU_IODelayRemoved','cpu.gdt.ldt_base','cpu.gdt.ldt_limit','cpu.gdt.ldt_value','cpu_tss.base','cpu_tss.limit','cpu_tss.selector','cpu_tss.is386','cpu_tss.desc.saved.fill[0]','cpu_tss.desc.saved.fill[1]','cpu.exception.which','cpu.exception.error','cpu_tss.valid','CPU_flag_id_toggle','cpu.direction','lastint')
selected={};active=[];events=0;serial=0;seen=set();sparse=bytearray(16777216)
bootstrap=os.environ.get('FIST_ORACLE_BOOTSTRAP')=='1'
boot_active=False;boot_finished=set();boot_context=0;boot_group=0;boot_fetches=0
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
    if not active and not boot_active:trace.enabled=False
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
  global boot_fetches
  if boot_active:
   boot_fetches+=1
   q=state();q.update(kind='boot-fetch',ordinal=boot_fetches,context=boot_context,group=boot_group,
       architecture=int(gdb.parse_and_eval('CPU_ArchitectureType')),
       auto_determine=int(gdb.parse_and_eval('CPU_AutoDetermineMode')),
       normal_core=bool(gdb.parse_and_eval('cpudecoder == &CPU_Core_Normal_Run')))
   m=memory();linear=(q['segments'][1]['base']+q['cpu_regs.ip.dword[0]'])&0xffffffff
   address=physical(linear,m,q)
   q.update(fetched_code_physical=address,fetched_code_hex=m[address:address+32].hex(),
       memory_file='boot-fetch-%d-%d.memory'%(boot_group,boot_fetches))
   (root/q['memory_file']).write_bytes(m)
   with (root/'boot-fetches.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
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
system_events=0;system_seen=set();system_contexts={}
class SystemReturned(gdb.FinishBreakpoint):
 def __init__(self,label,operation):
  self.label=label;self.operation=operation;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  global boot_active
  if boot_active and self.operation=='CPU_LTR':
   boot_active=False;boot_finished.add(boot_context)
   if not active:trace.enabled=False
  q=state();q.update(kind='after-system',label=self.label)
  if bootstrap:q['context']=system_contexts[self.label]
  q['memory_file']='system-after-%d.memory'%self.label
  (root/q['memory_file']).write_bytes(memory())
  if self.return_value is not None:q['return_value']=int(self.return_value)
  with (root/'system-events.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
  return False
class System(gdb.Breakpoint):
 def __init__(self,name):
  self.operation=name;super().__init__(name,internal=True)
 def stop(self):
  global system_events,boot_active,boot_context,boot_group,boot_fetches
  args={k:int(gdb.parse_and_eval(k)) for k in (('limit','base') if self.operation in ('CPU_LGDT','CPU_LIDT') else ('selector',))}
  context=active[-1] if bootstrap and active and selected[active[-1]]['cpu.pmode'] else 0
  key=(context,self.operation,tuple(args.items()))
  if key in system_seen:return False
  system_seen.add(key);system_events+=1;system_contexts[system_events]=context
  if bootstrap and self.operation=='CPU_LGDT' and context not in boot_finished:
   assert not boot_active
   boot_active=True;boot_context=context;boot_group+=1;boot_fetches=0;trace.enabled=True
  q=state();q.update(kind='before-system',label=system_events,operation=self.operation,arguments=args)
  if bootstrap:q['context']=context
  q['memory_file']='system-before-%d.memory'%system_events
  (root/q['memory_file']).write_bytes(memory())
  with (root/'system-events.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
  SystemReturned(system_events,self.operation);return False
for name in ('CPU_LGDT','CPU_LIDT','CPU_LLDT','CPU_LTR'):System(name)

trace=Fetch('fist_cpu_trace');trace.enabled=False
Hardware('CPU_Interrupt');Iret('CPU_IRET')
def exited(event):
 q=dict(exit_code=event.exit_code,hardware_events=events,selected=list(selected),active=active,serial=serial,system_events=system_events,fetches=sum(1 for line in (root/'irq-fetches.jsonl').open()))
 if bootstrap:q.update(boot_fetches=boot_fetches,boot_finished=sorted(boot_finished),boot_groups=boot_group)
 (root/'irq-completion.json').write_text(json.dumps(q)+'\n')
gdb.events.exited.connect(exited)
end
run
