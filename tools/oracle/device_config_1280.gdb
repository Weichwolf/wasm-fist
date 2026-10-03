set pagination off
set confirm off
python
import gdb,json,os,struct
root=os.environ['FIST_DEVICE_CONFIG_DIR']+'/'
widths=os.environ.get('FIST_DEVICE_CONFIG_WIDTHS')=='1'
saved={}
def state():
 names=['PIC_Ticks','CPU_Cycles','CPU_CycleLeft','cpu_regs.ip.dword[0]','cpu.cr0','paging.cr3','cpu.code.big','cpu.stack.big','cpu.pmode','cpu.cpl','cpu_regs.flags','lflags.type','lflags.prev_type','lflags.oldcf','lflags.var1.dword[0]','lflags.var2.dword[0]','lflags.res.dword[0]']
 values={name:int(gdb.parse_and_eval(name)) for name in names}
 values['registers']=[int(gdb.parse_and_eval('cpu_regs.regs[%d].dword[0]'%i)) for i in range(8)]
 values['segments']=[{'value':int(gdb.parse_and_eval('Segs.val[%d]'%i)), 'base':int(gdb.parse_and_eval('Segs.phys[%d]'%i))} for i in range(6)]
 return values
def snapshot(label):
 with open(root+label+'-registers.json','w') as out:json.dump(state(),out,indent=2)
 memory=gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('MemBase')),16*1024*1024)
 with open(root+label+'-physical.bin','wb') as out:out.write(memory)
def physical(off):
 address=(int(gdb.parse_and_eval('Segs.phys[3]'))+off)&0xffffffff
 memory=gdb.selected_inferior();base=int(gdb.parse_and_eval('MemBase'));cr3=int(gdb.parse_and_eval('paging.cr3'))&~4095
 pde=struct.unpack('<I',memory.read_memory(base+cr3+4*(address>>22),4))[0]
 pte=struct.unpack('<I',memory.read_memory(base+(pde&~4095)+4*((address>>12)&1023),4))[0]
 assert pde&pte&1
 return base+(pte&~4095)+(address&4095)
class Capture(gdb.Breakpoint):
 def stop(self):
  ip=int(gdb.parse_and_eval('cpu_regs.ip.dword[0]'))
  if int(gdb.parse_and_eval('PIC_Ticks')) != 525 or int(gdb.parse_and_eval('Segs.val[1]')) != 43 or ip not in (0x1280,0x77ee):return False
  label='before' if ip==0x1280 else 'after'
  memory=gdb.selected_inferior()
  if widths and label=='before':
   snapshot('unmodified-before')
   tcb=struct.unpack('<I',memory.read_memory(physical(0xc93),4))[0]
   saved['tcb']=tcb
   saved['packet']=bytes(memory.read_memory(physical(tcb+0x490),6))
   saved['config']=bytes(memory.read_memory(physical(0x12c4),12))
   memory.write_memory(physical(tcb+0x490),struct.pack('<HHH',0xabc1,0xffa5,0xf123))
   memory.write_memory(physical(0x12ce),b'\x5a\xc3')
   gdb.execute('set variable cpu_regs.regs[0].dword[0] = 0x89ab0201')
   gdb.execute('set variable cpu_regs.regs[3].dword[0] = 0x65c4abcd')
  snapshot(label)
  if widths and label=='after':
   memory.write_memory(physical(saved['tcb']+0x490),saved['packet'])
   memory.write_memory(physical(0x12c4),saved['config'])
   original_dma=struct.unpack_from('<H',saved['packet'],4)[0]
   gdb.execute('set variable cpu_regs.regs[0].dword[0] = %d'%original_dma)
   snapshot('restored-after')
  if label=='after':self.enabled=False
  return False
capture=Capture('fist_cpu_trace');capture.enabled=False
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks')) != 524:return False
  capture.enabled=True;self.enabled=False;return False
arm=Arm('TIMER_AddTick');arm.condition='PIC_Ticks == 524'
end
run
