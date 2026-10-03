set pagination off
set confirm off
python
import gdb,json,os,struct
from pathlib import Path
root=Path(os.environ['FIST_DETAIL_OPERANDS_DIR']);repo=Path(os.environ['FIST_DETAIL_REPO']);level=os.environ.get('FIST_DETAIL_LEVEL');sky=os.environ.get('FIST_DETAIL_SKY');count=0
fields=('PIC_Ticks','CPU_Cycles','CPU_CycleLeft','CPU_CycleMax','cpu_regs.ip.dword[0]','cpu.cr0','paging.cr3','paging.enabled','cpu.code.big','cpu.stack.big','cpu.pmode','cpu.cpl','cpu_regs.flags','lflags.type','lflags.prev_type','lflags.oldcf','lflags.var1.dword[0]','lflags.var2.dword[0]','lflags.res.dword[0]')
def state():
 q={n:int(gdb.parse_and_eval(n)) for n in fields};q['registers']=[int(gdb.parse_and_eval('cpu_regs.regs[%d].dword[0]'%i)) for i in range(8)];q['segments']=[{'value':int(gdb.parse_and_eval('Segs.val[%d]'%i)),'base':int(gdb.parse_and_eval('Segs.phys[%d]'%i))} for i in range(6)];return q
def memory():return bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('MemBase')),16777216))
def physical(linear,m,q):
 if not q['paging.enabled']:return linear
 directory=q['paging.cr3']&0xfffff000;pde=struct.unpack_from('<I',m,directory+((linear>>22)&1023)*4)[0];assert pde&1
 if pde&128:return (pde&0xffc00000)|(linear&0x3fffff)
 pte=struct.unpack_from('<I',m,(pde&0xfffff000)+((linear>>12)&1023)*4)[0];assert pte&1
 return (pte&0xfffff000)|(linear&4095)
def snapshot(label):
 q=state();m=memory();ds=q['segments'][3]['base'];flat=lambda a:physical((ds+a)&0xffffffff,m,q)
 tcb=struct.unpack_from('<I',m,flat(0xc93))[0];tp=flat(tcb);dp=flat(struct.unpack_from('<I',m,flat(0x927))[0])
 q.update(task_status=struct.unpack_from('<H',m,tp)[0],error_reason=struct.unpack_from('<I',m,flat(0xd82))[0],saved_service_ESP=struct.unpack_from('<I',m,flat(0xf60))[0],error_reason_physical=flat(0xd82),detail_mode=m[flat(0x395c)],tcb_logical=tcb,tcb_physical=tp,detail=m[tp+0xd1],sky=m[tp+0xcc],DTA_hex=m[dp:dp+128].hex(),ext_slots={hex(a):struct.unpack_from('<I',m,flat(a))[0] for a in (0x927,0x937,0x3958,0x3a20)},detail_table_hex=m[flat(0x3a20):flat(0x3a20)+2052].hex())
 if q['cpu_regs.ip.dword[0]']==0x6032:q['filename']=m[flat(q['registers'][6]):flat(q['registers'][6])+32].split(b'\0',1)[0].decode('ascii')
 (root/(label+'.json')).write_text(json.dumps(q,indent=2)+'\n');(root/(label+'.memory')).write_bytes(m)
class Fetch(gdb.Breakpoint):
 def stop(self):
  with (root/'fetches.jsonl').open('a') as out:out.write(json.dumps(state())+'\n')
  return False
fetch=Fetch('fist_cpu_trace');fetch.enabled=False
watches=[]
class CodeRead(gdb.Breakpoint):
 def __init__(self,addr,expected):self.expected=expected;super().__init__('*(unsigned char *)(%d)'%addr,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True)
 def stop(self):
  q=state();cs=q['segments'][1]['value'];ip=q['cpu_regs.ip.dword[0]']
  if cs!=43 or ip!=self.expected:return False
  snapshot('at-%04x'%ip);self.enabled=False
  m=memory();base=int(gdb.parse_and_eval('MemBase'));code=q['segments'][1]['base']
  if ip==0x6032:
   for entry in (0x6044,0x5e3a,0x0f64):watches.append(CodeRead(base+physical((code+entry)&0xffffffff,m,q),entry))
  elif ip==0x6044:watches.append(CodeRead(base+physical((code+0xf57)&0xffffffff,m,q),0xf57))
  elif ip==0x5e3a:watches.append(CodeRead(base+physical((code+0xf5d)&0xffffffff,m,q),0xf5d))
  elif ip==0xf64:fetch.enabled=True
  elif ip==0xf5d:fetch.enabled=False
  return False
class CSWrite(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('Segs.val[1]'))!=43:return False
  q=state();m=memory();image=(repo/'re_out/fist_image.bin').read_bytes();base=int(gdb.parse_and_eval('MemBase'));addr=physical((q['segments'][1]['base']+0x6032)&0xffffffff,m,q)
  if m[addr:addr+5]!=image[0x6032:0x6037]:return False
  self.enabled=False;watches.append(CodeRead(base+addr,0x6032));return False
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=500:return False
  CSWrite('Segs.val[1]',type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_WRITE,internal=True);self.enabled=False;return False
Arm('TIMER_AddTick')
end
run
