set pagination off
set confirm off
python
import gdb,json,os,struct
from pathlib import Path
root=Path(os.environ['FIST_CALLBACK_DIAG_DIR']);count=0

def snapshot(name,extra=None):
 data=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('&g_mem[0]')),16777216))
 row={'name':name,'callback':struct.unpack_from('<HH',data,0x14628),'stale_alias':data[0x14f98:0x14f9c].hex(),'cf':int(gdb.parse_and_eval('g_fist_cf')),'clock':{n:int(gdb.parse_and_eval(n)) for n in ('g_clock','g_clock_fraction','g_cpu_remaining','g_cpu_time.counts','g_cpu_time.fraction')},'backtrace':gdb.execute('backtrace',to_string=True)}
 if extra:row.update(extra)
 (root/(name+'.memory')).write_bytes(data);(root/(name+'.json')).write_text(json.dumps(row,indent=2)+'\n')
class Return(gdb.FinishBreakpoint):
 def __init__(self):super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):snapshot('after',{'return_pointer':int(self.return_value)});return False
class Entry(gdb.Breakpoint):
 def stop(self):
  global count
  count+=1;assert count==1
  snapshot('before',{'incoming_offset':int(gdb.parse_and_eval('param_1')),'incoming_segment':int(gdb.parse_and_eval('param_2'))});Return();self.enabled=False;return False
class Poster(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('*(unsigned short *)(g_mem+0x2aa10)'))!=0x6c:return False
  assert count==1;snapshot('first-6c');return True
Entry('FUN_1000_46b6');Poster('fist_extender_gate')
end
run
quit
