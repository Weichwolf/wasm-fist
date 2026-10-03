set pagination off
set confirm off
python
import gdb,json,os
root=os.environ['FIST_INIT_CALL_DIR']+'/'
def snapshot(label):
 values={name:int(gdb.parse_and_eval(name)) for name in ['PIC_Ticks','CPU_Cycles','CPU_CycleLeft','cpu_regs.ip.dword[0]','cpu.cr0','paging.cr3','cpu.code.big']}
 values['registers']=[int(gdb.parse_and_eval('cpu_regs.regs[%d].dword[0]'%i)) for i in range(8)]
 values['segments']=[{'value':int(gdb.parse_and_eval('Segs.val[%d]'%i)), 'base':int(gdb.parse_and_eval('Segs.phys[%d]'%i))} for i in range(6)]
 with open(root+label+'-registers.json','w') as out:json.dump(values,out,indent=2)
 memory=gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('MemBase')),16*1024*1024)
 with open(root+label+'-physical.bin','wb') as out:out.write(memory)
class After(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks')) != 535 or int(gdb.parse_and_eval('Segs.val[1]')) != 43 or int(gdb.parse_and_eval('cpu_regs.ip.dword[0]')) != 0x138d:return False
  snapshot('after');self.enabled=False;return False
after=After('CPU_CLI');after.enabled=False
wanted={0x23ec:'before',0x2630:'mixer'}
class Capture(gdb.Breakpoint):
 def stop(self):
  ip=int(gdb.parse_and_eval('cpu_regs.ip.dword[0]'))
  if int(gdb.parse_and_eval('PIC_Ticks')) != 534 or int(gdb.parse_and_eval('Segs.val[1]')) != 43 or ip not in wanted:return False
  snapshot(wanted.pop(ip))
  if not wanted:self.enabled=False;after.enabled=True
  return False
capture=Capture('fist_cpu_trace');capture.enabled=False
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks')) != 533:return False
  capture.enabled=True;self.enabled=False;return False
arm=Arm('TIMER_AddTick');arm.condition='PIC_Ticks == 533'
end
run
