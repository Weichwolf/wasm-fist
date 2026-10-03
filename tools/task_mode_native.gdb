set pagination off
set confirm off
python
import gdb,json,os,struct
from pathlib import Path
root=Path(os.environ['FIST_TASK_MODE_DIAG_DIR']);counts={'get':0,'put':0,'poster':0}
def observe(name,extra=None):
 base=int(gdb.parse_and_eval('&g_mem[0]'));memory=bytes(gdb.selected_inferior().read_memory(base,16*1024*1024));offset,segment=struct.unpack_from('<HH',memory,0x2aa2c)
 row={'name':name,'mode':memory[0x123e9],'mode_neighbor':memory[0x123ea],'pointer_offset':offset,'pointer_segment':segment,'task_mode_address':(segment<<4)+((offset+0x496)&65535),'clock':{n:int(gdb.parse_and_eval(n)) for n in ('g_clock','g_clock_fraction','g_cpu_remaining','g_cpu_time.counts','g_cpu_time.fraction')},'backtrace':gdb.execute('backtrace',to_string=True)}
 if extra:row.update(extra)
 (root/(name+'.memory')).write_bytes(memory);(root/(name+'.json')).write_text(json.dumps(row,indent=2)+'\n')
class Return(gdb.FinishBreakpoint):
 def __init__(self,name):super().__init__(gdb.newest_frame(),internal=True);self.label=name
 def stop(self):
  assert self.return_value is not None
  observe(self.label,{'return_byte':int(self.return_value)});return False
class Get(gdb.Breakpoint):
 def stop(self):
  caller=gdb.newest_frame().older()
  if caller is None or caller.name()!='FUN_0000_d99b':return False
  counts['get']+=1;assert counts['get']==1
  observe('get-before');Return('get-after');return False
class Put(gdb.Breakpoint):
 def stop(self):
  caller=gdb.newest_frame().older()
  if caller is None or caller.name()!='FUN_0000_d99b':return False
  counts['put']+=1;assert counts['put']==1
  observe('put-before',{'incoming_byte':int(gdb.parse_and_eval('param_1'))});Return('put-after');return False
class Poster(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('*(unsigned short *)(g_mem+0x2aa10)'))!=0x6c:return False
  counts['poster']+=1;assert counts=={'get':1,'put':1,'poster':1}
  observe('first-6c');(root/'counts.json').write_text(json.dumps(counts)+'\n');return True
Get('FUN_1000_23ba');Put('FUN_1000_23bf');Poster('fist_extender_gate')
end
run
backtrace
quit
