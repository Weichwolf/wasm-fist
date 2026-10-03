set pagination off
set confirm off
python
import gdb,json,os,struct
from pathlib import Path
root=Path(os.environ['FIST_MEMMGR_SOURCE_DIR'])
repo=Path(os.environ['FIST_MEMMGR_SOURCE_REPO'])
fields=('PIC_Ticks','CPU_Cycles','CPU_CycleLeft','CPU_CycleMax','cpu_regs.ip.dword[0]',
        'cpu.cr0','paging.cr3','paging.enabled','cpu.code.big','cpu.stack.big','cpu.pmode',
        'cpu.cpl','cpu_regs.flags','lflags.type','lflags.prev_type','lflags.oldcf',
        'lflags.var1.dword[0]','lflags.var2.dword[0]','lflags.res.dword[0]')
def state():
 q={n:int(gdb.parse_and_eval(n)) for n in fields}
 q['registers']=[int(gdb.parse_and_eval('cpu_regs.regs[%d].dword[0]'%i)) for i in range(8)]
 q['segments']=[{'value':int(gdb.parse_and_eval('Segs.val[%d]'%i)),
                 'base':int(gdb.parse_and_eval('Segs.phys[%d]'%i))} for i in range(6)]
 return q
def memory():
 return bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('MemBase')),16777216))
def physical(linear,m,q):
 if not q['paging.enabled']:return linear
 pde=struct.unpack_from('<I',m,(q['paging.cr3']&0xfffff000)+((linear>>22)&1023)*4)[0]
 assert pde&1
 if pde&128:return (pde&0xffc00000)|(linear&0x3fffff)
 pte=struct.unpack_from('<I',m,(pde&0xfffff000)+((linear>>12)&1023)*4)[0]
 assert pte&1
 return (pte&0xfffff000)|(linear&4095)
def snapshot(label):
 q=state();m=memory();ds=q['segments'][3]['base']
 flat=lambda a:physical((ds+a)&0xffffffff,m,q)
 u32=lambda a:struct.unpack_from('<I',m,flat(a))[0]
 slots={hex(a):u32(a) for a in (0x90b,0x90f,0xc93,0xca1,0x2f50,0x2f54,0x2f58,0x2f60,
                              0x38ed,0x38f1,0x391c,0xbc98)}
 count=slots['0x2f54'];assert count<=100 # actual36bf cmp ESI,64h
 q.update(memmgr_slots=slots,
          blocks=[{'slot':u32(0x28ac+i*4),'address':u32(0x2a3c+i*4),
                   'size':u32(0x2bcc+i*4),'alignment':u32(0x2d5c+i*4),
                   'flags':m[flat(0x2eec+i)],'slot_value':u32(u32(0x28ac+i*4))} for i in range(count)],
          tcb_physical=flat(slots['0xc93']),
          stack_return=struct.unpack_from('<I',m,physical((q['segments'][2]['base']+q['registers'][4])&0xffffffff,m,q))[0])
 if label.startswith('kdv') or label.startswith('lookup') or label=='free-return':
  engine=flat(slots['0xca1']);off,seg=struct.unpack_from('<HH',m,engine+0xea2c)
  q.update(engine_dgroup_physical=engine,engine_task={'segment':seg,'offset':off,
             'physical':physical((seg<<4)+off,m,q)})
 (root/(label+'.json')).write_text(json.dumps(q,indent=2)+'\n')
 (root/(label+'.memory')).write_bytes(m)
 return q,m
watches=[];allocations=0;allocator=None
def watch(entry,label,q,m):
 base=int(gdb.parse_and_eval('MemBase'));code=q['segments'][1]['base']
 probe=CodeRead(base+physical((code+entry)&0xffffffff,m,q),entry,label)
 watches.append(probe);return probe
class CodeRead(gdb.Breakpoint):
 def __init__(self,addr,entry,label):
  self.entry=entry;self.label=label
  super().__init__('*(unsigned char *)(%d)'%addr,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True)
 def stop(self):
  global allocations,allocator
  q=state()
  if q['segments'][1]['value']!=43 or q['cpu_regs.ip.dword[0]']!=self.entry:return False
  label=self.label
  if label=='allocation':
   allocations+=1;assert allocations<=8
   q,m=snapshot('alloc-%02d-entry'%allocations)
   watch(q['stack_return'],'alloc-%02d-return'%allocations,q,m)
  else:
   q,m=snapshot(label);self.enabled=False
   if label=='init-entry':allocator=watch(0x36bf,'allocation',q,m)
   elif label=='alloc-08-return':
    allocator.enabled=False;watch(0x859c,'tcb-clear',q,m)
   elif label=='tcb-clear':watch(0x85a3,'init-return',q,m)
   elif label=='init-return':watch(0x6e95,'kdv-entry',q,m)
   elif label=='kdv-entry':watch(0x3322,'kdv-free-entry',q,m)
   elif label=='kdv-free-entry':watch(0x3661,'lookup-entry',q,m)
   elif label=='lookup-entry':watch(0x332f,'lookup-return',q,m)
   elif label=='lookup-return':watch(0x6ea6,'free-return',q,m)
  return False
class CSWrite(gdb.Breakpoint):
 def stop(self):
  q=state()
  if q['segments'][1]['value']!=43:return False
  m=memory();image=(repo/'re_out/fist_image.bin').read_bytes()
  addr=physical((q['segments'][1]['base']+0x84c0)&0xffffffff,m,q)
  if m[addr:addr+5]!=image[0x84c0:0x84c5]:return False
  self.enabled=False;watch(0x84c0,'init-entry',q,m);return False
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=10:return False
  CSWrite('Segs.val[1]',type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_WRITE,internal=True)
  self.enabled=False;return False
Arm('TIMER_AddTick')
end
run
