set pagination off
set confirm off
python
import gdb,json,os,struct
from pathlib import Path
root=Path(os.environ['FIST_FILE_CLOSE_DIAG_DIR']);counts={'config':0,'size':0,'de89':0,'op68':0};events=[]
base=int(gdb.parse_and_eval('&g_mem[0]'))
def regs():return list(struct.unpack('<11H',bytes(gdb.selected_inferior().read_memory(base+0xf0000,22))))
def handles():
 out=[]
 for h in range(5,256):
  if int(gdb.parse_and_eval('g_htab[%d]'%h)):
   fd=int(gdb.parse_and_eval('g_htab[%d]->_fileno'%h));path=os.readlink('/proc/%d/fd/%d'%(gdb.selected_inferior().pid,fd))
   out.append({'handle':h,'file':Path(path).name})
 return out
def state():return {'dos_regs':regs(),'handles':handles(),'clock':{n:int(gdb.parse_and_eval(n)) for n in ('g_clock','g_clock_fraction','g_cpu_remaining')},'backtrace':gdb.execute('backtrace 8',to_string=True)}
def write(name,row):(root/name).write_text(json.dumps(row,indent=2)+'\n')
class DOSReturn(gdb.FinishBreakpoint):
 def __init__(self,row):self.row=row;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  self.row['after']=state();events.append(self.row);write('dos-calls.json',events);return False
class DOS(gdb.Breakpoint):
 def stop(self):
  r=regs();command=r[0]>>8
  if r[10]!=0x21 or command not in (0x3d,0x3e,0x3f,0x42):return False
  row={'command':command,'before':state()}
  if command==0x3d:
   ptr=(r[7]<<4)+r[3];raw=bytes(gdb.selected_inferior().read_memory(base+ptr,256));row['name']=raw.split(b'\0',1)[0].decode('ascii')
  DOSReturn(row);return False
class HelperReturn(gdb.FinishBreakpoint):
 def __init__(self,label):self.label=label;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  row=state();row['return_ax']=int(self.return_value)&65535;write(self.label+'-after.json',row);return False
class Helper(gdb.Breakpoint):
 def __init__(self,function,label,param):self.label=label;self.param=param;super().__init__(function)
 def stop(self):
  counts[self.label]+=1;label=self.label+'-'+str(counts[self.label]);row=state();row['unrelated_close_param']=int(gdb.parse_and_eval(self.param));write(label+'-before.json',row);HelperReturn(label);return False
class Before(gdb.Breakpoint):
 def stop(self):
  counts['de89']+=1;row=state();row['param_1']=int(gdb.parse_and_eval('param_1'));write('de89-before.json',row);self.enabled=False;return False
class Poster(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('*(unsigned short *)(g_mem+0x2aa10)'))!=0x68:return False
  counts['op68']+=1;row=state();offset,segment=struct.unpack('<HH',bytes(gdb.selected_inferior().read_memory(base+0x2aa2c,4)));row['task_pointer']=[offset,segment];row['inbox_ebx']=struct.unpack('<I',bytes(gdb.selected_inferior().read_memory(base+(segment<<4)+offset+0x3f2,4)))[0];write('op68-gate.json',row);write('counts.json',counts);return True
DOS('fist_int_dispatch');Helper('FUN_0000_fefb','config','param_9');Helper('FUN_1000_50c8','size','param_7');Before('FUN_0000_de89');Poster('fist_extender_gate')
end
run
quit
