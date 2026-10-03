set pagination off
set confirm off
python
import gdb,json,os,struct
from pathlib import Path
root=Path(os.environ['FIST_TASK_MODE_OUTPUT']);case=os.environ['FIST_TASK_MODE_CASE'];assert case in ('natural','nonzero','exchange','null')
fields=['PIC_Ticks','CPU_Cycles','CPU_CycleLeft','cpu_regs.ip.dword[0]','cpu.cr0','paging.cr3','cpu.code.big','cpu.stack.big','cpu.pmode','cpu.cpl','cpu_regs.flags','lflags.type','lflags.prev_type','lflags.oldcf','lflags.var1.dword[0]','lflags.var2.dword[0]','lflags.res.dword[0]']
inside=False;original=None;load_bias=None

def state():
 q={name:int(gdb.parse_and_eval(name)) for name in fields};q['registers']=[int(gdb.parse_and_eval('cpu_regs.regs[%d].dword[0]'%i)) for i in range(8)];q['segments']=[{'value':int(gdb.parse_and_eval('Segs.val[%d]'%i)),'base':int(gdb.parse_and_eval('Segs.phys[%d]'%i))} for i in range(6)];return q

def memory():return bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('MemBase')),16*1024*1024))
def write(address,data):gdb.selected_inferior().write_memory(int(gdb.parse_and_eval('MemBase'))+address,data)
def snapshot(name):
 (root/(name+'.json')).write_text(json.dumps(state(),indent=2)+'\n');(root/(name+'.memory')).write_bytes(memory())

class Capture(gdb.Breakpoint):
 def stop(self):
  global inside,original
  q=state();cs=q['segments'][1]['value'];ip=q['cpu_regs.ip.dword[0]']
  if not inside:
   assert cs==0x2082 and ip==0x2d2a
   inside=True;m=memory();ss=q['segments'][2]['base'];mode=q['segments'][1]['base']+0x2d59;off,seg=struct.unpack_from('<HH',m,ss+0xea2c);task=(seg<<4)+((off+0x496)&65535)
   original=dict(eax=q['registers'][0],mode_address=mode,mode=m[mode],neighbor=m[mode+1],pointer_address=ss+0xea2c,pointer=m[ss+0xea2c:ss+0xea30].hex(),task_address=task,task_mode=m[task],load_bias=load_bias,uncontrolled_get=q)
   (root/'inputs.json').write_text(json.dumps(original,indent=2)+'\n')
   if case!='natural':
    write(mode,b'\x42');gdb.execute('set cpu_regs.regs[0].dword[0] = %u'%((0x89ab7b<<8)|(q['registers'][0]&255)))
    if case=='nonzero':write(mode+1,b'\x7d')
   snapshot('get-entry')
  if cs==0x2082 and ip==0x2d2f:
   if case in ('exchange','null'):gdb.execute('set cpu_regs.regs[0].byte[0] = 0xa5')
   if case=='null':write(original['pointer_address'],b'\0\0\0\0')
   snapshot('put-entry')
  elif cs==0x1119 and ip==0xda4b:snapshot('get-return')
  elif cs==0x1119 and ip==0xda4f:
   snapshot('put-return')
   if case!='natural':
    write(original['mode_address'],bytes((original['mode'],original['neighbor'])));write(original['pointer_address'],bytes.fromhex(original['pointer']));write(original['task_address'],bytes((original['task_mode'],)))
    gdb.execute('set cpu_regs.regs[0].dword[0] = %u'%(original['eax']&0xffffff00))
    if case=='null':
     baseline=json.loads(Path(os.environ['FIST_TASK_MODE_NORMAL_RETURN']).read_text())
     for name in ('cpu_regs.flags','lflags.type','lflags.prev_type','lflags.oldcf','lflags.var1.dword[0]','lflags.var2.dword[0]','lflags.res.dword[0]'):gdb.execute('set %s = %u'%(name,baseline[name]))
   snapshot('restored-return');self.enabled=False
  with (root/'fetches.jsonl').open('a') as out:out.write(json.dumps(q if ip==0xda4f else state())+'\n')
  return False
capture=Capture('fist_cpu_trace');capture.enabled=False
class VectorRead(gdb.Breakpoint):
 def stop(self):
  global load_bias
  if int(gdb.parse_and_eval('Segs.val[1]'))!=0x1119 or int(gdb.parse_and_eval('cpu_regs.ip.dword[0]'))!=0xda47:return False
  load_bias=int(gdb.parse_and_eval('Segs.phys[1]'));capture.enabled=True;self.enabled=False;return False
class Arm(gdb.Breakpoint):
 def stop(self):
  base=int(gdb.parse_and_eval('MemBase'));VectorRead('*(unsigned int *)(%d+0x2d4e4)'%base,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True);self.enabled=False;return False
arm=Arm('TIMER_AddTick');arm.condition='PIC_Ticks == 19'
end
run
