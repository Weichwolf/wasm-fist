set pagination off
set confirm off
python
import gdb,json,os,struct
from pathlib import Path
root=Path(os.environ['FIST_DETAIL_INIT_DIR']);repo=Path(os.environ['FIST_DETAIL_REPO'])
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
def snapshot(name):
 q=state();m=memory();q['dta_physical']=physical(0x10000927,m,q);q['dta_value']=struct.unpack_from('<I',m,q['dta_physical'])[0]
 (root/(name+'.json')).write_text(json.dumps(q,indent=2)+'\n');(root/(name+'.memory')).write_bytes(m)
class Capture(gdb.Breakpoint):
 def stop(self):
  q=state();cs=q['segments'][1]['value'];ip=q['cpu_regs.ip.dword[0]'];assert cs==43
  if ip==0xa8d:snapshot('before-store')
  elif ip==0xa93:snapshot('after-store');self.enabled=False
  else:raise AssertionError((cs,ip))
  with (root/'fetches.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
  return False
capture=Capture('fist_cpu_trace');capture.enabled=False
class CodeRead(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('Segs.val[1]'))!=43 or int(gdb.parse_and_eval('cpu_regs.ip.dword[0]'))!=0xa88:return False
  snapshot('before-load');capture.enabled=True;self.enabled=False;return False
class CSWrite(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('Segs.val[1]'))!=43:return False
  q=state();m=memory();addr=physical(0x10000a88,m,q)
  image=(repo/'re_out/fist_image.bin').read_bytes()
  if m[addr:addr+5]!=image[0xa88:0xa8d]:return False
  (root/'arm.json').write_text(json.dumps({'state':q,'code_physical':addr},indent=2)+'\n')
  base=int(gdb.parse_and_eval('MemBase'));CodeRead('*(unsigned char *)(%d+%d)'%(base,addr),type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True);self.enabled=False;return False
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=10:return False
  CSWrite('Segs.val[1]',type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_WRITE,internal=True);self.enabled=False;return False
Arm('TIMER_AddTick')
end
run
