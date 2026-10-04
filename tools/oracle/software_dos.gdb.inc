# Actual reached INT21 API and final RETF pair; observation never writes guest state.
software_active=False;software_done=False;software_serial=0
software_sparse=bytearray(16777216)
def software_fetch_state():
 q=state()
 base=int(gdb.parse_and_eval('MemBase'));inferior=gdb.selected_inferior()
 linear=(q['segments'][1]['base']+q['cpu_regs.ip.dword[0]'])&0xffffffff
 if q['paging.enabled']:
  directory=(q['paging.cr3']&0xfffff000)+4*(linear>>22)
  software_sparse[directory:directory+4]=bytes(inferior.read_memory(base+directory,4))
  pde=struct.unpack_from('<I',software_sparse,directory)[0]
  if not pde&128:
   table=(pde&0xfffff000)+4*((linear>>12)&1023)
   software_sparse[table:table+4]=bytes(inferior.read_memory(base+table,4))
 address=physical(linear,software_sparse,q)
 q.update(fetched_code_physical=address,fetched_code_hex=bytes(inferior.read_memory(base+address,32)).hex())
 return q
def software_record(kind,arguments=None):
 global software_serial
 software_serial+=1
 q=state();q.update(kind=kind,serial=software_serial)
 if arguments:q['arguments']=arguments
 q['memory_file']=kind+'.memory'
 q['memory_context']=memory_context(q['memory_file'])
 (root/q['memory_file']).write_bytes(memory())
 with (root/'software-events.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
 return q
class SoftwareReturned(gdb.FinishBreakpoint):
 def __init__(self):super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  software_record('after-software');return False
class Software(gdb.Breakpoint):
 def stop(self):
  global software_active,software_done
  if software_done or int(gdb.parse_and_eval('cpu_regs.ip.dword[0]'))!=0x5cdd:return False
  q=state()
  if q['segments'][1]['value']!=ext_cs:return False
  args={key:int(gdb.parse_and_eval(key)) for key in ('num','type','oldeip')}
  assert args==dict(num=0x21,type=1,oldeip=0x5cdf),args
  software_active=True;software_done=True
  software_caller.enabled=True
  software_record('before-software',args);SoftwareReturned();return False
class SoftwareFarReturned(gdb.FinishBreakpoint):
 def __init__(self):super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  q=software_record('after-software-ret')
  assert q['segments'][1]['value']==ext_cs and q['cpu_regs.ip.dword[0]']==0x5cdf
  return False
class SoftwareFar(gdb.Breakpoint):
 def stop(self):
  if not software_active:return False
  q=state()
  if q['segments'][1]['value']!=8 or q['cpu_regs.ip.dword[0]']!=0x1b41:return False
  args={key:int(gdb.parse_and_eval(key)) for key in ('use32','bytes','oldeip')}
  software_record('before-software-ret',args);SoftwareFarReturned();return False
class SoftwareCaller(gdb.Breakpoint):
 def stop(self):
  global software_active
  if not software_active:return False
  q=software_fetch_state()
  with (root/'software-fetches.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
  if q['segments'][1]['value']!=ext_cs or q['cpu_regs.ip.dword[0]']!=0x5cdf:return False
  software_record('software-caller');software_active=False;self.enabled=False;return False
Software('CPU_Interrupt');SoftwareFar('CPU_RET')
software_caller=SoftwareCaller('fist_cpu_trace');software_caller.enabled=False
