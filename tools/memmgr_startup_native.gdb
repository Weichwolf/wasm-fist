set pagination off
set confirm off
python
import gdb,json,os,struct
from pathlib import Path
root=Path(os.environ['FIST_MEMMGR_PORT_DIR'])
def snapshot(label):
 inferior=gdb.selected_inferior();base=int(gdb.parse_and_eval('&g_mem[0]'));module=int(gdb.parse_and_eval('fist_ext_base'))
 m=bytes(inferior.read_memory(base,16777216));u=lambda a:struct.unpack_from('<I',m,module+a)[0]
 count=u(0x2f54);assert count<=100
 task=u(0xc93);off,seg=struct.unpack_from('<HH',m,0x2aa2c)
 q=dict(g_mem_host=base,module_offset=module,block_count=count,checkpoint=u(0xbc98),
        reason=u(0xd82),current_TCB=task,task_status=struct.unpack('<H',bytes(inferior.read_memory(task,2)))[0],
        engine_task_far={'offset':off,'segment':seg,'address':base+(seg<<4)+off},
        blocks=[dict(slot=u(0x28ac+i*4),address=u(0x2a3c+i*4),size=u(0x2bcc+i*4),
                     alignment=u(0x2d5c+i*4),flags=m[module+0x2eec+i]) for i in range(count)])
 (root/(label+'.json')).write_text(json.dumps(q,indent=2)+'\n');(root/(label+'.memory')).write_bytes(m)
class Finished(gdb.FinishBreakpoint):
 def stop(self):snapshot('constructor');return False
class Init(gdb.Breakpoint):
 def stop(self):Finished(gdb.newest_frame(),internal=True);self.enabled=False;return False
class Kdv(gdb.Breakpoint):
 def stop(self):snapshot('kdv-entry');self.enabled=False;return False
class Error(gdb.Breakpoint):
 def stop(self):
  q=dict(reason=int(gdb.parse_and_eval('param_1')),backtrace=gdb.execute('bt 18',to_string=True))
  with (root/'errors.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
  return False
class Exit(gdb.Breakpoint):
 def stop(self):snapshot('final');self.enabled=False;return False
Init('ext_module_init');Kdv('m_ext_FUN_0000_6e95');Error('m_ext_FUN_0000_0f64');Exit('exit')
end
run
