set pagination off
set confirm off
python
import gdb,json,os,struct
from pathlib import Path
root=Path(os.environ['FIST_DTA_DIAG_DIR'])
expected=int(os.environ['FIST_DTA_EXPECTED_OFFSET'])
class Return(gdb.FinishBreakpoint):
 def __init__(self):super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  base=int(gdb.parse_and_eval('&g_mem[0]'));module=int(gdb.parse_and_eval('fist_ext_base'))
  read=lambda address,size:bytes(gdb.selected_inferior().read_memory(base+module+address,size))
  row={'g_mem_host':base,'module_offset':module,'stored_DTA_DWORD':struct.unpack('<I',read(0x927,4))[0],'reserved_module_DTA_hex':read(expected,128).hex(),'original_initializer_code_hex':read(0xa88,11).hex(),'ready':int(gdb.parse_and_eval('g_ext_ready')),'backtrace':gdb.execute('backtrace',to_string=True)}
  (root/'state.json').write_text(json.dumps(row,indent=2)+'\n');return True
class Entry(gdb.Breakpoint):
 def stop(self):Return();self.enabled=False;return False
Entry('ext_module_init')
end
run
quit
